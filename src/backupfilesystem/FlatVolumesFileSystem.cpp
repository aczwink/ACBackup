/*
 * Copyright (c) 2019-2020 Amir Czwink (amir130@hotmail.de)
 *
 * This file is part of ACBackup.
 *
 * ACBackup is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ACBackup is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ACBackup.  If not, see <http://www.gnu.org/licenses/>.
 */
//Class header
#include "FlatVolumesFileSystem.hpp"
//Local
#include "VolumesOutputStream.hpp"
#include "../InjectionContainer.hpp"
#include "../config/ConfigManager.hpp"
#include "../Util.hpp"
#include "FlatVolumesFile.hpp"
#include "FlatVolumesDirectory.hpp"
#include "FlatVolumesLink.hpp"

//Constructor
FlatVolumesFileSystem::FlatVolumesFileSystem(const Path &dirPath, BackupNodeIndex& index)
		: FileSystem(nullptr), dirPath(dirPath), index(index)
{
	this->writing.createdDataDir = false;
	this->writing.nextVolumeNumber = 0;

	//count volumes
	BinaryTreeSet<uint64> volumes;
	for(uint32 i = 0; i < index.GetNumberOfNodes(); i++)
	{
		BackupNodeAttributes& attributes = index.GetNodeAttributes(i);
		for(const Block& block : attributes.Blocks())
			volumes.Insert(block.volumeNumber);
	}

	this->reading.openVolumes = new FixedArray<OpenVolumeForReading>(volumes.GetNumberOfElements());
}

//Public methods
void FlatVolumesFileSystem::CloseFile(const VolumesOutputStream& outputStream)
{
	AutoLock lock(this->writing.openVolumesMutex);
	for(OpenVolumeForWriting& openVolume : this->writing.openVolumes)
	{
		if(openVolume.ownedWriter == &outputStream)
		{
			openVolume.ownedWriter = nullptr;
			break;
		}
	}
}

UniquePointer<OutputStream> FlatVolumesFileSystem::CreateFile(const Path &filePath)
{
	return new VolumesOutputStream(*this, filePath);
}

bool FlatVolumesFileSystem::Exists(const Path &path) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return false;
}

void FlatVolumesFileSystem::Flush()
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
}

AutoPointer<FileSystemNode> FlatVolumesFileSystem::GetNode(const Path &path)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return AutoPointer<FileSystemNode>();
}

AutoPointer<const FileSystemNode> FlatVolumesFileSystem::GetNode(const Path &path) const
{
	if(this->index.HasNodeIndex(path))
	{
		uint32 index = this->index.GetNodeIndex(path);
		const BackupNodeAttributes &attributes = this->index.GetNodeAttributes(index);

		switch (attributes.Type())
		{
			case FileSystemNodeType::Directory:
				return new FlatVolumesDirectory(index, this->index);
			case FileSystemNodeType::File:
				return new FlatVolumesFile(index, this->index, const_cast<FlatVolumesFileSystem &>(*this));
			case FileSystemNodeType::Link:
				return new FlatVolumesLink(index, this->index, *this);
		}
	}

	return nullptr;
}

AutoPointer<Directory> FlatVolumesFileSystem::GetRoot()
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return AutoPointer<Directory>();
}

AutoPointer<const Directory> FlatVolumesFileSystem::GetRoot() const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return AutoPointer<const Directory>();
}

uint64 FlatVolumesFileSystem::GetSize() const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return 0;
}

void FlatVolumesFileSystem::Move(const Path &from, const Path &to)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
}

UniquePointer<InputStream> FlatVolumesFileSystem::OpenLinkTargetAsStream(const Path& linkPath, bool verify) const
{
	uint32 nodeIndex = this->index.GetNodeIndex(linkPath);

	FlatVolumesFile file(nodeIndex, this->index, const_cast<FlatVolumesFileSystem&>(*this));
	return file.OpenForReading(verify);
}

uint32 FlatVolumesFileSystem::ReadBytes(const FlatVolumesBlockInputStream &reader, void *destination, uint64 volumeNumber, uint64 offset, uint32 count) const
{
	uint8* dest = static_cast<uint8 *>(destination);
	while(count)
	{
		SeekableInputStream& inputStream = this->LockVolumeStream(volumeNumber);
		inputStream.SetCurrentOffset(offset);
		uint32 nBytesRead = inputStream.ReadBytes(dest, count);
		this->UnlockVolumeStream(volumeNumber);

		dest += nBytesRead;
		count -= nBytesRead;
	}

	return dest - static_cast<uint8 *>(destination);
}

void FlatVolumesFileSystem::WriteBytes(const VolumesOutputStream& writer, const void *source, uint32 size)
{
	const uint8* src = static_cast<const uint8 *>(source);
	while(size)
	{
		uint64 leftSize;
		SeekableOutputStream& outputStream = this->FindStream(&writer, leftSize);

		uint32 bytesToWrite = Math::Min( (uint32)leftSize, (uint32)size );
		uint64 offset = outputStream.GetCurrentOffset();
		uint32 nBytesWritten = outputStream.WriteBytes(src, bytesToWrite);

		src += nBytesWritten;
		size -= nBytesWritten;
		this->BytesWereWrittenToVolume(&writer, offset, nBytesWritten);
	}
}

void FlatVolumesFileSystem::WriteProtect()
{
	this->writing.openVolumes.Release(); //close open files

	auto dir = OSFileSystem::GetInstance().GetDirectory(this->dirPath);
	auto dirWalker = dir->WalkFiles();

	for(const Path& relPath : dirWalker)
	{
		WriteProtectFile(this->dirPath / relPath);
	}
	WriteProtectFile(this->dirPath);
}

//Private methods
void FlatVolumesFileSystem::BytesWereWrittenToVolume(const VolumesOutputStream* writer, uint64 offset, uint32 nBytesWritten)
{
	AutoLock lock(this->writing.openVolumesMutex);

	auto it = this->writing.openVolumes.begin();
	while(it != this->writing.openVolumes.end())
	{
		if((*it).ownedWriter == writer)
		{
			this->index.AddBlock(writer->Path(), (*it).number, offset, nBytesWritten);
			(*it).leftSize -= nBytesWritten;
			if((*it).leftSize == 0)
				it.Remove();
			break;
		}
	}
}

SeekableOutputStream &FlatVolumesFileSystem::FindStream(const OutputStream *writer, uint64 &leftSize)
{
	AutoLock lock(this->writing.openVolumesMutex);

	OpenVolumeForWriting* free = nullptr;
	for(OpenVolumeForWriting& openVolume : this->writing.openVolumes)
	{
		if(openVolume.ownedWriter == writer)
		{
			leftSize = openVolume.leftSize;
			return *openVolume.file;
		}
		if(openVolume.ownedWriter == nullptr)
			free = &openVolume;
	}

	if(free)
	{
		free->ownedWriter = writer;

		leftSize = free->leftSize;
		return *free->file;
	}

	if(!this->writing.createdDataDir)
	{
		OSFileSystem::GetInstance().GetDirectory(this->dirPath.GetParent())->CreateSubDirectory(this->dirPath.GetName());
		this->writing.createdDataDir = true;
	}

	OpenVolumeForWriting newVolume;
	newVolume.number = this->writing.nextVolumeNumber++;
	newVolume.file = new FileOutputStream(this->dirPath / String::Number(newVolume.number));
	newVolume.ownedWriter = writer;
	newVolume.leftSize = InjectionContainer::Instance().Get<ConfigManager>().Config().volumeSize;

	leftSize = newVolume.leftSize;
	SeekableOutputStream& result = *newVolume.file;

	this->writing.openVolumes.InsertTail(StdXX::Move(newVolume));

	return result;
}

SeekableInputStream &FlatVolumesFileSystem::LockVolumeStream(uint64 volumeNumber) const
{
	OpenVolumeForReading &volume = (*this->reading.openVolumes)[volumeNumber];
	volume.mutex.Lock();

	if(volume.file.IsNull())
		volume.file = new FileInputStream(this->dirPath / String::Number(volumeNumber));

	return *volume.file;
}

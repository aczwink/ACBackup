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

//Constructor
FlatVolumesFileSystem::FlatVolumesFileSystem(const Path &dirPath, BackupNodeIndex& index)
		: FileSystem(nullptr), dirPath(dirPath), index(index)
{
	this->writing.nextVolumeNumber = 0;
}

//Public methods
void FlatVolumesFileSystem::CloseFile(const VolumesOutputStream& outputStream)
{
	AutoLock lock(this->writing.openVolumesMutex);
	for(OpenVolume& openVolume : this->writing.openVolumes)
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
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return AutoPointer<const FileSystemNode>();
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

	OpenVolume* free = nullptr;
	for(OpenVolume& openVolume : this->writing.openVolumes)
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

	OpenVolume newVolume;
	newVolume.number = this->writing.nextVolumeNumber++;
	newVolume.file = new FileOutputStream(this->dirPath / String::Number(newVolume.number));
	newVolume.ownedWriter = writer;
	newVolume.leftSize = InjectionContainer::Instance().Get<ConfigManager>().Config().volumeSize;

	leftSize = newVolume.leftSize;
	SeekableOutputStream& result = *newVolume.file;

	this->writing.openVolumes.InsertTail(StdXX::Move(newVolume));

	return result;
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

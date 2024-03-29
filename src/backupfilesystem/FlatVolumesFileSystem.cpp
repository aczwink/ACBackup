/*
 * Copyright (c) 2019-2021 Amir Czwink (amir130@hotmail.de)
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
#include "FlatVolumesBlockInputStream.hpp"

//Constructor
FlatVolumesFileSystem::FlatVolumesFileSystem(const Path &dirPath, BackupNodeIndex& index)
		: dirPath(dirPath), index(index)
{
	this->writing.createdDataDir = false;
	this->writing.nextVolumeNumber = 0;

	//count volumes
	BinaryTreeSet<uint64> volumes;
	for(uint32 i = 0; i < index.GetNumberOfNodes(); i++)
	{
		const BackupNodeAttributes& attributes = index.GetNodeAttributes(i);
		for(const Block& block : attributes.Blocks())
			volumes.Insert(block.volumeNumber);
	}

	this->reading.volumes = new FixedArray<VolumeForReading>(volumes.GetNumberOfElements());
	this->reading.nOpenVolumes = 0;
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

void FlatVolumesFileSystem::CreateLink(const Path &linkPath, const Path &linkTargetPath)
{
	NOT_IMPLEMENTED_ERROR; //implement me
}

void FlatVolumesFileSystem::Flush()
{
	NOT_IMPLEMENTED_ERROR; //implement me
}

/*AutoPointer<const Node> FlatVolumesFileSystem::GetNode(const Path &path) const
{
	if(this->index.HasNodeIndex(path))
	{
		uint32 index = this->index.GetNodeIndex(path);
		const BackupNodeAttributes &attributes = this->index.GetNodeAttributes(index);

		switch (attributes.Type())
		{
			case NodeType::Directory:
				return new FlatVolumesDirectory(index, this->index);
			case NodeType::File:
				return new FlatVolumesFile(index, this->index, const_cast<FlatVolumesFileSystem &>(*this));
			case NodeType::Link:
				return new FlatVolumesLink(index, this->index, *this);
		}
	}

	return nullptr;
}*/

void FlatVolumesFileSystem::Move(const Path &from, const Path &to)
{
	NOT_IMPLEMENTED_ERROR; //implement me
}

UniquePointer<InputStream> FlatVolumesFileSystem::OpenFileForReading(uint32 fileIndex, bool verify) const
{
	const BackupNodeAttributes& attributes = this->index.GetNodeAttributes(fileIndex);
	this->IncrementVolumeCounters(attributes.Blocks());

	UniquePointer<InputStream> blockInputStream = new FlatVolumesBlockInputStream(*this, attributes.Blocks());

	ChainedInputStream* chain = new ChainedInputStream(StdXX::Move(blockInputStream));

	const Config &config = InjectionContainer::Instance().Config();

	chain->Add( new BufferedInputStream(chain->GetEnd()) );

	if(attributes.CompressionSetting().HasValue())
	{
		CompressionSettings compressionSettings;
		ConfigManager::GetCompressionSettings(*attributes.CompressionSetting(), compressionSettings);
		chain->Add(Decompressor::Create(compressionSettings.compressionStreamFormatType, chain->GetEnd(), verify));
	}

	if(verify)
	{
		Crypto::HashAlgorithm hashAlgorithm = config.hashAlgorithm;
		String expected = attributes.Hash(hashAlgorithm);
		chain->Add(new Crypto::CheckedHashingInputStream(chain->GetEnd(), hashAlgorithm, expected));
	}

	return chain;
}

UniquePointer<InputStream> FlatVolumesFileSystem::OpenFileForReading(const Path &path, bool verify) const
{
	return this->OpenFileForReading(this->index.GetNodeIndex(path), verify);
}

UniquePointer<InputStream> FlatVolumesFileSystem::OpenLinkTargetAsStream(const Path& linkPath, bool verify) const
{
	uint32 nodeIndex = this->index.GetNodeIndex(linkPath);

	return this->OpenFileForReading(nodeIndex, verify);
}

uint32 FlatVolumesFileSystem::ReadBytes(const FlatVolumesBlockInputStream &reader, void *destination, uint64 volumeNumber, uint64 offset, uint32 count) const
{
	this->CloseUnusedVolumes();

	uint8* dest = static_cast<uint8 *>(destination);
	while(count)
	{
		SeekableInputStream& inputStream = this->LockVolumeStream(volumeNumber);
		inputStream.SeekTo(offset);
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
		uint64 offset = outputStream.QueryCurrentOffset();
		uint32 nBytesWritten = outputStream.WriteBytes(src, bytesToWrite);

		src += nBytesWritten;
		size -= nBytesWritten;
		this->BytesWereWrittenToVolume(&writer, offset, nBytesWritten);
	}
}

void FlatVolumesFileSystem::WriteProtect()
{
	this->writing.openVolumes.Release(); //close open files

	File dir(this->dirPath);

	if(!dir.Exists())
		return;
	auto dirWalker = dir.WalkFiles();

	for(const Path& path : dirWalker)
	{
		WriteProtectFile(path);
	}
	WriteProtectFile(this->dirPath);
}

SpaceInfo FlatVolumesFileSystem::QuerySpace() const
{
	NOT_IMPLEMENTED_ERROR; //implement me
	return SpaceInfo();
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
		++it;
	}
}

void FlatVolumesFileSystem::CloseUnusedVolumes() const
{
	AutoLock lock(this->reading.nOpenVolumesLock);

	if(this->reading.nOpenVolumes > 100)
	{
		for(auto& volume : *this->reading.volumes)
		{
			AutoLock volumeLock(volume.mutex);
			if( (volume.counter == 0) and !volume.file.IsNull())
			{
				volume.file = nullptr;
				this->reading.nOpenVolumes--;
			}
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
		File dir(this->dirPath);
		dir.CreateDirectory();
		this->writing.createdDataDir = true;
	}

	OpenVolumeForWriting newVolume;
	newVolume.number = this->writing.nextVolumeNumber++;
	newVolume.file = new FileOutputStream(this->dirPath / String::Number(newVolume.number));
	newVolume.ownedWriter = writer;
	newVolume.leftSize = InjectionContainer::Instance().Config().volumeSize;

	leftSize = newVolume.leftSize;
	SeekableOutputStream& result = *newVolume.file;

	this->writing.openVolumes.InsertTail(StdXX::Move(newVolume));

	return result;
}

void FlatVolumesFileSystem::IncrementVolumeCounters(const DynamicArray<Block> &blocks) const
{
	for(const Block& b : blocks)
	{
		VolumeForReading& volume = this->reading.volumes->operator[](b.volumeNumber);

		AutoLock lock(volume.mutex);
		volume.counter++;
	}
}

SeekableInputStream &FlatVolumesFileSystem::LockVolumeStream(uint64 volumeNumber) const
{
	VolumeForReading &volume = (*this->reading.volumes)[volumeNumber];
	volume.mutex.Lock();

	if(volume.file.IsNull())
	{
		volume.file = new FileInputStream(this->dirPath / String::Number(volumeNumber));
		this->reading.nOpenVolumes++;
	}

	return *volume.file;
}

//TODO: NOT IMPLEMENTED
UniquePointer<DirectoryEnumerator> FlatVolumesFileSystem::EnumerateChildren(const Path &path) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return UniquePointer<DirectoryEnumerator>();
}

Optional<FileInfo> FlatVolumesFileSystem::QueryFileInfo(const Path &path) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return Optional<FileInfo>();
}

Optional<Path> FlatVolumesFileSystem::ReadLinkTarget(const Path &path) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return Optional<Path>();
}

Optional<Errors::CreateDirectoryError> FlatVolumesFileSystem::CreateDirectory(const Path &path, const Permissions *permissions)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
}

void FlatVolumesFileSystem::DeleteFile(const Path &path)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
}

UniquePointer<OutputStream> FlatVolumesFileSystem::OpenFileForWriting(const Path &path)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return UniquePointer<OutputStream>();
}

void FlatVolumesFileSystem::RemoveDirectory(const Path &path)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
}

void FlatVolumesFileSystem::ChangePermissions(const Path &path, const Permissions &newPermissions)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
}
//TODO: NOT IMPLEMENTED
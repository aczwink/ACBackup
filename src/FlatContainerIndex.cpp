/*
 * Copyright (c) 2019 Amir Czwink (amir130@hotmail.de)
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
#include "FlatContainerIndex.hpp"
//Local
#include "FlatContainerFileSystem.hpp"

//Constructor
FlatContainerIndex::FlatContainerIndex(const Path &prefixPath) : BackupContainerIndex(prefixPath)
{
	if(OSFileSystem::GetInstance().Exists(this->GetIndexPath()))
	{
		//open in read mode
		this->ReadIndexFile();
		this->reading.dataContainer = new FlatContainerFileSystem(*this, this->GetDataPath());
	}
	else
	{
		//open in write mode
		this->writing.dataFile = new FileOutputStream(this->GetDataPath());
	}
}

//Public methods
void FlatContainerIndex::AddPreviousFile(const Path &filePath, const FileIndex &index)
{
	const FileAttributes& fileAttributes = index.GetFileAttributes(index.FindFileIndex(filePath));

	//fill out entry
	FlatContainerFileAttributes backupEntry;

	MemCopy(backupEntry.digest, fileAttributes.digest, sizeof(fileAttributes.digest));
	backupEntry.size = fileAttributes.size;
	backupEntry.offset = Unsigned<uint64>::Max(); //special, means that we don't have it but another snapshot

	//add entry
	AutoLock lock(this->fileHeaderLock);
	uint32 attrIndex = this->fileAttributes.Push(Move(backupEntry));
	this->fileEntries.Insert(filePath, attrIndex);
}

float32 FlatContainerIndex::BackupFile(const Path& filePath, const FileIndex& index, float32 compressionRate, const int8 maxCompressionLevel, const uint64 memLimit)
{
	const FileAttributes& fileAttributes = index.GetFileAttributes(index.FindFileIndex(filePath));

	//fill out entry
	FlatContainerFileAttributes backupEntry;

	MemCopy(backupEntry.digest, fileAttributes.digest, sizeof(fileAttributes.digest));
	backupEntry.size = fileAttributes.size;

	//compute compression level
	float32 c = (maxCompressionLevel+1) * (1 - compressionRate);
	uint8 compressionLevel = static_cast<uint8>(round(c));
	backupEntry.isCompressed = !((maxCompressionLevel == -1) or (compressionLevel == 0));
	compressionLevel--;

	/*static Mutex m;
	m.Lock();
	stdOut << backupEntry.isCompressed << u8": " << compressionRate << u8" -> " << compressionLevel << endl;
	m.Unlock();*/

	bool filter = backupEntry.isCompressed; //TODO: or encrypt
	if((fileAttributes.size < memLimit) && filter)
	{
		FIFOBuffer buffer;
		buffer.EnsureCapacity(static_cast<uint32>(fileAttributes.size));

		//filter data first
		OutputStream* sink = &buffer;
		UniquePointer<Compressor> compressor;

		UniquePointer<InputStream> fileInputStream = index.OpenFileForReading(filePath);
		if(backupEntry.isCompressed)
		{
			compressor = Compressor::Create(CompressionAlgorithm::LZMA, *sink, compressionLevel);
			sink = compressor.operator->();
		}

		fileInputStream->FlushTo(*sink);
		sink->Flush();

		//now write
		AutoLock lock(this->writing.writeLock);

		backupEntry.offset = this->writing.dataFile->GetCurrentOffset();
		buffer.FlushTo(*this->writing.dataFile);
		backupEntry.blockSize = this->writing.dataFile->GetCurrentOffset() - backupEntry.offset;
	}
	else
	{
		AutoLock lock(this->writing.writeLock);

		backupEntry.offset = this->writing.dataFile->GetCurrentOffset();

		//stream data
		OutputStream* sink = this->writing.dataFile.operator->();
		UniquePointer<Compressor> compressor;

		UniquePointer<InputStream> fileInputStream = index.OpenFileForReading(filePath);
		if(backupEntry.isCompressed)
		{
			compressor = Compressor::Create(CompressionAlgorithm::LZMA, *this->writing.dataFile, compressionLevel);
			sink = compressor.operator->();
		}

		fileInputStream->FlushTo(*sink);
		sink->Flush();

		backupEntry.blockSize = this->writing.dataFile->GetCurrentOffset() - backupEntry.offset;
	}

	//add entry
	AutoLock lock(this->fileHeaderLock);
	uint32 attrIndex = this->fileAttributes.Push(Move(backupEntry));
	this->fileEntries.Insert(filePath, attrIndex);

	if(backupEntry.size == 0)
		return 1;
	return backupEntry.blockSize / float32(backupEntry.size);
}

uint32 FlatContainerIndex::FindFileIndex(const Path &path) const
{
	if(this->fileEntries.Contains(path))
		return this->fileEntries.Get(path);
	return Unsigned<uint32>::Max();
}

const Path &FlatContainerIndex::GetFile(uint32 index) const
{
	return this->fileEntries.GetReverse(index);
}

const FileAttributes &FlatContainerIndex::GetFileAttributes(uint32 index) const
{
	return this->fileAttributes[index];
}

uint32 FlatContainerIndex::GetNumberOfFiles() const
{
	return this->fileAttributes.GetNumberOfElements();
}

bool FlatContainerIndex::HasFileData(uint32 index) const
{
	return this->fileAttributes[index].offset != Unsigned<uint64>::Max();
}

UniquePointer<InputStream> FlatContainerIndex::OpenFileForReading(const Path& filePath) const
{
	AutoPointer<const File> file = this->reading.dataContainer->GetFile(filePath);
	return file->OpenForReading();
}

void FlatContainerIndex::Serialize() const
{
	FileOutputStream fileOutputStream(this->GetIndexPath(), true); //overwrite old

	//write header
	{
		DataWriter dataWriter(true, fileOutputStream);

		//header
		fileOutputStream.WriteBytes(u8"acbkpidx", 8);
		dataWriter.WriteUInt16(1); //version major
		dataWriter.WriteUInt16(0); //version minor
		fileOutputStream.WriteBytes(u8"flat", 4);
	}

	//file entries
	Crypto::HashingOutputStream hashingOutputStream(fileOutputStream, Crypto::HashAlgorithm::SHA256);
	BufferedOutputStream bufferedOutputStream(hashingOutputStream);
	DataWriter dataWriter(true, bufferedOutputStream);
	TextWriter textWriter(bufferedOutputStream, TextCodecType::UTF8);

	dataWriter.WriteUInt32(this->fileAttributes.GetNumberOfElements());
	for(const auto& fileEntry : this->fileEntries)
	{
		const FlatContainerFileAttributes& attributes = this->fileAttributes[fileEntry.value];

		textWriter.WriteStringZeroTerminated(fileEntry.key.GetString());
		dataWriter.WriteUInt64(attributes.size);
		bufferedOutputStream.WriteBytes(attributes.digest, sizeof(attributes.digest));
		dataWriter.WriteUInt64(attributes.offset);
		dataWriter.WriteUInt64(attributes.blockSize);
		dataWriter.WriteByte(static_cast<byte>(attributes.isCompressed));
	}
	bufferedOutputStream.Flush();

	UniquePointer<Crypto::HashFunction> hasher = hashingOutputStream.Reset();
	hasher->Finish();
	FixedArray<byte> digest = hasher->GetDigest();
	fileOutputStream.WriteBytes(&digest[0], digest.GetNumberOfElements());
}

//Class functions
Snapshot FlatContainerIndex::Deserialize(const Path &path)
{
	Path prefixPath = path.GetParent() / path.GetTitle();
	FlatContainerIndex* idx = new FlatContainerIndex(prefixPath);

	return { path.GetTitle(), idx };
}

//Private methods
void FlatContainerIndex::ReadIndexFile()
{
	FileInputStream indexFile(this->GetIndexPath());
	indexFile.Skip(16); //skip header

	//read file entries
	LimitedInputStream limitedInputStream(indexFile, indexFile.GetRemainingBytes() - 32); //read all but the sha256 digest at the end
	Crypto::HashingInputStream hashingInputStream(limitedInputStream, Crypto::HashAlgorithm::SHA256);
	BufferedInputStream bufferedInputStream(hashingInputStream);
	DataReader dataReader(true, bufferedInputStream);
	TextReader textReader(bufferedInputStream, TextCodecType::UTF8);

	uint32 nFiles = dataReader.ReadUInt32();
	for(uint32 i = 0; i < nFiles; i++)
	{
		Path filePath = textReader.ReadZeroTerminatedString();

		FlatContainerFileAttributes fileEntry;
		fileEntry.size = dataReader.ReadUInt64();
		bufferedInputStream.ReadBytes(fileEntry.digest, sizeof(fileEntry.digest));
		fileEntry.offset = dataReader.ReadUInt64();
		fileEntry.blockSize = dataReader.ReadUInt64();
		fileEntry.isCompressed = dataReader.ReadByte() != 0;

		uint32 attrIndex = this->fileAttributes.Push(Move(fileEntry));
		this->fileEntries.Insert(Move(filePath), attrIndex);
	}

	UniquePointer<Crypto::HashFunction> hasher = hashingInputStream.Reset();
	hasher->Finish();
	FixedArray<byte> digest = hasher->GetDigest();
	FixedArray<byte> readDigest(digest.GetNumberOfElements());

	uint32 bytesRead = indexFile.ReadBytes(&readDigest[0], digest.GetNumberOfElements());
	ASSERT(bytesRead == digest.GetNumberOfElements(), u8"INDEX FILE IS CORRUPT");
	ASSERT(digest == readDigest, u8"INDEX FILE IS CORRUPT");
}
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
FlatContainerIndex::FlatContainerIndex(const Path &prefixPath, const Optional<EncryptionInfo>& encryptionInfo) : BackupContainerIndex(prefixPath)
{
	this->isEncrypted = encryptionInfo.HasValue();
	if(this->isEncrypted)
	{
		this->encryptionKey.Resize(AES_KEY_SIZE);
		DeriveFileDataKey(*encryptionInfo, this->GetDataPath().GetName(), &this->encryptionKey[0]);
	}

	if(OSFileSystem::GetInstance().Exists(this->GetIndexPath()))
	{
		//open in read mode
		this->ReadIndexFile(encryptionInfo);
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

	//generate counter value
	UniquePointer<Crypto::Counter> counter;
	UniquePointer<Crypto::CTRCipher> cipher;
	if(this->isEncrypted)
	{
		if(fileAttributes.size >= Unsigned<uint32>::Max())
		{
			GenerateSafeRandomBytes(backupEntry.encCounterValue.big.nonce, sizeof(backupEntry.encCounterValue.big.nonce));
			GenerateSafeRandomBytes(reinterpret_cast<uint8 *>(&backupEntry.encCounterValue.big.initialValue), sizeof(backupEntry.encCounterValue.big.initialValue));

			counter = new Crypto::DefaultCounter<8, uint64>(backupEntry.encCounterValue.big.nonce, backupEntry.encCounterValue.big.initialValue);
		}
		else
		{
			GenerateSafeRandomBytes(backupEntry.encCounterValue.small.nonce, sizeof(backupEntry.encCounterValue.small.nonce));
			GenerateSafeRandomBytes(reinterpret_cast<uint8 *>(&backupEntry.encCounterValue.small.initialValue), sizeof(backupEntry.encCounterValue.small.initialValue));

			counter = new Crypto::DefaultCounter<12, uint32>(backupEntry.encCounterValue.small.nonce, backupEntry.encCounterValue.small.initialValue);
		}
	}

	//compute compression level
	float32 c = (maxCompressionLevel+1) * (1 - compressionRate);
	uint8 compressionLevel = static_cast<uint8>(round(c));
	backupEntry.isCompressed = !((maxCompressionLevel == -1) or (compressionLevel == 0));
	compressionLevel--;

	/*static Mutex m;
	m.Lock();
	stdOut << backupEntry.isCompressed << u8": " << compressionRate << u8" -> " << compressionLevel << endl;
	m.Unlock();*/

	bool filter = backupEntry.isCompressed or this->isEncrypted;
	if((fileAttributes.size < memLimit) && filter)
	{
		FIFOBuffer buffer;
		buffer.EnsureCapacity(static_cast<uint32>(fileAttributes.size));

		//filter data first
		OutputStream* sink = &buffer;

		if(this->isEncrypted)
		{
			cipher = new Crypto::CTRCipher(CipherAlgorithm::AES, &this->encryptionKey[0],
										   static_cast<uint16>(this->encryptionKey.GetNumberOfElements()), *counter, *sink);
			sink = cipher.operator->();
		}

		UniquePointer<Compressor> compressor;
		if(backupEntry.isCompressed)
		{
			compressor = Compressor::Create(CompressionAlgorithm::LZMA, *sink, compressionLevel);
			sink = compressor.operator->();
		}

		UniquePointer<InputStream> fileInputStream = index.OpenFileForReading(filePath);
		fileInputStream->FlushTo(*sink);
		sink->Flush();
		if(backupEntry.isCompressed)
			compressor->Finalize();

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
		BufferedOutputStream bufferedOutputStream(*this->writing.dataFile.operator->());
		OutputStream* sink = &bufferedOutputStream;

		if(this->isEncrypted)
		{
			cipher = new Crypto::CTRCipher(CipherAlgorithm::AES, &this->encryptionKey[0],
										   static_cast<uint16>(this->encryptionKey.GetNumberOfElements()), *counter, *sink);
			sink = cipher.operator->();
		}

		UniquePointer<Compressor> compressor;
		if(backupEntry.isCompressed)
		{
			compressor = Compressor::Create(CompressionAlgorithm::LZMA, *sink, compressionLevel);
			sink = compressor.operator->();
		}

		UniquePointer<InputStream> fileInputStream = index.OpenFileForReading(filePath);
		fileInputStream->FlushTo(*sink);
		sink->Flush();
		if(backupEntry.isCompressed)
			compressor->Finalize();

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

	UniquePointer<InputStream> stream = file->OpenForReading();
	ChainedInputStream* chain = new ChainedInputStream(Move(stream));

	chain->Add( new BufferedInputStream(chain->GetEnd()) );

	uint32 index = this->fileEntries.Get(filePath);
	const FlatContainerFileAttributes& backupEntry = this->fileAttributes[index];

	if(this->isEncrypted)
	{
		UniquePointer<Crypto::Counter> counter;
		if(backupEntry.size >= Unsigned<uint32>::Max())
			counter = new Crypto::DefaultCounter<8, uint64>(backupEntry.encCounterValue.big.nonce, backupEntry.encCounterValue.big.initialValue);
		else
			counter = new Crypto::DefaultCounter<12, uint32>(backupEntry.encCounterValue.small.nonce, backupEntry.encCounterValue.small.initialValue);

		Crypto::Counter* ownedCtr = chain->AddCounter(Move(counter));
		chain->Add( new Crypto::CTRDecipher(CipherAlgorithm::AES, &this->encryptionKey[0], static_cast<uint16>(this->encryptionKey.GetNumberOfElements()), *ownedCtr, chain->GetEnd()) );
	}

	if(backupEntry.isCompressed)
	{
		chain->Add( Decompressor::Create(CompressionAlgorithm::LZMA, chain->GetEnd()) );
	}

	return chain;
}

void FlatContainerIndex::Serialize(const Optional<EncryptionInfo>& encryptionInfo) const
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
	OutputStream* target = &fileOutputStream;
	UniquePointer<Crypto::CBCCipher> cipher;
	if(encryptionInfo.HasValue())
	{
		uint8 key[AES_KEY_SIZE];
		uint8 iv[AES_BLOCK_SIZE];

		DeriveFileKey(*encryptionInfo, this->GetIndexPath().GetName(), key, iv);
		cipher = new Crypto::CBCCipher(CipherAlgorithm::AES, key, AES_KEY_SIZE, iv, *target);
		target = cipher.operator->();
	}
	Crypto::HashingOutputStream hashingOutputStream(*target, Crypto::HashAlgorithm::SHA256);
	BufferedOutputStream bufferedOutputStream(hashingOutputStream);

	DataWriter dataWriter(true, bufferedOutputStream);
	TextWriter textWriter(bufferedOutputStream, TextCodecType::UTF8);

	dataWriter.WriteUInt32(this->fileAttributes.GetNumberOfElements());
	for(const auto& fileEntry : this->fileEntries)
	{
		const FlatContainerFileAttributes& attributes = this->fileAttributes[fileEntry.value];

		textWriter.WriteStringZeroTerminated(fileEntry.key.GetString());
		dataWriter.WriteUInt64(attributes.size);
		dataWriter.WriteBytes(attributes.digest, sizeof(attributes.digest));
		dataWriter.WriteUInt64(attributes.offset);
		dataWriter.WriteUInt64(attributes.blockSize);
		dataWriter.WriteByte(static_cast<byte>(attributes.isCompressed));
		if(this->isEncrypted)
		{
			if(attributes.size >= Unsigned<uint32>::Max())
			{
				dataWriter.WriteBytes(attributes.encCounterValue.big.nonce, sizeof(attributes.encCounterValue.big.nonce));
				dataWriter.WriteUInt64(attributes.encCounterValue.big.initialValue);
			}
			else
			{
				dataWriter.WriteBytes(attributes.encCounterValue.small.nonce, sizeof(attributes.encCounterValue.small.nonce));
				dataWriter.WriteUInt32(attributes.encCounterValue.small.initialValue);
			}
		}
	}
	bufferedOutputStream.Flush();
	if(!cipher.IsNull())
		cipher->Finalize();

	UniquePointer<Crypto::HashFunction> hasher = hashingOutputStream.Reset();
	hasher->Finish();
	FixedArray<byte> digest = hasher->GetDigest();
	fileOutputStream.WriteBytes(&digest[0], digest.GetNumberOfElements());
}

//Class functions
Snapshot FlatContainerIndex::Deserialize(const Path &path, const Optional<EncryptionInfo>& encryptionInfo)
{
	Path prefixPath = path.GetParent() / path.GetTitle();
	FlatContainerIndex* idx = new FlatContainerIndex(prefixPath, encryptionInfo);

	return { path.GetTitle(), idx };
}

//Private methods
void FlatContainerIndex::ReadIndexFile(const Optional<EncryptionInfo>& encryptionInfo)
{
	FileInputStream indexFile(this->GetIndexPath());
	indexFile.Skip(16); //skip header

	//read file entries
	LimitedInputStream limitedInputStream(indexFile, indexFile.GetRemainingBytes() - 32); //read all but the sha256 digest at the end

	InputStream* source = &limitedInputStream;
	UniquePointer<Crypto::CBCDecipher> decipher;
	if(encryptionInfo.HasValue())
	{
		uint8 key[AES_KEY_SIZE];
		uint8 iv[AES_BLOCK_SIZE];

		DeriveFileKey(*encryptionInfo, this->GetIndexPath().GetName(), key, iv);
		decipher = new Crypto::CBCDecipher(CipherAlgorithm::AES, key, AES_KEY_SIZE, iv, *source);
		source = decipher.operator->();
	}

	Crypto::HashingInputStream hashingInputStream(*source, Crypto::HashAlgorithm::SHA256);
	BufferedInputStream bufferedInputStream(hashingInputStream);

	DataReader dataReader(true, bufferedInputStream);
	TextReader textReader(bufferedInputStream, TextCodecType::UTF8);

	uint32 nFiles = dataReader.ReadUInt32();
	for(uint32 i = 0; i < nFiles; i++)
	{
		Path filePath = textReader.ReadZeroTerminatedString();

		FlatContainerFileAttributes fileEntry;
		fileEntry.size = dataReader.ReadUInt64();
		dataReader.ReadBytes(fileEntry.digest, sizeof(fileEntry.digest));
		fileEntry.offset = dataReader.ReadUInt64();
		fileEntry.blockSize = dataReader.ReadUInt64();
		fileEntry.isCompressed = dataReader.ReadByte() != 0;
		if(this->isEncrypted)
		{
			if(fileEntry.size >= Unsigned<uint32>::Max())
			{
				dataReader.ReadBytes(fileEntry.encCounterValue.big.nonce, sizeof(fileEntry.encCounterValue.big.nonce));
				fileEntry.encCounterValue.big.initialValue = dataReader.ReadUInt64();
			}
			else
			{
				dataReader.ReadBytes(fileEntry.encCounterValue.small.nonce, sizeof(fileEntry.encCounterValue.small.nonce));
				fileEntry.encCounterValue.small.initialValue = dataReader.ReadUInt32();
			}
		}

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
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
#include "../encryption/FlatContainerIndex.hpp"
//Local
#include "../FlatContainerFileSystem.hpp"

//Constructor
FlatContainerIndex::FlatContainerIndex(const Path &prefixPath, const Optional<EncryptionInfo>& encryptionInfo) : BackupContainerIndex(prefixPath)
{
	this->isEncrypted = encryptionInfo.HasValue();
	if(this->isEncrypted)
	{
		this->encryptionKey.Resize(AES_KEY_SIZE);
		DeriveFileDataKey(*encryptionInfo, this->GetDataPath().GetName(), &this->encryptionKey[0]);
	}
}

//Public methods
float32 FlatContainerIndex::BackupFile(const Path& filePath, const FileSystemNodeIndex& index, const uint64 memLimit)
{
	/*
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

	if((fileAttributes.size < memLimit) && filter)
	{
		if(this->isEncrypted)
		{
			cipher = new Crypto::CTRCipher(CipherAlgorithm::AES, &this->encryptionKey[0],
										   static_cast<uint16>(this->encryptionKey.GetNumberOfElements()), *counter, *sink);
			sink = cipher.operator->();
		}
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

		backupEntry.blockSize = this->writing.dataFile->GetCurrentOffset() - backupEntry.offset;
	}
	*/
}

UniquePointer<InputStream> FlatContainerIndex::OpenFileForReading(const Path& filePath) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: REIMPLEMENT ME
	return nullptr;
	/*
	AutoPointer<const File> file = this->reading.dataContainer->GetFile(filePath);

	uint32 index = this->fileEntries.Get(filePath);
	const FlatContainerFileAttributes& backupEntry = *this->fileAttributes[index];

	if(this->isEncrypted)
	{
		UniquePointer<Crypto::Counter> counter;
		if(backupEntry.Size() >= Unsigned<uint32>::Max())
			counter = new Crypto::DefaultCounter<8, uint64>(backupEntry.encCounterValue.big.nonce, backupEntry.encCounterValue.big.initialValue);
		else
			counter = new Crypto::DefaultCounter<12, uint32>(backupEntry.encCounterValue.small.nonce, backupEntry.encCounterValue.small.initialValue);

		Crypto::Counter* ownedCtr = chain->AddCounter(Move(counter));
		chain->Add( new Crypto::CTRDecipher(CipherAlgorithm::AES, &this->encryptionKey[0], static_cast<uint16>(this->encryptionKey.GetNumberOfElements()), *ownedCtr, chain->GetEnd()) );
	}

	if(backupEntry.isCompressed)
	{
		chain->Add( Decompressor::Create(CompressionAlgorithm::LZMA, chain->GetEnd(), false) );
	}

	return chain;*/
}

void FlatContainerIndex::Serialize(const Optional<EncryptionInfo>& encryptionInfo) const
{
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

void FlatContainerIndex::ReadIndexFile(const Optional<EncryptionInfo>& encryptionInfo)
{
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

	UniquePointer<Crypto::HashFunction> hasher = hashingInputStream.Reset();
	hasher->Finish();
	FixedArray<byte> digest = hasher->GetDigest();
	FixedArray<byte> readDigest(digest.GetNumberOfElements());

	uint32 bytesRead = indexFile.ReadBytes(&readDigest[0], digest.GetNumberOfElements());
	ASSERT(bytesRead == digest.GetNumberOfElements(), u8"INDEX FILE IS CORRUPT");
	ASSERT(digest == readDigest, u8"INDEX FILE IS CORRUPT");
}
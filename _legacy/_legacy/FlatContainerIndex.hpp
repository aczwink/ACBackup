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
#pragma once
//Local
#include "../BackupContainerIndex.hpp"
#include "BackupNodeAttributes.hpp"

struct FlatContainerFileAttributes : public FileSystemNodeAttributes
{
	union
	{
		struct
		{
			uint8 nonce[12];
			uint32 initialValue;
		} small;
		struct
		{
			uint8 nonce[8];
			uint64 initialValue;
		} big;
	} encCounterValue;
};

class FlatContainerIndex : public BackupContainerIndex
{
public:
	//Constructor
	FlatContainerIndex(const Path& prefixPath, const Optional<EncryptionInfo>& encryptionInfo);

	//Methods
	void AddPreviousFile(const Path &filePath, const FileSystemNodeIndex &index) override;
	float32 BackupFile(const Path& filePath, const FileSystemNodeIndex& index, float32 compressionRate, int8 maxCompressionLevel, uint64 memLimit) override;
	uint32 FindNodeIndex(const Path &path) const override;
	const Path &GetNodePath(uint32 index) const override;
	const FileSystemNodeAttributes &GetNodeAttributes(uint32 index) const override;
	uint32 GetNumberOfNodes() const override;
	bool HasFileData(uint32 index) const override;
	UniquePointer<InputStream> OpenFileForReading(const Path &fileEntry) const override;
	void Serialize(const Optional<EncryptionInfo>& encryptionInfo) const override;

	//Functions
	//static Snapshot Deserialize(const Path& path, const Optional<EncryptionInfo>& encryptionInfo);

	//Inline
	inline const BackupNodeAttributes& GetSpecificNodeAttributes(uint32 index) const
	{
		return *this->nodeAttributes[index];
	}

private:
	//Members
	bool isEncrypted;
	DynamicArray<uint8> encryptionKey;
	Mutex fileHeaderLock;
	BijectiveMap<Path, uint32> fileEntries;
	DynamicArray<UniquePointer<BackupNodeAttributes>> nodeAttributes;
	struct
	{
		UniquePointer<ContainerFileSystem> dataContainer;
	} reading;
	struct
	{
		Mutex writeLock;
		UniquePointer<FileOutputStream> dataFile;
	} writing;

	//Methods
	void BackupFileInMemory(const Path& filePath, BackupNodeAttributes& backupEntry, const FileSystemNodeIndex& index);
	void ReadIndexFile(const Optional<EncryptionInfo>& encryptionInfo);

	//Inline
	inline Path GetDataPath() const
	{
		return this->GetPathPrefix().GetString() + String(u8".data");
	}

	inline Path GetIndexPath() const
	{
		return this->GetPathPrefix().GetString() + String(u8".index");
	}
};
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
#include "BackupContainerIndex.hpp"

struct FlatContainerFileAttributes : public FileAttributes
{
	uint64 offset;
	uint64 blockSize; //size of data after compression / encryption
};

class FlatContainerIndex : public BackupContainerIndex
{
public:
	//Constructor
	FlatContainerIndex(const Path& prefixPath);

	//Methods
	void AddPreviousFile(const Path &filePath, const FileIndex &index) override;
	float32 BackupFile(const Path& filePath, const FileIndex& index, float32 compressionRate, const uint64 memLimit) override;
	uint32 FindFileIndex(const Path &path) const override;
	const Path &GetFile(uint32 index) const override;
	const FileAttributes &GetFileAttributes(uint32 index) const override;
	uint32 GetNumberOfFiles() const override;
	bool HasFileData(uint32 index) const override;
	UniquePointer<InputStream> OpenFileForReading(const Path &fileEntry) const override;
	void Serialize() const override;

	//Functions
	static Snapshot Deserialize(const Path& path);

	//Inline
	inline const FlatContainerFileAttributes& GetSpecificFileAttributes(uint32 index) const
	{
		return this->fileAttributes[index];
	}

private:
	//Members
	Mutex fileHeaderLock;
	BijectiveMap<Path, uint32> fileEntries;
	DynamicArray<FlatContainerFileAttributes> fileAttributes;
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
	void ReadIndexFile();

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
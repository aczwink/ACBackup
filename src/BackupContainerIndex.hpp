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
#include "FileIndex.hpp"
#include "KeyDerivation.hpp"

//Forward declarations
class BackupContainerIndex;

struct Snapshot
{
	String creationText;
	BackupContainerIndex* index;
	Snapshot* prev;

	/**
	 * Find the index that has the payload data of the file identified by index of this snapshot.
	 * @param fileIndex
	 * @return
	 */
	inline BackupContainerIndex* FindDataIndex(uint32 fileIndex) const;
};

class BackupContainerIndex : public FileIndex
{
public:
	//Constructor
	inline BackupContainerIndex(const Path& prefixPath) : prefixPath(prefixPath)
	{
	}

	//Abstract
	virtual void AddPreviousFile(const Path& filePath, const FileIndex& index) = 0;
	virtual float32 BackupFile(const Path& filePath, const FileIndex& index, float32 compressionRate, int8 maxCompressionLevel, uint64 memLimit) = 0;
	virtual bool HasFileData(uint32 index) const = 0;
	virtual void Serialize(const Optional<EncryptionInfo>& encryptionInfo) const = 0;

	//Functions
	static Snapshot Deserialize(const Path &path, const Optional<EncryptionInfo>& encryptionInfo);

	//Inline
	/**
	 * No compression or filtering whatsoever. The size of the files as they were originally.
	 * @return
	 */
	inline uint64 ComputeTotalFileDataSize() const
	{
		uint64 totalSize = 0;
		for(uint32 i = 0; i < this->GetNumberOfFiles(); i++)
			totalSize += this->GetFileAttributes(i).size;
		return totalSize;
	}

protected:
	//Inline
	inline const Path& GetPathPrefix() const
	{
		return this->prefixPath;
	}

private:
	//Members
	Path prefixPath;
};

inline BackupContainerIndex* Snapshot::FindDataIndex(uint32 fileIndex) const
{
	if(!this->index->HasFileData(fileIndex))
	{
		const Path& filePath = this->index->GetFile(fileIndex);
		return this->prev->FindDataIndex(this->prev->index->FindFileIndex(filePath));
	}

	return this->index;
}
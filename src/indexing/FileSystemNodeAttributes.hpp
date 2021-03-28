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
#pragma once
#include <StdXX.hpp>
using namespace StdXX;
using namespace StdXX::FileSystem;
//Local
#include "../Util.hpp"

class FileSystemNodeAttributes
{
public:
	//Constructors
	inline FileSystemNodeAttributes(FileType type, uint64 size, const Optional<DateTime>& lastModifiedTime, UniquePointer<Permissions>&& permissions)
	{
		this->type = type;
		this->size = size;
		this->lastModifiedTime = lastModifiedTime;
        this->permissions = Move(permissions);
	}

	inline FileSystemNodeAttributes(const FileInfo& fileInfo)
	{
		this->type = fileInfo.type;
		if(fileInfo.lastModifiedTime.HasValue())
			this->lastModifiedTime = fileInfo.lastModifiedTime;
		this->permissions = fileInfo.permissions->Clone();

		if(fileInfo.type == FileType::Directory)
			this->size = 0;
		else
			this->size = fileInfo.size;
	}

	inline FileSystemNodeAttributes(const FileSystemNodeAttributes& attributes)
    {
	    *this = attributes;
    }

	//Destructor
	virtual ~FileSystemNodeAttributes() = default;

	//Properties
	inline const Optional<DateTime>& LastModifiedTime() const
	{
		return this->lastModifiedTime;
	}

	inline const FileSystem::Permissions& Permissions() const
    {
	    return *this->permissions;
    }

	inline uint64 Size() const
	{
		return this->size;
	}

	inline FileType Type() const
	{
		return this->type;
	}

	//Operators
	FileSystemNodeAttributes& operator=(const FileSystemNodeAttributes& attributes)
    {
        this->type = attributes.type;
        this->size = attributes.size;
        this->permissions = attributes.permissions->Clone();
        this->lastModifiedTime = attributes.lastModifiedTime;

        return *this;
    }

	//Inline operators
	inline bool operator==(const FileSystemNodeAttributes& other) const
	{
		return (this->type == other.type) && (this->size == other.size) && (this->lastModifiedTime == other.lastModifiedTime);
	}

	inline bool operator!=(const FileSystemNodeAttributes& other) const
	{
		return !(*this == other);
	}

	//Inline
	inline void CopyFrom(const FileSystemNodeAttributes& other)
	{
		this->type = other.type;
		this->size = other.size;
		this->lastModifiedTime = other.lastModifiedTime;
	}

private:
	//Members
	FileType type;
	uint64 size;
	UniquePointer<FileSystem::Permissions> permissions;
	Optional<DateTime> lastModifiedTime;
};
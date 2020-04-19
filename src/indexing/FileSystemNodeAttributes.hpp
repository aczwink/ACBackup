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
#pragma once
#include <Std++.hpp>
using namespace StdXX;

class FileSystemNodeAttributes
{
public:
	//Constructors
	inline FileSystemNodeAttributes(FileSystemNodeType type, uint64 size, const Optional<DateTime>& lastModifiedTime)
	{
		this->type = type;
		this->size = size;
		this->lastModifiedTime = lastModifiedTime;
	}

	inline FileSystemNodeAttributes(const AutoPointer<const Directory>& directory)
	{
		this->type = FileSystemNodeType::Directory;
		this->Init(directory);
		this->size = 0;
	}

	inline FileSystemNodeAttributes(const AutoPointer<const File>& file)
	{
		this->type = FileSystemNodeType::File;
		this->Init(file);
		this->size = file->GetSize();
	}

	inline FileSystemNodeAttributes(const AutoPointer<const Link>& link)
	{
		this->type = FileSystemNodeType::Link;
		this->Init(link);
		this->size = link->QueryInfo().storedSize;
	}

	FileSystemNodeAttributes(const FileSystemNodeAttributes& attributes) = default; //copy ctor

	//Destructor
	virtual ~FileSystemNodeAttributes() = default;

	//Properties
	inline const Optional<DateTime>& LastModifiedTime() const
	{
		return this->lastModifiedTime;
	}

	inline uint64 Size() const
	{
		return this->size;
	}

	inline FileSystemNodeType Type() const
	{
		return this->type;
	}

	//Operators
	FileSystemNodeAttributes& operator=(const FileSystemNodeAttributes&) = default;

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
	FileSystemNodeType type;
	uint64 size;
	Optional<DateTime> lastModifiedTime;

	//Inline
	inline void Init(const AutoPointer<const FileSystemNode>& node)
	{
		const FileSystemNodeInfo info = node->QueryInfo();
		if(info.lastModifiedTime.HasValue())
			this->lastModifiedTime = info.lastModifiedTime;
	}
};
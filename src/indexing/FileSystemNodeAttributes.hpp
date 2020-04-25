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
#include <StdXX.hpp>
using namespace StdXX;
using namespace StdXX::FileSystem;

class FileSystemNodeAttributes
{
public:
	//Constructors
	inline FileSystemNodeAttributes(NodeType type, uint64 size, const Optional<DateTime>& lastModifiedTime)
	{
		this->type = type;
		this->size = size;
		this->lastModifiedTime = lastModifiedTime;
	}

	inline FileSystemNodeAttributes(const AutoPointer<const Directory>& directory)
	{
		this->type = NodeType::Directory;
		this->Init(directory);
		this->size = 0;
	}

	inline FileSystemNodeAttributes(const AutoPointer<const File>& file)
	{
		this->type = NodeType::File;
		this->Init(file);
	}

	inline FileSystemNodeAttributes(const AutoPointer<const Link>& link)
	{
		this->type = NodeType::Link;
		this->Init(link);
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

	inline NodeType Type() const
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
	NodeType type;
	uint64 size;
	Optional<DateTime> lastModifiedTime;

	//Inline
	inline void Init(const AutoPointer<const Node>& node)
	{
		const NodeInfo info = node->QueryInfo();

		this->size = info.size;
		if(info.lastModifiedTime.HasValue())
			this->lastModifiedTime = info.lastModifiedTime;
	}
};
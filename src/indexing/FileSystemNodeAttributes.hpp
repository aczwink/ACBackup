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
#include <Std++.hpp>
using namespace StdXX;
//Local
#include "../Config.hpp"

enum class IndexableNodeType
{
	File,
	Link
};

class FileSystemNodeAttributes
{
public:
	//Constructors
	inline FileSystemNodeAttributes(const AutoPointer<const File>& file, const Config& config) : config(config)
	{
		this->type = IndexableNodeType::File;
		this->Init(file);
		this->size = file->GetSize();
	}

	inline FileSystemNodeAttributes(const AutoPointer<const Link>& link, const Path& target, const Config& config) : config(config)
	{
		this->type = IndexableNodeType::Link;
		this->target = target;
		this->Init(link);
	}

	//Properties
	inline uint64 Size() const
	{
		ASSERT(this->type == IndexableNodeType::File, u8"Only files have a size!");
		return this->size;
	}

	inline IndexableNodeType Type() const
	{
		return this->type;
	}

protected:
	//Members
	const Config& config;

private:
	//Members
	IndexableNodeType type;
	Optional<DateTime> lastModifiedTime;
	uint64 size; //only valid for files
	Path target; //only valid for links

	//Inline
	inline void Init(const AutoPointer<const FileSystemNode>& node)
	{
		FileSystemNodeInfo info = node->QueryInfo();
		if(info.lastModifiedTime.HasValue())
			this->lastModifiedTime = info.lastModifiedTime->dt;
	}
};
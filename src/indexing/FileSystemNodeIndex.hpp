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
//Local
#include "FileSystemNodeAttributes.hpp"

class FileSystemNodeIndex
{
public:
	//Destructor
	virtual ~FileSystemNodeIndex() {}

	//Methods
	/**
	 * No compression or filtering whatsoever.
	 * The raw user data file size.
	 * @return
	 */
	uint64 ComputeTotalSize() const;
	uint64 ComputeTotalSize(const BinaryTreeSet<uint32>& nodeIndices) const;

	//Properties
	inline const BijectiveMap<Path, uint32> PathMap() const
    {
	    return this->pathMap;
    }

	//Inline
	inline uint32 AddNode(const Path& path, UniquePointer<FileSystemNodeAttributes>&& attributes)
	{
	    AutoLock lock(this->lock);

		uint32 index = this->nodeAttributes.Push(Move(attributes));
		this->pathMap.Insert(path.IsAbsolute() ? path : u8"/" + path.String(), index);
		return index;
	}

	inline const FileSystemNodeAttributes& GetNodeAttributes(uint32 index) const
	{
        AutoLock lock(this->lock);

		return *this->nodeAttributes[index];
	}

	inline uint32 GetNodeIndex(const Path& path) const
	{
        AutoLock lock(this->lock);
		return this->pathMap.Get(path);
	}

	inline const Path& GetNodePath(uint32 index) const
	{
		return this->pathMap.GetReverse(index);
	}

	inline uint32 GetNumberOfNodes() const
	{
		return this->nodeAttributes.GetNumberOfElements();
	}

	inline bool HasNodeIndex(const Path& path) const
	{
		return this->pathMap.Contains(path);
	}

protected:
    //Members
    mutable Mutex lock;

private:
	//Members
	DynamicArray<UniquePointer<FileSystemNodeAttributes>> nodeAttributes;
	BijectiveMap<Path, uint32> pathMap;
};
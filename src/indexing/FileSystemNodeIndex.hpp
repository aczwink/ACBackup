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
//Local
#include "FileSystemNodeAttributes.hpp"

enum class NodeDifferenceType
{
	BackupFully,
	BackreferenceWithDifferentPath,
	Delete,
	UpdateMetadataOnly,
};

struct NodeDifference
{
	NodeDifferenceType type;
	uint32 moveIndex;
};

class FileSystemNodeIndex
{
public:
	//Destructor
	virtual ~FileSystemNodeIndex() {}

	//Methods
	/**
	 * Returns the indices from this index that are different from other
	 * @return
	 */
	BinaryTreeSet<uint32> ComputeDifference(const FileSystemNodeIndex& other) const;
	/**
	 * No compression or filtering whatsoever.
	 * The raw user data file size.
	 * @return
	 */
	uint64 ComputeTotalSize() const;
	uint64 ComputeTotalSize(const BinaryTreeSet<uint32>& nodeIndices) const;
	uint64 ComputeTotalSize(const Map<uint32, NodeDifference>& nodeIndices) const;

	//Inline
	inline uint32 AddNode(const Path& path, UniquePointer<FileSystemNodeAttributes>&& attributes)
	{
		uint32 index = this->nodeAttributes.Push(Move(attributes));
		this->pathMap.Insert(path.IsAbsolute() ? path : u8"/" + path.GetString(), index);
		return index;
	}

	inline const FileSystemNodeAttributes& GetNodeAttributes(uint32 index) const
	{
		return *this->nodeAttributes[index];
	}

	inline uint32 GetNodeIndex(const Path& path) const
	{
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
	DynamicArray<UniquePointer<FileSystemNodeAttributes>> nodeAttributes;

private:
	//Members
	BijectiveMap<Path, uint32> pathMap;
};
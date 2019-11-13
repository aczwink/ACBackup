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
#include "FileSystemNodeAttributes.hpp"

class FileSystemNodeIndex
{
public:
	//Inline
	inline void AddNode(const Path& path, UniquePointer<FileSystemNodeAttributes>&& attributes)
	{
		uint32 index = this->nodeAttributes.Push(Move(attributes));
		this->pathMap.Insert(path, index);
	}

	inline uint64 ComputeTotalSize(const BinaryTreeSet<uint32>& nodeIndices) const
	{
		uint64 totalSize = 0;
		for(uint32 idx : nodeIndices)
		{
			const FileSystemNodeAttributes& fileAttributes = this->GetNodeAttributes(idx);
			if(fileAttributes.Type() == IndexableNodeType::File)
				totalSize += fileAttributes.Size();
		}

		return totalSize;
	}

	inline const FileSystemNodeAttributes& GetNodeAttributes(uint32 index) const
	{
		return *this->nodeAttributes[index];
	}

	inline const Path& GetNodePath(uint32 index) const
	{
		return this->pathMap.GetReverse(index);
	}

	inline uint32 GetNumberOfNodes() const
	{
		return this->nodeAttributes.GetNumberOfElements();
	}

private:
	//Members
	DynamicArray<UniquePointer<FileSystemNodeAttributes>> nodeAttributes;
	BijectiveMap<Path, uint32> pathMap;
};
/*
 * Copyright (c) 2020-2021 Amir Czwink (amir130@hotmail.de)
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
#include "indexing/OSFileSystemNodeIndex.hpp"
#include "backup/BackupNodeIndex.hpp"

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

struct NodeIndexDifferences
{
	//referring to the left index
	BinaryTreeSet<uint32> deleted;
	//referring to the right index
	BinaryTreeSet<uint32> differentData; //implies also that metadata is different
	BinaryTreeSet<uint32> differentMetadata;
	BinaryTreeMap<uint32, uint32> moved; //maps right index to left index

	//Inline
	inline bool Exist() const
    {
	    return !(deleted.IsEmpty() and differentData.IsEmpty() and differentMetadata.IsEmpty() and moved.IsEmpty());
    }
};

class NodeIndexDifferenceResolver
{
public:
	//Methods
	NodeIndexDifferences ComputeDiff(const BackupNodeIndex& leftIndex, const FileSystemNodeIndex& rightIndex);

private:
	//Methods
	BinaryTreeSet<uint32> ComputeDeleted(const FileSystemNodeIndex& leftIndex, const FileSystemNodeIndex& rightIndex, const BinaryTreeSet<uint32>& indexes) const;
	/**
	 * Returns the indices from this index that are different from other
	 * @return
	 */
	BinaryTreeSet<uint32> ComputeDifference(const FileSystemNodeIndex& leftIndex, const FileSystemNodeIndex& rightIndex) const;
	void ComputeNodeDifferences(NodeIndexDifferences& nodeIndexDifferences, const BackupNodeIndex& leftIndex, const FileSystemNodeIndex& rightIndex, const BinaryTreeSet<uint32>& rightToLeftDiffs) const;
	NodeIndexDifferences ResolveDifferences(const BackupNodeIndex& leftIndex, const FileSystemNodeIndex& rightIndex, const BinaryTreeSet<uint32>& leftToRightDiffs, const BinaryTreeSet<uint32>& rightToLeftDiffs) const;
	String RetrieveNodeHash(uint32 nodeIndex, const FileSystemNodeIndex& index) const;
};
/*
 * Copyright (c) 2021 Amir Czwink (amir130@hotmail.de)
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
#include "../src/backup/SnapshotManager.hpp"

class TreeNode
{
public:
	//Destructor
	virtual ~TreeNode() = default;

	//Abstract
	virtual void DownloadTo(const FileSystem::Path& dirPath) const = 0;
	virtual const DynamicArray<UniquePointer<TreeNode>>& GetChildren() = 0;
	virtual String GetName() const = 0;
};

class DirectoryTreeNode : public TreeNode
{
public:
	//Constructor
	DirectoryTreeNode(const FileSystem::Path& path, const Snapshot& snapshot) : path(path), snapshot(snapshot)
	{
		this->nodeIndex = snapshot.Index().GetNodeIndex(path);
		this->childrenCached = false;
	}

	//Methods
	void DownloadTo(const Path &dirPath) const override;
	const DynamicArray<UniquePointer<TreeNode>> &GetChildren() override
	{
		if(!this->childrenCached)
			this->LoadChildren();
		return this->children;
	}

	String GetName() const override
	{
		return this->path.GetName();
	}

private:
	//Members
	uint32 nodeIndex;
	FileSystem::Path path;
	const Snapshot& snapshot;
	bool childrenCached;
	DynamicArray<UniquePointer<TreeNode>> children;

	//Methods
	void LoadChildren();
};
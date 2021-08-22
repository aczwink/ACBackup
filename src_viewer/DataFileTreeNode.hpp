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
//Local
#include "Nodes.hpp"

class DataFileTreeNode : public TreeNode
{
public:
	inline DataFileTreeNode(const FileSystem::Path& path, const Snapshot& snapshot) : path(path), snapshot(snapshot)
	{
		this->revisionsLoaded = false;
		this->nodeIndex = snapshot.Index().GetNodeIndex(path);
	}

	//Methods
	void DownloadTo(const Path &dirPath) const override;
	const DynamicArray<UniquePointer<TreeNode>> &GetChildren() override;
	String GetName() const override;

private:
	//Members
	uint32 nodeIndex;
	FileSystem::Path path;
	const Snapshot& snapshot;
	bool revisionsLoaded;
	DynamicArray<UniquePointer<TreeNode>> revisions;

	//Methods
	void LoadRevisions();
};
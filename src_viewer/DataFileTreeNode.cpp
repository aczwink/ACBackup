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
//Class header
#include "DataFileTreeNode.hpp"
//Local
#include "FileRevisionNode.hpp"

//Public methods
void DataFileTreeNode::DownloadTo(const Path &dirPath) const
{
	FileOutputStream fileOutputStream(dirPath / this->path.GetName());
	auto input = this->snapshot.Filesystem().OpenFileForReading(this->nodeIndex, true);
	input->FlushTo(fileOutputStream);
}

const DynamicArray<UniquePointer<TreeNode>> &DataFileTreeNode::GetChildren()
{
	if(!this->revisionsLoaded)
		this->LoadRevisions();

	return this->revisions;
}

String DataFileTreeNode::GetName() const
{
	return this->path.GetName();
}

//Private methods
void DataFileTreeNode::LoadRevisions()
{
	const Snapshot* currentSnapshot = &this->snapshot;
	FileSystem::Path nodePath = this->path;

	while(currentSnapshot and currentSnapshot->Index().HasNodeIndex(nodePath))
	{
		uint32 nodeIndex = currentSnapshot->Index().GetNodeIndex(nodePath);
		currentSnapshot = currentSnapshot->FindDataSnapshot(nodeIndex, nodePath);
		if(currentSnapshot)
		{
			this->revisions.Push(new FileRevisionNode(nodeIndex, *currentSnapshot));
			currentSnapshot = currentSnapshot->Previous();
		}
	}

	this->revisionsLoaded = true;
}
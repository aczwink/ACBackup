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
#include "Nodes.hpp"
//Local
#include "DataFileTreeNode.hpp"

//Private methods
void DirectoryTreeNode::LoadChildren()
{
	auto& index = this->snapshot.Index();
	for(uint32 i = 0; i < index.GetNumberOfNodes(); i++)
	{
		if(i == this->nodeIndex)
			continue;

		const auto& nodePath = index.GetNodePath(i);
		if(nodePath.GetParent() == this->path)
		{
			if(index.GetNodeAttributes(i).Type() == FileSystem::FileType::Directory)
				this->children.Push(new DirectoryTreeNode(nodePath, this->snapshot));
			else
				this->children.Push(new DataFileTreeNode(nodePath, this->snapshot));
		}
	}

	this->childrenCached = true;
}

void DirectoryTreeNode::DownloadTo(const Path &dirPath) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
}

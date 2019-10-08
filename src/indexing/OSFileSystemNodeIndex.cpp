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
//Class header
#include "OSFileSystemNodeIndex.hpp"

//Constructor
OSFileSystemNodeIndex::OSFileSystemNodeIndex(const Path &path, StatusTracker &tracker, const Config &config) : basePath(path), tracker(tracker)
{
	this->GenerateIndex(config);
}

//Private methods
void OSFileSystemNodeIndex::GenerateIndex(const Config &config)
{
	auto dir = OSFileSystem::GetInstance().GetDirectory(this->basePath);
	auto dirWalker = dir->WalkFiles();

	//find files
	ProcessStatus& findStatus = this->tracker.AddProcessStatusTracker(u8"Reading directory");
	uint64 totalSize = 0;
	for(const Path& relPath : dirWalker)
	{
		AutoPointer<const FileSystemNode> node = OSFileSystem::GetInstance().GetNode(this->basePath / relPath);
		FileSystemNodeAttributes* attributes;
		switch(node->GetType())
		{
			case FileSystemNodeType::File:
			{
				AutoPointer<const File> file = node.Cast<const File>();

				attributes = new FileSystemNodeAttributes(file, config);
				totalSize += attributes->Size();
			}
				break;
			case FileSystemNodeType::Link:
			{
				AutoPointer<const Link> link = node.Cast<const Link>();
				Path target = link->ReadTarget();

				//check if target points out
				Path absoluteTarget;
				if(target.IsRelative())
					absoluteTarget = this->basePath / relPath.GetParent() / target;
				else
					absoluteTarget = target;

				if(!this->basePath.IsParentOf(absoluteTarget))
					throw LinkPointsOutOfIndexDirException();

				attributes = new FileSystemNodeAttributes(link, target, config);
			}
				break;
			default:
				RAISE(ErrorHandling::IllegalCodePathError);
		}

		uint32 index = this->nodeAttributes.Push(attributes);
		this->pathMap.Insert(relPath, index);
		findStatus.IncFileCount();
	}
	findStatus.Finished();
}

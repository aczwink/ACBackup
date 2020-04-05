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
//Class header
#include "OSFileSystemNodeIndex.hpp"
//Local
#include "../InjectionContainer.hpp"
#include "../status/StatusTracker.hpp"

//Constructor
OSFileSystemNodeIndex::OSFileSystemNodeIndex(const Path &path) : basePath(path)
{
	this->GenerateIndex();
}

//Public methods
UniquePointer<InputStream> OSFileSystemNodeIndex::OpenFile(const Path &filePath) const
{
	return new FileInputStream(this->basePath.GetString() + filePath.GetString());
}

//Private methods
void OSFileSystemNodeIndex::GenerateIndex()
{
	InjectionContainer& ic = InjectionContainer::Instance();

	auto dir = OSFileSystem::GetInstance().GetDirectory(this->basePath);
	auto dirWalker = dir->WalkFiles();

	ProcessStatus& findStatus = ic.Get<StatusTracker>().AddProcessStatusTracker(u8"Reading directory");
	for(const Path& relPath : dirWalker)
	{
		AutoPointer<const FileSystemNode> node = OSFileSystem::GetInstance().GetNode(this->basePath / relPath);

		UniquePointer<FileSystemNodeAttributes> attributes;
		switch(node->GetType())
		{
			case FileSystemNodeType::File:
			{
				FileSystemNodeAttributes *fileAttributes = new FileSystemNodeAttributes(node.Cast<const File>());
				findStatus.AddTotalSize(fileAttributes->Size());
				attributes = fileAttributes;
			}
			break;
			case FileSystemNodeType::Link:
				NOT_IMPLEMENTED_ERROR; //TODO: implement me
				break;
			default:
				RAISE(ErrorHandling::IllegalCodePathError);
		}

		this->AddNode(relPath, Move(attributes));
		findStatus.IncFileCount();
	}
	findStatus.Finished();
}
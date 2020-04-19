/*
 * Copyright (c) 2020 Amir Czwink (amir130@hotmail.de)
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
#include "FlatVolumesDirectory.hpp"
//Namespaces
using namespace StdXX;

class FlatVolumesDirectoryIteratorState : public _stdxx_::DirectoryIteratorState
{
public:
	//Constructor
	inline FlatVolumesDirectoryIteratorState(uint32 directoryIndex, const BackupNodeIndex& index)
		: index(index), directoryIndex(directoryIndex)
	{
		this->currentChildIndex = 0;
	}

	bool Equals(DirectoryIteratorState *other) const override
	{
		if(other == nullptr)
		{
			const auto& children = this->index.ChildrenOf(this->directoryIndex);
			return this->currentChildIndex >= children.GetNumberOfElements();
		}

		return false;
	}

	String GetCurrent() override
	{
		const auto& children = this->index.ChildrenOf(this->directoryIndex);
		uint32 childIndex = children[this->currentChildIndex];

		return this->index.GetNodePath(childIndex).GetName();
	}

	void Next() override
	{
		this->currentChildIndex++;
	}

private:
	//Members
	const BackupNodeIndex& index;
	uint32 directoryIndex;
	uint32 currentChildIndex;
};

//Public methods
DirectoryIterator FlatVolumesDirectory::begin() const
{
	return new FlatVolumesDirectoryIteratorState(this->directoryIndex, this->index);
}

DirectoryIterator FlatVolumesDirectory::end() const
{
	return nullptr;
}

FileSystemNodeInfo FlatVolumesDirectory::QueryInfo() const
{
	return this->index.GetFileSystemNodeInfo(this->directoryIndex);
}




//NOT IMPLEMENTED
StdXX::UniquePointer<StdXX::OutputStream> FlatVolumesDirectory::CreateFile(const StdXX::String &name)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return StdXX::UniquePointer<StdXX::OutputStream>();
}

void FlatVolumesDirectory::CreateSubDirectory(const StdXX::String &name)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
}

bool FlatVolumesDirectory::Exists(const StdXX::Path &path) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return false;
}

StdXX::AutoPointer<StdXX::FileSystemNode> FlatVolumesDirectory::GetChild(const StdXX::String &name)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return StdXX::AutoPointer<FileSystemNode>();
}

StdXX::AutoPointer<const StdXX::FileSystemNode> FlatVolumesDirectory::GetChild(const StdXX::String &name) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return StdXX::AutoPointer<const FileSystemNode>();
}

StdXX::FileSystem *FlatVolumesDirectory::GetFileSystem()
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return nullptr;
}

const StdXX::FileSystem *FlatVolumesDirectory::GetFileSystem() const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return nullptr;
}

StdXX::AutoPointer<const StdXX::Directory> FlatVolumesDirectory::GetParent() const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return StdXX::AutoPointer<const Directory>();
}

StdXX::Path FlatVolumesDirectory::GetPath() const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return StdXX::Path();
}

bool FlatVolumesDirectory::IsEmpty() const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return false;
}

void FlatVolumesDirectory::ChangePermissions(const StdXX::Filesystem::NodePermissions &newPermissions)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
}
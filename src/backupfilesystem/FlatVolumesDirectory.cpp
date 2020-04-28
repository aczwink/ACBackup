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

NodeInfo FlatVolumesDirectory::QueryInfo() const
{
	return this->index.GetFileSystemNodeInfo(this->directoryIndex);
}




//NOT IMPLEMENTED
StdXX::UniquePointer<StdXX::OutputStream> FlatVolumesDirectory::CreateFile(const StdXX::String &name)
{
	NOT_IMPLEMENTED_ERROR; //implement me
	return StdXX::UniquePointer<StdXX::OutputStream>();
}

void FlatVolumesDirectory::CreateSubDirectory(const StdXX::String &name, const FileSystem::NodePermissions* permissions)
{
	NOT_IMPLEMENTED_ERROR; //implement me
}

bool FlatVolumesDirectory::Exists(const Path &path) const
{
	NOT_IMPLEMENTED_ERROR; //implement me
	return false;
}

StdXX::AutoPointer<Node> FlatVolumesDirectory::GetChild(const StdXX::String &name)
{
	NOT_IMPLEMENTED_ERROR; //implement me
	return StdXX::AutoPointer<Node>();
}

StdXX::AutoPointer<const Node> FlatVolumesDirectory::GetChild(const StdXX::String &name) const
{
	NOT_IMPLEMENTED_ERROR; //implement me
	return StdXX::AutoPointer<const Node>();
}

bool FlatVolumesDirectory::IsEmpty() const
{
	NOT_IMPLEMENTED_ERROR; //implement me
	return false;
}

void FlatVolumesDirectory::ChangePermissions(const NodePermissions &newPermissions)
{
	NOT_IMPLEMENTED_ERROR; //implement me
}
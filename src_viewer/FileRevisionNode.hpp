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

class FileRevisionNode : public TreeNode
{
public:
	//Constructor
	inline FileRevisionNode(uint32 nodeIndex, const Snapshot& snapshot) : nodeIndex(nodeIndex), snapshot(snapshot)
	{
	}

	//Methods
	const DynamicArray<UniquePointer<TreeNode>> &GetChildren() override
	{
		return this->emptyChildren;
	}

	String GetName() const override
	{
		return u8"Revision from snapshot: " + this->snapshot.Name();
	}

	void DownloadTo(const Path &dirPath) const override
	{
		const auto& nodePath = this->snapshot.Index().GetNodePath(this->nodeIndex);
		String name = nodePath.GetTitle() + u8"_" + this->snapshot.Name() + u8"." + nodePath.GetFileExtension();

		FileOutputStream fileOutputStream(dirPath / name);
		auto input = this->snapshot.Filesystem().OpenFileForReading(this->nodeIndex, true);
		input->FlushTo(fileOutputStream);
	}

private:
	//Members
	uint32 nodeIndex;
	const Snapshot& snapshot;
	DynamicArray<UniquePointer<TreeNode>> emptyChildren;
};
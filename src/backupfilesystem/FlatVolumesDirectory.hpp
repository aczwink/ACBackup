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
#include <StdXX.hpp>
//Local
#include "../backup/BackupNodeIndex.hpp"

class FlatVolumesDirectory : public Directory
{
public:
	//Constructor
	inline FlatVolumesDirectory(uint32 directoryIndex, const BackupNodeIndex& index)
		: directoryIndex(directoryIndex), index(index)
	{
	}

	//Methods
	DirectoryIterator begin() const override;
	DirectoryIterator end() const override;
	NodeInfo QueryInfo() const override;

	//NOT IMPLEMENTED
	StdXX::UniquePointer<StdXX::OutputStream> CreateFile(const StdXX::String &name) override;
	void CreateSubDirectory(const StdXX::String &name, const FileSystem::NodePermissions* permissions) override;
	bool Exists(const Path &path) const override;
	StdXX::AutoPointer<Node> GetChild(const StdXX::String &name) override;
	StdXX::AutoPointer<const Node> GetChild(const StdXX::String &name) const override;
	bool IsEmpty() const override;
	void ChangePermissions(const NodePermissions &newPermissions) override;
	//END OF NOT IMPLEMENTED

private:
	//Members
	uint32 directoryIndex;
	const BackupNodeIndex& index;
};
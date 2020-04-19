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
#include <Std++.hpp>
//Local
#include "../backup/BackupNodeAttributes.hpp"
#include "FlatVolumesFileSystem.hpp"

class FlatVolumesFile : public StdXX::File
{
public:
	//Constructor
	inline FlatVolumesFile(uint32 fileIndex, const BackupNodeIndex& index, FlatVolumesFileSystem &fileSystem)
		: index(index), fileSystem(fileSystem), attributes(index.GetNodeAttributes(fileIndex))
	{
		this->fileIndex = fileIndex;
	}

	//Methods
	void ChangePermissions(const Filesystem::NodePermissions &newPermissions) override;
	uint64 GetSize() const override;
	UniquePointer<InputStream> OpenForReading(bool verify) const override;
	UniquePointer<OutputStream> OpenForWriting() override;
	FileSystemNodeInfo QueryInfo() const override;

private:
	//Members
	uint32 fileIndex;
	const BackupNodeIndex& index;
	const BackupNodeAttributes& attributes;
	FlatVolumesFileSystem& fileSystem;
};
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
#include "FlatVolumesLink.hpp"
//Local
#include "FlatVolumesFile.hpp"

//Public methods
void FlatVolumesLink::ChangePermissions(const NodePermissions &newPermissions)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
}

NodeInfo FlatVolumesLink::QueryInfo() const
{
	return this->index.GetFileSystemNodeInfo(this->nodeIndex);
}

Path FlatVolumesLink::ReadTarget() const
{
	FlatVolumesFile file(this->nodeIndex, this->index, const_cast<FlatVolumesFileSystem&>(this->fileSystem));
	UniquePointer<InputStream> inputStream = file.OpenForReading(false);
	TextReader textReader(*inputStream, TextCodecType::UTF8);

	const BackupNodeAttributes& attributes = this->index.GetNodeAttributes(this->nodeIndex);
	return textReader.ReadStringBySize(attributes.Size());
}

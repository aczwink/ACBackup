/*
 * Copyright (c) 2019-2021 Amir Czwink (amir130@hotmail.de)
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
#pragma once
#include <StdXX.hpp>
using namespace StdXX;
using namespace StdXX::FileSystem;
//Local
#include "FileSystemNodeIndex.hpp"
#include "../status/ProcessStatus.hpp"

class OSFileSystemNodeIndex : public FileSystemNodeIndex
{
public:
	//Constructor
	OSFileSystemNodeIndex(const Path& path);

	//Methods
	String ComputeNodeHash(uint32 nodeIndex) const;
	UniquePointer<InputStream> OpenLinkTargetAsStream(const Path& nodePath) const;
	UniquePointer<InputStream> OpenFile(const Path& filePath) const;

private:
	//Members
	Path basePath;

	//Methods
	void GenerateIndex();
	void IndexDirectoryChildren(const Path& path, ProcessStatus& findStatus);
	void IndexNode(const Path& nodePath, ProcessStatus& findStatus);

	//Inline
	inline Path MapNodePathToFileSystemPath(const Path &nodePath) const
	{
		return basePath.String() + nodePath.String();
	}
};
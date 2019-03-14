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
#pragma once
#include <Std++.hpp>
using namespace StdXX;

struct FileAttributes
{
	uint64 size;
	byte digest[16];
};

class FileIndex
{
public:
	//Destructor
	virtual ~FileIndex() {}

	//Abstract
	virtual uint32 FindFileIndex(const Path& path) const = 0;
	virtual const Path& GetFile(uint32 index) const = 0;
	virtual const FileAttributes& GetFileAttributes(uint32 index) const = 0;
	virtual uint32 GetNumberOfFiles() const = 0;
	virtual UniquePointer<InputStream> OpenFileForReading(const Path& fileEntry) const = 0;
};
/*
 * Copyright (c) 2020-2021 Amir Czwink (amir130@hotmail.de)
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
using namespace StdXX;
//Local
#include "Snapshot.hpp"

class VirtualSnapshotFilesystem : public ReadableFileSystem
{
public:
	//Constructor
	inline VirtualSnapshotFilesystem(const Snapshot& snapshot) : snapshot(snapshot)
	{
	}

	//Methods
	UniquePointer<DirectoryEnumerator> EnumerateChildren(const Path &path) const override;
	UniquePointer<InputStream> OpenFileForReading(const Path &path, bool verify) const override;
	Optional<FileInfo> QueryFileInfo(const Path &path) const override;

	//TODO: NOT IMPLEMENTED
	SpaceInfo QuerySpace() const override;
	Optional<Path> ReadLinkTarget(const Path &path) const override;
	//TODO: NOT IMPLEMENTED

private:
	//Members
	const Snapshot& snapshot;
};
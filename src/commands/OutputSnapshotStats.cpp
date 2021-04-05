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
#include "../backup/SnapshotManager.hpp"

int32 CommandOutputSnapshotStats(const Snapshot& snapshot)
{
	const BackupNodeIndex& index = snapshot.Index();
	uint32 nFiles = 0, nDirs = 0, nLinks = 0, nStoredNodes = 0;
	uint64 totalStoredSize = 0;
	for(uint32 i = 0; i < index.GetNumberOfNodes(); i++)
	{
		const BackupNodeAttributes &attributes = index.GetNodeAttributes(i);
		switch(attributes.Type())
		{
			case FileType::Directory:
				nDirs++;
				break;
			case FileType::File:
				nFiles++;
				break;
			case FileType::Link:
				nLinks++;
				break;
		}

		if(attributes.OwnsBlocks())
		{
			nStoredNodes++;
			totalStoredSize += attributes.ComputeSumOfBlockSizes();
		}
	}

	stdOut << u8"Number of nodes: " << index.GetNumberOfNodes() << endl
		<< u8"Number of directories: " << nDirs << endl
		<< u8"Number of files: " << nFiles << endl
		<< u8"Number of links: " << nLinks << endl
		<< u8"Number of nodes with data but without backreferences: " << nStoredNodes << endl
		<< u8"Total size of nodes including backreferences: " << String::FormatBinaryPrefixed(index.ComputeTotalSize()) << endl
		<< u8"Total stored size of nodes including backreferences: " << String::FormatBinaryPrefixed(index.ComputeSumOfBlockSizes()) << endl
		<< u8"Total stored size of nodes excluding backreferences: " << String::FormatBinaryPrefixed(totalStoredSize) << endl
		;


	return EXIT_SUCCESS;
}
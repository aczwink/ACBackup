/*
 * Copyright (c) 2019-2020 Amir Czwink (amir130@hotmail.de)
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
//Corresponding header
#include "Commands.hpp"
#include "../backup/SnapshotManager.hpp"

bool OutputVerificationResults(const DynamicArray<uint32> failedNodes, const Snapshot &snapshot)
{
	if(failedNodes.IsEmpty())
	{
		stdOut << u8"Snapshot '" << snapshot.Name() << u8"' successfully verified!" << endl;
		return true;
	}

	stdOut << u8"Snapshot '" << snapshot.Name() << u8"' could not be successfully verified!" << endl;
	for(uint32 fileIndex : failedNodes)
	{
		const Path& filePath = snapshot.Index().GetNodePath(fileIndex);
		stdErr << u8"File '" << filePath << u8"' is corrupt." << endl;
	}
	stdOut << u8"IMPORTANT: These files are also corrupt in all subsequent snapshots until a newer version was backed up." << endl;

	return false;
}

static bool Verify(const SnapshotManager& snapshotManager, const Snapshot &snapshot, bool full)
{
	DynamicArray<uint32> failedNodes = snapshotManager.VerifySnapshot(snapshot, full);
	return OutputVerificationResults(failedNodes, snapshot);
}

int32 CommandVerifyAllSnapshots(const SnapshotManager& snapshotManager)
{
	DynamicArray<String> snapshotsWithCorruption;
	for(const auto& snapshot : snapshotManager.Snapshots())
	{
		if(!Verify(snapshotManager, *snapshot, false))
			snapshotsWithCorruption.Push(snapshot->Name());
	}

	stdOut << u8"Summary:" << endl;
	if(snapshotsWithCorruption.IsEmpty())
		stdOut << u8"All snapshots have been successfully verified." << endl;
	else
	{
		stdOut << u8"Not all snapshots could be verified. The following suffer from data corruption: " << endl;
		for(const String& snapshotName : snapshotsWithCorruption)
			stdOut << snapshotName << endl;
	}

	return EXIT_SUCCESS;
}

int32 CommandVerifySnapshot(const SnapshotManager& snapshotManager, const Snapshot& snapshot, bool full)
{
	Verify(snapshotManager, snapshot, full);

	return EXIT_SUCCESS;
}

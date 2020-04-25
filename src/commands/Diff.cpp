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
//Local
#include "../backup/SnapshotManager.hpp"

static void Diff(const Snapshot& snapshot, const FileSystemNodeIndex& sourceIndex)
{
	NodeIndexDifferenceResolver differenceResolver;
	NodeIndexDifferences differences = differenceResolver.ComputeDiff(snapshot.Index(), sourceIndex);

	CommonFileFormats::CSVWriter csvWriter(stdOut, CommonFileFormats::csvDialect_excel);

	for(uint32 index : differences.deleted)
	{
		const Path& path = snapshot.Index().GetNodePath(index);
		csvWriter << path.String() << u8"deleted" << endl;
	}

	for(uint32 index : differences.differentData)
	{
		const Path& path = sourceIndex.GetNodePath(index);
		csvWriter << path.String() << u8"data and metadata has changed" << endl;
	}

	for(uint32 index : differences.differentMetadata)
	{
		const Path& path = sourceIndex.GetNodePath(index);
		csvWriter << path.String() << u8"metadata (only) has changed" << endl;
	}

	for(const auto& kv : differences.moved)
	{
		const Path& oldPath = snapshot.Index().GetNodePath(kv.value);
		const Path& newPath = sourceIndex.GetNodePath(kv.key);

		csvWriter << oldPath.String() << u8"node was moved" << newPath.String() << endl;
	}
}

int32 CommandDiffSnapshotWithSourceDirectory(SnapshotManager& snapshotManager, const String& snapshotName)
{
	const Snapshot* snapshot = snapshotManager.FindSnapshot(snapshotName);
	if(!snapshot)
	{
		stdErr << u8"Snapshot with name '" << snapshotName << u8"' not found." << endl;
		return EXIT_FAILURE;
	}
	OSFileSystemNodeIndex sourceIndex(InjectionContainer::Instance().Get<ConfigManager>().Config().sourcePath);
	Diff(*snapshot, sourceIndex);

	return EXIT_SUCCESS;
}

int32 CommandDiffSnapshots(SnapshotManager& snapshotManager, const String& snapshotName, const String& otherSnapshotName)
{
	const Snapshot* snapshot1 = snapshotManager.FindSnapshot(snapshotName);
	if(!snapshot1)
	{
		stdErr << u8"Snapshot with name '" << snapshotName << u8"' not found." << endl;
		return EXIT_FAILURE;
	}
	const Snapshot* snapshot2 = snapshotManager.FindSnapshot(otherSnapshotName);
	if(!snapshot2)
	{
		stdErr << u8"Snapshot with name '" << otherSnapshotName << u8"' not found." << endl;
		return EXIT_FAILURE;
	}
	Diff(*snapshot1, snapshot2->Index());

	return EXIT_SUCCESS;
}
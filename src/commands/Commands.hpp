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
#include <StdXX.hpp>
using namespace StdXX;
using namespace StdXX::FileSystem;
//Local
#include "../backup/SnapshotManager.hpp"

//Prototypes
int32 CommandAddSnapshot(SnapshotManager& snapshotManager);
int32 CommandDiffSnapshots(SnapshotManager& snapshotManager, const String& snapshotName, const String& otherSnapshotName);
int32 CommandDiffSnapshotWithSourceDirectory(SnapshotManager& snapshotManager, const String& snapshotName);
int32 CommandInit(const Path& sourcePath);
int32 CommandOutputSnapshotHashValues(const Snapshot& snapshot);
int32 CommandOutputSnapshotHashValues(const Snapshot& snapshot, const String& hashAlgorithm);
int32 CommandOutputSnapshotStats(const Snapshot& snapshot);
int32 CommandVerifyAllSnapshots();
int32 CommandVerifySnapshot(SnapshotManager& snapshotManager, const Snapshot& snapshot, bool full);
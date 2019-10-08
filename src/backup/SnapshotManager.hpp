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
//Local
#include "../Config.hpp"
#include "../indexing/FileSystemNodeIndex.hpp"
#include "../status/StatusTracker.hpp"
#include "Snapshot.hpp"

class SnapshotManager
{
public:
	//Constructor
	SnapshotManager(const Path& path, const Config& config, const StatusTracker& statusTracker);

	//Methods
	void AddSnapshot(const FileSystemNodeIndex& index);

private:
	//Members
	Path backupPath;
	DynamicArray<UniquePointer<Snapshot>> snapshots;

	//Methods
	void ReadInSnapshots();
};
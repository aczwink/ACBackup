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
#include <Std++.hpp>
using namespace StdXX;
//Local
#include "../Config.hpp"
#include "../status/StatusTracker.hpp"
#include "../backup/SnapshotManager.hpp"
#include "../indexing/OSFileSystemNodeIndex.hpp"

int32 CommandAddSnapshot(const Path& backupPath, const Path& sourcePath)
{
	Config config(backupPath);

	StatusTracker tracker(config.Port());
	SnapshotManager snapshotManager(backupPath, config, tracker);

	OSFileSystemNodeIndex sourceIndex(sourcePath, tracker, config);
	snapshotManager.AddSnapshot(sourceIndex);

	return EXIT_SUCCESS;
}
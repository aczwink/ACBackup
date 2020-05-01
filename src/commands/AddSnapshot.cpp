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
//Local
#include "../indexing/OSFileSystemNodeIndex.hpp"
#include "../status/StatusTracker.hpp"
#include "../backup/SnapshotManager.hpp"
#include "../config/CompressionStatistics.hpp"

int32 CommandAddSnapshot(SnapshotManager& snapshotManager)
{
	InjectionContainer& ic = InjectionContainer::Instance();

	OSFileSystemNodeIndex sourceIndex(ic.Get<ConfigManager>().Config().sourcePath);
	if(snapshotManager.AddSnapshot(sourceIndex))
		stdOut << u8"Snapshot creating successful." << endl;
	else
		stdOut << u8"Snapshot creation failed. The snapshot is corrupt." << endl;

	return EXIT_SUCCESS;
}

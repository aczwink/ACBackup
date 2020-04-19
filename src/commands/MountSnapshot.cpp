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

int32 CommandMountSnapshot(const String& snapshotName, const Path& mountPoint)
{
	InjectionContainer &ic = InjectionContainer::Instance();

	ConfigManager configManager;
	ic.Register(configManager);

	UniquePointer<StatusTracker> statusTracker = StatusTracker::CreateInstance(configManager.Config().statusTrackerType);
	ic.Register(*statusTracker);

	StaticThreadPool threadPool;
	ic.Register(threadPool);

	SnapshotManager snapshotManager;
	const Snapshot* snapshot = snapshotManager.FindSnapshot(snapshotName);
	if(!snapshot)
	{
		stdErr << u8"Snapshot with name '" << snapshotName << u8"' not found." << endl;
		return EXIT_FAILURE;
	}
	snapshot->Mount(mountPoint);

	return EXIT_SUCCESS;
}
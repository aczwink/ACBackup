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
//Local
#include "commands/Commands.hpp"
#include "InjectionContainer.hpp"

static void PrintManual()
{
    stdOut << u8"Usage: ACBackup [options...]" << endl
            << endl

            << u8"-a sourceDirectory, --add-snapshot sourceDirectory" << endl
            << u8"  " << u8"Backup a new snapshot into backup directory. Verifies the snapshot after backup." << endl
            << endl

            << u8"-i, --init" << endl
            << u8"  " << u8"Initialize new empty backup directory in current working directory." << endl
            << endl

			<< u8"-m snapshotName mountPoint, --mount snapshotName mountPoint" << endl
			<< u8"  " << u8"Mounts a snapshot into the filesystem. The mounted filesystem can only be read but not be modified." << endl
			<< endl

			<< u8"-v snapshotName, --verify-snapshot snapshotName" << endl
			<< u8"  " << u8"Verifies the integrity of all data including metadata of a snapshot." << endl
			<< endl

			<< u8"-V snapshotName, --verify-snapshot-fully snapshotName" << endl
			<< u8"  " << u8"Verifies the integrity of all data including metadata of a snapshot including references to older snapshots." << endl
			<< endl

			<< u8"--verify-all-snapshots" << endl
			<< u8"  " << u8"Verifies the integrity of all data including metadata of all snapshots (i.e. the whole history)." << endl
			<< endl

			<< u8"--verify-newest-snapshot" << endl
			<< u8"  " << u8"Verifies the integrity of all data including metadata of the newest snapshot including references to older snapshots." << endl
			<< endl;
}

int32 Main(const String& programName, const FixedArray<String>& args)
{
    //TODO debugging
    String newestSnapshot = "snapshot_2020-04-19_13_59_08";
	CommandInit(String(u8"/home/amir/opt"));
	//CommandAddSnapshot();
	CommandMountSnapshot(newestSnapshot, String(u8"/home/amir/mount"));
	//CommandVerifyAllSnapshots();
	//CommandVerifyNewestSnapshot();
	//CommandVerifySnapshot(u8"snapshot_2020-04-14_18_20_20", false);
    //TODO end debugging

    PrintManual();

	InjectionContainer::Instance().UnregisterAll();
    return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
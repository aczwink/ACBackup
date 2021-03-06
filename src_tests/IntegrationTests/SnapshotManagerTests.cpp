/*
 * Copyright (c) 2021 Amir Czwink (amir130@hotmail.de)
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
#include <StdXXTest.hpp>
//Local
#include "../../src/backup/SnapshotManager.hpp"
#include "../../src/commands/Commands.hpp"
#include "TestBackupCreator.hpp"
//Namespaces
using namespace StdXX;

uint32 CountOwnedFiles(const Snapshot& snapshot)
{
	uint32 count = 0;

	for(uint32 i = 0; i < snapshot.Index().GetNumberOfNodes(); i++)
	{
		if(snapshot.Index().GetNodeAttributes(i).OwnsBlocks())
			count++;
	}

	return count;
}

TEST_SUITE(SnapshotManagerTests)
{
	TEST_CASE(CreateSnapshotAndThenReadStats)
	{
		TestBackupCreator testBackupCreator;
		SnapshotManager snapshotManager;

		testBackupCreator.AddSourceDir({u8"/testdir"});
		testBackupCreator.AddSourceFile({u8"/testdir/nested"}, u8"test");
		testBackupCreator.AddSourceFile({u8"/test"}, u8"test");
		testBackupCreator.AddSourceLink({u8"/testlink"}, u8"test");

		int32 result = CommandAddSnapshot(snapshotManager);
		ASSERT_EQUALS(EXIT_SUCCESS, result);

		testBackupCreator.VerifySnapshotMatchesTestState(snapshotManager.NewestSnapshot());
		ASSERT_EQUALS(3, CountOwnedFiles(snapshotManager.NewestSnapshot()));
	}

	TEST_CASE(UnchangedFileShouldNotBeBackuppedAgain)
	{
		TestBackupCreator testBackupCreator;
		SnapshotManager snapshotManager;

		testBackupCreator.AddSourceFile({u8"/test"}, u8"test");

		int32 result = CommandAddSnapshot(snapshotManager);
		ASSERT_EQUALS(EXIT_SUCCESS, result);

		testBackupCreator.VerifySnapshotMatchesTestState(snapshotManager.NewestSnapshot());
		ASSERT_EQUALS(1, CountOwnedFiles(snapshotManager.NewestSnapshot()));

		Sleep(1 * 1000 * 1000 * 1000); //snapshot names are based on the current time and have second precision

		result = CommandAddSnapshot(snapshotManager);
		ASSERT_EQUALS(EXIT_SUCCESS, result);

		testBackupCreator.VerifySnapshotMatchesTestState(snapshotManager.NewestSnapshot());
		ASSERT_EQUALS(0, CountOwnedFiles(snapshotManager.NewestSnapshot()));
	}

	TEST_CASE(ChangedFileShouldBeBackuppedAgain)
	{
		TestBackupCreator testBackupCreator;
		SnapshotManager snapshotManager;

		testBackupCreator.AddSourceFile({u8"/test"}, u8"test");

		int32 result = CommandAddSnapshot(snapshotManager);
		ASSERT_EQUALS(EXIT_SUCCESS, result);

		testBackupCreator.VerifySnapshotMatchesTestState(snapshotManager.NewestSnapshot());
		ASSERT_EQUALS(1, CountOwnedFiles(snapshotManager.NewestSnapshot()));

		Sleep(1 * 1000 * 1000 * 1000); //snapshot names are based on the current time and have second precision
		testBackupCreator.AddSourceFile({u8"/test"}, u8"test2");

		result = CommandAddSnapshot(snapshotManager);
		ASSERT_EQUALS(EXIT_SUCCESS, result);

		testBackupCreator.VerifySnapshotMatchesTestState(snapshotManager.NewestSnapshot());
		ASSERT_EQUALS(1, CountOwnedFiles(snapshotManager.NewestSnapshot()));
	}

	TEST_CASE(MovedFileShouldNotBeBackuppedAgain)
	{
		TestBackupCreator testBackupCreator;
		SnapshotManager snapshotManager;

		testBackupCreator.AddSourceFile({u8"/test"}, u8"test");

		int32 result = CommandAddSnapshot(snapshotManager);
		ASSERT_EQUALS(EXIT_SUCCESS, result);

		testBackupCreator.VerifySnapshotMatchesTestState(snapshotManager.NewestSnapshot());
		ASSERT_EQUALS(1, CountOwnedFiles(snapshotManager.NewestSnapshot()));

		Sleep(1 * 1000 * 1000 * 1000); //snapshot names are based on the current time and have second precision
		testBackupCreator.RemoveFile({u8"/test"});
		testBackupCreator.AddSourceFile({u8"/changed"}, u8"test");

		result = CommandAddSnapshot(snapshotManager);
		ASSERT_EQUALS(EXIT_SUCCESS, result);

		testBackupCreator.VerifySnapshotMatchesTestState(snapshotManager.NewestSnapshot());
		ASSERT_EQUALS(0, CountOwnedFiles(snapshotManager.NewestSnapshot()));
	}
};
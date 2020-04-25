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
#include "backup/SnapshotManager.hpp"
#include "config/CompressionStatistics.hpp"
//Namespaces
using namespace StdXX::CommandLine;

struct AutoInjectionClearer
{
	~AutoInjectionClearer()
	{
		InjectionContainer::Instance().UnregisterAll();
	}
};

int32 Main(const String& programName, const FixedArray<String>& args)
{
	AutoInjectionClearer autoInjectionClearer;

	Parser commandLineParser(programName);
	commandLineParser.AddHelpOption();

	OptionWithArgument snapshotName(u8's', u8"snapshot-name", u8"Use another snapshot than the newest one as source");

	SubCommandArgument subCommandArgument(u8"command", u8"The command that should be executed");


	Group addSnapshot(u8"add-snapshot", u8"Backup a new snapshot into backup directory. Verifies the snapshot after backup.");
	subCommandArgument.AddCommand(addSnapshot);


	Group diff(u8"diff", u8"Finds the differences between the newest snapshot and the source directory.");

	OptionWithArgument sourceSnapshotName(u8's', u8"source-snapshot-name", u8"Use another snapshot than the newest one as source");
	diff.AddOption(sourceSnapshotName);
	OptionWithArgument targetSnapshotName(u8'd', u8"target-snapshot-name", u8"Instead of comparing against the source directory, compare against a given snapshot");
	diff.AddOption(targetSnapshotName);

	subCommandArgument.AddCommand(diff);


	Group hashes(u8"hashes", u8"Outputs hashes with of all nodes including backreferences of the newest snapshot.");

	hashes.AddOption(snapshotName);
	OptionWithArgument hashAlgorithm(u8'a', u8"algorithm", u8"Use another hash algorithm than the one defined in the config");
	hashes.AddOption(hashAlgorithm);

	subCommandArgument.AddCommand(hashes);


	Group init(u8"init", u8"Initialize new empty backup directory in current working directory.");
	PathArgument sourceDirectory(u8"sourceDir", u8"The root directory that serves as a source for backup.");
	init.AddPositionalArgument(sourceDirectory);
	subCommandArgument.AddCommand(init);


	Group mount(u8"mount", u8"Mounts the newest snapshot into the filesystem. The mounted filesystem can only be read but not be modified.");
	mount.AddOption(snapshotName);
	PathArgument mountPoint(u8"mountPoint", u8"The path where the snapshot will be mounted in.");
	mount.AddPositionalArgument(mountPoint);
	subCommandArgument.AddCommand(mount);



	Group restoreSnapshot(u8"restore-snapshot", u8"Restores the newest snapshot back into a specified directory in the filesystem.");
	restoreSnapshot.AddOption(snapshotName);
	PathArgument restorePoint(u8"restorePoint", u8"The path where the snapshot will be restored to.");
	restoreSnapshot.AddPositionalArgument(restorePoint);
	subCommandArgument.AddCommand(restoreSnapshot);


	Group stats(u8"stats", u8"Output statistical information about the newest snapshot.");
	stats.AddOption(snapshotName);
	subCommandArgument.AddCommand(stats);



	Group verify(u8"verify", u8"Verifies the integrity of all data including metadata of the newest snapshot including references to older snapshots.");
	verify.AddOption(snapshotName);
	Option local(u8'l', u8"local", u8"Skip backreferences");
	verify.AddOption(local);
	subCommandArgument.AddCommand(verify);


	CommandLine::Group verifyAll(u8"verify-all", u8"Verifies the integrity of all data including metadata of all snapshots (i.e. the whole history).");
	subCommandArgument.AddCommand(verifyAll);


	commandLineParser.AddPositionalArgument(subCommandArgument);

	if(!commandLineParser.Parse(args))
	{
		stdErr << commandLineParser.GetErrorText() << endl;
		return EXIT_FAILURE;
	}

	if(commandLineParser.IsHelpActivated())
	{
		commandLineParser.PrintHelp();
		return EXIT_SUCCESS;
	}

	const MatchResult& matchResult = commandLineParser.ParseResult();

	if(matchResult.IsActivated(init))
		return CommandInit(sourceDirectory.Value(matchResult));

	InjectionContainer &ic = InjectionContainer::Instance();

	ConfigManager configManager;
	ic.Register(configManager);

	UniquePointer<StatusTracker> statusTracker = StatusTracker::CreateInstance(configManager.Config().statusTrackerType);
	ic.Register(*statusTracker);

	StaticThreadPool threadPool;
	ic.Register(threadPool);

	SnapshotManager snapshotManager;

	if(matchResult.IsActivated(addSnapshot))
	{
		CompressionStatistics compressionStatistics(configManager.Config().backupPath);
		ic.Register(compressionStatistics);

		return CommandAddSnapshot(snapshotManager);
	}
	else if(matchResult.IsActivated(diff))
	{
		if(snapshotManager.Snapshots().IsEmpty())
		{
			stdErr << u8"No snapshot has ever been created." << endl;
			return EXIT_FAILURE;
		}
		String sourceSnapshotNameText = matchResult.IsActivated(sourceSnapshotName) ? sourceSnapshotName.Value(matchResult) : snapshotManager.NewestSnapshot().Name();

		if(matchResult.IsActivated(targetSnapshotName))
			return CommandDiffSnapshots(snapshotManager, sourceSnapshotNameText, targetSnapshotName.Value(matchResult));
		return CommandDiffSnapshotWithSourceDirectory(snapshotManager, sourceSnapshotNameText);
	}

	if(snapshotManager.Snapshots().IsEmpty())
	{
		stdErr << u8"No snapshot has ever been created." << endl;
		return EXIT_FAILURE;
	}
	const Snapshot* snapshot;
	if(matchResult.IsActivated(snapshotName))
	{
		String snapshotNameText = snapshotName.Value(matchResult);
		snapshot = snapshotManager.FindSnapshot(snapshotNameText);
		if(!snapshot)
		{
			stdErr << u8"Snapshot with name '" << snapshotNameText << u8"' not found." << endl;
			return EXIT_FAILURE;
		}
	}
	else
		snapshot = &snapshotManager.NewestSnapshot();

	if(matchResult.IsActivated(hashes))
	{
		if(matchResult.IsActivated(hashAlgorithm))
			return CommandOutputSnapshotHashValues(*snapshot, hashAlgorithm.Value(matchResult));
		return CommandOutputSnapshotHashValues(*snapshot);
	}
	else if(matchResult.IsActivated(mount))
	{
		snapshot->Mount(mountPoint.Value(matchResult));
		return EXIT_SUCCESS;
	}
	else if(matchResult.IsActivated(restoreSnapshot))
	{
		snapshot->Restore(restorePoint.Value(matchResult));
		return EXIT_SUCCESS;
	}
	else if(matchResult.IsActivated(stats))
	{
		return CommandOutputSnapshotStats(*snapshot);
	}
	else if(matchResult.IsActivated(verify))
	{
		bool localBool = snapshot != &snapshotManager.NewestSnapshot();
		if(matchResult.IsActivated(local))
			localBool = true;
		return CommandVerifySnapshot(snapshotManager, *snapshot, !localBool);
	}
	else if(matchResult.IsActivated(verifyAll))
	{
		return CommandVerifyAllSnapshots();
	}

	stdErr << commandLineParser.GetErrorText() << endl;
	return EXIT_FAILURE;
}
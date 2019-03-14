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
#include "FileSystemIndex.hpp"
#include "BackupManager.hpp"
#include "KeyDerivation.hpp"

static void PrintManual()
{
	stdOut << u8"Usage: ACBackup command [args...] [options...]" << endl
		   << endl
		   << u8"command: add-snapshot" << endl
		   << u8"Backup a new snapshot into backup directory. Verifies the snapshot after backup." << endl
		   << u8"Args: source directory" << endl
		   << u8"Options:" << endl
		   << u8"-l size\t>Limit in MiB for files that are compressed in memory rather than streamed. Defaults to 0 (i.e. never compress in memory)."
		   << endl

		   << u8"command: init" << endl
		   << u8"Initialize new empty backup directory in current working directory." << endl
		   << u8"Args: none" << endl
		   << u8"Options:" << endl
		   << u8"-e\tEncrypt backup dir" << endl
		   << endl

		   << u8"command: verify" << endl
		   << u8"Verifies file integrity of the current backed up state (i.e. for each file just checks the newest version)." << endl
		   << u8"Args: none" << endl
		   << u8"Options: none" << endl
		   << endl

		   << u8"command: verify-full" << endl
		   << u8"Verifies the integrity of all files in all snapshots (i.e. the whole history)." << endl
		   << u8"Args: none" << endl
		   << u8"Options: none" << endl
		   << endl

		   << u8"command: verify-snapshot" << endl
		   << u8"Verifies the integrity of all files in one snapshots." << endl
		   << u8"Args: snapshot name" << endl
		   << u8"Options: none" << endl
		   << endl;
};

static int32 AddSnapshot(const Path& backupPath, const Path& sourcePath, uint64 limit)
{
	BackupManager backupManager(backupPath);
	StatusTracker tracker;

	FileSystemIndex sourceIndex(sourcePath, tracker);
	backupManager.AddSnapshot(sourceIndex, tracker, limit);

	return EXIT_SUCCESS;
}

static int32 Init(const Path& dirPath, bool encrypted)
{
	//check if dir is empty
	auto dir = OSFileSystem::GetInstance().GetDirectory(dirPath);
	auto dirWalker = dir->WalkFiles();

	bool empty = true;
	for (const auto &file : dirWalker)
	{
		empty = false;
		break;
	}

	if (!empty)
	{
		stdErr << u8"Directory is not empty. Can not create backup dir here...";
		return EXIT_FAILURE;
	}

	//generate files
	BackupManager::WriteCompressionStatsFile(dirPath, {});

	if (encrypted)
	{
		//ask for master password
		String pw = u8"test"; //TODO: add enter password

		//generate 64 bytes of random salt
		constexpr uint8 saltSize = 64;
		uint8 salt[saltSize];

		for(uint8 i = 0; i < saltSize; i++)
		{
			salt[i] = 0xAB; //TODO: add randomization
		}

		//... and master key
		FixedArray<uint8> masterKey = DeriveMasterKey(pw, salt, saltSize);

		//write master key
		Tuple<FixedArray<uint8>, FixedArray<uint8>> appKeyAndIV = DeriveAppKey();
		FixedArray<uint8> appKey = appKeyAndIV.Get<0>();
		FixedArray<uint8> appIV = appKeyAndIV.Get<1>();
		FileOutputStream masterKeyFile(String(u8"master_key"));
		Crypto::CBCCipher masterKeyCipher(CipherAlgorithm::AES, &appKey[0], appKey.GetNumberOfElements(), &appIV[0], masterKeyFile);

		masterKeyCipher.WriteBytes(salt, saltSize); //we don't actually write the key but the salt, that is required to generate the key
		FixedArray<uint8> verificationMsg = GenerateVerificationMessage();

		uint8 len = static_cast<uint8>(verificationMsg.GetNumberOfElements());
		masterKeyCipher.WriteBytes(&len, 1);

		masterKeyCipher.WriteBytes(&verificationMsg[0], verificationMsg.GetNumberOfElements());

		masterKeyCipher.Flush();

		//TODO: if when decrypting verificationMsgSize is odd, fail
		//TODO: if when decrypting sha256(verificationMsg) = readverificationmessagesha then ok else false
		//TODO: all snapshot keys are derived by master key with their "snapshot_2019-02-22_19_59_01" name. figure out what is smart
	}

	return EXIT_SUCCESS;
}

static int32 VerifySnapshot(const Path& backupPath, const String& snapshotName)
{
	BackupManager backupManager(backupPath);
	StatusTracker tracker;

	if(backupManager.VerifySnapshot(snapshotName, tracker))
	{
		stdOut << u8"Snapshot verification successful." << endl;
	}
	else
	{
		stdErr << u8"Couldn't find snapshot: " << snapshotName << endl;
	}

	return EXIT_SUCCESS;
}

int32 Main(const String& programName, const FixedArray<String>& args)
{
	if(args.GetNumberOfElements() == 0)
	{
		PrintManual();
		return EXIT_SUCCESS;
	}

	//TODO debugging
	//Init(OSFileSystem::GetInstance().GetWorkingDirectory(), false);
	//add-snapshot /Users/amir/Desktop -l 128
	//add-snapshot /Users/amir/Downloads -l 128
	//verify-snapshot snapshot_2019-03-14_22_22_52
	//TODO end debugging

	/*
	 * LZMA stats: ca. 1 - 2 mb/s for compression
	 * 17 mb/s uncompressing
	 */

	Path backupDir = OSFileSystem::GetInstance().GetWorkingDirectory();
	const String& command = args[0];
	if(command == u8"add-snapshot")
	{
		if(args.GetNumberOfElements() < 2)
		{
			stdErr << u8"Missing source path" << endl;
			return EXIT_FAILURE;
		}

		uint64 memLimit = 0;
		for(uint32 i = 2; i < args.GetNumberOfElements(); i++)
		{
			if(args[i] == u8"-l")
			{
				i++;
				memLimit = args[i].ToUInt();
			}
		}

		Path sourcePath = OSFileSystem::GetInstance().FromNativePath(args[1]);

		return AddSnapshot(backupDir, sourcePath, memLimit);
	}
	else if(command == u8"init")
	{
		bool encrypt = false;
		for(uint32 i = 1; i < args.GetNumberOfElements(); i++)
		{
			if(args[i] == u8"-e")
				encrypt = true;
		}

		return Init(OSFileSystem::GetInstance().GetWorkingDirectory(), encrypt);
	}
	else if(command == u8"verify-snapshot")
	{
		if(args.GetNumberOfElements() < 2)
		{
			stdErr << u8"Missing snapshot name" << endl;
			return EXIT_FAILURE;
		}
		String snapshotName = args[1];
		return VerifySnapshot(backupDir, snapshotName);
	}
	else
	{
		stdErr << u8"Unknown command" << endl;
		PrintManual();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
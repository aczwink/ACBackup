//Local
#include "indexing/OSFileSystemNodeIndex.hpp"
#include "backup/BackupManager.hpp"
#include "KeyDerivation.hpp"

static void PrintManual()
{
	stdOut << u8"Usage: ACBackup command [args...] [options...]" << endl
		   << u8"command: restore-snapshot" << endl
		   << u8"Restore a snapshot to a given location." << endl
		   << u8"Args: snapshot name and target location in that order" << endl
		   << u8"Options: none" << endl
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

/*
static int32 GenerateMasterKey(const Path& dirPath, bool encrypted)
{
	//master key generation

	//ask for master password
	stdOut << u8"Enter password: ";
	String pw = stdIn.ReadUnechoedLine();

	stdOut << u8"Enter password again: ";
	String confirmation = stdIn.ReadUnechoedLine();

	if(pw != confirmation)
	{
		stdErr << u8"Passwords do not match!" << endl;
		return EXIT_FAILURE;
	}

	//generate 64 bytes of random salt
	uint8 salt[c_masterKey_saltSize];
	GenerateSafeRandomBytes(salt, c_masterKey_saltSize);

	//... and master key
	FixedArray<uint8> masterKey = DeriveMasterKey(pw, salt, c_masterKey_saltSize);

	//... and salt for sub key derivation
	uint8 subKeySalt[c_subKeyDerive_saltSize];
	GenerateSafeRandomBytes(subKeySalt, c_subKeyDerive_saltSize);

	//... and an iv for verification
	uint8 verifyIV[AES_BLOCK_SIZE];
	GenerateSafeRandomBytes(verifyIV, AES_BLOCK_SIZE);

	//write master key
	Tuple<FixedArray<uint8>, FixedArray<uint8>> appKeyAndIV = DeriveAppKey();
	FixedArray<uint8> appKey = Move(appKeyAndIV.Get<0>());
	FixedArray<uint8> appIV = Move(appKeyAndIV.Get<1>());

	FileOutputStream masterKeyFile(String(u8"master_key"));

	//we write the master key salt encrypted with the app key
	Crypto::CBCCipher appKeyCipher(CipherAlgorithm::AES, &appKey[0], appKey.GetNumberOfElements(), &appIV[0], masterKeyFile);

	appKeyCipher.WriteBytes(&c_masterKey_saltSize, 1);
	appKeyCipher.WriteBytes(salt, c_masterKey_saltSize); //we don't actually write the key but the salt, that is required to generate the key
	appKeyCipher.WriteBytes(verifyIV, AES_BLOCK_SIZE);
	appKeyCipher.Finalize();

	//write subkey salt, generate verification message, encrypt it with master key (and NOT with app key) and write it to file
	Crypto::CBCCipher masterKeyCipher(CipherAlgorithm::AES, &masterKey[0], masterKey.GetNumberOfElements(), verifyIV, masterKeyFile);

	uint8 len = c_subKeyDerive_saltSize;
	masterKeyCipher.WriteBytes(&len, 1);
	masterKeyCipher.WriteBytes(subKeySalt, c_subKeyDerive_saltSize);

	FixedArray<uint8> verificationMsg = GenerateVerificationMessage();
	len = static_cast<uint8>(verificationMsg.GetNumberOfElements());
	masterKeyCipher.WriteBytes(&len, 1);
	masterKeyCipher.WriteBytes(&verificationMsg[0], verificationMsg.GetNumberOfElements());
	masterKeyCipher.Finalize();
}*/

static bool IsDirectoryEmpty(const Path& dirPath)
{
	auto dir = OSFileSystem::GetInstance().GetDirectory(dirPath);
	return dir->IsEmpty();
}

static int32 RestoreSnapshot(const Path& backupPath, const String& snapshotName, const Path& targetPath)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	/*
	BackupManager backupManager(backupPath, config);
	StatusTracker tracker;

	if(!IsDirectoryEmpty(targetPath))
	{
		stdErr << u8"Target directory is not empty. Can not restore snapshot to a not empty location..." << endl;
		return EXIT_FAILURE;
	}

	if(backupManager.RestoreSnapshot(snapshotName, targetPath, tracker))
	{
		stdOut << u8"Snapshot restoration successful." << endl;
	}
	else
	{
		stdErr << u8"Couldn't find snapshot: " << snapshotName << endl;
	}*/
	return EXIT_SUCCESS;
}

static int32 VerifySnapshot(const Path& backupPath, const String& snapshotName)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	/*
	BackupManager backupManager(backupPath);
	StatusTracker tracker;

	if(backupManager.VerifySnapshot(snapshotName, tracker))
	{
		stdOut << u8"Snapshot verification successful." << endl;
	}
	else
	{
		stdErr << u8"Couldn't find snapshot: " << snapshotName << endl;
	}*/

	return EXIT_SUCCESS;
}

int32 Main(const String& programName, const FixedArray<String>& args)
{
	if(args.GetNumberOfElements() == 0)
	{
		PrintManual();
		return EXIT_SUCCESS;
	}

	Path backupDir = OSFileSystem::GetInstance().GetWorkingDirectory();
	const String& command = args[0];
	if(command == u8"add-snapshot")
	{
		if(args.GetNumberOfElements() != 2)
		{
			stdErr << u8"The 'add-snapshot' command does take only the source directory as argument." << endl;
			return EXIT_FAILURE;
		}

		Path sourcePath = OSFileSystem::GetInstance().FromNativePath(args[1]);
		return AddSnapshot(backupDir, sourcePath);
	}
	else if(command == u8"init")
	{
		if(args.GetNumberOfElements() != 1)
		{
			stdErr << u8"The 'init' command does not take arguments." << endl;
			return EXIT_FAILURE;
		}

		return Init(OSFileSystem::GetInstance().GetWorkingDirectory());
	}
	else if(command == u8"restore-snapshot")
	{
		if(args.GetNumberOfElements() < 3)
		{
			stdErr << u8"Wrong arguments" << endl;
			return EXIT_FAILURE;
		}
		String snapshotName = args[1];
		Path targetPath = OSFileSystem::GetInstance().FromNativePath(args[2]);
		return RestoreSnapshot(backupDir, snapshotName, targetPath);
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
	}
}
//Local
#include "indexing/OSFileSystemNodeIndex.hpp"
#include "../backup/BackupManager.hpp"
#include "encryption/KeyDerivation.hpp"

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
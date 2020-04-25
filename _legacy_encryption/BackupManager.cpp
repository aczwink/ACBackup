
//Local
#include "../encryption/FlatContainerIndex.hpp"
#include "../encryption/KeyDerivation.hpp"
#include "../encryption/KeyVerificationFailedException.hpp"
#include "../BackupContainerIndex.hpp"

//Constructor
BackupManager::BackupManager(const Path &path, const Config& config), config(config)
{
	//check for master_key
	Path masterKeyPath = this->backupPath / String(u8"master_key");
	if(OSFileSystem::GetInstance().Exists(masterKeyPath))
	{
		//ask for master password
		stdOut << u8"Enter password: ";
		String pw = stdIn.ReadUnechoedLine();

		Tuple<FixedArray<uint8>, FixedArray<uint8>> appKeyAndIV = DeriveAppKey();
		FixedArray<uint8> appKey = Move(appKeyAndIV.Get<0>());
		FixedArray<uint8> appIV = Move(appKeyAndIV.Get<1>());

		FileInputStream fileInputStream(masterKeyPath);
		Crypto::CBCDecipher appKeyDecipher(CipherAlgorithm::AES, &appKey[0], static_cast<uint16>(appKey.GetNumberOfElements()), &appIV[0], fileInputStream);

		uint8 masterKeySaltLen;
		appKeyDecipher.ReadBytes(&masterKeySaltLen, 1);
		FixedArray<uint8> masterKeySalt(masterKeySaltLen);
		appKeyDecipher.ReadBytes(&masterKeySalt[0], masterKeySaltLen);

		uint8 verifyIV[AES_BLOCK_SIZE];
		appKeyDecipher.ReadBytes(verifyIV, AES_BLOCK_SIZE);

		//generate master key and read verification message
		FixedArray<uint8> masterKey = DeriveMasterKey(pw, &masterKeySalt[0], masterKeySaltLen);
		Crypto::CBCDecipher masterKeyDecipher(CipherAlgorithm::AES, &masterKey[0], static_cast<uint16>(masterKey.GetNumberOfElements()), verifyIV, fileInputStream);

		uint8 len;
		masterKeyDecipher.ReadBytes(&len, 1);
		FixedArray<uint8> subKeyDeriveSalt(len);
		masterKeyDecipher.ReadBytes(&subKeyDeriveSalt[0], len);

		uint8 verificationMsgLen;
		masterKeyDecipher.ReadBytes(&verificationMsgLen, 1);

		FixedArray<uint8> verificationMessage(verificationMsgLen);
		uint32 nBytesRead = masterKeyDecipher.ReadBytes(&verificationMessage[0], verificationMsgLen);
		if(nBytesRead != verificationMsgLen)
			throw KeyVerificationFailedException();

		//generate sha of verification message
		UniquePointer<Crypto::HashFunction> hasher = Crypto::HashFunction::CreateInstance(Crypto::HashAlgorithm::SHA256);
		hasher->Update(&verificationMessage[0], verificationMessage.GetNumberOfElements() - hasher->GetDigestSize());
		hasher->Finish();
		FixedArray<uint8> computedVerificationHash = hasher->GetDigest();

		bool ok = MemCmp(&computedVerificationHash[0], &verificationMessage[verificationMessage.GetNumberOfElements() - hasher->GetDigestSize()], hasher->GetDigestSize()) == 0;
		if(!ok)
			throw KeyVerificationFailedException();

		EncryptionInfo encInfo = {Move(masterKey), Move(subKeyDeriveSalt)};
		this->encryptionInfo = Move(encInfo);
	}
}
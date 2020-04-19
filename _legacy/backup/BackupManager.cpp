
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

//Public methods
void BackupManager::AddSnapshot(const FileSystemNodeIndex &index, StatusTracker& tracker)
{
	for(uint32 i = 0; i < index.GetNumberOfNodes(); i++)
	{
		if(diffNodeIndices.Contains(i))
		{
			this->threadPool.EnqueueTask([this, &index, &snapshot, &process, i, maxCompressionLevel, memLimit]()
			{
				switch(fileAttributes.Type())
				{
					case IndexableNodeType::File:
					{
						String ext = filePath.GetFileExtension();
						float32 compressionRate = this->GetCompressionRate(ext);
						compressionRate = snapshot->BackupFile(filePath, index, compressionRate, maxCompressionLevel, memLimit);
						this->AddCompressionRateSample(ext, compressionRate);
					}
					break;
				}
			});
		}
	}
}

void BackupManager::RestoreSnapshot(const Snapshot &snapshot, const Path &targetPath, StatusTracker &tracker) const
{
	ProcessStatus& process = tracker.AddProcessStatusTracker(u8"Restoring snapshot: " + snapshot.Name(),
	                                                         snapshot.Index().GetNumberOfNodes(), snapshot.Index().ComputeTotalSize());
	for(uint32 i = 0; i < snapshot.Index().GetNumberOfNodes(); i++)
	{
		this->threadPool.EnqueueTask([&snapshot, i, &targetPath, &process]()
		{
			BackupContainerIndex* dataIndex = snapshot.FindDataIndex(i);

			//open files
			const Path& filePath = snapshot.Index().GetNodePath(i);
			Path fileRestorePath = targetPath / filePath;
			Path fileDir = fileRestorePath.GetParent();
			if(!OSFileSystem::GetInstance().Exists(fileDir))
				OSFileSystem::GetInstance().CreateDirectoryTree(fileDir);

			UniquePointer<InputStream> input = dataIndex->OpenFileForReading(filePath);
			FileOutputStream output(fileRestorePath);

			//write
			uint32 flushedSize = input->FlushTo(output);

			process.AddFinishedSize(flushedSize);
			process.IncFinishedCount();
		});
	}

	this->threadPool.WaitForAllTasksToComplete();
}

//Private methods
void BackupManager::AddCompressionRateSample(const String &fileExtension, float32 compressionRate)
{
	AutoLock lock(this->compressionStatsLock);

	compressionRate = Math::Clamp(compressionRate, 0.0f, 1.0f);

	String extLower = fileExtension.ToLowercase();
	this->compressionStats[extLower] = (this->compressionStats[extLower] + compressionRate) / 2.0f;
}

float32 BackupManager::GetCompressionRate(const String &fileExtension)
{
	AutoLock lock(this->compressionStatsLock);

	String extLower = fileExtension.ToLowercase();
	if(!this->compressionStats.Contains(extLower))
		this->compressionStats[extLower] = 0; //assume at first that file is perfectly compressible

	return this->compressionStats[extLower];
}
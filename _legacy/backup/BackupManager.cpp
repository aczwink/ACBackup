
//Local
#include "../_legacy/FlatContainerIndex.hpp"
#include "../KeyDerivation.hpp"
#include "../KeyVerificationFailedException.hpp"
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

//Destructor
BackupManager::~BackupManager()
{
	this->DropSnapshots();
}

//Public methods
void BackupManager::AddSnapshot(const FileSystemNodeIndex &index, StatusTracker& tracker)
{
	for(uint32 i = 0; i < index.GetNumberOfNodes(); i++)
	{
		if(diffNodeIndices.Contains(i))
		{
			const int8 maxCompressionLevel = this->config.MaxCompressionLevel();
			const uint64 memLimit = this->config.CompressionMemoryLimit();

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

						process.AddFinishedSize(fileAttributes.Size());
					}
					break;
					case IndexableNodeType::Link:
						snapshot->BackupLink(filePath);
						break;
				}
			});
		}
	}

	NOT_IMPLEMENTED_ERROR; //TODO: WHAT FOLLOWS IS LEGACY CODE

	/*
	snapshot->Serialize(this->encryptionInfo);

	//close snapshot and read it in again
	snapshot = nullptr;
	this->DropSnapshots();

	//deserialize and verify
	this->ReadInSnapshots();
	this->VerifySnapshot(this->snapshots.Last(), tracker);
	*/
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

void BackupManager::VerifySnapshot(const Snapshot &snapshot, StatusTracker& tracker) const
{
	const BackupContainerIndex& index = snapshot.Index();

	DynamicArray<uint32> failedFiles;
	Mutex failedFilesLock;

	ProcessStatus& process = tracker.AddProcessStatusTracker(u8"Verifying snapshot: " + snapshot.Name(),
	                                                         index.GetNumberOfNodes(), index.ComputeTotalSize());
	for(uint32 i = 0; i < index.GetNumberOfNodes(); i++)
	{
		this->threadPool.EnqueueTask([this, &snapshot, i, &process, &failedFiles, &failedFilesLock](){
			BackupContainerIndex* dataIndex = snapshot.FindDataIndex(i);

			//compute digest
			UniquePointer<InputStream> input = dataIndex->OpenFileForReading(snapshot.Index().GetNodePath(i));
			UniquePointer<Crypto::HashFunction> hasher = Crypto::HashFunction::CreateInstance(Crypto::HashAlgorithm::MD5);
			uint64 readSize = hasher->Update(*input);

			hasher->Finish();
			String digestString = hasher->GetDigestString().ToLowercase();

			//checks
			const FileSystemNodeAttributes& fileAttributes = dataIndex->GetNodeAttributes(i);
			if((readSize != fileAttributes.Size()) || (fileAttributes.GetDigest(config.HashAlgorithm()) != digestString))
			{
				failedFilesLock.Lock();
				failedFiles.Push(i);
				failedFilesLock.Unlock();
				return;
			}

			process.AddFinishedSize(fileAttributes.Size());
			process.IncFinishedCount();
		});
	}

	this->threadPool.WaitForAllTasksToComplete();

	if(failedFiles.IsEmpty())
		process.Finished();
	else
	{
		for(uint32 fileIndex : failedFiles)
		{
			const Path& filePath = index.GetNodePath(fileIndex);
			stdErr << u8"File '" << filePath << u8"' is corrupt." << endl;
		}
	}
}

//Class functions
void BackupManager::WriteCompressionStatsFile(const Path &path, const Map<String, float32>& compressionStats)
{
	FileOutputStream compr(path / String(c_comprStatsFileName), true);
	BufferedOutputStream bufferedOutputStream(compr);

	CommonFileFormats::CSVWriter csvWriter(bufferedOutputStream, CommonFileFormats::csvDialect_excel);
	csvWriter << u8"File extension" << u8"Compression rate" << endl;
	for(const auto& kv : compressionStats)
	{
		csvWriter << kv.key << kv.value << endl;
	}

	bufferedOutputStream.Flush();
}

//Private methods
void BackupManager::AddCompressionRateSample(const String &fileExtension, float32 compressionRate)
{
	AutoLock lock(this->compressionStatsLock);

	compressionRate = Math::Clamp(compressionRate, 0.0f, 1.0f);

	String extLower = fileExtension.ToLowercase();
	this->compressionStats[extLower] = (this->compressionStats[extLower] + compressionRate) / 2.0f;
}

void BackupManager::DropSnapshots()
{
	BackupManager::WriteCompressionStatsFile(this->backupPath, this->compressionStats);

	this->snapshots.Release();
}

float32 BackupManager::GetCompressionRate(const String &fileExtension)
{
	AutoLock lock(this->compressionStatsLock);

	String extLower = fileExtension.ToLowercase();
	if(!this->compressionStats.Contains(extLower))
		this->compressionStats[extLower] = 0; //assume at first that file is perfectly compressible

	return this->compressionStats[extLower];
}
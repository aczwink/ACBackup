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
//Class header
#include "BackupManager.hpp"
//Local
#include "FlatContainerIndex.hpp"
#include "KeyDerivation.hpp"
#include "KeyVerificationFailedException.hpp"

const char* c_comprStatsFileName = u8"compression_stats.csv";

//Constructor
BackupManager::BackupManager(const Path &path) : backupPath(path)
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

	this->ReadInSnapshots();

	//read in compression stats
	FileInputStream fileInputStream(this->backupPath / String(c_comprStatsFileName));
	BufferedInputStream bufferedInputStream(fileInputStream);
	TextReader textReader(bufferedInputStream, TextCodecType::UTF8);
	CommonFileFormats::CSVReader csvReader(textReader, CommonFileFormats::csvDialect_excel);

	//skip first line
	String cell;
	csvReader.ReadCell(cell);
	csvReader.ReadCell(cell);

	//read lines
	String rate;
	while(true)
	{
		csvReader.ReadCell(cell);
		bool haveValue = csvReader.ReadCell(rate);

		if(haveValue)
			this->compressionStats[cell] = rate.ToFloat32();
		else
			break;
	}
}

//Destructor
BackupManager::~BackupManager()
{
	this->DropSnapshots();
}

//Public methods
void BackupManager::AddSnapshot(const FileIndex &index, StatusTracker& tracker, const int8 maxCompressionLevel, const uint64 memLimit)
{
	DateTime dateTime = DateTime::Now();
	String snapshotName = u8"snapshot_" + dateTime.GetDate().ToISOString() + u8"_";
	snapshotName += String::Number(dateTime.GetTime().GetHour(), 10, 2) + u8"_" + String::Number(dateTime.GetTime().GetMinute(), 10, 2) + u8"_" + String::Number(dateTime.GetTime().GetSecond(), 10, 2);

	//compute total size
	uint64 totalSize = 0;
	for(uint32 i = 0; i < index.GetNumberOfFiles(); i++)
		totalSize += index.GetFileAttributes(i).size;

	//find previous snapshot
	const FileIndex* prevSnapshot = nullptr;
	if(!this->snapshots.IsEmpty())
		prevSnapshot = this->snapshots.Last().index;

	UniquePointer<FlatContainerIndex> snapshot = new FlatContainerIndex(this->backupPath / snapshotName, this->encryptionInfo);
	ProcessStatus& process = tracker.AddProcessStatusTracker(u8"Creating snapshot: " + snapshotName, index.GetNumberOfFiles(), totalSize);
	for(uint32 i = 0; i < index.GetNumberOfFiles(); i++)
	{
		this->threadPool.EnqueueTask([i, &index, prevSnapshot, &snapshot, &process, this, maxCompressionLevel, memLimit](){
			const Path& filePath = index.GetFile(i);
			const FileAttributes& fileAttributes = index.GetFileAttributes(i);

			uint32 prevIndex = prevSnapshot ? prevSnapshot->FindFileIndex(filePath) : Unsigned<uint32>::Max();
			bool backupFile = false;
			if(prevIndex != Unsigned<uint32>::Max())
			{
				const FileAttributes& prevFileAttributes = prevSnapshot->GetFileAttributes(prevIndex);

				//this file is known
				bool didChange = !((fileAttributes.size == prevFileAttributes.size) && (MemCmp(fileAttributes.digest, prevFileAttributes.digest, sizeof(prevFileAttributes.digest)) == 0));
				if(didChange)
					backupFile = true; //back it up again
				else
				{
					snapshot->AddPreviousFile(filePath, index); //don't back it up again, but make a hint that a previous snapshot has it
				}
			}
			else
			{
				backupFile = true; //new file
			}

			if(backupFile)
			{
				String ext = filePath.GetFileExtension();
				float32 compressionRate = this->GetCompressionRate(ext);
				compressionRate = snapshot->BackupFile(filePath, index, compressionRate, maxCompressionLevel, memLimit);
				this->AddCompressionRateSample(ext, compressionRate);

				process.AddFinishedSize(fileAttributes.size);
			}
			else
			{
				process.ReduceTotalSize(fileAttributes.size);
			}
			process.IncFinishedCount();
		});
	}
	this->threadPool.WaitForAllTasksToComplete();
	process.Finished();

	snapshot->Serialize(this->encryptionInfo);

	//close snapshot and read it in again
	snapshot = nullptr;
	this->DropSnapshots();

	//deserialize and verify
	this->ReadInSnapshots();
	this->VerifySnapshot(this->snapshots.Last(), tracker);
}

void BackupManager::VerifySnapshot(const Snapshot &snapshot, StatusTracker& tracker) const
{
	BackupContainerIndex* index = snapshot.index;

	//compute total size
	uint64 totalSize = 0;
	for(uint32 i = 0; i < index->GetNumberOfFiles(); i++)
	{
		totalSize += index->GetFileAttributes(i).size;
	}

	DynamicArray<uint32> failedFiles;
	Mutex failedFilesLock;

	ProcessStatus& process = tracker.AddProcessStatusTracker(u8"Verifying snapshot: " + snapshot.creationText, index->GetNumberOfFiles(), totalSize);
	for(uint32 i = 0; i < index->GetNumberOfFiles(); i++)
	{
		this->threadPool.EnqueueTask([&snapshot, i, &process, &failedFiles, &failedFilesLock](){
			//find data first
			uint32 fileIndex = i;
			const Path& filePath = snapshot.index->GetFile(i);
			const Snapshot* dataSnapshot = &snapshot;
			while(!dataSnapshot->index->HasFileData(fileIndex))
			{
				dataSnapshot = dataSnapshot->prev;
				fileIndex = dataSnapshot->index->FindFileIndex(filePath);
			}
			BackupContainerIndex* dataIndex = dataSnapshot->index;

			//compute digest
			UniquePointer<InputStream> input = dataIndex->OpenFileForReading(filePath);
			UniquePointer<Crypto::HashFunction> hasher = Crypto::HashFunction::CreateInstance(Crypto::HashAlgorithm::MD5);
			uint64 readSize = hasher->Update(*input);

			hasher->Finish();
			byte computedDigest[16];
			hasher->StoreDigest(computedDigest);

			//checks
			const FileAttributes& fileAttributes = dataIndex->GetFileAttributes(i);
			if((readSize != fileAttributes.size) || (MemCmp(computedDigest, fileAttributes.digest, sizeof(fileAttributes.digest)) != 0))
			{
				failedFilesLock.Lock();
				failedFiles.Push(i);
				failedFilesLock.Unlock();
				return;
			}

			process.AddFinishedSize(fileAttributes.size);
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
			const Path& filePath = index->GetFile(fileIndex);
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

	for(auto& snapshot : this->snapshots)
		delete snapshot.index;
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

void BackupManager::ReadInSnapshots()
{
	//read in snapshots
	DynamicArray<Path> snapshotFiles;

	auto dir = OSFileSystem::GetInstance().GetDirectory(this->backupPath);
	auto dirWalker = dir->WalkFiles();

	for(Tuple<Path, AutoPointer<File>> kv : dirWalker)
	{
		snapshotFiles.Push(kv.Get<0>());
	}
	snapshotFiles.Sort();

	for(const Path& ssfile : snapshotFiles)
	{
		//try to deserialize
		Snapshot snapshot = BackupContainerIndex::Deserialize(this->backupPath / ssfile, this->encryptionInfo);
		if(snapshot.index)
		{
			this->snapshots.Push(snapshot);
			if(this->snapshots.GetNumberOfElements() > 1)
				this->snapshots.Last().prev = &this->snapshots[this->snapshots.GetNumberOfElements()-2];
		}
	}
}
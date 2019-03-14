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

const char* c_comprStatsFileName = u8"compression_stats.csv";

//Constructor
BackupManager::BackupManager(const Path &path) : backupPath(path)
{
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
void BackupManager::AddSnapshot(const FileIndex &index, StatusTracker& tracker)
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

	UniquePointer<FlatContainerIndex> snapshot = new FlatContainerIndex(this->backupPath / snapshotName);
	ProcessStatus& process = tracker.AddProcessStatusTracker(u8"Creating snapshot: " + snapshotName, index.GetNumberOfFiles(), totalSize);
	for(uint32 i = 0; i < index.GetNumberOfFiles(); i++)
	{
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
			Clock c;
			c.Start();

			String ext = filePath.GetFileExtension();
			float32 compressionRate = this->GetCompressionRate(ext);
			compressionRate = snapshot->BackupFile(filePath, index, compressionRate);
			this->AddCompressionRateSample(ext, compressionRate);

			process.AddFinishedSize(fileAttributes.size, c.GetElapsedMicroseconds());
		}
		else
		{
			process.ReduceTotalSize(fileAttributes.size);
		}
		process.IncFinishedCount();
	}
	process.Finished();

	snapshot->Serialize();

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
	uint32 nFilesToCheck = 0;
	uint64 totalSize = 0;
	for(uint32 i = 0; i < index->GetNumberOfFiles(); i++)
	{
		if(!index->HasFileData(i))
			continue; //file data is in a previous snapshot
		nFilesToCheck++;
		totalSize += index->GetFileAttributes(i).size;
	}

	ProcessStatus& process = tracker.AddProcessStatusTracker(u8"Verifying snapshot: " + snapshot.creationText, nFilesToCheck, totalSize);

	for(uint32 i = 0; i < index->GetNumberOfFiles(); i++)
	{
		if(!index->HasFileData(i))
			continue; //file data is in a previous snapshot

		this->threadPool.EnqueueTask([&index, i, &process](){
			const Path& filePath = index->GetFile(i);
			const FileAttributes& fileAttributes = index->GetFileAttributes(i);

			Clock c;
			c.Start();
			//compute digest
			UniquePointer<InputStream> input = index->OpenFileForReading(filePath);
			UniquePointer<Crypto::HashFunction> hasher = Crypto::HashFunction::CreateInstance(Crypto::HashAlgorithm::MD5);
			uint64 readSize = hasher->Update(*input);

			hasher->Finish();
			byte computedDigest[16];
			hasher->StoreDigest(computedDigest);

			//checks
			if((readSize != fileAttributes.size) || (MemCmp(computedDigest, fileAttributes.digest, sizeof(fileAttributes.digest)) != 0))
			{
				NOT_IMPLEMENTED_ERROR; //TODO: ERROR
			}

			process.AddFinishedSize(fileAttributes.size, c.GetElapsedMicroseconds());
			process.IncFinishedCount();
		});
	}

	this->threadPool.WaitForAllTasksToComplete();
	process.Finished();
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
		Snapshot snapshot = BackupContainerIndex::Deserialize(this->backupPath / ssfile);
		if(snapshot.index)
		{
			this->snapshots.Push(snapshot);
		}
	}
}
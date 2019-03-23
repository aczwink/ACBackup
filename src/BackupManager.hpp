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
#include "BackupContainerIndex.hpp"
#include "StatusTracker.hpp"

class BackupManager
{
public:
	//Constructor
	BackupManager(const Path& path);

	//Destructor
	~BackupManager();

	//Methods
	void AddSnapshot(const FileIndex& index, StatusTracker& tracker, int8 maxCompressionLevel, uint64 memLimit);
	void RestoreSnapshot(const Snapshot& snapshot, const Path& targetPath, StatusTracker& tracker) const;
	void VerifySnapshot(const Snapshot& snapshot, StatusTracker& tracker) const;

	//Functions
	static void WriteCompressionStatsFile(const Path& path, const Map<String, float32>& compressionStats);

	//Inline
	inline bool RestoreSnapshot(const String& snapshotName, const Path& targetPath, StatusTracker& tracker) const
	{
		for(const Snapshot& snapshot : this->snapshots)
		{
			if(snapshot.creationText == snapshotName)
			{
				this->RestoreSnapshot(snapshot, targetPath, tracker);
				return true;
			}
		}
		return false;
	}

	inline bool VerifySnapshot(const String& snapshotName, StatusTracker& tracker) const
	{
		for(const Snapshot& snapshot : this->snapshots)
		{
			if(snapshot.creationText == snapshotName)
			{
				this->VerifySnapshot(snapshot, tracker);
				return true;
			}
		}
		return false;
	}

private:
	//Members
	Path backupPath;
	DynamicArray<Snapshot> snapshots;
	Map<String, float32> compressionStats;
	Mutex compressionStatsLock;
	Optional<EncryptionInfo> encryptionInfo;
	mutable StaticThreadPool threadPool;

	//Methods
	void AddCompressionRateSample(const String& fileExtension, float32 compressionRate);
	void DropSnapshots();
	float32 GetCompressionRate(const String& fileExtension);
	void ReadInSnapshots();
};

//Local
#include "../BackupContainerIndex.hpp"
#include "../StatusTracker.hpp"

class BackupManager
{
public:
	//Destructor
	~BackupManager();
	
	void RestoreSnapshot(const Snapshot& snapshot, const Path& targetPath, StatusTracker& tracker) const;
	void VerifySnapshot(const Snapshot& snapshot, StatusTracker& tracker) const;

	//Functions
	static void WriteCompressionStatsFile(const Path& path, const Map<String, float32>& compressionStats);

	//Inline
	inline bool RestoreSnapshot(const String& snapshotName, const Path& targetPath, StatusTracker& tracker) const
	{
		for(const UniquePointer<Snapshot>& snapshot : this->snapshots)
		{
			if(snapshot->Name() == snapshotName)
			{
				this->RestoreSnapshot(*snapshot, targetPath, tracker);
				return true;
			}
		}
		return false;
	}

	inline bool VerifySnapshot(const String& snapshotName, StatusTracker& tracker) const
	{
		for(const UniquePointer<Snapshot>& snapshot : this->snapshots)
		{
			if(snapshot->Name() == snapshotName)
			{
				this->VerifySnapshot(*snapshot, tracker);
				return true;
			}
		}
		return false;
	}

private:
	//Members
	const Config& config;
	Map<String, float32> compressionStats;
	Mutex compressionStatsLock;
	Optional<EncryptionInfo> encryptionInfo;

	void AddCompressionRateSample(const String& fileExtension, float32 compressionRate);
	void DropSnapshots();
	float32 GetCompressionRate(const String& fileExtension);

	//Inline
	inline BinaryTreeSet<uint32> ComputeDifference(const FileSystemNodeIndex& index)
	{
		if(this->LastIndex())
			return index.ComputeDifference(*this->LastIndex(), this->threadPool, tracker);
	}
};
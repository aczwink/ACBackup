
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
	Mutex compressionStatsLock;
	Optional<EncryptionInfo> encryptionInfo;

	void AddCompressionRateSample(const String& fileExtension, float32 compressionRate);
	float32 GetCompressionRate(const String& fileExtension);

	//Inline
	inline BinaryTreeSet<uint32> ComputeDifference(const FileSystemNodeIndex& index)
	{
		if(this->LastIndex())
			return index.ComputeDifference(*this->LastIndex(), this->threadPool, tracker);
	}
};

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

private:
	//Members
	Mutex compressionStatsLock;
	Optional<EncryptionInfo> encryptionInfo;

	void AddCompressionRateSample(const String& fileExtension, float32 compressionRate);
	float32 GetCompressionRate(const String& fileExtension);
};
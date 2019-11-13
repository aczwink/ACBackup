//Local
#include "../BackupContainerIndex.hpp"

class Snapshot
{
public:
	//Methods
	/**
	 * Find the index that has the payload data of the file identified by index of this snapshot.
	 * @param fileIndex
	 * @return
	 */
	BackupContainerIndex* FindDataIndex(uint32 fileIndex) const;

	//Inline
	inline float32 BackupFile(const Path& path, const FileSystemNodeIndex& index, float32& compressionRate, int8 maxCompressionLevel, uint64 memLmit)
	{
		return this->index->BackupFile(path, index, compressionRate, maxCompressionLevel, memLmit);
	}

	inline void BackupLink(const Path& path)
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
	}
};
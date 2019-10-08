
//Local
#include "FileSystemNodeIndex.hpp"
#include "../StatusTracker.hpp"
#include "FileSystemNodeAttributes.hpp"

class OSFileSystemNodeIndex : public FileSystemNodeIndex
{
public:
	//Methods
	uint32 FindNodeIndex(const Path &path) const override;
	const Path &GetNodePath(uint32 index) const override;
	const FileSystemNodeAttributes &GetNodeAttributes(uint32 index) const override;
	uint32 GetNumberOfNodes() const override;
	UniquePointer<InputStream> OpenFileForReading(const Path &fileEntry) const override;

	//Members
	StatusTracker& tracker;
	BijectiveMap<Path, uint32> pathMap;
	DynamicArray<UniquePointer<FileSystemNodeAttributes>> nodeAttributes;
	StaticThreadPool threadPool;
};
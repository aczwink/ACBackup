
//Local
#include "FileSystemNodeIndex.hpp"
#include "../StatusTracker.hpp"
#include "FileSystemNodeAttributes.hpp"

class OSFileSystemNodeIndex : public FileSystemNodeIndex
{
public:
	//Methods
	uint32 FindNodeIndex(const Path &path) const override;
	UniquePointer<InputStream> OpenFileForReading(const Path &fileEntry) const override;


	StatusTracker& tracker;
	StaticThreadPool threadPool;
};
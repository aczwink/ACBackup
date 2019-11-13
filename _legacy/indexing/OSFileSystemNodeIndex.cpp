//Local
#include "IndexingReadException.hpp"
#include "FileSystemNodeAttributes.hpp"
#include "../../src/indexing/OSFileSystemNodeIndex.hpp"


//Constructor
OSFileSystemNodeIndex::OSFileSystemNodeIndex(const Path &path, StatusTracker& tracker, const Config& config) :, tracker(tracker)
{
	//compute attributes
	/*
	Atomic<bool> failed = false;
	ProcessStatus& attrStatus = tracker.AddProcessStatusTracker(u8"Computing hashes", this->nodeAttributes.GetNumberOfElements(), totalSize);
	for(uint32 i = 0; i < this->nodeAttributes.GetNumberOfElements(); i++)
	{
		this->threadPool.EnqueueTask([this, i, &path, &attrStatus, &failed](){
			if(*failed)
				return;

			Path filePath = path / this->pathMap.GetReverse(i);
			AutoPointer<const File> file = OSFileSystem::GetInstance().GetFile(filePath);

			//compute digest
			UniquePointer<InputStream> input = file->OpenForReading(false);
			UniquePointer<Crypto::HashFunction> hasher = Crypto::HashFunction::CreateInstance(Crypto::HashAlgorithm::MD5);
			uint64 hashedSize = hasher->Update(*input);
			hasher->Finish();

			UniquePointer<FileSystemNodeAttributes>& attrs = this->nodeAttributes[i];
			if(hashedSize != attrs->Size())
				failed = true;

			hasher->StoreDigest(attrs->digest);

			attrStatus.IncFinishedCount();
			attrStatus.AddFinishedSize(hashedSize);
		});
	}
	this->threadPool.WaitForAllTasksToComplete();

	if(*failed)
		throw IndexingReadException();

	attrStatus.Finished();
	 */
}

//Public methods
uint32 OSFileSystemNodeIndex::FindNodeIndex(const Path &path) const
{
	if(this->pathMap.Contains(path))
		return this->pathMap.Get(path);
	return Unsigned<uint32>::Max();
}

UniquePointer<InputStream> OSFileSystemNodeIndex::OpenFileForReading(const Path &fileEntry) const
{
	return new FileInputStream(this->basePath / fileEntry);
}

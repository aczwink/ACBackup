#pragma once
#include <Std++.hpp>
using namespace StdXX;
#include "../StatusTracker.hpp"

class FileSystemNodeIndex
{
public:
	//Destructor
	virtual ~FileSystemNodeIndex() {}

	//Abstract
	virtual uint32 FindNodeIndex(const Path& path) const = 0;
	virtual const Path& GetNodePath(uint32 index) const = 0;
	virtual const FileSystemNodeAttributes& GetNodeAttributes(uint32 index) const = 0;
	virtual uint32 GetNumberOfNodes() const = 0;
	virtual UniquePointer<InputStream> OpenFileForReading(const Path& fileEntry) const = 0;

	//Methods
	/**
	 * Returns the indices from this index that are different from other
	 * @param comparer
	 * @return
	 */
	BinaryTreeSet<uint32> ComputeDifference(const FileSystemNodeIndex& other, StaticThreadPool& threadPool, StatusTracker& tracker) const;
	/**
	 * No compression or filtering whatsoever.
	 * The raw user data file size.
	 * @return
	 */
	uint64 ComputeTotalSize() const;
};
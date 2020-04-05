#pragma once
#include <Std++.hpp>
using namespace StdXX;
#include "../StatusTracker.hpp"

class FileSystemNodeIndex
{
public:
	//Destructor
	virtual ~FileSystemNodeIndex() {}

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
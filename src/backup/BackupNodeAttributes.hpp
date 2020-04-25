/*
 * Copyright (c) 2019-2020 Amir Czwink (amir130@hotmail.de)
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
#pragma once
//Local
#include "../indexing/FileSystemNodeAttributes.hpp"
#include "../CompressionSetting.hpp"
//Namespaces
using namespace StdXX::FileSystem;

struct Block
{
	uint64 volumeNumber;
	uint64 offset;
	uint64 size;
};

class BackupNodeAttributes : public FileSystemNodeAttributes
{
public:
	//Constructors
	inline BackupNodeAttributes(NodeType type, uint64 size, const Optional<DateTime>& lastModifiedTime, DynamicArray<Block>&& blocks, Map<Crypto::HashAlgorithm, String>&& hashes)
		: FileSystemNodeAttributes(type, size, lastModifiedTime),
		blocks(Forward<DynamicArray<Block>>(blocks)), hashes(Forward<Map<Crypto::HashAlgorithm, String>>(hashes))
	{
	}

	inline BackupNodeAttributes(const FileSystemNodeAttributes& attributes) : FileSystemNodeAttributes(attributes)
	{
	}

	BackupNodeAttributes(const BackupNodeAttributes& attributes) = default;

	//Properties
	inline const Optional<Path>& BackReferenceTarget() const
	{
		return this->backReferenceTarget;
	}

	inline void BackReferenceTarget(const Optional<Path>& backreferenceTarget)
	{
		this->backReferenceTarget = backreferenceTarget;
	}

	inline const DynamicArray<Block>& Blocks() const
	{
		return this->blocks;
	}

	inline const Optional<enum CompressionSetting>& CompressionSetting() const
	{
		return this->compressionSetting;
	}

	inline void CompressionSetting(const Optional<enum CompressionSetting>& compressionSetting)
	{
		this->compressionSetting = compressionSetting;
	}

	inline const String& Hash(Crypto::HashAlgorithm hashAlgorithm) const
	{
		return this->hashes[hashAlgorithm];
	}

	inline const Map<Crypto::HashAlgorithm, String>& HashValues() const
	{
		return this->hashes;
	}

	inline bool OwnsBlocks() const
	{
		return this->ownsBlocks;
	}

	inline void OwnsBlocks(bool value)
	{
		this->ownsBlocks = value;
	}

	//Methods
	void AddBlock(const Block& block);
	uint64 ComputeSumOfBlockSizes() const;

	//Inline
	inline void AddHashValue(Crypto::HashAlgorithm hashAlgorithm, const String& hashValue)
	{
		if(this->hashes.Contains(hashAlgorithm))
			ASSERT_EQUALS(this->hashes[hashAlgorithm], hashValue);
		this->hashes[hashAlgorithm] = hashValue;
	}

private:
	//Members
	bool ownsBlocks;
	Optional<enum CompressionSetting> compressionSetting;
	Optional<Path> backReferenceTarget;
	DynamicArray<Block> blocks;
	Map<Crypto::HashAlgorithm, String> hashes;
};
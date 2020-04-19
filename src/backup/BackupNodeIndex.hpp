/*
 * Copyright (c) 2020 Amir Czwink (amir130@hotmail.de)
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
#include "../indexing/FileSystemNodeIndex.hpp"
#include "BackupNodeAttributes.hpp"

class BackupNodeIndex : public FileSystemNodeIndex
{
public:
	//Constructors
	BackupNodeIndex() = default;
	BackupNodeIndex(StdXX::Serialization::XmlDeserializer& xmlDeserializer);

	//Methods
	uint64 ComputeSumOfBlockSizes() const;
	uint32 FindNodeIndexByHash(const String& hash) const;
	FileSystemNodeInfo GetFileSystemNodeInfo(uint32 nodeIndex) const;
	void Serialize(Serialization::XmlSerializer& xmlSerializer) const;

	//Properties
	inline const DynamicArray<uint32> ChildrenOf(uint32 directoryIndex) const
	{
		return this->nodeChildren[directoryIndex];
	}

	//Inline
	inline void AddBlock(const Path& path, uint64 volumeNumber, uint64 offset, uint64 size)
	{
		uint32 nodeIndex = this->GetNodeIndex(path);
		BackupNodeAttributes& attributes = this->GetNodeAttributes(nodeIndex);
		attributes.AddBlock({ .volumeNumber =  volumeNumber, .offset = offset, .size = size });
	}

	inline BackupNodeAttributes& GetNodeAttributes(uint32 index)
	{
		return (BackupNodeAttributes&)*this->nodeAttributes[index];
	}

	inline const BackupNodeAttributes& GetNodeAttributes(uint32 index) const
	{
		return (BackupNodeAttributes&)*this->nodeAttributes[index];
	}

	inline bool HasNodeData(uint32 index) const
	{
		const BackupNodeAttributes &attributes = this->GetNodeAttributes(index);
		return attributes.OwnsBlocks();
	}

private:
	//Members
	Map<uint32, DynamicArray<uint32>> nodeChildren;
	Map<String, uint32> hashIndex;

	//Methods
	void ComputeNodeChildren();
	DynamicArray<Block> DeserializeBlocks(StdXX::Serialization::XmlDeserializer& xmlDeserializer, bool& ownsBlocks);
	Map<Crypto::HashAlgorithm, String> DeserializeHashes(StdXX::Serialization::XmlDeserializer& xmlDeserializer);
	void GenerateHashIndex();
	void DeserializeNode(StdXX::Serialization::XmlDeserializer& xmlDeserializer);
	void SerializeBlocks(Serialization::XmlSerializer& xmlSerializer, const DynamicArray<Block>& blocks, bool ownsBlocks) const;
	void SerializeHashes(Serialization::XmlSerializer& xmlSerializer, const Map<Crypto::HashAlgorithm, String>& hashes) const;
	void SerializeNode(Serialization::XmlSerializer& xmlSerializer, const Path &path, const BackupNodeAttributes& attributes) const;
};
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
	NodeInfo GetFileSystemNodeInfo(uint32 nodeIndex) const;
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
		BackupNodeAttributes& attributes = this->GetChangeableNodeAttributes(nodeIndex);
		attributes.AddBlock({ .volumeNumber =  volumeNumber, .offset = offset, .size = size });
	}

	inline const BackupNodeAttributes& GetNodeAttributes(uint32 index) const
	{
        return (BackupNodeAttributes&)FileSystemNodeIndex::GetNodeAttributes(index);
	}

	inline bool HasNodeData(uint32 index) const
	{
		const BackupNodeAttributes &attributes = this->GetNodeAttributes(index);
		return attributes.OwnsBlocks() or (attributes.Size() == 0);
	}

private:
	//Members
	Map<uint32, DynamicArray<uint32>> nodeChildren;
	Map<String, uint32> hashIndex;

	//Methods
	void ComputeNodeChildren();
	DynamicArray<Block> DeserializeBlocks(StdXX::Serialization::XmlDeserializer& xmlDeserializer, bool& ownsBlocks, Optional<enum CompressionSetting>& compressionSetting, Optional<Path>& owner);
	Map<Crypto::HashAlgorithm, String> DeserializeHashes(StdXX::Serialization::XmlDeserializer& xmlDeserializer);
	void DeserializeNode(StdXX::Serialization::XmlDeserializer& xmlDeserializer);
	UniquePointer<NodePermissions> DeserializePermissions(StdXX::Serialization::XmlDeserializer& xmlDeserializer);
    void GenerateHashIndex();
	void SerializeBlocks(Serialization::XmlSerializer& xmlSerializer, const DynamicArray<Block>& blocks, bool ownsBlocks, Optional<CompressionSetting>& compressionSetting, Optional<Path>& owner) const;
	void SerializeHashes(Serialization::XmlSerializer& xmlSerializer, const Map<Crypto::HashAlgorithm, String>& hashes) const;
	void SerializeNode(Serialization::XmlSerializer& xmlSerializer, const Path &path, const BackupNodeAttributes& attributes) const;
	void SerializePermissions(Serialization::XmlSerializer& xmlSerializer, const NodePermissions& nodePermissions) const;

	//Inline
    inline BackupNodeAttributes& GetChangeableNodeAttributes(uint32 index)
    {
        return (BackupNodeAttributes&)FileSystemNodeIndex::GetNodeAttributes(index);
    }
};
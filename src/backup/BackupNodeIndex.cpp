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
//Class header
#include "BackupNodeIndex.hpp"
//Local
#include "../config/ConfigManager.hpp"
#include "../InjectionContainer.hpp"
#include "../Serialization.hpp"
//Namespaces
using namespace StdXX::CommonFileFormats;
using namespace StdXX::Serialization;

//Constants
static const char *const c_tag_node_name = u8"Node";
static const char *const c_tag_node_attribute_type_name = u8"type";
static const char *const c_tag_node_blocks_name = u8"Blocks";
const char* const c_tag_node_blocks_attribute_owned_name = u8"owned";
const char* const c_tag_node_blocks_attribute_owner_name = u8"owner";
const char* const c_tag_node_blocks_attribute_compression_name = u8"compression";
static const char *const c_tag_node_blocks_block_name = u8"Block";
static const char *const c_tag_node_blocks_block_attribute_offset = u8"offset";
static const char *const c_tag_node_blocks_block_attribute_size = u8"size";
static const char *const c_tag_node_blocks_block_attribute_volumeNumber = u8"volumeNumber";
static const char *const c_tag_node_lastModified_name = u8"LastModified";
static const char *const c_tag_node_size_name = u8"Size";

static const char *const c_tag_nodes_name = u8"Nodes";
static const char *const c_tag_path_name = u8"Path";

static const char *const c_tag_snapshotIndex_name = u8"SnapshotIndex";
static const char *const c_tag_node_hashValues_name = u8"HashValues";

namespace StdXX::Serialization
{
	inline void operator<<(XmlSerializer& serializer, const DateTime& value)
	{
		serializer << value.ToISOString();
	}

	template <typename ArchiveType>
	void CustomArchive(ArchiveType& ar, Block& block)
	{
		ar.EnterElement(c_tag_node_blocks_block_name);
		ar.EnterAttributes();

		ar & Binding(c_tag_node_blocks_block_attribute_volumeNumber, block.volumeNumber);
		ar & Binding(c_tag_node_blocks_block_attribute_offset, block.offset);
		ar & Binding(c_tag_node_blocks_block_attribute_size, block.size);

		ar.LeaveAttributes();
		ar.LeaveElement();
	}

	template <typename ArchiveType>
	void CustomArchive(ArchiveType& ar, Crypto::HashAlgorithm& hashAlgorithm, String& hashValue)
	{
		ar.EnterElement(u8"Hash");
		ar.EnterAttributes();

		CustomArchive(ar, u8"algorithm", hashAlgorithm);

		ar & Binding(u8"value", hashValue);

		ar.LeaveAttributes();
		ar.LeaveElement();
	}

	template <typename ArchiveType>
	void CustomArchive(ArchiveType& ar, const String& name, NodeType& fileSystemNodeType)
	{
		StaticArray<Tuple<NodeType, String>, 3> fileSystemNodeTypeMapping = { {
			{ NodeType::Directory, u8"directory"},
			{ NodeType::File, u8"file"},
			{ NodeType::Link, u8"link"},

		} };
		ar & Binding(name, StringMapping(fileSystemNodeType, fileSystemNodeTypeMapping));
	}
}

//Constructor
BackupNodeIndex::BackupNodeIndex(XmlDeserializer &xmlDeserializer)
{
	xmlDeserializer.EnterElement(c_tag_snapshotIndex_name);
	xmlDeserializer.EnterElement(c_tag_nodes_name);
	while(xmlDeserializer.MoreChildrenExistsAtCurrentLevel())
	{
		xmlDeserializer.EnterElement(c_tag_node_name);
		this->DeserializeNode(xmlDeserializer);
		xmlDeserializer.LeaveElement();
	}
	xmlDeserializer.LeaveElement();
	xmlDeserializer.LeaveElement();

	this->ComputeNodeChildren();
	this->GenerateHashIndex();
}

//Public methods
uint64 BackupNodeIndex::ComputeSumOfBlockSizes() const
{
	uint64 sum = 0;
	for(uint32 i = 0; i < this->GetNumberOfNodes(); i++)
	{
		const BackupNodeAttributes& attributes = this->GetNodeAttributes(i);
		for(const Block& block : attributes.Blocks())
			sum += block.size;
	}
	return sum;
}

uint32 BackupNodeIndex::FindNodeIndexByHash(const String& hash) const
{
	if(this->hashIndex.Contains(hash))
		return this->hashIndex[hash];
	return Unsigned<uint32>::Max();
}

NodeInfo BackupNodeIndex::GetFileSystemNodeInfo(uint32 nodeIndex) const
{
	const BackupNodeAttributes& attributes = this->GetNodeAttributes(nodeIndex);

	NodeInfo fileSystemNodeInfo;
	fileSystemNodeInfo.size = attributes.Size();
	fileSystemNodeInfo.storedSize = attributes.ComputeSumOfBlockSizes();
	fileSystemNodeInfo.lastModifiedTime = attributes.LastModifiedTime();

	return fileSystemNodeInfo;
}

void BackupNodeIndex::Serialize(XmlSerializer& xmlSerializer) const
{
	xmlSerializer.EnterElement(c_tag_snapshotIndex_name);
	xmlSerializer.EnterElement(c_tag_nodes_name);

	for(uint32 i = 0; i < this->GetNumberOfNodes(); i++)
	{
		this->SerializeNode(xmlSerializer, this->GetNodePath(i), this->GetNodeAttributes(i));
	}

	xmlSerializer.LeaveElement();
	xmlSerializer.LeaveElement();
}

//Private methods
void BackupNodeIndex::ComputeNodeChildren()
{
	for(uint32 i = 0; i < this->GetNumberOfNodes(); i++)
	{
		const Path& path = this->GetNodePath(i);
		if(path.IsRoot())
			continue;
		Path parentPath = path.GetParent();
		if(this->HasNodeIndex(parentPath))
			this->nodeChildren[this->GetNodeIndex(parentPath)].Push(i);
	}
}

DynamicArray<Block> BackupNodeIndex::DeserializeBlocks(XmlDeserializer &xmlDeserializer, bool& ownsBlocks, Optional<enum CompressionSetting>& compressionSetting, Optional<Path>& owner)
{
	if(!xmlDeserializer.HasChildElement(c_tag_node_blocks_name))
		return {};

	xmlDeserializer.EnterElement(c_tag_node_blocks_name);

	xmlDeserializer.EnterAttributes();
	xmlDeserializer & Binding(c_tag_node_blocks_attribute_owned_name, ownsBlocks);
	xmlDeserializer & Binding(c_tag_node_blocks_attribute_owner_name, owner);
	xmlDeserializer & Binding(c_tag_node_blocks_attribute_compression_name, compressionSetting);
	xmlDeserializer.LeaveAttributes();
	
	DynamicArray<Block> blocks;
	while(xmlDeserializer.MoreChildrenExistsAtCurrentLevel())
	{
		Block block;

		CustomArchive(xmlDeserializer, block);

		blocks.Push(block);
	}
	
	xmlDeserializer.LeaveElement();
	
	return blocks;
}

Map<Crypto::HashAlgorithm, String> BackupNodeIndex::DeserializeHashes(Serialization::XmlDeserializer &xmlDeserializer)
{
	if(!xmlDeserializer.HasChildElement(c_tag_node_hashValues_name))
		return {};

	Map<Crypto::HashAlgorithm, String> result;

	xmlDeserializer.EnterElement(c_tag_node_hashValues_name);
	while(xmlDeserializer.MoreChildrenExistsAtCurrentLevel())
	{
		Crypto::HashAlgorithm hashAlgorithm;
		String hashValue;

		CustomArchive(xmlDeserializer, hashAlgorithm, hashValue);

		result[hashAlgorithm] = hashValue;
	}
	xmlDeserializer.LeaveElement();

	return result;
}

void BackupNodeIndex::GenerateHashIndex()
{
	Crypto::HashAlgorithm hashAlgorithm = InjectionContainer::Instance().Get<ConfigManager>().Config().hashAlgorithm;

	for(uint32 i = 0; i < this->GetNumberOfNodes(); i++)
	{
		const BackupNodeAttributes& attributes = this->GetNodeAttributes(i);
		if(attributes.HashValues().Contains(hashAlgorithm))
			this->hashIndex[attributes.Hash(hashAlgorithm)] = i;
	}
}

void BackupNodeIndex::DeserializeNode(XmlDeserializer& xmlDeserializer)
{
	xmlDeserializer.EnterAttributes();

	NodeType type;
	CustomArchive(xmlDeserializer, c_tag_node_attribute_type_name, type);

	xmlDeserializer.LeaveAttributes();

	Path path;
	xmlDeserializer & Binding(c_tag_path_name, path);

	Optional<DateTime> lastModifiedTime;
	if(xmlDeserializer.HasChildElement(c_tag_node_lastModified_name))
	{
		DateTime lastModified{Date::Epoch, Time()}; //initialize randomly
		xmlDeserializer & Binding(c_tag_node_lastModified_name, lastModified);
		lastModifiedTime = lastModified;
	}
	
	uint64 size = 0;
	if(xmlDeserializer.HasChildElement(c_tag_node_size_name))
		xmlDeserializer & Binding(c_tag_node_size_name, size);

	bool ownsBlocks = false;
	Optional<CompressionSetting> compressionSetting;
	Optional<Path> owner;
	DynamicArray<Block> blocks = this->DeserializeBlocks(xmlDeserializer, ownsBlocks, compressionSetting, owner);
	Map<Crypto::HashAlgorithm, String> hashes = this->DeserializeHashes(xmlDeserializer);

	UniquePointer<BackupNodeAttributes> attributes = new BackupNodeAttributes(type, size, lastModifiedTime, Move(blocks), Move(hashes));
	attributes->OwnsBlocks(ownsBlocks);
	attributes->CompressionSetting(compressionSetting);
	attributes->BackReferenceTarget(owner);
	this->AddNode(path, Move(attributes));
}

void BackupNodeIndex::SerializeBlocks(Serialization::XmlSerializer& xmlSerializer, const DynamicArray<Block> &blocks, bool ownsBlocks, Optional<CompressionSetting>& compressionSetting, Optional<Path>& owner) const
{
	if(blocks.IsEmpty())
		return;

	xmlSerializer.EnterElement(c_tag_node_blocks_name);
	xmlSerializer.EnterAttributes();
	xmlSerializer & Binding(c_tag_node_blocks_attribute_owned_name, ownsBlocks);
	xmlSerializer & Binding(c_tag_node_blocks_attribute_owner_name, owner);
	xmlSerializer & Binding(c_tag_node_blocks_attribute_compression_name, compressionSetting);
	xmlSerializer.LeaveAttributes();

	for(Block block : blocks)
	{
		CustomArchive(xmlSerializer, block);
	}

	xmlSerializer.LeaveElement();
}

void BackupNodeIndex::SerializeHashes(Serialization::XmlSerializer &xmlSerializer, const Map<Crypto::HashAlgorithm, String> &hashes) const
{
	if(hashes.IsEmpty())
		return;

	xmlSerializer.EnterElement(c_tag_node_hashValues_name);
	for(KeyValuePair<Crypto::HashAlgorithm, String> kv : hashes)
	{
		CustomArchive(xmlSerializer, kv.key, kv.value);
	}
	xmlSerializer.LeaveElement();
}

void BackupNodeIndex::SerializeNode(XmlSerializer& xmlSerializer, const Path &path, const BackupNodeAttributes& attributes) const
{
	xmlSerializer.EnterElement(c_tag_node_name);

	xmlSerializer.EnterAttributes();
	NodeType type = attributes.Type();
	CustomArchive(xmlSerializer, c_tag_node_attribute_type_name, type);
	xmlSerializer.LeaveAttributes();

	xmlSerializer & Binding(c_tag_path_name, path);

	if(attributes.LastModifiedTime().HasValue())
	{
		xmlSerializer & Binding(c_tag_node_lastModified_name, *attributes.LastModifiedTime());
	}

	uint64 size = attributes.Size();
	if(attributes.Type() != NodeType::Directory)
		xmlSerializer & Binding(c_tag_node_size_name, size);

	Optional<CompressionSetting> compressionSetting = attributes.CompressionSetting();
	Optional<Path> backreferenceTarget = attributes.BackReferenceTarget();
	this->SerializeBlocks(xmlSerializer, attributes.Blocks(), attributes.OwnsBlocks(), compressionSetting, backreferenceTarget);
	this->SerializeHashes(xmlSerializer, attributes.HashValues());

	xmlSerializer.LeaveElement();
}
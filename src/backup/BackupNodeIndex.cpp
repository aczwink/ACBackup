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
//Namespaces
using namespace StdXX::CommonFileFormats;
using namespace StdXX::Serialization;

//Constants
static const char *const c_tag_node_name = u8"Node";
static const char *const c_tag_node_type_name = u8"Type";
static const char *const c_tag_node_type_file = u8"file";
static const char *const c_tag_node_type_link = u8"link";
static const char *const c_tag_node_lastModified_name = u8"LastModified";

static const char *const c_tag_nodes_name = u8"Nodes";
static const char *const c_tag_path_name = u8"Path";

static const char *const c_tag_snapshotIndex_name = u8"SnapshotIndex";

namespace StdXX::Serialization
{
	inline void operator>>(XmlDeserializer& deserializer, DateTime& value)
	{
		String tmp;
		deserializer >> tmp;
		value = DateTime::ParseISOString(tmp);
	}

	inline void operator>>(XmlDeserializer& deserializer, Path& value)
	{
		String tmp;
		deserializer >> tmp;
		value = tmp;
	}
}

//Constructor
BackupNodeIndex::BackupNodeIndex(XmlDeserializer &xmlDeserializer)
{
	xmlDeserializer.EnterElement(c_tag_snapshotIndex_name);
	xmlDeserializer.EnterElement(c_tag_nodes_name);
	while(xmlDeserializer.MoreChildrenExistsAtCurrentLevel())
	{
		xmlDeserializer.EnterElement(c_tag_nodes_name);
		this->DeserializeNode(xmlDeserializer);
		xmlDeserializer.LeaveElement();
	}
	xmlDeserializer.LeaveElement();
	xmlDeserializer.LeaveElement();
}

//Public methods
void BackupNodeIndex::Serialize(OutputStream &outputStream) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	/*
	XML::Document document;

	XML::Element* root = new XML::Element(c_tag_snapShotIndex_name);

	XML::Element* nodes = new XML::Element(c_tag_nodes_name);
	for(uint32 i = 0; i < this->GetNumberOfNodes(); i++)
	{
		XML::Element* node = this->SerializeNode(this->GetNodePath(i), this->GetNodeAttributes(i));
		nodes->AppendChild(node);
	}

	root->AppendChild(nodes);

	document.SetRootElement(root);

	BufferedOutputStream bufferedOutputStream(outputStream);
	document.Write(bufferedOutputStream);
	bufferedOutputStream.Flush();
	 */
}

//Private methods
void BackupNodeIndex::DeserializeNode(XmlDeserializer& xmlDeserializer)
{
	String typeString;
	xmlDeserializer & Binding(c_tag_node_type_name, typeString);

	IndexableNodeType type;
	if(typeString == c_tag_node_type_file)
		type = IndexableNodeType::File;
	else if(typeString == c_tag_node_type_link)
		type = IndexableNodeType::Link;
	else
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
	}

	Path path;
	xmlDeserializer & Binding(c_tag_path_name, path);
	
	DateTime lastModified = DateTime::Now();
	xmlDeserializer & Binding(c_tag_node_lastModified_name, lastModified);

	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	//UniquePointer<FileSystemNodeAttributes> attributes = new BackupNodeAttributes(type, );
	//this->AddNode(path, Move(attributes));
}

void BackupNodeIndex::SerializeBlocks(const DynamicArray<Block> &blocks, XML::Element *node) const
{
	if(blocks.IsEmpty())
		return;

	XML::Element* blocksElement = new XML::Element(u8"Blocks");

	for(const Block& block : blocks)
	{
		XML::Element* blockElement = new XML::Element(u8"Block");
		blocksElement->AppendChild(blockElement);

		blockElement->SetAttribute(u8"volumeNumber", String::Number(block.volumeNumber));
		blockElement->SetAttribute(u8"offset", String::Number(block.offset));
		blockElement->SetAttribute(u8"size", String::Number(block.size));
	}

	node->AppendChild(blocksElement);
}

CommonFileFormats::XML::Element *BackupNodeIndex::SerializeNode(const Path &path, const BackupNodeAttributes& attributes) const
{
	XML::Element* node = new XML::Element(c_tag_node_name);

	switch(attributes.Type())
	{
		case IndexableNodeType::File:
			node->SetAttribute(c_tag_node_type_name, c_tag_node_type_file);
			break;
		case IndexableNodeType::Link:
			node->SetAttribute(c_tag_node_type_name, c_tag_node_type_link);
			break;
	}

	XML::Element* pathNode = new XML::Element(c_tag_path_name);
	pathNode->AppendChild(new XML::TextNode(path.GetString()));
	node->AppendChild(pathNode);

	if(attributes.LastModifiedTime().HasValue())
	{
		XML::Element* lastModifiedNode = new XML::Element(c_tag_node_lastModified_name);
		lastModifiedNode->AppendChild(new XML::TextNode(attributes.LastModifiedTime()->ToISOString()));
		node->AppendChild(lastModifiedNode);
	}

	XML::Element* sizeNode = new XML::Element(u8"Size");
	sizeNode->AppendChild(new XML::TextNode(String::Number(attributes.Size())));
	node->AppendChild(sizeNode);

	this->SerializeBlocks(attributes.Blocks(), node);

	return node;
}
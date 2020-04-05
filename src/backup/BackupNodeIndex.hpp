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
	void Serialize(OutputStream& outputStream) const;

	//Inline
	inline void AddBlock(const Path& path, uint64 volumeNumber, uint64 offset, uint64 size)
	{
		uint32 nodeIndex = this->GetNodeIndex(path);
		BackupNodeAttributes& attributes = this->GetNodeAttributes(nodeIndex);
		attributes.AddBlock({ .volumeNumber =  volumeNumber, .offset = offset, .size = size });
	}

private:
	//Methods
	void DeserializeNode(StdXX::Serialization::XmlDeserializer& xmlDeserializer);
	void SerializeBlocks(const DynamicArray<Block>& blocks, CommonFileFormats::XML::Element* node) const;
	CommonFileFormats::XML::Element *SerializeNode(const Path &path, const BackupNodeAttributes& attributes) const;

	//Inline
	inline BackupNodeAttributes& GetNodeAttributes(uint32 index) const
	{
		return (BackupNodeAttributes&)*this->nodeAttributes[index];
	}
};
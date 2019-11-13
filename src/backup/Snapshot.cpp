/*
 * Copyright (c) 2019 Amir Czwink (amir130@hotmail.de)
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
#include "Snapshot.hpp"

//Constructor
Snapshot::Snapshot(const Path& dirPath) : dirPath(dirPath)
{
	DateTime dateTime = DateTime::Now();
	String snapshotName = u8"snapshot_" + dateTime.GetDate().ToISOString() + u8"_";
	snapshotName += String::Number(dateTime.GetTime().GetHour(), 10, 2) + u8"_" + String::Number(dateTime.GetTime().GetMinute(), 10, 2) + u8"_" + String::Number(dateTime.GetTime().GetSecond(), 10, 2);

	this->name = snapshotName;
	this->prev = nullptr;
	this->index = new FileSystemNodeIndex();
	this->fileSystem = FileSystem::Create(FileTypes::UTI::zip, this->dirPath / (snapshotName + u8".zip"));
}

//Public methods
void Snapshot::AddNode(uint32 nodeIndex, const FileSystemNodeIndex &sourceIndex)
{
	const Path& filePath = sourceIndex.GetNodePath(nodeIndex);
	const FileSystemNodeAttributes& fileAttributes = sourceIndex.GetNodeAttributes(nodeIndex);

	this->index->AddNode(filePath, new FileSystemNodeAttributes(fileAttributes));

	switch(fileAttributes.Type())
	{
		case IndexableNodeType::File:
			{
				UniquePointer<OutputStream> fileOutputStream = this->fileSystem->CreateFile(filePath);
				FileInputStream fileInputStream(this->dirPath / filePath);

				fileInputStream.FlushTo(*fileOutputStream);

				NOT_IMPLEMENTED_ERROR; //TODO: implement me
			}
			break;
		case IndexableNodeType::Link:
			NOT_IMPLEMENTED_ERROR; //TODO: implement me
		default:
			RAISE(ErrorHandling::IllegalCodePathError);
	}
}

//Public functions
UniquePointer<Snapshot> Snapshot::Deserialize(const Path &path)
{
	//important! do not assume anything from the file name
	FileInputStream fileInputStream(path);
	BufferedInputStream bufferedInputStream(fileInputStream);
	UniquePointer<XML::Document> document = XML::Document::Parse(bufferedInputStream);

	if(!document.IsNull())
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implementme
	}

	return nullptr;
}

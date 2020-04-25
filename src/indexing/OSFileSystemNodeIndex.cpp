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
//Class header
#include "OSFileSystemNodeIndex.hpp"
//Local
#include "../InjectionContainer.hpp"
#include "../status/StatusTracker.hpp"
#include "LinkPointsOutOfIndexDirException.hpp"
#include "../config/ConfigManager.hpp"
#include "../StreamPipingFailedException.hpp"

//Constructor
OSFileSystemNodeIndex::OSFileSystemNodeIndex(const Path &path) : basePath(path)
{
	this->GenerateIndex();
}

//Public methods
String OSFileSystemNodeIndex::ComputeNodeHash(uint32 nodeIndex) const
{
	const FileSystemNodeAttributes& attributes = this->GetNodeAttributes(nodeIndex);
	ASSERT(attributes.Type() != NodeType::Directory, u8"Can't hash directory");

	const Path &nodePath = this->GetNodePath(nodeIndex);

	UniquePointer<InputStream> inputStream;
	if(attributes.Type() == NodeType::File)
		inputStream = this->OpenFile(nodePath);
	else if(attributes.Type() == NodeType::Link)
		inputStream = this->OpenLinkTargetAsStream(nodePath);

	InjectionContainer &injectionContainer = InjectionContainer::Instance();
	const Config &config = injectionContainer.Get<ConfigManager>().Config();

	Crypto::HashingInputStream hashingInputStream(*inputStream, config.hashAlgorithm);
	NullOutputStream nullOutputStream;
	uint64 readSize = hashingInputStream.FlushTo(nullOutputStream);
	UniquePointer<Crypto::HashFunction> hasher = hashingInputStream.Reset();
	hasher->Finish();
	if(readSize != attributes.Size())
		throw StreamPipingFailedException(nodePath);

	return hasher->GetDigestString().ToLowercase();
}

UniquePointer<InputStream> OSFileSystemNodeIndex::OpenLinkTargetAsStream(const Path& nodePath) const
{
	AutoPointer<const Link> link = OSFileSystem::GetInstance().GetNode(this->MapNodePathToFileSystemPath(nodePath)).MoveCast<const Link>();
	Path linkTarget = link->ReadTarget();
	return new StringInputStream(linkTarget.String(), true);
}

UniquePointer<InputStream> OSFileSystemNodeIndex::OpenFile(const Path &filePath) const
{
	return new FileInputStream(this->MapNodePathToFileSystemPath(filePath));
}

//Private methods
void OSFileSystemNodeIndex::GenerateIndex()
{
	InjectionContainer& ic = InjectionContainer::Instance();

	ProcessStatus& findStatus = ic.Get<StatusTracker>().AddProcessStatusTracker(u8"Reading directory");
	this->IndexNode(OSFileSystem::GetInstance().GetDirectory(this->basePath), String(u8"/"), findStatus);
	findStatus.Finished();
}

void OSFileSystemNodeIndex::IndexDirectoryChildren(AutoPointer<const Directory> dir, const Path& path, ProcessStatus& findStatus)
{
	for(const String& childName : *dir)
	{
		this->IndexNode(dir->GetChild(childName), path / childName, findStatus);
	}
}

void OSFileSystemNodeIndex::IndexNode(AutoPointer<const Node> node, const Path& nodePath, ProcessStatus& findStatus)
{
	UniquePointer<FileSystemNodeAttributes> attributes;
	switch(node->GetType())
	{
		case NodeType::Directory:
			attributes = new FileSystemNodeAttributes(node.Cast<const Directory>());
			break;
		case NodeType::File:
			attributes = new FileSystemNodeAttributes(node.Cast<const File>());;
			break;
		case NodeType::Link:
		{
			AutoPointer<const Link> link = node.Cast<const Link>();
			Path target = link->ReadTarget();

			//check if target points out
			Path absoluteTarget;
			if(target.IsRelative())
				absoluteTarget = this->MapNodePathToFileSystemPath(nodePath.GetParent() / target);
			else
				absoluteTarget = target;

			if(!this->basePath.IsParentOf(absoluteTarget))
				throw LinkPointsOutOfIndexDirException();

			attributes = new FileSystemNodeAttributes(link);
		}
		break;
		default:
			RAISE(ErrorHandling::IllegalCodePathError);
	}

	findStatus.AddTotalSize(attributes->Size());
	this->AddNode(nodePath, Move(attributes));
	findStatus.IncFileCount();

	if(node->GetType() == NodeType::Directory)
		this->IndexDirectoryChildren(node.Cast<const Directory>(), nodePath, findStatus);
}
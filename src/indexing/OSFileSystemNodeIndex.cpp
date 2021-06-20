/*
 * Copyright (c) 2019-2021 Amir Czwink (amir130@hotmail.de)
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
	ASSERT(attributes.Type() != FileType::Directory, u8"Can't hash directory");

	const Path &nodePath = this->GetNodePath(nodeIndex);

	UniquePointer<InputStream> inputStream;
	if(attributes.Type() == FileType::File)
		inputStream = this->OpenFile(nodePath);
	else if(attributes.Type() == FileType::Link)
		inputStream = this->OpenLinkTargetAsStream(nodePath);

	InjectionContainer &injectionContainer = InjectionContainer::Instance();
	const Config &config = injectionContainer.Get<ConfigManager>().Config();

	UniquePointer<Crypto::HashFunction> hasher = Crypto::HashFunction::CreateInstance(config.hashAlgorithm);
	Crypto::HashingInputStream hashingInputStream(*inputStream, hasher.operator->());
	NullOutputStream nullOutputStream;
	uint64 readSize = hashingInputStream.FlushTo(nullOutputStream);
	hasher->Finish();
	if(readSize != attributes.Size())
		throw StreamPipingFailedException(nodePath);

	return hasher->GetDigestString().ToLowercase();
}

UniquePointer<InputStream> OSFileSystemNodeIndex::OpenLinkTargetAsStream(const Path& nodePath) const
{
	File link(this->MapNodePathToFileSystemPath(nodePath));
	Path linkTarget = link.ReadLinkTarget();
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

	ProcessStatus& findStatus = ic.StatusTracker().AddProcessStatusTracker(u8"Reading directory");
	this->IndexNode(String(u8"/"), findStatus);
	findStatus.Finished();
}

void OSFileSystemNodeIndex::IndexDirectoryChildren(const Path& path, ProcessStatus& findStatus)
{
	File dir(this->MapNodePathToFileSystemPath(path));

	for(const DirectoryEntry& child : dir)
	{
		this->IndexNode(path / child.name, findStatus);
	}
}

void OSFileSystemNodeIndex::IndexNode(const Path& nodePath, ProcessStatus& findStatus)
{
	File node(this->MapNodePathToFileSystemPath(nodePath));

	if(node.Type() == FileType::Link)
	{
		Path target = node.ReadLinkTarget();

		//check if target points out
		Path absoluteTarget;
		if(target.IsRelative())
			absoluteTarget = this->MapNodePathToFileSystemPath(nodePath.GetParent() / target);
		else
			absoluteTarget = target;

		if(!this->basePath.IsParentOf(absoluteTarget))
			throw LinkPointsOutOfIndexDirException();
	}

	UniquePointer<FileSystemNodeAttributes> attributes = new FileSystemNodeAttributes(node.Info());

	findStatus.AddTotalSize(attributes->Size());
	this->AddNode(nodePath, Move(attributes));
	findStatus.IncFileCount();

	if(node.Type() == FileType::Directory)
		this->IndexDirectoryChildren(nodePath, findStatus);
}
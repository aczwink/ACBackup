/*
 * Copyright (c) 2019-2022 Amir Czwink (amir130@hotmail.de)
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
#include "Filtering/ThumbsDbFilter.hpp"
#include "Filtering/DesktopIniFilter.hpp"
#include "Filtering/AppleDoubleFilter.hpp"
#include "Filtering/AppleDesktopServicesStoreFilter.hpp"

//Constructor
OSFileSystemNodeIndex::OSFileSystemNodeIndex(const Path &path) : basePath(path)
{
    this->fileFilters.Push(new AppleDesktopServicesStoreFilter);
	this->fileFilters.Push(new AppleDoubleFilter);
    this->fileFilters.Push(new DesktopIniFilter);
    this->fileFilters.Push(new ThumbsDbFilter);

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
	const Config &config = injectionContainer.Config();

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
	this->IndexFile(String(u8"/"), findStatus);
	findStatus.Finished();
}

void OSFileSystemNodeIndex::IndexDirectoryChildren(const Path& path, ProcessStatus& findStatus)
{
	File dir(this->MapNodePathToFileSystemPath(path));

	for(const DirectoryEntry& child : dir)
	{
		this->IndexFile(path / child.name, findStatus);
	}
}

void OSFileSystemNodeIndex::IndexFile(const Path& filePath, ProcessStatus& findStatus)
{
	File file(this->MapNodePathToFileSystemPath(filePath));

	if( (file.Type() == FileType::File) and !this->ShouldFileBeIndexed(filePath) )
		return;
	if(file.Type() == FileType::Link)
		this->VerifyThatLinkPointsInsideBackupPath(filePath, file);

	UniquePointer<FileSystemNodeAttributes> attributes = new FileSystemNodeAttributes(file.Info());

	findStatus.AddTotalSize(attributes->Size());
	this->AddNode(filePath, Move(attributes));
	findStatus.IncFileCount();

	if(file.Type() == FileType::Directory)
		this->IndexDirectoryChildren(filePath, findStatus);
}

bool OSFileSystemNodeIndex::ShouldFileBeIndexed(const Path &filePath) const
{
	Path fsPath = this->MapNodePathToFileSystemPath(filePath);
	for(const auto& filter : this->fileFilters)
	{
		if(filter->Matches(fsPath))
			return false;
	}
	return true;
}

void OSFileSystemNodeIndex::VerifyThatLinkPointsInsideBackupPath(const Path& filePath, const File& file)
{
	Path target = file.ReadLinkTarget();

	//check if target points out
	Path absoluteTarget;
	if(target.IsRelative())
		absoluteTarget = this->MapNodePathToFileSystemPath(filePath.GetParent() / target);
	else
		absoluteTarget = target;

	if(!this->basePath.IsParentOf(absoluteTarget))
		throw LinkPointsOutOfIndexDirException();
}
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
#include "FileSystemIndex.hpp"

//Constructor
FileSystemIndex::FileSystemIndex(const Path &path, StatusTracker& tracker) : basePath(path)
{
	auto dir = OSFileSystem::GetInstance().GetDirectory(path);
	auto dirWalker = dir->WalkFiles();

	//find files
	ProcessStatus& findStatus = tracker.AddProcessStatusTracker(u8"Reading directory");
	uint64 totalSize = 0;
	for(Tuple<Path, AutoPointer<File>> kv : dirWalker)
	{
		const Path& relPath = kv.Get<0>();
		AutoPointer<const File> file = kv.Get<1>();

		//fill out file entry
		FileAttributes attrs;
		attrs.size = file->GetSize();
		totalSize += attrs.size;

		uint32 index = this->fileAttributes.Push(Move(attrs));
		this->pathMap.Insert(relPath, index);
		findStatus.IncFileCount();
	}
	findStatus.Finished();

	//compute attributes
	ProcessStatus& attrStatus = tracker.AddProcessStatusTracker(u8"Computing hashes", this->fileAttributes.GetNumberOfElements(), totalSize);
	for(uint32 i = 0; i < this->fileAttributes.GetNumberOfElements(); i++)
	{
		this->threadPool.EnqueueTask([this, i, &path, &attrStatus](){
			Path filePath = path / this->pathMap.GetReverse(i);
			AutoPointer<const File> file = OSFileSystem::GetInstance().GetFile(filePath);

			//compute digest
			UniquePointer<InputStream> input = file->OpenForReading();
			UniquePointer<Crypto::HashFunction> hasher = Crypto::HashFunction::CreateInstance(Crypto::HashAlgorithm::MD5);
			uint64 hashedSize = hasher->Update(*input);
			hasher->Finish();

			FileAttributes& attrs = this->fileAttributes[i];
			if(hashedSize != attrs.size)
				NOT_IMPLEMENTED_ERROR; //TODO: implement me

			hasher->StoreDigest(attrs.digest);

			attrStatus.IncFinishedCount();
			attrStatus.AddFinishedSize(hashedSize);
		});
	}
	this->threadPool.WaitForAllTasksToComplete();
	attrStatus.Finished();
}

//Public methods
uint32 FileSystemIndex::FindFileIndex(const Path &path) const
{
	if(this->pathMap.Contains(path))
		return this->pathMap.Get(path);
	return Unsigned<uint32>::Max();
}

const Path &FileSystemIndex::GetFile(uint32 index) const
{
	return this->pathMap.GetReverse(index);
}

const FileAttributes &FileSystemIndex::GetFileAttributes(uint32 index) const
{
	return this->fileAttributes[index];
}

uint32 FileSystemIndex::GetNumberOfFiles() const
{
	return this->fileAttributes.GetNumberOfElements();
}

UniquePointer<InputStream> FileSystemIndex::OpenFileForReading(const Path &fileEntry) const
{
	return new FileInputStream(this->basePath / fileEntry);
}

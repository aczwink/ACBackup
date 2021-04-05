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
#include "Snapshot.hpp"
//Local
#include "BackupNodeAttributes.hpp"
#include "../Serialization.hpp"
#include "VirtualSnapshotFilesystem.hpp"
#include "../config/CompressionStatistics.hpp"
#include "../StreamPipingFailedException.hpp"
#include "../status/StatusTrackingOutputStream.hpp"

struct HashAlgorithmAndValue
{
	Crypto::HashAlgorithm hashAlgorithm;
	String hashValue;
};

namespace StdXX::Serialization
{
	template <typename ArchiveType>
	void Archive(ArchiveType& ar, HashAlgorithmAndValue& hashAlgorithmAndValue)
	{
		CustomArchive(ar, u8"algorithm", hashAlgorithmAndValue.hashAlgorithm);
		ar & Binding(u8"hash", hashAlgorithmAndValue.hashValue);
	}
}

//Constructors
Snapshot::Snapshot()
{
	DateTime dateTime = DateTime::Now();
	String snapshotName = u8"snapshot_" + dateTime.Date().ToISOString() + u8"_";
	snapshotName += String::Number(dateTime.GetTime().Hours(), 10, 2) + u8"_" + String::Number(dateTime.GetTime().Minutes(), 10, 2) + u8"_" + String::Number(dateTime.GetTime().Seconds(), 10, 2);

	this->name = snapshotName;
	this->prev = nullptr;
	this->index = new BackupNodeIndex();

	const Path& dataPath = InjectionContainer::Instance().Get<ConfigManager>().Config().dataPath;

	this->fileSystem = new FlatVolumesFileSystem(dataPath / this->name, *this->index);
}

Snapshot::Snapshot(const String& name, Serialization::XmlDeserializer& xmlDeserializer)
{
	this->name = name;
	this->prev = nullptr;
	this->index = new BackupNodeIndex(xmlDeserializer);

	const Path& dataPath = InjectionContainer::Instance().Get<ConfigManager>().Config().dataPath;
	this->fileSystem = new FlatVolumesFileSystem(dataPath / this->name, *this->index);
}

//Public methods
void Snapshot::BackupMove(uint32 nodeIndex, const OSFileSystemNodeIndex &sourceIndex, const BackupNodeAttributes& oldAttributes, const Path& oldPath)
{
	const Path& filePath = sourceIndex.GetNodePath(nodeIndex);
	const FileSystemNodeAttributes& newAttributes = sourceIndex.GetNodeAttributes(nodeIndex);

	BackupNodeAttributes* attributes = new BackupNodeAttributes(oldAttributes);
	attributes->CopyFrom(newAttributes);
	attributes->OwnsBlocks(false);
	attributes->BackReferenceTarget(oldPath);
	this->index->AddNode(filePath, attributes);
}

void Snapshot::BackupNode(uint32 index, const OSFileSystemNodeIndex &sourceIndex, ProcessStatus& processStatus)
{
	const Path& filePath = sourceIndex.GetNodePath(index);
	const FileSystemNodeAttributes& fileAttributes = sourceIndex.GetNodeAttributes(index);

	BackupNodeAttributes *attributes = new BackupNodeAttributes(fileAttributes);
	this->index->AddNode(filePath, attributes);

	InjectionContainer &injectionContainer = InjectionContainer::Instance();
	const ConfigManager &configManager = injectionContainer.Get<ConfigManager>();
	const Config &config = configManager.Config();
	CompressionStatistics& compressionStatistics = injectionContainer.Get<CompressionStatistics>();

	String ext = filePath.GetFileExtension();

	UniquePointer<InputStream> nodeInputStream;
	float32 compressionRate;
	if(fileAttributes.Type() == FileType::File)
	{
		nodeInputStream = sourceIndex.OpenFile(filePath);
		compressionRate = compressionStatistics.GetCompressionRate(ext);
		if(fileAttributes.Size() == 0)
		    compressionRate = 1; //don't compress empty files
	}
	else if(fileAttributes.Type() == FileType::Link)
	{
		nodeInputStream = sourceIndex.OpenLinkTargetAsStream(filePath);
		if(fileAttributes.Size() > 100)
			compressionRate = 0; //text usually compresses well
		else
			compressionRate = 1; //but it doesnt make sense to compress to short text
	}
	else
	{
		return;
	}
	UniquePointer<Crypto::HashFunction> hasher = Crypto::HashFunction::CreateInstance(config.hashAlgorithm);
	Crypto::HashingInputStream hashingInputStream(*nodeInputStream, hasher.operator->());

	UniquePointer<OutputStream> fileOutputStream = this->fileSystem->CreateFile(filePath);
	BufferedOutputStream blockBuffer(*fileOutputStream, config.blockSize);

	OutputStream* outputStream = &blockBuffer;

	UniquePointer<Compressor> compressor;
	if(compressionRate <= 0.9f)
	{
		uint8 compressionLevel = compressionStatistics.GetCompressionLevel(compressionRate);
		compressor = Compressor::Create(config.compressionStreamFormatType, config.compressionAlgorithm, blockBuffer, compressionLevel);
		outputStream = compressor.operator->();
		attributes->CompressionSetting(configManager.CompressionSetting());
	}

	StatusTrackingOutputStream statusTrackingOutputStream(*outputStream, processStatus);

	uint64 readSize = hashingInputStream.FlushTo(statusTrackingOutputStream);
	if(readSize != sourceIndex.GetNodeAttributes(index).Size())
		throw StreamPipingFailedException(filePath);
	if(!compressor.IsNull())
		compressor->Finalize();
	outputStream->Flush();

	if(!compressor.IsNull() && (fileAttributes.Type() == FileType::File))
	{
		compressionRate = attributes->ComputeSumOfBlockSizes() / (float32)attributes->Size();
		compressionStatistics.AddCompressionRateSample(ext, compressionRate);
	}

	hasher->Finish();
	attributes->AddHashValue(config.hashAlgorithm, hasher->GetDigestString().ToLowercase());
}

void Snapshot::BackupNodeMetadata(uint32 index, const BackupNodeAttributes& oldAttributes, const OSFileSystemNodeIndex &sourceIndex)
{
	const Path& filePath = sourceIndex.GetNodePath(index);
	const FileSystemNodeAttributes& newAttributes = sourceIndex.GetNodeAttributes(index);

	BackupNodeAttributes* attributes = new BackupNodeAttributes(oldAttributes);
	attributes->CopyFrom(newAttributes);
	attributes->OwnsBlocks(false);
	this->index->AddNode(filePath, attributes);
}

const Snapshot *Snapshot::FindDataSnapshot(uint32 nodeIndex, Path& nodePathInSnapshot) const
{
	if(!this->index->HasNodeData(nodeIndex))
	{
        const BackupNodeAttributes& attributes = this->index->GetNodeAttributes(nodeIndex);
		Path parentPath;
		if(attributes.BackReferenceTarget().HasValue())
			parentPath = *attributes.BackReferenceTarget();
		else
			parentPath = this->index->GetNodePath(nodeIndex);
		return this->prev->FindDataSnapshot(this->prev->index->GetNodeIndex(parentPath), nodePathInSnapshot);
	}

	nodePathInSnapshot = this->index->GetNodePath(nodeIndex);
	return this;
}

void Snapshot::Mount(const Path& mountPoint) const
{
	VirtualSnapshotFilesystem vsf(*this);
	FileSystemsManager::Instance().OSFileSystem().MountReadOnly(mountPoint, vsf);
}

void Snapshot::Restore(const Path &restorePoint) const
{
	InjectionContainer& ic = InjectionContainer::Instance();

	StaticThreadPool& threadPool = InjectionContainer::Instance().Get<StaticThreadPool>();

	ProcessStatus& process = ic.Get<StatusTracker>().AddProcessStatusTracker(u8"Restoring snapshot: " + this->Name(),
			this->Index().GetNumberOfNodes(), this->Index().ComputeTotalSize());

	//all dirs in order first
	for(const auto& kv : this->index->PathMap())
    {
	    uint32 i = this->index->GetNodeIndex(kv.key);
        const BackupNodeAttributes& attributes = this->index->GetNodeAttributes(i);

        Path nodeRestorePath = restorePoint.String() + kv.key.String();
        if(nodeRestorePath.GetName().IsEmpty())
            nodeRestorePath = nodeRestorePath.GetParent();

        if(attributes.Type() == FileType::Directory)
        {
        	FileSystemsManager::Instance().OSFileSystem().CreateDirectory(nodeRestorePath, &attributes.Permissions());
            process.IncFinishedCount();
        }
    }

	for(uint32 i = 0; i < this->Index().GetNumberOfNodes(); i++)
	{
		threadPool.EnqueueTask([this, i, &restorePoint, &process]()
		{
			const BackupNodeAttributes& attributes = this->index->GetNodeAttributes(i);
			if(attributes.Type() == FileType::Directory)
                return; //skip

			const Path& filePath = this->Index().GetNodePath(i);
			Path nodeRestorePath = restorePoint.String() + filePath.String();
			if(nodeRestorePath.GetName().IsEmpty())
				nodeRestorePath = nodeRestorePath.GetParent();

			switch(attributes.Type())
			{
				case FileType::File:
				{
					Path realNodePath;
					const Snapshot* snapshot = this->FindDataSnapshot(i, realNodePath);
					UniquePointer<InputStream> input = snapshot->fileSystem->OpenFileForReading(snapshot->Index().GetNodeIndex(realNodePath), true);
					FileOutputStream output(nodeRestorePath, false, &attributes.Permissions());

					//write
					uint64 flushedSize = input->FlushTo(output);
					if(flushedSize != attributes.Size())
						throw StreamPipingFailedException(filePath);

					process.AddFinishedSize(flushedSize);
				}
				break;
				case FileType::Link:
				{
					Path realNodePath;
					const Snapshot* snapshot = this->FindDataSnapshot(i, realNodePath);
					Optional<Path> target = snapshot->fileSystem->ReadLinkTarget(realNodePath);

					File link(nodeRestorePath);
					link.CreateLink(target.Value());

					process.AddFinishedSize(attributes.Size());
				}
				break;
			}

			//open files
			process.IncFinishedCount();
		});
	}
	threadPool.WaitForAllTasksToComplete();
	process.Finished();
}

void Snapshot::Serialize() const
{
	ConfigManager& configManager = InjectionContainer::Instance().Get<ConfigManager>();
	const Config &config = configManager.Config();
	Crypto::HashAlgorithm hashAlgorithm = config.hashAlgorithm;

	FileOutputStream indexFile(this->IndexFilePath());
	UniquePointer<Compressor> compressor = Compressor::Create(config.compressionStreamFormatType, config.compressionAlgorithm, indexFile, config.maxCompressionLevel);
	BufferedOutputStream bufferedOutputStream(*compressor);
	Crypto::HashingOutputStream hashingOutputStream(bufferedOutputStream, hashAlgorithm);
	Serialization::XmlSerializer xmlSerializer(hashingOutputStream);
	this->index->Serialize(xmlSerializer);
	hashingOutputStream.Flush();
	compressor->Finalize();

	UniquePointer<Crypto::HashFunction> hasher = hashingOutputStream.Reset();
	hasher->Finish();

	HashAlgorithmAndValue protection;
	protection.hashAlgorithm = hashAlgorithm;
	protection.hashValue = hasher->GetDigestString().ToLowercase();

	FileOutputStream indexHashFile(this->IndexHashFilePath());
	BufferedOutputStream bufferedOutputStream2(indexHashFile);
	Serialization::JSONSerializer protectionSerializer(bufferedOutputStream2);
	protectionSerializer << protection;
	bufferedOutputStream2.Flush();
}

bool Snapshot::VerifyNode(const Path& path) const
{
	uint32 nodeIndex = this->index->GetNodeIndex(path);
	const FileSystemNodeAttributes& attributes = this->index->GetNodeAttributes(nodeIndex);

	//just read the file in once with verification
	UniquePointer<InputStream> input;

	if(attributes.Type() == FileType::Link)
		input = this->fileSystem->OpenLinkTargetAsStream(path, true);
	else
		input = this->fileSystem->OpenFileForReading(nodeIndex, true);
	NullOutputStream nullOutputStream;
	try
	{
		const uint64 readSize = input->FlushTo(nullOutputStream);
		const FileSystemNodeAttributes& fileAttributes = ((FileSystemNodeIndex&)*this->index).GetNodeAttributes(this->index->GetNodeIndex(path));
		if(readSize != fileAttributes.Size())
			return false;
	}
	catch(ErrorHandling::VerificationFailedException&)
	{
		return false;
	}

	return true;
}

//Class functions
UniquePointer<Snapshot> Snapshot::Deserialize(const Path &path)
{
	String title = path.GetTitle();
	String extension = path.GetFileExtension();

	if(extension == u8"lzma")
	{
		ConfigManager& configManager = InjectionContainer::Instance().Get<ConfigManager>();

		FileInputStream hashInputStream(path.GetParent() / title + String(c_hashFileSuffix));
		BufferedInputStream hashBufferedInputStream(hashInputStream);
		Serialization::JSONDeserializer jsonDeserializer(hashBufferedInputStream);
		HashAlgorithmAndValue protection;
		jsonDeserializer >> protection;

		FileInputStream fileInputStream(path);
		BufferedInputStream bufferedInputStream(fileInputStream);
		UniquePointer<Decompressor> decompressor = Decompressor::Create(CompressionStreamFormatType::lzma, bufferedInputStream, false);
		UniquePointer<Crypto::HashFunction> hashFunction = Crypto::HashFunction::CreateInstance(protection.hashAlgorithm);
		Crypto::HashingInputStream hashingInputStream(*decompressor, hashFunction.operator->());

		Serialization::XmlDeserializer deserializer(hashingInputStream);

		Path name(title); //strip of .xml
		UniquePointer<Snapshot> snapshot = new Snapshot(name.GetTitle(), deserializer);

		hashFunction->Finish();
		String got = hashFunction->GetDigestString().ToLowercase();

		if(protection.hashValue != got)
			throw StreamPipingFailedException(path);

		return snapshot;
	}

	return nullptr; //not an index file
}
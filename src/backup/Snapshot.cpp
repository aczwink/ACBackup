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
#include "Snapshot.hpp"
//Local
#include "BackupNodeAttributes.hpp"

//Constructors
Snapshot::Snapshot()
{
	DateTime dateTime = DateTime::Now();
	String snapshotName = u8"snapshot_" + dateTime.GetDate().ToISOString() + u8"_";
	snapshotName += String::Number(dateTime.GetTime().GetHour(), 10, 2) + u8"_" + String::Number(dateTime.GetTime().GetMinute(), 10, 2) + u8"_" + String::Number(dateTime.GetTime().GetSecond(), 10, 2);

	this->name = snapshotName;
	this->prev = nullptr;
	this->index = new BackupNodeIndex();

	const Path& dataPath = InjectionContainer::Instance().Get<ConfigManager>().Config().dataPath;

	OSFileSystem::GetInstance().GetDirectory(dataPath)->CreateSubDirectory(this->name);
	this->fileSystem = new FlatVolumesFileSystem(dataPath / this->name, *this->index);
}

Snapshot::Snapshot(const String& name, Serialization::XmlDeserializer& xmlDeserializer)
{
	this->name = name;
	this->prev = nullptr;
	this->index = new BackupNodeIndex(xmlDeserializer);
}

//Public methods
void Snapshot::AddNode(uint32 index, const OSFileSystemNodeIndex &sourceIndex, ProcessStatus& processStatus)
{
	const Path& filePath = sourceIndex.GetNodePath(index);
	const FileSystemNodeAttributes& fileAttributes = sourceIndex.GetNodeAttributes(index);

	this->index->AddNode(filePath, new BackupNodeAttributes(fileAttributes));

	if(fileAttributes.Type() == IndexableNodeType::File)
	{
		UniquePointer<OutputStream> fileOutputStream = this->fileSystem->CreateFile(filePath);
		BufferedOutputStream blockBuffer(*fileOutputStream, InjectionContainer::Instance().Get<ConfigManager>().Config().blockSize);

		UniquePointer<InputStream> fileInputStream = sourceIndex.OpenFile(filePath);

		fileInputStream->FlushTo(blockBuffer);
		blockBuffer.Flush();

		processStatus.AddFinishedSize(fileAttributes.Size());
	}
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
	this->index->Serialize(hashingOutputStream);
	compressor->Finalize();
	hashingOutputStream.Flush();

	UniquePointer<Crypto::HashFunction> hasher = hashingOutputStream.Reset();
	hasher->Finish();

	CommonFileFormats::JsonValue json = CommonFileFormats::JsonValue::Object();
	json[u8"type"] = CommonFileFormats::JsonValue(configManager.MapHashAlgorithm(hashAlgorithm));
	json[u8"hash"] = CommonFileFormats::JsonValue(hasher->GetDigestString().ToLowercase());

	FileOutputStream indexHashFile(this->IndexHashFilePath());
	BufferedOutputStream bufferedOutputStream2(indexHashFile);
	TextWriter textWriter(bufferedOutputStream2, TextCodecType::UTF8);
	textWriter.WriteString(json.Dump());
	bufferedOutputStream2.Flush();
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
		TextReader textReader(hashBufferedInputStream, TextCodecType::UTF8);
		CommonFileFormats::JsonValue json = CommonFileFormats::ParseJson(textReader);
		Crypto::HashAlgorithm hashAlgorithm = configManager.MapHashAlgorithm(json[u8"type"].StringValue());
		String expectedHash = json[u8"hash"].StringValue();

		FileInputStream fileInputStream(path);
		BufferedInputStream bufferedInputStream(fileInputStream);
		UniquePointer<Decompressor> decompressor = Decompressor::Create(CompressionStreamFormatType::lzma, bufferedInputStream, false);
		Crypto::HashingInputStream hashingInputStream(*decompressor, hashAlgorithm);

		Serialization::XmlDeserializer deserializer(hashingInputStream);

		Path name(title); //strip of .xml
		UniquePointer<Snapshot> snapshot = new Snapshot(name.GetTitle(), deserializer);

		UniquePointer<Crypto::HashFunction> hashFunction = hashingInputStream.Reset();
		hashFunction->Finish();
		String got = hashFunction->GetDigestString().ToLowercase();

		ASSERT(expectedHash == got, u8"DO THIS CORRECTLY");

		return snapshot;
	}

	return nullptr; //not an index file
}
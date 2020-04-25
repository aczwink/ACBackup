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
//Local
#include "../config/ConfigManager.hpp"
#include "../backup/SnapshotManager.hpp"
#include "../Serialization.hpp"
#include "../StreamPipingFailedException.hpp"

static String GenerateHashValue(uint32 i, const BackupNodeIndex& index, Crypto::HashAlgorithm hashAlgorithm, const Snapshot& snapshot)
{
	const BackupNodeAttributes& attributes = index.GetNodeAttributes(i);

	if(attributes.HashValues().Contains(hashAlgorithm))
		return attributes.Hash(hashAlgorithm);

	Path realNodePath;
	const Snapshot* dataSnapshot = snapshot.FindDataSnapshot(i, realNodePath);

	UniquePointer<InputStream> input;
	if(attributes.Type() == NodeType::Link)
		input = dataSnapshot->Filesystem().OpenLinkTargetAsStream(realNodePath, false);
	else
		input = dataSnapshot->Filesystem().GetFile(realNodePath)->OpenForReading(false);
	NullOutputStream nullOutputStream;
	Crypto::HashingOutputStream hashingOutputStream(nullOutputStream, hashAlgorithm);

	uint64 readSize = input->FlushTo(hashingOutputStream);
	if(readSize != attributes.Size())
		throw StreamPipingFailedException(realNodePath);

	UniquePointer<Crypto::HashFunction> hasher = hashingOutputStream.Reset();
	hasher->Finish();
	return hasher->GetDigestString().ToLowercase();
}

static int32 OutputSnapshotHashValues(const Snapshot& snapshot, Crypto::HashAlgorithm hashAlgorithm)
{
	InjectionContainer& ic = InjectionContainer::Instance();

	StatusTracker& statusTracker = ic.Get<StatusTracker>();
	StaticThreadPool& threadPool = ic.Get<StaticThreadPool>();

	const BackupNodeIndex& index = snapshot.Index();

	CommonFileFormats::CSVWriter csvWriter(stdOut, CommonFileFormats::csvDialect_excel);
	Mutex csvWriterMutex;

	ProcessStatus& process = statusTracker.AddProcessStatusTracker(u8"Generating hash values", index.GetNumberOfNodes(), index.ComputeTotalSize());
	for(uint32 i = 0; i < index.GetNumberOfNodes(); i++)
	{
		const BackupNodeAttributes& attributes = index.GetNodeAttributes(i);
		if(attributes.Type() == NodeType::Directory)
			continue;

		threadPool.EnqueueTask([&csvWriter, &csvWriterMutex, &index, i, hashAlgorithm, &snapshot, &process]()
		{
			String hash = GenerateHashValue(i, index, hashAlgorithm, snapshot);

			csvWriterMutex.Lock();
			csvWriter << index.GetNodePath(i).String() << hash << endl;
			csvWriterMutex.Unlock();

			const BackupNodeAttributes& attributes = index.GetNodeAttributes(i);
			process.AddFinishedSize(attributes.Size());
			process.IncFinishedCount();
		});
	}
	threadPool.WaitForAllTasksToComplete();
	process.Finished();

	return EXIT_SUCCESS;
}

int32 CommandOutputSnapshotHashValues(const Snapshot& snapshot)
{
	return OutputSnapshotHashValues(snapshot, InjectionContainer::Instance().Get<ConfigManager>().Config().hashAlgorithm);
}

int32 CommandOutputSnapshotHashValues(const Snapshot& snapshot, const String& hashAlgorithmString)
{
	Crypto::HashAlgorithm hashAlgorithm;
	Serialization::StringMapping(hashAlgorithm, Serialization::HashMapping()) = hashAlgorithmString;

	return OutputSnapshotHashValues(snapshot, hashAlgorithm);
}
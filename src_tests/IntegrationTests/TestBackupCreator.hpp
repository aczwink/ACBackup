/*
 * Copyright (c) 2021 Amir Czwink (amir130@hotmail.de)
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

class TestBackupCreator
{
	struct TestFileData
	{
		FileType fileType;
		String contentHash;
	};
public:
	//Constructor
	inline TestBackupCreator()
	{
		File sourceDir(this->SourcePath());
		File backupDir(this->tempDirectory.Path() / String(u8"backuptarget"));

		sourceDir.CreateDirectory();
		backupDir.CreateDirectory();

		int32 result = CommandInit(backupDir.Path(), sourceDir.Path());
		ASSERT_EQUALS(EXIT_SUCCESS, result);

		this->configManager = new ConfigManager(backupDir.Path());
		this->comprStats = new CompressionStatistics(backupDir.Path());

		InjectionContainer &ic = InjectionContainer::Instance();
		ic.ConfigManager(this->configManager.operator->());
		ic.CompressionStats(this->comprStats.operator->());
		ic.StatusTracker(new StatusTracker);
		ic.TaskQueue(GetHardwareConcurrency());
	}

	//Destructor
	inline ~TestBackupCreator()
	{
		this->GrantFullAccessRecursive(this->tempDirectory.Path());
		File dir(this->tempDirectory.Path());
		dir.RemoveChildrenRecursively();

		InjectionContainer &ic = InjectionContainer::Instance();
		ic.UnregisterAll();
	}

	//Inline
	inline void AddSourceDir(const Path& virtualRootPath)
	{
		File dir(this->SourcePath() + virtualRootPath);
		dir.CreateDirectory();

		TestFileData testFileData;
		testFileData.fileType = FileType::Directory;
		this->testPaths.Insert(virtualRootPath.Normalized(), testFileData);
	}

	inline void AddSourceFile(const Path& virtualRootPath, const String& content)
	{
		InjectionContainer &ic = InjectionContainer::Instance();
		FileOutputStream fileOutputStream(this->SourcePath() + virtualRootPath, true);
		Crypto::HashingOutputStream hashingOutputStream(fileOutputStream, ic.Config().hashAlgorithm);
		BufferedOutputStream bufferedOutputStream(hashingOutputStream);
		TextWriter textWriter(bufferedOutputStream, TextCodecType::UTF8);
		textWriter.WriteString(content);
		bufferedOutputStream.Flush();

		auto hasher = hashingOutputStream.Reset();
		hasher->Finish();
		TestFileData testFileData;
		testFileData.fileType = FileType::File;
		testFileData.contentHash = hasher->GetDigestString().ToLowercase();
		this->testPaths.Insert(virtualRootPath.Normalized(), testFileData);
	}

	inline void AddSourceLink(const Path& virtualRootPath, const String& linkTarget)
	{
		File link(this->SourcePath() + virtualRootPath);
		link.CreateLink(linkTarget);

		InjectionContainer &ic = InjectionContainer::Instance();
		linkTarget.ToUTF8();
		BufferInputStream bufferInputStream(linkTarget.GetRawData(), linkTarget.GetSize());
		auto hasher = Crypto::HashFunction::CreateInstance(ic.Config().hashAlgorithm);
		Crypto::HashingInputStream hashingOutputStream(bufferInputStream, hasher.operator->());
		NullOutputStream nullOutputStream;
		hashingOutputStream.FlushTo(nullOutputStream);
		hasher->Finish();

		TestFileData testFileData;
		testFileData.fileType = FileType::Link;
		testFileData.contentHash = hasher->GetDigestString().ToLowercase();
		this->testPaths.Insert(virtualRootPath.Normalized(), testFileData);
	}

	inline void RemoveFile(const Path& virtualRootPath)
	{
		File file(this->SourcePath() + virtualRootPath);
		file.DeleteFile();
		this->testPaths.Remove(virtualRootPath.Normalized());
	}

	inline void VerifySnapshotMatchesTestState(const Snapshot& snapshot)
	{
		ASSERT_EQUALS(this->testPaths.GetNumberOfElements() + 1, snapshot.Index().GetNumberOfNodes()); //the backup stores the root path also as extra node, which we don't

		for(const auto& kv : this->testPaths)
		{
			uint32 index = snapshot.Index().GetNodeIndex(kv.key.String());
			const auto& attribs = snapshot.Index().GetNodeAttributes(index);

			ASSERT_EQUALS(kv.value.fileType, attribs.Type());
			if(kv.value.fileType != FileType::Directory)
				ASSERT_EQUALS(kv.value.contentHash, attribs.HashValues().Get(this->configManager->Config().hashAlgorithm));
		}
	}

private:
	//Members
	TempDirectory tempDirectory;
	UniquePointer<ConfigManager> configManager;
	UniquePointer<CompressionStatistics> comprStats;
	BinaryTreeMap<Path, TestFileData> testPaths;

	//Properties
	inline Path SourcePath() const
	{
		return this->tempDirectory.Path() / String(u8"source");
	}

	//Inline
	inline void GrantFullAccessRecursive(const Path& path)
	{
		File file(path);

		POSIXPermissions posixPermissions(getuid(), getgid(), 0x1FF);
		file.ChangePermissions(posixPermissions);

		if(file.Type() == FileType::Directory)
		{
			for(const auto& entry : file)
				this->GrantFullAccessRecursive(path / entry.name);
		}
	}
};
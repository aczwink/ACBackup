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
#pragma once
#include <StdXX.hpp>
using namespace StdXX;
using namespace StdXX::FileSystem;
//Local
#include "../backup/BackupNodeIndex.hpp"

//Forward declarations
class FlatVolumesBlockInputStream;
class VolumesOutputStream;

class FlatVolumesFileSystem : public RWFileSystem
{
	struct VolumeForReading
	{
		UniquePointer<FileInputStream> file;
		uint64 counter = 0;
		Mutex mutex;
	};

	struct OpenVolumeForWriting
	{
		uint64 number;
		UniquePointer<FileOutputStream> file;
		uint64 leftSize;
		const OutputStream* ownedWriter;
	};
public:
	//Constructor
	FlatVolumesFileSystem(const Path &dirPath, BackupNodeIndex& index);

	//Methods
	void CloseFile(const VolumesOutputStream& writer);
	UniquePointer<OutputStream> CreateFile(const Path &filePath) override;
	void CreateLink(const Path &linkPath, const Path &linkTargetPath) override;
	void Flush() override;
	void Move(const Path &from, const Path &to) override;
	UniquePointer<InputStream> OpenFileForReading(uint32 fileIndex, bool verify) const;
	UniquePointer<InputStream> OpenFileForReading(const Path &path, bool verify) const override;
	UniquePointer<InputStream> OpenLinkTargetAsStream(const Path& linkPath, bool verify) const;
	uint32 ReadBytes(const FlatVolumesBlockInputStream& reader, void *destination, uint64 volumeNumber, uint64 offset, uint32 count) const;
	void WriteBytes(const VolumesOutputStream& writer, const void* source, uint32 size);
	void WriteProtect();
	SpaceInfo QuerySpace() const override;

	//Inline
	inline void DecrementVolumeCount(uint64 volumeNumber) const
	{
		VolumeForReading& volume = this->reading.volumes->operator[](volumeNumber);
		AutoLock lock(volume.mutex);
		volume.counter--;
	}


	//TODO: NOT IMPLEMENTED
	UniquePointer<DirectoryEnumerator> EnumerateChildren(const Path &path) const override;
	Optional<FileInfo> QueryFileInfo(const Path &path) const override;
	Optional<Path> ReadLinkTarget(const Path &path) const override;
	Optional<Errors::CreateDirectoryError> CreateDirectory(const Path &path, const Permissions *permissions) override;
	void DeleteFile(const Path &path) override;
	UniquePointer<OutputStream> OpenFileForWriting(const Path &path) override;
	void RemoveDirectory(const Path &path) override;
	void ChangePermissions(const Path &path, const Permissions &newPermissions) override;
	//TODO: NOT IMPLEMENTED

private:
	//Members
	Path dirPath;
	BackupNodeIndex& index;
	struct
	{
		BinaryTreeSet<Path> directories;
		mutable UniquePointer<FixedArray<VolumeForReading>> volumes;
		mutable Atomic<uint16> nOpenVolumes;
		mutable Mutex nOpenVolumesLock;
	} reading;
	struct
	{
		bool createdDataDir;
		uint64 nextVolumeNumber;
		LinkedList<OpenVolumeForWriting> openVolumes;
		Mutex openVolumesMutex;
	} writing;

	//Methods
	void BytesWereWrittenToVolume(const VolumesOutputStream* writer, uint64 offset, uint32 nBytesWritten);
	void CloseUnusedVolumes() const;
	SeekableOutputStream& FindStream(const OutputStream* writer, uint64& leftSize);
	void IncrementVolumeCounters(const DynamicArray<Block>& blocks) const;
	SeekableInputStream& LockVolumeStream(uint64 volumeNumber) const;

	//Inline
	inline void UnlockVolumeStream(uint64 volumeNumber) const
	{
		(*this->reading.volumes)[volumeNumber].mutex.Unlock();
	}
};
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
#pragma once
#include <Std++.hpp>
using namespace StdXX;
//Local
#include "../backup/BackupNodeIndex.hpp"

//Forward declarations
class FlatVolumesBlockInputStream;
class VolumesOutputStream;

class FlatVolumesFileSystem : public FileSystem
{
	struct OpenVolumeForReading
	{
		UniquePointer<FileInputStream> file;
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
	bool Exists(const Path &path) const override;
	void Flush() override;
	AutoPointer<FileSystemNode> GetNode(const Path &path) override;
	AutoPointer<const FileSystemNode> GetNode(const Path &path) const override;
	AutoPointer<Directory> GetRoot() override;
	AutoPointer<const Directory> GetRoot() const override;
	uint64 GetSize() const override;
	void Move(const Path &from, const Path &to) override;
	UniquePointer<InputStream> OpenLinkTargetAsStream(const Path& linkPath, bool verify) const;
	uint32 ReadBytes(const FlatVolumesBlockInputStream& reader, void *destination, uint64 volumeNumber, uint64 offset, uint32 count) const;
	void WriteBytes(const VolumesOutputStream& writer, const void* source, uint32 size);
	void WriteProtect();

private:
	//Members
	Path dirPath;
	BackupNodeIndex& index;
	struct
	{
		BinaryTreeSet<Path> directories;
		mutable UniquePointer<FixedArray<OpenVolumeForReading>> openVolumes;
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
	SeekableOutputStream& FindStream(const OutputStream* writer, uint64& leftSize);
	SeekableInputStream& LockVolumeStream(uint64 volumeNumber) const;

	//Inline
	inline void UnlockVolumeStream(uint64 volumeNumber) const
	{
		(*this->reading.openVolumes)[volumeNumber].mutex.Unlock();
	}
};
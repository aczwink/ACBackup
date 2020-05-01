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
#include <StdXX.hpp>
using namespace StdXX;
//Local
#include "../indexing/FileSystemNodeIndex.hpp"
#include "Snapshot.hpp"
#include "../NodeIndexDifferenceResolver.hpp"

class SnapshotManager
{
public:
	//Constructor
	SnapshotManager();

	//Properties
	inline const DynamicArray<UniquePointer<Snapshot>>& Snapshots() const
	{
		return this->snapshots;
	}

	//Methods
	bool AddSnapshot(const OSFileSystemNodeIndex& sourceIndex);
	DynamicArray<uint32> VerifySnapshot(const Snapshot& snapshot, bool full) const;

	//Inline
	inline const Snapshot* FindSnapshot(const String& name) const
	{
		for(const auto& snapshot : this->snapshots)
		{
			if(snapshot->Name() == name)
				return snapshot.operator->();
		}
		return nullptr;
	}

	inline const Snapshot& NewestSnapshot() const
	{
		return *this->snapshots.Last();
	}

private:
	//Members
	DynamicArray<UniquePointer<Snapshot>> snapshots;

	//Methods
	NodeIndexDifferences ComputeDifference(const OSFileSystemNodeIndex& sourceIndex, bool updateDefault) const;
	void EnsureNoDifferenceExists(const OSFileSystemNodeIndex& sourceIndex) const;
	DynamicArray<Path> ListPathsInIndexDirectory();
	void ReadInSnapshots();

	//Inline
	inline const BackupNodeIndex* LastIndex() const
	{
		if(this->snapshots.IsEmpty())
			return nullptr;
		return &this->snapshots.Last()->Index();
	}
};

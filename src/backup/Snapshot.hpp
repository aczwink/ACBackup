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
#include <Std++.hpp>
using namespace StdXX;
//Local
#include "../indexing/FileSystemNodeIndex.hpp"

class Snapshot
{
public:
	//Constructor
	Snapshot(const Path& path);

	//Properties
	inline const FileSystemNodeIndex& Index() const
	{
		return *this->index;
	}

	inline const String& Name() const
	{
		return this->name;
	}

	inline void Previous(Snapshot* newPrevious)
	{
		this->prev = newPrevious;
	}

	//Methods
	void AddNode(uint32 index, const FileSystemNodeIndex& sourceIndex);

	//Functions
	static UniquePointer<Snapshot> Deserialize(const Path& path);

private:
	//Members
	Path dirPath;
	String name;
	Snapshot* prev;
	UniquePointer<FileSystemNodeIndex> index;
	UniquePointer<FileSystem> fileSystem;
};
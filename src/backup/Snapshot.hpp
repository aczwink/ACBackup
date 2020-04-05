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
#include <Std++.hpp>
using namespace StdXX;
//Local
#include "BackupNodeIndex.hpp"
#include "../status/ProcessStatus.hpp"
#include "../indexing/OSFileSystemNodeIndex.hpp"
#include "../Util.hpp"
#include "../config/ConfigManager.hpp"
#include "../InjectionContainer.hpp"
#include "../backupfilesystem/FlatVolumesFileSystem.hpp"

//Constants
static const char *const c_hashFileSuffix = u8"_hash.json";

class Snapshot
{
public:
	//Constructor
	Snapshot();

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
	void AddNode(uint32 index, const OSFileSystemNodeIndex &sourceIndex, ProcessStatus& processStatus);
	void Serialize() const;

	//Functions
	static UniquePointer<Snapshot> Deserialize(const Path& path);

	//Inline
	inline void WriteProtect()
	{
		WriteProtectFile(this->IndexFilePath());
		WriteProtectFile(this->IndexHashFilePath());

		this->fileSystem->WriteProtect();
	}

private:
	//Members
	String name;
	UniquePointer<BackupNodeIndex> index;
	Snapshot* prev;
	UniquePointer<FlatVolumesFileSystem> fileSystem;

	//Constructor
	Snapshot(const String& name, Serialization::XmlDeserializer& xmlDeserializer);

	//Properties
	inline Path IndexFilePath() const
	{
		return this->PathPrefix() + String(u8".lzma");
	}

	inline Path IndexHashFilePath() const
	{
		return this->PathPrefix() + String(c_hashFileSuffix);
	}

	inline Path PathPrefix() const
	{
		ConfigManager& configManager = InjectionContainer::Instance().Get<ConfigManager>();
		const Config &config = configManager.Config();
		return config.indexPath / this->name + String(u8".xml");
	}
};
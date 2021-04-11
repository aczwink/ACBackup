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
//Local
#include "Config.hpp"
#include "../status/StatusTracker.hpp"
#include "../CompressionSetting.hpp"

class ConfigManager
{
public:
	//Constructors
	ConfigManager(const Path& backupPath);
	ConfigManager(const Path& backupPath, const Path& sourcePath);

	//Methods
	void Write(const Path& dirPath);

	//Functions
	static void GetCompressionSettings(enum CompressionSetting compressionSetting, CompressionSettings& settings);

	//Properties
	enum CompressionSetting CompressionSetting() const
	{
		return CompressionSetting::lzma;
	}

	//Inline
	inline const struct Config& Config() const
	{
		return this->config;
	}

private:
	//Constants
	const String c_configFileName = u8"config.json";

	//Members
	struct Config config;

	//Inline
	inline void SetPathsInConfig()
	{
		this->config.dataPath = this->config.backupPath / String(u8"data");
		this->config.indexPath = this->config.backupPath / String(u8"index");
	}

	template<typename T>
	inline void WriteConfigValue(TextWriter& textWriter, uint16 nTabs, const String& key, T value, const String& comment) const
	{
		textWriter.WriteTabs(nTabs);
		textWriter << u8"\"" << key << "\": " << value << u8", //" << comment << endl;
	}

	inline void WriteConfigStringValue(TextWriter& textWriter, uint16 nTabs, const String& key, const String& value, const String& comment) const
	{
		this->WriteConfigValue(textWriter, nTabs, key, u8"\"" + value + u8"\"", comment);
	}
};
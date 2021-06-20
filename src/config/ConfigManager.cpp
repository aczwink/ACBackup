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
#include "ConfigManager.hpp"
//Local
#include "ConfigException.hpp"
#include "../Serialization.hpp"
//Namespaces
using namespace StdXX::CommonFileFormats;
using namespace StdXX::Serialization;

//Constants
const char8_t* c_blockSize = u8"blockSize";

const char8_t* c_compression = u8"compression";
const char8_t* c_compression_lzma = u8"lzma";

const char8_t* c_maxCompressionLevel = u8"maxCompressionLevel";

static const char8_t *const c_hashAlgorithm = u8"hashAlgorithm";

static const char8_t *const c_sourcePath = u8"sourcePath";

const char8_t* c_statusTracker = u8"statusTracker";
const char8_t* c_statusTracker_web = u8"web";

const char8_t* c_statusTracker_port = u8"statusTrackerPort";

const char8_t* c_volumeSize = u8"volumeSize";

namespace StdXX::Serialization
{
	void Archive(JSONDeserializer& ar, Config& config)
	{
		CompressionSetting compressionSetting = CompressionSetting::lzma;
		StaticArray<Tuple<StatusTrackerType, String>, 2> statusTrackerMapping = { {
				{ StatusTrackerType::Terminal, u8"terminal"},
				{ StatusTrackerType::Web, c_statusTracker_web},
		} };

		ar & Binding(c_sourcePath, config.sourcePath)
			& Binding(c_blockSize, config.blockSize)
			& Binding(c_volumeSize, config.volumeSize)
		;
		CustomArchive(ar, c_compression, compressionSetting);
		ar & Binding(c_maxCompressionLevel, config.maxCompressionLevel);
		CustomArchive(ar, c_hashAlgorithm, config.hashAlgorithm);
		ar & Binding(c_statusTracker, StringMapping(config.statusTrackerType, statusTrackerMapping))
			& Binding(c_statusTracker_port, config.statusTrackerPort)
		;

		ConfigManager::GetCompressionSettings(compressionSetting, config);

		if(!Math::IsValueInInterval(config.maxCompressionLevel, 0_u8, 9_u8))
			throw ConfigException(u8"Invalid value for field '" + String(c_maxCompressionLevel) + u8"'");

		config.blockSize *= KiB;
		config.volumeSize *= MiB;
	}
}

//Constructors
ConfigManager::ConfigManager(const Path& backupPath)
{
	this->config.backupPath = backupPath;

	this->SetPathsInConfig();

	Path filePath = this->config.backupPath / this->c_configFileName;
	FileInputStream file(filePath);
	BufferedInputStream bufferedInputStream(file);

	JSONDeserializer deserializer(bufferedInputStream, true);
	deserializer >> this->config;
}

ConfigManager::ConfigManager(const Path& backupPath, const Path& sourcePath)
{
	this->config.backupPath = backupPath;
	this->config.sourcePath = sourcePath;
	this->SetPathsInConfig();
}

//Public methods
void ConfigManager::Write(const Path &dirPath)
{
	Path filePath = dirPath / this->c_configFileName;

	FileOutputStream file(filePath);
	BufferedOutputStream bufferedOutputStream(file);
	TextWriter textWriter(bufferedOutputStream, TextCodecType::UTF8);

	textWriter << u8"{" << endl;
	this->WriteConfigStringValue(textWriter, 1, c_sourcePath, this->config.sourcePath.String(), u8"The path to the directory that should be backed up");
	this->WriteConfigValue(textWriter, 1, c_blockSize, 1024, u8"The maximum size of a block in KiB");
	this->WriteConfigValue(textWriter, 1, c_volumeSize, 100, u8"The maximum size of a volume in MiB");
	this->WriteConfigStringValue(textWriter, 1, c_compression, c_compression_lzma, u8"The used compression method");
	this->WriteConfigValue(textWriter, 1, c_maxCompressionLevel, 6, u8"The maximum compression level");
	this->WriteConfigStringValue(textWriter, 1, c_hashAlgorithm, c_hashAlgorithm_sha512_256, u8"The algorithm used to compute hash values");
	this->WriteConfigStringValue(textWriter, 1, c_statusTracker, c_statusTracker_web, u8"The type of status reporting that should be used. Currently there is 'terminal' and 'web'.");
	this->WriteConfigValue(textWriter, 1, c_statusTracker_port, 8080, u8"Port that the status tracking web service will listen on if enabled.");
	textWriter << u8"}" << endl;

	bufferedOutputStream.Flush();
}

//Class functions
void ConfigManager::GetCompressionSettings(enum CompressionSetting compressionSetting, CompressionSettings& settings)
{
	switch(compressionSetting)
	{
		case CompressionSetting::lzma:
			settings.compressionAlgorithm = CompressionAlgorithm::LZMA;
			settings.compressionStreamFormatType = CompressionStreamFormatType::lzma;
			break;
	}
}
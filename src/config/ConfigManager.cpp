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
#include "ConfigManager.hpp"
//Namespaces
using namespace StdXX::CommonFileFormats;
//Local
#include "ConfigException.hpp"

//Constants
const char* c_blockSize = u8"blockSize";

const char* c_compression = u8"compression";
const char* c_compression_lzma = u8"lzma";

const char* c_maxCompressionLevel = u8"maxCompressionLevel";

static const char *const c_hashAlgorithm = u8"hashAlgorithm";
static const char *const c_hashAlgorithm_sha512_256 = u8"sha512/256";

static const char *const c_sourcePath = u8"sourcePath";

const char* c_statusTracker = u8"statusTracker";
const char* c_statusTracker_terminal = u8"terminal";

const char* c_volumeSize = u8"volumeSize";

//Constructors
ConfigManager::ConfigManager()
{
	this->SetPathsInConfig();

	Path filePath = this->config.backupPath / this->c_configFileName;
	FileInputStream file(filePath);
	BufferedInputStream bufferedInputStream(file);
	TextReader textReader(bufferedInputStream, TextCodecType::UTF8);

	JsonValue cfg = CommonFileFormats::ParseHumanReadableJson(textReader);
	this->ReadConfig(cfg);
}

ConfigManager::ConfigManager(const Path& sourcePath)
{
	this->config.sourcePath = sourcePath;
	this->SetPathsInConfig();
}

//Public methods
Crypto::HashAlgorithm ConfigManager::MapHashAlgorithm(const String& string) const
{
	if(string == c_hashAlgorithm_sha512_256)
		return Crypto::HashAlgorithm::SHA512_256;
	throw ConfigException(u8"Invalid value for field '" + String(c_hashAlgorithm) + u8"'");
}

String ConfigManager::MapHashAlgorithm(Crypto::HashAlgorithm hashAlgorithm) const
{
	switch(hashAlgorithm)
	{
		case Crypto::HashAlgorithm::SHA512_256:
			return c_hashAlgorithm_sha512_256;
	}
	RAISE(StdXX::ErrorHandling::IllegalCodePathError);
}

void ConfigManager::Write(const Path &dirPath)
{
	Path filePath = dirPath / this->c_configFileName;

	FileOutputStream file(filePath);
	BufferedOutputStream bufferedOutputStream(file);
	TextWriter textWriter(bufferedOutputStream, TextCodecType::UTF8);

	textWriter << u8"{" << endl;
	this->WriteConfigStringValue(textWriter, 1, c_sourcePath, this->config.sourcePath.GetString(), u8"The path to the directory that should be backed up");
	this->WriteConfigValue(textWriter, 1, c_blockSize, 1024, u8"The maximum size of a block in KiB");
	this->WriteConfigValue(textWriter, 1, c_volumeSize, 100, u8"The maximum size of a volume in MiB");
	this->WriteConfigStringValue(textWriter, 1, c_compression, c_compression_lzma, u8"The used compression method");
	this->WriteConfigValue(textWriter, 1, c_maxCompressionLevel, 6, u8"The maximum compression level");
	this->WriteConfigStringValue(textWriter, 1, c_hashAlgorithm, c_hashAlgorithm_sha512_256, u8"The algorithm used to compute hash values");
	this->WriteConfigStringValue(textWriter, 1, c_statusTracker, c_statusTracker_terminal, u8"The type of status reporting that should be used");
	textWriter << u8"}" << endl;

	bufferedOutputStream.Flush();
}

//Private methods
void ConfigManager::MapCompressionFields(struct Config &config, const JsonValue &object) const
{
	String string = object.Get(c_compression, String()).StringValue();
	if(string == c_compression_lzma)
	{
		config.compressionAlgorithm = CompressionAlgorithm::LZMA;
		config.compressionStreamFormatType = CompressionStreamFormatType::lzma;
	}
	else
		throw ConfigException(u8"Invalid value for field '" + String(c_compression) + u8"'");

	float64 value = object.Get(c_maxCompressionLevel, -1.0).NumberValue();
	for(uint8 i = 0; i <= 9; i++)
	{
		if(value == i)
		{
			config.maxCompressionLevel = i;
			return;
		}
	}
	throw ConfigException(u8"Invalid value for field '" + String(c_maxCompressionLevel) + u8"'");
}

StatusTrackerType ConfigManager::MapStatusTrackerType(const CommonFileFormats::JsonValue& object) const
{
	String string = object.Get(c_statusTracker, String()).StringValue();
	if(string == c_statusTracker_terminal)
		return StatusTrackerType::Terminal;
	throw ConfigException(u8"Invalid value for field 'statusTracker'");
}

void ConfigManager::ReadConfig(const JsonValue &cfg)
{
	ASSERT(cfg.Type() == JsonType::Object, u8"REPORT THIS PLEASE!");

	this->config.sourcePath = cfg.MapValue()[c_sourcePath].StringValue();
	this->config.blockSize = uint32(cfg.MapValue()[c_blockSize].NumberValue()) * KiB;
	this->config.volumeSize = uint64(cfg.MapValue()[c_volumeSize].NumberValue()) * MiB;
	this->config.hashAlgorithm = this->MapHashAlgorithm(cfg.Get(c_hashAlgorithm, String()).StringValue());
	this->config.statusTrackerType = this->MapStatusTrackerType(cfg);

	this->MapCompressionFields(this->config, cfg);
}
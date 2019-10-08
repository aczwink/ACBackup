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
//Class header
#include "Config.hpp"
//Namespaces
using namespace StdXX::CommonFileFormats;

//Constructor
Config::Config()
{
	this->port = 8080;
}

Config::Config(const Path &dirPath)
{
	Path filePath = dirPath / this->GetDefaultConfigFileName();
	FileInputStream file(filePath);
	BufferedInputStream bufferedInputStream(file);
	TextReader textReader(bufferedInputStream, TextCodecType::UTF8);

	JsonValue cfg = CommonFileFormats::ParseHumanReadableJson(textReader);
	ASSERT(cfg.Type() == JsonType::Object, u8"REPORT THIS PLEASE!");

	const Map<String, JsonValue>& cfgMap = cfg.MapValue();

	ASSERT(cfgMap.Contains(u8"port"), u8"REPORT THIS PLEASE!");
	this->port = cfgMap[u8"port"].NumberValue();
}

//Public methods
void Config::Write(const Path &dirPath)
{
	Path filePath = dirPath / this->GetDefaultConfigFileName();

	FileOutputStream file(filePath);
	BufferedOutputStream bufferedOutputStream(file);
	TextWriter textWriter(bufferedOutputStream, TextCodecType::UTF8);

	textWriter << u8"{" << endl;
	this->WriteConfigValue(textWriter, 1, u8"port", this->port, u8"Port that the status tracking web service will listen on.");
	textWriter << u8"}" << endl;

	bufferedOutputStream.Flush();
}
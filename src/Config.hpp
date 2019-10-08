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
#pragma once
#include <Std++.hpp>
using namespace StdXX;

class Config
{
public:
	//Constructors
	Config();
	Config(const Path& dirPath);

	//Properties
	inline uint16 Port() const
	{
		return this->port;
	}

	//Methods
	void Write(const Path& dirPath);

private:
	//Members
	uint16 port;

	//Inline
	inline String GetDefaultConfigFileName() const
	{
		return u8"bkpconfig.json";
	}

	template<typename T>
	inline void WriteConfigValue(TextWriter& textWriter, uint16 nTabs, const String& key, T value, const String& comment) const
	{
		textWriter.WriteTabs(nTabs);
		textWriter << u8"\"" << key << "\": " << value << u8", //" << comment << endl;
	}
};
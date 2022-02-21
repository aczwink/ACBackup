/*
 * Copyright (c) 2022 Amir Czwink (amir130@hotmail.de)
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
#include "DesktopIniFilter.hpp"

//Public methods
bool DesktopIniFilter::Matches(const Path &path) const
{
    if(path.GetName() == u8"desktop.ini")
    {
        FileInputStream fileInputStream(path);
        return this->MatchesData(fileInputStream);
    }
    return false;
}

//Private methods
bool DesktopIniFilter::MatchesData(InputStream& inputStream) const
{
    DataReader dataReader(false, inputStream);

    if(dataReader.ReadUInt16() == 0xFEFF)
    {
        TextReader textReader(inputStream, TextCodecType::UTF16_LE);

        if(textReader.ReadLine().IsEmpty())
            return textReader.ReadLine() == u8"[.ShellClassInfo]";
    }

    return false;
}
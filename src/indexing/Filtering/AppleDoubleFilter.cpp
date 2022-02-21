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
#include "AppleDoubleFilter.hpp"

//Public methods
bool AppleDoubleFilter::Matches(const Path &path) const
{
    if(path.GetName().StartsWith(u8"._"))
    {
        FileInputStream fileInputStream(path);
        return this->MatchesData(fileInputStream);
    }
    return false;
}

//Private methods
bool AppleDoubleFilter::MatchesData(InputStream& inputStream) const
{
    DataReader dataReader(true, inputStream);

    if(dataReader.ReadUInt32() != 0x00051607) //signature
        return false;
    if(dataReader.ReadUInt32() != 0x00020000) //version
        return false;
    dataReader.Skip(16); //filler

    uint16 nEntries = dataReader.ReadUInt16();
    for(uint16 i = 0; i < nEntries; i++)
    {
        uint32 entryId = dataReader.ReadUInt32();
        uint32 offset = dataReader.ReadUInt32();
        uint32 length = dataReader.ReadUInt32();

        switch(entryId)
        {
            case 2: //resource fork
            case 9: //finder info
                break;
            default:
                NOT_IMPLEMENTED_ERROR; //TODO: implement me
        }
    }

    return true;
}
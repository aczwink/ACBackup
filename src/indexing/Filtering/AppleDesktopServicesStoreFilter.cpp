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
#include "AppleDesktopServicesStoreFilter.hpp"

//Public methods
bool AppleDesktopServicesStoreFilter::Matches(const Path &path) const
{
    if(path.GetName() == u8".DS_Store")
    {
        FileInputStream fileInputStream(path);
        return this->MatchesData(fileInputStream);
    }
    return false;
}

//Private methods
bool AppleDesktopServicesStoreFilter::MatchesData(InputStream& inputStream) const
{
    uint8 sig[8];

    uint32 nBytesRead = inputStream.ReadBytes(sig, sizeof(sig));
    if(nBytesRead == sizeof(sig))
    {
        const static uint8 expectedSig[8] = {0x00, 0x00, 0x00, 0x01, 0x42, 0x75, 0x64, 0x31};

        return MemCmp(sig, expectedSig, sizeof(sig)) == 0;
    }

    return false;
}
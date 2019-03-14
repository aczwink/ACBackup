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
#include "BackupContainerIndex.hpp"
#include "FlatContainerIndex.hpp"

//Class functions
Snapshot BackupContainerIndex::Deserialize(const Path &path)
{
	if(path.GetFileExtension() == u8"index")
	{
		//check if this is a valid index
		enum class IndexType
		{
			Unknown,
			Flat
		};

		IndexType indexType = IndexType::Unknown;
		{
			FileInputStream fileInputStream(path);
			byte signature[8];
			fileInputStream.ReadBytes(signature, sizeof(signature));

			if(MemCmp(signature, u8"acbkpidx", 8) == 0)
			{
				DataReader dataReader(true, fileInputStream);
				uint16 maj = dataReader.ReadUInt16();
				uint16 min = dataReader.ReadUInt16();

				if((maj == 1) && (min == 0))
				{
					byte idxType[4];
					fileInputStream.ReadBytes(idxType, sizeof(idxType));
					if(MemCmp(idxType, u8"flat", sizeof(idxType)) == 0)
					{
						indexType = IndexType::Flat;
					}
				}
			}
		}

		switch(indexType)
		{
			case IndexType::Flat:
				return FlatContainerIndex::Deserialize(path);
			case IndexType::Unknown:
				break;
		}
	}

	return {};
}

/*
 * Copyright (c) 2020 Amir Czwink (amir130@hotmail.de)
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
#include "FlatVolumesBlockInputStream.hpp"

//Public methods
uint32 FlatVolumesBlockInputStream::GetBytesAvailable() const
{
	return 0; //this stream does not buffer
}

bool FlatVolumesBlockInputStream::IsAtEnd() const
{
	return this->currentBlockIndex >= this->blocks.GetNumberOfElements();
}

uint32 FlatVolumesBlockInputStream::ReadBytes(void *destination, uint32 count)
{
	uint8* dest = static_cast<uint8 *>(destination);

	if(this->IsAtEnd())
		return 0;

	while(count)
	{
		if(this->blockOffset >= this->blocks[this->currentBlockIndex].size)
		{
			this->currentBlockIndex++;
			if(this->IsAtEnd())
				break;
			this->blockOffset = 0;
		}
		const Block& block = this->blocks[this->currentBlockIndex];
		const uint32 leftSize = Math::Min(count, Unsigned<uint32>::DowncastToClosest(block.size - this->blockOffset));
		uint32 nBytesRead = this->fileSystem.ReadBytes(*this, dest, block.volumeNumber, block.offset + this->blockOffset, leftSize);

		dest += nBytesRead;
		this->blockOffset += nBytesRead;
		count -= nBytesRead;
	}

	return dest - static_cast<uint8 *>(destination);;
}

uint32 FlatVolumesBlockInputStream::Skip(uint32 nBytes)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return 0;
}
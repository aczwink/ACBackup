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
#include "BackupNodeAttributes.hpp"

//Public methods
void BackupNodeAttributes::AddBlock(const Block &block)
{
	this->ownsBlocks = true;
	if(!this->blocks.IsEmpty())
	{
		Block& lastBlock = this->blocks.Last();
		if( (lastBlock.volumeNumber == block.volumeNumber) && ((lastBlock.offset + lastBlock.size) == block.offset) )
		{
			lastBlock.size += block.size;
			return;
		}
	}
	this->blocks.Push(block);
}

uint64 BackupNodeAttributes::ComputeSumOfBlockSizes() const
{
	uint64 sum = 0;
	for(const Block& block : this->blocks)
		sum += block.size;
	return sum;
}

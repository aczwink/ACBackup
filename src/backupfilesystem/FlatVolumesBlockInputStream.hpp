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
#include <StdXX.hpp>
using namespace StdXX;
//Local
#include "../backup/BackupNodeAttributes.hpp"
#include "FlatVolumesFileSystem.hpp"

class FlatVolumesBlockInputStream : public InputStream
{
public:
	//Constructor
	inline FlatVolumesBlockInputStream(const FlatVolumesFileSystem &fileSystem, const DynamicArray<Block>& blocks)
		: fileSystem(fileSystem), blocks(blocks)
	{
		this->currentBlockIndex = 0;
		this->blockOffset = 0;
	}

	//Methods
	uint32 GetBytesAvailable() const override;
	bool IsAtEnd() const override;
	uint32 ReadBytes(void *destination, uint32 count) override;
	uint32 Skip(uint32 nBytes) override;

private:
	//Members
	uint32 currentBlockIndex;
	uint64 blockOffset;
	const FlatVolumesFileSystem &fileSystem;
	const DynamicArray<Block>& blocks;
};
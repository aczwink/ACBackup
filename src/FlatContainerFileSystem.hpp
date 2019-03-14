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
//Local
#include "FlatContainerIndex.hpp"

class FlatContainerFileSystem : public ContainerFileSystem
{
public:
	//Constructor
	inline FlatContainerFileSystem(const FlatContainerIndex& index, const Path& path) : ContainerFileSystem(nullptr, path)
	{
		//add files to file system
		for(uint32 i = 0; i < index.GetNumberOfFiles(); i++)
		{
			const FlatContainerFileAttributes& attributes = index.GetSpecificFileAttributes(i);

			ContainerFileHeader header;
			header.offset = attributes.offset;
			header.uncompressedSize = attributes.size;
			header.compressedSize = attributes.blockSize;
			header.compression = CompressionAlgorithm::LZMA;

			this->AddSourceFile(index.GetFile(i), header);
		}
	}

	//Methods
	void Flush() override;
};
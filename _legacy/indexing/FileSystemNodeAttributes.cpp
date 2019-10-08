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
#include "FileSystemNodeAttributes.hpp"

//Operators
bool FileSystemNodeAttributes::operator==(const FileSystemNodeAttributes &rhs) const
{
	if(this->type != rhs.type)
		return false;
	if(this->lastModifiedTime != rhs.lastModifiedTime)
		return false;

	switch(this->type)
	{
		case IndexableNodeType::File:
		{
			Crypto::HashAlgorithm hashAlgorithm = this->config.HashAlgorithm();
			return (this->size == rhs.size) && (this->hashes[hashAlgorithm] == rhs.hashes[hashAlgorithm]);
		}
		break;
		case IndexableNodeType::Link:
			return this->target == rhs.target;
	}

	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return false;
}

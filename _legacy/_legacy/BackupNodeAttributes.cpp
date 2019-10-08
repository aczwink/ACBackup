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
#include "BackupNodeAttributes.hpp"

//Private methods
void BackupNodeAttributes::ConfigureCompression(float32 compressionRate)
{
	int8 maxCompressionLevel = this->config.MaxCompressionLevel();

	float32 c = (maxCompressionLevel+1) * (1 - compressionRate);
	this->compressionLevel = static_cast<uint8>(round(c));
	this->isCompressed = !((maxCompressionLevel == -1) or (this->compressionLevel == 0));
	this->compressionLevel--;
}

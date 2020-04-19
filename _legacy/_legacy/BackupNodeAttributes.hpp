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
class BackupNodeAttributes : public FileSystemNodeAttributes
{
public:
	//Constructor
	inline BackupNodeAttributes(float32 compressionRate, const FileSystemNodeAttributes& attributes) : FileSystemNodeAttributes(attributes)
	{
		this->ConfigureCompression(compressionRate);
	}

	//Properties
	inline uint8 CompressionLevel() const
	{
		return this->compressionLevel;
	}

	inline float32 CompressionRate() const
	{
		if(this->Size() == 0)
			return 1;
		NOT_IMPLEMENTED_ERROR; //TODO: reimplement me
		//return backupEntry.blockSize / float32(backupEntry.size);
	}

	inline bool IsCompressed() const
	{
		return this->isCompressed;
	}

private:
	//Members
	bool isCompressed;
	uint8 compressionLevel;

	//Methods
	void ConfigureCompression(float32 compressionRate);
};
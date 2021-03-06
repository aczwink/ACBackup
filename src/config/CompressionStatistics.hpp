/*
 * Copyright (c) 2019-2021 Amir Czwink (amir130@hotmail.de)
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
#pragma once
#include <StdXX.hpp>
using namespace StdXX;
using namespace StdXX::FileSystem;

class CompressionStatistics
{
public:
	//Constructors
	CompressionStatistics() = default;
	explicit CompressionStatistics(const Path& path);

    //Methods
	void AddCompressionRateSample(const String& fileExtension, float32 compressionRate);
	uint8 GetCompressionLevel(float32 compressionRate) const;
	float32 GetCompressionRate(const String& fileExtension);
    void Write(const Path& dirPath);

    //Inline
    inline void SetAsIncompressible(const String& extension)
    {
        this->compressionStats[extension] = 1;
    }

private:
    //Constants
    const String c_comprStatsFileName = u8"compression_stats.csv";

    //Members
	BinaryTreeMap<String, float32> compressionStats;
	Mutex compressionStatsLock;
};
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
//Class header
#include "CompressionStatistics.hpp"
#include "../InjectionContainer.hpp"
#include "ConfigManager.hpp"

//Constructor
CompressionStatistics::CompressionStatistics(const Path &path)
{
	//read in compression stats
	FileInputStream fileInputStream(path / this->c_comprStatsFileName);
	BufferedInputStream bufferedInputStream(fileInputStream);
	TextReader textReader(bufferedInputStream, TextCodecType::UTF8);
	CommonFileFormats::CSVReader csvReader(textReader, CommonFileFormats::csvDialect_excel);

	//skip first line
	String cell;
	csvReader.ReadCell(cell);
	csvReader.ReadCell(cell);

	//read lines
	String rate;
	while(!textReader.IsAtEnd())
	{
		csvReader.ReadCell(cell);
		csvReader.ReadCell(rate);

		this->compressionStats[cell] = rate.ToFloat32();
	}
}

//Public methods
void CompressionStatistics::AddCompressionRateSample(const String &fileExtension, float32 compressionRate)
{
	AutoLock lock(this->compressionStatsLock);

	compressionRate = Math::Clamp(compressionRate, 0.0f, 1.0f);

	String extLower = fileExtension.ToLowercase();
	this->compressionStats[extLower] = (this->compressionStats[extLower] + compressionRate) / 2.0f;
}

uint8 CompressionStatistics::GetCompressionLevel(float32 compressionRate) const
{
	const Config& config = InjectionContainer::Instance().Get<ConfigManager>().Config();

	float32 c = (config.maxCompressionLevel+1) * (1 - compressionRate);
	float32 compressionLevel = roundf(c) - 1;

	return (uint8)Math::Clamp(compressionLevel, 0.0f, 9.0f);
}

float32 CompressionStatistics::GetCompressionRate(const String &fileExtension)
{
	AutoLock lock(this->compressionStatsLock);

	String extLower = fileExtension.ToLowercase();
	if(!this->compressionStats.Contains(extLower))
		this->compressionStats[extLower] = 0; //assume at first that file is perfectly compressible

	return this->compressionStats[extLower];
}

void CompressionStatistics::Write(const Path &dirPath)
{
    FileOutputStream fileOutputStream(dirPath / this->c_comprStatsFileName, true);
    BufferedOutputStream bufferedOutputStream(fileOutputStream);
    CommonFileFormats::CSVWriter csvWriter(bufferedOutputStream, CommonFileFormats::csvDialect_excel);

    csvWriter << u8"File extension" << u8"Compression rate" << endl;
    for(const auto& kv: this->compressionStats)
    {
        csvWriter.WriteCell(kv.key);
        csvWriter.WriteCell(String::Number(kv.value));
        csvWriter.TerminateRow();
    }

    bufferedOutputStream.Flush();
}
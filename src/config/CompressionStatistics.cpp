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
#include "CompressionStatistics.hpp"

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
	while(true)
	{
		csvReader.ReadCell(cell);
		bool haveValue = csvReader.ReadCell(rate);

		if(haveValue)
			this->compressionStats[cell] = rate.ToFloat32();
		else
			break;
	}
}

//Public methods
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
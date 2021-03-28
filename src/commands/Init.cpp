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
#include <StdXX.hpp>
//Local
#include "../config/CompressionStatistics.hpp"
#include "../config/ConfigManager.hpp"

using namespace StdXX;

static void AddIncompressibleFileExtensions(CompressionStatistics& compressionStatistics)
{
    const String exts[] =
    {
        //archives
        u8"7z",
        u8"cab",
        u8"dmg",
        u8"gz",
        u8"rar",
        u8"zip",

        //audio
        u8"m4a",
        u8"mp3",

        //images
        u8"jpeg", u8"jpg",

        //video
        u8"avi",
        u8"bik",
        u8"flv",
        u8"m2ts",
        u8"mkv",
        u8"mpg",
        u8"mov",
        u8"mp4",
        u8"msi",
        u8"webm",
        u8"wmv",
        u8"vob",
    };

    for(const String& ext : exts)
    {
        compressionStatistics.SetAsIncompressible(ext);
    }
}

static bool IsDirectoryEmpty(const Path& dirPath)
{
	File dir(dirPath);
	return dir.IsEmptyDirectory();
}

int32 CommandInit(const Path& sourcePath)
{
	ConfigManager c(sourcePath);

	const Path &backupPath = c.Config().backupPath;

    //check if dir is empty
	if (!IsDirectoryEmpty(backupPath))
    {
        stdErr << u8"Directory is not empty. Can not create backup dir here..." << endl;
        return EXIT_FAILURE;
    }
    
    c.Write(backupPath);

    CompressionStatistics compressionStatistics;
    AddIncompressibleFileExtensions(compressionStatistics);
    compressionStatistics.Write(backupPath);

    //create dirs
	File dataDir(c.Config().dataPath);
	dataDir.CreateDirectories();

	File indexDir(c.Config().indexPath);
	indexDir.CreateDirectories();

    return EXIT_SUCCESS;
}
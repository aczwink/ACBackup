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
#include <Std++.hpp>
using namespace StdXX;
//Local
#include "../Config.hpp"

static bool IsDirectoryEmpty(const Path& dirPath)
{
	auto dir = OSFileSystem::GetInstance().GetDirectory(dirPath);
	return dir->IsEmpty();
}

int32 CommandInit(const Path& dirPath)
{
	//check if dir is empty
	if (!IsDirectoryEmpty(dirPath))
	{
		stdErr << u8"Directory is not empty. Can not create backup dir here..." << endl;
		return EXIT_FAILURE;
	}

	Config c;
	c.Write(dirPath);
}
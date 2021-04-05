/*
 * Copyright (c) 2020-2021 Amir Czwink (amir130@hotmail.de)
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
//Corresponding header
#include "Util.hpp"

void UnprotectFile(const Path& filePath)
{
	File file(filePath);
	FileInfo nodeInfo = file.Info();

	if(nodeInfo.permissions.IsInstanceOf<POSIXPermissions>())
	{
        POSIXPermissions& permissions = dynamic_cast<POSIXPermissions &>(*nodeInfo.permissions);

		permissions.owner.write = true;

		file.ChangePermissions(permissions);
	}
	else
	{
		NOT_IMPLEMENTED_ERROR; //implement me
	}
}

void WriteProtectFile(const Path& filePath)
{
	File file(filePath);
	FileInfo nodeInfo = file.Info();

	if(nodeInfo.permissions.IsInstanceOf<POSIXPermissions>())
	{
        POSIXPermissions& permissions = dynamic_cast<POSIXPermissions &>(*nodeInfo.permissions);

		permissions.others.write = false;
		permissions.group.write = false;
		permissions.owner.write = false;

		file.ChangePermissions(permissions);
	}
	else
	{
		NOT_IMPLEMENTED_ERROR; //implement me
	}
}
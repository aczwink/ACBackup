/*
 * Copyright (c) 2020 Amir Czwink (amir130@hotmail.de)
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
	return; //TODO: reintegrate this, also test if a snapshot can be created if the dir is locked (i.e. if unlocking and so on works)

	AutoPointer<FileSystemNode> node = OSFileSystem::GetInstance().GetNode(filePath);
	FileSystemNodeInfo nodeInfo = node->QueryInfo();

	if(nodeInfo.permissions.IsInstanceOf<Filesystem::UnixPermissions>())
	{
		Filesystem::UnixPermissions& permissions = dynamic_cast<Filesystem::UnixPermissions &>(*nodeInfo.permissions);

		permissions.owner.write = true;

		node->ChangePermissions(permissions);
	}
	else
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
	}
}

void WriteProtectFile(const Path& filePath)
{
	return; //TODO: reintegrate this, also test if a snapshot can be created if the dir is locked (i.e. if unlocking and so on works)

	AutoPointer<FileSystemNode> node = OSFileSystem::GetInstance().GetNode(filePath);
	FileSystemNodeInfo nodeInfo = node->QueryInfo();

	if(nodeInfo.permissions.IsInstanceOf<Filesystem::UnixPermissions>())
	{
		Filesystem::UnixPermissions& permissions = dynamic_cast<Filesystem::UnixPermissions &>(*nodeInfo.permissions);

		permissions.others.write = false;
		permissions.group.write = false;
		permissions.owner.write = false;

		node->ChangePermissions(permissions);
	}
	else
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
	}
}
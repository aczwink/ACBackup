/*
 * Copyright (c) 2021 Amir Czwink (amir130@hotmail.de)
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

class TestBackupCreator
{
public:
	//Constructor
	inline TestBackupCreator()
	{
		File sourceDir(this->tempDirectory.Path() / String(u8"source"));
		File backupDir(this->tempDirectory.Path() / String(u8"backuptarget"));

		sourceDir.CreateDirectory();
		backupDir.CreateDirectory();

		int32 result = CommandInit(backupDir.Path(), sourceDir.Path());
		ASSERT_EQUALS(EXIT_SUCCESS, result);

		this->configManager = new ConfigManager(backupDir.Path());

		InjectionContainer &ic = InjectionContainer::Instance();
		ic.Config(this->configManager->Config());

		ic.StatusTracker(new StatusTracker);
	}

	//Destructor
	inline ~TestBackupCreator()
	{
		File dir(this->tempDirectory.Path());
		dir.RemoveChildrenRecursively();

		InjectionContainer &ic = InjectionContainer::Instance();
		ic.UnregisterAll();
	}

private:
	//Members
	TempDirectory tempDirectory;
	UniquePointer<ConfigManager> configManager;
};
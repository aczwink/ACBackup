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
//Class header
#include "StatusTracker.hpp"
//Local
#include "TerminalStatusTracker.hpp"
#include "WebStatusTracker.hpp"
#include "../InjectionContainer.hpp"
#include "../config/ConfigManager.hpp"

//Class functions
StatusTracker *StatusTracker::CreateInstance(StatusTrackerType type)
{
	switch(type)
	{
		case StatusTrackerType::Terminal:
			return new TerminalStatusTracker;
		case StatusTrackerType::Web:
			return new WebStatusTracker(InjectionContainer::Instance().Config().statusTrackerPort);
	}
	return nullptr;
}

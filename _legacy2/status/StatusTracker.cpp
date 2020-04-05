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
//Class header
#include "StatusTracker.hpp"

static StatusTracker* g_tracker = nullptr;
static int32 NetworkThreadMain()
{
	stdOut << u8"Listening on 0.0.0.0:" << g_tracker->GetPort() << endl;
	g_tracker->GetServer().Serve();
	return EXIT_SUCCESS;
}

//Constructor
StatusTracker::StatusTracker(uint16 port) : httpServer(*this, port), thread(NetworkThreadMain)
{
	g_tracker = this;
	this->thread.Start();
}

//Destructor
StatusTracker::~StatusTracker()
{
	Sleep(uint64(2) * 1000 * 1000 * 1000); //Wait so that status trackers can get the result
	stdOut << u8"Shutting down server..." << endl;
	this->httpServer.Shutdown();
	this->thread.Join();
	stdOut << u8"Server has been shut down!" << endl;
}

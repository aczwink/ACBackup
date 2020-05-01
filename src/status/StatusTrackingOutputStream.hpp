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
#include <StdXX.hpp>
using namespace StdXX;
//Local
#include "ProcessStatus.hpp"

class StatusTrackingOutputStream : public OutputStream
{
public:
	//Constructor
	inline StatusTrackingOutputStream(OutputStream& outputStream, ProcessStatus& processStatus)
		: outputStream(outputStream), processStatus(processStatus)
	{
	}

	//Methods
	void Flush() override;
	uint32 WriteBytes(const void *source, uint32 size) override;

private:
	//Members
	OutputStream& outputStream;
	ProcessStatus& processStatus;
};
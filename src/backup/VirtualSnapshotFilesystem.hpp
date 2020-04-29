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
#include "Snapshot.hpp"

class VirtualSnapshotFilesystem : public ReadableFileSystem
{
public:
	//Constructor
	inline VirtualSnapshotFilesystem(const Snapshot& snapshot) : snapshot(snapshot)
	{
	}

	//Methods
	bool Exists(const Path &path) const override;
	AutoPointer<const Node> GetNode(const Path &path) const override;
	SpaceInfo QuerySpace() const override;

private:
	//Members
	const Snapshot& snapshot;
};
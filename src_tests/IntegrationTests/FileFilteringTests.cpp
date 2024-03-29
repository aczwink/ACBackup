/*
 * Copyright (c) 2022 Amir Czwink (amir130@hotmail.de)
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
#include <StdXXTest.hpp>
//Local
#include "../../src/indexing/OSFileSystemNodeIndex.hpp"
#include "TestBackupCreator.hpp"
//Namespaces
using namespace StdXX;

TEST_SUITE(FileFilteringTests)
{
    TEST_CASE(FilterTests)
    {
        TestBackupCreator testBackupCreator;

        Path path = String(u8"../testfiles");
        OSFileSystemNodeIndex index(path);

        ASSERT_EQUALS(1, index.GetNumberOfNodes());
    }
};
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
//Local
#include <Std++.hpp>

//Constants
static const char *const c_hashAlgorithm_sha512_256 = u8"sha512/256";

namespace StdXX::Serialization
{
	template <typename ArchiveType>
	void CustomArchive(ArchiveType& ar, const String& name, Crypto::HashAlgorithm& hashAlgorithm)
	{
		StaticArray<Tuple<Crypto::HashAlgorithm, String>, 1> hashAlgorithmMapping = { {
			{ Crypto::HashAlgorithm::SHA512_256, c_hashAlgorithm_sha512_256}
		} };
		ar & Binding(name, StringMapping(hashAlgorithm, hashAlgorithmMapping));
	}
}
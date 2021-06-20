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
//Local
#include <StdXX.hpp>

//Constants
static const char8_t *const c_hashAlgorithm_sha512_256 = u8"sha512/256";

namespace StdXX::Serialization
{
	inline StaticArray<Tuple<Crypto::HashAlgorithm, String>, 2> HashMapping()
	{
		return { {
			{ Crypto::HashAlgorithm::MD5, u8"md5" },
			{ Crypto::HashAlgorithm::SHA512_256, c_hashAlgorithm_sha512_256 },
		} };
	}

	template <typename ArchiveType>
	void CustomArchive(ArchiveType& ar, const String& name, CompressionSetting& compressionSetting)
	{
		StaticArray<Tuple<CompressionSetting, String>, 1> settingMapping = { {
			{ CompressionSetting::lzma, u8"lzma"}
		} };
		ar & Binding(name, StringMapping(compressionSetting, settingMapping));
	}

	template <typename ArchiveType>
	inline ArchiveType& operator<<(ArchiveType& serializer, const Binding<CompressionSetting>& binding)
	{
		CustomArchive(serializer, binding.name, binding.value);
		return serializer;
	}

	template <typename ArchiveType>
	inline ArchiveType& operator>>(ArchiveType& deserializer, const Binding<CompressionSetting>& binding)
	{
		CustomArchive(deserializer, binding.name, binding.value);
		return deserializer;
	}

	template <typename ArchiveType>
	void CustomArchive(ArchiveType& ar, const String& name, Crypto::HashAlgorithm& hashAlgorithm)
	{
		ar & Binding(name, StringMapping(hashAlgorithm, HashMapping()));
	}


	//clang needs this :(
    inline XmlSerializer& operator<<(XmlSerializer& serializer, const Binding<CompressionSetting>& binding)
    {
        return operator<<<XmlSerializer>(serializer, binding);
    }
    inline XmlDeserializer& operator>>(XmlDeserializer& deserializer, const Binding<CompressionSetting>& binding)
    {
        return operator>><XmlDeserializer>(deserializer, binding);
    }
}
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
#pragma once
#include <Std++.hpp>
using namespace StdXX;

const uint8 c_masterKey_saltSize = 192;
const uint8 c_subKeyDerive_saltSize = 128;
const uint8 AES_BLOCK_SIZE = 16;
const uint8 AES_KEY_SIZE = 32;

struct EncryptionInfo
{
	FixedArray<uint8> masterKey;
	FixedArray<uint8> subKeySalt;
};

Tuple<FixedArray<uint8>, FixedArray<uint8>> DeriveAppKey();
void DeriveFileKey(const EncryptionInfo& encryptionInfo, const String& fileName, uint8* key, uint8* iv);
void DeriveFileDataKey(const EncryptionInfo& encryptionInfo, const String& fileName, uint8* key);
FixedArray<uint8> DeriveMasterKey(const String& masterPassword, const uint8* salt, uint8 saltSize);
void GenerateSafeRandomBytes(uint8* destination, uint8 nBytes);
FixedArray<uint8> GenerateVerificationMessage();
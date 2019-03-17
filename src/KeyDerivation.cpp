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
//Corresponding header
#include "KeyDerivation.hpp"
//Namespaces
using namespace StdXX::Crypto;

Tuple<FixedArray<uint8>, FixedArray<uint8>> DeriveAppKey()
{
	String appPw = u8"W,W.n)zPy2f,Af77W#CdKTapY)[Y$K{Lhg,_cpMDF7V)4625{Lrsr?#ayX=e{X9]";
	appPw.ToUTF8();

	//hash the pw
	UniquePointer<HashFunction> hashFunction = HashFunction::CreateInstance(HashAlgorithm::SHA512);
	hashFunction->Update(appPw.GetRawData(), appPw.GetSize());
	hashFunction->Finish();
	FixedArray<uint8> digest = hashFunction->GetDigest();

	//gen key
	FixedArray<uint8> keyAndIV(AES_KEY_SIZE + AES_BLOCK_SIZE);
	Crypto::PBKDF2(hashFunction->GetDigestString() + appPw, &digest[0], digest.GetNumberOfElements(), HashAlgorithm::SHA256, 1000, &keyAndIV[0], keyAndIV.GetNumberOfElements());

	//split into key and iv
	FixedArray<uint8> key(AES_KEY_SIZE);
	MemCopy(&key[0], &keyAndIV[0], AES_KEY_SIZE);
	FixedArray<uint8> iv(AES_BLOCK_SIZE);
	MemCopy(&iv[0], &keyAndIV[AES_KEY_SIZE], AES_BLOCK_SIZE);

	return {Move(key), Move(iv)};
}

void DeriveFileKey(const EncryptionInfo& encryptionInfo, const String& fileName, uint8* key, uint8* iv)
{
	fileName.ToUTF8();

	uint8 derived[AES_KEY_SIZE + AES_BLOCK_SIZE];
	HKDF(&encryptionInfo.masterKey[0], static_cast<uint8>(encryptionInfo.masterKey.GetNumberOfElements()), &encryptionInfo.subKeySalt[0],
		 static_cast<uint8>(encryptionInfo.subKeySalt.GetNumberOfElements()), fileName.GetRawData(),
		 static_cast<uint8>(fileName.GetSize()), HashAlgorithm::SHA512, derived, AES_KEY_SIZE + AES_BLOCK_SIZE);

	MemCopy(key, derived, AES_KEY_SIZE);
	MemCopy(iv, &derived[AES_KEY_SIZE], AES_BLOCK_SIZE);
}

void DeriveFileDataKey(const EncryptionInfo &encryptionInfo, const String &fileName, uint8 *key)
{
	fileName.ToUTF8();

	HKDF(&encryptionInfo.masterKey[0], static_cast<uint8>(encryptionInfo.masterKey.GetNumberOfElements()), &encryptionInfo.subKeySalt[0],
		 static_cast<uint8>(encryptionInfo.subKeySalt.GetNumberOfElements()), fileName.GetRawData(),
		 static_cast<uint8>(fileName.GetSize()), HashAlgorithm::SHA512, key, AES_KEY_SIZE);
}

FixedArray<uint8> DeriveMasterKey(const String& masterPassword, const uint8* salt, uint8 saltSize)
{
	masterPassword.ToUTF8(); //make sure it is utf8 encoded

	//generate salt and pepper
	String pepper = u8"?_FCSqsb.`:,4T@W8KQ5xaR}F`!Lx)=kPj[='<\"=n)`{Fuw2V$pS&qC.$Q9X}y:DPdFb?4dwhFb]C2K3L9.Vmqr?7kmh]$Z%2+(Z2*F'N^Jqc,5nh2.2\"kCv4D&$e,88";
	pepper.ToUTF8();

	FixedArray<uint8> saltAndPepper(saltSize + pepper.GetSize());
	MemCopy(&saltAndPepper[0], salt, saltSize);
	MemCopy(&saltAndPepper[saltSize], pepper.GetRawData(), pepper.GetSize());

	//generate master key
	FixedArray<uint8> masterKey(AES_KEY_SIZE);
	Crypto::scrypt(masterPassword, &saltAndPepper[0], saltAndPepper.GetNumberOfElements(), &masterKey[0], masterKey.GetNumberOfElements(), 10); //TODO: reset cost factor to 20

	return masterKey;
}

void GenerateSafeRandomBytes(uint8* destination, uint8 nBytes)
{
	for(uint8 i = 0; i < nBytes; i++)
	{
		destination[i] = 0xAB; //TODO: add randomization
	}
}

FixedArray<uint8> GenerateVerificationMessage()
{
	//generate random message
	constexpr uint8 verificationMsgSize = 32;
	uint8 verificationMsg[verificationMsgSize];

	for(uint8 i = 0; i < verificationMsgSize; i++)
	{
		verificationMsg[i] = 0xCD; //TODO: add randomization
	}

	//hash message
	UniquePointer<HashFunction> hashFunction = HashFunction::CreateInstance(HashAlgorithm::SHA256);
	hashFunction->Update(verificationMsg, verificationMsgSize);
	hashFunction->Finish();
	FixedArray<uint8> digest = hashFunction->GetDigest();

	//generate full verification token
	FixedArray<uint8> verificationToken(verificationMsgSize + digest.GetNumberOfElements());
	MemCopy(&verificationToken[0], verificationMsg, verificationMsgSize);
	MemCopy(&verificationToken[verificationMsgSize], &digest[0], digest.GetNumberOfElements());

	return verificationToken;
}
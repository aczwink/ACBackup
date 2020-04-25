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
//Class header
#include "FlatVolumesFile.hpp"
//Local
#include "FlatVolumesBlockInputStream.hpp"
#include "../InjectionContainer.hpp"
#include "../config/ConfigManager.hpp"

//Public methods
UniquePointer<InputStream> FlatVolumesFile::OpenForReading(bool verify) const
{
	UniquePointer<InputStream> blockInputStream = new FlatVolumesBlockInputStream(this->fileSystem, this->attributes.Blocks());
	ChainedInputStream* chain = new ChainedInputStream(Move(blockInputStream));
	chain->Add( new BufferedInputStream(chain->GetEnd()) );

	if(this->attributes.CompressionSetting().HasValue())
	{
		CompressionSettings compressionSettings;
		ConfigManager::GetCompressionSettings(*this->attributes.CompressionSetting(), compressionSettings);
		chain->Add(Decompressor::Create(compressionSettings.compressionStreamFormatType, chain->GetEnd(), verify));
	}

	if(verify)
	{
		const Config &config = InjectionContainer::Instance().Get<ConfigManager>().Config();
		Crypto::HashAlgorithm hashAlgorithm = config.hashAlgorithm;
		String expected = this->attributes.Hash(hashAlgorithm);
		chain->Add(new Crypto::CheckedHashingInputStream(chain->GetEnd(), hashAlgorithm, expected));
	}

	return chain;
}

UniquePointer<OutputStream> FlatVolumesFile::OpenForWriting()
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return UniquePointer<OutputStream>();
}

void FlatVolumesFile::ChangePermissions(const NodePermissions &newPermissions)
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
}

NodeInfo FlatVolumesFile::QueryInfo() const
{
	return this->index.GetFileSystemNodeInfo(this->fileIndex);
}

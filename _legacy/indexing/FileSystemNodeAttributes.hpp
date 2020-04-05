
//Local
#include "../Config.hpp"

class FileSystemNodeAttributes
{
public:
	//Overrideable operators
	virtual bool operator==(const FileSystemNodeAttributes& rhs) const;

	//Inline operators
	inline bool operator!=(const FileSystemNodeAttributes& rhs) const
	{
		return !(*this == rhs);
	}

	//Inline
	inline const String& GetDigest(Crypto::HashAlgorithm hashAlgorithm) const
	{
		return this->hashes[hashAlgorithm];
	}

	Map<Crypto::HashAlgorithm, String> hashes; //lower-case-hex-string of digest; only valid for files
};
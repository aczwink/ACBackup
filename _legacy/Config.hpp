class Config
{
public:
	//Properties
	inline uint64 CompressionMemoryLimit() const
	{
		return this->compressionMemLimit;
	}

	inline Crypto::HashAlgorithm HashAlgorithm() const
	{
		return this->hashAlgorithm;
	}

	inline int8 MaxCompressionLevel() const
	{
		return this->maxCompressionLevel;
	}

private:
	//Members
	int8 maxCompressionLevel;
	uint64 compressionMemLimit;
	Crypto::HashAlgorithm hashAlgorithm;

	//Methods
	void WriteCompressionPart(TextWriter& textWriter) const;
};
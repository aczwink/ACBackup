//Constructor
Config::Config()
{
	this->maxCompressionLevel = 3;
	this->compressionMemLimit = 0;
}

Config::Config(const Path &dirPath)
{
	ASSERT(cfgMap.Contains(u8"enableCompression"), u8"REPORT THIS PLEASE!");
	if(cfgMap[u8"enableCompression"] == false)
	{
		this->maxCompressionLevel = -1;
	}
	else
	{
		ASSERT(cfgMap.Contains(u8"maxCompressionLevel"), u8"REPORT THIS PLEASE!");
		this->maxCompressionLevel = Math::Clamp((int8)cfgMap[u8"maxCompressionLevel"].NumberValue(), 0_i8, 9_i8);
	}

	ASSERT(cfgMap.Contains(u8"compressionMemLimit"), u8"REPORT THIS PLEASE!");
	this->compressionMemLimit = static_cast<uint64>(cfgMap[u8"compressionMemLimit"].NumberValue());
}

//Public methods
void Config::Write(const Path &dirPath)
{
	this->WriteCompressionPart(textWriter);
}

//Private methods
void Config::WriteCompressionPart(TextWriter &textWriter) const
{
	bool enableCompression = this->maxCompressionLevel != -1;
	this->WriteConfigValue(textWriter, 1, u8"enableCompression", enableCompression, u8"Enable or disable compression");
	this->WriteConfigValue(textWriter, 1, u8"compressionMemLimit", this->compressionMemLimit, u8"Limit in MiB for files that are compressed in memory rather than streamed. Defaults to 0 (i.e. never compress in memory)");
	this->WriteConfigValue(textWriter, 1, u8"maxCompressionLevel", this->maxCompressionLevel, u8"Set the maximum compression level. A number between 0 to 9");
}
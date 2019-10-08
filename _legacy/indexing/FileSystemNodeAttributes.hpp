
//Local
#include "../Config.hpp"

enum class IndexableNodeType
{
	File,
	Link
};

class FileSystemNodeAttributes
{
public:
	//Constructors
	FileSystemNodeAttributes(const FileSystemNodeAttributes& attributes) = default; //copy ctor

	inline FileSystemNodeAttributes(const AutoPointer<const File>& file, const Config& config) : config(config)
	{
		this->type = IndexableNodeType::File;
		this->Init(file);
		this->size = file->GetSize();
	}

	inline FileSystemNodeAttributes(const AutoPointer<const Link>& link, const Path& target, const Config& config) : config(config)
	{
		this->type = IndexableNodeType::Link;
		this->target = target;
		this->Init(link);
	}

	//Overrideable operators
	virtual bool operator==(const FileSystemNodeAttributes& rhs) const;

	//Inline operators
	inline bool operator!=(const FileSystemNodeAttributes& rhs) const
	{
		return !(*this == rhs);
	}

	//Properties
	inline uint64 Size() const
	{
		ASSERT(this->type == IndexableNodeType::File, u8"Only files have a size!");
		return this->size;
	}

	inline IndexableNodeType Type() const
	{
		return this->type;
	}

	//Inline
	inline const String& GetDigest(Crypto::HashAlgorithm hashAlgorithm) const
	{
		return this->hashes[hashAlgorithm];
	}

protected:
	//Members
	const Config& config;

private:
	//Members
	IndexableNodeType type;
	Optional<DateTime> lastModifiedTime;
	Path target; //only valid for links
	uint64 size; //only valid for files
	Map<Crypto::HashAlgorithm, String> hashes; //lower-case-hex-string of digest; only valid for files

	//Inline
	inline void Init(const AutoPointer<const FileSystemNode>& node)
	{
		FileSystemNodeInfo info = node->QueryInfo();
		if(info.lastModifiedTime.HasValue())
			this->lastModifiedTime = info.lastModifiedTime->dt;
	}
};
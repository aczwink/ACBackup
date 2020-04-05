class ProcessStatus
{
	//Methods
	CommonFileFormats::JsonValue ToJSON() const;

	//Inline
	inline void ReduceTotalSize(uint64 size)
	{
		AutoLock lock(this->mutex);
		this->totalSize -= size;
	}
};
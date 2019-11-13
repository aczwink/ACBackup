class ProcessStatus
{
	//Methods
	CommonFileFormats::JsonValue ToJSON() const;

	//Inline
	inline void AddFinishedSize(uint64 size)
	{
		AutoLock lock(this->mutex);
		this->doneSize += size;
	}

	inline void ReduceTotalSize(uint64 size)
	{
		AutoLock lock(this->mutex);
		this->totalSize -= size;
	}
};
class ProcessStatus
{

	inline ProcessStatus(const String& title, uint32 nFiles, uint64 totalSize)
			: title(title), nFiles(nFiles), totalSize(totalSize), startTime(DateTime::Now())
	{
		this->isEndDeterminate = true;
		this->nFinishedFiles = 0;
		this->doneSize = 0;

		this->clock.Start();
	}

	//Methods
	CommonFileFormats::JsonValue ToJSON() const;

	//Inline
	inline void AddFinishedSize(uint64 size)
	{
		AutoLock lock(this->mutex);
		this->doneSize += size;
	}

	inline void Finished()
	{
		AutoLock lock(this->mutex);
		this->endTime = DateTime::Now();
		this->totalTaskDuration = this->clock.GetElapsedMicroseconds();
	}

	inline void IncFileCount()
	{
		AutoLock lock(this->mutex);
		this->nFiles++;
	}

	inline void IncFinishedCount()
	{
		AutoLock lock(this->mutex);
		this->nFinishedFiles++;
	}

	inline void ReduceTotalSize(uint64 size)
	{
		AutoLock lock(this->mutex);
		this->totalSize -= size;
	}


	Clock clock;
	mutable Mutex mutex;

	Optional<DateTime> endTime;
	uint64 totalTaskDuration;
};
//Constructor
Snapshot::Snapshot(const Path& path, const Optional<EncryptionInfo>& encryptionInfo)
{
	DateTime dateTime = DateTime::Now();
	String snapshotName = u8"snapshot_" + dateTime.GetDate().ToISOString() + u8"_";
	snapshotName += String::Number(dateTime.GetTime().GetHour(), 10, 2) + u8"_" + String::Number(dateTime.GetTime().GetMinute(), 10, 2) + u8"_" + String::Number(dateTime.GetTime().GetSecond(), 10, 2);

	this->name = snapshotName;
	this->prev = nullptr;
	this->index = new FlatContainerIndex(path / snapshotName, encryptionInfo);
}

//Public methods
BackupContainerIndex *Snapshot::FindDataIndex(uint32 fileIndex) const
{
	NOT_IMPLEMENTED_ERROR; //TODO: implement me
	return nullptr;
}
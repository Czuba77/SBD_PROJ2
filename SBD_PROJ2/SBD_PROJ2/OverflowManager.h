#pragma once
#include <string>
class Record;
class DBM;
class IndexFile;
class OverflowManager;

class OverflowManager
{
private:
	int* V_numberOfRecordsOverflow;
	int blockingFactor;
	std::string fileName;
	Record* records;
	DBM* dbm;
	int offsetOfCurrentRecord;
public:
	OverflowManager(int* v, int bf, std::string fn, DBM* dbm);
	~OverflowManager();
	int addOverflowRecord(Record record, int offsetOfEarlierRecord);
	Record ReadOverflowRecord(int pointer);
	Record* ReadOverflowRecord(int pointer, bool modify);
	void readPageBuffer(int offset);
	void writePageBuffer();
	int isOffsetInOverflowBuffer(int offsetOfSearchedRec);
	void clearPageBuffer();
	void putToEOF(Record record);
	void recSendRecordsToReorg(int pointer, PageBuffer* reorgPage);
	bool deletionOfOverflowRecord(int key, int pointer);
	bool modificationOfOverflowRecord(int key, int pointer);

	bool automatedModify(int key, int pointer, char* newData);
};


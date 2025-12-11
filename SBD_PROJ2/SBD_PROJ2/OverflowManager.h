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
	int offsetOfCurrentRecord;
public:
	OverflowManager(int* v, int bf, std::string fn);
	~OverflowManager();
	int addOverflowRecord(Record record);
	int addOverflowRecord(Record record, int offsetOfEarlierRecord);
	Record ReadOverflowRecord(int pointer);
	void readPageBuffer(int offset);
	void writePageBuffer();
	int isOffsetInOverflowBuffer(int offsetOfSearchedRec);
	void clearPageBuffer();
	void putToEOF(Record record);
};


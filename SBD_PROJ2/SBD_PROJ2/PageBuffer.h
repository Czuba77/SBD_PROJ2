#pragma once

class Record;
class DBM;
class IndexFile;
class OverflowManager;
#include <string>

class PageBuffer {
private:
	DBM* dbm;
	IndexFile* indexFile;
	OverflowManager* overflowManager;
	Record* records;
	int recordsInBuffer = 0;
	int blockingFactor;
	int alpha_pageUtill;
	int pageNumber;
	int diskPageCap;
	std::string fileName;
public:
	PageBuffer();
	void setPageBuffer(DBM* db, IndexFile* indexF, OverflowManager* om, int bf, int apu, int pn, int dpc, std::string fn);
	PageBuffer(DBM* db, IndexFile* indexF, OverflowManager* om, int bf, int apu, int pn, int dpc,std::string fn);
	~PageBuffer();
	void readPageBuffer();
	void writePageBuffer();
	bool writePageBuffer(bool changingFirst);
	int addRecordToPageBuffer(Record rec);
	void populateBuffer();
	void clearPageBuffer();
	void chooseAndLoadPage(int pn);
	void sendPageRecordsToReorg(PageBuffer* reorgPage);
	void recievePageRecords(Record rec);
	bool deletionOfRecord(int key);
	bool modificationOfRecord(int key);

	bool automatedModify(int key, char* newData);

	void setPageNumber(int pn) { pageNumber = pn; }
};
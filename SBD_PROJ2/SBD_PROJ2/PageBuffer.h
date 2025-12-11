#pragma once

class Record;
class DBM;
class IndexFile;
class OverflowManager;

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
	PageBuffer(DBM* db, IndexFile* indexF, int bf, int apu, int pn, int dpc,std::string fn);
	~PageBuffer();
	void readPageBuffer();
	void writePageBuffer();
	void addRecordToPageBuffer(Record rec);
	void populateBuffer();
	static char* genRegisterPlate();
};
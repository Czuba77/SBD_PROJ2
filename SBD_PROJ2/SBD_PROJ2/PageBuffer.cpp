#include "IndexFile.h"
#include "PageBuffer.h"
#include "Record.h"
#include "DBM.h"
#include "OverflowManager.h"


PageBuffer::PageBuffer(DBM* db, IndexFile* indexF,int bf, int apu,int pn, int dpc, std::string fn)
{
	dbm = db;
	indexFile = indexF;
	blockingFactor = bf;
	alpha_pageUtill = apu;
	records = new Record[bf];
	for(int i=0;i<bf;i++){
		records[i]=Record();
	}
	diskPageCap = dpc;
	pageNumber = pn;
	fileName = fn;
}

PageBuffer::~PageBuffer()
{
	dbm=nullptr;
	delete[] records;
}

void PageBuffer::addRecordToPageBuffer(Record rec)
{
	for(int i=0;i< blockingFactor;i++){
		if(records[i].getKey() == -1) {
			records[i]=rec;
			return;
		}
		else if(records[i].getKey() > rec.getKey()){
			records[i-1].setPointer(overflowManager->addOverflowRecord(rec,(records[i - 1].getPointer())));
			return;
		}
	}
	if(records[blockingFactor -1].getKey() < rec.getKey()){
		records[blockingFactor - 1].setPointer(overflowManager->addOverflowRecord(rec, records[blockingFactor -1].getPointer()));
	}
}

void PageBuffer::readPageBuffer()
{
	std::fstream f;
	f.open(fileName, std::ios::binary | std::ios::in);
	f.seekg(pageNumber * diskPageCap, std::ios::beg);
	for (int i = 0; i < blockingFactor; i++) {
		records[i].readRecord(&f);
	}
	f.close();
}

void PageBuffer::writePageBuffer()
{
	std::fstream f;
	f.open(fileName, std::ios::binary | std::ios::out | std::ios::in);
	f.seekp(pageNumber *diskPageCap, std::ios::beg);
	indexFile->addEntry(records[0].getKey(), pageNumber, 0);
	for (int i = 0; i < blockingFactor; i++) {
		records[i].writeRecord(&f);
	}
	f.close();
}

void PageBuffer::populateBuffer()
{	
	int randval=0;
	Record* rec;
	for (int i = 0; i < alpha_pageUtill; i++) {
		randval += rand() % 3;
		rec = new Record();
		rec->setRecord(randval, "XXXYYYYY", -1, false);
	}
}

char* PageBuffer::genRegisterPlate()
{
	char* plate = new char[9];
	const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	for (int i = 0; i < 8; i++) {
		int key = rand() % (sizeof(charset) - 1);
		plate[i] = charset[key];
	}
	plate[8] = '\0';
	return plate;
}

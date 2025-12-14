#include "IndexFile.h"
#include "PageBuffer.h"
#include "Record.h"
#include "DBM.h"
#include "OverflowManager.h"
#include <iostream>


PageBuffer::PageBuffer()
{
	dbm = nullptr;
	indexFile = nullptr;
	blockingFactor = 0;
	alpha_pageUtill = 0;
	records = nullptr;
	diskPageCap = 0;
	pageNumber = 0;
	fileName = "";
}

void PageBuffer::setPageBuffer(DBM* db, IndexFile* indexF, OverflowManager* om, int bf, int apu, int pn, int dpc, std::string fn)
{
	dbm = db;
	indexFile = indexF;
	overflowManager = om;
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

PageBuffer::PageBuffer(DBM* db, IndexFile* indexF, OverflowManager* om,int bf, int apu,int pn, int dpc, std::string fn)
{
	dbm = db;
	indexFile = indexF;
	overflowManager = om;
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
	delete[] records;
}

//1 if added to page, 0 if to overflow -1 if error
int PageBuffer::addRecordToPageBuffer(Record rec)
{
	for(int i=0;i< blockingFactor;i++){
		if(records[i].getKey() == -1) {
			records[i]=rec;
			if(i==0)
				writePageBuffer(true); // changing first record
			else
				writePageBuffer();
			return true;
		}
		else if(records[i].getKey() == rec.getKey())
			return -1; // duplicate key error
		else if (records[i].getKey() > rec.getKey() && i == 0) {
			Record temp = records[i];
			records[i] = rec;
			int pointerTemp = overflowManager->addOverflowRecord(temp, records[i].getPointer());
			if (pointerTemp == -2) {
				return -1; // duplicate key error
			}
			records[i].setPointer(pointerTemp);
			writePageBuffer(true); 
			return false;
		}
		else if (records[i].getKey() > rec.getKey()) { //i != -1 bc of how we fill the page
			int pointerTemp = overflowManager->addOverflowRecord(rec, (records[i - 1].getPointer()));
			if (pointerTemp == -2) {
				return -1; // duplicate key error
			}
			if(records[i-1].getPointer() != pointerTemp){
				records[i - 1].setPointer(pointerTemp);
				writePageBuffer();
			}
			return false;
		}
	}
	if(records[blockingFactor -1].getKey() < rec.getKey()){
		int pointerTemp = overflowManager->addOverflowRecord(rec, records[blockingFactor - 1].getPointer());
		if(pointerTemp == -2){
			return -1; // duplicate key error
		}
		if(records[blockingFactor -1].getPointer() != pointerTemp){
			records[blockingFactor - 1].setPointer(pointerTemp);
			writePageBuffer();
		}
		return false;
	}
	return -2;
}

void PageBuffer::readPageBuffer()
{
	std::fstream f;
	f.open(fileName, std::ios::binary | std::ios::in);
	f.seekg(pageNumber * diskPageCap, std::ios::beg);
	DBM::diskReads++;
	for (int i = 0; i < blockingFactor; i++) {
		records[i].readRecord(&f);
	}
	f.close();
}

void PageBuffer::writePageBuffer()
{
	std::fstream f;
	f.open(fileName, std::ios::binary | std::ios::out | std::ios::in);
	f.seekp(pageNumber * diskPageCap, std::ios::beg);
	DBM::diskWrites++;
	for (int i = 0; i < blockingFactor; i++) {
		records[i].writeRecord(&f);
	}
	f.close();
}

bool PageBuffer::writePageBuffer(bool changingFirst)
{
	if(records[0].getKey() == -1){
		return false;
	}
	std::fstream f;
	f.open(fileName, std::ios::binary | std::ios::out | std::ios::in);
	f.seekp(pageNumber * diskPageCap, std::ios::beg);
	DBM::diskWrites++;
	indexFile->addEntry(records[0].getKey(), pageNumber, pageNumber * diskPageCap);
	for (int i = 0; i < blockingFactor; i++) {
		records[i].writeRecord(&f);
	}
	f.close();
	return true;
}

void PageBuffer::clearPageBuffer()
{
	for (int i = 0; i < blockingFactor; i++) {
		records[i] = Record();
	}
}

void PageBuffer::populateBuffer()
{	
	clearPageBuffer();

    int randval = pageNumber * 30;

    for (int i = 0; i < alpha_pageUtill; i++) {
        randval += rand() % 3 + 1;
        char* data = dbm->genData();
        records[i].setRecord(randval, data, -1, false);
        delete[] data;
    }

    writePageBuffer(); 

    if (blockingFactor > 0 && records[0].getKey() != -1) {
        indexFile->addEntry(records[0].getKey(), pageNumber, pageNumber * diskPageCap);
    }
}

void PageBuffer::chooseAndLoadPage(int pn)
{
	if (this->pageNumber == pn) {
		return;
	}
	pageNumber = pn;
	readPageBuffer();
}

void PageBuffer::sendPageRecordsToReorg(PageBuffer* reorgPage)
{
	Record tmp;
	for (int i = 0; i < blockingFactor; i++) {
		if (records[i].getKey() != -1) {
			tmp = records[i];
			if(tmp.getPointer() != -1){
				int pointer = tmp.getPointer();
				tmp.setPointer(-1);
				reorgPage->recievePageRecords(tmp);
				overflowManager->recSendRecordsToReorg(pointer, reorgPage);
			}
			else{
				reorgPage->recievePageRecords(tmp);
			}
		}
	}
}

void PageBuffer::recievePageRecords(Record rec)
{
	if(rec.getIsDeleted()==true){
		return;
	}
	records[recordsInBuffer++] = rec;
	dbm->incrementReorgRecordCount();
	if(recordsInBuffer == alpha_pageUtill){
		writePageBuffer(true);
		recordsInBuffer=0;
		pageNumber++;
		clearPageBuffer();
		dbm->incrementReorgPageCount();
	}
}


bool PageBuffer::deletionOfRecord(int key)
{
	for(int i=0;i< blockingFactor;i++){
		if(records[i].getKey() == key){
			records[i].setIsDeleted(true);
			writePageBuffer();
			return true;
		}
		else if (records[i].getKey() > key) { //i != -1 bc of how we fill the page
			if(overflowManager->deletionOfOverflowRecord(key, (records[i - 1].getPointer())))
				return true;
			else
				return false;
		}
	}
	if (records[blockingFactor - 1].getKey() <= key) {
		if(overflowManager->deletionOfOverflowRecord(key, records[blockingFactor - 1].getPointer()))
			return true;
		else
			return false;
	}
	

	return false;
}

bool PageBuffer::modificationOfRecord(int key)
{
	for (int i = 0; i < blockingFactor; i++) {
		if (records[i].getKey() == key) {

			if (records[i].getIsDeleted()) {
				char choice;
				std::cout << "Record with key " << key << " is flagged as DELETED.\n";
				std::cout << "Do you want to restore it before editing? (y/n): ";
				std::cin >> choice;

				if (choice == 'y' || choice == 'Y') {
					records[i].setIsDeleted(false);
					dbm->incrementMainRecordCount();
					writePageBuffer();
					std::cout << "Record restored. Proceeding to edit.\n";
				}
				else {
					std::cout << "Editing of the deleted record cancelled.\n";
					return true;
				}
			}

			char* newData;
			std::cout << "Modifying data for record with key: " << records[i].getKey() << "\n";

			newData = dbm->manualPlateCreation();

			records[i].setRecord(records[i].getKey(), newData, records[i].getPointer(), records[i].getIsDeleted());

			writePageBuffer();

			delete[] newData;
			std::cout << "Data modified successfully.\n";
			return true;
		}
		else if (records[i].getKey() > key) {
			if (i > 0) {
				if (overflowManager->modificationOfOverflowRecord(key, (records[i - 1].getPointer())))
					return true;
			}
			return false;
		}
	}

	if (blockingFactor > 0 && records[blockingFactor - 1].getKey() <= key) {
		if (overflowManager->modificationOfOverflowRecord(key, records[blockingFactor - 1].getPointer()))
			return true;
		else
			return false;
	}

	return false;
}

bool PageBuffer::automatedModify(int key, char* newData)
{
	for (int i = 0; i < blockingFactor; i++) {
		if (records[i].getKey() == key) {
			if (records[i].getIsDeleted()) records[i].setIsDeleted(false); 

			records[i].setRecord(records[i].getKey(), newData, records[i].getPointer(), records[i].getIsDeleted());
			writePageBuffer();
			return true;
		}
		else if (records[i].getKey() > key) {
			if (i > 0) {
				if (overflowManager->automatedModify(key, records[i - 1].getPointer(), newData))
					return true;
			}
			return false;
		}
	}
	if (blockingFactor > 0 && records[blockingFactor - 1].getKey() <= key) {
		if (overflowManager->automatedModify(key, records[blockingFactor - 1].getPointer(), newData))
			return true;
		else
			return false;
	}
	return false;
}
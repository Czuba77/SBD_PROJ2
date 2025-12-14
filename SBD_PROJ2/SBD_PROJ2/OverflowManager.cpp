#include "IndexFile.h"
#include "PageBuffer.h"
#include "Record.h"
#include "DBM.h"
#include "OverflowManager.h"
#include <iostream>


OverflowManager::OverflowManager(int*v,int bf, std::string fn, DBM* dbm)
{
	V_numberOfRecordsOverflow = v;
	fileName = fn;
	blockingFactor = bf;
	records = new Record[bf];
	this->dbm = dbm;
}

OverflowManager::~OverflowManager()
{
	delete[] records;
}




int OverflowManager::addOverflowRecord(Record record, int offsetOfEarlierRecord)
{
	if(offsetOfEarlierRecord==-1){
		putToEOF(record);
		int newAddress = (*V_numberOfRecordsOverflow) * R_sizeOfRecord;
		return newAddress;
	}
	else { // there is an earlier record in overflow
		Record* recordTMP;
		int indexOfTmp = isOffsetInOverflowBuffer(offsetOfEarlierRecord);
		if (indexOfTmp != -1)
			recordTMP = &records[indexOfTmp];
		else {
			writePageBuffer();
			readPageBuffer(offsetOfEarlierRecord);
			recordTMP = &records[0];
		}
		if (recordTMP->getKey() == record.getKey())
		{
			return -2; // duplicate key error
		}
		else if (recordTMP->getKey() <= record.getKey()) {
			if (recordTMP->getPointer() == -1) {
				recordTMP->setPointer((*V_numberOfRecordsOverflow) * R_sizeOfRecord);
				putToEOF(record);
			}
			else {
				int nextPointer = recordTMP->getPointer();
				if(addOverflowRecord(record, nextPointer)==-2)
					return -2;
				return offsetOfEarlierRecord;
			}
		}
		else {
			if (recordTMP->getPointer() == -1) {
				record.setPointer((*V_numberOfRecordsOverflow) * R_sizeOfRecord);
				Record tmp = *recordTMP;
				records[indexOfTmp] = record;
				putToEOF(tmp);
			}
			else {
				int nextPointer = recordTMP->getPointer();
				record.setPointer(nextPointer);
				Record tmp = *recordTMP;
				records[indexOfTmp] = record;
				tmp.setPointer(-1);
				if (addOverflowRecord(tmp, nextPointer) == -2)
					return -2;
				return offsetOfEarlierRecord;
			}
		}
		writePageBuffer();
		return offsetOfEarlierRecord;
	}
}


void OverflowManager::readPageBuffer(int offset)
{
	std::fstream f;
	offsetOfCurrentRecord = offset;
	f.open(fileName, std::ios::binary | std::ios::in);
	f.seekg(offset, std::ios::beg);
	DBM::diskReads++;
	for (int i = 0; i < blockingFactor; i++) {
		records[i].readRecord(&f);
		if(f.eof() == true)
			records[i] = Record();
	}
	f.close();
}

void OverflowManager::writePageBuffer()
{
	std::fstream f;

	f.open(fileName, std::ios::binary | std::ios::out | std::ios::in);
	f.seekp(offsetOfCurrentRecord, std::ios::beg);
	DBM::diskWrites++;
	for (int i = 0; i < blockingFactor; i++) {
		if (records[i].getKey() == -1) break;
		records[i].writeRecord(&f);
	}
	f.close();
}

int OverflowManager::isOffsetInOverflowBuffer(int offsetOfSearchedRec)
{
	if (offsetOfSearchedRec >= offsetOfCurrentRecord && 
		offsetOfSearchedRec < offsetOfCurrentRecord + blockingFactor * R_sizeOfRecord) {
		int index = (offsetOfSearchedRec  - offsetOfCurrentRecord)/R_sizeOfRecord;
		return index;
	}
	else {
		return -1;
	}
}

void OverflowManager::clearPageBuffer()
{
	for (int i = 0; i < blockingFactor; i++) {
		records[i] = Record();
	}
}

void OverflowManager::putToEOF(Record record)
{
	int index = isOffsetInOverflowBuffer((*V_numberOfRecordsOverflow) * R_sizeOfRecord);
	if (index != -1) {
		records[index] = record;
		writePageBuffer();
	}
	else {
		writePageBuffer();
		clearPageBuffer();
		records[0] = record;
		offsetOfCurrentRecord = (*V_numberOfRecordsOverflow) * R_sizeOfRecord;
		writePageBuffer();
	}
}

Record OverflowManager::ReadOverflowRecord(int pointer)
{
	int indexOfTmp = isOffsetInOverflowBuffer(pointer);
	if (indexOfTmp != -1) {
		return records[indexOfTmp];
	}
	else {
		writePageBuffer();
		readPageBuffer(pointer);
		return records[0];
	}
}

Record* OverflowManager::ReadOverflowRecord(int pointer,bool modify)
{
	int indexOfTmp = isOffsetInOverflowBuffer(pointer);
	if (indexOfTmp != -1) {
		return &records[indexOfTmp];
	}
	else {
		writePageBuffer();
		readPageBuffer(pointer);
		return &records[0];
	}
}

void OverflowManager::recSendRecordsToReorg(int pointer, PageBuffer* reorgPage)
{
	Record tmp = ReadOverflowRecord(pointer);
	int nextPointer = tmp.getPointer();
	tmp.setPointer(-1);
	reorgPage->recievePageRecords(tmp);
	if (nextPointer != -1) {
		recSendRecordsToReorg(nextPointer, reorgPage);
	}
}


bool OverflowManager::deletionOfOverflowRecord(int key,int pointer)
{
	Record * tmp = ReadOverflowRecord(pointer, true); 
	if (tmp->getKey() == key) {
		tmp->setIsDeleted(true);
		writePageBuffer();
		return true;
	}
	else if (tmp->getKey() > key) {
		return false;
	}
	int nextPointer = tmp->getPointer();
	if (nextPointer != -1) {
		return (deletionOfOverflowRecord(key, nextPointer));
	}
}


bool OverflowManager::modificationOfOverflowRecord(int key, int pointer)
{
	if (pointer == -1) return false;

	Record* tmp = ReadOverflowRecord(pointer, true);

	if (tmp->getKey() == key) {
		if (tmp->getIsDeleted()) {
			char choice;
			std::cout << "Record (Overflow) with key " << key << " is flagged as DELETED.\n";
			std::cout << "Do you want to restore it before editing? (y/n): ";
			std::cin >> choice;

			if (choice == 'y' || choice == 'Y') {
				tmp->setIsDeleted(false);
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
		std::cout << "Modifying data for overflow record with key: " << tmp->getKey() << "\n";
		newData = DBM::manualPlateCreation();
		tmp->setRecord(tmp->getKey(), newData, tmp->getPointer(), tmp->getIsDeleted());
		writePageBuffer();
		delete[] newData;
		std::cout << "Data modified successfully.\n";
		return true;
	}
	else if (tmp->getKey() > key) {
		return false;
	}

	int nextPointer = tmp->getPointer();
	if (nextPointer != -1) {
		return (modificationOfOverflowRecord(key, nextPointer));
	}

	return false;
}


bool OverflowManager::automatedModify(int key, int pointer, char* newData)
{
	if (pointer == -1) return false;

	Record* tmp = ReadOverflowRecord(pointer, true);

	if (tmp->getKey() == key) {
		if (tmp->getIsDeleted()) tmp->setIsDeleted(false);

		tmp->setRecord(tmp->getKey(), newData, tmp->getPointer(), tmp->getIsDeleted());
		writePageBuffer();
		return true;
	}
	else if (tmp->getKey() > key) {
		return false;
	}

	int nextPointer = tmp->getPointer();
	if (nextPointer != -1) {
		return (automatedModify(key, nextPointer, newData));
	}
	return false;
}
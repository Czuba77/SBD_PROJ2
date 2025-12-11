#include "IndexFile.h"
#include "PageBuffer.h"
#include "Record.h"
#include "DBM.h"
#include "OverflowManager.h"


OverflowManager::OverflowManager(int*v,int bf, std::string fn)
{
	V_numberOfRecordsOverflow = v;
	fileName = fn;
	blockingFactor = bf;
	for (int i = 0; i < bf; i++) {
		records[i] = Record();
	}
}

OverflowManager::~OverflowManager()
{
	delete V_numberOfRecordsOverflow;
}

int OverflowManager::addOverflowRecord(Record record)
{
	std::fstream f;
	f.open(fileName, std::ios::binary | std::ios::out | std::ios::app);
	record.writeRecord(&f);
	f.close();
	int pointer = (*V_numberOfRecordsOverflow) * R_sizeOfRecord;
	(*V_numberOfRecordsOverflow)++;
	return pointer;
}




int OverflowManager::addOverflowRecord(Record record, int offsetOfEarlierRecord)
{
	Record* recordTMP;
	int indexOfTmp = isOffsetInOverflowBuffer(offsetOfEarlierRecord);
	if (indexOfTmp!=-1)
		recordTMP = &records[indexOfTmp];
	else {
		writePageBuffer();
		readPageBuffer(offsetOfEarlierRecord);
		recordTMP = &records[0];
	}
	if (recordTMP->getKey() <= record.getKey()) {
		if(recordTMP->getPointer()==-1){
			recordTMP->setPointer((*V_numberOfRecordsOverflow) * R_sizeOfRecord);
			putToEOF(record);
		}
		else {
			int nextPointer = recordTMP->getPointer();
			addOverflowRecord(record, nextPointer);
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
			addOverflowRecord(tmp, nextPointer);
			return offsetOfEarlierRecord;
		}
	}
	(*V_numberOfRecordsOverflow)++;
	writePageBuffer();
	return offsetOfEarlierRecord;
}


void OverflowManager::readPageBuffer(int offset)
{
	std::fstream f;
	offsetOfCurrentRecord = offset;
	f.open(fileName, std::ios::binary | std::ios::in);
	f.seekg(offset, std::ios::beg);
	for (int i = 0; i < blockingFactor && f.eof()!= true; i++) {
		records[i].readRecord(&f);
	}
	f.close();
}

void OverflowManager::writePageBuffer()
{
	std::fstream f;

	f.open(fileName, std::ios::binary | std::ios::out | std::ios::in);
	f.seekp(offsetOfCurrentRecord, std::ios::beg);
	for (int i = 0; i < blockingFactor; i++) {
		if (records[i].getKey() == -1) break;
		records[i].writeRecord(&f);
	}
	clearPageBuffer();
	offsetOfCurrentRecord = -1;
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
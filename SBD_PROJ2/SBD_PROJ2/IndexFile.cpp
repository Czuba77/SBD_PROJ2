#include "IndexFile.h"
#include "PageBuffer.h"
#include "Record.h"
#include "DBM.h"
#include "OverflowManager.h"




IndexFile::IndexFile(std::string fn, int blockingFactor)
{
	fileName = fn;
	this->blockingFactor = blockingFactor;
	entries = new EntryIF[blockingFactor];
	offsetOfCurrentEntry = 0;
}

IndexFile::~IndexFile()
{
	delete[] entries;
}

void IndexFile::addEntry(int key, int pageNumber, int pageOffset)
{
	if(pageNumber>=numberOfEntries){
		EntryIF entry;
		entry.key = key;
		entry.pageNumber = pageNumber;
		entry.pageOffset = pageOffset;

		putToEOF(entry);
		numberOfEntries++;
		offsetOfCurrentEntry = numberOfEntries * sizeOfEntry;
	}
	else { //modify existing entry
		int index = isOffsetInOverflowBuffer(pageNumber * sizeOfEntry);
		if (index != -1) {
			entries[index].key = key;
			entries[index].pageNumber = pageNumber;
			entries[index].pageOffset = pageOffset;
			writePageBuffer();
		}
		else {
			writePageBuffer();
			readPageBuffer(pageNumber * sizeOfEntry);
			entries[0].key = key;
			entries[0].pageNumber = pageNumber;
			entries[0].pageOffset = pageOffset;
			writePageBuffer();
		}
	}

}

EntryIF IndexFile::readEntry(int index)
{
	int offset = index * sizeOfEntry;
	int indexInBuffer = isOffsetInOverflowBuffer(offset);
	if (indexInBuffer != -1) {
		return entries[indexInBuffer];
	}
	else{
		writePageBuffer();
		readPageBuffer(offset);
		return entries[0];
	}
}
/* Old linear search version
int IndexFile::findRightPage(int key)
{
	EntryIF entrylow,entryhigh;
	for (int i = 0; i < numberOfEntries-1; i++) {
		entrylow = readEntry(i);
		entryhigh = readEntry(i + 1);
		if (entrylow.key < key && entryhigh.key > key) {
			return entrylow.pageNumber;
		}
	}
	if(entryhigh.key <= key){
		return entryhigh.pageNumber;
	}
	if (numberOfEntries > 0 && readEntry(0).key > key) {
		return readEntry(0).pageNumber;
	}
	return -1;
}
*/
int IndexFile::findRightPage(int key)
{
	if (numberOfEntries == 0) {
		return 0;
	}
	int left = 0;
	int right = numberOfEntries - 1;
	int resultPage = -1;
	while (left <= right) {
		int mid = left + (right - left) / 2;
		EntryIF entry = readEntry(mid);

		if (entry.key <= key) {
			resultPage = entry.pageNumber;
			left = mid + 1;
		}
		else {
			right = mid - 1;
		}
	}
	if (resultPage != -1) {
		return resultPage;
	}
	return readEntry(0).pageNumber;
}

void IndexFile::readPageBuffer(int offset)
{
	std::fstream f;
	offsetOfCurrentEntry = offset;
	f.open(fileName, std::ios::binary | std::ios::in);
	f.seekg(offset, std::ios::beg);
	DBM::diskReads++;
	for (int i = 0; i < blockingFactor; i++) {
		entries[i].readEntry(&f);
		if (f.eof() == true)
			entries[i] = EntryIF();
	}
	f.close();
}

void IndexFile::writePageBuffer()
{
	std::fstream f;

	f.open(fileName, std::ios::binary | std::ios::out | std::ios::in);
	f.seekp(offsetOfCurrentEntry, std::ios::beg);
	DBM::diskWrites++;
	for (int i = 0; i < blockingFactor; i++) {
		if (entries[i].key == -1) break;
		entries[i].writeRecord(&f);
	}

	f.close();
}

int IndexFile::isOffsetInOverflowBuffer(int offsetOfSearchedEnt)
{
	if (offsetOfCurrentEntry == -1) return -1;

	if (offsetOfSearchedEnt >= offsetOfCurrentEntry &&
		offsetOfSearchedEnt < offsetOfCurrentEntry + blockingFactor * sizeOfEntry) {
		int index = (offsetOfSearchedEnt - offsetOfCurrentEntry) / sizeOfEntry;
		return index;
	}
	else {
		return -1;
	}
}

void IndexFile::clearPageBuffer()
{
	for (int i = 0; i < blockingFactor; i++) {
		entries[i] = EntryIF();
	}
}

void IndexFile::putToEOF(EntryIF entry)
{
	int index = isOffsetInOverflowBuffer(numberOfEntries * sizeOfEntry);
	if (index != -1) {
		entries[index] = entry;
		writePageBuffer();
	}
	else {
		writePageBuffer();
		clearPageBuffer();
		entries[0] = entry;
		offsetOfCurrentEntry = numberOfEntries * sizeOfEntry;
		writePageBuffer();
	}
}
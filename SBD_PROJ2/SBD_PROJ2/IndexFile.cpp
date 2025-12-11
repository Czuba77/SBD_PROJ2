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
}

void IndexFile::addEntry(int key, int pageNumber, int pageOffset)
{
	EntryIF entry;
	entry.key = key;
	entry.pageNumber = pageNumber;
	entry.pageOffset = pageOffset;

	putToEOF(entry);
	numberOfEntries++;
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

int IndexFile::findRightPage(int key)
{
	EntryIF entry;
	for (int i = 0; i < numberOfEntries; i++) {
		entry = readEntry(i);
		if (entry.key > key) {
			return entry.pageNumber;
		}
	}
	return -1;
}

void IndexFile::readPageBuffer(int offset)
{
	std::fstream f;
	offsetOfCurrentEntry = offset;
	f.open(fileName, std::ios::binary | std::ios::in);
	f.seekg(offset, std::ios::beg);
	for (int i = 0; i < blockingFactor && f.eof() != true; i++) {
		entries[i].readEntry(&f);
	}
	f.close();
}

void IndexFile::writePageBuffer()
{
	std::fstream f;

	f.open(fileName, std::ios::binary | std::ios::out | std::ios::in);
	f.seekp(offsetOfCurrentEntry, std::ios::beg);
	for (int i = 0; i < blockingFactor; i++) {
		if (entries[i].key == -1) break;
		entries[i].writeRecord(&f);
	}
	clearPageBuffer();
	offsetOfCurrentEntry = -1;
	f.close();
}

int IndexFile::isOffsetInOverflowBuffer(int offsetOfSearchedEnt)
{
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
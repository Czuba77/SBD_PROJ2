#pragma once
#include <vector>
#include <string>
#include "DBM.h"
#include <fstream>

#pragma pack(push, 1)
struct EntryIF {
	int key; 
	int pageNumber; 
	int pageOffset; 
	char padding[R_sizeOfRecord - (3 * sizeof(int))] = {};
	EntryIF() 
	{ 
	key = -1; 
	pageNumber = -1; 
	pageOffset = -1; 
	for (int i = 0; i < R_sizeOfRecord - (3 * sizeof(int)); i++) {
		padding[i] = '\0'; 
	} 
	}

	void readEntry(std::fstream* f)
	{
		f->read((char*)&key, sizeof(int));
		f->read((char*)&pageNumber, sizeof(int));
		f->read((char*)&pageOffset, sizeof(int));
		f->read((char*)padding, R_sizeOfRecord - (3 * sizeof(int)));
	}

	void writeRecord(std::fstream* f)
	{
		f->write((char*)&key, sizeof(int));
		f->write((char*)&pageNumber, sizeof(int));
		f->write((char*)&pageOffset, sizeof(int));
		f->write((char*)padding, R_sizeOfRecord - (3 * sizeof(int)));
	}
};
#pragma pack(pop)


class DBM;


class IndexFile {
private:
	std::string fileName;
	int sizeOfEntry = sizeof(EntryIF);
	int blockingFactor;
	int paddingSize= R_sizeOfRecord - (3 * sizeof(int));
	int numberOfEntries = 0;
	EntryIF* entries;
	int offsetOfCurrentEntry;
public:
	IndexFile(std::string fn, int blockingFactor);
	~IndexFile();
	void addEntry(int key, int pageNumber, int pageOffset);
	EntryIF readEntry(int index);
	int findRightPage(int key);
	void readPageBuffer(int offset);
	void writePageBuffer();
	int isOffsetInOverflowBuffer(int offsetOfSearchedEnt);
	void clearPageBuffer();
	void putToEOF(EntryIF entry);
};
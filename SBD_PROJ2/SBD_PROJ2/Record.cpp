#include "IndexFile.h"
#include "PageBuffer.h"
#include "Record.h"
#include "DBM.h"
#include "OverflowManager.h"


Record::Record()
{
	key = -1;
	for (int i = 0; i < 9; i++) {
		data[i] = '\0';
	}
	pointer = -1;
	isDeleted = false;
}

void Record::setRecord(int k, char* d, int p, bool del)
{
	key = k;
	for (int i = 0; i < 9; i++) {
		data[i] = d[i];
	}
	pointer = p;
	isDeleted = del;
}

Record::~Record()
{
}

void Record::readRecord(std::fstream* f)
{
	f->read((char*)&key, sizeof(int));
	f->read((char*)data, sizeof(data));
	f->read((char*)&pointer, sizeof(int));
	f->read((char*)&isDeleted, sizeof(bool));
}

void Record::writeRecord(std::fstream* f)
{
	f->write((char*)&key, sizeof(int));
	f->write((char*)data, sizeof(data));
	f->write((char*)&pointer, sizeof(int));
	f->write((char*)&isDeleted, sizeof(bool));
}



#pragma once
#include <fstream>

class Record
{
private:
	int key;
	char data[9];
	int pointer;
	bool isDeleted;
public:
	Record();
	void setRecord(int k, char*d, int p,bool del);
	~Record();
	void readRecord(std::fstream* f);
	void writeRecord(std::fstream* f);
	int getKey() { return key; }
	void setPointer(int p) { pointer = p; }	
	int getPointer() { return pointer; }
	bool getIsDeleted() { return isDeleted; }
	void setIsDeleted(bool del) { isDeleted = del; }
	void setKey(int k) { key = k; }
};


#pragma once
#include <vector>
#include <string>

class Record;
class DBM;
class IndexFile;
class OverflowManager;
const int D_sizeOfData = sizeof("XXXYYYYY"); // 9 bajtów
const int K_sizeOfKey = sizeof(int);         // 4 bajty
const int P_sizeOfPoint = sizeof(int);       // 4 bajty
const int R_sizeOfRecord = K_sizeOfKey + D_sizeOfData + P_sizeOfPoint + sizeof(bool); // 18 bajtów


class DBM {
private:
	int N_numberOfRecordsMain=0;
	int V_numberOfRecordsOverflow=0;
	int blockingFactor = 4;
	int B_diskPageCap = R_sizeOfRecord * blockingFactor;
	int alpha_pageUtill = 3;

	std::string dataFileName = "datafile.dat";
	std::string overflowFileName = "offile.dat";
	std::string indexfileName = "indexfile.dat";
	
	PageBuffer* pageBuffers;
	IndexFile* indexFile;
	OverflowManager* overflowManager;


public:
	DBM();
	~DBM();
	void DBMmain();
};
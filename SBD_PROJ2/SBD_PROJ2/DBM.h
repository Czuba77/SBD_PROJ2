#pragma once
#include <vector>
#include <string>

class Record;
class DBM;
class IndexFile;
class OverflowManager;
class PageBuffer;

const int D_sizeOfData = sizeof("XXXYYYYY"); // 9 bajtów
const int K_sizeOfKey = sizeof(int);         // 4 bajty
const int P_sizeOfPoint = sizeof(int);       // 4 bajty
const int R_sizeOfRecord = K_sizeOfKey + D_sizeOfData + P_sizeOfPoint + sizeof(bool); // 18 bajtów


class DBM {
private:
	int numberOfRecordsReorg = 0;
	int N_numberOfRecordsMain=0;
	int V_numberOfRecordsOverflow=0;
	int blockingFactor = 8;
	int B_diskPageCap = R_sizeOfRecord * blockingFactor;
	int alpha_pageUtill = 7;

	int pageCount = 0;
	int reorgPageCount = 0;

	std::string dataFileName = "datafile.bin";
	std::string reorgFileName = "datafile_reorg.bin";
	std::string overflowFileName = "offile.bin";
	std::string reorgIndexFileName = "indexfile_reorg.bin";
	std::string indexfileName = "indexfile.bin";
	
	PageBuffer* pageBuffer,*reorgBuffer;
	IndexFile* indexFile,* reorgIndexFile;
	OverflowManager* overflowManager;


public:

	static long long diskReads;
	static long long diskWrites;
	static void resetIO() { diskReads = 0; diskWrites = 0; }

	DBM();
	~DBM();
	void DBMmain();
	char genRandomChar(bool forceLetter);
    char* genData();
	static bool isOnlyLetters(const char* s);
	static bool isOnlyLettersAndNumbers(const char* s);
	void manualAddRecord();
	void addRandomRecords(int records_number);
	void reorganizeFiles();
	void printFiles();
	void populatePageBuffer(int howManyPages);
	int addRecordToDBM(Record rec);
	void dbReset();
	void modifyRecordInDBM();
	void deleteRecordInDBM();
	static char* manualPlateCreation();

	int getDiskPageCap() { return B_diskPageCap; }
	void incrementMainRecordCount() { N_numberOfRecordsMain++; }
	void incrementReorgRecordCount() { numberOfRecordsReorg++; }
	void incrementReorgPageCount() { reorgPageCount++; }

	void runExperiment();
	void automatedModifyInDBM(int key, char* newData);
};
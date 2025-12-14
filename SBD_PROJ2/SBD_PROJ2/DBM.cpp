#include "IndexFile.h"
#include "PageBuffer.h"
#include "Record.h"
#include "DBM.h"
#include "OverflowManager.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <iomanip> 
#include <vector>

long long DBM::diskReads = 0;
long long DBM::diskWrites = 0;


DBM::DBM()
{
    std::ofstream fData(dataFileName, std::ios::binary | std::ios::trunc);
    fData.close();

    std::ofstream fOverflow(overflowFileName, std::ios::binary | std::ios::trunc);
    fOverflow.close();

    std::ofstream fIndex(indexfileName, std::ios::binary | std::ios::trunc);
    fIndex.close();

    std::ofstream frIndex(reorgIndexFileName, std::ios::binary | std::ios::trunc);
    frIndex.close();

    std::ofstream fReorg(reorgFileName, std::ios::binary | std::ios::trunc);
    fReorg.close();

	indexFile =new IndexFile(indexfileName, blockingFactor);
    reorgIndexFile = new IndexFile(reorgIndexFileName, blockingFactor);
	overflowManager =new OverflowManager(&V_numberOfRecordsOverflow, blockingFactor, overflowFileName,this);
	pageBuffer = new PageBuffer(this, indexFile, overflowManager, blockingFactor, alpha_pageUtill, 0, B_diskPageCap, dataFileName);
    reorgBuffer = new PageBuffer(this, reorgIndexFile, overflowManager, blockingFactor, alpha_pageUtill, 0, B_diskPageCap, reorgFileName);



}

DBM::~DBM()
{
    delete pageBuffer;
    delete reorgBuffer;
    delete overflowManager;
	delete reorgIndexFile;
    delete indexFile;
}

void DBM::DBMmain()
{
    int choice;
    bool exitCondition = false;
    int recordsNumber;

    while (!exitCondition)
    {
        std::cout << "\nChoose action\n"
            << "1. Generate random records (Add Random)\n"
            << "2. Add a record manually (Add Manual)\n"
            << "3. Populate page buffer (Create & Fill Pages)\n"
            << "4. Print content of all files (Print)\n"
            << "5. DELETE ALL records (Reset Database)\n"
            << "6. Reorganize files (Merge/Update)\n"
            << "7. Modify a record\n"
            << "8. Delete a record\n"
            << "9. experiment\n"
            << "0. Exit\n";

        std::cin >> choice;

        switch (choice)
        {
        case 1:
            std::cout << "How many records do you want to generate?\n";
            std::cin >> recordsNumber;
            addRandomRecords(recordsNumber);
            break;
        case 2: manualAddRecord(); break;
        case 3:
            std::cout << "How many pages you want to create and fill?\n";
            std::cin >> recordsNumber;
            populatePageBuffer(recordsNumber);
            break;
        case 4: printFiles(); break;
        case 5: dbReset(); break;
        case 6: reorganizeFiles(); break;
        case 7: modifyRecordInDBM(); break;
        case 8: deleteRecordInDBM(); break;
        case 9: runExperiment(); break;

        case 0:
            exitCondition = true;
            std::cout << "Exiting...\n";
            break;
        default:
            std::cout << "Invalid choice. Please try again.\n";
            break;
        }
    }
}

void DBM::dbReset() {
    N_numberOfRecordsMain = 0;
    V_numberOfRecordsOverflow = 0;
    pageCount = 0;
    reorgPageCount = 0;
    numberOfRecordsReorg = 0;

    std::ofstream file(dataFileName, std::ios::trunc);
    N_numberOfRecordsMain = 0;
    file.close();

    std::ofstream file2(overflowFileName, std::ios::trunc);
    V_numberOfRecordsOverflow = 0;
    file2.close();

    std::ofstream file3(indexfileName, std::ios::trunc);
    pageCount = 0;
    file3.close();

    std::ofstream frIndex(reorgIndexFileName, std::ios::binary | std::ios::trunc);
    frIndex.close();

    std::ofstream fReorg(reorgFileName, std::ios::binary | std::ios::trunc);
    fReorg.close();

	delete indexFile;
	indexFile = new IndexFile(indexfileName, blockingFactor);
	delete overflowManager;
	overflowManager = new OverflowManager(&V_numberOfRecordsOverflow, blockingFactor, overflowFileName, this);
	delete pageBuffer;
	pageBuffer = new PageBuffer(this, indexFile, overflowManager, blockingFactor, alpha_pageUtill, 0, B_diskPageCap, dataFileName);
	delete reorgIndexFile;
	reorgIndexFile = new IndexFile(reorgIndexFileName, blockingFactor);
	delete reorgBuffer;
	reorgBuffer = new PageBuffer(this, reorgIndexFile, overflowManager, blockingFactor, alpha_pageUtill, 0, B_diskPageCap, reorgFileName);

    std::cout << "Database reset complete (files truncated).\n";
}


char DBM::genRandomChar(bool forceLetter) {
    static const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const char numbers[] = "1234567890";

    if (forceLetter) {
        return letters[rand() % (sizeof(letters) - 1)];
    }
    else {
        if (rand() % 5 == 1)
            return letters[rand() % (sizeof(letters) - 1)];
        else
            return numbers[rand() % (sizeof(numbers) - 1)];
    }
}

bool DBM::isOnlyLetters(const char* s) {
    for (int i = 0; s[i] != '\0'; ++i) {
        if (!isupper(s[i])) return false;
    }
    return true;
}

bool DBM::isOnlyLettersAndNumbers(const char* s) {
    for (int i = 0; s[i] != '\0'; ++i) {
        if (!isupper(s[i]) && !isdigit(s[i])) return false;
    }
    return true;
}

char* DBM::manualPlateCreation() {
    char wojPart[10];
    char indPart[10];
    char* finalRecord = new char[9];
    while (true) {
        std::cout << "Write first part of license plate\n";
        std::cin >> wojPart;


        size_t len1 = strlen(wojPart);
        if (len1 < 2 || len1 > 3 || !isOnlyLetters(wojPart)) {
            std::cout << "First part of license plate must be 2 or 3 letters long, and should contain only capital letters\n";
            continue;
        }
        for (int i = 0; i < len1; i++) {
            finalRecord[i] = wojPart[i];
        }

        std::cout << "Write second part of license plate\n";
        std::cin >> indPart;


        size_t len2 = strlen(indPart);
        if (len2 != 5 || !isOnlyLettersAndNumbers(indPart)) {
            std::cout << "It must be 5 characters long, and should contain only capital letters and numbers\n";
            continue;
        }

        for (int i = 0; i < len2; i++) {
            finalRecord[len1 + i] = indPart[i];


            if (len1 == 2) {
                finalRecord[7] = ' ';
                finalRecord[8] = '\0';
            }
            else {
                finalRecord[8] = '\0';
            }

        }
        return finalRecord;
    }
	return nullptr;
}

void DBM::manualAddRecord() {
    int key;
    std::cout << "Enter the key for the new record (integer): ";
    std::cin >> key;
    char* data = this->genData();
    Record rec;
    rec.setRecord(key, data, -1, false);

    long long startR = diskReads;
    long long startW = diskWrites;

    int condition = addRecordToDBM(rec);

    long long endR = diskReads;
    long long endW = diskWrites;

    if (condition == 1) {
        N_numberOfRecordsMain += 1;
        std::cout << "Record added to Main file.\n";
    }
    else if (condition == 0) {
        V_numberOfRecordsOverflow += 1;
        std::cout << "Record added to Overflow file.\n";
    }
    else if (condition == -1)
        std::cout << "Error: Duplicate key. Record not added.\n";
    else
        std::cout << "Error\n";

    std::cout << "Disk IO Operations -> Reads: " << (endR - startR) << ", Writes: " << (endW - startW) << "\n";

    delete[] data;
}

void DBM::addRandomRecords(int records_number) {
    for (int i = 0; i < records_number; ++i) {
        char* data = this->genData();
        int modBase = (pageCount > 0) ? (pageCount * 10) : 10;
        int key = rand() % modBase + rand() % 10;
        Record rec;
        rec.setRecord(key, data, -1, false);
        int condition = addRecordToDBM(rec);
        if (condition == 1) {
            N_numberOfRecordsMain += 1;
        }
        else if (condition == 0)
            V_numberOfRecordsOverflow += 1;
        else
            i--;
        delete[] data;
    }
}

char* DBM::genData() {
    char* data = new char[9];
    data[8] = '\0';
    int part1Len = rand() % 2 + 2;
    int currentIdx = 0;
    for (int j = 0; j < part1Len; ++j) {
        data[currentIdx++] = genRandomChar(true);
    }
    for (int j = 0; j < 5; ++j) {
        data[currentIdx++] = genRandomChar(false);
    }
    if (part1Len == 2) {
        data[7] = ' ';
    }
    return data;
}

void DBM::reorganizeFiles() {
        std::ofstream frIndex(reorgIndexFileName, std::ios::binary | std::ios::trunc);
        frIndex.close();

        std::ofstream fReorg(reorgFileName, std::ios::binary | std::ios::trunc);
        fReorg.close();

        for (int i = 0; i < pageCount; i++) {
            pageBuffer->chooseAndLoadPage(i);
			pageBuffer->sendPageRecordsToReorg(reorgBuffer);
		}
        if (reorgBuffer->writePageBuffer(true)) {
			reorgPageCount++;
        };
		std::cout << "Number of pages before reorganization: " <<pageCount <<" after: "<< reorgPageCount << "\n";


        std::ofstream file(dataFileName, std::ios::trunc);
        file.close();

        std::ofstream file2(overflowFileName, std::ios::trunc);
        file2.close();

        std::ofstream file3(indexfileName, std::ios::trunc);
        file3.close();

        std::swap(dataFileName, reorgFileName);
        std::swap(indexfileName, reorgIndexFileName);

        delete pageBuffer;
        delete indexFile;

        indexFile = reorgIndexFile;
        pageBuffer = reorgBuffer;

		N_numberOfRecordsMain = numberOfRecordsReorg;
		pageCount = reorgPageCount;


		reorgIndexFile = new IndexFile(reorgIndexFileName, blockingFactor);
		reorgBuffer = new PageBuffer(this, reorgIndexFile, overflowManager, blockingFactor, alpha_pageUtill, 0, B_diskPageCap, reorgFileName);

        reorgPageCount = 0;
        V_numberOfRecordsOverflow = 0;
        numberOfRecordsReorg = 0;

        std::cout << "Reorganization complete.\n";
}

void DBM::printFiles() {

#pragma pack(push, 1)
    struct RecordStruct {
        int key;
        char data[9]; 
        int pointer;
        bool isDeleted; 
    };

    struct IndexStruct {
        int key;        
        int pageNumber;     
        int pageOffset;    
        char padding[6];    
    };
#pragma pack(pop)

    auto printRecordFile = [](const std::string& filename, const std::string& label) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Error opening " << label << " file.\n";
            return;
        }

        std::cout  << label << "\n";
        RecordStruct rec;
        int id = 0;

        while (file.read(reinterpret_cast<char*>(&rec), sizeof(RecordStruct))) {
            std::cout << "[" << id++ << "] "
                << "Key: " << rec.key << " | "
                << "Data: ";

            std::cout.write(rec.data, 9);

            std::cout << " | "
                << "Ptr: " << rec.pointer << " | "
                << "Del: " << (rec.isDeleted ? "true" : "false")
                << "\n";
        }
        std::cout << "\n";
        file.close();
        };
    auto printIndexFile = [](const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Error opening Index file.\n";
            return;
        }

        std::cout << "INDEX FILE\n";
        IndexStruct entry;
        int id = 0;

        while (file.read(reinterpret_cast<char*>(&entry), sizeof(IndexStruct))) {
            std::cout << "[" << id++ << "] "
                << "Key: " << entry.key << " | "
                << "Page: " << entry.pageNumber << " | "
                << "Offset: " << entry.pageOffset
                << "\n";
        }
        std::cout << "\n";
        file.close();
        };

    printRecordFile(dataFileName, "DATA FILE");
    printRecordFile(overflowFileName, "OVERFLOW FILE");
    printIndexFile(indexfileName);
}

void DBM::populatePageBuffer(int howManyPages) {
	pageCount = howManyPages;
    for (int i = 0; i < howManyPages; i++) {
        pageBuffer->setPageNumber(i);
        pageBuffer->populateBuffer();
    }

}

int DBM::addRecordToDBM(Record rec) {
    int pageNumber = indexFile->findRightPage(rec.getKey());
    if (pageNumber == -1) {
		pageNumber = 0; // If no page found, use the first page
    }
    pageBuffer->chooseAndLoadPage(pageNumber);
    return pageBuffer->addRecordToPageBuffer(rec);
}

void DBM::modifyRecordInDBM() {
    int key;
    std::cout << "Enter the key of the record to modify: ";
    std::cin >> key;

    long long startR = diskReads;
    long long startW = diskWrites;

    int pageNumber = indexFile->findRightPage(key);
    pageBuffer->chooseAndLoadPage(pageNumber);
    if (!pageBuffer->modificationOfRecord(key)) {
        std::cout << "Record with key " << key << " not found in the specified page.\n";
    }

    long long endR = diskReads;
    long long endW = diskWrites;

    std::cout << "Disk IO Operations -> Reads: " << (endR - startR) << ", Writes: " << (endW - startW) << "\n";
}

void DBM::deleteRecordInDBM() {
    int key;
    std::cout << "Enter the key of the record to delete: ";
    std::cin >> key;

    long long startR = diskReads;
    long long startW = diskWrites;

    int pageNumber = indexFile->findRightPage(key);
    pageBuffer->chooseAndLoadPage(pageNumber);
    if (!pageBuffer->deletionOfRecord(key)) {
        std::cout << "Record with key " << key << " not found in the specified page.\n";
    }

    long long endR = diskReads;
    long long endW = diskWrites;

    std::cout << "Disk IO Operations -> Reads: " << (endR - startR) << ", Writes: " << (endW - startW) << "\n";
}

void DBM::automatedModifyInDBM(int key, char* newData) {
    int pageNumber = indexFile->findRightPage(key);
    pageBuffer->chooseAndLoadPage(pageNumber);
    pageBuffer->automatedModify(key, newData);
}



void DBM::runExperiment() {

    dbReset();
    populatePageBuffer(10);
    //printFiles();

    const int ITERATIONS = 35;
    std::vector<int> successfulKeys;
    successfulKeys.reserve(ITERATIONS);

    int modBase = (pageCount > 0) ? (pageCount * 30) : 10;

    double totalAddReads = 0, totalAddWrites = 0;
    double totalModReads = 0, totalModWrites = 0;
    double totalDelReads = 0, totalDelWrites = 0;
    double totalReorgReads = 0, totalReorgWrites = 0;

    resetIO();

    for (int i = 0; i < ITERATIONS; i++) {
        int key = rand() % modBase + rand() % 30;
        char* data = genData();
        Record rec;
        rec.setRecord(key, data, -1, false);

        long long startR = diskReads;
        long long startW = diskWrites;

        int res = addRecordToDBM(rec);

        long long endR = diskReads;
        long long endW = diskWrites;

        if (res == 1 || res == 0) {
            if (res == 1) N_numberOfRecordsMain++;
            else V_numberOfRecordsOverflow++;

            totalAddReads += (endR - startR);
            totalAddWrites += (endW - startW);

            successfulKeys.push_back(key);
        }
        delete[] data;
    }

    int successfulCount = successfulKeys.size();

    for (int key : successfulKeys) {
        long long startR = diskReads;
        long long startW = diskWrites;

        char* newData = genData();
        automatedModifyInDBM(key, newData);
        delete[] newData;

        totalModReads += (diskReads - startR);
        totalModWrites += (diskWrites - startW);
    }

    int deleteCount = successfulCount / 2;
    for (int i = 0; i < deleteCount; i++) {
        int key = successfulKeys[i];

        long long startR = diskReads;
        long long startW = diskWrites;

        int pageNumber = indexFile->findRightPage(key);
        pageBuffer->chooseAndLoadPage(pageNumber);
        pageBuffer->deletionOfRecord(key);

        totalDelReads += (diskReads - startR);
        totalDelWrites += (diskWrites - startW);
    }

    {
        long long startR = diskReads;
        long long startW = diskWrites;
        reorganizeFiles();

        totalReorgReads = (double)(diskReads - startR);
        totalReorgWrites = (double)(diskWrites - startW);
    }

    std::cout << "\n=== EXPERIMENT RESULTS ===\n";
    std::cout << "Attempts: " << ITERATIONS << ", Successful Adds: " << successfulCount << "\n";

    double avgAddR = (successfulCount > 0) ? (totalAddReads / successfulCount) : 0.0;
    double avgAddW = (successfulCount > 0) ? (totalAddWrites / successfulCount) : 0.0;

    double avgModR = (successfulCount > 0) ? (totalModReads / successfulCount) : 0.0;
    double avgModW = (successfulCount > 0) ? (totalModWrites / successfulCount) : 0.0;

    double avgDelR = (deleteCount > 0) ? (totalDelReads / deleteCount) : 0.0;
    double avgDelW = (deleteCount > 0) ? (totalDelWrites / deleteCount) : 0.0;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Operation      | Avg Reads  | Avg Writes\n";
    std::cout << "--------------------------------------\n";
    std::cout << "Add Record     | " << std::setw(10) << avgAddR << " | " << std::setw(10) << avgAddW << "\n";
    std::cout << "Modify Data    | " << std::setw(10) << avgModR << " | " << std::setw(10) << avgModW << "\n";
    std::cout << "Delete Record  | " << std::setw(10) << avgDelR << " | " << std::setw(10) << avgDelW << "\n";
    std::cout << "--------------------------------------\n";
    std::cout << "Operation      | Total Reads| Total Writes\n";
    std::cout << "--------------------------------------\n";
    std::cout << "Reorganization | " << std::setw(10) << totalReorgReads << " | " << std::setw(10) << totalReorgWrites << "\n";
    std::cout << "--------------------------------------\n";
    std::cout << "Alpha page utill = " << alpha_pageUtill << " \n";
    std::cout << "Blocking factor  = " << blockingFactor << " \n";
}
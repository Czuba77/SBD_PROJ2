#include "IndexFile.h"
#include "PageBuffer.h"
#include "Record.h"
#include "DBM.h"
#include "OverflowManager.h"

DBM::DBM()
{
	indexFile =new IndexFile(indexfileName, blockingFactor);
	overflowManager =new OverflowManager(&V_numberOfRecordsOverflow, blockingFactor, overflowFileName);
	pageBuffers = new PageBuffer(this, indexFile, blockingFactor, alpha_pageUtill, 0, B_diskPageCap, dataFileName);

}

DBM::~DBM()
{
	delete pageBuffers;
	delete overflowManager;
	delete indexFile;
}

void DBM::DBMmain()
{

}
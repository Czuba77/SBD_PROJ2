#include <iostream>
#include "IndexFile.h"
#include "PageBuffer.h"
#include "Record.h"
#include "DBM.h"
#include "OverflowManager.h"

int main()
{
    DBM dbm;
	dbm.DBMmain();
	return 0;
}

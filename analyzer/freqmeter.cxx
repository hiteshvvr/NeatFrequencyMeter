#include "freqmeter.h"

#include <iomanip>
#include <iostream>


freqmeter::freqmeter(int bklen, int bktype, const char* name, void *pdata):
    TGenericData(bklen, bktype, name, pdata)
{
  intcount = GetData32()[0];
  inttime = GetData32()[1];
  intfreq = GetData32()[2];
}



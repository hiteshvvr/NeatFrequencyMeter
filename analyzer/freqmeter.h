#ifndef freqmeter_hxx_seen
#define freqmeter_hxx_seen

#include <vector>

#include "TGenericData.hxx"


/// This decoder is for getting the absolute timestamps from computer time
class freqmeter: public TGenericData {

public:

  /// Constructor
  freqmeter(int bklen, int bktype, const char* name, void *pdata);

  uint32_t GetIntCount() const { return (intcount & 0xFFFFFFFF ); }
  uint32_t GetIntTime () const { return (inttime & 0xFFFFFFFF );}

  float GetFrequency() const {return (((intcount & 0xffffffff)*1000)/(inttime & 0xffffffff));}

private:

  uint32_t intcount;
  uint32_t inttime;
  

};

#endif

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
  uint32_t GetIntTime() const { return (inttime & 0xFFFFFFFF );}

  // float GetFrequency() const {return (((intcount & 0xffffffff)*1000000)/(inttime & 0xffffffff));}
  float GetFrequency() const {return (((intcount & 0xffffffff)*4000));}
  // uint32_t GetFrequency() const { return (intfreq & 0xFFFFFFFF );}

private:

  uint32_t intcount;
  uint32_t inttime;
  uint32_t timebase = 1;
  

};

#endif

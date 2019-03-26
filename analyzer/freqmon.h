#ifndef freqmon_h
#define freqmon_h
#include <string>
#include "THistogramArrayBase.h"
#include "TH2D.h"

class freqmon : public THistogramArrayBase {
public:
  freqmon(){
    CreateHistograms();
  };
  virtual ~freqmon(){};

  void UpdateHistograms(TDataContainer& dataContainer);

  // Reset the histograms; needed before we re-fill each histo.
  void Reset();
  
  void CreateHistograms();
  
  /// Take actions at begin run
  void BeginRun(int transition,int run,int time){		
    CreateHistograms();		
  }

private:
float numpoint = 100;
// INSERT THESE VALUES
//float intercept = -5.976;
//float slope  = 0.0125;


};

#endif


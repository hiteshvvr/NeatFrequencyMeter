#ifndef TAnaManager_h
#define TAnaManager_h

// Use this list here to decide which type of equipment to use.

// #define USE_V792
//#define USE_V1190
//#define USE_L2249
//#define USE_AGILENT
#define USE_V1720
//#define USE_V1720_CORRELATIONS
#define USE_MASSSELECT
#define USE_V1290
#define USE_TIMESTAMP
#define USE_FREQ
// #define USE_NEATEVT
//#define USE_V1730DPP
// #define USE_V1730RAW
// #define USE_DT724

// #include "TV792Histogram.h"
// #include "TV1190Histogram.h"
// #include "TL2249Histogram.h"
// #include "TAgilentHistogram.h"
#include "TV1720Waveform.h"
#include "TV1720Correlations.h"
#include "TV1290Histogram.h"
#include "massselect.h"
#include "freqmon.h"
// #include "TV1730DppWaveform.h"
// #include "TV1730RawWaveform.h"
// #include "TDT724Waveform.h"

/// This is an example of how to organize a set of different histograms
/// so that we can access the same information in a display or a batch
/// analyzer.
/// Change the set of ifdef's above to define which equipment to use.
class TAnaManager  {
public:
  TAnaManager();
  virtual ~TAnaManager(){};

  /// Processes the midas event, fills histograms, etc.
  int ProcessMidasEvent(TDataContainer& dataContainer);

	/// Methods for determining if we have a particular set of histograms.
	// bool HaveV792Histograms();
	// bool HaveV1190Histograms();
	// bool HaveL2249Histograms();
	// bool HaveAgilentistograms();
	bool HaveV1720Histograms();
  	bool HaveV1720Correlations();
        bool Havemassselect();
        bool Havefreqmon();
        bool HaveV1720Ch1();
        bool HaveV1290Histograms();
	// bool HaveV1730DPPistograms();
	// bool HaveV1730Rawistograms();
	// bool HaveDT724Histograms();

	/// Methods for getting particular set of histograms.
	// TV792Histograms* GetV792Histograms();
	// TV1190Histograms* GetV1190Histograms();
	// TL2249Histograms* GetL2249Histograms();
	// TAgilentHistograms* GetAgilentistograms();
        TV1290Histograms* GetV1290Histograms();
	TV1720Waveform* GetV1720Histograms();
  	TV1720Correlations* GetV1720Correlations();
        massselect* Getmassselect();
        freqmon* Getfreqmon();
    // TV1720Ch1* GetV1720Ch1();
	// TV1730DppWaveform* GetV1730DPPistograms();
	// TV1730RawWaveform* GetV1730Rawistograms();
	// TDT724Waveform* GetDT724Histograms();


private:

	// TV792Histograms *fV792Histogram;
	// TV1190Histograms *fV1190Histogram;
	// TL2249Histograms *fL2249Histogram;
	// TAgilentHistograms *fAgilentHistogram;
        TV1290Histograms *fV1290Histogram;
	TV1720Waveform *fV1720Waveform;
  	TV1720Correlations *fV1720Correlations;
        massselect *fmassselect;
        freqmon *ffreqmon;
    // TV1720Ch1 *fV1720Ch1;

	// TV1730DppWaveform *fV1730DppWaveform;
	// TV1730RawWaveform *fV1730RawWaveform;
	// TDT724Waveform *fDT724Waveform;

  // Make some cross-channel histograms
  TH2F *fV1720PHCompare;
  TH2F *fV1720TimeCompare;

  // Make some normal channel histograms
  TH1D  *fV1720Ch1;
  TH1D  *fV1720Ch2;
  TH1D  *fV1720Ch3;
  TH1D  *fV1720Ch4;
};



#endif



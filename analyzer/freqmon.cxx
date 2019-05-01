#include "freqmon.h"
#include "freqmeter.h"

#include "TDirectory.h"
#include "TV1720RawData.h"
#include "TV1290Data.hxx"
#include "TLine.h"

#include "TInterestingEventManager.hxx"

void freqmon::CreateHistograms() {
    clear();

    TH1D *lashist = new TH1D("LaserSignal", "LaserSiganl", 1000, 0, 1000);
    lashist->SetXTitle("Number");
    lashist->SetYTitle("Signal");
    push_back(lashist);

    TH1D *freqhist = new TH1D("Freqency", "Frequency", 1000, 0, 1000);
    freqhist->SetXTitle("Number");
    freqhist->SetYTitle("Frequency");
    push_back(freqhist);

    
    TH1D *nlashist = new TH1D("Normalized LaserSignal", "Normalized LaserSiganl", 1000, 0, 1000);
    nlashist->SetXTitle("Number");
    nlashist->SetYTitle("Signal");
    push_back(nlashist);

    TH1D *nfreqhist = new TH1D("Normalized Freqency", "Normalized Frequency", 1000, 0, 1000);
    nfreqhist->SetXTitle("Number");
    nfreqhist->SetYTitle("Frequency");
    push_back(nfreqhist);

 
    TH2D *neondim = new TH2D("neondim", "Neon Dimer", 50, 0,1, 50,0,1);
    neondim->SetXTitle("X Position");
    neondim->SetYTitle("Y Position");
    push_back(neondim);

}

void freqmon::UpdateHistograms(TDataContainer &dataContainer) {

    int midasid = dataContainer.GetMidasEvent().GetSerialNumber();
    midasid = midasid%1000;

    TV1720RawData *v1720 = dataContainer.GetEventData<TV1720RawData>("DG01");

    if (v1720 && v1720->IsZLECompressed()) {
        // can't handle compressed data yet...
    }

    if (v1720 && !v1720->IsZLECompressed()) {

        TV1720RawChannel channelDatasum = v1720->GetChannelData(0);
        double sadc, smaxadc = 0;
        double sum = 0;
        float lassig = 0;

        int numsam = channelDatasum.GetNSamples();

        for (int k = 0; k < numsam ; k++)
            sum = sum + channelDatasum.GetADCSample(k);
        
        lassig = sum/numsam;
        GetHistogram(0)->SetBinContent(midasid,lassig);
/*        printf("%d\n", midasid);
        if(lassig>lasthre)
        {
            if(reflag == 0)
            {
                reflag = 1;
                GetHistogram(2)->SetBinContent(tindx,lassig);
                tindx++;
            }
            if(reflag == 1)
            {
                GetHistogram(2)->SetBinContent(tindx, lassig);
                tindx++;
            }
        }

        if(lassig<lasthre)
            if(reflag == 1)
            {
                reflag = 0;
                tindx = 0;

            }
*/
    }

    freqmeter *freqdata = dataContainer.GetEventData<freqmeter>("FPGA");

    if(freqdata)
    {
        double frequency;
        frequency = freqdata->GetFrequency();
        GetHistogram(1)->SetBinContent(midasid ,frequency);

        if(frequency > freqmax)
            freqmax = frequency;
        frequency = frequency/freqmax;

        GetHistogram(3)->SetBinContent(midasid ,frequency);
    }

}

void freqmon::Reset() {
    for (int i = 0; i < 8; i++) {  // loop over channels
        GetHistogram(i)->Reset();
    }
    lasmax = 1;
    freqmax = 1;
}

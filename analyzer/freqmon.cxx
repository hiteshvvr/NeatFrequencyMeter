#include "freqmon.h"

#include "TDirectory.h"
#include "TV1720RawData.h"
#include "TV1290Data.hxx"
#include "TLine.h"

#include "TInterestingEventManager.hxx"

void freqmon::CreateHistograms() {
    clear();

    TH1D *freqhist = new TH1D("ADC", "TimeStamping", 1000, 0, 5000);
    masshist->SetXTitle("Mass in amu");
    masshist->SetYTitle("Counts");
    push_back(freqhist);

    TH2D *neondim = new TH2D("neondim", "Neon Dimer", 50, 0,1, 50,0,1);
    neondim->SetXTitle("X Position");
    neondim->SetYTitle("Y Position");
    push_back(neondim);

}

void freqmon::UpdateHistograms(TDataContainer &dataContainer) {

    int midasid = dataContainer.GetMidasEvent.GetSerialNumber();
    midasid = midasid/1000;

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

        for (k = 0; k < numsam ; k++)
            sum = sum + channelDatasum.GetADCSample(k);
        lassig = sum/numsam;


        GetHistogram(0)->SetBinContent(midasid,lassig);
    }

    freqmeter *freqdata = dataContainer.GetEventData<freqmeter>("FPGA");

    if(freqdata)
    {
        double frequency;
        frequency = freqdata->GetFrequency();

        GetHistogram(1)->SetBinContent(midasid,frequency);
    }

}

void freqmon::Reset() {
    for (int i = 0; i < 8; i++) {  // loop over channels
        GetHistogram(i)->Reset();
    }
}

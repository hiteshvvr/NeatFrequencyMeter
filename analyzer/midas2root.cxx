// Example Program for converting MIDAS format to ROOT format.
//
// T. Lindner (Jan 2016)
//
// Example is for the CAEN V792 ADC module

#include <stdio.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <bitset>

#include "TFile.h"
#include "TRootanaEventLoop.hxx"
#include "TTree.h"
#include "TGraph.h"
#include "TNtuple.h"

#include <fstream>
// using namespace std;

#include "TAnaManager.hxx"

#ifdef USE_V792
#include "TV792Data.hxx"
#endif

#ifdef USE_V1720
#include "TV1720RawData.h"
#endif

#ifdef USE_V1290
#include "TV1290Data.hxx"
#endif

#ifdef USE_NEATEVT
#include "TV1720RawData.h"
#include "TV1290Data.hxx"
#endif

// #include "TIMEStamp.h"
#include "freqmeter.h"


class Analyzer : public TRootanaEventLoop
{
    public:
        // An analysis manager.  Define and fill histograms in
        // analysis manager.
        TAnaManager *anaManager;

        int getrawdata = 1;
        std::ofstream outfile;
#ifdef USE_FREQ
        TTree *fluxtree;

        struct fluxsig{
            int lassig;
            int freqsig;
        }sfluxsig;
        
        TH1D *lashist =  new TH1D("Laser Signal Hist", "Laser Hist", 1000, 00, 3000);
        TH1D *freqhist =  new TH1D("Freq Signal Hist", "Freq Hist", 1000, 00, 50);
#endif

#ifdef USE_NEATEVT
        // The tree to fill.
        TTree *neat;
        TGraph *gr1720, *gr1290;
        TH1D *h1290;
        float RES_1290N = 0.0245;

        struct simpleneateventdata{
            int tdc_midasid;
            int tdc_eventid;
            int tdc_num_of_hits;
            unsigned int tdc_timestampdata[5];
            int tdc_channodata[5];
            unsigned int tdc_chan0Data;
            unsigned int tdc_chan1Data;
            float tdc_tdiff;
            int digi_midasid;
            int digi_eventid;
            uint32_t digi_timetag;
            int digi_maxadcvalue[5];
            uint32_t digi_maxtimetag[5];
            double posx;
            double posy;
        } sneatevent;

        struct neateventdata{
            int tdc_midasid;
            int tdc_eventid;
            int tdc_num_of_hits;
            unsigned int tdc_timestampdata[5];
            int tdc_channodata[5];
            unsigned int tdc_chan0Data;
            unsigned int tdc_chan1Data;
            float tdc_tdiff;
            int digi_midasid;
            int digi_eventid;
            uint32_t digi_timetag;
            float digi_ch0dat[4096];
            float digi_ch1dat[4096];
            float digi_ch2dat[4096];
            float digi_ch3dat[4096];
            float digi_ch4dat[4096];
            int digi_maxadcvalue[5];
            uint32_t digi_maxtimetag[5];
            double posx;
            double posy;
        } neatevent;

#endif

#ifdef USE_V1720
        // The tree to fill.
        TTree *f1720Tree;
        TGraph *gr1720;
        TTree *neat;

        // Getting raw data if required Generates large files

        // CAEN V1720 tree variables

        struct psdEvent {
            int midasid = 0;
            int eventid = 0;
            uint32_t timetag = 0;
            int maxadc[5] = {0};
            int maxtimetag[5] = {0};
            float base[5] = {0};
            int ch0data[4096] = {0};
            int ch1data[4096] = {0};
            int ch2data[4096] = {0};
            int ch3data[4096] = {0};
            int ch4data[4096] = {0};
            float posx = 0;
            float posy = 0;
        } psevent;

        // Define the Histograms
        TH1D *a =  new TH1D("QuadA", "QuadA", 1000, 00, 5000);
        TH1D *b =  new TH1D("QuadB", "QuadB", 1000, 00, 5000);
        TH1D *c =  new TH1D("QuadC", "QuadC", 1000, 00, 5000);
        TH1D *d =  new TH1D("QuadD", "QuadD", 1000, 00, 5000);
        TH1D *hsum =  new TH1D("Sum of Channels", "hsum", 4000, 00, 12000);
        TH2D *complete = new TH2D("complete", "complete", 200, 0, 1, 200, 0, 1);
        TH2D *focused = new TH2D("focused", "focused", 200, 0.1, 0.6, 200, 0.4, 0.6);
#endif

#ifdef USE_V1290
        TTree *f1290Tree;
        TH1D *h1290;
        TGraph *gr1290;
        //    float RES_1290N = 0.0245;

        TH1D *alltdiff = new TH1D("TotalSpectrum", "TotalSpectrum", 10000, -5000, 5000);
        TH1D *shits = new TH1D("singleHit", "singleHit", 10000, 0, 5000);
        TH1D *mhits = new TH1D("multiHit", "multiHit", 5000, -5000, 5000);
        TH1D *triples2ion = new TH1D("triples2ion", "Triple With Two Ions", 5000, -7000, 7000);
        TH1D *triples2electrs = new TH1D("triples2electrs", "Triples With Two Electrons", 5000, -7000, 7000);
        TH1I *nuofhits = new TH1I("NuOfHits", "NumberOfHits", 300, -1, 100);
        TH1I *hitcounts = new TH1I("NuOfHits", "HitDistribution", 50 , -1, 20);
        TH1F *tripdis = new TH1F("TripleDis", "Triple distributed 0-eee, 1-eei, 2-eie, 3-eii, 4-iee, 5-iei, 6-iie, 7-iii", 1000, -5, 10);

        // Getting raw data if required Generates large files

        // CAEN V1290 TREE VARIABLES
        struct v1290Event {
            int midasid;
            int eventid;
            int num_of_hits;
            unsigned int timestampdata[5];
            int channodata[5];
            unsigned int chan0Data;
            unsigned int chan1Data;
            float tdiff;
        } event;

        //    std::vector<unsigned int> timestampdata;   // vector containing timestamp for each hit.
        //    std::vector<unsigned int> channodata;     // vector containing channel number for each hit.
#endif


        Analyzer() {};

        virtual ~Analyzer() {};

        void Initialize() {}

        void BeginRun(int transition, int run, int time)
        {

            if (getrawdata == 1)
            {
                TString csvfile = Form("outfiles/run_csv%05d.csv", run);
                outfile.open(csvfile);
                outfile << "Freq Counts" << "\t" <<  "Laser Signal" << "\n";
            }

            // Create a TTree

#ifdef USE_FREQ 
            fluxtree = new TTree("Flux Monitor Data", "Flux Monitor Data");

            fluxtree->Branch("Laser Signal", &sfluxsig.lassig, "lassig/I");
            fluxtree->Branch("Freq Signal", &sfluxsig.freqsig, "freqsig/I");
#endif

#ifdef USE_V1720
            f1720Tree = new TTree("v1720Data", "v1720Data");

            f1720Tree->Branch("digi_midasid", &psevent.midasid, "midasid/I");
            f1720Tree->Branch("digi_eventid", &psevent.eventid, "midasid/I");
            f1720Tree->Branch("digi_timetag", &psevent.timetag, "timetag/i");
            f1720Tree->Branch("digi_maxadcvalue", psevent.maxadc, "maxadc[5]/I");
            f1720Tree->Branch("digi_maxtimetag", psevent.maxtimetag, "maxtimetag[5]/i");
            f1720Tree->Branch("digi_base", psevent.base, "base[5]/F");
            f1720Tree->Branch("digi_ch0data", psevent.ch0data,"ch0data[4096]/I");
            f1720Tree->Branch("digi_ch1data", psevent.ch1data,"ch1data[4096]/I");
            f1720Tree->Branch("digi_ch2data", psevent.ch2data,"ch2data[4096]/I");
            f1720Tree->Branch("digi_ch3data", psevent.ch3data,"ch3data[4096]/I");
            f1720Tree->Branch("digi_ch4data", psevent.ch4data,"ch4data[4096]/I");
            f1720Tree->Branch("digi_XhitPosition", &psevent.posx,"posx/F");
            f1720Tree->Branch("digi_YhitPosition", &psevent.posy,"posy/F");
#endif

#ifdef USE_V1290

            f1290Tree = new TTree("v1290Data", "v1290Data");

            f1290Tree->Branch("tdc_midasid", &event.midasid, "midasid/I");
            f1290Tree->Branch("tdc_eventid", &event.eventid, "eventid/I");
            f1290Tree->Branch("tdc_nu_of_hits", &event.num_of_hits, "num_of_hits/I");
            f1290Tree->Branch("tdc_timestampdata", event.timestampdata, "timestampdata[num_of_hits]/I");
            f1290Tree->Branch("tdc_ChannelNumData", event.channodata, "channodata[num_of_hits]/I");
            f1290Tree->Branch("tdc_Channel0Data", &event.chan0Data, "chan0Data/I");
            f1290Tree->Branch("tdc_Channel1Data", &event.chan1Data, "chan1Data/I");
            f1290Tree->Branch("tdc_tdiff", &event.tdiff, "tdc_chan1Data/F");
#endif

#ifdef USE_NEATEVT
            neat = new TTree("NeatData", "Neat DAta Tree");
            //       neatsimpleTree = new TTree("NeatSimpleData", "Neat Simple Data Tree");

            neat->Branch("digi_midasid", &psevent.midasid, "midasid/I");
            neat->Branch("digi_eventid", &psevent.eventid, "midasid/I");
            neat->Branch("digi_timetag", &psevent.timetag, "timetag/i");
            neat->Branch("digi_maxadcvalue", psevent.maxadc, "maxadc[5]/I");
            neat->Branch("digi_maxtimetag", psevent.maxtimetag, "maxtimetag[5]/i");
            neat->Branch("digi_base", psevent.base, "base[5]/F");
            // neat->Branch("digi_ch0data", psevent.ch0data,"ch0data[4096]/I");
            // neat->Branch("digi_ch1data", psevent.ch1data,"ch1data[4096]/I");
            // neat->Branch("digi_ch2data", psevent.ch2data,"ch2data[4096]/I");
            // neat->Branch("digi_ch3data", psevent.ch3data,"ch3data[4096]/I");
            // neat->Branch("digi_ch4data", psevent.ch4data,"ch4data[4096]/I");
            neat->Branch("xpos", &psevent.posx,"posx/F");
            neat->Branch("ypos", &psevent.posy,"posy/F");
            neat->Branch("tdc_midasid", &event.midasid, "midasid/I");
            neat->Branch("tdc_eventid", &event.eventid, "eventid/I");
            neat->Branch("tdc_nu_of_hits", &event.num_of_hits, "num_of_hits/I");
            neat->Branch("tdc_timestampdata", event.timestampdata, "timestampdata[num_of_hits]/I");
            neat->Branch("tdc_ChannelNumData", event.channodata, "channodata[num_of_hits]/I");
            neat->Branch("tdc_Channel0Data", &event.chan0Data, "chan0Data/I");
            neat->Branch("tdc_Channel1Data", &event.chan1Data, "chan1Data/I");
            neat->Branch("tdc_tdiff", &event.tdiff, "tdc_chan1Data/F");

#endif

        }

        void EndRun(int transition, int run, int time) {

#ifdef USE_V1720
            a->SetXTitle("ADC Value");
            a->SetYTitle("Counts");
            a->Write("Quadrand A histogram");

            b->SetXTitle("ADC Value");
            b->SetYTitle("Counts");
            b->Write("Quadrand B histogram");

            c->SetXTitle("ADC Value");
            c->SetYTitle("Counts");
            c->Write("Quadrand C histogram");
            d->SetXTitle("ADC Value");
            d->SetYTitle("Counts");
            d->Write("Quadrand D histogram");

            hsum->SetXTitle("ADC Value");
            hsum->SetYTitle("Counts");
            hsum->Write("Sum of all Quadrand histogram");

            complete->SetXTitle("X Coordinate");
            complete->SetYTitle("Y Coordinate");
            complete->Write("PSD hit postition histogram");


            //    gr1720->Write("Single Sample Pulse");

#endif

#ifdef USE_V1290
            shits->Write("singlehits");
            mhits->Write("multiplehits");
            alltdiff->Write("completespectrum");
            nuofhits->Write("numberofhits");
            hitcounts->Write("hitdistribution");
            triples2ion->Write("triples with two ions");
            triples2electrs->Write("triples with two electrons");
            tripdis->Write("triples distribution");
#endif

            if (getrawdata == 1)
                outfile.close();
            printf("\n");
        }

        // main work here; create ttree events for every sequenced event in
        // lecroy data packets.
        bool ProcessMidasEvent(TDataContainer &dataContainer)
        {
            // psevent.midasid = dataContainer.GetMidasEvent().GetSerialNumber();
            // event.midasid = dataContainer.GetMidasEvent().GetSerialNumber();
            // if (midasid % 10 == 0) printf(".");
            // int i,k;
            int i, numsamples, j;
            //        if(getrawdata == 1)
            //outfile << "!!!" << "\n" << psevent.midasid << "\n";
            //outfile << dataContainer.GetMidasData().GetTimeStamp() << ", ";

#ifdef USE_FREQ

            freqmeter *freqdata = dataContainer.GetEventData<freqmeter>("FPGA");

            if(freqdata)
            {
                double frequency;
                double counts;
                double timedata;
                frequency = freqdata->GetFrequency();
                counts = freqdata->GetIntCount();
                timedata = freqdata->GetIntTime();
                std::cout.precision(10);
                outfile << counts << "\t";
                outfile << timedata << "\t";
                // std::cout << counts << "\t"<< timedata << "\n";
                sfluxsig.freqsig = counts;
                // outfile << std::fixed << frequency << "\t ";
                // std::cout << freqdata->GetFrequency()<< "\t" << freqdata->GetIntCount() << "\t";
                // std::cout << freqdata->GetIntTime();
                // std::cout << std::endl;
            }
#endif


#ifdef USE_V1720

            TV1720RawData *v1720 = dataContainer.GetEventData<TV1720RawData>("DG01");


            if (v1720 && !v1720->IsZLECompressed())
            {

                int numsam, k;

                TV1720RawChannel channel0data = v1720->GetChannelData(0);
                numsam = channel0data.GetNSamples();
                int adc0 = 0;
                for (k = 0; k < numsam ; k++)
                    adc0 =  adc0 + channel0data.GetADCSample(k);
                if(numsam != 0)
                adc0 = adc0/numsam;
                sfluxsig.lassig = adc0;
                outfile <<adc0<< "\n ";
            }
#endif
fluxtree->Fill();
#ifdef USE_V1290
            TV1290Data *v1290data= dataContainer.GetEventData<TV1290Data>("TDC0");
            if (!v1290data) return 0 ;
            if (v1290data) {

                int numhit;
                int startchancount = 0;
                int stopchancount = 0;
                double stoptime = 0;
                double starttime = 0;
                int startchannel = 0;
                int stopchannel = 1;

                std::vector<TDCV1290Measurement> measurements = v1290data->GetMeasurements();

                //       eventid = data->GetEventID();
                numhit = measurements.size();
                //        std::cout<< eventid<<std::endl;
                //      std::cout<< event.midasid<<std::endl;


                event.num_of_hits = numhit;
                if (numhit > 5)
                    numhit = 5;

                for (i = 0; i < numhit ; i++)
                {
                    int channo = measurements[i].GetChannel();
                    unsigned int chandata = measurements[i].GetMeasurement();
                    if (channo == stopchannel)
                    {
                        stoptime = measurements[i].GetMeasurement() * RES_1290N;
                        stopchancount++;
                    }
                    if (channo == startchannel)
                    {
                        starttime = measurements[i].GetMeasurement() * RES_1290N;
                        startchancount++;
                    }

                    event.channodata[i] = channo;
                    event.timestampdata[i] = chandata;

                    if (channo == 0)
                        event.chan0Data = chandata;
                    if (channo == 1)
                        event.chan1Data = chandata;

                    if (getrawdata == 10)
                        outfile << channo << ", " << chandata << "\n";

                    event.tdiff = event.chan1Data - event.chan0Data;
                }
                hitcounts->Fill(0);
                nuofhits->Fill(0);

                float temptdiff = 0;
                if (numhit == 2)
                {
                    double stopdata = 0;
                    double startdata = 0;

                    for (i = 0; i < numhit; i++) // loop over measurements
                    {
                        int chan = measurements[i].GetChannel();
                        if (chan == stopchannel)
                            stopdata  = measurements[i].GetMeasurement() * RES_1290N;
                        if (chan == startchannel)
                            startdata = measurements[i].GetMeasurement() * RES_1290N;
                    }

                    float tdiff = stopdata - startdata;
                    temptdiff = tdiff;
                    shits->Fill(0);
                    alltdiff->Fill(0);
                    event.tdiff = tdiff;
                }

                if (getrawdata == 188)
                    // outfile <<  ", " <<psevent.midasid<<"\t"<<temptdiff<<"\n";



                    if (numhit == 3)
                    {
                        for (i = 0; i < numhit; i++)
                        {
                            int chan = measurements[i].GetChannel();
                            double meas = measurements[i].GetMeasurement() * RES_1290N;
                            if (startchancount == 2 && chan != stopchannel)
                            {
                                float ttdiff = stoptime - meas;
                                triples2electrs->Fill(0);
                                alltdiff->Fill(0);
                            }
                            if (stopchancount == 2 && chan != startchannel)
                            {
                                float ttdiff = meas - starttime;
                                triples2ion->Fill(0);
                                alltdiff->Fill(0);
                            }
                        }

                        int chan0 = measurements[0].GetChannel();
                        // double meas0 = measurements[0].GetMeasurement() * RES_1290N;
                        int chan1 = measurements[1].GetChannel();
                        // double meas1 = measurements[1].GetMeasurement() * RES_1290N;
                        int chan2 = measurements[2].GetChannel();
                        // double meas2 = measurements[2].GetMeasurement() * RES_1290N;

                        int tdis = 4 * chan0 + 2 * chan1 + 1 * chan2;
                        tripdis->Fill(0);

                    }

                if (numhit > 3)
                {
                    for (i = 0; i < numhit; i++)
                    {
                        int chan = measurements[i].GetChannel();
                        if (chan != stopchannel)
                        {
                            float mtdiff = stoptime - measurements[i].GetMeasurement();
                            mhits->Fill(0);
                            alltdiff->Fill(0);
                        }
                    }
                }

                //f1290Tree->Fill();
            }
#endif
            // neat->Fill();

            return true;
        };

        // Complicated method to set correct filename when dealing with subruns.
        std::string SetFullOutputFileName(int run, std::string midasFilename)
        {
            char buff[128]; 
            Int_t in_num = 0, part = 0;
            Int_t num[2] = { 0, 0 }; // run and subrun values
            // get run/subrun numbers from file name
            for (int i=0; ; ++i) {
                char ch = midasFilename[i];
                if (!ch) break;
                if (ch == '/') {
                    // skip numbers in the directory name
                    num[0] = num[1] = in_num = part = 0;
                } else if (ch >= '0' && ch <= '9' && part < 2) {
                    num[part] = num[part] * 10 + (ch - '0');
                    in_num = 1;
                } else if (in_num) {
                    in_num = 0;
                    ++part;
                }
            }
            if (part == 2) {
                if (run != num[0]) {
                    std::cerr << "File name run number (" << num[0]
                        << ") disagrees with MIDAS run (" << run << ")" << std::endl;
                    exit(1);
                }
                sprintf(buff,"outfiles/output_%.6d_%.4d.root", run, num[1]);
                printf("Using filename %s\n",buff);
            } else {
                sprintf(buff,"outfiles/run_root%.5d.root", run);
            }
            return std::string(buff);
        };


};

int main(int argc, char *argv[]) {
    Analyzer::CreateSingleton<Analyzer>();
    return Analyzer::Get().ExecuteLoop(argc, argv);
}

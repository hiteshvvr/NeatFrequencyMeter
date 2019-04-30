/***********************************************************************************************
 *
 * Name: frontend.c
 * Contents: Frontend for the Frequency Meter using CAEN FPGA V2495
 *
 * $Id: frontend.c 0001 25-02-19    hitesh.rahangdale@mail.huji.ac.il
 *
 * ******************************************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "midas.h"
// #include "mcstd.h"

#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <strings.h>

#include <CAENVMElib.h>
#include <CAENVMEtypes.h>
#include "experim.h"
#include "frontend.h"

unsigned int fAddrCTS;
unsigned int fTdcLastChannelNo;
unsigned int fTdcEpochCounter;
unsigned int fCTSExtTrigger;
unsigned int fCTSExtTriggerStatus;
bool fIsIRQEnabled;
bool IsRunStopped;

// Related to Frequency Meter
DWORD ptime = 1000;
int maxrate = 10000;
int tptime = 100;
int tenum = 1000;
int datrates[10] = {0,0,0,0,0,0,0,0,0,0};

static const size_t fPacketMaxSize = 65507;
unsigned int fPacket[65507];

int DW;
int DataWaiting;
unsigned int DataToWrite[30000];
unsigned int offsetnow, offsetbefore;


void seq_callback(INT hDB, INT hseq, void *info)
{
    printf("odb ... trigger settings touched\n");
}
/****************************************************************************
  INITIALIZATION OF FRONTEND
 *****************************************************************************/
INT frontend_init()
{
    printf("Initialize Frontend::'\n");

    set_equipment_status(equipment[0].name, "Initializing...", "yellow");
    set_equipment_status(equipment[1].name, "Initializing...", "yellow");
    int i, status, csr;
    status = mvme_open(&myvme, 0);

    printf(" Status::: %d\n Hello world\n",status);

    CAENVME_DeviceReset((int) myvme->info);

    CAENVME_SetOutputConf((int) myvme->info, cvOutput0, cvDirect, cvDirect, cvManualSW);
    CAENVME_SetOutputConf((int) myvme->info, cvOutput1, cvDirect, cvDirect, cvManualSW);

    CAENVME_SetOutputRegister((int) myvme->info, cvOut1Bit);// Although using it now its used End of transmission

    CAENVME_SetInputConf((int) myvme->info, cvInput0, cvDirect, cvDirect);

    unsigned int InpDat;
    CAENVME_ReadRegister((int) myvme->info, 0x0B, &InpDat);
    printf("Input Mux Register %4.4x\n", InpDat);

    CAENVME_ReadRegister((int) myvme->info, cvInputReg, &InpDat);
    printf("Input Register %4.4x\n", InpDat);

    mvme_set_am(myvme, MVME_AM_A32_ND);                // Setting the addressing mode to 32 Bit non privileged
    mvme_set_dmode(myvme, MVME_DMODE_D16);             // Setting the Data mode to 16 Bit
    printf("I am here\n");
    /******************************************************************************
     *
     * INITIALIZE VARIOUS DEVICES
     *
     *****************************************************************************/
#if defined V2495_CODE
    printf("Setting up FPGA\n");
    v2495_FirmwareRevision(myvme,V2495_BASE_ADDR);
    v2495_EarlyWindow(myvme, V2495_BASE_ADDR, 0x0000);
    v2495_LateWindow(myvme, V2495_BASE_ADDR, 0x0000);
#endif

#if defined V1720_CODE
    //check the Digitizer
    printf("V1720\n");
    set_equipment_status(equipment[0].name, "Setting TDC", "yellow");

    // Status and Firmware version of Digitizer

    printf("Status of the Digitizer::::\n");
    v1720_Status(myvme, V1720_BASE_ADDR);

    int ser_h = v1720_RegisterRead(myvme, V1720_BASE_ADDR, 0xF080);
    int ser_l = v1720_RegisterRead(myvme, V1720_BASE_ADDR, 0xF084);
    printf("Serial Number of Digitizer:::: 0x%x%x\n", ser_l, ser_h);

    csr = mvme_read_value(myvme, V1720_BASE_ADDR, V1720_ROC_FW_VER);
    printf("V1720 ROC(motherboard)Firmware Revision: 0x%8.8x\n", csr);

    // Software reset of the Digitizer
    v1720_Reset(myvme, V1720_BASE_ADDR);

    // Setting mode 1. It sets various parameters of the device.
    v1720_Setup(myvme, V1720_BASE_ADDR, 0x03);

    printf("v1720 INITIALIZED\n");
#endif

    //-----------------CODE To Initialize CFD's--------------------------------//
#if defined V812_CODE
    // Setting Threshholds for all 16 channels of 812 model, 0 does something weird, 255 (or 0xFF) is -250ms
    printf("Setting V812 Threshholds, Width, Deadtime\n");
    for (i = 0; i < 16; i++)
    {
        v812_SetTh(myvme, V812_BASE_ADDR, 0, 150);
    }
    // Setting the Width of CFD's
    // 0x00 sets output width of 15ns and 0xFF sets width of 250ns
    v812_SetWidth(myvme, V812_BASE_ADDR, 0, 0xAF);   // from 0-7 channels
    v812_SetWidth(myvme, V812_BASE_ADDR, 1, 0xAF);   // from 8-15 channels
    // Setting deadtime for all the channels
    // 0 sets deadtime of 150ns and 255 sets 2us
    v812_SetDeadTime(myvme, V812_BASE_ADDR, 0, 255);   // from 0-7 channels
    v812_SetDeadTime(myvme, V812_BASE_ADDR, 1, 255);   // from 8-15 channels
    // Open the channel to take data
    // 0x01 opens only channel 0 and 0xFF opens all 16 channels
    v812_SetInhibit(myvme, V812_BASE_ADDR, 0xFF);

    printf("V812 Fixed Code: 0x%x\n", v812_Read16(myvme, V812_BASE_ADDR, V812_FIXED_CODE));
    printf("V812 Version Serial: 0x%x\n", v812_Read16(myvme, V812_BASE_ADDR, V812_VER_SERIAL));
#endif

    /****************Device Initialization Finished *************************/

    int gemlatch = 9; // I do not understand the use of this function
    IsRunStopped = true;
    printf("End of Frontend Initialization\n");

    //****************************TRIGGER SETTINGS*********************************

    set_equipment_status(equipment[0].name, "Started", "green");
    set_equipment_status(equipment[1].name, "Started", "green");

    // trigger_settings_str="hitesh";
    char set_str[30];
    int size;

    TRIGGER_SETTINGS_STR(trigger_settings_str);
    sprintf(set_str, "/Equipment/Trigger/Settings");
    status = db_create_record(hDB, 0, set_str, strcomb(trigger_settings_str));
    status = db_find_key(hDB, 0, set_str, &hSet);
    if (status != DB_SUCCESS) {
        cm_msg(MINFO, "FE", "Key %s not found", set_str);
        return status;
    }
    /*Enabling Hot-Linking on settings of the Equipments*/
    size = sizeof(TRIGGER_SETTINGS);
    status = db_open_record(hDB, hSet, &ts, size, MODE_READ, seq_callback, NULL);
    if (status != DB_SUCCESS) // printf("Hitesh2\n");
    return status;

    //****************************PERIODIC SETTINGS*********************************
    char per_str[30];

    PERIODIC_DISPLAY_STR(periodic_display_str);
    sprintf(per_str, "/Equipment/Periodic/Display");

    status = db_create_record(hDB, 0, per_str, strcomb(periodic_display_str));
    status = db_find_key(hDB, 0, per_str, &hper);
    if (status != DB_SUCCESS) {
        //   printf("Hitesh\n");
        //   cm_msg(MINFO,"FE","Key %s not found", per_str);
        return status;
    }
    // // /*Enabling Hot-Linking on settings of the Equipments*/
    size = sizeof(PERIODIC_DISPLAY);
    status = db_open_record(hDB, hper, &pd, size, MODE_WRITE, NULL, NULL);
    if (status != DB_SUCCESS) // printf("Hitesh2\n");
    return status;

    /////////////////////SOME TRIALS///////////////////////////////////////////////////////////
    int datam = 0;
    char trig[10];
    datam = v2495_Read16(myvme, V2495_BASE_ADDR, 0x1800);
    printf("%d\n", datam);
    sprintf(trig, "%d", datam);

    /**********************FiRMWARE REVISION OF FPGA****************************************/
    // int  v2495_FirmwareRevision(myvme,V2495_BASE_ADDR);
    pd.v2495.fpgarevno = v2495_FirmwareRevision(myvme, V2495_BASE_ADDR);

    // printf("DB_SUCCESS:::::%d\n", DB_SUCCESS);
    // set_equipment_status(equipment[1].name, "OK", "green");
    return SUCCESS;
}

/****************************************************************************
  FRONTEND EXIT FUNCTION
 *****************************************************************************/
INT frontend_exit()
{
    return SUCCESS;
}

/****************************************************************************
  GETTING USER PARAMETER
 *****************************************************************************/
INT get_user_parameters()
{
    INT Status, size;
    size = sizeof(TRIGGER_SETTINGS);
    Status = db_get_record (hDB, hSet, &ts, &size, 0);

    if (Status != DB_SUCCESS)
    {
        cm_msg(MERROR, "get_exp_settings", "Cannot retrive trigger Settings Record (size of ts = %d)", size);
        return Status;
    }

    if (Status == DB_SUCCESS)
    {
        //Setting FPGA
        printf("FPGA Early Window %d\n", ts.v2495.fpgaearlywindow);
        v2495_EarlyWindow(myvme, V2495_BASE_ADDR, ts.v2495.fpgaearlywindow);
        printf("FPGA Late Window %d\n", ts.v2495.fpgaearlywindow);
        v2495_LateWindow(myvme, V2495_BASE_ADDR, ts.v2495.fpgalatewindow);
    }

    return SUCCESS;
}


/****************************************************************************
  BEGINING THE RUN
 *****************************************************************************/

INT begin_of_run(INT run_number, char *error)
{

    INT Status, size;
    Status = get_user_parameters();

    /* read Triggger settings */

    printf("Start the Run\n");
    int i, csr, status, evtcnt, temp;
    WORD threshold[32];

    mvme_set_am(myvme, MVME_AM_A32_ND);                // Setting the addressing mode to 32 Bit non privileged
    mvme_set_dmode(myvme, MVME_DMODE_D16);             // Setting the Data mode to 16 Bit

#if defined V1720_CODE
    //check the Digitizer
    printf("V1720\n");
    csr = mvme_read_value(myvme, V1720_BASE_ADDR, V1720_ROC_FW_VER);
    printf("V1720 ROC(motherboard)Firmware Revision: 0x%x\n", csr);

    v1720_Reset(myvme, V1720_BASE_ADDR); // Software reset of the Digitizer

    v1720_Setup(myvme, V1720_BASE_ADDR, 0x03); // Setting mode 1. It sets various parameters of the device.

    // Setting the buffer size to 1k
    // v1720_RegisterWrite(myvme, V1720_BASE_ADDR, V1720_BUFFER_ORGANIZATION,  0x0A);

    // Setting the threshold
    // v1720_ChannelConfig(myvme,V1720_BASE_ADDR,0x2);
    /*  for (i = 1; i < 9; i++)
        {
    //v1720_ChannelThresholdSet(myvme, V1720_BASE_ADDR, i, 0x0868);
    //        v1720_ChannelThresholdSet(myvme, V1720_BASE_ADDR, i, 0x50);
    }*/

    v1720_ChannelDACSet(myvme,V1720_BASE_ADDR,0,0xAA00);
    v1720_ChannelDACSet(myvme,V1720_BASE_ADDR,1,0xAA00);
    v1720_ChannelDACSet(myvme,V1720_BASE_ADDR,2,0xAA00);
    v1720_ChannelDACSet(myvme,V1720_BASE_ADDR,3,0xAA00);
    v1720_ChannelDACSet(myvme,V1720_BASE_ADDR,4,0x2000);

    v1720_ChannelThresholdSet(myvme,V1720_BASE_ADDR,4,0x6A4);
    v1720_ChannelOUThresholdSet(myvme,V1720_BASE_ADDR,4,8);
    // Individual Settings for the Run:
    // Setting the post trigger value
    // v1720_RegisterWrite(myvme, V1720_BASE_ADDR, V1720_POST_TRIGGER_SETTING, 80);
    // SEtting the sample length
    // v1720_RegisterWrite(myvme, V1720_BASE_ADDR, V1720_CUSTOM_SIZE, 200);

    // v1720_RegisterWrite(myvme, V1720_BASE_ADDR, V1720_VME_CONTROL, 64);

    printf("V1720 IS Configured\n");

    ss_sleep(1000);
#endif


    //******************Setting up Devices**********************************//
    int trig, elec, ion;
    trig = v2495_GetValidEventRate(myvme, V2495_BASE_ADDR);
    elec = v2495_GetElecRate(myvme, V2495_BASE_ADDR);
    ion = v2495_GetIonRate(myvme, V2495_BASE_ADDR);
    pd.v2495.trigfreq = abs(trig);
    pd.v2495.elecfreq = abs(elec);
    pd.v2495.ionfreq = abs(ion);
    //******************Device setup Done*********************************//

    event_counter_for_interface = 0;
    offsetbefore = 0;

    printf("Waiting\n");
    for (i = 0; i < 3; i++)
    {
        ss_sleep(1000);
        printf(".\n");
    }

    printf("Starting now\n");
    printf("End 'Begin of Run'\n");

    IsRunStopped = false;

    CAENVME_SetOutputRegister((int) myvme->info, cvOut0Bit);
    CAENVME_ClearOutputRegister((int) myvme->info, cvOut0Bit);
    CAENVME_ClearOutputRegister((int) myvme->info, cvOut1Bit);

    v1720_AcqCtl(myvme, V1720_BASE_ADDR, V1720_RUN_START);

    csr = v1720_RegisterRead(myvme, V1720_BASE_ADDR, V1720_ACQUISITION_CONTROL);
    printf("V1720_ACQUISITION_CONTROL::::%8.8x\n", csr);
    printf("Begin of Run is done \n");


    return SUCCESS;
}

/****************************************************************************
  END OF RUN
 *****************************************************************************/

INT end_of_run(INT run_number, char *error)
{
    //#if defined V2495_CODE
    //check the TDC's
    printf("Restoring FPGA so no trigger is accepted\n");

    // v2495_FirmwareRevision(myvme,V2495_BASE_ADDR);
    v2495_EarlyWindow(myvme, V2495_BASE_ADDR, 0x0000);
    v2495_LateWindow(myvme, V2495_BASE_ADDR, 0x0000);
    //#endif

    IsRunStopped = true;
    fIsIRQEnabled = false;                                                    // used for disabling triggers

    CAENVME_SetOutputRegister((int) myvme->info, cvOut1Bit);
    v1720_AcqCtl(myvme, V1720_BASE_ADDR, V1720_RUN_STOP);
    printf("End of Run\n");
    return SUCCESS;
}

/****************************************************************************
  PAUSE RUN
 *****************************************************************************/
// CHECK WHAT HAPPENS WHEN YOU REMOVE THIS FROM THE FRONTEND CODE
INT pause_run(INT run_number, char *error)
{
    return SUCCESS;
}

/****************************************************************************
  RESUME RUN
 *****************************************************************************/
// CHECK WHAT HAPPENS WHEN YOU REMOVE THIS FROM THE FRONTEND CODE
INT resume_run(INT run_number, char *error)
{
    return SUCCESS;
}

/****************************************************************************
  FRONTEND LOOP
 *****************************************************************************/
INT frontend_loop()
{
    /* if frontend_call_loop is true, this routine gets called when
       the frontend is idle or once between every event */
    return SUCCESS;
}


/****************************************************************************
 *
 *
 * DATA READOUT ROUTINES
 *
 *
 ****************************************************************************/

/****************************************************************************
  Triggered event Data Availability Check
 ****************************************************************************/
//Checking if the events are available to be stored in the data bank
//*****************POLLING EVENT********************************************/
//Computer/Midas asks vme device if the data is available or not.
INT poll_event(INT source, INT count, BOOL test)
    /* Plling routine for events. Returns TRUE if event is available. If the test
     * equals TRUE, don't return. The test flag is used to time tht polling*/
{
    if(IsRunStopped == true) {return 0;} 
    //#if defined V1720_CODE
    /* v1720_SendSoftTrigger(myvme, V1720_BASE_ADDR); */
    //#endif
    /* ss_sleep(ptime);   // This line actually controls the data rate */
    //    usleep(ptime);
    int InpDat = v1720_RegisterRead(myvme, V1720_BASE_ADDR, V1720_VME_STATUS);
    InpDat = InpDat & 0x00000001;
    if(InpDat == 1)
        return 1;
    else 
        return 0;
}


//*****************INTERRUPT EVENT********************************************/
// Actually we do not use our frontend in the polling mode. I am just keeping
// it for historic reason.
// TEST : WHAT HAPPENS WHEN I REMOVE THIS BLOCK OF CODE

INT interrupt_configure(INT cmd, INT source, POINTER_T adr)
{
    switch (cmd) {
        case CMD_INTERRUPT_ENABLE:
            break;
        case CMD_INTERRUPT_DISABLE:
            break;
        case CMD_INTERRUPT_ATTACH:
            break;
        case CMD_INTERRUPT_DETACH:
            break;
    }
    return SUCCESS;
}

/****************************************************************************
 *                      DATA READOUT
 ****************************************************************************/

INT read_trigger_event(char *pevent, INT off)
{
    DWORD *fdata, *evdata;
    DWORD *ddata;
    DWORD drate,elec;
    bk_init32(pevent);
    /* int tbase = 1000000; // for Microseconds sleep */
    int tbase = 1000; // for milliseconds sleep

    //-----------------Take Data of TDC'S----------------------------------//
#if defined V2495_CODE
    drate = 0;
    bk_create(pevent, "FPGA", TID_DWORD, &fdata);                            // Create data bank for TDC

    elec = v2495_GetElecRate(myvme, V2495_BASE_ADDR);

    /* drate =  abs(abs(elec)-abs(pd.v2495.elecfreq)); */
    drate = abs(abs(elec)-tenum);
    tenum = abs(elec);

    int rate = drate*tbase/ptime;

    int datratesmax = 10;
    int avgr = 0;

    /*    for(int mm = 0; mm<datratesmax-1; mm++)
          {
          datrates[mm] = datrates[mm+1];
          avgr = avgr + datrates[mm];
          }
          datrates[9] = rate;
          avgr = avgr + datrates[9];
          */
    /* int flo_rate = avgr/datratesmax; */

    pd.v2495.elecfreq = abs(elec);

    if(drate<50000)
    {
        pd.v2495.trigfreq = rate;
    }

    if(drate<50000)
    {
        *fdata++ = drate;
        *fdata++ = ptime;     // TIME SHOULD BE IN MICROSECONDS IF USED IN ROOTANA
        *fdata++ = rate;
    }

    /* printf("Counts:: %d, Time:: %d, Rate:: %d, AvgRate:: %d \n",drate, ptime, rate, flo_rate); */
    printf("Time:: %d, Event: %d,  Rate:: %d, \n",ptime, drate , rate);

    int digrate = (drate*tbase/ptime)*4096/maxrate;
    /* int digrate = (flo_rate*tbase/ptime)*4096/maxrate; */
    if(digrate<4096)
        v1720_SetMonitorVoltageValue(myvme, V1720_BASE_ADDR, digrate);

    bk_close(pevent, fdata);
#endif

#if defined V1720_CODE
    v1720_AcqCtl(myvme, V1720_BASE_ADDR, V1720_RUN_STOP);

    int dentry = 0;
    int dextra = 0;
    int devtcnt = 0;
    int size_of_evt = 0;
    int nu_of_evt = 0;
    int readmsg = 0;
    int cycle = 0,  maxcycles = 6;  // This varialble is important to read just the first event and not let ODB overflow

    int bytes_totransfer = 0, bytes_remaining = 0 ;
    int bytes_transferred =  0;
    int bytes_max =  4096;

    bk_create(pevent, "DG01", TID_DWORD, &ddata); // Create data bank for Digitizer

    mvme_set_am(myvme, MVME_AM_A32_ND); //set addressing mode to A32 non-privileged data
    mvme_set_dmode(myvme, MVME_DMODE_D32);
    mvme_set_blt(myvme, MVME_BLT_BLT32);

    int InpDat = v1720_RegisterRead(myvme, V1720_BASE_ADDR, V1720_VME_STATUS);
    InpDat = InpDat & 0x00000001;

    if (InpDat == 1)
    {
        size_of_evt =  mvme_read_value(myvme, V1720_BASE_ADDR, 0x814C);
        nu_of_evt =  mvme_read_value(myvme, V1720_BASE_ADDR, 0x812C);

        printf("Size of Event : %d Nu of Event: %d \n", size_of_evt, nu_of_evt);
        bytes_remaining = size_of_evt * nu_of_evt * 4;     //chaning from 32bit to byte
        int toread = bytes_remaining;
        if (bytes_remaining > 1 )
        {
            *ddata = (DWORD *) malloc(bytes_remaining);
            cycle = 0;
            int buffer_address = V1720_EVENT_READOUT_BUFFER;

            while(bytes_remaining > 1 && cycle < maxcycles) 
            {                                                  
                if (bytes_remaining > 4096)
                {
                    bytes_totransfer = bytes_max;
                }
                else
                    bytes_totransfer = bytes_remaining;

                readmsg = CAENVME_BLTReadCycle((long *)myvme->info, V1720_BASE_ADDR + V1720_EVENT_READOUT_BUFFER , ddata, bytes_totransfer, cvA32_U_BLT, 0x04, &bytes_transferred);
                bytes_remaining = bytes_remaining - bytes_transferred;
                if (readmsg == 0)
                {
                    ddata += bytes_transferred/4;
                }
                if (readmsg != 0)
                {
                    printf("\n\n\n\nBLT REad Message::%d\n", readmsg);
                    printf("Cycles Requ:: %d \t Cycles READ:: %d \n", bytes_totransfer, bytes_transferred);
                }
                cycle++;
            }
        }
    }
    bk_close(pevent, ddata);
#endif


    //-----------------Bank for DAQ event counter----------------------------------//
    bk_create(pevent, "EVNT", TID_DWORD, &evdata);
    *evdata++ = event_counter_for_interface;
    bk_close(pevent, evdata);

    event_counter_for_interface++;

    // Generating a pulse
    CAENVME_SetOutputRegister((int) myvme->info, cvOut0Bit);
    CAENVME_ClearOutputRegister((int) myvme->info, cvOut0Bit);

    v1720_AcqCtl(myvme, V1720_BASE_ADDR, V1720_RUN_START);
    return bk_size(pevent);
}


INT read_periodic_event(char *levent, INT off)
{
    float *pdata;
    int a;
    int trig, elec, ion;

    /* init bank structure */
    bk_init(levent);

    /* create SCLR bank */
    bk_create(levent, "PRDC", TID_FLOAT, (void **)&pdata);
    *pdata++ = 100;

    ptime = ts.v2495.freqmeterinttime;
    maxrate = ts.v2495.freqmetermaxrate;

    trig = v2495_GetValidEventRate(myvme, V2495_BASE_ADDR);
    elec = v2495_GetElecRate(myvme, V2495_BASE_ADDR);
    ion = v2495_GetIonRate(myvme, V2495_BASE_ADDR);

    // db_set_value(hDB,0,"/Equipment/Trigger/Settings/v2495/FPGAValidEvents", &a, sizeof(a), 1 , TID_INT);
    //********************PUTTING VALUES IN ODB TREE******************************
    pd.v2495.trigfreq = abs(trig);
    pd.v2495.elecfreq = abs(elec);
    pd.v2495.ionfreq = abs(ion);

    bk_close(levent, pdata);
    return bk_size(levent);
}


/*************************************************************************************
 * DON'T KNOW EXACTLY
 ************************************************************************************/

int eventcounteroffset(int event_counter_for_interface, int gemlatch)
{
    return ( ( event_counter_for_interface & 0x7 ) - ((gemlatch & 0xf) >> 1) ) & 0x7;
}

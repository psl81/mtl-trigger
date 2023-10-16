#define LOG_STREAM_ALL  0x0001 // Continuous indefinite streaming
#define LOG_STREAM      0x0002 // Conditional streaming
#define LOG_PKV         0x0004 // Peak/valley

#define LOG_START_NOW   0x0010 // Start immediately
#define LOG_START_TIME  0x0020 // Start at specified calendar time
#define LOG_START_CYC   0x0040 // Start at specified cycle count
#define LOG_START_PULSE 0x0080 // Start on external pulse input
#define LOG_END_POINTS  0x0100 // End upon specified data points
#define	LOG_END_MILSECS 0x0200 // End upon specified duration (ms)
#define LOG_END_CYC     0x0400 // End upon specified incremental cycle count
#define LOG_END_PULSE   0x0800 // End on external pulse input

typedef struct 
{
    unsigned 
        uDSPIO,
        bShutDown,
        bPowerOn,
        bPowerHi,
        bPowerFault,
        bSGIdle,
        bSGHold,
        uBitsIn,
        uBitsOut,
        uCycleCount,
        uCycResidue,
        uBlockCount,
        uLogChannels,
        uCurrentMode,
        uAsigndPhysCh[LOG_CHANNELS];
    float 
        fSetPoint,
        fDAQRate,
        fRange[LOG_CHANNELS],
        fReadout[LOG_CHANNELS],
        fMax[LOG_CHANNELS],
        fMin[LOG_CHANNELS];
    char 
        sChID[LOG_CHANNELS][32],
        sChUnits[LOG_CHANNELS][16];
} CTRL_CHANNEL_STATUS;

//GDS Station calls
typedef int (__stdcall *GDSLogProc)(unsigned uIDCode, U_WORD *Data, unsigned uCols, unsigned uPoints);   
typedef int (__stdcall *GDSCallback)(CTRL_CHANNEL_STATUS *); 


// 1
_export int ActLimitStatus(int uCtrlCh, int uFdbk, int *bHiLimit, int *bLoLimit, int *bError);

// 2
_export void AddUpNoOfFeedbacks(void);

// 3

// 4
_export int AvailableSGBuffer(unsigned uCtrlCh);

// 5
_export int AvailableSPBuffer(unsigned uCtrlCh);

// 6
_export float BinaryToEngineering(long lRevValue, unsigned uChannel);

// 7
_export int CheckIfCurMode(unsigned uSCChannel);

// 8
_export int CheckIfLOGCh(unsigned uSCChannel);

// 9
//_export int DSPCmdOut(COMMAND_DESCRIPTOR *cmd);

// 10
_export int DSPEscOut(unsigned esc);

// 11
_export int DSPImageUpdate(void* lpAddress, unsigned uBytes, unsigned bToPC);

// 12
_export int DeLinearizeAndConvert(float fX, unsigned uCh);

// 13

// 14
_export int EngineeringToBinary(float fX, long *lRevValue, unsigned uChannel);

// 15
_export void ExtractPath(char *sPath, char *sDirectory);

// 16

// 17

// 18
_export int GDSActLimitStatus(int uCtrlCh, int uFdbk, int *bHiLimit, int *bLoLimit, int *bError);

// 19 IO Bits
_export int GDSActivateDigitalInputLatch(unsigned uLatch);

// 20

// 21
_export int GDSAltMeanCycleBlock(int uCtrlCh, unsigned iAltChannel, float fAltMean,
    float fAmplitude, float fFrequency, unsigned uQCycles, unsigned uWaveform, unsigned uQuadrant, 
    BOOL bCycleCount, BOOL bIncBlockCount);

// 22
_export float GDSApplyConversionFactor(int iCh, float fX);

// 23 Aribtrary  Calls
_export unsigned GDSArbitraryWaveformMode(int uCtrlCh);

// 24 OK
_export int GDSArmLimit(int uCtrlCh, int iEnable);
/*
    Use to arm or disarm safety limit interlocks on given actuator.
    Action will apply to all actuators in Station if in synched mode
    
    uCtrlCh - selected actuator
    iEnable
    - if < 0 (negative) call will merely return status on this actuator:
        -1 if some limit has been hit
        1 if limit interlocks are armed
        0 if disarmed
    - if 0 - limit interlocks are disarmed and status cleared
    - if 1 - limit interlocks are armed and status cleared

    returns -9999 if GDS-VM is not connected   
    returns -2 if illegal actuator channel is specified
*/

// 25

// 26

// 27

// 28
_export int GDSChangeAdaptiveControl(int uCtrlCh, int bEnable);

// 29

// 30
_export int GDSChangeFrequency(int uCtrlCh, float fFrequency);

// 31
_export int GDSChangeMeanAmplitude(int uCtrlCh, float fMean, float fAmplitude);

// 32
_export int GDSChangeResidueCycles(int uCtrlCh, unsigned uCycles);

// 33
_export int GDSChangeWaveform(int uCtrlCh, int uWaveform);

// 34

// 35

// 36 OK
_export int GDSCycleBlockControlSegment(int uCtrlCh, int iMode, float fMean, float fAmplitude, float fFrequency, 
        unsigned uQCycles, unsigned uQuadrant, unsigned uWaveform, BOOL bRelative,
        BOOL bAdaptiveControl, BOOL bConstantRate, BOOL bCycleCount, BOOL bIncBlockCount);
/*
    This call is used to generate a multiple cycle block control waveform of given mean, amplitude and rate.
    
    (int) uCtrlCh - Control channel (actuator No) on this station (starting from zero)
    
    (int) iMode - Control Mode (0-Stroke, 1-Load or 2-Straiin) to be imposed
    
    (float) fMean - Mean in engineering units
    (float) fAmplitude - Amplitude in engineering units
    (float) fFrequency - Cycling rate in Hz (or rate/s if bConstantRate = 1 (TRUE))
    (unsigned) uQCycles - No of quarter cycles to be generated. 4 will give one complete cycle
    (unsigned) uQuadrant - Start point (0-mean down, 1- min, 2 - mean-up, 3-max)
    (unsigned) uWaveform - 0-sine, 1-Ramp, 2-Pulse
    (int) bRelative - if =0, specified mean is absolute. If =1, specified mean is shift from existing readout
    (int) bAdaptiveControl - =1 if TRUE. Then, adaptive control will be imposed
    (int) bConstantrate - if rate is to be imposed rather than rate
    (int) bCycleCount - =1 - cycle counter will be updated. =0 cyclecount will remain unchanged
    (int) bIncBlockCount = 1 to increment block count upon this call. =0 to leave count unchanged
    
    returns -9999 if GDS-VM is not connected
    returns -1 if specified mean and amplitude exceed feedback range
    returns -2 if uCtrlCh exceeds available ctrl channels on station
    returns -3 if SG buffer is already full
    On success returns available buffer space in segments
*/

// 37

// 38

// 39 OK
_export int GDSDriveStatus(int uCtrlCh, int iEnable);
/*
    This call returns drive status and can change it as required.
    
    If Station is in Synched mode, the operation will apply to all channels on the station.
    If not, it will apply to selected control channel (uCtrlCh) only.
    
    If iEnable = 0, drive is switched Off.
    If iEnable = 1, drive is switched On (Lo).
    If iEnable = 2, drive is switched Hi.
    
    If iEnable = -1, current drive status will be returned as 0/1/2 respectively.
    
    returns -9999 if GDS-VM is not connected   
    returns -2 if illegal actuator channel (uCtrlCh) is specified
*/

// 40

// 41

// 42

// 43

// 44

// 45

// 46

// 47

// 48

// 49 OK
_export int GDSFetchNoOfActuators(void);

// 50

// 51
_export int GDSGetActuatorID(unsigned uAct, char *sActID);

// 52 Aribtrary  Calls
_export unsigned GDSGetArbBuffer(int uCtrlCh) ;

// 53
_export int GDSGetDataPath(char *sDataPath);

// 54
_export float GDSGetDigitalOffset(unsigned uPhysCh);

// 55

// 56

// 57

// 58
_export int GDSGetPhysChDescription(unsigned uPhysCh, float *fActiveRange, char *sChID, char *sChUnits);

// 59
_export int GDSGetPhysChNoFromActLOGChNo(unsigned uCtrlCh, unsigned uLOGCh);

// 60
_export int GDSGetPhysChOfAllocCh(unsigned uCtrlCh, unsigned uFdbk, 
    unsigned *uPhysCh, float *fActiveRange, char *sChID, char *sChUnits);

// 61
_export int GDSGetPhysicalChannel(int uCtrlCh, int uFeedbackCh);

// 62

// 63
_export int GDSGetSGTimeToCompletion(unsigned uCtrlCh, int *iSegments);

// 64
_export int GDSGetSettingsPath(char *sSettingsPath);

// 65

// 66 OK
_export int GDSHoldContinueWaveform(int uCtrlCh, int iHold);
/*
    This calls triggers immediate Hold or Continue on selected uCtrlCh Control Channel of
    this station. If Station is in Synched mode, the operation will apply to all channels on the station.
    If not, it will apply to selected control channel only.
    
    If iHold = 0, waveform generation will Continue.
    If iHold = 1, waveform will be put on Hold.
    If iHold = -1, current waveform status (Hold/Continue) will be returned as 1/0 respectively.
    
    returns -9999 if GDS-VM is not connected   
    returns -2 if illegal control channel is specified
*/

// 67

// 68

// 69

// 70

// 71
_export int GDSInitializeHardwareSettings(void);

// 72

// 73

// 74

//75

// 76 OK
_export int GDSLimitInterlocks(int uCtrlCh, unsigned uFeedbackCh, 
    float fLimit, int bHiLimit, int bStop, int bHold, int bTrip);
/*
    Use to set Limit Interlock settings on selected actuator channel's feedback channel
    
    uCtrlCh - actuator channel starting 0
    uFeedbackCh - feedback channel 0/1/2
    fLimit - limit readout
    bHiLimit - HiLimit (1) or LoLimit (0)
    bStop - activate (1) or disable (0) Stop option. Waveform generation will be
        stopped (put on Hold) if this limit is hit. Will cause similar action
        on other actuator channels if station is in synched mode
    bHold - 1-activate, 0-disable; If this is not selected control channel, 
        mode transfer will be enforced to this channel
    bTrip - 1-activate, 0-disable
    Hold and Trip will also apply to all other channels on station if station is
        in synched mode

    returns -9999 if GDS-VM is not connected   
    returns -2 if illegal actuator channel or feedback channel is specified
    returns -1 if Limit value fLimit is beyond legal range (Active Range of feedback)
*/

// 77 Aribtrary  Calls
_export int GDSModifyArbitraryCycles(int uCtrlCh, int iCode, int iCycles);

// 78 OK
int GDSInitializePacketPointer ( int *lpFrom );

// 79 OK
float GDSNextFloatPoint();

// 80 OK
int GDSNextLongPoint();

// 81

// 82
_export int GDSOffsetReadout(unsigned uFdbkCh, float fOffset);

// 83

// 84

// 85

// 86 OK
_export int GDSPeakValleySegment(int uCtrlCh, float fToSetPoint, float fGain, float fFrequency, 
        unsigned uWaveform, int bRelative, int bConstantRate);
/*



    returns -9999 if GDS-VM is not connected
    returns -1 if specified setpoint exceeds feedback range or gain is 
        too high (>100) or too low (< 0.05)
    returns -2 if uCtrlCh exceeds available ctrl channels on station
    returns -3 if SG buffer is already full
    On success returns available buffer space in segments

*/

// 87

// 88

// 89

// 90 OK
_export int GDSPresetBlockCount(int uCtrlCh, unsigned uCount);
/*
    Use to preset block counter on selected actuator to uCount
    
    returns -9999 if GDS-VM is not connected   
    returns -2 if illegal actuator channel is specified
*/

// 91 OK
_export int GDSPresetCycleCount(int uCtrlCh, unsigned uCount);
/*
    Use to preset cycle counter on selected actuator to uCount
    
    returns -9999 if GDS-VM is not connected   
    returns -2 if illegal actuator channel is specified

*/

// 92
_export int GDSRampToAltModeSetPoint(int uCtrlCh, unsigned uMode, float fToSetPoint, float fRate);

// 93

// 94  IO Bits
_export unsigned GDSReadDigitalInputLatchStatus(unsigned uLatch);

// 95

// 96

// 97 OK
_export int GDSRegisterDAQRequest(unsigned uCtrlCh, unsigned uDataType, unsigned uStartCycCount,
    unsigned uNoOfCycles, unsigned uEndPts, SYSTEMTIME StartTime, unsigned uEndTicks, GDSLogProc lpCallback);
/*

    This call schedules user selectable type, mode, periodicity and volume of data.
    
    The data log request is registered with the GDS-VM (Global Data Sharing Virtual Machine),
    which will automatically call the registered (GDSLogProc) lpCallback subroutine when data become
    available. This callback must be part of the calling program or its libraries.
    
    Multiple data log requests of different types may be filed with the GDS-VM. Each can have its own
    callback that will process the acquired data.
    
    The Data Log request is associated with the specified control channel (unsigned) uCtrlCh of the station. Each control channel
    carries its own, user-specified list of channels to be logged in real time. Tie up with a specific 
    control channel is inevitable because command signal sequence determines such parameters as Peak and Valley.
    

    (unsigned) uDataType specifies the bit pattern that describes this data log request. Or-red flags are:
    
            #define LOG_STREAM_ALL	0x0001		 // Continuous indefinite streaming 
            #define LOG_STREAM			0x0002		 // Conditional streaming
            #define LOG_PKV					0x0004		 // Peak/valley

            #define LOG_START_NOW		0x0010		 // Start immediately
            #define LOG_START_TIME	0x0020		 // Start at specified calendar time
            #define LOG_START_CYC		0x0040		 // Start at specified cycle count 
            #define LOG_START_PULSE	0x0080		 // Start on external pulse input
            #define LOG_END_POINTS	0x0100		 // End upon specified data points
            #define	LOG_END_MILSECS	0x0200		 // End upon specified duration (ms)
            #define LOG_END_CYC			0x0400		 // End upon specified incremental cycle count
            #define LOG_END_PULSE		0x0800	 	 // End on external pulse input
            
    (unsigned) uStartCycCount - cycle count (optional) when logging will commence
    
    (unsigned) uNoOfCycles - No of cycles (optional) after which, DAQ will be stopped
    
    (unsigned) uEndPts - No of data points (optional) to conclude DAQ
    
    (SYSTEMTIME) StartTime - Calendar time (optional) when DAQ should commence
    
    (unsigned) uEndTicks  - (optional) duration in ms
    
    (GDSLogProc) lpCallback - callback to be installed. This routine will be called by GDS-VM when data are available.
    NOTE: Callback argument list for data logging is NOT the same as status callback upon GDS task registration!!
    Two (and only two) different callbacks are used in applications.
    
    Return value: Positive value specifies unique token associated with this request.
    Save it to refer to request if required
    

*/

// 98 OK
_export int /*_stdcall*/ GDSRegisterStation(char *sPort, char* sPanel, int iStation, GDSCallback lpGDSTaskCall);
/* 
    This call hooks up user application onto the GDS environment. From this point on, 
    the GDS application has continuous, concurrent access to the virtual machine representing
    the test system. 
    
    As multiple independent test systems may be hooked onto the same host computer, the call needs to
    specify a "Port" (unsigned uPort) to which GDS memory access is sought. 
    
    Character string sPanel (up to 63 chars) identifies the calling GDS task to the Virtual Machine.
    This entry will assist the System Manager in monitoring active GDS tasks by their assigned names.
    
    As multiple test stations (each with one or more control channels) may be associated with the
    same controller, (int) iStation specifies the station to which this application will be connected.
    
    (GDSCallback) lpGDSTaskCal specifies the subroutine in the user application that is to be installed
    as the real-time callback into the Virtual Machine. From this moment on, the specified subroutine will
    be automatically called by the Virtual Machine to periodically 
    
    All subsequent calls to the GDS library will apply to this Station and its control channels.
    
    This call is made only once and prior to other GDS calls. 
    
    0 is returned upon normal completion
    
    -1 is returned if the call was a failure due to absence of GDS space associated with the call. Make sure station
    is connected and On-Line and make sure Port No was correct in the call
    
    -2 is returned if the application was already registered by a previous call
    
*/

// 99 OK
_export int GDSRegisterVBDAQRequest(unsigned uCtrlCh, unsigned uDataType, unsigned uStartCycCount,
    unsigned uNoOfCycles, unsigned uEndPts, unsigned uEndTicks, GDSLogProc lpCallback);

// 100 OK
_export int GDSRemoveDAQRequest(int uIDCode);

/* 
    Removes DAQ request carrying specified IDCode. Upon deletion of specified request,
        No of remaining requests is returned. -1 if no such request exists.
        
*/

// 101

// 102
_export int GDSRequestDisplayUpdate(void);

// 103

// 104

// 105 OK
_export int GDSResetControlWaveform(int uCtrlCh);
/*
    This call is used to generate a multiple cycle block control waveform of given mean, amplitude and rate.
    
    (int) uCtrlCh - Control channel (actuator No) on this station (starting from zero)

    returns -9999 if GDS-VM is not connected
    returns -2 if illegal control channel is specified 
    returns 0 on success
*/

// 106
_export int GDSResetDAQMaxMin(int uCtrlCh);

// 107

// 108

// 109

// 110
_export int GDSReturnVariableTypeOfLOGCh(unsigned uCtrlCh, unsigned uCh);

// 111

// 112
_export int GDSSCHardwareOffset(unsigned uCh, int uOffset);

// 113
_export int GDSSCPrimaryGain(unsigned uCh, int uGain);

// 114
_export int GDSSCSecondaryGain(unsigned uCh, int uGain);

// 115
_export int GDSSelectFeedbackUnits(int iCh, int uParam);

// 116

// 117

// 118

// 119

// 120 Aribtrary  Calls
_export unsigned GDSSetArbitraryDelay(int uCtrlCh, float *fMS);

// 121 Aribtrary  Calls
_export int GDSSetArbitraryWaveform(int uCtrlCh, unsigned uCycles);

// 122

// 123
_export float GDSSetDAQRate(unsigned uCtrlCh, float fDAQRate) ;

// 124 IO Bits
_export int GDSSetDigitalOutput(unsigned uLatch, unsigned uMask, unsigned uBits);

// 125

// 126

// 127
_export char* GDSSetReadoutFormat(float fRange);

// 128

// 129

// 130

// 131

// 132
_export int GDSSlowDownLogPipe(int uCtrlCh, unsigned uFactor);


// 133 OK
_export int GDSStationSyncMode(int iMode);
/*
    This calls sets synchronization mode on this station.
    If 0, synchronization is disabled. Waveform generation and control on 
        individual channels will operate independent of each other
    If 1, synchronization is enabled. Any control action on channel will affect others. Thus,
        HOLD, drive On/Off, limit-induced Hold or Reset will simultaneously affect all achannels of this station
        
    If -1, current mode will be returned. 1 indicates sync enforced. O - indicates disabled.
    returns -9999 if GDS-VM is not connected   
*/



// 134 Aribtrary  Calls
_export int GDSStopArbitraryWaveform(int uCtrlCh);

// 136 Aribtrary  Calls
_export int GDSStreamSetPoints(int uCtrlCh, int iMode, float *fX, float *fY, unsigned uPoints, int bCycle);

// 137 OK
_export int GDSSwitchControlMode(int uCtrlCh, int iMode);
/*
    Use to switch control mode on specified actuator uCtrlCh to
        new feedback iMode.
    If Waveform is busy, it will be automatically reset prior to
        mode transfer
        
    If iMode < 0, will return current Control Mode. No switch will occur

    returns -9999 if GDS-VM is not connected   
    returns -2 if illegal actuator channel is specified
    returns -1 if illegal iMode is specified (iMode > 2)
*/

// 140 OK
_export int GDSTransferActuatorStatusPacket(int uCtrlCh, LPVOID lpTo);

// 141
_export int GDSTransferBinaryData(LPVOID lpTo, LPVOID lpFrom, unsigned uPoints);

// 143 OK
_export int GDSUnregisterStation(void);

// 145
_export int GDSWaveformAddCycles(int uCtrlCh, long unsigned uCycs);

// 146
_export int GDSWaveformDwell(int uCtrlCh, float fSeconds, BOOL bIncBlockCount);

// 149
_export int HowManyLogChannels(int uActuator);

// 150
_export void InitializeAllActuatorHeaders(void);

// 151
_export void InitializeControlChannelSettings(int iCh);

// 152
_export int InstallAdditionalStatusCallback(GDSCallback lpGDSTaskCall);

// 154
_export int MSSGMeanAmpBlock(int uCtrlCh, int iMode, float fMean, float fAmplitude, float fFrequency, 
        unsigned uQCycles, unsigned uQuadrant, unsigned uWaveform, BOOL bRelative,
        BOOL bAdaptiveControl, BOOL bConstantRate, BOOL bCycleCount, BOOL bIncBlockCount);

// 155
_export int OffsetReadout(unsigned uSCChannel, float fOffset);

// 156
_export int PeakValleySegment(int uCtrlCh, float fToSetPoint, float fGain, float fFrequency, 
        unsigned uWaveform, int bRelative, int bConstantRate);

// 157
_export BOOL RTConfirmPopup(char *sTitle, char *sMessage);

// 158
_export void RTMessagePopup(char *sTitle, char *sMessage);

// 159
_export float RefreshDAQRates(void);

// 160
_export int RegisterGDSDAQRequest(unsigned uCtrlCh, unsigned uDataType, unsigned uStartCycCount,
    unsigned uNoOfCycles, unsigned uEndPts, SYSTEMTIME StartTime, unsigned uEndTicks, GDSLogProc lpCallback);

// 161
_export void ReverseFloat(float fx, float *f);

// 162
_export unsigned long ReverseLong(unsigned long ul);

// 163
_export int SGBlockSegment(int iMode, float fMean, float fAmplitude, float fFrequency, 
        unsigned uQCycles, unsigned uQuadrant, unsigned uWaveform, BOOL bRelative,
        BOOL bAdaptiveControl, BOOL bConstantRate, BOOL bCycleCount, BOOL bIncBlockCount);

// 166
//_export int StreamWaveformSegment(unsigned uCtrlCh, SG_STEP *sg);


//_export float GDSSystemUpdateInterval(void);

//  IO Bits
//_export int GDSGenerateDigitalPulse(unsigned uLatch, unsigned uBit, unsigned uOnMSTicks, unsigned uOffMSTicks);





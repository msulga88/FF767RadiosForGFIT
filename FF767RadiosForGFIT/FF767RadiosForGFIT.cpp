// This plugin for XPlane11 is intended for use with:
// GoFlight GF166 Radio stack hardware (and GoFlight Interface Tool)
// FlightFactor Boeing 767-300
//
// The FlightFactor Boeing 767-300 VHF radios depart from the typical design
// of radios in GA aicraft.
// GA aicraft typically have an active freqency and standby frequency display
// that swaps these frequencies when the flip button is pressed
//
// Typical GA application (Active frequency is always on the left):
// 
//     Active Freq                      Standby Freq
//   Left LED display(GF166)       Right LED display(GF166)
//       118.10            <-Flip->               119.40
//                    Flip button is pressed
//       119.40            <-Flip->               118.10
//
// On Boeing 767-300 radios there is a physical 2-way toggle switch which
// points to the active frequency and the frequencies don't swap sides
//
// Boeing 737-300 application (Active Frequency can be left or right selected by toggle switch):
//   Left LED display(GF166)       Right LED display(GF166)
//       118.10            <--Toggle Active        119.40
//                    Flip button is pressed
//       118.10            Toggle Active-->        119.40
//
// This plugin acts like an adapter between native GA functionality
// and 767-300. It prevents the frequencies from swapping sides when
// the flip button is pressed on the GF166 radio stack hardware, but
// Xplane contiues to use the same native datarefs to represent
// the active and standby frequencies.
//
// Installation pre-requisites
// 1) Flight Factor 767-300
// 2) 2 x GF166
// 3) Go Filght Interface Tool 
// 4) DataRefs.txt (attached) copied to B767 aicraft folder
// 5) Go Flight Interface Tool native GA configuration
//    modified to use new datarefs (in DataRefs.txt) for
//    Left and Right LED displays in GF166 config

#ifndef XPLM200
#error This example requires the v2.0 SDK or newer
#endif

#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMScenery.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


// com1
XPLMDataRef vhf1LeftFreq = NULL;
XPLMDataRef vhf1RightFreq = NULL;
XPLMDataRef com1StdbyFreqHz = NULL;
XPLMDataRef com1FreqHz = NULL;
XPLMDataRef com1ActiveSide = NULL;

// com2
XPLMDataRef vhf2LeftFreq = NULL;
XPLMDataRef vhf2RightFreq = NULL;
XPLMDataRef com2StdbyFreqHz = NULL;
XPLMDataRef com2FreqHz = NULL;
XPLMDataRef com2ActiveSide = NULL;

float PluginDependencyCallback
(
    float inElapsedSinceLastCall,
    float inElapsedTimeSinceLastFlightLoop,
    int inCounter,
    void* inRefcon
);

static float VHFRadioCallback
(
    float inElapsedSinceLastCall,
    float inElapsedTimeSinceLastFlightLoop,
    int inCounter,
    void* inRefcon
);

int     vhf1LeftFreqValue;
int     vhf1RightFreqValue;
int     GetVhf1LeftFreq(void* inRefcon);
void    SetVhf1LeftFreq(void* inRefcon, int outValue);
int     GetVhf1RightFreq(void* inRefcon);
void    SetVhf1RightFreq(void* inRefcon, int outValue);
int     previousCom1StdbyFreqHz;
bool     previousCom1RightIsActive;

int     vhf2LeftFreqValue;
int     vhf2RightFreqValue;
int     GetVhf2LeftFreq(void* inRefcon);
void    SetVhf2LeftFreq(void* inRefcon, int outValue);
int     GetVhf2RightFreq(void* inRefcon);
void    SetVhf2RightFreq(void* inRefcon, int outValue);
int     previousCom2StdbyFreqHz;
bool     previousCom2RightIsActive;


PLUGIN_API int XPluginStart(
    char* outName,
    char* outSig,
    char* outDesc)
{

    // Plugin Info
    strcpy(outName, "FF767RadiosForGFIT");
    strcpy(outSig, "msulga.ff767radiosforgfit");
    strcpy(outDesc, "Creates custom datarefs for B737 radio stack for readiing by GFIT.");

    //  Create our custom integer dataref
    // com1
    vhf1LeftFreq = XPLMRegisterDataAccessor("msulga/radios/FFB767vhf1LeftFreq",
        xplmType_Int,                                  // The types we support
        1,                                             // Writable
        GetVhf1LeftFreq, SetVhf1LeftFreq,              // Integer accessors
        NULL, NULL,                                    // Float accessors
        NULL, NULL,                                    // Doubles accessors
        NULL, NULL,                                    // Int array accessors
        NULL, NULL,                                    // Float array accessors
        NULL, NULL,                                    // Raw data accessors
        NULL, NULL);                                   // Refcons not used

    vhf1RightFreq = XPLMRegisterDataAccessor("msulga/radios/FFB767vhf1RightFreq",
        xplmType_Int,                                  // The types we support
        1,                                             // Writable
        GetVhf1RightFreq, SetVhf1RightFreq,            // Integer accessors
        NULL, NULL,                                    // Float accessors
        NULL, NULL,                                    // Doubles accessors
        NULL, NULL,                                    // Int array accessors
        NULL, NULL,                                    // Float array accessors
        NULL, NULL,                                    // Raw data accessors
        NULL, NULL);                                   // Refcons not used

    // com2
    vhf2LeftFreq = XPLMRegisterDataAccessor("msulga/radios/FFB767vhf2LeftFreq",
        xplmType_Int,                                  // The types we support
        1,                                             // Writable
        GetVhf2LeftFreq, SetVhf2LeftFreq,              // Integer accessors
        NULL, NULL,                                    // Float accessors
        NULL, NULL,                                    // Doubles accessors
        NULL, NULL,                                    // Int array accessors
        NULL, NULL,                                    // Float array accessors
        NULL, NULL,                                    // Raw data accessors
        NULL, NULL);                                   // Refcons not used

    vhf2RightFreq = XPLMRegisterDataAccessor("msulga/radios/FFB767vhf2RightFreq",
        xplmType_Int,                                  // The types we support
        1,                                             // Writable
        GetVhf2RightFreq, SetVhf2RightFreq,            // Integer accessors
        NULL, NULL,                                    // Float accessors
        NULL, NULL,                                    // Doubles accessors
        NULL, NULL,                                    // Int array accessors
        NULL, NULL,                                    // Float array accessors
        NULL, NULL,                                    // Raw data accessors
        NULL, NULL);                                   // Refcons not used

    // Find and intialize our Counter dataref
    //com 1
    vhf1LeftFreq = XPLMFindDataRef("msulga/radios/FFB767vhf1LeftFreq");
    vhf1RightFreq = XPLMFindDataRef("msulga/radios/FFB767vhf1RightFreq");
    com1StdbyFreqHz = XPLMFindDataRef("sim/cockpit/radios/com1_stdby_freq_hz");
    com1FreqHz = XPLMFindDataRef("sim/cockpit/radios/com1_freq_hz"); 
    //com1ActiveSide = XPLMFindDataRef("1-sim/vhf/1/active");
    //com1ActiveSide = XPLMFindDataRef("sim/cockpit/electrical/taxi_light_on"); // uncomment for testing when FF767 not installed

    //com 2
    vhf2LeftFreq = XPLMFindDataRef("msulga/radios/FFB767vhf2LeftFreq");
    vhf2RightFreq = XPLMFindDataRef("msulga/radios/FFB767vhf2RightFreq");
    com2StdbyFreqHz = XPLMFindDataRef("sim/cockpit/radios/com2_stdby_freq_hz");
    com2FreqHz = XPLMFindDataRef("sim/cockpit/radios/com2_freq_hz");
    //com2ActiveSide = XPLMFindDataRef("1-sim/vhf/2/active");
    //com2ActiveSide = XPLMFindDataRef("sim/cockpit/electrical/taxi_light_on"); // uncomment for testing when FF767 not installed

    XPLMSetDatai(vhf1LeftFreq, 0);
    XPLMSetDatai(vhf1RightFreq, 0);
    XPLMSetDatai(vhf2LeftFreq, 0);
    XPLMSetDatai(vhf2RightFreq, 0);

    previousCom1StdbyFreqHz = XPLMGetDatai(com1StdbyFreqHz);
    previousCom1RightIsActive = XPLMGetDatai(com1ActiveSide) == 1;
    previousCom2StdbyFreqHz = XPLMGetDatai(com2StdbyFreqHz);
    previousCom2RightIsActive = XPLMGetDatai(com2ActiveSide) == 1;

    /* Register FlightLoop Callbacks */
    XPLMRegisterFlightLoopCallback(
        PluginDependencyCallback, // Callback
        -1.0, // Interval
        NULL);

    return 1;
}


PLUGIN_API void     XPluginStop(void)
{
    XPLMUnregisterDataAccessor(vhf1LeftFreq);
    XPLMUnregisterDataAccessor(vhf1RightFreq);
    XPLMUnregisterDataAccessor(vhf2LeftFreq);
    XPLMUnregisterDataAccessor(vhf2RightFreq);

    XPLMUnregisterFlightLoopCallback(
        VHFRadioCallback,
        NULL);

    XPLMUnregisterFlightLoopCallback(
        PluginDependencyCallback,
        NULL);
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API int XPluginEnable(void)
{
    return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID    inFromWho,
    long             inMessage,
    void* inParam)
{
}

float VHFRadioCallback
(
    float inElapsedSinceLastCall,
    float inElapsedTimeSinceLastFlightLoop,
    int inCounter,
    void* inRefcon
)
{
    // com1
    int currentCom1StdbyFreqHz = XPLMGetDatai(com1StdbyFreqHz);
    int currentCom1FreqHz = XPLMGetDatai(com1FreqHz);

    bool currentCom1RightIsActive = false;

    if (com1ActiveSide)
    {
        currentCom1RightIsActive = XPLMGetDatai(com1ActiveSide) == 1;
        //std::string rightIsActiveStr = std::string("rightIsActive is: ") + (rightIsActive ? "TRUE" : "FALSE") + "\n";
        //XPLMDebugString(rightIsActiveStr.c_str());
        //return 2;
    }
    else
    {
        XPLMDebugString("MSI: 1-sim/vhf/1/active is NULL\n");
        com1ActiveSide = XPLMFindDataRef("1-sim/vhf/1/active");
        //com1ActiveSide = XPLMFindDataRef("sim/cockpit/electrical/taxi_light_on");
        return 5; //wait 5 second for the next callback
    }

    bool com1StdbyFreqHzHasChanged = currentCom1StdbyFreqHz != previousCom1StdbyFreqHz;
    bool com1ActiveSideHasChanged = currentCom1RightIsActive != previousCom1RightIsActive;
    
    //if (currentCom1StdbyFreqHz != previousCom1StdbyFreqHz || currentRightIsActive != previousCom1RightIsActive)
    if(com1StdbyFreqHzHasChanged || com1ActiveSideHasChanged)
    {
        if (currentCom1RightIsActive)
        {
            //std::string rightIsActiveStr = std::string("rightIsActive is: TRUE\n");
            //XPLMDebugString(rightIsActiveStr.c_str());
            XPLMSetDatai(vhf1LeftFreq, currentCom1StdbyFreqHz);
            XPLMSetDatai(vhf1RightFreq, currentCom1FreqHz);
        }
        else
        {
            //std::string rightIsActiveStr = std::string("rightIsActive is: FALSE\n");
            //XPLMDebugString(rightIsActiveStr.c_str());
            XPLMSetDatai(vhf1LeftFreq, currentCom1FreqHz);
            XPLMSetDatai(vhf1RightFreq, currentCom1StdbyFreqHz);
        }

        previousCom1StdbyFreqHz = currentCom1StdbyFreqHz;
        previousCom1RightIsActive = currentCom1RightIsActive;
    }

    //com 2
    int currentCom2StdbyFreqHz = XPLMGetDatai(com2StdbyFreqHz);
    int currentCom2FreqHz = XPLMGetDatai(com2FreqHz);

    bool currentCom2RightIsActive = false;

    if (com2ActiveSide)
    {
        currentCom2RightIsActive = XPLMGetDatai(com2ActiveSide) == 1;
        //std::string rightIsActiveStr = std::string("rightIsActive is: ") + (rightIsActive ? "TRUE" : "FALSE") + "\n";
        //XPLMDebugString(rightIsActiveStr.c_str());
        //return 2;
    }
    else
    {
        XPLMDebugString("MSI: 1-sim/vhf/2/active is NULL\n");
        com2ActiveSide = XPLMFindDataRef("1-sim/vhf/2/active");
        //com2ActiveSide = XPLMFindDataRef("sim/cockpit/electrical/taxi_light_on");
        return 5; //wait 5 second for the next callback
    }

    bool com2StdbyFreqHzHasChanged = currentCom2StdbyFreqHz != previousCom2StdbyFreqHz;
    bool com2ActiveSideHasChanged = currentCom2RightIsActive != previousCom2RightIsActive;

    //if (currentCom2StdbyFreqHz != previousCom2StdbyFreqHz || currentCom2RightIsActive != previousCom2RightIsActive)
    if (com2StdbyFreqHzHasChanged || com2ActiveSideHasChanged)
    {
        if (currentCom2RightIsActive)
        {
            //std::string rightIsActiveStr = std::string("rightIsActive is: TRUE\n");
            //XPLMDebugString(rightIsActiveStr.c_str());
            XPLMSetDatai(vhf2LeftFreq, currentCom2StdbyFreqHz);
            XPLMSetDatai(vhf2RightFreq, currentCom2FreqHz);
        }
        else
        {
            //std::string rightIsActiveStr = std::string("rightIsActive is: FALSE\n");
            //XPLMDebugString(rightIsActiveStr.c_str());
            XPLMSetDatai(vhf2LeftFreq, currentCom2FreqHz);
            XPLMSetDatai(vhf2RightFreq, currentCom2StdbyFreqHz);
        }

        previousCom2StdbyFreqHz = currentCom2StdbyFreqHz;
        previousCom2RightIsActive = currentCom2RightIsActive;
    }

    return -1.0;
}

//com1 Get and Set
int     GetVhf1LeftFreq(void* inRefcon)
{
    return vhf1LeftFreqValue;
}

void	SetVhf1LeftFreq(void* inRefcon, int inValue)
{
    vhf1LeftFreqValue = inValue;
}

int     GetVhf1RightFreq(void* inRefcon)
{
    return vhf1RightFreqValue;
}

void	SetVhf1RightFreq(void* inRefcon, int inValue)
{
    vhf1RightFreqValue = inValue;
}

//com2 Get and Set
int     GetVhf2LeftFreq(void* inRefcon)
{
    return vhf2LeftFreqValue;
}

void	SetVhf2LeftFreq(void* inRefcon, int inValue)
{
    vhf2LeftFreqValue = inValue;
}

int     GetVhf2RightFreq(void* inRefcon)
{
    return vhf2RightFreqValue;
}

void	SetVhf2RightFreq(void* inRefcon, int inValue)
{
    vhf2RightFreqValue = inValue;
}

/*
 * PluginDependencyCallback
 *
 * This is the only flight loop call back registered by XPluginStart.
 * Its purpose is to detect 1-sim Boeing 757/767 plugin before
 * trying to access any of its datarefs.
 */
float PluginDependencyCallback
(
    float inElapsedSinceLastCall,
    float inElapsedTimeSinceLastFlightLoop,
    int inCounter,
    void* inRefcon
)
{
    //static int pluginCallCount = 0;

    XPLMPluginID boeing767ID =
        XPLMFindPluginBySignature("ru.stsff.757767avionics");

    if (boeing767ID == XPLM_NO_PLUGIN_ID)
    {
        XPLMDebugString("FF767RadiosForGFIT: ru.stsff.757767avionics plugin not found\n");
        //if (pluginCallCount >= 5)
        //{
        //    // no sign of STMA Hangar Ops plugin after 5 seconds
        //    XPLMDebugString("HangarOpsToSlave: Disabling HangarOpsToSlave\n");
        //    XPLMDisablePlugin(XPLMGetMyID());
        //}
    }
    else
    {
        XPLMDebugString("FF767RadiosForGFIT: ru.stsff.757767avionics plugin found\n");
        if (XPLMIsPluginEnabled(boeing767ID) != 0)
        {
            XPLMDebugString("FF767RadiosForGFIT: ru.stsff.757767avionics plugin enabled - registering VHFRadioCallback\n");
            XPLMRegisterFlightLoopCallback(
                VHFRadioCallback, // Callback
                2.0, // 2 sec delay to next callback to allow ru.stsff.757767avionics plugin to setup
                NULL);

            com1ActiveSide = XPLMFindDataRef("1-sim/vhf/1/active");
            com2ActiveSide = XPLMFindDataRef("1-sim/vhf/2/active");

            return 0;
        }
        else
        {
            XPLMDebugString("FF767RadiosForGFIT: Waiting for ru.stsff.757767avionics plugin to be enabled\n");
        }

    }

    /* Call me every 5 seconds to check 1-sim Boeing 757/767 plugin is loaded and enabled */
    //pluginCallCount++;
    return 5.0;
}
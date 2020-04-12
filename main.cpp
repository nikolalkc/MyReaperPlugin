// check if the build environment is Windows, and then include the Windows API header,
// else include SWELL which provides functions with the same names but are implemented
// for another operating system (namely, OS-X)
#ifdef _WIN32
#include <windows.h>
#include "WDL/WDL/win32_utf8.h"
#pragma warning ( disable : 4267 ) // size_t to int
#pragma warning ( disable : 4244 ) // double to int
#pragma warning ( disable : 4800 ) // 'unsigned __int64': forcing value to bool 'true' or 'false' (performance warning)
#else
#include "WDL/WDL/swell/swell.h"
#endif

#include "WDL/WDL/lice/lice.h"
#define REAPERAPI_IMPLEMENT
#include "reaper_plugin/reaper_plugin_functions.h"

#include "utilfuncs.h"
#include "reaper_action_helper.h"
#include "reaper_function_helper.h"

#include <stdio.h>
#include <string>
#include <functional>
#include <vector>
#include <memory>


//ja dodao
#include "icontheme.h"

reaper_plugin_info_t* g_plugin_info = nullptr;
REAPER_PLUGIN_HINSTANCE g_hInst; // handle to the dll instance. could be useful for making win32 API calls
HWND g_parent; // global variable that holds the handle to the Reaper main window, useful for various win32 API calls

#include "main.hpp" 
#include "reascript.hpp" /*** HERE THE FUNCTIONS DO THEIR WORK ***/



void doAction1() {
	ShowMessageBox("Hello World!", "Reaper extension", 0);
}


//MOJ KOD
static int iRulerLaneCol[3];
static int iTrackBackgroundCol[2];
static int ripple_state = -1; //0 - off, 1 - per track, 2 - all tracks
//


void SetTimelineYellow()
{
	static int iRulerLaneCol[3];
	static int iTimelineBGColor;


	int iSize;
	ColorTheme* colors = (ColorTheme*)GetColorThemeStruct(&iSize);

	//moj kod
 //   iTimelineBGColor = colors->timeline_bgcolor;
 //   colors->timeline_bgcolor = RGB(200,150,0);
	//colors->itembgcolor = RGB(200, 100, 50);
	//colors->peaks[0] = RGB(200, 200, 0);
	//colors->peaks[1] = RGB(200, 150, 0);
	//colors->peakssel2[0] = RGB(200, 200, 0)*0.7;
	//colors->peakssel2[1] = RGB(200, 200, 0)*0.7;
	//colors->arrange_bg = RGB(255, 0, 255);

	colors->trackbgs[0] = RGB(87, 72, 45);
	colors->trackbgs[1] = RGB(78, 71, 61);
	for (int i = 0; i < 3; i++)
	{
		iRulerLaneCol[i] = colors->ruler_lane_bgcolor[i];
		colors->ruler_lane_bgcolor[i] = RGB(200, 150, 0);
	}
	UpdateTimeline();
	//Main_OnCommand(40311, 0); //ripple all
}


void SetTimelineBlue()
{
	static int iRulerLaneCol[3];
	static int iTimelineBGColor;

	int iSize;
	ColorTheme* colors = (ColorTheme*)GetColorThemeStruct(&iSize);

	colors->trackbgs[0] = RGB(44, 60, 73);
	colors->trackbgs[1] = RGB(34, 50, 63);
	for (int i = 0; i < 3; i++)
	{
		iRulerLaneCol[i] = colors->ruler_lane_bgcolor[i];
		colors->ruler_lane_bgcolor[i] = RGB(85, 200, 255);
	}
	UpdateTimeline();
	//Main_OnCommand(40310, 0); //ripple per track
}

void SetTimelineGray()
{
	//static int iRulerLaneCol[3];
	//static int iTimelineBGColor;

	int iSize;
	ColorTheme* colors = (ColorTheme*)GetColorThemeStruct(&iSize);

	colors->trackbgs[0] = iTrackBackgroundCol[0];
	colors->trackbgs[1] = iTrackBackgroundCol[1];
	for (int i = 0; i < 3; i++)
	{
		colors->ruler_lane_bgcolor[i] = iRulerLaneCol[i];
	}
	UpdateTimeline();
	//Main_OnCommand(40309, 0); //ripple off
}

// just a dummy example, not used ATM (commented in the entry point)
void hookPostCommandProc(int iCmd, int flag)
{
	//WDL_FastString str;
	//str.SetFormatted(512, "hookPostCommandProc: %s, flag=%d\r\n", kbd_getTextFromCmd(iCmd, NULL), flag);
	//ShowConsoleMsg(str.Get());

	if (iCmd == 40310) //per track 40311 --all tracks , 40309 --off
	{
		//ShowConsoleMsg("RIPPLE PER TRACK\r\n");
		SetTimelineBlue();
	}
	else if (iCmd == 40311) {
		//ShowConsoleMsg("RIPPLE ALL TRACKS\r\n");
		SetTimelineYellow();
	}
	else if (iCmd == 40309) {
		//ShowConsoleMsg("RIPPLE OFF\r\n");
		SetTimelineGray();
	}
	else if (iCmd == 1155) {
		//ShowConsoleMsg("CYCLE RIPPLE MODE");
		//ShowConsoleMsg("RIPPLE PER TRACK:");
		//ShowConsoleMsg("RIPPLE ALL TRACKS:");

		//char buf2[128] = "";
		//sprintf(buf2, "---> %d\r\n", per_track_state);
		//ShowConsoleMsg(buf2);


	}

	

}

void SaveOriginalTimelineColors() {
	int iSize;
	ColorTheme* original_colors = (ColorTheme*)GetColorThemeStruct(&iSize);
	for (int i = 0; i < 3; i++)
	{
		iRulerLaneCol[i] = original_colors->ruler_lane_bgcolor[i];
	}

	iTrackBackgroundCol[0] = original_colors->trackbgs[0];
	iTrackBackgroundCol[1] = original_colors->trackbgs[1];

}

int GetRippleState() {
	int cycle_state = GetToggleCommandState(1155);
	int per_track_state = GetToggleCommandState(40310);
	int all_tracks_state = GetToggleCommandState(40311);

	if (cycle_state == 1 && per_track_state == 1 && all_tracks_state == 0) {
		return 1;
	}
	else if (cycle_state == 1 && per_track_state == 0 && all_tracks_state == 1) {
		return 2;
	}
	else
	{
		return 0;
	}
}

void SetRippleColors() {

	int current_ripple_state = GetRippleState();

	if (current_ripple_state != ripple_state)
	{
		if (ripple_state == 0)
			SaveOriginalTimelineColors();

		switch (current_ripple_state)
		{
		default:
			break;
		case 0:
			SetTimelineGray();
			break;
		case 1:
			SetTimelineBlue();
			break;
		case 2:

			SetTimelineYellow();
			break;
		}
	}

	ripple_state = current_ripple_state;
}



extern "C"
{
	// this is the only function that needs to be exported by a Reaper extension plugin dll
	// everything then works from function pointers and others things initialized within this
	// function
	REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec) {
		g_hInst=hInstance;
		if (rec) {
			if (rec->caller_version != REAPER_PLUGIN_VERSION || !rec->GetFunc)
				return 0; /*todo: proper error*/
			g_plugin_info = rec;
			g_parent = rec->hwnd_main;

			// load all Reaper API functions in one go, byebye ugly IMPAPI macro!
			int error_count = REAPERAPI_LoadAPI(rec->GetFunc);
			if (error_count > 0)
			{
				char errbuf[256];
				sprintf(errbuf, "Failed to load %d expected API function(s)", error_count);
				MessageBox(g_parent, errbuf, "MRP extension error", MB_OK);
				return 0;
			}

#ifndef WIN32
			// Perhaps to get Reaper faders on OSX...
			SWELL_RegisterCustomControlCreator((SWELL_ControlCreatorProc)rec->GetFunc("Mac_CustomControlCreator"));
#endif
			// Use C++11 lambda to call the doAction1() function that doesn't have the action_entry& as input parameter
			//add_action("LKC++ - RIPPLE YELLOW", "LKC_TIMELINEYELLOW", CannotToggle, [](action_entry&) { SetTimelineYellow(); });
			//
			//add_action("LKC++ - RIPPLE BLUE", "LKC_TIMELINEBLUE", CannotToggle, [](action_entry&) { SetTimelineBlue(); });
			//
			//add_action("LKC++ - RIPPLE GRAY", "LKC_TIMELINEGRAY", CannotToggle, [](action_entry&) { SetTimelineGray(); });

			// Pass in the doAction2() function directly since it's compatible with the action adding function signature
			//auto togact = add_action("Simple extension togglable test action", "EXAMPLE_ACTION_02", ToggleOff, doAction2);

				// Add functions
#define func(f) add_function(f, #f)
			func(MRP_DoublePointer);
			func(MRP_IntPointer);
			func(MRP_CalculateEnvelopeHash);
			func(MRP_DoublePointerAsInt);
			func(MRP_CastDoubleToInt);
			func(MRP_ReturnMediaItem);
			func(MRP_DoNothing);

			func(MRP_CreateWindow);
			func(MRP_DestroyWindow);
			func(MRP_WindowIsClosed);
			func(MRP_WindowSetTitle);
			func(MRP_WindowAddControl);
			func(MRP_SetControlBounds);
			func(MRP_WindowIsDirtyControl);
			func(MRP_WindowClearDirtyControls);
			func(MRP_GetControlFloatNumber);
			func(MRP_SetControlFloatNumber);
			func(MRP_GetControlIntNumber);
			func(MRP_SetControlIntNumber);
			func(MRP_SetControlString);
			func(MRP_SendCommandString);
			func(MRP_GetWindowDirty);
			func(MRP_SetWindowDirty);
			func(MRP_GetWindowPosSizeValue);
#ifdef REASCRIPTGUIWORKS
			func(MRP_GetControlText);
			func(MRP_SetControlText);
			func(MRP_WindowAddSlider);
			func(MRP_WindowAddButton);
			func(MRP_WindowAddLineEdit);
			func(MRP_WindowAddLabel);
			func(MRP_WindowAddLiceControl);
			
#endif
			func(MRP_CreateArray);
			func(MRP_DestroyArray);
			func(MRP_GenerateSine);
			func(MRP_WriteArrayToFile);
			func(MRP_MultiplyArrays);
			func(MRP_SetArrayValue);
			func(MRP_GetArrayValue);
#ifdef WIN32
			func(MRP_MultiplyArraysMT);
#endif
#undef func

			if (!rec->Register("hookcommand2", (void*)hookCommandProcEx)) { 
				MessageBox(g_parent, "Could not register hookcommand2", "MRP extension error", MB_OK);
			}
			if (!rec->Register("toggleaction", (void*)toggleActionCallback)) { 
				MessageBox(g_parent, "Could not register toggleaction", "MRP extension error", MB_OK);
			}
			
			//MOJ KOD****************************************************************************************
			//if (!rec->Register("hookpostcommand", (void*)hookPostCommandProc)) //ja dodao
			//	MessageBox(g_parent, "Could not register hookpostcommand", "MRP extension error", MB_OK);
			
			SaveOriginalTimelineColors(); //init setup
			//ripple_state = GetRippleState();

			plugin_register("timer", (void*)SetRippleColors);

			//MOJ KOD END***********************************************************************************

			if (!RegisterExportedFuncs(rec)) { /*todo: error*/ }

			start_or_stop_main_thread_executor(false);
			return 1; // our plugin registered, return success
		}
		else {
			test_pcm_source(1);
			start_or_stop_main_thread_executor(true);
			return 0;
		}
	}
};

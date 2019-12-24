#include <Bela.h>


#include <libpd/z_libpd.h>
extern "C" {
#include <libpd/s_stuff.h>
};

//Librairies utiles
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <bits/stdc++.h>

//Headers custom
#include "LCDscreen.h"
#include "MenuFile.h"






//Constantes Audio 
static unsigned int gAnalogChannelsInUse;
static unsigned int gLibpdBlockSize;
static unsigned int gChannelsInUse;
static unsigned int gFirstAnalogInChannel;
static unsigned int gFirstAnalogOutChannel;



//Variables Audio
float* gInBuf;
float* gOutBuf;

//Instances Générales
int gDataPin[8] = {66,67,69,68,45,44,47,46};
LCD Ecran(86,88,gDataPin);

Menu MainMenu(&Ecran, "Presets/Liste.txt");

Button BoutonScrollUp(50);
Button BoutonScrollDown(51);
Button BoutonValidate(60);

void *gPatch;

AuxiliaryTask MenuMainLoop;
void *arg;



//Variables Custom
bool gPatchOpen = 0;









//Fonction Settings
void Bela_userSettings(BelaInitSettings *settings)
{
settings->uniformSampleRate = 1;
settings->interleave = 0;
settings->analogOutputsPersist = 0;
}

//Hook
void Bela_printHook(const char *received){
rt_printf("%s", received);
}

//ChangePatch
bool ChangePatch(std::string PatchToOpen)
{
	//If there's already a patch running we want to close it before anything else
	if(gPatchOpen)
	{
		//The render loop will no longer process 
		gPatchOpen = 0;
		
		//close dsp of libpd
		libpd_start_message(1);
		libpd_add_float(0.0f);
		libpd_finish_message("pd", "dsp");
		
		//Close the patch
		libpd_closefile(gPatch);

	}
	
	
	//Checking if the pd file exists
	int n = PatchToOpen.length();
	char file[n + 1];
	std::strcpy(file, PatchToOpen.c_str());
	
	char folder[] = ".//Presets/";
	unsigned int strSize = strlen(file) + strlen(folder) + 1;
	char* str = (char*)malloc(sizeof(char) * strSize);
	snprintf(str, strSize, "%s%s", folder, file);
	if(access(str, F_OK) == -1 ) {
		printf("Error file %s%s not found. The %s file should be your main patch.\n", folder, file, file);
		return false;
	}
	free(str);
	
	//If the pd file exists
	
	
	//Starting DSP
	libpd_start_message(1);
	libpd_add_float(1.0f);
	libpd_finish_message("pd", "dsp");
	
	
	// open patch:
	gPatch = libpd_openfile(file, folder);
	if(gPatch == NULL){
		printf("Error: file %s/%s is corrupted.\n", folder, file); 
		return false;
		
	}
	
	//The render loop can go on and process 
	gPatchOpen = 1;

	return true;
	
}




//MainLoop
void MainLoop(void*)
{
	while(!gShouldStop)
	{
		
		//Just a bunch of scrolling on LCD screen to select the Patch we want to run
		if(BoutonScrollUp.CheckState())
		{
			MainMenu.Scroll(0);
			MainMenu.UpdateScreen();
		}
		if(BoutonScrollDown.CheckState())
		{
			MainMenu.Scroll(1);
			MainMenu.UpdateScreen();
		}
		
		//If we choose a patch
		if(BoutonValidate.CheckState())
		{
			MainMenu.SelectPreset();
			MainMenu.UpdateScreen();
			
			if(MainMenu.getActivePreset() == "quit") //If the selected option is to quit
			{
				system("halt");
			}
			else
			{
				ChangePatch(MainMenu.getActivePreset());
			}
			
		}
			
		usleep(10000);
	}
}










bool setup(BelaContext *context, void *userData)
{
	// Check Pd's version
	int major, minor, bugfix;
	sys_getversion(&major, &minor, &bugfix);
	printf("Running Pd %d.%d-%d\n", major, minor, bugfix);
	
	
	// We requested in Bela_userSettings() to have uniform sampling rate for audio
	// and analog and non-interleaved buffers.
	// So let's check this actually happened
	if(context->analogSampleRate != context->audioSampleRate)
	{
		fprintf(stderr, "The sample rate of analog and audio must match. Try running with --uniform-sample-rate\n");
		return false;
	}
	if(context->flags & BELA_FLAG_INTERLEAVED)
	{
		fprintf(stderr, "The audio and analog channels must be interleaved.\n");
		return false;
	}
	
	//Printing instructions on LCD screen
	MainMenu.UpdateScreen();
	
	// analog setup
	gAnalogChannelsInUse = context->analogInChannels;
	printf("Audio channels in use: %d\n", context->audioOutChannels);
	printf("Analog channels in use: %d\n", gAnalogChannelsInUse);
	
	// Channel distribution
	gFirstAnalogInChannel = std::max(context->audioInChannels, context->audioOutChannels);
	gFirstAnalogOutChannel = gFirstAnalogInChannel;
	gChannelsInUse = gFirstAnalogInChannel + std::max(context->analogInChannels, context->analogOutChannels);
	
	
	// check that we are not running with a blocksize smaller than gLibPdBlockSize
	gLibpdBlockSize = libpd_blocksize();
	if(context->audioFrames < gLibpdBlockSize){
		fprintf(stderr, "Error: minimum block size must be %d\n", gLibpdBlockSize);
		return false;
	}
	
	// set hooks before calling libpd_init
	libpd_set_printhook(Bela_printHook);
	
	
	
	//initialize libpd. This clears the search path
	libpd_init();
	//Add the current folder to the search path for externals
	libpd_add_to_search_path(".");
	libpd_add_to_search_path("../pd-externals");
	
	libpd_init_audio(gChannelsInUse, gChannelsInUse, context->audioSampleRate);
	gInBuf = get_sys_soundin();
	gOutBuf = get_sys_soundout();
	
	

	//Lauching the Menu in an AuxiliaryTask
	MenuMainLoop = Bela_createAuxiliaryTask(&MainLoop, 50, "MenuMainLoop", arg);
	Bela_scheduleAuxiliaryTask(MenuMainLoop);

	return true;
}



void render(BelaContext *context, void *userData)
{
	if(gPatchOpen) //Checking if we are currently running a patch
	{
		
		unsigned int numberOfPdBlocksToProcess = context->audioFrames / gLibpdBlockSize;
	
		// Remember: we have non-interleaved buffers and the same sampling rate for
		// analogs, audio and digitals
		for(unsigned int tick = 0; tick < numberOfPdBlocksToProcess; ++tick)
		{
			//audio input
			for(int n = 0; n < context->audioInChannels; ++n)
			{
				memcpy(
					gInBuf + n * gLibpdBlockSize,
					context->audioIn + tick * gLibpdBlockSize + n * context->audioFrames, 
					sizeof(context->audioIn[0]) * gLibpdBlockSize
				);
			}
	
			// analog input
			for(int n = 0; n < context->analogInChannels; ++n)
			{
				memcpy(
					gInBuf + gLibpdBlockSize * gFirstAnalogInChannel + n * gLibpdBlockSize,
					context->analogIn + tick * gLibpdBlockSize + n * context->analogFrames, 
					sizeof(context->analogIn[0]) * gLibpdBlockSize
				);
			}
			
			
	
			libpd_process_sys(); // process the block
	
			
			// audio output
			for(int n = 0; n < context->audioOutChannels; ++n)
			{
				memcpy(
					context->audioOut + tick * gLibpdBlockSize + n * context->audioFrames, 
					gOutBuf + n * gLibpdBlockSize,
					sizeof(context->audioOut[0]) * gLibpdBlockSize
				);
			}
	
			//analog output
			for(int n = 0; n < context->analogOutChannels; ++n)
			{
				memcpy(
					context->analogOut + tick * gLibpdBlockSize + n * context->analogFrames, 
					gOutBuf + gLibpdBlockSize * gFirstAnalogOutChannel + n * gLibpdBlockSize,
					sizeof(context->analogOut[0]) * gLibpdBlockSize
				);
			}
			
		}
		
	}
	
}


void cleanup(BelaContext *context, void *userData)
{
	
	libpd_closefile(gPatch);
}

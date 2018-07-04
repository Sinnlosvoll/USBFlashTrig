#include <stdio.h>
#include <libusb.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <getopt.h>

#include "../common/defines.h"
#include "FlashTrig.cpp"

using namespace std;

void PrintHelp()
{
    std::cout <<
    		" Options " << endl << 
            "  --trigger              -t            Trigger the camera, but don't flash" << endl <<
            "  --flash-and-trigger    -f            Trigger the camere, also flash" << endl <<
            "  --light-on             -o            Turn the light on" << endl <<
            "  --light-off            -l            Turn the light off" << endl <<
            "  --light-state          -c            Fetch the light state [0|1]" << endl <<
            "  --set-flash-time       -s <val>      Set the time, the flash is on when flash-and-triggering" << endl <<
            "  --get-flash-time       -i            Fetch the set flash time" << endl <<
            "  --help                 -h            Print help" << endl ;

    exit(1);
}


int main(int argc, char *argv[])
{
	// Parse arguments	
	int num = 0;
	bool state = false;
	uint16_t time = -1;
	int selectedCommand = 0;

	static struct option long_opts[] = {
		{"trigger",			no_argument, 		0,  't' },
		{"flash-and-trigger", no_argument,		0,  'f' },
		{"light-on",  		no_argument, 		0,  'o' },
		{"light-off", 		no_argument,		0,  'l' },
		{"light-state",  	no_argument, 		0,  'c' },
		{"help",  			no_argument, 		0,  'h' },
		{"set-flash-time",	required_argument, 	0,  's' },
		{"get-flash-time",	no_argument, 		0,  'i' },
		{0,					0,					0,   0 }
	};


	while (true) {
        const auto opt = getopt_long(argc, argv, "htfolcsig", long_opts, nullptr);

        if (-1 == opt)
            break;

    	if(opt == 'h') {
    		PrintHelp();
    		break;
		}
		if(opt == 't') {
			selectedCommand = FT_CMD_TRIGGER;
			break;
		}
		if(opt == 'f') {
			selectedCommand = FT_CMD_FLASH_AND_TRIGGER;
			break;
		}
		if(opt == 'o') {
			selectedCommand = FT_CMD_LIGHT_ON;
			break;
		}
		if(opt == 'l') {
			selectedCommand = FT_CMD_LIGHT_OFF;
			break;
		}
		if(opt == 'c') {
			selectedCommand = FT_CMD_LIGHT_STATE;
			break;
		}
		if(opt == 's') {
			selectedCommand = FT_CMD_FLASH_TIME_SET;
			time = stoi(optarg);
			break;
		}
		if(opt == 'i') {
			selectedCommand = FT_CMD_FLASH_TIME_GET;
			break;
		}
		
		PrintHelp();
		break;
	}

	// Init Flashtrig device
	FlashTrig * ft = new FlashTrig();

	if (!ft->isOkay)
	{
		cout << "FlashTrig failed initialisation" << endl;
		exit(1);
	}



	// Execute selected option
	switch(selectedCommand) {
		case FT_CMD_TRIGGER:
			cout << "Triggering ";
			ft->trigger();
			ft->isOkay ? cout << "successful" : cout << "failed";
			break;

		case FT_CMD_FLASH_AND_TRIGGER:
			cout << "Flash and Triggering ";
			ft->flashAndTrigger();
			ft->isOkay ? cout << "successful" : cout << "failed";
			break;

		case FT_CMD_LIGHT_ON:
			cout << "Turning light on ";
			ft->setLight(true);
			ft->isOkay ? cout << "successful" : cout << "failed";
			break;

		case FT_CMD_LIGHT_OFF:
			cout << "Turning light off ";
			ft->setLight(false);
			ft->isOkay ? cout << "successful" : cout << "failed";
			break;

		case FT_CMD_LIGHT_STATE:
			cout << "Fetching light state ";
			state = ft->lightState();
			ft->isOkay ? cout << "successful. State is " << state : cout << "failed";
			break;

		case FT_CMD_FLASH_TIME_SET:
			cout << "Setting flash time" << endl;
			ft->setFlashTime(time);
			ft->isOkay ? cout << "successful. Time is " << time : cout << "failed";
			break;

		case FT_CMD_FLASH_TIME_GET:
			cout << "Fetching flash time" << endl;
			time = ft->getFlashTime();
			ft->isOkay ? cout << "successful. Time is " << time : cout << "failed";
			break;
	}
	cout << endl;
	return 0;
}
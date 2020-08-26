#include "cmdargs.h"
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <termios.h>

using namespace std;

CmdArgs::CmdArgs(int argc, char** argv ){
	int c;
	// let cmdargs overwrite the defaults
	while ((c = getopt (argc, argv, "d:c:m:s:h")) != -1){
		switch (c){
			case 'd':
			    if( strcmp("trace", optarg) == 0){
			        loglevel = spdlog::level::trace;
                }else if( strcmp("info", optarg) == 0){
                    loglevel = spdlog::level::info;
                }else if( strcmp("debug", optarg) == 0){
                    loglevel = spdlog::level::debug;
                }else{
                    cout << "Error, loglevel (" << optarg << ") unknown" << std::endl;
                    exit(0);
                }
			    break;
            case 'c':
                cfgfile = std::string( optarg);
                break;
            case 'm':
                micdev = std::string( optarg);
                break;
            case 's':
                spkdev = std::string( optarg);
                break;
			case 'h':
				cout << "hotword" << endl;
//				cout << "   -c FILE         : read config file" << endl;
				cout << "   -d [LEVEL]      : set debug level (trace, info, debug)" << endl;
				cout << "   -m ALSADEV      : set micrpohone ALSA device (default='respeaker')" << endl;
				cout << "   -s ALSADEV      : set speaker ALSA device (default='respeaker_speaker')" << endl;
				exit(0);
				break;
		}
	}
    

}

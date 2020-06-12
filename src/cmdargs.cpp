#include "cmdargs.h"
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <termios.h>

using namespace std;

CmdArgs::CmdArgs(int argc, char** argv ){
	int c;
	loglevel = spdlog::level::info;
	while ((c = getopt (argc, argv, "d:p:b:h")) != -1){
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
			case 'h':
				cout << "hotword" << endl;
				cout << "   -d [LEVEL]      : set debug level (trace, info, debug)" << endl;
				exit(0);
				break;
		}
	}
}

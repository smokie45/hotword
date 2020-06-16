#include "cmdargs.h"
#include "alsadev.h"
#include <iostream>
using namespace std;

int main(int argv, char **argc) {

    // parse cmdline arguments
    CmdArgs* arg = new CmdArgs( argv, argc );
    // set debug level
    spdlog::set_level( arg->loglevel );
    spdlog::set_pattern("# [%L] %v");

    cout << "adev= " << arg->alsadev << endl;

    if( alsadev_open_capture( arg->alsadev ) ){

        char* buf;
        // buf = Samplerate * bytes per sample * channel * seconds
        //      = 48000 * 2 (16bit) * 2 * .1 (100ms)
        int srate = 48000;          // samplerate
        int frame_size = 2 * 2;     // 2 byte per sample, 2 channel
        int sec = 3;
        buf = (char*) malloc( srate * frame_size * sec );

        int request = srate / 10;    // size for 100ms
        cout << "Start recording ..." << endl;
        void* b = (void*) buf;
        for( int i=0; i < 10*sec; i++){
            int num = alsadev_capture( b, request);
            if( num != request ){
                spdlog::debug("Got {}, but requested {}", num, request );
            }
            cout << "got buffer num " << i << " [ " << b << " ] " << endl;
            b = (void*) ((char*)b + num*frame_size);
        }
        alsadev_close_capture();

        cout << "waiting ..." << endl;
        sleep( 3 );

        if( alsadev_open_play( arg->alsadev ) ){
            cout << "Start playback ..." << endl;
            void* b = (void*) buf;
            for( int i=0; i < 10*sec; i++){
                int num = alsadev_play( b, request );
                if( num != request){
                    spdlog::debug("played {}, but send {}", num, request );
                }
                cout << " buffer num "  << i << " [ " << b << " ] " << endl;
                b = (void*) ((char*)b +  num * frame_size);
                usleep(90000);
            }
            alsadev_close_play();
        }
        free( buf );
    }
    return 0;
}

#include "cmdargs.h"
#include "alsadev.h"
#include <iostream>
#include <time.h>
using namespace std;


#include "pv_porcupine.h"
#include "fvad.h"

pv_porcupine_t*  pp_init(){
    const char *model_path = "external/porcupine/lib/common/porcupine_params.pv";
    const char *keyword_path = "external/porcupine/resources/keyword_files/linux/picovoice_linux.ppn";
    const float sensitivity = 0.5f;

    pv_porcupine_t *handle;

    const pv_status_t status = pv_porcupine_init(
            model_path,
            1,
            &keyword_path,
            &sensitivity,
            &handle);

    if (status != PV_STATUS_SUCCESS) {
        spdlog::error("Error on init porcupine");
        // error handling logic
        return NULL;
    }
    return handle;
}
void pp_delete( pv_porcupine_t* pp){
    pv_porcupine_delete( pp );
}

int pp_detect( pv_porcupine_t* pp, const int16_t* pcm){
    int32_t keyword_index;
    const pv_status_t status = pv_porcupine_process( pp, pcm, &keyword_index);
    if (status != PV_STATUS_SUCCESS) {
        spdlog::error("Error on detecting hotword");
        return -1;
    }
    if (keyword_index != -1) {
        spdlog::info("Hotword detected !");
        return 1;
    }
    return 0;
}
class Timer {
    timespec t1, t2;
    bool isStarted = false;

    public:
    Timer( void ){};
    void start( void ){
        clock_gettime( CLOCK_MONOTONIC, &t1);
        isStarted = true;
    }
    void stop( void ){
        isStarted = false;
    }

    bool isElapsed( int sec ){
        timespec t;
        if( isStarted ){
            clock_gettime( CLOCK_MONOTONIC, &t2);
            t = diff( t1, t2);
            if( t.tv_sec >= sec ){
                return true;
            }
        }
        else {
            // timer was not started, so we start it now
            start();
        }
        return false;
    }

    private:
    timespec diff(timespec start, timespec end) {
        timespec temp;
        if ((end.tv_nsec-start.tv_nsec)<0) {
            temp.tv_sec = end.tv_sec-start.tv_sec-1;
            temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
        } else {
            temp.tv_sec = end.tv_sec-start.tv_sec;
            temp.tv_nsec = end.tv_nsec-start.tv_nsec;
        }
        return temp;
    }
};

// bool isTimeElapsed( timespec t1, int sec){
//     struct timespec t2,t;
//     clock_gettime( CLOCK_MONOTONIC, &t2);
//     t = diff( t1, t2);
//     if( t.tv_sec > sec ){
//         return true;
//     }
//     return false;
// }
//

int main(int argv, char **argc) {

    // parse cmdline arguments
    CmdArgs* arg = new CmdArgs( argv, argc );
    // set debug level
    spdlog::set_level( arg->loglevel );
    spdlog::set_pattern("# [%L] %v");

    cout << "Hotword detection on ALSA device ' " << arg->alsadev << "'" << endl;

    if( alsadev_open_capture( arg->alsadev ) ){

#if 1
        pv_porcupine_t* pp = pp_init();
        int32_t frame_length = pv_porcupine_frame_length();
        spdlog::debug("Using frame len of {}", frame_length);
        // TODO: optimize buffer -> ch =1 ?1?
        int16_t *pcm = (int16_t*)malloc(frame_length * sizeof(int16_t)*2);
        if (!pcm) {
            spdlog::error("Failed on malloc()");
            exit(1);
        } 

       enum State { DETECT, RECORD }state = DETECT;
       // struct timespec t1;
       Fvad *vad = fvad_new();
       Timer timeout, silence;
       while( true ){ 
            int num = alsadev_capture( pcm, frame_length);
            if( num != frame_length ){
                spdlog::error("Read {} frames instead of {}", num, frame_length);
                continue;
            }
            switch( state ) {
                case DETECT:
                    if( pp_detect( pp, pcm ) == 1 ){
                        state = RECORD;
                        frame_length = 480;
                        fvad_reset( vad );
                        fvad_set_sample_rate( vad, 16000);
                        fvad_set_mode( vad, 2);
                        timeout.start();
                        spdlog::info("Start recording");
                    }
                    break;
                case RECORD:
                    // if( isTimeElapsed( t1, 5) ){
                    if( timeout.isElapsed( 5 ) ){
                        spdlog::info("Stop recording due to timeout");
                        state = DETECT;
                        frame_length = 512;
                        break;
                    }
                    // fvad requires 10,20,30... ms buffers. For 16kHz it's 160, 320, 480, ...
                    int ret = fvad_process( vad, pcm, 480);
                    if( ret == 0 ){
                        cout << "." << std::flush; 
                        // voice inactive
                        // if( isTimeElapsed( t2, 2) ){
                        if( silence.isElapsed( 2) ){
                            spdlog::info("Stop recording due to voice inactivity");
                            state = DETECT;
                            frame_length = 512;
                            silence.stop();
                        }
                        break;
                    }
                    else if (ret = 1){
                        cout << "v" << std::flush; 
                        // detected voice, stop silence timer
                        silence.stop();
                    }
                    else if( ret == -1){
                        cout << "E" << std::flush; 
                    }
                    break;
                    // else if( isSilence( pcm ) ){
                    //
                    // }
            }
       }
        alsadev_close_capture();
        fvad_free( vad );
        pp_delete( pp );
#else
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
#endif
    }
    return 0;
}

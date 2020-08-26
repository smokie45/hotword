#include "cmdargs.h"
#include "audio.h"
#include <iostream>
#include "pv_porcupine.h"
#include "fvad.h"
#include "timer.h"
#include "net.h"

using namespace std;

// TODO: add yaml configuration file

// initialize the porcupine library
pv_porcupine_t*  pp_init(){
    const char *model_path   = "/usr/share/porcupine/porcupine_params.pv";
    const char *keyword_path = "/usr/share/porcupine/keyword_files/picovoice.ppn";
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

// delete porcupine handler and free ressources
void pp_delete( pv_porcupine_t* pp){
    pv_porcupine_delete( pp );
}

// Take given audio samples and try to detect the hotword
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

int main(int argv, char **argc) {

    // parse cmdline arguments
    CmdArgs* cfg = new CmdArgs( argv, argc );
    // set debug level
    spdlog::set_level( cfg->loglevel );
    spdlog::set_pattern("# [%L] %v");
    cout << "hotword " << VERSION << endl;
    cout << "ALSA devices, capture='" << cfg->micdev << "', playback='" << cfg->spkdev << "'" << endl;

    Audio mic( cfg->micdev, Audio::CAPTURE, 16000, 1);
    // Audio player( "respeaker_play", Audio::PLAYBACK, 44100, 2);
    Audio player( cfg->spkdev, Audio::PLAYBACK, 44100, 2);

    if( !mic.open() ){
        spdlog::error("Failed to open mic !");
        return -1;
    }

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
    Fvad *vad = fvad_new();
    Timer timeout_timer, novoice_timer;
    Net udp( "localhost", 12345);
    while( true ){ 
        // fetch some audio samples
        int num = mic.capture( pcm, frame_length);
        if( num != frame_length ){
            spdlog::error("Read {} frames instead of {}", num, frame_length);
            continue;
        }
        switch( state ) {
            case DETECT:
                if( pp_detect( pp, pcm ) == 1 ){
                    // porcupine detected hotword ...
                    player.playFile ("/usr/share/hotword/res/beep_hi.wav");
                    state = RECORD;
                    frame_length = 480;     // for fvad
                    fvad_reset( vad );
                    fvad_set_sample_rate( vad, 16000);
                    fvad_set_mode( vad, 2);     // set aggresivness of vad
                    timeout_timer.start();
                    spdlog::info("Start recording");
                }
                break;
            case RECORD:
                // send pcm data to remote udp server
                udp.send( pcm, frame_length );
                if( timeout_timer.isElapsed( 5 ) ){
                    spdlog::info("Stop recording due to timeout");
                    player.playFile ("/usr/share/hotword/res/beep_lo.wav");
                    state = DETECT;
                    frame_length = 512;     // for porcupine
                    break;
                }
                // check for voice activity using fvad
                // fvad requires 10,20,30... ms buffers. For 16kHz it's 160, 320, 480, ...
                int ret = fvad_process( vad, pcm, 480);
                if( ret == 0 ){
                    cout << "." << std::flush; 
                    // voice inactive
                    if( novoice_timer.isElapsed( 2) ){
                        spdlog::info("Stop recording due to voice inactivity");
                        player.playFile ("/usr/share/hotword/res/beep_lo.wav");
                        state = DETECT;
                        frame_length = 512;     // for porcupine
                        novoice_timer.stop();
                    }
                    break;
                }
                else if (ret == 1){
                    cout << "v" << std::flush; 
                    // detected voice, stop novoice_timer
                    novoice_timer.stop();
                }
                else if( ret == -1){
                    cout << "E" << std::flush; 
                }
                //TODO: send samples by udp
                break;
        }  //switch( state )
    } // while(1)
    mic.close();
    free( pcm );
    fvad_free( vad );
    pp_delete( pp );
    return 0;
}

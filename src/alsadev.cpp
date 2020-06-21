#include "alsadev.h"
#include <alsa/asoundlib.h>
#include <iostream>
#include "spdlog/spdlog.h"
// Globals are generally a bad idea in code.  We're using one here to keep it
// simple.
#define FUNC_TRACE spdlog::trace("{}", __PRETTY_FUNCTION__);

snd_pcm_t *_capdev;
snd_async_handler_t *capcb;

snd_pcm_t *_playdev;
using namespace std;

bool _init_params( snd_pcm_t* _adev ){
    int err;
    snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
	// *_INTERLEAVED: data is represented in frames of one left and right sample only
	// *_NONINTERLEAVED: data is represented as 'period' samples, first n left samples followed by n right samples. 'period' is 2*n
	// *_RW_*: access using snd_pcm_[read, write]
	// *_MMAP_*: acces writing to buffer pointer
	snd_pcm_access_t access = SND_PCM_ACCESS_RW_INTERLEAVED;
    // unsigned int rate = 48000;
    unsigned int rate = 16000;
    int channels = 1;
    // int periods = 2;
    int periods = 2;
    // snd_pcm_uframes_t buffersize = 48000; // size in frames
    snd_pcm_uframes_t buffersize = 256; // size in frames -> is same as rate we have 1s buff
    
    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        spdlog::error( "Can't alloc HW param struct [{}]", snd_strerror (err));
        return false;
    }

    if ((err = snd_pcm_hw_params_any (_adev, hw_params)) < 0) {
        spdlog::error( "Can't init HW param struct [{}]", snd_strerror (err));
        return false;
    }

    // TODO: for capture we need NONINTERLEAVED
    if ((err = snd_pcm_hw_params_set_access ( _adev, hw_params, access )) < 0) {
        spdlog::error( "Can't set access type [{}]", snd_strerror (err));
        return false;
    }
    if ((err = snd_pcm_hw_params_set_format (_adev, hw_params, format)) < 0) {
        spdlog::error( "Can't set format [{}]", snd_strerror (err));
        return false;
    }
    // cout << "Format width=" << snd_pcm_format_width(format) << endl;

    if ((err = snd_pcm_hw_params_set_rate_near (_adev, hw_params, &rate, 0)) < 0) {        
        spdlog::error( "Can't set rate [{}]", snd_strerror (err));
        return false;
    }
    if ((err = snd_pcm_hw_params_set_channels (_adev, hw_params, channels)) < 0) {
        spdlog::error( "Can't set channels [{}]", snd_strerror (err));
        return false;
    }
        if (snd_pcm_hw_params_set_periods(_adev, hw_params, periods, 0) < 0) {
        spdlog::error( "Can't set periods [{}]", snd_strerror (err));
        return false;
    }
    // set buffer size in frames
    if (snd_pcm_hw_params_set_buffer_size(_adev, hw_params, buffersize ) < 0) {
        spdlog::error( "Can't set buffer size [{}]", snd_strerror (err));
        return false;
    }

    if ((err = snd_pcm_hw_params (_adev, hw_params)) < 0) {
        spdlog::error( "Can't set HW params  [{}]", snd_strerror (err));
        return false;
    }
    snd_pcm_hw_params_free (hw_params);
    if ((err = snd_pcm_prepare (_adev)) < 0) {
        spdlog::error( "Can't prepare cpature device [{}]", snd_strerror (err));
        return false;
    }

    spdlog::debug( "ALSA HW params configured" );
    return true;
}

bool alsadev_open_capture( std::string adev ){
    int err;
    
    //  mode = 0 - standard (block?), SND_PCM_NONBLOCK, SND_PCM_ASYNC (sigio usage)    
    // TODO: switch to SND_PCM_ASYNC and utilize sigio
    if ((err = snd_pcm_open (&_capdev, adev.c_str() , SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        spdlog::error( "Can't open alsa device {} [{}]", adev.c_str(), snd_strerror (err));
        return false;
    }
    spdlog::debug("Opened {}", adev);
    err = _init_params( _capdev );

#ifdef USE_CALLBACK
    if ((err = snd_async_add_pcm_handler( &capcb, _capdev,  MyCallback, NULL)) < 0 ){
        spdlog::error( "Can't install callback");
        return false;
    }
#endif
    return err;
}

bool alsadev_open_play( std::string adev){
    int err;
    
    //  mode = 0 - standard (block?), SND_PCM_NONBLOCK, SND_PCM_ASYNC (sigio usage)    
    // TODO: switch to SND_PCM_ASYNC and utilize sigio
    if ((err = snd_pcm_open (&_playdev, adev.c_str(), SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        spdlog::error( "Can't open alsa device {} [{}]", adev.c_str(), snd_strerror (err));
        return false;
    }
    spdlog::debug("Opened {}", adev);
    return _init_params( _playdev );
}

int alsadev_capture(void *buf, int num ){
    int err;
    if ((err = snd_pcm_readi( _capdev, buf, num)) < 0){
        spdlog::error( "Error, reading from ALSA [{}]", snd_strerror (err));
    }
    spdlog::trace("Buffer [{} / {} B] captured", err, num);
    return err;
}

int  alsadev_play( void* buf, int num){
    int err;
    if ((err = snd_pcm_writei( _playdev, buf, num)) < 0){
        spdlog::error( "Error, writing to ALSA [{}]", snd_strerror (err));
        snd_pcm_prepare( _playdev );
    }
    spdlog::trace("Buffer [{} / {} B] played", err, num);
    return err;
}

bool alsadev_close_capture() {
  snd_pcm_close( _capdev);
  spdlog::debug("Capture device closed");
  return true;
}

bool alsadev_close_play() {
  snd_pcm_close( _playdev);
  spdlog::debug("Playback device closed");
  return true;
}

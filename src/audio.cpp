#include "audio.h"
#include <alsa/asoundlib.h>
#include <sndfile.h>
#include <iostream>
#include "spdlog/spdlog.h"
// Globals are generally a bad idea in code.  We're using one here to keep it
// simple.
#define FUNC_TRACE spdlog::trace("{}", __PRETTY_FUNCTION__);

Audio::Audio( std::string adev, Audio::Type type, unsigned int srate, int ch){
    _name = adev;

    if( type == Audio::CAPTURE ){
        _stream = SND_PCM_STREAM_CAPTURE;
        _dir = "CAPTURE";
    }
    else{
        _stream = SND_PCM_STREAM_PLAYBACK;
        _dir = "PLAYBACK";
    }
    _samplerate = srate;
    _channels = ch;
}

bool Audio::open(){
    if( _isOpen ){
        spdlog::trace("ALSA device [{}] is already open ... continue", _name);
        return true;
    }
    int err;
    //  mode = 0 - standard (block?), SND_PCM_NONBLOCK, SND_PCM_ASYNC (sigio usage)    
    // TODO: switch to SND_PCM_ASYNC and utilize sigio
    if ((err = snd_pcm_open (&_adev, _name.c_str(), _stream, 0)) < 0) {
        spdlog::error( "Can't open alsa device {} [{}]",_name , snd_strerror (err));
        return false;
    }
    spdlog::debug("Opened ALSA device '{}' for {}", _name, _dir);
    
    snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
	// *_INTERLEAVED: data is represented in frames of one left and right sample only
	// *_NONINTERLEAVED: data is represented as 'period' samples, first n left samples followed by n right samples. 'period' is 2*n
	// *_RW_*: access using snd_pcm_[read, write]
	// *_MMAP_*: acces writing to buffer pointer
	snd_pcm_access_t access = SND_PCM_ACCESS_RW_INTERLEAVED;
    int periods = 2;
    
    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        spdlog::error( "   Can't alloc HW param struct [{}]", snd_strerror (err));
        return false;
    }

    if ((err = snd_pcm_hw_params_any (_adev, hw_params)) < 0) {
        spdlog::error( "   Can't init HW param struct [{}]", snd_strerror (err));
        return false;
    }

    // TODO: for capture we need NONINTERLEAVED
    if ((err = snd_pcm_hw_params_set_access ( _adev, hw_params, access )) < 0) {
        spdlog::error( "   Can't set access type [{}]", snd_strerror (err));
        return false;
    }
    if ((err = snd_pcm_hw_params_set_format (_adev, hw_params, format)) < 0) {
        spdlog::error( "   Can't set format [{}]", snd_strerror (err));
        return false;
    }
    // cout << "Format width=" << snd_pcm_format_width(format) << endl;

    if ((err = snd_pcm_hw_params_set_rate_near (_adev, hw_params, &_samplerate, 0)) < 0) {        
        spdlog::error( "   Can't set rate [{}]", snd_strerror (err));
        return false;
    }
    spdlog::debug("   Use samplerate of {} Hz on {}", _samplerate, _name);

    if ((err = snd_pcm_hw_params_set_channels_near (_adev, hw_params, &_channels)) < 0) {
        spdlog::error( "   Can't set channels [{}]", snd_strerror (err));
        return false;
    }
    spdlog::debug("   Use {} channels on {}", _channels, _name);
    // if (( err = snd_pcm_hw_params_set_periods(_adev, hw_params, periods, 0)) < 0) {
    //     spdlog::error( "Can't set periods [{}]", snd_strerror (err));
    //     return false;
    // }
    // above for respeaker, below for laptop and respeaker ?
    // TODO: validate hw params. Maybe we use period_time to define to fetch 2 periods 
    // per second indepenbent of samplerate ?
    snd_pcm_uframes_t frames = 32;
    if (( err = snd_pcm_hw_params_set_period_size_near(_adev, hw_params, &frames, 0)) < 0) {
        spdlog::error( "   Can't set periods [{}]", snd_strerror (err));
        return false;
    }
    spdlog::debug("   Use ALSA period size of {} frames on {}", frames, _name);
    // set buffer size in frames
    if ((err = snd_pcm_hw_params_set_buffer_size_near(_adev, hw_params, &_buffsize )) < 0) {
        spdlog::error( "   Can't set buffer size [{}]", snd_strerror (err));
        return false;
    }
    spdlog::debug("   Use ALSA buffer size of {} frames", _buffsize);

    if ((err = snd_pcm_hw_params (_adev, hw_params)) < 0) {
        spdlog::error( "   Can't set HW params  [{}]", snd_strerror (err));
        return false;
    }
    snd_pcm_hw_params_free (hw_params);
    if ((err = snd_pcm_prepare (_adev)) < 0) {
        spdlog::error( "   Unable to prepare ALSA dev {} [{}]", _name, snd_strerror (err));
        return false;
    }

    spdlog::debug( "   ALSA HW params configured for {}", _name );
    _isOpen = true;
    return true;
}

int Audio::capture( void *buf, int num ){
    int err;
    if( !open() ){
        spdlog::error("Failed to open ALSA device !!!");
        return -1;
    }
    if ((err = snd_pcm_readi( _adev, buf, num)) < 0){
        if( err == -EPIPE){
            spdlog::trace("Buffer overrun on capture. Retry ...");
            // drop all pending samples in buffer and advance pcm state to SETUP
            if( (err = snd_pcm_drop( _adev )) < 0 ){
                spdlog::error("Error on droping capture buffer [{}]", snd_strerror( err));
                return -1;
            }
            // we want to capture/play now, so prepare stream to state PREPARED
            if( (err=snd_pcm_prepare( _adev ) < 0) ){
                spdlog::error( "Error, on preparing after drop [{}]", snd_strerror (err));
                return -1;
            }
            // retry reading ....
            if ((err = snd_pcm_readi( _adev, buf, num)) < 0){
                spdlog::error( "Error, reading from ALSA after overrun[{}]{}", snd_strerror (err));
            }
        }
        else{
            spdlog::error( "Error, reading from ALSA [{}]{}", snd_strerror (err));
        }
    }
    spdlog::trace("Buffer [{} / {} B] captured", err, num);
    return err;
}

void Audio::pause(){
    int err;
    // drop all pending samples in buffer and advance pcm state to SETUP
    if( (err = snd_pcm_drop( _adev )) < 0 ){
        spdlog::error("unable to pause [{}]", snd_strerror( err));
    }
    // we want to capture/play soon, so prepare stream to state PREPARED
    if( (err=snd_pcm_prepare( _adev ) < 0) ){
        spdlog::error( "Error, on preparing after pasue[{}]", snd_strerror (err));
    }
}

int  Audio::play( void* buf, int num){
    // try open().If done before, it will just return
    if( !open() ){
        spdlog::error("Failed to open ALSA device !!!");
        return -1;
    }
    int err;
    if ((err = snd_pcm_writei( _adev, buf, num)) < 0){
        spdlog::error( "Unable to write to ALSA device [{}]", snd_strerror (err));
        snd_pcm_prepare( _adev );
    }
    spdlog::trace("Buffer [{} / {} B] played", err, num);
    return err;
}

int Audio::playFile( std::string fname){
    int err;
    if( _stream != SND_PCM_STREAM_PLAYBACK){
        spdlog::error("This is no playback device !");
        return -1;
    } 
    
    SNDFILE *f;
    SF_INFO info;
    if((f = sf_open( fname.c_str(), SFM_READ, &info)) == NULL ){
        spdlog::error("Unable to open {} for playback!", fname);
        return -1;
    } 

    spdlog::debug("Open {} for playback [rate={},ch={},frames={}]", fname, info.samplerate, info.channels, info.frames);
    
    _channels   = info.channels;
    _samplerate = info.samplerate;
    _buffsize   = 8192;

    if( !open() ){
        spdlog::error("Failed to open ALSA device !!!");
        return -1;
    }
    int n,m;
    int frame_num = 1024;
    short *buf = (short*)malloc(frame_num * sizeof(short)*2);

    while( (n = sf_readf_short( f, buf, frame_num )) != 0 ){
        if( n < 0){
            spdlog::error("Unabel to read from file [{}]", sf_strerror( f ));
        }
        // int m = play( buf, n );
        if ((m = snd_pcm_writei( _adev, (void*)buf, n)) < 0){
            spdlog::error( "Error, writing to ALSA [{}]", snd_strerror (err));
        }
        spdlog::trace("Read {} frames from file and played {} frames via ALSA.", n, m );
    }
    sf_close( f );
    close();
    return 0;
}

void Audio::close() {
   _isOpen = false;
  snd_pcm_close( _adev);
  spdlog::debug("ALSA device closed");
}


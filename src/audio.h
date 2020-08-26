#include <alsa/asoundlib.h>
#include <stdbool.h>
#include <string>

class Audio {
    
    std::string _name = "";
    std::string _dir = "";
    snd_pcm_t *_adev;
    snd_pcm_stream_t _stream;
    unsigned int _samplerate = 16000;
    unsigned int _channels = 1;
    bool _isOpen = false;

    
    // A sample if the number of bytes for one channel at a time (e.g 2byte for 16 bit resolution)
    // The sample rate is the nmber of samples per second (e.g. 480000 samples per sec for 48kHz )
    // A frame is one sample per channel (e.g. 4 byte for 16bit resolution and 2 channel)
    // The frame size is the number of bytes for one sample 16bit Stereo -> 4byte
    // The period is the number of frames per irq. Buffer is a ringbuffer and it
    // size must be greater than one period size
    
    snd_pcm_uframes_t _buffsize = 256; // size in frames -> is same as _srate to have 1s buff
    // snd_pcm_uframes_t _buffsize = 1024; // size in frames -> is same as _srate to have 1s buff


    public:
    enum Type{ PLAYBACK, CAPTURE};

    Audio( std::string adev, Audio::Type type, unsigned int srate=16000, int ch=1);
    bool open();
    int play(void *buf, int num);
    void pause();
    int playFile( std::string fname);
    int capture(void *buf, int num);
    void close();
};

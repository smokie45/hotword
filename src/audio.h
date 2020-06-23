#include <alsa/asoundlib.h>
#include <stdbool.h>
#include <string>

class Audio {
    
    std::string _name = "";
    snd_pcm_t *_adev;
    snd_pcm_stream_t _stream;
    unsigned int _samplerate = 16000;
    int _channels = 1;
    bool _isOpen = false;

    snd_pcm_uframes_t _buffsize = 256; // size in frames -> is same as _srate to have 1s buff
    
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

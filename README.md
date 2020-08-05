# hotword 
A small example to detect a hotword from continous audio stream
The decision was for ALSA to decrease overhaed. If this ends-up being to
low-level, we'll investigate on portaudio. GStreamer was avoided due to GObject
dependency.


    MIC -> HW buffer -> readi -> app buffer 

## Depends:
    spdlog - C++ debugging library
    alsa
    libsndfile
    porcupine: https://github.com/Picovoice/porcupine
    libfvad: https://github.com/dpirch/libfvad

# TODO:
    spdlog compiles slowly
    Unit test

# References
    https://www.alsa-project.org/alsa-doc/alsa-lib/index.html
    http://alsamodular.sourceforge.net/alsa_programming_howto.html
    https://gist.github.com/albanpeignier/104902
    https://alsa.opensrc.org/HowTo_Asynchronous_Playback
    https://gist.github.com/ghedo/963382/815c98d1ba0eda1b486eb9d80d9a91a81d995283

    arecord -c2 -v -f S16_LE -D "hw:0" /tmp/test.wav
    gst-launch-1.0 udpsrc port=12345 ! rawaudioparse use-sink-caps=false format=pcm pcm-format=s16le sample-rate=16000 num-channels=1 ! queue ! audioconvert ! audioresample ! filesink location=/tmp/udp.wav


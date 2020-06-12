#include "alsadev.h"
#include <alsa/asoundlib.h>
#include <iostream>
#include "spdlog/spdlog.h"
// Globals are generally a bad idea in code.  We're using one here to keep it
// simple.
#define FUNC_TRACE spdlog::trace("{}", __PRETTY_FUNCTION__);

snd_pcm_t *_adev;
using namespace std;
bool alsadev_open_rec(const char *name){
    int err;
    snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

    if ((err = snd_pcm_open (&_adev, name , SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        spdlog::error( "Can't open alsa device {} [{}]", name, snd_strerror (err));
        return false;
    }
    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        spdlog::error( "Can't alloc HW param struct [{}]", snd_strerror (err));
        return false;
    }

    if ((err = snd_pcm_hw_params_any (_adev, hw_params)) < 0) {
        spdlog::error( "Can't init HW param struct [{}]", snd_strerror (err));
        return false;
    }
    if ((err = snd_pcm_hw_params_set_access ( _adev, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        spdlog::error( "Can't set access type [{}]", snd_strerror (err));
        return false;
    }
return true;
}

bool alsadev_open_play(const char *name) {
  int i;
  int err;
  snd_pcm_hw_params_t *hw_params;

  if (name == NULL) {
    // Try to open the default device
    // err = snd_pcm_open(&_adev, "plughw:0,0", SND_PCM_STREAM_PLAYBACK, 0);
    err = snd_pcm_open(&_adev, "hw:0,0", SND_PCM_STREAM_PLAYBACK, 0);
  } else {
    // Open the device we were told to open.
    err = snd_pcm_open(&_adev, name, SND_PCM_STREAM_PLAYBACK, 0);
  }

  // Check for error on open.
  if (err < 0) {
    cout << "Init: cannot open audio device " << name << " ("
         << snd_strerror(err) << ")" << endl;
    return false;
  } else {
    cout << "Audio device opened successfully." << endl;
  }

  // Allocate the hardware parameter structure.
  if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
    cout << "Init: cannot allocate hardware parameter structure ("
         << snd_strerror(err) << ")" << endl;
    return false;
  }

  if ((err = snd_pcm_hw_params_any(_adev, hw_params)) < 0) {
    cout << "Init: cannot initialize hardware parameter structure ("
         << snd_strerror(err) << ")" << endl;
    return false;
  }

  // Enable resampling.
  unsigned int resample = 1;
  err = snd_pcm_hw_params_set_rate_resample(_adev, hw_params, resample);
  if (err < 0) {
    cout << "Init: Resampling setup failed for playback: " << snd_strerror(err)
         << endl;
    return err;
  }

  // Set access to RW interleaved.
  if ((err = snd_pcm_hw_params_set_access(_adev, hw_params,
                                          SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    cout << "Init: cannot set access type (" << snd_strerror(err) << ")"
         << endl;
    return false;
  }

  if ((err = snd_pcm_hw_params_set_format(_adev, hw_params,
                                          SND_PCM_FORMAT_S16_LE)) < 0) {
    cout << "Init: cannot set sample format (" << snd_strerror(err) << ")"
         << endl;
    return false;
  }

  // Set channels to stereo (2).
  if ((err = snd_pcm_hw_params_set_channels(_adev, hw_params, 2)) < 0) {
    cout << "Init: cannot set channel count (" << snd_strerror(err) << ")"
         << endl;
    return false;
  }

  // Set sample rate.
  unsigned int actualRate = 44100;
  if ((err = snd_pcm_hw_params_set_rate_near(_adev, hw_params,
                                             &actualRate, 0)) < 0) {
    cout << "Init: cannot set sample rate to 44100. (" << snd_strerror(err)
         << ")" << endl;
    return false;
  }
  if (actualRate < 44100) {
    cout << "Init: sample rate does not match requested rate. ("
         << "44100 requested, " << actualRate << " acquired)" << endl;
  }

  // Apply the hardware parameters that we've set.
  if ((err = snd_pcm_hw_params(_adev, hw_params)) < 0) {
    cout << "Init: cannot set parameters (" << snd_strerror(err) << ")" << endl;
    return false;
  } else {
    cout << "Audio device parameters have been set successfully." << endl;
  }

  // Get the buffer size.
  snd_pcm_uframes_t bufferSize;
  snd_pcm_hw_params_get_buffer_size(hw_params, &bufferSize);
  // If we were going to do more with our sound device we would want to store
  // the buffer size so we know how much data we will need to fill it with.
  cout << "Init: Buffer size = " << bufferSize << " frames." << endl;

  // Display the bit size of samples.
  cout << "Init: Significant bits for linear samples = "
       << snd_pcm_hw_params_get_sbits(hw_params) << endl;

  // Free the hardware parameters now that we're done with them.
  snd_pcm_hw_params_free(hw_params);

  // Prepare interface for use.
  if ((err = snd_pcm_prepare(_adev)) < 0) {
    cout << "Init: cannot prepare audio interface for use ("
         << snd_strerror(err) << ")" << endl;
    return false;
  } else {
    cout << "Audio device has been prepared for use." << endl;
  }

  return true;
}

bool alsadev_close() {
  snd_pcm_close(_adev);
  cout << "Audio device has been uninitialized." << endl;
  return true;
}


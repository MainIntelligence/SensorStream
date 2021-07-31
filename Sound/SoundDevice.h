#pragma once
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>

bool constexpr LittleEndian() {
	return __BYTE_ORDER == __LITTLE_ENDIAN;
};


template< unsigned int bytes = 2>
struct SoundSettings {
	snd_pcm_access_t mode = SND_PCM_ACCESS_RW_INTERLEAVED;
	snd_pcm_format_t format;
	unsigned int channels = 2; 
	unsigned int rate = 44100; //CD quality
	unsigned int framesinperiod = 32;
	SoundSettings();
};

template< unsigned int bytes >
SoundSettings<bytes>::SoundSettings() {
	if constexpr (LittleEndian()) {
		if constexpr (bytes == 2) { format = SND_PCM_FORMAT_S16_LE; }
	} else {
		if constexpr (bytes == 2) { format = SND_PCM_FORMAT_S16_BE; }
	}
}

struct SoundDevice {
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t frames;
	
	unsigned int periodbytes;
  	unsigned int periodtime;
  	unsigned int sampleBytes;
  	
  	long GetPeriodsFromMicroseconds(unsigned int microsecs) { return microsecs / periodtime; }
	unsigned int PeriodBytes() { return periodbytes; }
	unsigned int PeriodTime() { return periodtime; }
	void Drain() { snd_pcm_drain(handle); }
	void Close() { if (handle == nullptr) { return; }; snd_pcm_close(handle); handle = nullptr; }
	
	template <unsigned int bytes = 2>
	SoundDevice( bool record, const char * devicename = "default", SoundSettings<bytes> settings = SoundSettings<bytes>() );
	~SoundDevice();
	
	template <unsigned int bytes = 2>
	void SetProperties(SoundSettings<bytes> settings = SoundSettings<bytes>());
};

template <unsigned int bytes>
SoundDevice::SoundDevice(bool record, const char * devicename, SoundSettings<bytes> settings) : sampleBytes {bytes} {
  int rc;
  if (record) {
  	rc = snd_pcm_open(&handle, devicename, SND_PCM_STREAM_CAPTURE, 0);
  } else {
  	rc = snd_pcm_open(&handle, devicename, SND_PCM_STREAM_PLAYBACK, 0);
  }
  if (rc < 0) {
    fprintf(stderr,
            "unable to open pcm device: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_malloc(&params);
  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);
  
  SetProperties(settings);
  
  snd_pcm_hw_params_free (params);
  
  int err = snd_pcm_prepare (handle);
}

SoundDevice::~SoundDevice() {
  if (handle == nullptr) { return; }
  //snd_pcm_drain(handle);
  snd_pcm_close(handle); 
  handle = nullptr;
}

template <unsigned int bytes>
void SoundDevice::SetProperties(SoundSettings<bytes> settings) {
  int rc;
  snd_pcm_hw_params_set_access(handle, params,  settings.mode);

  snd_pcm_hw_params_set_format(handle, params, settings.format);
                              
  snd_pcm_hw_params_set_channels(handle, params, settings.channels);

  int dir;
  snd_pcm_hw_params_set_rate_near(handle, params, &settings.rate, &dir);

  frames = settings.framesinperiod;
  snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);

  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
    fprintf(stderr, "unable to set hw parameters: %s\n",
            snd_strerror(rc));
    return;
  }
  sampleBytes = bytes;
  
  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params, &frames, &dir);
  /* 2 bytes/sample, 2 channels */
  periodbytes = frames * settings.channels * sampleBytes;
  
  snd_pcm_hw_params_get_period_time(params, &periodtime, &dir);
  

}

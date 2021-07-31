#pragma once
#include "SoundDevice.h"

struct Speaker : public SoundDevice {
	template <typename ...Args>
	Speaker(Args... args) : SoundDevice(false, args...) {};
	~Speaker() = default;
	void Write(const void * buf);
};

void Speaker::Write(const void * buffer) {
  int rc = snd_pcm_writei(handle, buffer, frames);
  if (rc == -EPIPE) {
    /* EPIPE means underrun */
    fprintf(stderr, "underrun occurred\n");
    snd_pcm_prepare(handle);
  } else if (rc < 0) {
    fprintf(stderr, "error from writei: %s\n",
    snd_strerror(rc));
  }  else if (rc != (int)frames) {
    fprintf(stderr,"short write, write %d frames\n", rc);
  }
}

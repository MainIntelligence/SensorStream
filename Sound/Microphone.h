
#pragma once
#include "SoundDevice.h"

struct Microphone : public SoundDevice {
	template <typename ...Args>
	Microphone(Args... args) : SoundDevice(true, args...) {};
	~Microphone() = default;
	void Read(void * buf);
};

void Microphone::Read(void * buffer) {
    int rc = snd_pcm_readi(handle, buffer, frames);
    if (rc == -EPIPE) {
      fprintf(stderr, "overrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (rc < 0) {
      fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
    } else if (rc != (int)frames) {
      fprintf(stderr, "short read, read %d frames\n", rc);
    }
}



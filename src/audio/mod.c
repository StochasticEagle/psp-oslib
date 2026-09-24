#include "oslib.h"
#include <libxmp-lite/xmp.h>

typedef struct {
	xmp_context context;
	int frequency;
	int stereo;
	int shift;
	int playing;
} OSL_MOD_DATA;

static int osl_modFrequency = 44100;
static int osl_modStereo = 1;
static int osl_modShift = 0;

static OSL_MOD_DATA *oslGetModData(OSL_SOUND *s) {
	if (!s) {
		return NULL;
	}
	return (OSL_MOD_DATA *)s->data;
}

void oslAudioCallback_PlaySound_MOD(OSL_SOUND *s) {
	OSL_MOD_DATA *mod = oslGetModData(s);
	if (!mod) {
		return;
	}

	xmp_end_player(mod->context);
	mod->playing = (xmp_start_player(mod->context, mod->frequency, mod->stereo ? 0 : XMP_FORMAT_MONO) == 0);
	if (mod->playing) {
		xmp_play_buffer(mod->context, NULL, 0, 0);
	}
}

void oslAudioCallback_StopSound_MOD(OSL_SOUND *s) {
	OSL_MOD_DATA *mod = oslGetModData(s);
	if (!mod) {
		return;
	}

	xmp_end_player(mod->context);
	mod->playing = 0;
}

int oslAudioCallback_AudioCallback_MOD(unsigned int i, void *buf, unsigned int length) {
	OSL_SOUND *s = osl_audioVoices[i].sound;
	OSL_MOD_DATA *mod = oslGetModData(s);
	unsigned int scale;
	unsigned int sourceFrames;
	unsigned int channels;
	unsigned int sourceBytes;
	short *samples;
	int result;

	if (!mod || !mod->playing || length == 0) {
		if (buf && length > 0) {
			unsigned int bytesPerFrame = (s && s->mono == 0x10) ? 2 : 4;
			memset(buf, 0, length * bytesPerFrame);
		}
		return 0;
	}

	channels = mod->stereo ? 2 : 1;
	scale = 1U << mod->shift;
	sourceFrames = (length + scale - 1) / scale;
	sourceBytes = sourceFrames * channels * sizeof(short);

	result = xmp_play_buffer(mod->context, buf, (int)sourceBytes, s->endCallback == oslSoundLoopFunc ? 0 : 1);
	if (result < 0) {
		memset(buf, 0, length * channels * sizeof(short));
		mod->playing = 0;
		return 0;
	}

	if (mod->shift > 0) {
		samples = (short *)buf;
		for (int outputFrame = (int)length - 1; outputFrame >= 0; outputFrame--) {
			unsigned int sourceFrame = (unsigned int)outputFrame >> mod->shift;
			for (unsigned int channel = 0; channel < channels; channel++) {
				samples[(unsigned int)outputFrame * channels + channel] = samples[sourceFrame * channels + channel];
			}
		}
	}

	return 1;
}

VIRTUAL_FILE **oslAudioCallback_ReactiveSound_MOD(OSL_SOUND *s, VIRTUAL_FILE *f) {
	(void)s;
	(void)f;
	return NULL;
}

VIRTUAL_FILE *oslAudioCallback_StandBy_MOD(OSL_SOUND *s) {
	(void)s;
	return NULL;
}

void oslAudioCallback_DeleteSound_MOD(OSL_SOUND *s) {
	OSL_MOD_DATA *mod = oslGetModData(s);
	if (!mod) {
		return;
	}

	xmp_end_player(mod->context);
	xmp_release_module(mod->context);
	xmp_free_context(mod->context);
	free(mod);
	s->data = NULL;
}

void oslSetModSampleRate(int freq, int stereo, int shift) {
	if (freq < XMP_MIN_SRATE) {
		freq = XMP_MIN_SRATE;
	} else if (freq > XMP_MAX_SRATE) {
		freq = XMP_MAX_SRATE;
	}
	if (shift < 0) {
		shift = 0;
	} else if (shift > 30) {
		shift = 30;
	}

	osl_modFrequency = freq;
	osl_modStereo = stereo ? 1 : 0;
	osl_modShift = shift;
}

OSL_SOUND *oslLoadSoundFileMOD(const char *filename, int stream) {
	OSL_SOUND *s = NULL;
	OSL_MOD_DATA *mod = NULL;

	(void)stream;

	s = (OSL_SOUND *)malloc(sizeof(OSL_SOUND));
	mod = (OSL_MOD_DATA *)malloc(sizeof(OSL_MOD_DATA));
	if (!s || !mod) {
		goto error;
	}

	memset(s, 0, sizeof(OSL_SOUND));
	memset(mod, 0, sizeof(OSL_MOD_DATA));

	mod->context = xmp_create_context();
	if (!mod->context || xmp_load_module(mod->context, filename) < 0) {
		goto error;
	}

	mod->frequency = osl_modFrequency;
	mod->stereo = osl_modStereo;
	mod->shift = osl_modShift;

	s->data = mod;
	s->endCallback = NULL;
	s->volumeLeft = s->volumeRight = OSL_VOLUME_MAX;
	s->format = 0;
	s->mono = mod->stereo ? 0 : 0x10;
	s->divider = OSL_FMT_44K;
	s->isStreamed = 0;
	s->numSamples = 0;
	s->audioCallback = oslAudioCallback_AudioCallback_MOD;
	s->playSound = oslAudioCallback_PlaySound_MOD;
	s->stopSound = oslAudioCallback_StopSound_MOD;
	s->standBySound = oslAudioCallback_StandBy_MOD;
	s->reactiveSound = oslAudioCallback_ReactiveSound_MOD;
	s->deleteSound = oslAudioCallback_DeleteSound_MOD;
	return s;

error:
	if (mod) {
		if (mod->context) {
			xmp_release_module(mod->context);
			xmp_free_context(mod->context);
		}
		free(mod);
	}
	free(s);
	oslHandleLoadNoFailError(filename);
	return NULL;
}

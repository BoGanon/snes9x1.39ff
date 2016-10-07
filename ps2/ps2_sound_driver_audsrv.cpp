#include <ps2_sound_driver_audsrv.h>

#include <settings.h>

#include <snes9x.h>

static void audsrv_samples_available(void *data)
{
	((S9xAudsrvSoundDriver *)data)->samples_available();
}

S9xAudsrvSoundDriver::S9xAudsrvSoundDriver(void)
{

	format = NULL;

	return;

}

void S9xAudsrvSoundDriver::init(void)
{
	return;
}

void S9xAudsrvSoundDriver::terminate(void)
{

	stop ();

	//S9xSetSamplesAvailableCallback(NULL, NULL);

	if (format)
	{
		free(format);
	}

	return;

}

void S9xAudsrvSoundDriver::start(void)
{
	settings_t settings = settings_get();
	audsrv_set_volume(settings.sound.volume);
	return;
}

void S9xAudsrvSoundDriver::stop(void)
{

	return;
}

bool8 S9xAudsrvSoundDriver::open_device(void)
{

	settings_t settings = settings_get();
	format = (audsrv_fmt_t*)malloc(sizeof(audsrv_fmt_t));

	if (!format)
	{
		return FALSE;
	}

	format->channels = 2;//Settings.Stereo ? 2 : 1;
	format->bits = 16;
	format->freq = 44100;//Settings.SoundPlaybackRate;

	if (!audsrv_set_format(format))
	{
		printf ("OK\n");
	}
	else
	{
		return FALSE;
	}

	audsrv_set_volume(settings.sound.volume);

	return TRUE;

}

void S9xAudsrvSoundDriver::mix(void)
{
	return;
}

void S9xAudsrvSoundDriver::samples_available(void)
{
/*
	int samples, bytes;

	samples = so.samples_mixed_so_far;

	bytes = (samples << (Settings.SixteenBitSound ? 1 : 0));

	if (bytes <= 128)
		return;

	if (buffer_size < bytes || buffer == NULL)
	{
		delete[] buffer;
		buffer = new uint8[bytes];
		buffer_size = bytes;
	}

	//S9xMixSamples (buffer, samples);

	if (Settings.TurboMode)
		return;

	audsrv_wait_audio(bytes);
	audsrv_play_audio((const char*)buffer,bytes);

	return;
*/
}

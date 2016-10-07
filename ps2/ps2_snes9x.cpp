// C++ standard headers
#include <iostream>

// C Standard headers
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

// ps2sdk headers
#include <input.h>
#include <audsrv.h>
#include <fileXio_rpc.h>
#include <dma.h>
#include <packet.h>
#include <gs_psm.h>
#include <graph.h>
#include <draw.h>

// Ported headers
#include <zlib.h>
#include <libconfig.h>

// Interface headers
#include <browser.h>
#include <init.h>
#include <interface.h>
#include <paths.h>
#include <settings.h>

// Snes9x headers
#include <snes9x.h>
#include <display.h>
#include <memmap.h>
#include <apu.h>
#include <port.h>
#include <soundux.h>

#include <snes9x_cfg.h>
#include <ps2_controls.h>
#include <ps2_video.h>
int stuff = 0;
int stop_emulation = 0;
void make_s9x_dirs();

void *S9xProcessSound(void*);
void S9xDeinitSoundDevice(void);

extern void *_gp;

static u8 sound_thread_stack[0x40000] __attribute__ ((aligned(16)));
static int sound_thread_id = -1;

static u8 dispatcher_thread_stack[0x2000] __attribute__ ((aligned(16)));
static int dispatcher_thread_id = -1;

//static int main_thread_id = -1;

static int sound_sema = -1;
inline int sound_mutex_lock(void)
{
	return(WaitSema(sound_sema));
}
inline int sound_mutex_unlock(void)
{
	return(SignalSema(sound_sema));
}

inline int sound_mutex_trylock(void)
{
	ee_sema_t sema;

	ReferSemaStatus(sound_sema,&sema);

	if (sema.count)
	{
		return 0;
	}

	return -1;
}
//static int switch_sema = -1;

void dispatcher(void* arg)
{
	while(1)
	{
		SleepThread();
	}
}

void alarmfunction(s32 id, u16 time, void *arg)
{
	iWakeupThread(dispatcher_thread_id);
	iRotateThreadReadyQueue(30);
	iSetAlarm(625,alarmfunction,NULL);
}

void S9xInit()
{
	char *path;

	ps2_input_open_pads();

	path = check_boot("snes9x.cfg");

	S9xLoadCFG(path);

	Settings.Transparency = TRUE;
	Settings.SixteenBit = TRUE;
	Settings.Mode7Interpolate = TRUE;
	Settings.SupportHiRes = TRUE;
	Settings.BGLayering = TRUE;
	Settings.ForceSA1 = TRUE;
	Settings.SkipFrames = 0;

	Settings.ThreadSound = TRUE;
	Settings.APUEnabled = Settings.NextAPUEnabled = TRUE;
	Settings.SoundBufferSize = MAX_BUFFER_SIZE;
	Settings.SoundPlaybackRate = 1;
	Settings.SoundSkipMethod = 1;
	Settings.SoundSync = 1;
	Settings.DisableSoundEcho = FALSE;
	Settings.SixteenBitSound = TRUE;
	Settings.Stereo = TRUE;
	Settings.ReverseStereo = FALSE;
	Settings.SoundEnvelopeHeightReading = TRUE;
	Settings.DisableSampleCaching = FALSE;
	Settings.InterpolatedSound = TRUE;
	Settings.DisableMasterVolume = TRUE;
	Settings.AltSampleDecode = FALSE;
	Settings.FixFrequency = TRUE;

	// Might be able to wriggle some speedup from this
#ifdef USE_GLIDE
	Settings.GlideEnable = TRUE;
#endif

	S9xInitGFX();

#ifdef GFX_MULTI_FORMAT
	S9xSetRenderPixelFormat(BGR555);
#endif

	if (!S9xGraphicsInit())
	{
		printf("Out of Memory\n");
	}

}

#ifdef USE_THREADS
void S9xInitTimer()
{

	ee_sema_t sema;
	ee_thread_t thread;
	audsrv_set_volume(100);

	if (Settings.ThreadSound)
	{
		SetAlarm( 625, alarmfunction, NULL);
		ChangeThreadPriority(GetThreadId(), 29);

		sema.init_count = 1;
		sema.max_count = 1;
		sema.option = 0;
		sound_sema = CreateSema(&sema);
//		switch_sema = CreateSema(&sema);

//		main_thread_id = GetThreadId();

		thread.func = (void*)dispatcher;
		thread.stack = (void*)dispatcher_thread_stack;
		thread.stack_size = 0x2000;
		thread.gp_reg = &_gp;
		thread.initial_priority = 0;
		dispatcher_thread_id = CreateThread(&thread);
		StartThread (dispatcher_thread_id, NULL);

		thread.func = (void*)S9xProcessSound;
		thread.stack = (void*)sound_thread_stack;
		thread.stack_size = 0x40000;
		thread.gp_reg = &_gp;
		thread.initial_priority = 30;
		sound_thread_id = CreateThread(&thread);
		StartThread(sound_thread_id, NULL);
	}

}

void S9xDeinitTimer()
{
		if (Settings.ThreadSound)
	{
		TerminateThread(sound_thread_id);
		DeleteThread(sound_thread_id);
		DeleteSema(sound_sema);
//		DeleteSema(switch_sema);
	}

	sound_sema = -1;
	sound_thread_id = -1;

}
#endif

void S9xDeinit()
{
#ifdef USE_THREADS
	S9xDeinitTimer();
#endif

	browser_reset_path();

	stop_emulation = 0;

	S9xGraphicsDeinit();

	S9xDeinitGFX();

	audsrv_set_volume(0);
	audsrv_stop_audio();

	ps2_input_close_pads();

}

int main(int argc, char **argv)
{
	int loaded = 0;
	unsigned long current;
	unsigned long last = 0;
	char fps_str[20];
	int frames = 0;

	std::ios::sync_with_stdio(false);

	parse_args(argc,argv);

	init("snes9x.cfg");
	init_sound_modules(NULL);

	S9xInitSettings();

	make_s9x_dirs();

	if (!Memory.Init () || !S9xInitAPU ())
	{
		printf("Out of Memory\n");
	}

	S9xInitSound(Settings.SoundPlaybackRate,Settings.Stereo,Settings.SoundBufferSize);

	while (1)
	{

		interface_open();

		interface_run();

		interface_close();

		S9xInit();

		loaded = Memory.LoadROM(browser_get_path());

		if (loaded)
		{
			printf("Loading SRAM: %s\n", S9xGetFilename(".srm", SRAM_DIR));
			printf("loaded = %d\n",Memory.LoadSRAM(S9xGetFilename(".srm", SRAM_DIR)));
		}
		else
		{
			stop_emulation = 1;
			S9xReset();
		}

#ifdef USE_THREADS
		if (Settings.ThreadSound)
		{
			S9xInitTimer();
		}
#endif
		S9xSetSoundMute(TRUE);


		static float frame_time = 16.6667f;
		static unsigned long next_game_tick = clock() + (unsigned long)(576.0f * frame_time);

		ChangeThreadPriority(GetThreadId(),30);

		while (!stop_emulation)
		{

			if (Settings.DisplayFrameRate)
			{

				current = (unsigned int)(((float)clock()) / 576000.0f);

				if (current != last)
				{
					sprintf(fps_str,"fps = %d",frames);
					S9xSetInfoString (fps_str);
					//printf("fps = %d\n",frames);
					frames = 0;
					last = current;
				}

			}
#if 1
			S9xMainLoop();
			frames++;
#else
			if (clock() > next_game_tick)
			{
				S9xMainLoop();
				frames++;
				next_game_tick = clock() + (unsigned long)(576.0f * frame_time);
			}
#endif

//#ifdef USE_THREADS
//			if (Settings.ThreadSound)
//			{
				//ReleaseWaitThread(sound_thread_id);
				//WakeupThread(sound_thread_id);
//				SignalSema(switch_sema);
//				SleepThread();
//			}
//#endif

		}

		printf("Saving SRAM: %s\n",S9xGetFilename(".srm",SRAM_DIR));
		printf("saved = %d\n",Memory.SaveSRAM(S9xGetFilename(".srm", SRAM_DIR)));

		S9xDeinit();

	}

	return 0;
}

bool8 S9xOpenSnapshotFile (const char *filepath, bool8 read_only, STREAM *file)
{

	STREAM getfile = *file;

	if ((filepath == NULL) || (file == NULL))
	{
		return FALSE;
	}

	if (read_only)
	{
		getfile = OPEN_STREAM(filepath,"r");
	}
	else
	{
#ifdef ZLIB
		getfile = OPEN_STREAM(filepath,"w6h");
#else
		getfile = OPEN_STREAM(filepath,"w");
#endif
	}

	if ((getfile == Z_NULL) || (getfile == NULL))
	{
		return FALSE;
	}

	return TRUE;

}

void S9xCloseSnapshotFile (STREAM file)
{
	CLOSE_STREAM(file);
}

void S9xExit (void)
{
	stop_emulation = 1;
}

void S9xMessage (int type, int number, const char *message)
{
	printf("%s\n",message);
}

const char	dir_types[13][32] =
{
	"",				// DEFAULT_DIR
	"",				// HOME_DIR
	"",				// ROMFILENAME_DIR
	"rom",			// ROM_DIR
	"sram",			// SRAM_DIR
	"savestate",	// SNAPSHOT_DIR
	"screenshot",	// SCREENSHOT_DIR
	"spc",			// SPC_DIR
	"cheat",		// CHEAT_DIR
	"patch",		// IPS_DIR
	"bios",			// BIOS_DIR
	"log",			// LOG_DIR
	""				// LAST_DIR
};


void make_s9x_dirs(void)
{

	int i;

	for (i = 0; i < LAST_DIR; i++)
	{

		// DEFAULT_DIR is the same as these
		if (i == HOME_DIR)
		{
			continue;
		}
		else if (i == ROMFILENAME_DIR)
		{
			continue;
		}

		fileXioMkdir(S9xGetDirectory((s9x_getdirtype)i),0666);

	}

}

const char *S9xGetDirectory (enum s9x_getdirtype dirtype)
{
	static char directory[PATH_MAX+1];
	settings_t settings = settings_get();

	// Initialize directory with base directory
	strcpy(directory,settings.home.directory);

	// Make path for subdirectories
	if (dir_types[dirtype][0])
	{
		sprintf(directory,"%s/%s",settings.home.directory,dir_types[dirtype]);
	}

	printf("directory = %s\n", directory);

	return directory;

}

const char *S9xGetFilename (const char *extension, enum s9x_getdirtype dirtype)
{

	// directory
	static char filename[PATH_MAX+1];
	char directory[PATH_MAX+1];

	// components
	char drive[10];
	char dir[PATH_MAX+1];
	char fname[PATH_MAX+1];
	char ext[PATH_MAX+1];

	strcpy(directory,S9xGetDirectory(dirtype));
	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);

	snprintf(filename, PATH_MAX + 1, "%s%s%s%s", directory, SLASH_STR, fname, extension);

	return filename;

}

const char *S9xGetFilenameInc (const char *extension, enum s9x_getdirtype dirtype)
{

	static char filename[PATH_MAX+1];
	iox_stat_t stat;
	int i = 0;

	char directory[PATH_MAX+1];
	char drive[10];
	char dir[PATH_MAX+1];
	char fname[PATH_MAX+1];
	char ext[PATH_MAX+1];

	strcpy(directory,S9xGetDirectory(dirtype));
	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);

	do
	{
		snprintf(filename, PATH_MAX + 1, "%s%s%s.%03d%s", directory, SLASH_STR, fname, i++, extension);

		if (i > 100)
		{
			break;
		}
	}
	while (!fileXioGetStat(filename, &stat));

	return filename;

}

const char *S9xChooseFilename (bool8 read_only)
{
	return NULL;
}

const char *S9xBasename (const char *path)
{
	char base[256];

	strcpy(base,path);

	return basename(base);
}

void S9xAutoSaveSRAM (void)
{
	//Memory.SaveSRAM(S9xGetFilename(".srm", SRAM_DIR));
}

void S9xToggleSoundChannel (int c)
{
	static unsigned char sound_switch = 255;

	if (c == 8)
		sound_switch = 255;
	else
		sound_switch ^= 1 << c;

	S9xSetSoundControl(sound_switch);
}

void S9xSetPalette (void)
{
}

bool8_32 S9xOpenSoundDevice (int mode, bool8_32 stereo, int buffer_size)
{
	printf("mode = %d :: stereo = %d :: buffer_size = %d\n", mode, stereo,buffer_size);
	settings_t settings = settings_get();
	audsrv_fmt_t format;

	format.channels = 2;
	format.bits = 16;
	format.freq = 48000;

	so.mute_sound  = TRUE;

#ifndef OPTI
	so.sound_switch = 255;
	so.stereo       = TRUE;
	so.sixteen_bit  = TRUE;
#endif // OPTI
	so.buffer_size  = buffer_size;
	so.encoded      = FALSE;

	so.sound_fd = audsrv_set_format(&format);

	audsrv_set_volume(100);

	if ( so.sound_fd != 0 )
	{
		return FALSE;
	}
 
#ifdef OPTI
	so.buffer_size *= 2;
	so.buffer_size *= 2;
#else
	if ( so.stereo )
	{
		so.buffer_size *= 2;
	}
	if ( so.sixteen_bit )
	{
		so.buffer_size *= 2;
	}
#endif // OPTI
	if ( so.buffer_size > MAX_BUFFER_SIZE )
	{
		so.buffer_size = MAX_BUFFER_SIZE;
	}
 
	S9xSetPlaybackRate( 48000 );
 
	so.mute_sound  = FALSE;

	printf ("Rate: %d, Buffer size: %d, 16-bit: %s, Stereo: %s, Encoded: %s\n",
	        so.playback_rate, so.buffer_size, so.sixteen_bit ? "yes" : "no",
	        so.stereo ? "yes" : "no", so.encoded ? "yes" : "no");

	return TRUE;

}

#define FIXED_POINT 0x10000
#define FIXED_POINT_SHIFT 16
#define FIXED_POINT_REMAINDER 0xffff
void *S9xProcessSound (void *);
static volatile bool8 block_signal = FALSE;
static volatile bool8 pending_signal = FALSE;

static volatile bool8 block_generate_sound = FALSE;

static uint8 Buf[MAX_BUFFER_SIZE] __attribute__((aligned(16)));

void S9xGenerateSound ()
{

#ifdef OPTI
	int bytes_so_far = (so.samples_mixed_so_far << 1);
#else
	int bytes_so_far = so.sixteen_bit ? (so.samples_mixed_so_far << 1) :
	                   so.samples_mixed_so_far;
#endif

#ifndef _EE
#ifndef _ZAURUS
	if (Settings.SoundSync == 2)
	{
		// Assumes sound is signal driven
		while (so.samples_mixed_so_far >= so.buffer_size && !so.mute_sound)
		{
			pause ();
		}
	}
	else
#endif
#endif
		if (bytes_so_far >= so.buffer_size)
		{
			return;
		}

#ifdef USE_THREADS
	if (Settings.ThreadSound)
	{
		if (block_generate_sound || sound_mutex_trylock())
		{
			return;
		}
	}
#endif
	block_signal = TRUE;

	so.err_counter += so.err_rate;
	if (so.err_counter >= FIXED_POINT)
	{
		int sample_count = so.err_counter >> FIXED_POINT_SHIFT;
		int byte_offset;
		int byte_count;

		so.err_counter &= FIXED_POINT_REMAINDER;
#ifndef OPTI
		if (so.stereo)
#endif
		{
			sample_count <<= 1;
		}
		byte_offset = bytes_so_far + so.play_position;

		do
		{
			int sc = sample_count;
			byte_count = sample_count;
#ifndef OPTI
			if (so.sixteen_bit)
#endif
				byte_count <<= 1;

			if ((byte_offset & SOUND_BUFFER_SIZE_MASK) + byte_count > SOUND_BUFFER_SIZE)
			{
				sc = SOUND_BUFFER_SIZE - (byte_offset & SOUND_BUFFER_SIZE_MASK);
				byte_count = sc;
#ifndef OPTI
				if (so.sixteen_bit)
#endif
				{
					sc >>= 1;
				}
			}
			if (bytes_so_far + byte_count > so.buffer_size)
			{
				byte_count = so.buffer_size - bytes_so_far;
				if (byte_count == 0)
				{
					break;
				}
				sc = byte_count;
#ifndef OPTI
				if (so.sixteen_bit)
#endif
				{
					sc >>= 1;
				}
			}
			S9xMixSamplesO (Buf, sc, byte_offset & SOUND_BUFFER_SIZE_MASK);
			so.samples_mixed_so_far += sc;
			sample_count -= sc;
#ifdef OPTI
			bytes_so_far = (so.samples_mixed_so_far << 1);
#else
			bytes_so_far = so.sixteen_bit ? (so.samples_mixed_so_far << 1) :
	                       so.samples_mixed_so_far;
#endif
			byte_offset += byte_count;
		}
		while (sample_count > 0);
	}

	block_signal = FALSE;

#ifdef USE_THREADS
	if (Settings.ThreadSound)
	{
		sound_mutex_unlock();
	}
	else
#endif
		// This never evaluates TRUE
		if (pending_signal)
		{
			S9xProcessSound (NULL);
			pending_signal = FALSE;
		}
}

void *S9xProcessSound (void *)
{

#ifdef USE_THREADS
	do
	{
#endif
#ifdef USE_THREADS
		//SleepThread();
		//WaitSema(switch_sema);
#endif
		int sample_count = so.buffer_size;
		int byte_offset;

#ifndef OPTI
		if (so.sixteen_bit)
#endif
		{
			sample_count >>= 1;
		}

#ifdef USE_THREADS
		sound_mutex_lock();
#else
		//if (!Settings.ThreadSound)
		{
		//	if (block_signal)
			{
		//		pending_signal = TRUE;
		//		return (NULL);
			}
		}
		block_generate_sound = TRUE;
#endif

		/* If we need more audio samples */
		if (so.samples_mixed_so_far < sample_count)
		{
#ifdef OPTI
			byte_offset = so.play_position + (so.samples_mixed_so_far << 1);
#else
				byte_offset = so.play_position + (so.sixteen_bit ? (so.samples_mixed_so_far << 1) :
	                          so.samples_mixed_so_far);
#endif
#ifdef _EE
			S9xMixSamplesO (Buf, sample_count - so.samples_mixed_so_far,
							byte_offset & SOUND_BUFFER_SIZE_MASK);
#else
			if (Settings.SoundSync == 2)
			{
				memset (Buf + (byte_offset & SOUND_BUFFER_SIZE_MASK), 0, sample_count - so.samples_mixed_so_far);
			}
			else
			{
				S9xMixSamplesO (Buf, sample_count - so.samples_mixed_so_far, byte_offset & SOUND_BUFFER_SIZE_MASK);
			}
#endif
			so.samples_mixed_so_far = 0;
		}
		else
		{
			so.samples_mixed_so_far -= sample_count;
		}

#ifdef OPTI
		unsigned bytes_to_write = so.buffer_size;
#else
		unsigned bytes_to_write = sample_count;
		if (so.sixteen_bit)
		{
			bytes_to_write <<= 1;
		}
#endif

		byte_offset = so.play_position;
		so.play_position = (so.play_position + so.buffer_size) & SOUND_BUFFER_SIZE_MASK;

#ifdef USE_THREADS
		sound_mutex_unlock();
#endif
		block_generate_sound = FALSE;

		/* Feed the samples to the soundcard until nothing is left */
		for(;;)
		{

			int I = bytes_to_write;

			if (byte_offset + I > SOUND_BUFFER_SIZE)
			{
				I = SOUND_BUFFER_SIZE - byte_offset;
			}

			if(I == 0)
			{
				break;
			}

			audsrv_wait_audio(I);
			audsrv_play_audio((char*)Buf+byte_offset,I);

			if (I > 0)
			{
				bytes_to_write -= I;
				byte_offset += I;
				byte_offset &= SOUND_BUFFER_SIZE_MASK; /* wrap */
			}
		}
		/* All data sent. */
#ifdef USE_THREADS
		//ReleaseWaitThread(main_thread_id);
		//PollSema(switch_sema);
		//WakeupThread(main_thread_id);
	} while (Settings.ThreadSound);
#endif

	return (NULL);
}

static inline int __udelay(unsigned int usecs)
{
 
	register unsigned int loops_total = 0;
	register unsigned int loops_end   = usecs * 148;

	if (usecs > loops_end)
	{

		return -1;

	}

	asm volatile (".set noreorder\n\t"
				  "0:\n\t"
				  "beq %0,%2,0f\n\t"
				  "addiu %0,1\n\t"
				  "bne %0,%2,0b\n\t"
				  "addiu %0,1\n\t"
				  "0:\n\t"
				  ".set reorder\n\t"
				  :"=r" (loops_total)
				  :"0" (loops_total), "r" (loops_end));

	return 0;

}

#define CLOCKS_PER_MS 576.0f
void S9xSyncSpeed ()
{

#ifdef NETPLAY_SUPPORT
	if (Settings.NetPlay && NetPlay.Connected)
	{
#if defined(NP_DEBUG) && NP_DEBUG == 2
		printf("CLIENT: SyncSpeed @%d\n", S9xGetMilliTime());
#endif

		S9xNPSendJoypadUpdate(old_joypads[0]);
		for (int J = 0; J < 8; J++)
		{
			joypads[J] = S9xNPGetJoypad(J);
		}

		if (!S9xNPCheckForHeartBeat())
		{
			NetPlay.PendingWait4Sync = !S9xNPWaitForHeartBeatDelay(100);
#if defined(NP_DEBUG) && NP_DEBUG == 2
			if (NetPlay.PendingWait4Sync)
			{
				printf("CLIENT: PendingWait4Sync1 @%d\n", S9xGetMilliTime());
			}
#endif

			IPPU.RenderThisFrame = TRUE;
			IPPU.SkippedFrames = 0;
		}
		else
		{
			NetPlay.PendingWait4Sync = !S9xNPWaitForHeartBeatDelay(200);
#if defined(NP_DEBUG) && NP_DEBUG == 2
			if (NetPlay.PendingWait4Sync)
			{
				printf("CLIENT: PendingWait4Sync2 @%d\n", S9xGetMilliTime());
			}
#endif

			if (IPPU.SkippedFrames < NetPlay.MaxFrameSkip)
			{
				IPPU.RenderThisFrame = FALSE;
				IPPU.SkippedFrames++;
			}
			else
			{
				IPPU.RenderThisFrame = TRUE;
				IPPU.SkippedFrames = 0;
			}
		}

		if (!NetPlay.PendingWait4Sync)
		{
			NetPlay.FrameCount++;
			S9xNPStepJoypadHistory();
		}

		return;
	}
#endif
	if (!Settings.TurboMode && Settings.SkipFrames == AUTO_FRAMERATE)
	{
		static unsigned long next_ms = 0;
		unsigned long now_ms;

		now_ms = clock(); //(unsigned long)(((float)clock()) / CLOCKS_PER_MS);

		// If there is no known "next" frame, initialize it now.
		if (next_ms == 0)
		{
			next_ms = now_ms;
			next_ms++;
		}

		//if (timercmp(next,now,>))
		if (next_ms > now_ms)
		{
			if (IPPU.SkippedFrames == 0)
			{
				do
				{
					CHECK_SOUND ();
					now_ms = clock();
				}
				while (next_ms > now_ms);
			}
			IPPU.RenderThisFrame = TRUE;
			IPPU.SkippedFrames = 0;
		}
		else
		{
			if (IPPU.SkippedFrames < Settings.SkipFrames)
			{
				IPPU.SkippedFrames++;
				IPPU.RenderThisFrame = FALSE;
			}
			else
			{
				IPPU.RenderThisFrame = TRUE;
				IPPU.SkippedFrames = 0;
				next_ms = now_ms;
			}
		}

		next_ms += (unsigned long)(CLOCKS_PER_MS * ((float)Settings.FrameTime)/1000.0f);
	}
	else
	{
		IPPU.FrameSkip++;
		if (IPPU.FrameSkip >= (Settings.TurboMode ? Settings.TurboSkipFrames
		                         : Settings.SkipFrames))
		{
			IPPU.FrameSkip = 0;
			IPPU.SkippedFrames = 0;
			IPPU.RenderThisFrame = TRUE;
		}
		else
		{
			IPPU.SkippedFrames++;
			IPPU.RenderThisFrame = FALSE;
		}
	}
}
/*
void S9xSyncSpeed (void)
{

	unsigned int limit;
	int          lag;
	int          frametime;

#ifdef NETPLAY_SUPPORT
	if (S9xNetplaySyncSpeed())
		return;
#endif


	if (Settings.HighSpeedSeek > 0)
	{
		Settings.HighSpeedSeek--;
		IPPU.RenderThisFrame = FALSE;
		IPPU.SkippedFrames = 0;

		now_ms = (unsigned long)(((float)clock()) / 576.0f);
		now_sec = (unsigned int)(((float)now_ms) / 1000.0f);

		next_ms = now_ms;
		next_sec = now_sec;

		syncing = 0;

		return;
	}

	if (Settings.TurboMode)
	{
		if ((++IPPU.FrameSkip >= Settings.TurboSkipFrames) && !Settings.HighSpeedSeek)
		{
			IPPU.FrameSkip = 0;
			IPPU.SkippedFrames = 0;
			IPPU.RenderThisFrame = TRUE;
		}
		else
		{
			IPPU.SkippedFrames++;
			IPPU.RenderThisFrame = FALSE;
		}

		return;
	}

	now_ms = (unsigned long)(((float)clock()) / 576.0f);
	now_sec = (unsigned int)(((float)now_ms) / 1000.0f);

	// If there is no known "next" frame, initialize it now.
	if (next_sec == 0)
	{
		next_sec = now_sec;
		next_ms = now_ms+1;
	}

	if (Settings.SkipFrames == AUTO_FRAMERATE && !Settings.SoundSync)
	{
		lag = TIMER_DIFF(now,next);
		frametime = (int)(((float)Settings.FrameTime)/1000.0f);

		if (lag > frametime)
		{
			if (lag > (frametime * 10))
			{
				next_ms = now_ms;
				next_sec = now_sec;
				IPPU.RenderThisFrame = 1;
				IPPU.SkippedFrames = 0;
			}
			else
			{
				IPPU.RenderThisFrame = 0;
				IPPU.SkippedFrames++;
			}
		}
		else
		{
			IPPU.RenderThisFrame = 1;
			IPPU.SkippedFrames = 0;
		}
	}
	else
	{
		limit = Settings.SoundSync ? 1 : Settings.SkipFrames + 1;

		IPPU.SkippedFrames++;
		IPPU.RenderThisFrame = 0;

		if (IPPU.SkippedFrames >= limit)
		{
			IPPU.RenderThisFrame = 1;
			IPPU.SkippedFrames = 0;
		}
	}

	syncing = 1;

	return;

}
*/

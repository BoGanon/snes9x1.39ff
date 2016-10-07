#include <iostream>
#include <string.h>

#include <port.h>
#include <snes9x.h>

#include <cfg.h>
#include <libconfig.h>

#include <snes9x_cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUGGER
	static bool8 debug_settings[2];
#endif

#ifdef __cplusplus
};
#endif

void S9xInitSettings(void)
{
	ZeroMemory (&Settings, sizeof (Settings));

	Settings.JoystickEnabled = FALSE;	//unused
	Settings.SoundPlaybackRate = 4; //default would be '2', use 32000Hz as the genuine hardware does
	Settings.Stereo = TRUE;
	Settings.SoundBufferSize = 512; //256 //2048
	Settings.CyclesPercentage = 100;
	Settings.DisableSoundEcho = FALSE;
	Settings.APUEnabled = Settings.NextAPUEnabled = TRUE;
	Settings.H_Max = SNES_CYCLES_PER_SCANLINE;
	Settings.SkipFrames = 1;
	Settings.DisplayFrameRate = FALSE;
	Settings.ShutdownMaster = TRUE;
	Settings.FrameTimePAL = 20000;
	Settings.FrameTimeNTSC = 16667;
	Settings.FrameTime = Settings.FrameTimeNTSC;
	Settings.DisableSampleCaching = FALSE;
	Settings.DisableMasterVolume = FALSE;
	Settings.Mouse = FALSE;	//TRUE
	Settings.SuperScope = FALSE;
	Settings.MultiPlayer5 = FALSE;
	Settings.ControllerOption = SNES_MULTIPLAYER5;
	Settings.Transparency = TRUE;
	Settings.SixteenBit = TRUE;
	Settings.SupportHiRes = FALSE; //autodetected for known highres roms
	Settings.NetPlay = FALSE;
	Settings.ServerName [0] = 0;
	Settings.ThreadSound = TRUE;
	Settings.AutoSaveDelay = 0;
	Settings.ApplyCheats = TRUE;
	Settings.TurboMode = FALSE;
	Settings.TurboSkipFrames = 15;

	if (Settings.Transparency)
	{
		Settings.SixteenBit = TRUE;
	}

	Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;

#ifdef NETPLAY_SUPPORT
	Settings.ServerName[0] = 0;
#endif

}

void S9xParseCFG(config_t *config)
{

	bool8 autoframeskip = FALSE;
	char section_path[256];
	char setting[256];
	char controller[25];

	/// ROM
	sprintf(section_path,"%s%s",SNES9X_SECTION,".ROM.");

	sprintf(setting,"%s%s",section_path,"Interleaved");
	Settings.ForceInterleaved =            cfg_get_bool(setting, FALSE);

	if (Settings.ForceInterleaved)
	{
		Settings.ForceNotInterleaved = !Settings.ForceInterleaved;
	}

	sprintf(setting,"%s%s",section_path,"Interleaved2");
	Settings.ForceInterleaved2 =            cfg_get_bool(setting, FALSE);

	sprintf(setting,"%s%s",section_path,"Cheat");
	Settings.ApplyCheats =                 cfg_get_bool(setting, FALSE);

	sprintf(setting,"%s%s",section_path,"Patch");
	Settings.NoPatch =                    !cfg_get_bool(setting, FALSE);

	sprintf(setting,"%s%s",section_path,"LoROM");
	Settings.ForceLoROM =                  cfg_get_bool(setting, FALSE);

	sprintf(setting,"%s%s",section_path,"HiROM");
	Settings.ForceHiROM =                  cfg_get_bool(setting, FALSE);

	if (Settings.ForceLoROM)
	{
		Settings.ForceHiROM = FALSE;
	}

	sprintf(setting,"%s%s",section_path,"PAL");
	Settings.ForcePAL   =                  cfg_get_bool(setting, FALSE);

	sprintf(setting,"%s%s",section_path,"NTSC");
	Settings.ForceNTSC  =                  cfg_get_bool(setting, FALSE);

	if (Settings.ForcePAL)
	{
		Settings.ForceNTSC = FALSE;
	}

	sprintf(setting,"%s%s",section_path,"Header");
	Settings.ForceHeader =                 cfg_get_bool(setting, FALSE);

	sprintf(setting,"%s%s",section_path,"NoHeader");
	Settings.ForceNoHeader =               cfg_get_bool(setting, FALSE);

	if (Settings.ForceHeader)
	{
		Settings.ForceNoHeader = !Settings.ForceHeader;
	}

	/// Sound
	sprintf(section_path,"%s%s",SNES9X_SECTION,".Sound.");

	sprintf(setting,"%s%s",section_path,"Mute");
	Settings.Mute =                        cfg_get_bool(setting, FALSE);
	Settings.NextAPUEnabled = !Settings.Mute;

	sprintf(setting,"%s%s",section_path,"BufferSize");
	Settings.SoundBufferSize =   (unsigned int)cfg_string_to_int(cfg_get_string(setting, "512"));

	sprintf(setting,"%s%s",section_path,"Rate");
	Settings.SoundPlaybackRate = (unsigned int)cfg_string_to_int(cfg_get_string(setting, "44100"));

	sprintf(setting,"%s%s",section_path,"Skip");
	Settings.SoundSkipMethod =   (unsigned int)cfg_string_to_int(cfg_get_string(setting, "0"));

	sprintf(setting,"%s%s",section_path,"Echo");
	Settings.DisableSoundEcho =           !cfg_get_bool(setting, TRUE);

	sprintf(setting,"%s%s",section_path,"SixteenBitSound");
	Settings.SixteenBitSound =             cfg_get_bool(setting, TRUE);

	sprintf(setting,"%s%s",section_path,"Stereo");
	Settings.Stereo =                      cfg_get_bool(setting, TRUE);

	// Problem?
	if (Settings.Stereo)
	{
		Settings.APUEnabled = TRUE;
	}
	else
	{
		Settings.NextAPUEnabled = TRUE;
	}

	sprintf(setting,"%s%s",section_path,"ReverseStereo");
	Settings.ReverseStereo =               cfg_get_bool(setting, FALSE);

	sprintf(setting,"%s%s",section_path,"SoundEnvelopReading");
	Settings.SoundEnvelopeHeightReading =  cfg_get_bool(setting, FALSE);

	sprintf(setting,"%s%s",section_path,"SampleCaching");
	Settings.DisableSampleCaching =       !cfg_get_bool(setting, TRUE);

	sprintf(setting,"%s%s",section_path,"Interpolation");
	Settings.InterpolatedSound =           cfg_get_bool(setting, TRUE);

	sprintf(setting,"%s%s",section_path,"MasterVolume");
	Settings.DisableMasterVolume =        !cfg_get_bool(setting, TRUE);

	sprintf(setting,"%s%s",section_path,"Sync");
	Settings.SoundSync =         (unsigned int)cfg_string_to_int(cfg_get_string(setting, "1"));

	if (Settings.SoundSync)
	{
			Settings.SoundEnvelopeHeightReading = TRUE;
			Settings.InterpolatedSound = TRUE;
	}

	sprintf(setting,"%s%s",section_path,"AlternateDecode");
	Settings.AltSampleDecode =             cfg_get_bool(setting, TRUE);

	sprintf(setting,"%s%s",section_path,"FixFrequency");
	Settings.FixFrequency =                cfg_get_bool(setting, TRUE);

	/// Display
	sprintf(section_path,"%s%s",SNES9X_SECTION,".Display.");

	sprintf(setting,"%s%s",section_path,"HiRes");
	Settings.SupportHiRes =                cfg_get_bool(setting, TRUE);

	sprintf(setting,"%s%s",section_path,"SixteenBit");
	Settings.SixteenBit =                  cfg_get_bool(setting, TRUE);

	sprintf(setting,"%s%s",section_path,"Transparency");
	Settings.Transparency =                cfg_get_bool(setting, TRUE);

	sprintf(setting,"%s%s",section_path,"GraphicWindows");
	Settings.DisableGraphicWindows  =     !cfg_get_bool(setting, TRUE);

	sprintf(setting,"%s%s",section_path,"BGLayering");
	Settings.BGLayering  =                 cfg_get_bool(setting, TRUE);

	sprintf(setting,"%s%s",section_path,"Mode7Interpolate");
	Settings.Mode7Interpolate  =           cfg_get_bool(setting, TRUE);

	sprintf(setting,"%s%s",section_path,"DisplayFrameRate");
	Settings.DisplayFrameRate =            cfg_get_bool(setting, TRUE);

	/// Settings
	sprintf(section_path,"%s%s",SNES9X_SECTION,".Settings.");

	sprintf(setting,"%s%s",section_path,"BSXBootup");
	Settings.BS =                          cfg_get_bool(setting, FALSE);

	sprintf(setting,"%s%s",section_path,"SuperFX");
	Settings.ForceSuperFX =                cfg_get_bool(setting, FALSE);
	Settings.ForceNoSuperFX = !Settings.ForceSuperFX;

	sprintf(setting,"%s%s",section_path,"DSP1");
	Settings.ForceDSP1 =                   cfg_get_bool(setting, FALSE);
	Settings.ForceNoDSP1 = !Settings.ForceDSP1;

	sprintf(setting,"%s%s",section_path,"TurboMode");
	Settings.TurboMode =                   cfg_get_bool(setting, FALSE);

	sprintf(setting,"%s%s",section_path,"TurboFrameSkip");
	Settings.TurboSkipFrames =   (unsigned int)cfg_string_to_int(cfg_get_string(setting, "15"));

	sprintf(setting,"%s%s",section_path,"AutoSaveDelay");
	Settings.AutoSaveDelay =     (unsigned int)cfg_string_to_int(cfg_get_string(setting, "0"));

	sprintf(setting,"%s%s",section_path,"FrameTimePAL");
	Settings.FrameTimePAL =      (unsigned int)cfg_string_to_int(cfg_get_string(setting, "20000"));

	sprintf(setting,"%s%s",section_path,"FrameTimeNTSC");
	Settings.FrameTimeNTSC =     (unsigned int)cfg_string_to_int(cfg_get_string(setting, "16667"));

	sprintf(setting,"%s%s",section_path,"AutoFrameSkip");
	autoframeskip =                        cfg_get_bool(setting, FALSE);

	if (autoframeskip)
	{
		Settings.SkipFrames = AUTO_FRAMERATE;
	}
	else
	{
		sprintf(setting,"%s%s",section_path,"FrameSkip");
		Settings.SkipFrames =    (unsigned int)cfg_string_to_int(cfg_get_string(setting, "0"));// + 1;
	}

	/// Controls
	sprintf(section_path,"%s%s",SNES9X_SECTION,".Controls.");

	sprintf(setting,"%s%s",section_path,"MouseMaster");
	Settings.MouseMaster =                 cfg_get_bool(setting, TRUE);

	sprintf(setting,"%s%s",section_path,"SuperscopeMaster");
	Settings.SuperScopeMaster =            cfg_get_bool(setting, TRUE);

	sprintf(setting,"%s%s",section_path,"MP5Master");
	Settings.MultiPlayer5Master =          cfg_get_bool(setting, TRUE);

	sprintf(setting,"%s%s",section_path,"Controller");
	strcpy(controller,                     cfg_get_string(setting,"joypad"));

	if (!strcasecmp(controller,"joypad"))
	{
		Settings.MultiPlayer5 = FALSE;
		Settings.ControllerOption = SNES_JOYPAD;
	}
	else
	if (!strcasecmp(controller,"mp5"))
	{
		Settings.MultiPlayer5 = TRUE;
		Settings.ControllerOption = SNES_MULTIPLAYER5;
	}
	else
	if (!strcasecmp(controller,"superscope"))
	{
		Settings.SuperScope = TRUE;
		Settings.ControllerOption = SNES_SUPERSCOPE;
	}
	else
	if (!strcasecmp(controller,"mouse"))
	{
		Settings.ControllerOption = SNES_MOUSE_SWAPPED;
		Settings.Mouse = TRUE;
	}

	/// Hacks
	sprintf(section_path,"%s%s",SNES9X_SECTION,".Hacks.");

	sprintf(setting,"%s%s",section_path,"Cycles");
	Settings.CyclesPercentage =            cfg_string_to_int(cfg_get_string(setting, "100"));

	sprintf(setting,"%s%s",section_path,"SpeedHacks");
	Settings.ShutdownMaster =              cfg_get_bool(setting, FALSE);

	sprintf(setting,"%s%s",section_path,"DisableIRQ");
	Settings.DisableIRQ  =                 cfg_get_bool(setting, FALSE);

	sprintf(setting,"%s%s",section_path,"DisableHDMA");
	Settings.DisableHDMA =                 cfg_get_bool(setting, FALSE);

#ifdef NETPLAY_SUPPORT
	/// Netplay
	sprintf(section_path,"%s%s",SNES9X_SECTION,".Netplay.");

	sprintf(setting,"%s%s",section_path,"Enable");
	Settings.NetPlay =                     cfg_get_bool(setting, FALSE);

	sprintf(setting,"%s%s",section_path,"Port");
	Settings.Port =                       -cfg_string_to_int(cfg_get_string(setting,NP_DEFAULT_PORT));

	sprintf(setting,"%s%s",section_path,"Server");
	Settings.ServerName[0] = '\0';
	if (cfg_lookup(setting) != NULL)
		strncpy(Settings.ServerName,       cfg_get_string(setting, "Snes9x Server"), 128);
#endif

#ifdef DEBUGGER
	/// Debug
	sprintf(section_path,"%s%s",SNES9X_SECTION,".DEBUG.");

	sprintf(setting,"%s%s",section_path,"Debugger");
	debug_settings[0] =                    cfg_get_bool(setting, FALSE);

	if (debug_settings[0])
		CPU.Flags |= DEBUG_MODE_FLAG;

	sprintf(setting,"%s%s",section_path,"Trace");
	debug_settings[1] =                    cfg_get_bool(setting, FALSE);

	if (debug_settings[1])
	{
		if (!trace)
			trace = fopen("trace.log", "wb");
		CPU.Flags |= TRACE_FLAG;
	}
#endif

}

void S9xAddSettingsToCFG(config_t *config)
{

	char value[256];
	config_setting_t *root;
	config_setting_t *group;
	config_setting_t *subgroup;
	config_setting_t *setting;

	root = config_root_setting(config);

	group = config_setting_add(root,SNES9X_SECTION, CONFIG_TYPE_GROUP);
	{

		subgroup = config_setting_add(group,"ROM",CONFIG_TYPE_GROUP);
		{

			setting = config_setting_add(subgroup,"Interleaved",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.ForceInterleaved);

			setting = config_setting_add(subgroup,"Interleaved2",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.ForceInterleaved2);

			setting = config_setting_add(subgroup,"Cheat",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.ApplyCheats);

			setting = config_setting_add(subgroup,"Patch",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,!Settings.NoPatch);

			setting = config_setting_add(subgroup,"LoROM",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.ForceLoROM);

			setting = config_setting_add(subgroup,"HiROM",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.ForceHiROM);

			setting = config_setting_add(subgroup,"PAL",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.ForcePAL);

			setting = config_setting_add(subgroup,"NTSC",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.ForceNTSC);

			setting = config_setting_add(subgroup,"Header",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.ForceHeader);

			setting = config_setting_add(subgroup,"NoHeader",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.ForceNoHeader);

		}

		subgroup = config_setting_add(group,"Sound",CONFIG_TYPE_GROUP);
		{

			setting = config_setting_add(subgroup,"Mute",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.Mute);

			setting = config_setting_add(subgroup,"BufferSize",CONFIG_TYPE_STRING);
			cfg_int_to_string(value,(int)Settings.SoundBufferSize);
			config_setting_set_string(setting,value);

			setting = config_setting_add(subgroup,"Rate",CONFIG_TYPE_STRING);
			cfg_int_to_string(value,(int)Settings.SoundPlaybackRate);
			config_setting_set_string(setting,value);

			setting = config_setting_add(subgroup,"Skip",CONFIG_TYPE_STRING);
			cfg_int_to_string(value,(int)Settings.SoundSkipMethod);
			config_setting_set_string(setting,value);

			setting = config_setting_add(subgroup,"Sync",CONFIG_TYPE_STRING);
			cfg_int_to_string(value,(int)Settings.SoundSync);
			config_setting_set_string(setting,value);

			setting = config_setting_add(subgroup,"Echo",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,!Settings.DisableSoundEcho);

			setting = config_setting_add(subgroup,"SixteenBitSound",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.SixteenBitSound);

			setting = config_setting_add(subgroup,"Stereo",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.Stereo);

			setting = config_setting_add(subgroup,"ReverseStereo",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.ReverseStereo);

			setting = config_setting_add(subgroup,"SoundEnvelopeReading",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.SoundEnvelopeHeightReading);

			setting = config_setting_add(subgroup,"SampleCaching",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,!Settings.DisableSampleCaching);

			setting = config_setting_add(subgroup,"Interpolation",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.InterpolatedSound);

			setting = config_setting_add(subgroup,"MasterVolume",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,!Settings.DisableMasterVolume);

			setting = config_setting_add(subgroup,"AlternateDecode",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.AltSampleDecode);

			setting = config_setting_add(subgroup,"FixFrequency",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.FixFrequency);

		}

		subgroup = config_setting_add(group,"Display",CONFIG_TYPE_GROUP);
		{

			setting = config_setting_add(subgroup,"HiRes",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.SupportHiRes);

			setting = config_setting_add(subgroup,"SixteenBit",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.SixteenBit);

			setting = config_setting_add(subgroup,"Transparency",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.Transparency);

			setting = config_setting_add(subgroup,"GraphicWindows",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,!Settings.DisableGraphicWindows);

			setting = config_setting_add(subgroup,"BGLayering",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.BGLayering);

			setting = config_setting_add(subgroup,"Mode7Interpolate",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.Mode7Interpolate);

			setting = config_setting_add(subgroup,"DisplayFrameRate",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.DisplayFrameRate);

		}

		subgroup = config_setting_add(group,"Settings",CONFIG_TYPE_GROUP);
		{

			setting = config_setting_add(subgroup,"BSXBootup",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.BS);

			setting = config_setting_add(subgroup,"SuperFX",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.ForceSuperFX);

			setting = config_setting_add(subgroup,"DSP1",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.ForceDSP1);

			setting = config_setting_add(subgroup,"TurboMode",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.TurboMode);

			setting = config_setting_add(subgroup,"TurboFrameSkip",CONFIG_TYPE_STRING);
			cfg_int_to_string(value,(int)Settings.TurboSkipFrames);
			config_setting_set_string(setting,value);

			setting = config_setting_add(subgroup,"AutoSaveDelay",CONFIG_TYPE_STRING);
			cfg_int_to_string(value,(int)Settings.AutoSaveDelay);
			config_setting_set_string(setting,value);

			setting = config_setting_add(subgroup,"FrameTimePAL",CONFIG_TYPE_STRING);
			cfg_int_to_string(value,(int)Settings.FrameTimePAL);
			config_setting_set_string(setting,value);

			setting = config_setting_add(subgroup,"FrameTimeNTSC",CONFIG_TYPE_STRING);
			cfg_int_to_string(value,(int)Settings.FrameTimeNTSC);
			config_setting_set_string(setting,value);

			
			if (Settings.SkipFrames == AUTO_FRAMERATE)
			{
				setting = config_setting_add(subgroup,"AutoFrameSkip",CONFIG_TYPE_BOOL);
				config_setting_set_bool(setting,1);

				setting = config_setting_add(subgroup,"FrameSkip",CONFIG_TYPE_STRING);
				cfg_int_to_string(value,0);
				config_setting_set_string(setting,value);
			}
			else
			{
				setting = config_setting_add(subgroup,"AutoFrameSkip",CONFIG_TYPE_BOOL);
				config_setting_set_bool(setting,0);

				setting = config_setting_add(subgroup,"FrameSkip",CONFIG_TYPE_STRING);
				cfg_int_to_string(value,(int)Settings.SkipFrames);
				config_setting_set_string(setting,value);
			}

			setting = config_setting_add(subgroup,"FrameSkip",CONFIG_TYPE_STRING);
			cfg_int_to_string(value,(int)Settings.SkipFrames);
			config_setting_set_string(setting,value);
		}

		subgroup = config_setting_add(group,"Controls",CONFIG_TYPE_GROUP);
		{

			setting = config_setting_add(subgroup,"MouseMaster",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.MouseMaster);

			setting = config_setting_add(subgroup,"SuperscopeMaster",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.SuperScopeMaster);

			setting = config_setting_add(subgroup,"MP5Master",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.MultiPlayer5Master);

			setting = config_setting_add(subgroup,"Controller",CONFIG_TYPE_STRING);
			if (Settings.ControllerOption == SNES_JOYPAD)
			{
				config_setting_set_string(setting,"joypad");
			}
			else
			if (Settings.ControllerOption == SNES_MULTIPLAYER5)
			{
				config_setting_set_string(setting,"mp5");
			}
			else
			if (Settings.ControllerOption == SNES_SUPERSCOPE)
			{
				config_setting_set_string(setting,"superscope");
			}
			else
			if (Settings.ControllerOption == SNES_MOUSE_SWAPPED)
			{
				config_setting_set_string(setting,"mouse");
			}

		}

		subgroup = config_setting_add(group,"Hacks",CONFIG_TYPE_GROUP);
		{

			setting = config_setting_add(subgroup,"Cycles",CONFIG_TYPE_STRING);
			cfg_int_to_string(value,Settings.CyclesPercentage);
			config_setting_set_string(setting,value);

			setting = config_setting_add(subgroup,"SpeedHacks",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.ShutdownMaster);

			setting = config_setting_add(subgroup,"DisableIRQ",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.DisableIRQ);

			setting = config_setting_add(subgroup,"DisableHDMA",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.DisableHDMA);

		}

#ifdef NETPLAY_SUPPORT
		subgroup = config_setting_add(group,"Netplay",CONFIG_TYPE_GROUP);
		{
			setting = config_setting_add(subgroup,"Enable",CONFIG_TYPE_BOOL);
			config_setting_set_bool(setting,Settings.NetPlay);

			setting = config_setting_add(subgroup,"Port",CONFIG_TYPE_STRING);
			cfg_int_to_string(value,Settings.Port);
			config_setting_set_string(setting,value);

			setting = config_setting_add(subgroup,"Server",CONFIG_TYPE_STRING);
			config_setting_set_string(setting,Settings.ServerName);
		}
#endif

#ifdef DEBUGGER
		subgroup = config_setting_add(group,"DEBUG",CONFIG_TYPE_GROUP);
		{

			setting = config_setting_add(subgroup,"Debugger");
			config_setting_set_bool(setting,debug_settings[0]);

			setting = config_setting_add(subgroup,"Trace");
			config_setting_set_bool(setting,debug_settings[1]);

		}
#endif

	}

}

void S9xLoadCFG(char *path)
{

	config_t *cfg = cfg_open(path);

	S9xParseCFG(cfg);

	cfg_close(cfg);

}

void S9xSaveCFG(char *path)
{

	FILE *file;

	config_t *cfg = cfg_open(NULL);

	S9xAddSettingsToCFG(cfg);

	file = fopen(path,"w+");

	if (file == NULL)
	{
		return;
	}

	config_write(cfg,file);

	fclose(file);

	cfg_close(cfg);

}

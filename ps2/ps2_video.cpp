#include <port.h>

#include <gs_psm.h>
#include <graph.h>
#include <audsrv.h>

#include <settings.h>
#include <video.h>
#include <init.h>

#include <apu.h>
#include <snes9x.h>
#include <memmap.h>
#include <gfx.h>
#include <ppu.h>

int last_width = 0;
int last_height = 0;

extern void S9xDisplayFrameRate (uint8 *, uint32);
extern void S9xDisplayString (const char *string, uint8 *, uint32, int ypos);

void S9xInitGFX(void)
{

	video_packets_init();

	video_init_framebuffer(512,480);

	if (Settings.SupportHiRes)
	{
		video_init_texbuffer(512,480,GS_PSM_16S,0);
	}
	else
	{
		video_init_texbuffer(320,256,GS_PSM_16S,0);
	}

	PPU.CGDATA = (uint16*)memalign(128,2*256);

	GFX.Screen = (uint8*)memalign(128,512*480*2);
	GFX.SubScreen = (uint8*)memalign(128,512*480*2);
	GFX.ZBuffer = (uint8*)memalign(128,512*480*2);
	GFX.SubZBuffer = (uint8*)memalign(128,512*480*2);

	if (Settings.SupportHiRes)
	{
		GFX.Pitch = 512*2;
	}
	else
	{
		GFX.Pitch = 320*2;
	}

	//video_enable_vsync_handler();
}

void S9xDeinitGFX(void)
{

	last_width = 0;
	last_height = 0;

	video_packets_free();

	free(PPU.CGDATA);
	free(GFX.Screen);
	free(GFX.SubScreen);
	free(GFX.ZBuffer);
	free(GFX.SubZBuffer);

	//video_disable_vsync_handler();

}

bool8_32 S9xInitUpdate (void)
{
	return TRUE;
}

// texture buffer is 512x478
bool8_32 S9xDeinitUpdate (int width, int height)
{

	settings_t settings = settings_get();

	int mode = settings.display.mode;
	int interlace = 0;

	if (GFX.InfoString)
	{
		S9xDisplayString (GFX.InfoString, (uint8 *)GFX.Screen,GFX.Pitch,0);
	}
	//else if (Settings.DisplayFrameRate)
	{
		S9xDisplayFrameRate ((uint8 *)GFX.Screen, GFX.Pitch);
	}

	if ((width != last_width) || (height != last_height))
	{
		printf("width = %d\n",width);
		printf("height = %d\n",height);
		if (mode == GRAPH_MODE_HDTV_1080I)
		{
			interlace = 1;
		}

		if (height > 256)
		{
			if (mode == GRAPH_MODE_NTSC || mode == GRAPH_MODE_PAL)
			{
				interlace = 1;
			}
		}

		// 256x224
		video_init_screen(0,0,width,height,interlace,mode);
		video_init_draw_env(width,height);

		last_width = width;
		last_height = height;

		if (Settings.SupportHiRes)
		{
			video_send_packet(512,480,(void*)GFX.Screen,NULL);
		}
		else
		{
			//Glide
			//video_send_packet(256,256,(void*)GFX.Screen,NULL);
			video_send_packet(320,256,(void*)GFX.Screen,NULL);
		}

		video_draw_packet(256,height,GS_PSM_16S,0);
	}

	video_send_texture();
	video_draw_texture();

	//video_sync_flip();
	video_sync_wait();

	return 1;
}

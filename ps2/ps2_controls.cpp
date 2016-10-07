#include <libmtap.h>
#include <input.h>

#include <libconfig.h>
#include <cfg.h>

#include <ps2_controls.h>
#include <snes9x_cfg.h>

#include <snes9x.h>

static pad_t *pads[8];
static int pad_num = 0;

char multitap_ports[8] =
{
	0, 0, 0, 0,
	1, 1, 1, 1
};

char multitap_slots[8] = 
{
	0, 1, 2, 3,
	0, 1, 2, 3
};

static const char ps2_buttons[20][9] =
{
	"Select",
	"L3",
	"R3",
	"Start",
	"Up",
	"Right",
	"Down",
	"Left",
	"L2",
	"R2",
	"L1",
	"R1",
	"Triangle",
	"Circle",
	"Cross",
	"Square",
	"L_V",
	"L_H",
	"R_V",
	"R_H"
};

void ps2_input_open_pads()
{

	int i = 0;

	pad_t *pad;

	// hardcoded controllers
	for (i = 0; i < 8; i++)
	{

		pad = pad_open(multitap_ports[i],multitap_slots[i],MODE_DIGITAL,MODE_UNLOCKED);

		if (pad != NULL)
		{
			printf("Pad(%d,%d) initialized.\n",multitap_ports[i],multitap_slots[i]);

			pads[pad_num] = pad;
			pad_num++;
		}
	}

}

void ps2_input_close_pads()
{
	int i;

	for (i = 0; i < pad_num; i++)
	{
		pad_close(pads[i]);
	}

	pad_num = 0;

}

// handy-dandy macro to convert PS2 button masks to SNES button masks
#define PS2_TO_SNES(A)     (((A & PAD_R1)       >>  7) | \
                            ((A & PAD_L1)       >>  5) | \
                            ((A & PAD_TRIANGLE) >>  6) | \
                            ((A & PAD_CIRCLE)   >>  6) | \
                            ((A & PAD_RIGHT)    <<  3) | \
                            ((A & PAD_LEFT)     <<  2) | \
                            ((A & PAD_DOWN)     <<  4) | \
                            ((A & PAD_UP)       <<  7) | \
                            ((A & PAD_START)    <<  9) | \
                            ((A & PAD_SELECT)   << 13) | \
                            ((A & PAD_SQUARE)   >>  1) | \
                            ((A & PAD_CROSS)    <<  1))

extern "C" uint32 S9xReadJoypad(int which1)
{

	pad_t *pad;
	unsigned int snes_button = 0x80000000;

	if (which1 >= pad_num)
	{
		return 0;
	}

	pad = pads[which1];

	// Check if disconnected
	if (!pad->state)
	{
		return 0;
	}

	if (pad->buttons->btns == (PAD_R2 | PAD_L2 | PAD_START))
	{
		S9xExit();
	}

	pad_get_buttons(pad);
	pad->buttons->btns ^= 0xFFFF;

	snes_button += PS2_TO_SNES(pad->buttons->btns);

	//Analog
	if ((pad->buttons->mode >> 4) == 0x07)
	{
		if (pad->buttons->ljoy_h < 64)
		{
			snes_button |= SNES_LEFT_MASK;
		}
		else if (pad->buttons->ljoy_h > 192)
		{
			snes_button |= SNES_RIGHT_MASK;
		}
		if (pad->buttons->ljoy_v < 64)
		{
			snes_button |= SNES_UP_MASK;
		}
		else if (pad->buttons->ljoy_v > 192)
		{
			snes_button |= SNES_DOWN_MASK;
		}
	}

	return snes_button;

}

bool8 S9xReadMousePosition (int which1_0_to_1, int &x, int &y, uint32 &buttons)
{
	return FALSE;
}

bool8 S9xReadSuperScopePosition (int &x, int &y, uint32 &buttons)
{
	return FALSE;
}

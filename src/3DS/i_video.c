/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2006 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  DOOM graphics stuff for 3DS
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <3ds.h>

#include "m_argv.h"
#include "doomstat.h"
#include "doomdef.h"
#include "doomtype.h"
#include "v_video.h"
#include "r_draw.h"
#include "d_main.h"
#include "d_event.h"
#include "i_joy.h"
#include "i_video.h"
#include "z_zone.h"
#include "s_sound.h"
#include "sounds.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "lprintf.h"

extern void M_QuitDOOM(int choice);

int use_doublebuffer = 1; // Included not to break m_misc, but not relevant to libctru
int use_fullscreen;
int desired_fullscreen;

////////////////////////////////////////////////////////////////////////////
// Input code
int             leds_always_off = 0; // Expected by m_misc, not relevant

// Mouse handling
extern int     usemouse;        // config file var
static boolean mouse_enabled; // usemouse, but can be overriden by -nomouse
static boolean mouse_currently_grabbed;

/////////////////////////////////////////////////////////////////////////////////
// Keyboard handling

/////////////////////////////////////////////////////////////////////////////////
// Main input code

/* cph - pulled out common button code logic */

/* TODO for 3DS */
#if 0
static void I_GetEvent(SDL_Event *Event)
{
  event_t event;

  switch (Event->type) {
  case SDL_KEYDOWN:
    event.type = ev_keydown;
    event.data1 = I_TranslateKey(&Event->key.keysym);
    D_PostEvent(&event);
    break;

  case SDL_KEYUP:
  {
    event.type = ev_keyup;
    event.data1 = I_TranslateKey(&Event->key.keysym);
    D_PostEvent(&event);
  }
  break;

  case SDL_MOUSEBUTTONDOWN:
  case SDL_MOUSEBUTTONUP:
  if (mouse_enabled) // recognise clicks even if the pointer isn't grabbed
  {
    event.type = ev_mouse;
    event.data1 = I_SDLtoDoomMouseState(SDL_GetMouseState(NULL, NULL));
    event.data2 = event.data3 = 0;
    D_PostEvent(&event);
  }
  break;

  case SDL_MOUSEMOTION:
  if (mouse_currently_grabbed) {
    event.type = ev_mouse;
    event.data1 = I_SDLtoDoomMouseState(Event->motion.state);
    event.data2 = Event->motion.xrel << 5;
    event.data3 = -Event->motion.yrel << 5;
    D_PostEvent(&event);
  }
  break;


  case SDL_QUIT:
    S_StartSound(NULL, sfx_swtchn);
    M_QuitDOOM(0);

  default:
    break;
  }
}
#endif // 0

//
// I_StartTic
//

void I_StartTic (void)
{
  /* TODO for 3DS
  while ( SDL_PollEvent(&Event) )
    I_GetEvent(&Event);
  */
  I_PollJoystick();
}

//
// I_StartFrame
//
void I_StartFrame (void)
{
}

//
// I_InitInputs
//

static void I_InitInputs(void)
{
  mouse_enabled = 1;

  I_InitJoystick();
}
/////////////////////////////////////////////////////////////////////////////

// I_SkipFrame
//
// Returns true if it thinks we can afford to skip this frame

inline static boolean I_SkipFrame(void)
{
  static int frameno;

  frameno++;
  switch (gamestate) {
  case GS_LEVEL:
    if (!paused)
      return false;
  default:
    // Skip odd frames
    return (frameno & 1) ? true : false;
  }
}

///////////////////////////////////////////////////////////
// Palette stuff.
//
static void I_UploadNewPalette(int pal)
{

/* TODO for 3DS */
#if 0
  // This is used to replace the current 256 colour cmap with a new one
  // Used by 256 colour PseudoColor modes

  // Array of SDL_Color structs used for setting the 256-colour palette
  static SDL_Color* colours;
  static int cachedgamma;
  static size_t num_pals;

  if (V_GetMode() == VID_MODEGL)
    return;

  if ((colours == NULL) || (cachedgamma != usegamma)) {
    int pplump = W_GetNumForName("PLAYPAL");
    int gtlump = (W_CheckNumForName)("GAMMATBL",ns_prboom);
    register const byte * palette = W_CacheLumpNum(pplump);
    register const byte * const gtable = (const byte *)W_CacheLumpNum(gtlump) + 256*(cachedgamma = usegamma);
    register int i;

    num_pals = W_LumpLength(pplump) / (3*256);
    num_pals *= 256;

    if (!colours) {
      // First call - allocate and prepare colour array
      colours = malloc(sizeof(*colours)*num_pals);
    }

    // set the colormap entries
    for (i=0 ; (size_t)i<num_pals ; i++) {
      colours[i].r = gtable[palette[0]];
      colours[i].g = gtable[palette[1]];
      colours[i].b = gtable[palette[2]];
      palette += 3;
    }

    W_UnlockLumpNum(pplump);
    W_UnlockLumpNum(gtlump);
    num_pals/=256;
  }

#ifdef RANGECHECK
  if ((size_t)pal >= num_pals)
    I_Error("I_UploadNewPalette: Palette number out of range (%d>=%d)",
      pal, num_pals);
#endif

  // store the colors to the current display
  // SDL_SetColors(SDL_GetVideoSurface(), colours+256*pal, 0, 256);
  SDL_SetPalette(
      SDL_GetVideoSurface(),
      SDL_LOGPAL | SDL_PHYSPAL,
      colours+256*pal, 0, 256);
#endif
}

//////////////////////////////////////////////////////////////////////////////
// Graphics API

void I_ShutdownGraphics(void)
{
}

//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
}

//
// I_FinishUpdate
//
static int newpal = 0;
#define NO_PALETTE_CHANGE 1000

void I_FinishUpdate (void)
{
  if (I_SkipFrame()) return;

  /* TODO for 3DS
	...
	*/
	
	/* Update the display buffer (flipping video pages if supported)
	 * If we need to change palette, that implicitely does a flip */
	if (newpal != NO_PALETTE_CHANGE) {
		I_UploadNewPalette(newpal);
		newpal = NO_PALETTE_CHANGE;
	}
  
	gfxSwapBuffers();
	/* TODO for 3DS */
	gspWaitForVBlank();
}

//
// I_ScreenShot - moved to i_sshot.c
//

//
// I_SetPalette
//
void I_SetPalette (int pal)
{
  newpal = pal;
}

// I_PreInitGraphics

void I_PreInitGraphics(void)
{
  // currently done in main
  // gfxInitDefault();
  
  /* TODO for 3DS */
}

// CPhipps -
// I_CalculateRes
// Calculates the screen resolution, possibly using the supplied guide
void I_CalculateRes(unsigned int width, unsigned int height)
{
  SCREENWIDTH = (width+15) & ~15;
  SCREENHEIGHT = height;
  if (!(SCREENWIDTH % 1024)) {
    SCREENPITCH = SCREENWIDTH*V_GetPixelDepth()+32;
  } else {
    SCREENPITCH = SCREENWIDTH*V_GetPixelDepth();
  }
}

// CPhipps -
// I_SetRes
// Sets the screen resolution
void I_SetRes(void)
{
  int i;

  I_CalculateRes(SCREENWIDTH, SCREENHEIGHT);

  // set first three to standard values
  for (i=0; i<3; i++) {
    screens[i].width = SCREENWIDTH;
    screens[i].height = SCREENHEIGHT;
    screens[i].byte_pitch = SCREENPITCH;
    screens[i].short_pitch = SCREENPITCH / V_GetModePixelDepth(VID_MODE16);
    screens[i].int_pitch = SCREENPITCH / V_GetModePixelDepth(VID_MODE32);
  }

  // statusbar
  screens[4].width = SCREENWIDTH;
  screens[4].height = (ST_SCALED_HEIGHT+1);
  screens[4].byte_pitch = SCREENPITCH;
  screens[4].short_pitch = SCREENPITCH / V_GetModePixelDepth(VID_MODE16);
  screens[4].int_pitch = SCREENPITCH / V_GetModePixelDepth(VID_MODE32);

  lprintf(LO_INFO,"I_SetRes: Using resolution %dx%d\n", SCREENWIDTH, SCREENHEIGHT);
}

void I_InitGraphics(void)
{
  char titlebuffer[2048];
  static int    firsttime=1;

  if (firsttime)
  {
    firsttime = 0;

	/* TODO for 3DS: clean this up */
	SCREENWIDTH = 320;
	SCREENHEIGHT = 240;

    atexit(I_ShutdownGraphics);
    lprintf(LO_INFO, "I_InitGraphics: %dx%d\n", SCREENWIDTH, SCREENHEIGHT);

    /* Set the video mode */
    I_UpdateVideoMode();

    /* Initialize the input system */
    I_InitInputs();
  }
}

int I_GetModeFromString(const char *modestr)
{
  video_mode_t mode;

  if (!stricmp(modestr,"15")) {
    mode = VID_MODE15;
  } else if (!stricmp(modestr,"15bit")) {
    mode = VID_MODE15;
  } else if (!stricmp(modestr,"16")) {
    mode = VID_MODE16;
  } else if (!stricmp(modestr,"16bit")) {
    mode = VID_MODE16;
  } else if (!stricmp(modestr,"32")) {
    mode = VID_MODE32;
  } else if (!stricmp(modestr,"32bit")) {
    mode = VID_MODE32;
  } else if (!stricmp(modestr,"gl")) {
    mode = VID_MODEGL;
  } else {
    mode = VID_MODE8;
  }
  return mode;
}

void I_UpdateVideoMode(void)
{
  int init_flags;
  int i;
  video_mode_t mode;

  lprintf(LO_INFO, "I_UpdateVideoMode: %dx%d (%s)\n", SCREENWIDTH, SCREENHEIGHT, desired_fullscreen ? "fullscreen" : "nofullscreen");

  /* TODO for 3DS 
  mode = I_GetModeFromString(default_videomode);
  if ((i=M_CheckParm("-vidmode")) && i<myargc-1) {
    mode = I_GetModeFromString(myargv[i+1]);
  }
  */
  mode = VID_MODE15;
  //gfxSetScreenFormat(GFX_BOTTOM, GSP_RGB5_A1_OES);
  
  V_InitMode(mode);
  V_DestroyUnusedTrueColorPalettes();
  V_FreeScreens();

  I_SetRes();

  /* TODO for 3DS */
  // Get the info needed to render to the display
  screens[0].not_on_heap = false;
  
  V_AllocScreens();

  R_InitBuffer(SCREENWIDTH, SCREENHEIGHT);
}
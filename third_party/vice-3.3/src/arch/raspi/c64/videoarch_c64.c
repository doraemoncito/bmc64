/*
 * videoarch_c64.c
 *
 * Written by
 *  Randy Rossi <randy.rossi@gmail.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "videoarch_c64.h"

#include "c64/c64.h"
#include "c64/c64mem.h"

// This maps an ascii char to the charset's index in chargen rom
static const uint8_t char_to_screen[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
    0x3c, 0x3d, 0x3e, 0x3f, 0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53,
    0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x64,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x5f, 0x80, 0x81, 0x82, 0x83,
    0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b,
    0x9c, 0x9d, 0x9e, 0x9f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73,
    0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b,
    0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63,
    0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b,
    0x7c, 0x7d, 0x7e, 0x5e};

static unsigned int default_color_palette[] = {
    0x00, 0x00, 0x00, 0xFD, 0xFE, 0xFC, 0xBE, 0x1A, 0x24, 0x30, 0xE6, 0xC6,
    0xB4, 0x1A, 0xE2, 0x1F, 0xD2, 0x1E, 0x21, 0x1B, 0xAE, 0xDF, 0xF6, 0x0A,
    0xB8, 0x41, 0x04, 0x6A, 0x33, 0x04, 0xFE, 0x4A, 0x57, 0x42, 0x45, 0x40,
    0x70, 0x74, 0x6F, 0x59, 0xFE, 0x59, 0x5F, 0x53, 0xFE, 0xA4, 0xA7, 0xA2,
};

static unsigned int vice_color_palette[] = {
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x68, 0x37, 0x2b, 0x70, 0xa4, 0xb2,
    0x6f, 0x3d, 0x86, 0x58, 0x8d, 0x43, 0x35, 0x28, 0x79, 0xb8, 0xc7, 0x6f,
    0x6f, 0x4f, 0x25, 0x43, 0x39, 0x00, 0x9a, 0x67, 0x59, 0x44, 0x44, 0x44,
    0x6c, 0x6c, 0x6c, 0x9a, 0xd2, 0x84, 0x6c, 0x5e, 0xb5, 0x95, 0x95, 0x95,
};

static unsigned int c64hq_color_palette[] = {
    0x0A, 0x0A, 0x0A, 0xFF, 0xF8, 0xFF, 0x85, 0x1F, 0x02, 0x65, 0xCD, 0xA8,
    0xA7, 0x3B, 0x9F, 0x4D, 0xAB, 0x19, 0x1A, 0x0C, 0x92, 0xEB, 0xE3, 0x53,
    0xA9, 0x4B, 0x02, 0x44, 0x1E, 0x00, 0xD2, 0x80, 0x74, 0x46, 0x46, 0x46,
    0x8B, 0x8B, 0x8B, 0x8E, 0xF6, 0x8E, 0x4D, 0x91, 0xD1, 0xBA, 0xBA, 0xBA,
};

static unsigned int pepto_ntsc_color_palette[] = {
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x67, 0x37, 0x2B, 0x70, 0xA3, 0xB1,
    0x6F, 0x3D, 0x86, 0x58, 0x8C, 0x42, 0x34, 0x28, 0x79, 0xB7, 0xC6, 0x6E,
    0x6F, 0x4E, 0x25, 0x42, 0x38, 0x00, 0x99, 0x66, 0x59, 0x43, 0x43, 0x43,
    0x6B, 0x6B, 0x6B, 0x9A, 0xD1, 0x83, 0x6B, 0x5E, 0xB5, 0x95, 0x95, 0x95,
};

static unsigned int pepto_pal_color_palette[] = {
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x68, 0x37, 0x2b, 0x70, 0xa4, 0xb2,
    0x6f, 0x3d, 0x86, 0x58, 0x8d, 0x43, 0x35, 0x28, 0x79, 0xb8, 0xc7, 0x6f,
    0x6f, 0x4f, 0x25, 0x43, 0x39, 0x00, 0x9a, 0x67, 0x59, 0x44, 0x44, 0x44,
    0x6c, 0x6c, 0x6c, 0x9a, 0xd2, 0x84, 0x6c, 0x5e, 0xb5, 0x95, 0x95, 0x95,
};


void set_refresh_rate(int timing, struct video_canvas_s *canvas) {
  if (timing == MACHINE_TIMING_NTSC_HDMI ||
      timing == MACHINE_TIMING_NTSC_COMPOSITE ||
      timing == MACHINE_TIMING_NTSC_CUSTOM) {
    canvas->refreshrate = C64_NTSC_RFSH_PER_SEC;
  } else {
    canvas->refreshrate = C64_PAL_RFSH_PER_SEC;
  }
}

void set_video_font(void) {
  int i;
  video_font = mem_chargen_rom + 0x800;
  raw_video_font = mem_chargen_rom;
  for (i = 0; i < 256; ++i) {
    video_font_translate[i] = 8 * char_to_screen[i];
  }
}

unsigned int *raspi_get_palette(int index) {
  switch (index) {
  case 0:
    return default_color_palette;
    break;
  case 1:
    return vice_color_palette;
    break;
  case 2:
    return c64hq_color_palette;
    break;
  case 3:
    return pepto_ntsc_color_palette;
    break;
  case 4:
    return pepto_pal_color_palette;
    break;
  default:
    return NULL;
  }
}

/*
wa_httppost.h

Copyright (c) 2011, Philippe Proulx <eeppeliteloop@gmail.com>.
All rights reserved.

This plugin is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This plugin is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this plugin; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA 02110-1301  USA
*/
#ifndef _WA_HTTPPOST_H
#define _WA_HTTPPOST_H

#include <Windows.h>

#define WAP_GPPHDR_VER			0x10
#define WAP_PLUGIN_NAME			"eepp's Winamp HTTP POST"
#define WAP_INI_FILENAME		"wa_httppost.ini"
#define WAP_INI_FILENAME_W		L"wa_httppost.ini"

#define WAP_INI_KEY_POST_URL		"post_url"
#define WAP_INI_KEY_POST_KEY_FILENAME	"post_key_filename"
#define WAP_INI_KEY_POST_KEY_TITLE	"post_key_title"
#define WAP_INI_KEY_POST_KEY_BR		"post_key_br"
#define WAP_INI_KEY_POST_KEY_SR		"post_key_sr"
#define WAP_INI_KEY_POST_KEY_CH		"post_key_ch"
#define WAP_INI_KEY_POST_KEY_LEN_MS	"post_key_len_ms"

/* Winamp plugin structure */
struct winamp_gpp {
	int version;			/* version of the plugin structure */
	char* description;		/* name/title of the plugin */
	int (* init)();			/* init callback */
	void (* config)();		/* config callback */
	void (* quit)();		/* quit callback */
	HWND hwnd_parent;		/* HWND of the Winamp client main window (stored by Winamp when DLL is loaded) */
	HINSTANCE h_dll_instance;	/* HINSTANCE of this plugin DLL. (stored by Winamp when DLL is loaded) */
};

/* Song informations */
struct song_infos {
	char title [1024];	/* UTF-8 title */
	char filename [256];	/* UTF-8 filename */
	int br;			/* bit rate (kbps) - irrelevant if VBR */
	int sr;			/* sample rate (Hz) */
	int ch;			/* channels (1 or 2) */
	int len_ms;		/* length (ms) */
};

/* Plugin state */
struct plugin_state {
	BOOL config_loaded;		/* configuration was successfully loaded */
	char post_url [2000];		/* URL to use for posting data */
	char post_key_filename [64];	/* POST key for filename */
	char post_key_title [64];	/* POST key for title */
	char post_key_br [64];		/* POST key for bit rate */
	char post_key_sr [64];		/* POST key for sample rate */
	char post_key_ch [64];		/* POST key for channels */
	char post_key_len_ms [64];	/* POST key for length */
};

/**
 * Initialization callback for plugin.
 *
 * @return	0
 */
int init(void);

/**
 * Configuration callback for plugin.
 */
void config(void);

/**
 * Finalization callback for plugin.
 */
void fini(void);

#endif /* _WA_HTTPPOST_H */

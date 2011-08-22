/*
wa_httppost.c

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
#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <curl/curl.h>
#include <wa_ipc.h>

#include "wa_httppost.h"

/* last Winamp window procedure */
static WNDPROC g_old_winamp_wnd_proc = NULL;

/* Winamp plugin structure */
static struct winamp_gpp g_plugin = {
	WAP_GPPHDR_VER,
	WAP_PLUGIN_NAME,
	init,
	config,
	fini,
	0,
	0
};

/* main plugin state */
struct plugin_state g_plugin_state = {
	FALSE
};

/**
 * Fills a song infos structure with current playing song's infos.
 *
 * @param ansi_filename		Current filename (ANSI charset)
 * @param song_infos		Structure to be filled
 */
static void get_current_song_infos(char* ansi_filename, struct song_infos* song_infos) {
	wchar_t* utf16_title;
	wchar_t utf16_buf [512];

	/* convert ANSI filename to UTF-8 */
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, ansi_filename, -1, utf16_buf, 512);
	WideCharToMultiByte(CP_UTF8, 0, utf16_buf, -1, (LPSTR) song_infos->filename, 256, NULL, NULL);

	/* get current title */
	utf16_title = (wchar_t*) SendMessage(g_plugin.hwnd_parent, WM_WA_IPC, 0, IPC_GET_PLAYING_TITLE);
	WideCharToMultiByte(CP_UTF8, 0, utf16_title, -1, (LPSTR) song_infos->title, 1024, NULL, NULL);

	/* get audio informations */
	song_infos->br = (int) SendMessage(g_plugin.hwnd_parent, WM_WA_IPC, 1, IPC_GETINFO);
	song_infos->sr = (int) SendMessage(g_plugin.hwnd_parent, WM_WA_IPC, 5, IPC_GETINFO);
	song_infos->ch = (int) SendMessage(g_plugin.hwnd_parent, WM_WA_IPC, 2, IPC_GETINFO);
	song_infos->len_ms = (int) SendMessage(g_plugin.hwnd_parent, WM_WA_IPC, 2, IPC_GETOUTPUTTIME);
}

/**
 * Adds an integer to cURL POST data.
 *
 * @param i		Integer value
 * @param name		POST name
 * @param form_post	cURL form POST
 * @param last_pos	cURL form POST last pointer
 */
static void curl_formadd_int(int i, char* name,
struct curl_httppost** form_post, struct curl_httppost** last_post) {
	char buf [32];

	sprintf_s(buf, 32, "%d", i);
	curl_formadd(form_post, last_post,
		CURLFORM_COPYNAME, name,
		CURLFORM_COPYCONTENTS, buf,
		CURLFORM_END);
}

/**
 * Transfers song infos to cURL POST data.
 *
 * @param song_infos	Song infos
 * @param form_post	cURL form POST
 * @param last_pos	cURL form POST last pointer
 */
static void song_infos_to_curl_form(struct song_infos* song_infos,
struct curl_httppost** form_post, struct curl_httppost** last_post) {
	/* filename */
	curl_formadd(form_post, last_post,
		CURLFORM_COPYNAME, "filename",
		CURLFORM_COPYCONTENTS, song_infos->filename,
		CURLFORM_END);

	/* title */
	curl_formadd(form_post, last_post,
		CURLFORM_COPYNAME, "title",
		CURLFORM_COPYCONTENTS, song_infos->title,
		CURLFORM_END);

	/* numeric infos */
	curl_formadd_int(song_infos->br, "br", form_post, last_post);
	curl_formadd_int(song_infos->sr, "sr", form_post, last_post);
	curl_formadd_int(song_infos->ch, "ch", form_post, last_post);
	curl_formadd_int(song_infos->len_ms, "len_ms", form_post, last_post);
}

/**
 * Function to be called when a new file is being played.
 *
 * @param ansi_filename		Filename (ANSI charset)
 */
static void playing_file_cb(char* ansi_filename) {
	CURL* curl;
	CURLcode res;
	struct curl_httppost* form_post = NULL;
	struct curl_httppost* last_post = NULL;
	struct song_infos song_infos;

	/* fill song infos structure */
	get_current_song_infos(ansi_filename, &song_infos);

	/* transfer song infos to POST data */
	song_infos_to_curl_form(&song_infos, &form_post, &last_post);

	/* cURL calls */
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, g_plugin_state.post_url);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, form_post);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		curl_formfree(form_post);
	}
}

/**
 * Custom replacement window procedure for Winamp.
 */
LRESULT CALLBACK custom_wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
	switch (msg) {
		case WM_WA_IPC:
		switch (l_param) {
			case IPC_PLAYING_FILE:
			playing_file_cb((char*) w_param);
			break;
		}
		break;
	}

	/* call old Winamp window procedure so everything is okay */
 	return CallWindowProc(g_old_winamp_wnd_proc, hwnd, msg, w_param, l_param);
}

/**
 * Gets an INI file value.
 *
 * @param fh	INI file handler
 * @param key	INI key
 * @param val	Value
 * @return	TRUE if value found, else FALSE
 */
static BOOL get_ini_str(FILE* fh, const char* key, char* val) {
	char buf [64];
	BOOL found = FALSE;
	char c;
	unsigned int i = 0;

	sprintf_s(buf, 64, "%s = %%s", key);
	rewind(fh);
	while (!feof(fh)) {
		if (fscanf_s(fh, buf, val) == 1) {
			found = TRUE;
			break;
		}
		while (!feof(fh)) {
			c = (char) fgetc(fh);
			if (c == 0x0a) {
				break;
			}
		}
	}

	return found;
}

/**
 * Loads main configuration file, WAP_INI_FILENAME.
 */
static BOOL load_config(void) {
	BOOL ret;
	FILE* fh;

	fopen_s(&fh, WAP_INI_FILENAME, "rb");
	if (!fh) {
		MessageBox(NULL,
			L"eepp's Winamp HTTP POST: cannot open configuration file...",
			L"error", MB_OK);
	}
	if (!get_ini_str(fh, WAP_INI_KEY_POST_URL, g_plugin_state.post_url)) {
		ret = FALSE;
	}
	if (!get_ini_str(fh, WAP_INI_KEY_POST_KEY_FILENAME, g_plugin_state.post_key_filename)) {
		ret = FALSE;
	}
	if (!get_ini_str(fh, WAP_INI_KEY_POST_KEY_TITLE, g_plugin_state.post_key_title)) {
		ret = FALSE;
	}
	if (!get_ini_str(fh, WAP_INI_KEY_POST_KEY_BR, g_plugin_state.post_key_br)) {
		ret = FALSE;
	}
	if (!get_ini_str(fh, WAP_INI_KEY_POST_KEY_SR, g_plugin_state.post_key_sr)) {
		ret = FALSE;
	}
	if (!get_ini_str(fh, WAP_INI_KEY_POST_KEY_CH, g_plugin_state.post_key_ch)) {
		ret = FALSE;
	}
	if (!get_ini_str(fh, WAP_INI_KEY_POST_KEY_LEN_MS, g_plugin_state.post_key_len_ms)) {
		ret = FALSE;
	}
	fclose(fh);

	return ret;
}

int init(void) {
	g_plugin_state.config_loaded = load_config();
	if (g_plugin_state.config_loaded) {
		curl_global_init(CURL_GLOBAL_ALL);
		g_old_winamp_wnd_proc = (WNDPROC) SetWindowLong(g_plugin.hwnd_parent, GWL_WNDPROC,
			(LONG) custom_wnd_proc);
	} else {
		MessageBox(NULL,
			L"eepp's Winamp HTTP POST: error while trying to read configuration file",
			L"error", MB_OK);
	}

	return 0;
}

void config(void) {
	MessageBox(NULL,
		L"opening configuration file...\nWinamp needs to be restarted after editing this file",
		L"info", MB_OK);
	ShellExecute(g_plugin.hwnd_parent, L"open", WAP_INI_FILENAME_W, NULL, NULL, SW_SHOW);
}
 
void fini() {
	if (g_plugin_state.config_loaded) {
		curl_global_cleanup();
	}
}

struct winamp_gpp* winampGetGeneralPurposePlugin() {
	return &g_plugin;
}

eepp's Winamp HTTP POST v1.0
¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
Author: Philippe Proulx <eeppeliteloop@gmail.com>
Website: <http://code.google.com/p/eepp-wa-plugins/>


1 INTRODUCTION

eepp's Winamp HTTP POST v1.0 is a Winamp 5.x plugin which HTTP-POSTs
the current playing song informations to a custom URL. This data can
then be used for logging, display, statistics, etc.

Each time a song starts (in any way), the new data is sent.

All sent strings are UTF-8 encoded.


2 SETUP

Here's a quick install guide:

	1. put "dependencies\*" beside "winamp.exe"
	2. put "config\*" beside "winamp.exe"
	3. put "gen_wa_httppost.dll" into Winamp's "Plugins" directory


3 CONFIGURATION

Configuration is done by editing wa_httppost.ini. This file should
be edited using something better than Windows' Notepad because its
EOL are UNIX-style (LF only). The INI keys are:

	* post_url: URL where to HTTP-POST data
	* post_key_filename: POST key for complete filename (UTF-8)
	* post_key_title: POST key for display title (UTF-8)
	* post_key_br: POST key for bit rate (kbps)
	* post_key_sr: POST key for sample rate (Hz)
	* post_key_ch: POST key for channel mode (1 or 2)
	* post_key_len_ms: POST key for song total length (ms)


4 KNOWN BUGS

Here's a list of known bugs as of v1.0:

	* sometimes, Winamp HTTP POST will not be able to read numeric
	  values for the first played song after Winamp startup and
	  probably report 0 for each of them (filename and title should
	  be valid, though)

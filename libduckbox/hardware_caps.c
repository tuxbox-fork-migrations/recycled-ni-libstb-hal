/*
 * determine the capabilities of the hardware.
 * part of libstb-hal
 *
 * (C) 2010-2012 Stefan Seyfried
 *
 * License: GPL v2 or later
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <hardware_caps.h>

static int initialized = 0;
static hw_caps_t caps;

hw_caps_t *get_hwcaps(void)
{
	if (initialized)
		return &caps;

	memset(&caps, 0, sizeof(hw_caps_t));

	initialized = 1;
	char buf[64];
	int len = -1;
	int fd = open("/proc/stb/info/model", O_RDONLY);
	if (fd != -1) {
		len = read(fd, buf, sizeof(buf) - 1);
		close(fd);
	}
	if (len > 0) {
		buf[len] = 0;
		if (!strncmp(buf, "atevio7500", 10)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, buf);
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 1;
			caps.has_fan = 0;
			caps.has_CI = 2;
		}
		else if (!strncmp(buf, "ufs912", 6)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, buf);
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 1;
			caps.has_fan = 0;
			caps.has_CI = 2;
		}
		else if (!strncmp(buf, "ufs913", 6)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, buf);
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 1;
			caps.has_fan = 0;
			caps.has_CI = 2;
		}
		else if (!strncmp(buf, "ufs922", 6)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, buf);
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 1;
			caps.has_CI = 2;
		}
		else if (!strncmp(buf, "ufs910", 6)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, buf);
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 0;
			caps.has_CI = 2;
		}
		else if (!strncmp(buf, "hdbox", 5)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, buf);
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 0;
			caps.has_CI = 2;
		}
		else if (!strncmp(buf, "octagon1008", 11)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, buf);
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 0;
			caps.has_CI = 2;
		}
		else if (!strncmp(buf, "hs7110", 6)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, buf);
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 1;
			caps.has_fan = 0;
		}
		else if (!strncmp(buf, "hs7810a", 7)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, buf);
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 1;
			caps.has_fan = 0;
		}
		else if (!strncmp(buf, "hs7119", 6)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, buf);
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 1;
			caps.has_fan = 0;
		}
		else if (!strncmp(buf, "hs7819", 6)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, buf);
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 1;
			caps.has_fan = 0;
		}
		else if (!strncmp(buf, "dp7000", 6)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, buf);
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 1;
			caps.has_fan = 0;
		}
		else if (!strncmp(buf, "cuberevo", 8)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, buf);
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 0;
			caps.has_CI = 2;
		}
		else if (!strncmp(buf, "cuberevo-mini2", 14)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, buf);
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 0;
			caps.has_CI = 2;
		}
		else if (!strncmp(buf, "cuberevo-2000hd", 15)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, buf);
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 0;
			caps.has_CI = 0;
		}
		else if (!strncmp(buf, "tf7700", 6)) {
			strcpy(caps.boxvendor, "DUCKBOX");
			strcpy(caps.boxname, buf);
			caps.can_shutdown = 1;
			caps.has_HDMI = 1;
			caps.has_SCART = 1;
			caps.can_cec = 0;
			caps.has_fan = 0;
			caps.has_CI = 2;
		}
		else {
			strcpy(caps.boxvendor, "unknown");
			strcpy(caps.boxname, "unknown model");
		}
	}
	else
		strcpy(caps.boxname, "(unknown model)");

	return &caps;
}

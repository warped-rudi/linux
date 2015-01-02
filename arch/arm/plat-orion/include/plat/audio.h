#ifndef __PLAT_AUDIO_H
#define __PLAT_AUDIO_H

struct kirkwood_asoc_platform_data {
	int burst;
	int use_i2s;
	int use_spdif;
};
#endif

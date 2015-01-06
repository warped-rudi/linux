/*
 * sound/soc/codecs/tda19988.c
 *
 * Author:      Ajay Bhargav,  <contact@8051projects.net>
 *		Rickey's World <www.8051projects.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>

#include "tda19988.h"

#define DRV_NAME "tda19988-codec"

#define TDA19988_RATES		(SNDRV_PCM_RATE_32000 |	\
				 SNDRV_PCM_RATE_44100 | \
				 SNDRV_PCM_RATE_48000 | \
				 SNDRV_PCM_RATE_88200 | \
				 SNDRV_PCM_RATE_96000 | \
				 SNDRV_PCM_RATE_176400 | \
				 SNDRV_PCM_RATE_192000)

#define TDA19988_FORMATS	(SNDRV_PCM_FMTBIT_S16_LE | \
				 SNDRV_PCM_FMTBIT_S32_LE)

static struct snd_soc_codec_driver soc_codec_tda19988;

int __attribute__((weak)) tda_config_audioin(struct tda_audio_config *cfg)
{
	return 0;
}

static int tda19988_hw_params(struct snd_pcm_substream *substream,
			   struct snd_pcm_hw_params *params,
			   struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;
	struct tda_audio_config audio;

	dev_dbg(codec->dev, "Entered %s\n", __func__);

	audio.format = TMDL_HDMITX_AFMT_I2S;
	audio.i2sFormat = TMDL_HDMITX_I2SFOR_PHILIPS_L;
	audio.dstRate = TMDL_HDMITX_DSTRATE_SINGLE;
	audio.channelAllocation = 0;

	switch (params_rate(params)) {
	case 32000:
		audio.rate = TMDL_HDMITX_AFS_32K;
		break;
	case 44100:
		audio.rate = TMDL_HDMITX_AFS_44K;
		break;
	case 48000:
		audio.rate = TMDL_HDMITX_AFS_48K;
		break;
	case 88200:
		audio.rate = TMDL_HDMITX_AFS_88K;
		break;
	case 96000:
		audio.rate = TMDL_HDMITX_AFS_96K;
		break;
	case 176400:
		audio.rate = TMDL_HDMITX_AFS_176K;
		break;
	case 192000:
		audio.rate = TMDL_HDMITX_AFS_192K;
		break;
	default:
		dev_err(codec->dev, "Unknown samplerate %d\n",
				params_rate(params));
		return -EINVAL;
	}

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		audio.i2sQualifier = TMDL_HDMITX_I2SQ_16BITS;
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		audio.i2sQualifier = TMDL_HDMITX_I2SQ_32BITS;
		break;
	default:
		dev_err(codec->dev, "Unknown format %d\n",
				params_format(params));
		return -EINVAL;
	}

	tda_config_audioin(&audio);

	return 0;
}

const struct snd_soc_dai_ops tda19988_ops = {
	.hw_params	= tda19988_hw_params,
};

static struct snd_soc_dai_driver tda19988_codec_dai = {
	.name		= "tda19988-hifi",
	.playback 	= {
		.stream_name	= "Playback",
		.channels_min	= 1,
		.channels_max	= 2,
		.rates		= TDA19988_RATES,
		.formats	= TDA19988_FORMATS,
	},
	.ops = &tda19988_ops,
};

static int tda19988_snd_probe(struct platform_device *pdev)
{
	return snd_soc_register_codec(&pdev->dev, &soc_codec_tda19988,
			&tda19988_codec_dai, 1);
}

static int tda19988_snd_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

static struct platform_driver tda19988_driver = {
	.probe		= tda19988_snd_probe,
	.remove		= tda19988_snd_remove,
	.driver		= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};

module_platform_driver(tda19988_driver);

MODULE_AUTHOR("Ajay Bhargav <bhargav.ajay@gmail.com>");
MODULE_DESCRIPTION("TDA19988 codec driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);

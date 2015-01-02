/*
 * dove-d2plug.c
 *
 * (C) 2015 Ajay Bhargav <bhargav.ajay@gmail.com>
 *	    Rickey's World <www.8051projects.net>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/module.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <plat/audio.h>
#include <asm/mach-types.h>

#include "../codecs/rt5630.h"

static int d2plug_rt5630_hw_params(struct snd_pcm_substream *substream,	
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	unsigned int format, clk = 0;
	int ret = 0;

	dev_dbg(rtd->dev, "substream = %p, params = %p\n", substream, params);
	dev_dbg(rtd->dev, "rate = %d\n", params_rate(params));
	dev_dbg(rtd->dev, "codec_dai = %s\n", codec_dai->name);
	dev_dbg(rtd->dev, "cpu_dai = %s\n", cpu_dai->name);

	switch (params_rate(params)) {
	case 44100:
		clk = 11289600;
		break;
	case 48000:
		clk = 12288000;
		break;
	case 96000:
		clk = 24576000;
		break;
	default:
		return -EINVAL;
	}

	format = SND_SOC_DAIFMT_CBS_CFS |
		 SND_SOC_DAIFMT_I2S |
		 SND_SOC_DAIFMT_NB_NF;

	/* set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, format);
	if (ret < 0)
		return ret;

	/* codec system clock is supplied by codec pll1*/
	ret = snd_soc_dai_set_pll(codec_dai, RT5630_PLL1_FROM_MCLK, 0,
			clk, clk * 2);
	if (ret < 0)
		return ret;

	/* set the codec system clock */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, clk * 2, SND_SOC_CLOCK_IN);
	 if (ret < 0)
	  	return ret;
	 
	return 0;
}


static struct snd_soc_ops d2plug_rt5630_ops = {
	.hw_params = d2plug_rt5630_hw_params,
};

static int d2plug_card_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;

	dev_dbg(rtd->dev, "substream = %p, params = %p\n", substream, params);
	dev_dbg(rtd->dev, "rate = %d\n", params_rate(params));
	dev_dbg(rtd->dev, "codec_dai = %s\n", codec_dai->name);
	dev_dbg(rtd->dev, "cpu_dai = %s\n", cpu_dai->name);

	return 0;
}

static struct snd_soc_ops d2plug_card_ops = {
	.hw_params = d2plug_card_hw_params,
};

static struct snd_soc_dai_link d2plug_audio_dai[] = {
	{
		.name = "RT5630",
		.stream_name = "Audio Jack Playback",
		.platform_name = "kirkwood-pcm-audio.0",
		.cpu_dai_name = "kirkwood-i2s.0",
		.codec_dai_name = "rt5630-hifi",
		.codec_name = "rt5630-codec.0-001f",
		.ops = &d2plug_rt5630_ops,
	},
};

static struct snd_soc_dai_link d2plug_hdmi_dai[] = {
	{
		.name = "HDMI",
		.stream_name = "HDMI HiFi",
		.platform_name = "kirkwood-pcm-audio.1",
		.cpu_dai_name = "kirkwood-i2s.1",
		.codec_dai_name = "tda19988-hifi",
		.codec_name = "tda19988-codec",
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS,
		.ops = &d2plug_card_ops,
	},
};

static struct snd_soc_card d2plug_audio_card = {
	.name = "D2Plug Audio Jack",
	.owner = THIS_MODULE,
	.dai_link = d2plug_audio_dai,
	.num_links = ARRAY_SIZE(d2plug_audio_dai),
};

static struct snd_soc_card d2plug_hdmi_card = {
	.name = "D2Plug HDMI",
	.owner = THIS_MODULE,
	.dai_link = d2plug_hdmi_dai,
	.num_links = ARRAY_SIZE(d2plug_hdmi_dai),
};

static int __devinit d2plug_card_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card;
	int ret;

	dev_dbg(&pdev->dev, "pdev->id = %d\n", pdev->id);

	if (pdev->id)
		card = &d2plug_audio_card;
	else
		card = &d2plug_hdmi_card;

	card->dev = &pdev->dev;
	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n",
				ret);
	}


	return ret;
}

static int __devexit d2plug_card_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	snd_soc_unregister_card(card);

	return 0;
}

static struct platform_driver d2plug_card_driver = {
	.driver		= {
		.name	= "d2plug-audio",
		.owner	= THIS_MODULE,
	},
	.probe		= d2plug_card_probe,
	.remove		= __devexit_p(d2plug_card_remove),
};
module_platform_driver(d2plug_card_driver);

MODULE_AUTHOR("Ajay Bhargav <bhargav.ajay@gmail.com>");
MODULE_DESCRIPTION("ALSA SoC D2plug Audio Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:soc-audio");

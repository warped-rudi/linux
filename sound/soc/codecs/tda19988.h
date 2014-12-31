/*
 * Derived from tmdlHdmiTx_Types.h
 *
 * Copyright (C) 2007 NXP N.V., All Rights Reserved.
 * Copyright (C) 2014 Rickey's World <www.8051projects.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __TDA19988_H__
#define __TDA19988_H__

enum {
	TMDL_HDMITX_AFMT_SPDIF      = 0, /**< SPDIF */
	TMDL_HDMITX_AFMT_I2S        = 1, /**< I2S */
	TMDL_HDMITX_AFMT_OBA        = 2, /**< One bit audio / DSD */
	TMDL_HDMITX_AFMT_DST        = 3, /**< DST */
	TMDL_HDMITX_AFMT_HBR        = 4  /**< HBR */
};

enum {
	TMDL_HDMITX_AFS_32K           = 0, /**< 32kHz    */
	TMDL_HDMITX_AFS_44K           = 1, /**< 44.1kHz  */
	TMDL_HDMITX_AFS_48K           = 2, /**< 48kHz    */
	TMDL_HDMITX_AFS_88K           = 3, /**< 88.2kHz  */
	TMDL_HDMITX_AFS_96K           = 4, /**< 96kHz    */
	TMDL_HDMITX_AFS_176K          = 5, /**< 176.4kHz */
	TMDL_HDMITX_AFS_192K          = 6  /**< 192kHz   */
};

enum {
	TMDL_HDMITX_I2SFOR_PHILIPS_L	= 0,	/**< Philips like format */
	TMDL_HDMITX_I2SFOR_OTH_L	= 2,	/**< left justified */
	TMDL_HDMITX_I2SFOR_OTH_R	= 3,	/**< right justified */
	TMDL_HDMITX_I2SFOR_INVALID	= 4	/**< Invalid format */
};

enum {
	TMDL_HDMITX_I2SQ_16BITS		= 16, /**< 16 bits */
	TMDL_HDMITX_I2SQ_32BITS		= 32, /**< 32 bits */
	TMDL_HDMITX_I2SQ_OTHERS		= 0   /**< for SPDIF and DSD */
};

enum {
	TMDL_HDMITX_DSTRATE_SINGLE	= 0, /**< Single transfer rate */
	TMDL_HDMITX_DSTRATE_DOUBLE	= 1  /**< Double data rate */
};

/* PCM Idenfication */
enum {
	TMDL_HDMITX_AUDIO_DATA_PCM	= 0,  /**< linear PCM samples */
	TMDL_HDMITX_AUDIO_DATA_OTHER	= 1,  /**< other purposes */
	TMDL_HDMITX_AUDIO_DATA_INVALID	= 2   /**< Invalid value */  
};

/* Copyright */
enum {
	TMDL_HDMITX_CSCOPYRIGHT_PROTECTED	= 0,  /* Copyright protected */
	TMDL_HDMITX_CSCOPYRIGHT_UNPROTECTED	= 1,  /**< No copyright */
	TMDL_HDMITX_CSCOPYRIGHT_INVALID		= 2   /**< Invalid value */
};

/* Format Info */
enum {
	/* PCM 2 channels without pre-emphasis or NON Linear PCM */
	TMDL_HDMITX_CSFI_PCM_2CHAN_NO_PRE	= 0,
	/* PCM 2 channels with 50us/15us pre-emphasis */
	TMDL_HDMITX_CSFI_PCM_2CHAN_PRE		= 1,
	/* PCM Reserved for 2 channels with pre-emphasis */
	TMDL_HDMITX_CSFI_PCM_2CHAN_PRE_RSVD1	= 2,
	/* PCM Reserved for 2 channels with pre-emphasis */
	TMDL_HDMITX_CSFI_PCM_2CHAN_PRE_RSVD2	= 3,
	/**< Invalid value */
	TMDL_HDMITX_CSFI_INVALID		= 4 
};

/* Clock Accuracy */
enum {
	TMDL_HDMITX_CSCLK_LEVEL_II	= 0, /**< Level II */
	TMDL_HDMITX_CSCLK_LEVEL_I	= 1, /**< Level I */
	TMDL_HDMITX_CSCLK_LEVEL_III	= 2, /**< Level III */
	TMDL_HDMITX_CSCLK_NOT_MATCHED	= 3, /**< Not matched to sample freq */
	TMDL_HDMITX_CSCLK_INVALID	= 4  /**< Invalid */
};

/* Max Word Length */
enum {
	TMDL_HDMITX_CSMAX_LENGTH_20	= 0,  /**< Max word length is 20 bits */
	TMDL_HDMITX_CSMAX_LENGTH_24	= 1,  /**< Max word length is 24 bits */
	TMDL_HDMITX_CSMAX_INVALID	= 2   /**< Invalid value */
};

/* World Length */
enum {
	TMDL_HDMITX_CSWORD_DEFAULT = 0,     /**< Word length is not indicated                    */
	TMDL_HDMITX_CSWORD_20_OF_24 = 1,     /**< Sample length is 20 bits out of max 24 possible */
	TMDL_HDMITX_CSWORD_16_OF_20 = 1,     /**< Sample length is 16 bits out of max 20 possible */
	TMDL_HDMITX_CSWORD_22_OF_24 = 2,     /**< Sample length is 22 bits out of max 24 possible */
	TMDL_HDMITX_CSWORD_18_OF_20 = 2,     /**< Sample length is 18 bits out of max 20 possible */
	TMDL_HDMITX_CSWORD_RESVD = 3,     /**< Reserved - shall not be used */
	TMDL_HDMITX_CSWORD_23_OF_24 = 4,     /**< Sample length is 23 bits out of max 24 possible */
	TMDL_HDMITX_CSWORD_19_OF_20 = 4,     /**< Sample length is 19 bits out of max 20 possible */
	TMDL_HDMITX_CSWORD_24_OF_24 = 5,     /**< Sample length is 24 bits out of max 24 possible */
	TMDL_HDMITX_CSWORD_20_OF_20 = 5,     /**< Sample length is 20 bits out of max 20 possible */
	TMDL_HDMITX_CSWORD_21_OF_24 = 6,     /**< Sample length is 21 bits out of max 24 possible */
	TMDL_HDMITX_CSWORD_17_OF_20 = 6,     /**< Sample length is 17 bits out of max 20 possible */
	TMDL_HDMITX_CSWORD_INVALID = 7      /**< Invalid */
};

/* Original Sample frequency */
enum {
	TMDL_HDMITX_CSOFREQ_NOT_INDICATED	= 0,   /**< Not Indicated */
	TMDL_HDMITX_CSOFREQ_192k		= 1,   /**< 192kHz        */
	TMDL_HDMITX_CSOFREQ_12k			= 2,   /**< 12kHz         */
	TMDL_HDMITX_CSOFREQ_176_4k		= 3,   /**< 176.4kHz      */
	TMDL_HDMITX_CSOFREQ_RSVD1		= 4,   /**< Reserved      */
	TMDL_HDMITX_CSOFREQ_96k			= 5,   /**< 96kHz         */
	TMDL_HDMITX_CSOFREQ_8k			= 6,   /**< 8kHz          */
	TMDL_HDMITX_CSOFREQ_88_2k		= 7,   /**< 88.2kHz       */
	TMDL_HDMITX_CSOFREQ_16k			= 8,   /**< 16kHz         */
	TMDL_HDMITX_CSOFREQ_24k			= 9,   /**< 24kHz         */
	TMDL_HDMITX_CSOFREQ_11_025k		= 10,  /**< 11.025kHz     */
	TMDL_HDMITX_CSOFREQ_22_05k		= 11,  /**< 22.05kHz      */
	TMDL_HDMITX_CSOFREQ_32k			= 12,  /**< 32kHz         */
	TMDL_HDMITX_CSOFREQ_48k			= 13,  /**< 48kHz         */
	TMDL_HDMITX_CSOFREQ_RSVD2		= 14,  /**< Reserved      */
	TMDL_HDMITX_CSOFREQ_44_1k		= 15,  /**< 44.1kHz       */
	TMDL_HDMITX_CSAFS_INVALID		= 16   /**< Invalid value */
};

struct tda_aud_chstatus {
	int PcmIdentification;
	int CopyrightInfo;
	int FormatInfo;
	unsigned char categoryCode;
	int clockAccuracy;
	int maxWordLength;
	int wordLength;
	int origSampleFreq;
};

struct tda_audio_config {
	int format;             /**< Audio format (I2S, SPDIF, etc.) */
	int rate;               /**< Audio sampling rate */
	int i2sFormat;          /**< I2S format of the audio input */
	int i2sQualifier;       /**< I2S qualifier of the audio input (8,16,32 bits) */
	int dstRate;            /**< DST data transfer rate */
	unsigned char channelAllocation;  /**< Ref to CEA-861D p85 */
	struct tda_aud_chstatus channelStatus;      /**< Ref to IEC 60958-3 */
};

int tda_config_audioin(struct tda_audio_config *cfg);

#endif /* __TDA19988_H__ */

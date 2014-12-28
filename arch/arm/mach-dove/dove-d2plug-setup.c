/*
 * arch/arm/mach-dove/dove-d2plug-setup.c
 *
 * Based on Marvell Dove MV88F6781-RD Avengers Mobile Internet Device
 * Based on SolidRun CuBox Setup file
 *
 * Author: Ajay Bhargav <bhargav.ajay@gmail.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/nand.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/ata_platform.h>
#include <linux/mv643xx_eth.h>
#include <linux/i2c.h>
#include <linux/pci.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/gpio.h>
#include <linux/leds.h>
#include <linux/clkdev.h>
#include <video/dovefb.h>
#include <video/dovefbreg.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/dove.h>
#include <mach/sdhci.h>
#include "common.h"
#include "mpp.h"
#include <linux/clk-provider.h>
#include <linux/clkdev.h>

#define DOVE_AVNG_POWER_OFF_GPIO	(8)

static int hdmi_clk_enable(struct clk_hw *hw)
{
	int ret;

	ret = gpio_request_one(12, GPIOF_OUT_INIT_HIGH, "HDMI_CKSW");
	if (ret < 0) {
		pr_err("%s: failed to setup GPIO for HDMI_CKSW\n", __func__);
		return ret;
	}

	ret = gpio_request_one(13, GPIOF_OUT_INIT_HIGH, "HDMI_CKSWET1");
	if (ret < 0) {
		pr_err("%s: failed to setup GPIO for HDMI_CKSWET1\n", __func__);
		goto err_ckswet1;
	}

	ret = gpio_request_one(14, GPIOF_OUT_INIT_HIGH, "HDMI_CKSWET2");
	if (ret < 0) {
		pr_err("%s: failed to setup GPIO for HDMI_CKSWET2\n", __func__);
		goto err_ckswet2;
	}

	pr_info("%s: Clock enable success\n", __func__);

	return 0;

err_ckswet1:
	gpio_free(12);
err_ckswet2:
	gpio_free(13);

	return ret;
}

static void hdmi_clk_disable(struct clk_hw *hw)
{
	pr_info("%s: Freeing GPIO for HDMI Clock\n", __func__);
	gpio_free(12);
	gpio_free(13);
	gpio_free(14);
}

static int hdmi_clk_setrate(struct clk_hw *hw, unsigned long rate,
		unsigned long parent)
{
	pr_info("%s: Requested rate is %lu\n", __func__, rate);

	switch (rate) {
	case 27000000:
		gpio_set_value(12, 1);
	case 74250000:
		gpio_set_value(12, 0);
		gpio_set_value(13, 0);
		gpio_set_value(14, 0);
		break;
	case 74176000:
		gpio_set_value(12, 1);
		gpio_set_value(13, 1);
		gpio_set_value(14, 0);
		break;
	case 148500000:
		gpio_set_value(12, 1);
		gpio_set_value(13, 0);
		gpio_set_value(14, 1);
		break;
	case 148352000:
		gpio_set_value(12, 1);
		gpio_set_value(13, 1);
		gpio_set_value(14, 1);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static long hdmi_clk_roundrate(struct clk_hw *hw, unsigned long rate,
		unsigned long *parent)
{
	pr_info("%s: Requested rate %lu\n", __func__, rate);

	/*
	 * Ignoring last three digits as we are only checking
	 * upto three decimal places
	 */

	rate = rate/1000;

	pr_info("%s: Final calcuated rate %lu\n", __func__, rate);

	switch (rate) {
	case 27000:
	case 74250:
	case 74176:
	case 148500:
	case 148352:
		return rate * 1000;
	default:
		return 0;
	}
}

unsigned long hdmi_clk_recalcrate(struct clk_hw *hw, unsigned long parent)
{
	return 148500000;
}

const struct clk_ops hdmi_clk_ops = {
	.enable = hdmi_clk_enable,
	.disable = hdmi_clk_disable,
	.set_rate = hdmi_clk_setrate,
	.round_rate = hdmi_clk_roundrate,
	.recalc_rate = hdmi_clk_recalcrate,
};

static struct clk_init_data hdmi_clk_initdata = {
	.name = "hdmi_clk",
	.ops = &hdmi_clk_ops,
	.flags = CLK_IS_ROOT,
};

static struct clk_hw hdmi_hw_clk = {
	.init = &hdmi_clk_initdata,
};

static int __init setup_hdmi_clk(void)
{
	struct clk *clk;
	struct clk_lookup *cl;

	clk = clk_register(NULL, &hdmi_hw_clk);
	if (IS_ERR(clk)) {
		pr_err("%s: Error registring HDMI clock (%ld)\n",
				__func__, PTR_ERR(clk));
		return PTR_ERR(clk);
	}
	pr_info("%s: Clock register success", __func__);

	cl = clkdev_alloc(clk, "hdmi_clk", NULL);
	if (cl) {
		clkdev_add(cl);
	} else {
		pr_err("%s: Unable to allocate clock lookup\n", __func__);
		return -ENOMEM;
	}

	return 0;
}

/*
 * HDMI & VGA Setup
 */

/*
 * LCD HW output Red[0] to LDD[0] when set bit [19:16] of reg 0x190
 * to 0x0. Which means HW outputs BGR format default. All platforms
 * uses this controller should enable .panel_rbswap. Unless layout
 * design connects Blue[0] to LDD[0] instead.
 */
static struct dovefb_mach_info dove_d2plug_lcd0_dmi = {
	.id_gfx			= "GFX Layer 0",
	.id_ovly		= "Video Layer 0",
	.clk_src		= MRVL_EXT_CLK1,
	.clk_name		= "hdmi_clk",
	.pix_fmt		= PIX_FMT_RGB888PACK,
	.io_pin_allocation	= IOPAD_DUMB24,
	.panel_rgb_type		= DUMB24_RGB888_0,
	.panel_rgb_reverse_lanes= 0,
	.gpio_output_data	= 0,
	.gpio_output_mask	= 0,
	.ddc_i2c_adapter	= 1,
	.ddc_i2c_address	= 0x3f,
	.invert_composite_blank	= 0,
	.invert_pix_val_ena	= 0,
	.invert_pixclock	= 0,
	.invert_vsync		= 0,
	.invert_hsync		= 0,
	.panel_rbswap		= 1,
	.active			= 1,
};

static struct dovefb_mach_info dove_d2plug_lcd0_vid_dmi = {
	.id_ovly		= "Video Layer 0",
	.pix_fmt		= PIX_FMT_RGB888PACK,
	.io_pin_allocation	= IOPAD_DUMB24,
	.panel_rgb_type		= DUMB24_RGB888_0,
	.panel_rgb_reverse_lanes= 0,
	.gpio_output_data	= 0,
	.gpio_output_mask	= 0,
	.ddc_i2c_adapter	= -1,
	.invert_composite_blank	= 0,
	.invert_pix_val_ena	= 0,
	.invert_pixclock	= 0,
	.invert_vsync		= 0,
	.invert_hsync		= 0,
	.panel_rbswap		= 1,
	.active			= 0,
	.enable_lcd0		= 0,
};

static struct dovefb_mach_info dove_d2plug_lcd1_dmi = {
	.id_gfx			= "GFX Layer 1",
	.id_ovly		= "Video Layer 1",
	.clk_src		= MRVL_PLL_CLK,
	.clk_name		= "accurate_LCDCLK",
	.pix_fmt		= PIX_FMT_RGB888PACK,
	.io_pin_allocation	= IOPAD_DUMB24,
	.panel_rgb_type		= DUMB24_RGB888_0,
	.panel_rgb_reverse_lanes= 0,
	.gpio_output_data	= 0,
	.gpio_output_mask	= 0,
	.ddc_i2c_adapter	= 1	,
	.ddc_i2c_address	= 0x50,
	.invert_composite_blank	= 0,
	.invert_pix_val_ena	= 0,
	.invert_pixclock	= 0,
	.invert_vsync		= 0,
	.invert_hsync		= 0,
	.panel_rbswap		= 1,
	.active			= 1,
#ifndef CONFIG_FB_DOVE_CLCD
	.enable_lcd0		= 1,
#else
	.enable_lcd0		= 0,
#endif
};

static struct dovefb_mach_info dove_d2plug_lcd1_vid_dmi = {
	.id_ovly		= "Video Layer 1",
	.pix_fmt		= PIX_FMT_RGB888PACK,
	.io_pin_allocation	= IOPAD_DUMB24,
	.panel_rgb_type		= DUMB24_RGB888_0,
	.panel_rgb_reverse_lanes= 0,
	.gpio_output_data	= 0,
	.gpio_output_mask	= 0,
	.ddc_i2c_adapter	= -1,
	.invert_composite_blank	= 0,
	.invert_pix_val_ena	= 0,
	.invert_pixclock	= 0,
	.invert_vsync		= 0,
	.invert_hsync		= 0,
	.panel_rbswap		= 1,
	.active			= 0,
};

/*
 * TODO: Fix backlight... Don't know if its needed for HDMI atleast
 */
#if 0
/*
 * BACKLIGHT
 */
static struct dovebl_platform_data dove_d2plug_backlight_data = {
	.default_intensity = 0xa,
	.gpio_pm_control = 1,

	.lcd_start = DOVE_SB_REGS_VIRT_BASE,	/* lcd power control reg base. */
	.lcd_end = DOVE_SB_REGS_VIRT_BASE+GPP_DATA_OUT_REG(0),	/* end of reg map. */
	.lcd_offset = GPP_DATA_OUT_REG(0),	/* register offset */
	.lcd_mapped = 1,		/* va = 0, pa = 1 */
	.lcd_mask = (1<<26),		/* mask, bit[11] */
	.lcd_on = (1<<26),		/* value to enable lcd power */
	.lcd_off = 0x0,			/* value to disable lcd power */

	.blpwr_start = DOVE_SB_REGS_VIRT_BASE, /* bl pwr ctrl reg base. */
	.blpwr_end = DOVE_SB_REGS_VIRT_BASE+GPP_DATA_OUT_REG(0),	/* end of reg map. */
	.blpwr_offset = GPP_DATA_OUT_REG(0),	/* register offset */
	.blpwr_mapped = 1,		/* pa = 0, va = 1 */
	.blpwr_mask = (1<<28),		/* mask, bit[13] */
	.blpwr_on = (1<<28),		/* value to enable bl power */
	.blpwr_off = 0x0,		/* value to disable bl power */

	.btn_start = DOVE_LCD1_PHYS_BASE, /* brightness control reg base. */
	.btn_end = DOVE_LCD1_PHYS_BASE+0x1C8,	/* end of reg map. */
	.btn_offset = LCD_CFG_GRA_PITCH,	/* register offset */
	.btn_mapped = 0,		/* pa = 0, va = 1 */
	.btn_mask = 0xF0000000,		/* mask */
	.btn_level = 15,		/* how many level can be configured. */
	.btn_min = 0x1,			/* min value */
	.btn_max = 0xF,			/* max value */
	.btn_inc = 0x1,			/* increment */
};
#endif

void __init dove_d2plug_clcd_init(void) {
#ifdef CONFIG_FB_DOVE
	clcd_platform_init(&dove_d2plug_lcd0_dmi, &dove_d2plug_lcd0_vid_dmi,
			&dove_d2plug_lcd1_dmi, &dove_d2plug_lcd1_vid_dmi,
			NULL);
#endif /* CONFIG_FB_DOVE */
}

/*
 * I2C Device info
 *
 * Audio codec and HDMI TX Chip
 *
 * TODO: Need some cleanup here
 *
 */
static struct i2c_board_info __initdata dove_d2plug_i2c_bus0_devs[] = {
	{
		I2C_BOARD_INFO("rt5630", 0x1f),
	},
	{
		I2C_BOARD_INFO("rt5623", 0x1a),
	},
	{
		I2C_BOARD_INFO("tda998X", 0x70),
	},
	{
		I2C_BOARD_INFO("tda99Xcec", 0x34),
	},
};

static struct i2c_board_info __initdata dove_d2plug_i2c_bus1_devs[] = {
};

static int __init dove_d2plug_pci_init(void)
{
	if (machine_is_d2plug())
		dove_pcie_init(1, 1);

	return 0;
}

subsys_initcall(dove_d2plug_pci_init);

/*
 * Ethernet
 */
static struct mv643xx_eth_platform_data dove_d2plug_ge00_data = {
	.phy_addr = MV643XX_ETH_PHY_ADDR_DEFAULT,
};

static struct sdhci_dove_platform_data d2plug_sdio_data = {
	.gpio_cd = 16,
};

/*
 * SATA
 */
static struct mv_sata_platform_data dove_d2plug_sata_data = {
        .n_ports = 1,
};

/*
 * SPI Devices:
 *     SPI0: 4M Flash MX25L3205D
 */
static const struct flash_platform_data dove_d2plug_spi_flash_data = {
	.type = "mx25l3205d",
};

static struct spi_board_info __initdata dove_d2plug_spi_flash_info[] = {
	{
		.modalias       = "m25p80",
		.platform_data  = &dove_d2plug_spi_flash_data,
		.irq            = -1,
		.max_speed_hz   = 20000000,
		.bus_num        = 0,
		.chip_select    = 0,
	},
};

/*
 * NAND
 *
 * TODO: Add support for on-board nand
 *
 */
#if 0
static struct mtd_partition partition_dove[] = {
{ .name		= "Root",
	  .offset	= 0,
	  .size		= MTDPART_SIZ_FULL },
};
static u64 nfc_dmamask = DMA_BIT_MASK(32);
static struct nfc_platform_data dove_d2plug_nfc_hal_data = {
	.nfc_width      = 8,			/* Assume non ganaged mode by default */
	.num_devs       = 1,			/* Assume non ganaged mode by defautl */
	.num_cs         = 1,			/* K9LBG08U0D */
	.use_dma	= 1,
	.ecc_type	= MV_NFC_ECC_BCH_1K,	/* 8bit ECC required by K9LBG08U0D */
	.parts = partition_dove,
	.nr_parts = ARRAY_SIZE(partition_dove)
};

static struct nfc_platform_data dove_d2plug_nfc_data = {
	.nfc_width      = 8,
	.num_devs       = 1,
	.num_cs         = 1,
	.use_dma        = 1,
	.ecc_type	= MV_NFC_ECC_HAMMING,
	.parts = partition_dove,
	.nr_parts = ARRAY_SIZE(partition_dove)
};

static struct resource dove_nfc_resources[]  = {
	[0] = {
		.start	= (DOVE_NFC_PHYS_BASE),
		.end	= (DOVE_NFC_PHYS_BASE + 0xFF),
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_NAND,
		.end	= IRQ_NAND,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		/* DATA DMA */
		.start	= 97,
		.end	= 97,
		.flags	= IORESOURCE_DMA,
	},
	[3] = {
		/* COMMAND DMA */
		.start	= 99,
		.end	= 99,
		.flags	= IORESOURCE_DMA,
	},
};

static struct platform_device dove_nfc = {
	.name		= "orion-nfc",
	.id		= -1,
	.dev		= {
		.dma_mask		= &nfc_dmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
		.platform_data		= &dove_d2plug_nfc_data,
	},
	.resource	= dove_nfc_resources,
	.num_resources	= ARRAY_SIZE(dove_nfc_resources),
};

static void __init dove_d2plug_nfc_init(void)
{
	/* Check if HAL driver is intended */
	if (useHalDrivers || useNandHal) {
		dove_nfc.name = "orion-nfc-hal";

		/* Check for ganaged mode */
		if (strncmp(useNandHal, "ganged", 6) == 0) {
			dove_d2plug_nfc_hal_data.nfc_width = 16;
			dove_d2plug_nfc_hal_data.num_devs = 2;
			useNandHal += 7;
		}

		/* Check for ECC type directive */
		if (strcmp(useNandHal, "4bitecc") == 0)
			dove_d2plug_nfc_hal_data.ecc_type = MV_NFC_ECC_BCH_2K;
		else if (strcmp(useNandHal, "8bitecc") == 0)
			dove_d2plug_nfc_hal_data.ecc_type = MV_NFC_ECC_BCH_1K;
		else if (strcmp(useNandHal, "12bitecc") == 0)
			dove_d2plug_nfc_hal_data.ecc_type = MV_NFC_ECC_BCH_704B;
		else if (strcmp(useNandHal, "16bitecc") == 0)
			dove_d2plug_nfc_hal_data.ecc_type = MV_NFC_ECC_BCH_512B;

		dove_d2plug_nfc_hal_data.tclk = dove_tclk_get();
		dove_nfc.dev.platform_data = &dove_d2plug_nfc_hal_data;

	} else {
		/* NON HAL driver is used */
		dove_d2plug_nfc_data.tclk = dove_tclk_get();
	}

	platform_device_register(&dove_nfc);
}
#endif

/*
 * GPIO setup
 */
static unsigned int d2plug_mpp_list[] __initdata = {
	MPP0_GPIO0,
	MPP1_GPIO1,
	MPP2_GPIO2,
	MPP3_GPIO3,
	MPP4_GPIO4,
	MPP8_GPIO8,
	MPP11_GPIO11,
	MPP12_GPIO12,
	MPP13_GPIO13,
	MPP14_GPIO14,
	MPP15_GPIO15,
	MPP16_GPIO16,
	MPP17_TW_SDA2,
	MPP18_GPIO18,
	MPP19_TW_SCK2,
	MPP20_GPIO20,
	MPP21_UA1_RTSn,
	MPP22_UA1_CTSn,
	MPP23_GPIO23,
	0
};

static unsigned int d2plug_mpp_grp_list[] __initdata = {
	MPP_GRP_24_39_GPIO,
	MPP_GRP_40_45_SD0,
	MPP_GRP_46_51_SD1,
	MPP_GRP_58_61_SPI,
	MPP_GRP_62_63_UA1,
	0
};

/*
 * GPIO
 */
static void __init dove_d2plug_gpio_init(void)
{
	/* TODO: GPIO LEDs maybe on trigger? */
	orion_gpio_set_valid(0, 1);
	if (gpio_request_one(0, GPIOF_OUT_INIT_LOW, "LED_O0") != 0)
		pr_err("Dove: failed to setup GPIO for LED_O0\n");
	
	orion_gpio_set_valid(1, 1);
	if (gpio_request_one(1, GPIOF_OUT_INIT_LOW, "LED_O1") != 0)
		pr_err("Dove: failed to setup GPIO for LED_O1\n");

	orion_gpio_set_valid(2, 1);
	if (gpio_request_one(2, GPIOF_OUT_INIT_LOW, "LED_O2") != 0)
		pr_err("Dove: failed to setup GPIO for LED_O2\n");

	orion_gpio_set_valid(11, 1);
	if (gpio_request_one(11, GPIOF_IN, "AUDIO_INT") != 0)
		pr_err("Dove: failed to setup GPIO for AUDIO_INT\n");

	/* HDMI Clock Controller GPIOs */
	orion_gpio_set_valid(12, 1);
	orion_gpio_set_valid(13, 1);
	orion_gpio_set_valid(14, 1);

	/* Clock generator Enable */
	orion_gpio_set_valid(15, 1);
	if (gpio_request_one(15, GPIOF_OUT_INIT_HIGH, "VGA_CKSDOE") != 0)
		pr_err("Dove: failed to setup GPIO for VGA_CKSDOE\n");

	/* WLAN PD */
	orion_gpio_set_valid(23, 1);
	if (gpio_request_one(23, GPIOF_OUT_INIT_HIGH, "WLAN_PD") != 0)
		pr_err("Dove: failed to setup GPIO for WLAN_PD\n");
}

/*
 * Board Init
 */
static void __init dove_d2plug_init(void)
{
	/*
	 * Basic Dove setup. Needs to be called early.
	 */
	dove_init();

	dove_mpp_conf(d2plug_mpp_list, d2plug_mpp_grp_list, 1, 0);
	dove_d2plug_gpio_init();

	dove_hwmon_init();
	setup_hdmi_clk();

	dove_ge00_init(&dove_d2plug_ge00_data);
	dove_ehci0_init();
	dove_ehci1_init();
	dove_sata_init(&dove_d2plug_sata_data);
	dove_spi0_init();

	/* console to be on that device */
	dove_uart0_init();
	dove_uart1_init();
	dove_uart2_init();

	dove_i2c_init();
	dove_sdio0_init(&d2plug_sdio_data);
	dove_sdio1_init(&d2plug_sdio_data);
	/* dove_d2plug_nfc_init(); */

	dove_gpu_init();
	dove_vmeta_init();
	dove_d2plug_clcd_init();

	dove_i2s0_init();
	dove_i2s1_init();

	i2c_register_board_info(0, dove_d2plug_i2c_bus0_devs,
			ARRAY_SIZE(dove_d2plug_i2c_bus0_devs));

	i2c_register_board_info(1, dove_d2plug_i2c_bus1_devs,
			ARRAY_SIZE(dove_d2plug_i2c_bus1_devs));


	spi_register_board_info(dove_d2plug_spi_flash_info,
			ARRAY_SIZE(dove_d2plug_spi_flash_info));
}

MACHINE_START(D2PLUG, "Marvell MV88AP510 D2Plug")
	.atag_offset	= 0x100,
	.init_machine	= dove_d2plug_init,
	.map_io		= dove_map_io,
	.init_early	= dove_init_early,
	.init_irq	= dove_init_irq,
	.timer		= &dove_timer,
	.restart	= dove_restart,
	/* reserve memory for VMETA and GPU */
	.fixup          = dove_tag_fixup_mem32,
MACHINE_END

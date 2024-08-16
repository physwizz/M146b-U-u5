/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Common Clock Framework support for Exynos2100 SoC.
 */

#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <soc/samsung/cal-if.h>
#include <dt-bindings/clock/s5e9935.h>

#include "../../soc/samsung/cal-if/s5e9935/cmucal/cmucal-vclk.h"
#include "../../soc/samsung/cal-if/s5e9935/cmucal/cmucal-node.h"
#include "../../soc/samsung/cal-if/s5e9935/cmucal/cmucal-qch.h"
#include "../../soc/samsung/cal-if/s5e9935/cmucal/clkout_s5e9935.h"
#include "composite.h"

static struct samsung_clk_provider *s5e9935_clk_provider;
bool clk_exynos_skip_hw;
/*
 * list of controller registers to be saved and restored during a
 * suspend/resume cycle.
 */
/* fixed rate clocks generated outside the soc */
struct samsung_fixed_rate s5e9935_fixed_rate_ext_clks[] = {
	FRATE(OSCCLK1, "fin_pll1", NULL, 0, 76800000),
	FRATE(OSCCLK2, "fin_pll2", NULL, 0, 25600000),
};

struct init_vclk s5e9935_alive_hwacg_vclks[] = {
	HWACG_VCLK(GATE_MCT_ALIVE_QCH, MCT_ALIVE_QCH, "GATE_MCT_ALIVE_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_GREBEINTEGRATION1_QCH_GREBE, GREBEINTEGRATION1_QCH_GREBE, "GATE_GREBEINTEGRATION1_QCH_GREBE", NULL, 0, VCLK_GATE | VCLK_QCH_DIS, NULL),
};

struct init_vclk s5e9935_alive_vclks[] = {
	VCLK(DOUT_CLKALIVE_CHUBVTS_NOC, CLKALIVE_CHUBVTS_NOC, "DOUT_CLKALIVE_CHUBVTS_NOC", 0, 0, NULL),
	VCLK(DOUT_CLKALIVE_CHUB_PERI, CLKALIVE_CHUB_PERI, "DOUT_CLKALIVE_CHUB_PERI", 0, 0, NULL),
};

struct init_vclk s5e9935_allcsis_hwacg_vclks[] = {
	HWACG_VCLK(GATE_CSIS_PDP_QCH_DMA, CSIS_PDP_QCH_DMA, "GATE_CSIS_PDP_QCH_DMA", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_CSIS_PDP_QCH_PDP, CSIS_PDP_QCH_PDP, "GATE_CSIS_PDP_QCH_PDP", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_LH_AST_SI_OTF0_CSISCSTAT_QCH, LH_AST_SI_OTF0_CSISCSTAT_QCH, "GATE_LH_AST_SI_OTF0_CSISCSTAT_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_LH_AST_SI_OTF1_CSISCSTAT_QCH, LH_AST_SI_OTF1_CSISCSTAT_QCH, "GATE_LH_AST_SI_OTF1_CSISCSTAT_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_LH_AST_SI_OTF2_CSISCSTAT_QCH, LH_AST_SI_OTF2_CSISCSTAT_QCH, "GATE_LH_AST_SI_OTF2_CSISCSTAT_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_LH_AST_SI_OTF3_CSISCSTAT_QCH, LH_AST_SI_OTF3_CSISCSTAT_QCH, "GATE_LH_AST_SI_OTF3_CSISCSTAT_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_OIS_MCU_TOP_QCH, OIS_MCU_TOP_QCH, "GATE_OIS_MCU_TOP_QCH", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_aud_hwacg_vclks[] = {
	HWACG_VCLK(UMUX_CLKVTS_AUD_DMIC0, MUX_CLKVTS_AUD_DMIC0_USER, "UMUX_CLKVTS_AUD_DMIC0", NULL, 0, 0, NULL),
	HWACG_VCLK(GATE_ABOX_QCH_BCLK_DSIF, ABOX_QCH_BCLK_DSIF, "GATE_ABOX_QCH_BCLK_DSIF", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_ABOX_QCH_BCLK0, ABOX_QCH_BCLK0, "GATE_ABOX_QCH_BCLK0", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_ABOX_QCH_BCLK1, ABOX_QCH_BCLK1, "GATE_ABOX_QCH_BCLK1", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_ABOX_QCH_BCLK2, ABOX_QCH_BCLK2, "GATE_ABOX_QCH_BCLK2", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_ABOX_QCH_BCLK3, ABOX_QCH_BCLK3, "GATE_ABOX_QCH_BCLK3", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_ABOX_QCH_BCLK4, ABOX_QCH_BCLK4, "GATE_ABOX_QCH_BCLK4", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_ABOX_QCH_BCLK5, ABOX_QCH_BCLK5, "GATE_ABOX_QCH_BCLK5", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_ABOX_QCH_BCLK6, ABOX_QCH_BCLK6, "GATE_ABOX_QCH_BCLK6", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_aud_vclks[] = {
	VCLK(MOUT_MUX_CLK_AUD_PCMC, MUX_CLK_AUD_PCMC, "MOUT_MUX_CLK_AUD_PCMC", 0, 0, NULL),
	VCLK(MOUT_MUX_CLK_AUD_SCLK, MUX_CLK_AUD_SCLK, "MOUT_MUX_CLK_AUD_SCLK", 0, 0, NULL),
	VCLK(MOUT_CLK_AUD_UAIF6, MUX_CLK_AUD_UAIF6, "MOUT_CLK_AUD_UAIF6", 0, 0, NULL),
	VCLK(MOUT_CLK_AUD_UAIF0, MUX_CLK_AUD_UAIF0, "MOUT_CLK_AUD_UAIF0", 0, 0, NULL),
	VCLK(MOUT_CLK_AUD_UAIF1, MUX_CLK_AUD_UAIF1, "MOUT_CLK_AUD_UAIF1", 0, 0, NULL),
	VCLK(MOUT_CLK_AUD_UAIF2, MUX_CLK_AUD_UAIF2, "MOUT_CLK_AUD_UAIF2", 0, 0, NULL),
	VCLK(MOUT_CLK_AUD_UAIF3, MUX_CLK_AUD_UAIF3, "MOUT_CLK_AUD_UAIF3", 0, 0, NULL),
	VCLK(MOUT_CLK_AUD_UAIF4, MUX_CLK_AUD_UAIF4, "MOUT_CLK_AUD_UAIF4", 0, 0, NULL),
	VCLK(MOUT_CLK_AUD_UAIF5, MUX_CLK_AUD_UAIF5, "MOUT_CLK_AUD_UAIF5", 0, 0, NULL),
	VCLK(UMUX_CP_PCMC_CLK, MUX_CP_PCMC_CLK_USER, "UMUX_CP_PCMC_CLK", 0, 0, NULL),
	VCLK(MOUT_CLK_AUD_SERIAL_LIF, MUX_CLK_AUD_SERIAL_LIF, "MOUT_CLK_AUD_SERIAL_LIF", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_AUD_DSIF, DIV_CLK_AUD_DSIF, "DOUT_DIV_CLK_AUD_DSIF", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_AUD_UAIF0, DIV_CLK_AUD_UAIF0, "DOUT_DIV_CLK_AUD_UAIF0", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_AUD_UAIF1, DIV_CLK_AUD_UAIF1, "DOUT_DIV_CLK_AUD_UAIF1", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_AUD_UAIF2, DIV_CLK_AUD_UAIF2, "DOUT_DIV_CLK_AUD_UAIF2", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_AUD_UAIF3, DIV_CLK_AUD_UAIF3, "DOUT_DIV_CLK_AUD_UAIF3", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_AUD_NOC, DIV_CLK_AUD_NOC, "DOUT_DIV_CLK_AUD_NOC", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_AUD_CNT, DIV_CLK_AUD_CNT, "DOUT_DIV_CLK_AUD_CNT", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_AUD_UAIF4, DIV_CLK_AUD_UAIF4, "DOUT_DIV_CLK_AUD_UAIF4", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_AUD_UAIF5, DIV_CLK_AUD_UAIF5, "DOUT_DIV_CLK_AUD_UAIF5", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_AUD_UAIF6, DIV_CLK_AUD_UAIF6, "DOUT_DIV_CLK_AUD_UAIF6", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_AUD_SERIAL_LIF, DIV_CLK_AUD_SERIAL_LIF, "DOUT_DIV_CLK_AUD_SERIAL_LIF", 0, 0, NULL),
};

struct init_vclk s5e9935_cmgp_hwacg_vclks[] = {
	HWACG_VCLK(GATE_I2C_CMGP2_QCH, I2C_CMGP2_QCH, "GATE_I2C_CMGP2_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_I2C_CMGP3_QCH, I2C_CMGP3_QCH, "GATE_I2C_CMGP3_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_I2C_CMGP4_QCH, I2C_CMGP4_QCH, "GATE_I2C_CMGP4_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_I2C_CMGP5_QCH, I2C_CMGP5_QCH, "GATE_I2C_CMGP5_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_I2C_CMGP6_QCH, I2C_CMGP6_QCH, "GATE_I2C_CMGP6_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SPI_I2C_CMGP0_QCH, SPI_I2C_CMGP0_QCH, "GATE_SPI_I2C_CMGP0_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SPI_I2C_CMGP1_QCH, SPI_I2C_CMGP1_QCH, "GATE_SPI_I2C_CMGP1_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI_CMGP0_QCH, USI_CMGP0_QCH, "GATE_USI_CMGP0_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI_CMGP1_QCH, USI_CMGP1_QCH, "GATE_USI_CMGP1_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI_CMGP2_QCH, USI_CMGP2_QCH, "GATE_USI_CMGP2_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI_CMGP3_QCH, USI_CMGP3_QCH, "GATE_USI_CMGP3_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI_CMGP4_QCH, USI_CMGP4_QCH, "GATE_USI_CMGP4_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI_CMGP5_QCH, USI_CMGP5_QCH, "GATE_USI_CMGP5_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI_CMGP6_QCH, USI_CMGP6_QCH, "GATE_USI_CMGP6_QCH", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_cmgp_vclks[] = {
	VCLK(DOUT_DIV_CLK_CMGP_USI4, DIV_CLK_CMGP_USI4, "DOUT_DIV_CLK_CMGP_USI4", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_CMGP_USI1, DIV_CLK_CMGP_USI1, "DOUT_DIV_CLK_CMGP_USI1", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_CMGP_USI0, DIV_CLK_CMGP_USI0, "DOUT_DIV_CLK_CMGP_USI0", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_CMGP_USI2, DIV_CLK_CMGP_USI2, "DOUT_DIV_CLK_CMGP_USI2", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_CMGP_USI3, DIV_CLK_CMGP_USI3, "DOUT_DIV_CLK_CMGP_USI3", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_CMGP_USI5, DIV_CLK_CMGP_USI5, "DOUT_DIV_CLK_CMGP_USI5", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_CMGP_USI6, DIV_CLK_CMGP_USI6, "DOUT_DIV_CLK_CMGP_USI6", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_CMGP_I2C, DIV_CLK_CMGP_I2C, "DOUT_DIV_CLK_CMGP_I2C", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_CMGP_SPI_I2C0, DIV_CLK_CMGP_SPI_I2C0, "DOUT_DIV_CLK_CMGP_SPI_I2C0", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_CMGP_SPI_I2C1, DIV_CLK_CMGP_SPI_I2C1, "DOUT_DIV_CLK_CMGP_SPI_I2C1", 0, 0, NULL),
};

struct init_vclk s5e9935_csis_hwacg_vclks[] = {
	HWACG_VCLK(GATE_MIPI_PHY_LINK_WRAP_QCH_CSIS0, MIPI_PHY_LINK_WRAP_QCH_CSIS0, "GATE_MIPI_PHY_LINK_WRAP_QCH_CSIS0", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MIPI_PHY_LINK_WRAP_QCH_CSIS1, MIPI_PHY_LINK_WRAP_QCH_CSIS1, "GATE_MIPI_PHY_LINK_WRAP_QCH_CSIS1", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MIPI_PHY_LINK_WRAP_QCH_CSIS2, MIPI_PHY_LINK_WRAP_QCH_CSIS2, "GATE_MIPI_PHY_LINK_WRAP_QCH_CSIS2", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MIPI_PHY_LINK_WRAP_QCH_CSIS3, MIPI_PHY_LINK_WRAP_QCH_CSIS3, "GATE_MIPI_PHY_LINK_WRAP_QCH_CSIS3", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MIPI_PHY_LINK_WRAP_QCH_CSIS4, MIPI_PHY_LINK_WRAP_QCH_CSIS4, "GATE_MIPI_PHY_LINK_WRAP_QCH_CSIS4", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MIPI_PHY_LINK_WRAP_QCH_CSIS5, MIPI_PHY_LINK_WRAP_QCH_CSIS5, "GATE_MIPI_PHY_LINK_WRAP_QCH_CSIS5", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MIPI_PHY_LINK_WRAP_QCH_CSIS6, MIPI_PHY_LINK_WRAP_QCH_CSIS6, "GATE_MIPI_PHY_LINK_WRAP_QCH_CSIS6", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_dnc_hwacg_vclks[] = {
	HWACG_VCLK(UMUX_CLKCMU_DNC_NOC, MUX_CLKCMU_DNC_NOC_USER, "UMUX_CLKCMU_DNC_NOC", NULL, 0, 0, NULL),
	HWACG_VCLK(GATE_IP_DNC_QCH, IP_DNC_QCH, "GATE_IP_DNC_QCH", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_dpub_hwacg_vclks[] = {
	HWACG_VCLK(UMUX_CLKCMU_DPUB_NOC, MUX_CLKCMU_DPUB_NOC_USER, "UMUX_CLKCMU_DPUB_NOC", NULL, 0, 0, NULL),
};

struct init_vclk s5e9935_dsp_hwacg_vclks[] = {
	HWACG_VCLK(GATE_IP_DSP_QCH, IP_DSP_QCH, "GATE_IP_DSP_QCH", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_g3d_hwacg_vclks[] = {
	HWACG_VCLK(GATE_BG3D_PWRCTL_QCH, BG3D_PWRCTL_QCH, "GATE_BG3D_PWRCTL_QCH", NULL, 0, VCLK_GATE | VCLK_QCH_DIS, NULL),
};

struct init_vclk s5e9935_hsi0_hwacg_vclks[] = {
	HWACG_VCLK(UMUX_CLKCMU_HSI0_DPOSC, MUX_CLKCMU_HSI0_DPOSC_USER, "UMUX_CLKCMU_HSI0_DPOSC", NULL, 0, 0, NULL),
	HWACG_VCLK(UMUX_CLKCMU_HSI0_USB32DRD, MUX_CLKCMU_HSI0_USB32DRD_USER, "UMUX_CLKCMU_HSI0_USB32DRD", NULL, 0, 0, NULL),
	HWACG_VCLK(UMUX_CLKCMU_HSI0_NOC, MUX_CLKCMU_HSI0_NOC_USER, "UMUX_CLKCMU_HSI0_NOC", NULL, 0, 0, NULL),
	HWACG_VCLK(MOUT_CLK_HSI0_USB32DRD, MUX_CLK_HSI0_USB32DRD, "MOUT_CLK_HSI0_USB32DRD", NULL, 0, 0, NULL),
	HWACG_VCLK(GATE_USB32DRD_QCH_S_LINK, USB32DRD_QCH_S_LINK, "GATE_USB32DRD_QCH_S_LINK", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_ufs_hwacg_vclks[] = {
	HWACG_VCLK(GATE_UFS_EMBD_QCH, UFS_EMBD_QCH, "GATE_UFS_EMBD_QCH", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_ufs_vclks[] = {
};

struct init_vclk s5e9935_hsi1_hwacg_vclks[] = {
};

struct init_vclk s5e9935_lme_hwacg_vclks[] = {
	HWACG_VCLK(GATE_LME_QCH_0, LME_QCH_0, "GATE_LME_QCH_0", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_mfc_hwacg_vclks[] = {
	HWACG_VCLK(UMUX_CLKCMU_MFC_MFC, MUX_CLKCMU_MFC_MFC_USER, "UMUX_CLKCMU_MFC_MFC", NULL, 0, 0, NULL),
};

struct init_vclk s5e9935_mfd_hwacg_vclks[] = {
	HWACG_VCLK(UMUX_CLKCMU_MFD_MFD, MUX_CLKCMU_MFD_MFD_USER, "UMUX_CLKCMU_MFD_MFD", NULL, 0, 0, NULL),
	HWACG_VCLK(GATE_MFD, MFD_QCH, "GATE_MFD", "UMUX_CLKCMU_MFD_MFD", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MFD_FG, MFD_QCH_FG, "GATE_MFD_FG", "UMUX_CLKCMU_MFD_MFD", 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_m2m_hwacg_vclks[] = {
	HWACG_VCLK(GATE_M2M_QCH, M2M_QCH, "GATE_M2M_QCH", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_mcsc_hwacg_vclks[] = {
	HWACG_VCLK(GATE_MCFP_QCH, MCFP_QCH, "GATE_MCFP_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MCSC_QCH, MCSC_QCH, "GATE_MCSC_QCH", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_mif_hwacg_vclks[] = {
	HWACG_VCLK(MUX_MIF_DDRPHY2X, CLKMUX_MIF_DDRPHY2X, "MUX_MIF_DDRPHY2X", NULL, 0, 0, NULL),
};

struct init_vclk s5e9935_nocl1b_hwacg_vclks[] = {
	HWACG_VCLK(UMUX_CLKCMU_NOCL1B_NOC1, MUX_CLKCMU_NOCL1B_NOC1_USER, "UMUX_CLKCMU_NOCL1B_NOC1", NULL, 0, 0, NULL),
};

struct init_vclk s5e9935_peric0_hwacg_vclks[] = {
	HWACG_VCLK(GATE_PERIC0_CMU_PERIC0_QCH, PERIC0_CMU_PERIC0_QCH, "GATE_PERIC0_CMU_PERIC0_QCH", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_peric0_vclks[] = {
	VCLK(DOUT_DIV_CLK_PERIC0_USI04, DIV_CLK_PERIC0_USI04, "DOUT_DIV_CLK_PERIC0_USI04", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC0_I2C, DIV_CLK_PERIC0_I2C, "DOUT_DIV_CLK_PERIC0_I2C", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC0_USI10, DIV_CLK_PERIC0_USI10, "DOUT_DIV_CLK_PERIC0_USI10", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC0_DBG_UART, DIV_CLK_PERIC0_DBG_UART, "DOUT_DIV_CLK_PERIC0_DBG_UART", 0, 0, NULL),
};

struct init_vclk s5e9935_peric1_hwacg_vclks[] = {
	HWACG_VCLK(GATE_PERIC1_CMU_PERIC1_QCH, PERIC1_CMU_PERIC1_QCH, "GATE_PERIC1_CMU_PERIC1_QCH", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_peric1_vclks[] = {
	VCLK(DOUT_DIV_CLK_PERIC1_UART_BT, DIV_CLK_PERIC1_UART_BT, "DOUT_DIV_CLK_PERIC1_UART_BT", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC1_I2C, DIV_CLK_PERIC1_I2C, "DOUT_DIV_CLK_PERIC1_I2C", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC1_USI07, DIV_CLK_PERIC1_USI07, "DOUT_DIV_CLK_PERIC1_USI07", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC1_USI08, DIV_CLK_PERIC1_USI08, "DOUT_DIV_CLK_PERIC1_USI08", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC1_USI09, DIV_CLK_PERIC1_USI09, "DOUT_DIV_CLK_PERIC1_USI09", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC1_USI07_SPI_I2C, DIV_CLK_PERIC1_USI07_SPI_I2C, "DOUT_DIV_CLK_PERIC1_USI07_SPI_I2C", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC1_USI08_SPI_I2C, DIV_CLK_PERIC1_USI08_SPI_I2C, "DOUT_DIV_CLK_PERIC1_USI08_SPI_I2C", 0, 0, NULL),
};

struct init_vclk s5e9935_peric2_hwacg_vclks[] = {
	HWACG_VCLK(GATE_PERIC2_CMU_PERIC2_QCH, PERIC2_CMU_PERIC2_QCH, "GATE_PERIC2_CMU_PERIC2_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PWM_QCH, PWM_QCH, "GATE_PWM_QCH", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_peric2_vclks[] = {
	VCLK(DOUT_DIV_CLK_PERIC2_I2C, DIV_CLK_PERIC2_I2C, "DOUT_DIV_CLK_PERIC2_I2C", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC2_USI00, DIV_CLK_PERIC2_USI00, "DOUT_DIV_CLK_PERIC2_USI00", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC2_USI01, DIV_CLK_PERIC2_USI01, "DOUT_DIV_CLK_PERIC2_USI01", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC2_USI02, DIV_CLK_PERIC2_USI02, "DOUT_DIV_CLK_PERIC2_USI02", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC2_USI03, DIV_CLK_PERIC2_USI03, "DOUT_DIV_CLK_PERIC2_USI03", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC2_USI05, DIV_CLK_PERIC2_USI05, "DOUT_DIV_CLK_PERIC2_USI05", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC2_USI06, DIV_CLK_PERIC2_USI06, "DOUT_DIV_CLK_PERIC2_USI06", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC2_USI11, DIV_CLK_PERIC2_USI11, "DOUT_DIV_CLK_PERIC2_USI11", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC2_USI00_SPI_I2C, DIV_CLK_PERIC2_USI00_SPI_I2C, "DOUT_DIV_CLK_PERIC2_USI00_SPI_I2C", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_PERIC2_USI01_SPI_I2C, DIV_CLK_PERIC2_USI01_SPI_I2C, "DOUT_DIV_CLK_PERIC2_USI01_SPI_I2C", 0, 0, NULL),
};

struct init_vclk s5e9935_peris_hwacg_vclks[] = {
	HWACG_VCLK(UMUX_CLKCMU_PERIS_NOC, MUX_CLKCMU_PERIS_NOC_USER, "UMUX_CLKCMU_PERIS_NOC", NULL, 0, 0, NULL),
	HWACG_VCLK(GATE_MCT_QCH, MCT_QCH, "GATE_MCT_QCH", "UMUX_CLKCMU_PERIS_NOC", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_WDT0_QCH, WDT0_QCH, "GATE_WDT0_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_WDT1_QCH, WDT1_QCH, "GATE_WDT1_QCH", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_sdma_hwacg_vclks[] = {
	HWACG_VCLK(GATE_IP_SDMA_WRAP_QCH, IP_SDMA_WRAP_QCH, "GATE_IP_SDMA_WRAP_QCH", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IP_SDMA_WRAP_QCH_2, IP_SDMA_WRAP_QCH_2, "GATE_IP_SDMA_WRAP_QCH_2", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_top_hwacg_vclks[] = {
	HWACG_VCLK(GATE_DFTMUX_CMU_QCH_CIS_CLK0, DFTMUX_CMU_QCH_CIS_CLK0, "GATE_DFTMUX_CMU_QCH_CIS_CLK0", NULL, 0, VCLK_GATE | VCLK_QCH_DIS, NULL),
	HWACG_VCLK(GATE_DFTMUX_CMU_QCH_CIS_CLK1, DFTMUX_CMU_QCH_CIS_CLK1, "GATE_DFTMUX_CMU_QCH_CIS_CLK1", NULL, 0, VCLK_GATE | VCLK_QCH_DIS, NULL),
	HWACG_VCLK(GATE_DFTMUX_CMU_QCH_CIS_CLK2, DFTMUX_CMU_QCH_CIS_CLK2, "GATE_DFTMUX_CMU_QCH_CIS_CLK2", NULL, 0, VCLK_GATE | VCLK_QCH_DIS, NULL),
	HWACG_VCLK(GATE_DFTMUX_CMU_QCH_CIS_CLK3, DFTMUX_CMU_QCH_CIS_CLK3, "GATE_DFTMUX_CMU_QCH_CIS_CLK3", NULL, 0, VCLK_GATE | VCLK_QCH_DIS, NULL),
	HWACG_VCLK(GATE_DFTMUX_CMU_QCH_CIS_CLK4, DFTMUX_CMU_QCH_CIS_CLK4, "GATE_DFTMUX_CMU_QCH_CIS_CLK4", NULL, 0, VCLK_GATE | VCLK_QCH_DIS, NULL),
	HWACG_VCLK(GATE_DFTMUX_CMU_QCH_CIS_CLK5, DFTMUX_CMU_QCH_CIS_CLK5, "GATE_DFTMUX_CMU_QCH_CIS_CLK5", NULL, 0, VCLK_GATE | VCLK_QCH_DIS, NULL),
	HWACG_VCLK(GATE_DFTMUX_CMU_QCH_CIS_CLK6, DFTMUX_CMU_QCH_CIS_CLK6, "GATE_DFTMUX_CMU_QCH_CIS_CLK6", NULL, 0, VCLK_GATE | VCLK_QCH_DIS, NULL),
	HWACG_VCLK(GATE_DFTMUX_CMU_QCH_CIS_CLK7, DFTMUX_CMU_QCH_CIS_CLK7, "GATE_DFTMUX_CMU_QCH_CIS_CLK7", NULL, 0, VCLK_GATE | VCLK_QCH_DIS, NULL),
};

struct init_vclk s5e9935_top_vclks[] = {
	VCLK(DOUT_DIV_CLKCMU_AUD_AUDIF0, CLKCMU_AUD_AUDIF0, "DOUT_DIV_CLKCMU_AUD_AUDIF0", 0, 0, NULL),
	VCLK(DOUT_DIV_CLKCMU_AUD_AUDIF1, CLKCMU_AUD_AUDIF1, "DOUT_DIV_CLKCMU_AUD_AUDIF1", 0, 0, NULL),
	VCLK(DOUT_DIV_CLKCMU_CIS_CLK0, DIV_CLKCMU_CIS_CLK0, "DOUT_DIV_CLKCMU_CIS_CLK0", 0, 0, NULL),
	VCLK(DOUT_DIV_CLKCMU_CIS_CLK1, DIV_CLKCMU_CIS_CLK1, "DOUT_DIV_CLKCMU_CIS_CLK1", 0, 0, NULL),
	VCLK(DOUT_DIV_CLKCMU_CIS_CLK2, DIV_CLKCMU_CIS_CLK2, "DOUT_DIV_CLKCMU_CIS_CLK2", 0, 0, NULL),
	VCLK(DOUT_DIV_CLKCMU_CIS_CLK3, DIV_CLKCMU_CIS_CLK3, "DOUT_DIV_CLKCMU_CIS_CLK3", 0, 0, NULL),
	VCLK(DOUT_DIV_CLKCMU_CIS_CLK4, DIV_CLKCMU_CIS_CLK4, "DOUT_DIV_CLKCMU_CIS_CLK4", 0, 0, NULL),
	VCLK(DOUT_DIV_CLKCMU_CIS_CLK5, DIV_CLKCMU_CIS_CLK5, "DOUT_DIV_CLKCMU_CIS_CLK5", 0, 0, NULL),
	VCLK(DOUT_DIV_CLKCMU_CIS_CLK6, DIV_CLKCMU_CIS_CLK6, "DOUT_DIV_CLKCMU_CIS_CLK6", 0, 0, NULL),
	VCLK(DOUT_DIV_CLKCMU_CIS_CLK7, DIV_CLKCMU_CIS_CLK7, "DOUT_DIV_CLKCMU_CIS_CLK7", 0, 0, NULL),
	VCLK(DOUT_CLKCMU_UFS_UFS_EMBD, VCLK_DIV_CLK_UFS_UFS_EMBD, "DOUT_CLKCMU_UFS_UFS_EMBD", 0, 0, NULL),
	VCLK(DOUT_CLKCMU_HSI0_NOC, VCLK_CLKCMU_HSI0_NOC, "DOUT_CLKCMU_HSI0_NOC", 0, 0, NULL),
	VCLK(POUT_SHARED3_D1, PLL_SHARED3_D1, "POUT_SHARED3_D1", 0, 0, NULL),
	VCLK(POUT_SHARED4_D1, PLL_SHARED4_D1, "POUT_SHARED4_D1", 0, 0, NULL),
};

struct init_vclk s5e9935_vts_hwacg_vclks[] = {
	HWACG_VCLK(UMUX_CLKCMU_VTS_DMIC_USER, MUX_CLKCMU_VTS_DMIC_USER, "UMUX_CLKCMU_VTS_DMIC_USER", NULL, 0, 0, NULL),
	HWACG_VCLK(MOUT_CLK_VTS_DMIC, MUX_CLK_VTS_DMIC, "MOUT_CLK_VTS_DMIC", NULL, 0, 0, NULL),
	HWACG_VCLK(MOUT_CLKALIVE_VTS_NOC_USER, MUX_CLKALIVE_VTS_NOC_USER, "MOUT_CLKALIVE_VTS_NOC_USER", NULL, 0, 0, NULL),
	HWACG_VCLK(MOUT_CLKALIVE_VTS_RCO_USER, MUX_CLKALIVE_VTS_RCO_USER, "MOUT_CLKALIVE_VTS_RCO_USER", NULL, 0, 0, NULL),
};

struct init_vclk s5e9935_vts_vclks[] = {
	VCLK(DOUT_DIV_CLK_VTS_DMIC_IF, DIV_CLK_VTS_DMIC_IF, "DOUT_DIV_CLK_VTS_DMIC_IF", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_VTS_DMIC_IF_DIV2, DIV_CLK_VTS_DMIC_IF_DIV2, "DOUT_DIV_CLK_VTS_DMIC_IF_DIV2", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_VTS_SERIAL_LIF, DIV_CLK_VTS_SERIAL_LIF, "DOUT_DIV_CLK_VTS_SERIAL_LIF", 0, 0, NULL),
	VCLK(DOUT_DIV_CLK_VTS_CPU, DIV_CLK_VTS_CPU, "DOUT_DIV_CLK_VTS_CPU", 0, 0, NULL),
	VCLK(DOUT_DIV_CLKVTS_AUD_DMIC, DIV_CLK_VTS_AUD_DMIC, "DOUT_DIV_CLKVTS_AUD_DMIC", 0, 0, NULL),
};

struct init_vclk s5e9935_yuvp_hwacg_vclks[] = {
	HWACG_VCLK(GATE_YUVP_QCH, YUVP_QCH, "GATE_YUVP_QCH", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk s5e9935_cpucl0_glb_vclks[] = {
	VCLK(MOUT_CLKCMU_CPUCL0_DBG_NOC_USER, MUX_CLKCMU_CPUCL0_DBG_NOC_USER, "MOUT_CLKCMU_CPUCL0_DBG_NOC_USER", 0, 0, NULL),
};

struct init_vclk s5e9935_icpu_hwacg_vclks[] = {
	HWACG_VCLK(GATE_ICPU_QCH_CPU0, ICPU_QCH_CPU0, "GATE_ICPU_QCH_CPU0", NULL, 0, VCLK_GATE | VCLK_QCH_DIS, NULL),
	HWACG_VCLK(GATE_ICPU_QCH_PERI, ICPU_QCH_PERI, "GATE_ICPU_QCH_PERI", NULL, 0, VCLK_GATE | VCLK_QCH_DIS, NULL),
};

static struct init_vclk s5e9935_clkout_vclks[] = {
	VCLK(OSC_AUD, VCLK_CLKOUT0, "OSC_AUD", 0, 0, NULL),
};

static struct of_device_id ext_clk_match[] = {
	{.compatible = "samsung,s5e9935-oscclk", .data = (void *)0},
	{},
};

void s5e9935_vclk_init(void)
{
	/* Common clock init */
}

static int s5e9935_clock_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	void __iomem *reg_base;

	if (np) {
		reg_base = of_iomap(np, 0);
		if (!reg_base)
			panic("%s: failed to map registers\n", __func__);
	} else {
		panic("%s: unable to determine soc\n", __func__);
	}

	s5e9935_clk_provider = samsung_clk_init(np, reg_base, CLK_NR_CLKS);
	if (!s5e9935_clk_provider)
		panic("%s: unable to allocate context.\n", __func__);

	samsung_register_of_fixed_ext(s5e9935_clk_provider, s5e9935_fixed_rate_ext_clks,
					  ARRAY_SIZE(s5e9935_fixed_rate_ext_clks),
					  ext_clk_match);

	/* register HWACG vclk */
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_alive_hwacg_vclks, ARRAY_SIZE(s5e9935_alive_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_cmgp_hwacg_vclks, ARRAY_SIZE(s5e9935_cmgp_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_top_hwacg_vclks, ARRAY_SIZE(s5e9935_top_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_ufs_hwacg_vclks, ARRAY_SIZE(s5e9935_ufs_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_mif_hwacg_vclks, ARRAY_SIZE(s5e9935_mif_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_peric0_hwacg_vclks, ARRAY_SIZE(s5e9935_peric0_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_peric1_hwacg_vclks, ARRAY_SIZE(s5e9935_peric1_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_peric2_hwacg_vclks, ARRAY_SIZE(s5e9935_peric2_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_peris_hwacg_vclks, ARRAY_SIZE(s5e9935_peris_hwacg_vclks));

	/* register special vclk */
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_alive_vclks, ARRAY_SIZE(s5e9935_alive_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_cmgp_vclks, ARRAY_SIZE(s5e9935_cmgp_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_top_vclks, ARRAY_SIZE(s5e9935_top_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_peric0_vclks, ARRAY_SIZE(s5e9935_peric0_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_peric1_vclks, ARRAY_SIZE(s5e9935_peric1_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_peric2_vclks, ARRAY_SIZE(s5e9935_peric2_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_clkout_vclks, ARRAY_SIZE(s5e9935_clkout_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_ufs_vclks, ARRAY_SIZE(s5e9935_ufs_vclks));

	clk_exynos_skip_hw = true;
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_allcsis_hwacg_vclks, ARRAY_SIZE(s5e9935_allcsis_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_dpub_hwacg_vclks, ARRAY_SIZE(s5e9935_dpub_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_hsi0_hwacg_vclks, ARRAY_SIZE(s5e9935_hsi0_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_hsi1_hwacg_vclks, ARRAY_SIZE(s5e9935_hsi1_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_g3d_hwacg_vclks, ARRAY_SIZE(s5e9935_g3d_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_aud_hwacg_vclks, ARRAY_SIZE(s5e9935_aud_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_nocl1b_hwacg_vclks, ARRAY_SIZE(s5e9935_nocl1b_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_csis_hwacg_vclks, ARRAY_SIZE(s5e9935_csis_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_dnc_hwacg_vclks, ARRAY_SIZE(s5e9935_dnc_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_lme_hwacg_vclks, ARRAY_SIZE(s5e9935_lme_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_mfc_hwacg_vclks, ARRAY_SIZE(s5e9935_mfc_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_mfd_hwacg_vclks, ARRAY_SIZE(s5e9935_mfd_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_m2m_hwacg_vclks, ARRAY_SIZE(s5e9935_m2m_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_mcsc_hwacg_vclks, ARRAY_SIZE(s5e9935_mcsc_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_vts_hwacg_vclks, ARRAY_SIZE(s5e9935_vts_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_vts_vclks, ARRAY_SIZE(s5e9935_vts_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_yuvp_hwacg_vclks, ARRAY_SIZE(s5e9935_yuvp_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_dsp_hwacg_vclks, ARRAY_SIZE(s5e9935_dsp_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_sdma_hwacg_vclks, ARRAY_SIZE(s5e9935_sdma_hwacg_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_aud_vclks, ARRAY_SIZE(s5e9935_aud_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_cpucl0_glb_vclks, ARRAY_SIZE(s5e9935_cpucl0_glb_vclks));
	samsung_register_vclk(s5e9935_clk_provider, s5e9935_icpu_hwacg_vclks, ARRAY_SIZE(s5e9935_icpu_hwacg_vclks));
	clk_exynos_skip_hw = false;

	clk_register_fixed_factor(NULL, "pwm-clock", "fin_pll", CLK_SET_RATE_PARENT, 1, 1);

	samsung_clk_of_add_provider(np, s5e9935_clk_provider);

	s5e9935_vclk_init();

	pr_info("S5E9935: Clock setup completed\n");
	return 0;
}

static const struct of_device_id of_exynos_clock_match[] = {
	{ .compatible = "samsung,s5e9935-clock", },
	{ },
};
MODULE_DEVICE_TABLE(of, of_exynos_clock_match);

static const struct platform_device_id exynos_clock_ids[] = {
	{ "s5e9935-clock", },
	{ }
};

static struct platform_driver s5e9935_clock_driver = {
	.driver = {
		.name = "s5e9935_clock",
		.of_match_table = of_exynos_clock_match,
	},
	.probe		= s5e9935_clock_probe,
	.id_table	= exynos_clock_ids,
};

static int s5e9935_clock_init(void)
{
	return platform_driver_register(&s5e9935_clock_driver);
}
arch_initcall(s5e9935_clock_init);

static void s5e9935_clock_exit(void)
{
	return platform_driver_unregister(&s5e9935_clock_driver);
}
module_exit(s5e9935_clock_exit);

MODULE_LICENSE("GPL");

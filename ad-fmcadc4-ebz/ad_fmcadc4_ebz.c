/***************************************************************************//**
 *   @file   ad_fmcadc4_ebz.c
 *   @brief  Implementation of Main Function.
 *   @author DBogdan (dragos.bogdan@analog.com)
********************************************************************************
 * Copyright 2015(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "platform_drivers.h"
#include "ad9528.h"
#include "ad9680.h"
#include "adc_core.h"
#include "xcvr_core.h"
#include "jesd_core.h"
#include "dmac_core.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
#define GPIO_DEVICE_ID			XPAR_PS7_GPIO_0_DEVICE_ID
#define GPIO_AD9528_RSTN		32
#define GPIO_AD9528_STATUS		33
#define GPIO_AD9680_1_FDA		34
#define GPIO_AD9680_1_FDB		35
#define GPIO_AD9680_2_FDA		36
#define GPIO_AD9680_2_FDB		37


#define SPI_DEVICE_ID			XPAR_PS7_SPI_0_DEVICE_ID
#define ADC_DDR_BASEADDR		XPAR_DDR_MEM_BASEADDR + 0x800000
#define AD9680_CORE_0_BASEADDR		XPAR_AXI_AD9680_CORE_0_BASEADDR
#define AD9680_CORE_1_BASEADDR		XPAR_AXI_AD9680_CORE_1_BASEADDR
#define AD9680_DMA_BASEADDR		XPAR_AXI_AD9680_DMA_BASEADDR
#define AD9680_JESD_BASEADDR		XPAR_AXI_AD9680_JESD_BASEADDR
#define FMCADC4_GT_BASEADDR		XPAR_AXI_FMCADC4_GT_BASEADDR

/***************************************************************************//**
* @brief main
*******************************************************************************/
int main(void)
{

	spi_device ad9528_spi_device;
	spi_device ad9680_0_spi_device;
	spi_device ad9680_1_spi_device;
	ad9528_channel_spec ad9528_channels[6];
	ad9528_platform_data ad9528_param;
	ad9680_init_param ad9680_0_param;
	ad9680_init_param ad9680_1_param;
	jesd_core ad9680_jesd;
	xcvr_core ad9680_xcvr;
	adc_core ad9680_0_core;
	adc_core ad9680_1_core;
	dmac_core ad9680_dma;
	dmac_xfer rx_xfer;

	ad_spi_init(&ad9528_spi_device);
	ad_spi_init(&ad9680_0_spi_device);
	ad_spi_init(&ad9680_1_spi_device);

	ad9528_spi_device.chip_select = 0x0;
	ad9680_0_spi_device.chip_select = 0x1;
	ad9680_1_spi_device.chip_select = 0x2;


	// ad9528 defaults

	ad9528_param.num_channels = 6;
	ad9528_param.channels = &ad9528_channels[0];
	ad9528_init(&ad9528_param);

	// FPGA (500MHz)

	ad9528_channels[0].channel_num = 1;
	ad9528_channels[0].channel_divider = 2;

	// ADC2-sysref (31.25MHz)

	ad9528_channels[1].channel_num = 2;
	ad9528_channels[1].channel_divider = 32;

	// ADC2 (1000MHz)

	ad9528_channels[2].channel_num = 3;
	ad9528_channels[2].channel_divider = 1;

	// ADC1 (1000MHz)

	ad9528_channels[3].channel_num = 8;
	ad9528_channels[3].channel_divider = 1;

	// ADC1-sysref (31.25MHz)

	ad9528_channels[4].channel_num = 9;
	ad9528_channels[4].channel_divider = 32;

	// FPGA-sysref (31.25MHz)

	ad9528_channels[5].channel_num = 13;
	ad9528_channels[5].channel_divider = 32;

	// pllx settings

	ad9528_param.spi3wire = 1;
	ad9528_param.vcxo_freq = 80000000;

	/* REFA / REFB input configuration */
	ad9528_param.refa_en = 0;
	ad9528_param.refb_en = 0;

	/* Differential/ Single-Ended Input Configuration */
	ad9528_param.refa_diff_rcv_en = 1;
	ad9528_param.refb_diff_rcv_en = 0;
	ad9528_param.osc_in_diff_en = 0;

	ad9528_param.refa_cmos_neg_inp_en = 0;
	ad9528_param.refb_cmos_neg_inp_en = 0;
	ad9528_param.osc_in_cmos_neg_inp_en = 1;

	/* PLL1 Setting */
	ad9528_param.refa_r_div = 1;
	ad9528_param.refb_r_div = 1;
	ad9528_param.pll1_feedback_div = 8;
	ad9528_param.pll1_feedback_src_vcxo = 1;
	ad9528_param.pll1_charge_pump_current_nA = 10000;
	ad9528_param.pll1_bypass_en = 0;

	/* Reference */
	ad9528_param.ref_mode = REF_MODE_STAY_ON_REFB;
	ad9528_param.sysref_src = SYSREF_SRC_INTERNAL;
	ad9528_param.sysref_k_div = 20;

	/* PLL2 Setting */
	ad9528_param.pll2_charge_pump_current_nA = 806000;
	ad9528_param.pll2_ndiv_a_cnt = 0; // VCO CAL feedback divider
	ad9528_param.pll2_ndiv_b_cnt = 25; // VCO CAL feedback divider
	ad9528_param.pll2_freq_doubler_en = 0;
	ad9528_param.pll2_r1_div = 2;
	ad9528_param.pll2_n2_div = 25;
	ad9528_param.pll2_vco_diff_m1 = 4; /* 3..5 */

	/* Loop Filter PLL2 */
	ad9528_param.rpole2 = RPOLE2_900_OHM;
	ad9528_param.rzero= RZERO_3250_OHM;
	ad9528_param.cpole1 = CPOLE1_16_PF;
	ad9528_param.rzero_bypass_en = 0;

#ifdef MODE_1_24G
	ad9528_param.pll2_ndiv_a_cnt = 1;
	ad9528_param.pll2_ndiv_b_cnt = 23;
	ad9528_param.pll2_n2_div = 31;
	ad9528_param.pll2_vco_diff_m1 = 3;
#endif

	// adc 0 settings

	ad9680_0_param.lane_rate_kbps = 10000000;

	ad9680_xcvr.mmcm_present = 0;
	ad9680_xcvr.reconfig_bypass = 1;
	ad9680_xcvr.lane_rate_kbps = ad9680_0_param.lane_rate_kbps;

	ad9680_jesd.rx_tx_n = 1;
	ad9680_jesd.scramble_enable = 1;
	ad9680_jesd.octets_per_frame = 1;
	ad9680_jesd.frames_per_multiframe = 32;
	ad9680_jesd.subclass_mode = 1;

	ad9680_0_core.no_of_channels = 2;
	ad9680_0_core.resolution = 14;

	// adc 1 settings

	ad9680_1_param.lane_rate_kbps = ad9680_0_param.lane_rate_kbps;

	ad9680_1_core.no_of_channels = 2;
	ad9680_1_core.resolution = 14;

        // receiver DMA configuration

#ifdef ZYNQ
	rx_xfer.start_address = XPAR_DDR_MEM_BASEADDR + 0x800000;
#endif

	ad9680_dma.type 	= DMAC_RX;
	ad9680_dma.transfer 	= &rx_xfer;
	rx_xfer.id = 0;
	rx_xfer.size = 32768;

	// base addresses

#ifdef XILINX
	ad9680_xcvr.base_address = XPAR_AXI_AD9680_XCVR_BASEADDR;
	ad9680_jesd.base_address = XPAR_AXI_AD9680_JESD_BASEADDR;
	ad9680_dma.base_address = XPAR_AXI_AD9680_DMA_BASEADDR;
	ad9680_0_core.base_address = XPAR_AXI_AD9680_CORE_0_BASEADDR;
	ad9680_1_core.base_address = XPAR_AXI_AD9680_CORE_1_BASEADDR;
#endif

	// functions (do not modify below)

	ad_platform_init();

	ad_gpio_set(GPIO_AD9528_STATUS, 0x0);
	ad_gpio_set(GPIO_AD9528_RSTN, 0x0);
	mdelay(10);

	ad_gpio_set(GPIO_AD9528_RSTN, 0x1);
	mdelay(10);

	ad9528_setup(&ad9528_spi_device, &ad9528_param);

	ad9680_setup(&ad9680_0_spi_device, ad9680_0_param);
	ad9680_setup(&ad9680_1_spi_device, ad9680_1_param);
	jesd_setup(ad9680_jesd);
	xcvr_setup(ad9680_xcvr);
	jesd_status(ad9680_jesd);

	adc_setup(ad9680_0_core);
	ad9680_test(&ad9680_0_spi_device, AD9680_TEST_PN9);
	adc_pn_mon(ad9680_0_core, ADC_PN9);
	ad9680_test(&ad9680_0_spi_device, AD9680_TEST_PN23);
	adc_pn_mon(ad9680_0_core, ADC_PN23A);

	adc_setup(ad9680_1_core);
	ad9680_test(&ad9680_1_spi_device, AD9680_TEST_PN9);
	adc_pn_mon(ad9680_1_core, ADC_PN9);
	ad9680_test(&ad9680_1_spi_device, AD9680_TEST_PN23);
	adc_pn_mon(ad9680_1_core, ADC_PN23A);

	// ramp data

	ad9680_test(&ad9680_0_spi_device, AD9680_TEST_RAMP);
	ad9680_test(&ad9680_1_spi_device, AD9680_TEST_RAMP);

	if(!dmac_start_transaction(ad9680_dma)){
		adc_ramp_test(ad9680_0_core, 2, 32768, rx_xfer.start_address);
        };

	ad9680_test(&ad9680_1_spi_device, AD9680_TEST_OFF);
	if(!dmac_start_transaction(ad9680_dma)){
		ad_printf("RX capture done.\n");
        };

	ad_platform_close();

	return 0;
}

/*
 * Copyright (c) 2016, Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SOC_I2s_H_
#define SOC_I2s_H_

#include "data_type.h"
#include "soc_dma.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup common_drivers Common Drivers
 * Definition of the drivers APIs accessible from any processor.
 * @ingroup drivers
 */
    
/**
 * I2S audio formats.
 */
typedef enum {
    SOC_I2S_AUDIO_FORMAT_I2S_MODE = 0, /**< I2S Mode audio format */
    SOC_I2S_AUDIO_FORMAT_RIGHT_J,      /**< Right justified audio format */
    SOC_I2S_AUDIO_FORMAT_LEFT_J,       /**< Left justified audio format */
    SOC_I2S_AUDIO_FORMAT_DSP_MODE      /**< DSP mode audio format */
} soc_i2s_audio_format_t;

/**
 * I2S Audio sample resolution. Resolution ranges between 12 bits and 32 bits.
 */
typedef enum {
    /** 12 bits audio sample resolution */
    SOC_I2S_12_BIT_SAMPLE_RESOLUTION = 12,
    /** 13 bits audio sample resolution */
    SOC_I2S_13_BIT_SAMPLE_RESOLUTION = 13,
    /** 14 bits audio sample resolution */
    SOC_I2S_14_BIT_SAMPLE_RESOLUTION = 14,
    /** 15 bits audio sample resolution */
    SOC_I2S_15_BIT_SAMPLE_RESOLUTION = 15,
    /** 16 bits audio sample resolution */
    SOC_I2S_16_BIT_SAMPLE_RESOLUTION = 16,
    /** 17 bits audio sample resolution */
    SOC_I2S_17_BIT_SAMPLE_RESOLUTION = 17,
    /** 18 bits audio sample resolution */
    SOC_I2S_18_BIT_SAMPLE_RESOLUTION = 18,
    /** 19 bits audio sample resolution */
    SOC_I2S_19_BIT_SAMPLE_RESOLUTION = 19,
    /** 20 bits audio sample resolution */
    SOC_I2S_20_BIT_SAMPLE_RESOLUTION = 20,
    /** 21 bits audio sample resolution */
    SOC_I2S_21_BIT_SAMPLE_RESOLUTION = 21,
    /** 22 bits audio sample resolution */
    SOC_I2S_22_BIT_SAMPLE_RESOLUTION = 22,
    /** 23 bits audio sample resolution */
    SOC_I2S_23_BIT_SAMPLE_RESOLUTION = 23,
    /** 24 bits audio sample resolution */
    SOC_I2S_24_BIT_SAMPLE_RESOLUTION = 24,
    /** 25 bits audio sample resolution */
    SOC_I2S_25_BIT_SAMPLE_RESOLUTION = 25,
    /** 26 bits audio sample resolution */
    SOC_I2S_26_BIT_SAMPLE_RESOLUTION = 26,
    /** 27 bits audio sample resolution */
    SOC_I2S_27_BIT_SAMPLE_RESOLUTION = 27,
    /** 28 bits audio sample resolution */
    SOC_I2S_28_BIT_SAMPLE_RESOLUTION = 28,
    /** 29 bits audio sample resolution */
    SOC_I2S_29_BIT_SAMPLE_RESOLUTION = 29,
    /** 30 bits audio sample resolution */
    SOC_I2S_30_BIT_SAMPLE_RESOLUTION = 30,
    /** 31 bits audio sample resolution */
    SOC_I2S_31_BIT_SAMPLE_RESOLUTION = 31,
    /** 32 bits audio sample resolution */
    SOC_I2S_32_BIT_SAMPLE_RESOLUTION = 32
} soc_i2s_sample_resolution_t;

/**
 * I2S Audio rates available.
 */
typedef enum {
    SOC_I2S_RATE_4000KHZ = 0, /**< 4kHz audio rate */
    SOC_I2S_RATE_8000KHZ,     /**< 8kHz audio rate */
    SOC_I2S_RATE_11025KHZ,    /**< 11.025kHz audio rate */
    SOC_I2S_RATE_16000KHZ,    /**< 16kHz audio rate */
    SOC_I2S_RATE_22050KHZ,    /**< 22.05kHz audio rate */
    SOC_I2S_RATE_32000KHZ,    /**< 32kHz audio rate */
    SOC_I2S_RATE_44100KHZ,    /**< 44.1kHz audio rate */
    SOC_I2S_RATE_48000KHZ,    /**< 48kHz audio rate */
} soc_i2s_audio_rate_t;

/**
 * @defgroup soc_i2s Quark SE SOC I2S
 * Quark SE SOC I2S (Audio) drivers API.
 * @ingroup common_drivers
 * @{
 */

// I2S channels
#define I2S_CHANNEL_TX      0
#define I2S_CHANNEL_RX      1
#define I2S_NUM_CHANNELS    2

// I2S modes
#define I2S_MODE_SCK_POL    (0x01)
#define I2S_MODE_WS_POL     (0x02)
#define I2S_MODE_LR_ALIGN   (0x08)
#define I2S_MODE_SAMPLE_DEL (0x10)
#define I2S_MODE_WS_DSP     (0x20)

#define I2S_MODE_PHILLIPS   (I2S_MODE_SCK_POL | I2S_MODE_LR_ALIGN)
#define I2S_MODE_RJ         (I2S_MODE_SCK_POL | I2S_MODE_WS_POL | \
			     I2S_MODE_SAMPLE_DEL)
#define I2S_MODE_LJ         (I2S_MODE_SCK_POL | I2S_MODE_WS_POL | \
			     I2S_MODE_LR_ALIGN | I2S_MODE_SAMPLE_DEL)
#define I2S_MODE_DSP        (I2S_MODE_SCK_POL | I2S_MODE_LR_ALIGN | \
			     I2S_MODE_WS_DSP)

// I2S configuration object
struct soc_i2s_cfg {
	uint16_t sample_rate;     // in Hz
	uint8_t resolution;
	uint8_t mode;
	uint8_t master;

	void (*cb_done)(void *);
	void *cb_done_arg;

	void (*cb_err)(void *);
	void *cb_err_arg;
};

// Internal struct for use by the controller (and soc_config)
struct soc_i2s_info {
	uint32_t reg_base;
	uint32_t int_vector;
	uint32_t int_mask;

	uint32_t clk_speed;

	struct soc_i2s_cfg cfg[I2S_NUM_CHANNELS];
	uint8_t en[I2S_NUM_CHANNELS];
	uint8_t cfgd[I2S_NUM_CHANNELS];

	struct soc_dma_cfg dma_cfg[I2S_NUM_CHANNELS];
	struct soc_dma_channel dma_ch[I2S_NUM_CHANNELS];

	struct clk_gate_info_s *clk_gate_info;
};

extern struct driver soc_i2s_driver;

/**
 *  Function to configure specified I2S channel
 *
 *  Configuration parameters must be valid or an error is returned - see return values below.
 *
 *  @param   channel         : I2S channel number
 *  @param   cfg             : pointer to configuration structure
 *
 *  @return
 *           - DRV_RC_OK on success
 *           - DRV_RC_INVALID_CONFIG               - if any configuration parameters are not valid
 *           - DRV_RC_CONTROLLER_IN_USE,           - if controller is in use
 *           - DRV_RC_FAIL otherwise
 */
DRIVER_API_RC soc_i2s_config(uint8_t channel, struct soc_i2s_cfg *cfg);

/**
 *  Function to deconfigure specified I2S channel
 *
 *  @param   channel         : I2S channel number
 *
 *  @return
 *           - DRV_RC_OK on success
 *           - DRV_RC_CONTROLLER_IN_USE,           - if controller is in use
 *           - DRV_RC_FAIL otherwise
 */
DRIVER_API_RC soc_i2s_deconfig(uint8_t channel);

/**
 *  Function to transmit a block of audio data
 *
 *  @param   buf             : pointer to data to write
 *  @param   len             : length of data to write (in bytes)
 *
 *  @return
 *           - DRV_RC_OK on success
 *           - DRV_RC_FAIL otherwise
 */
DRIVER_API_RC soc_i2s_write(void *buf, uint32_t len, uint32_t len_per_data);

/**
 *  Function to continuously transmit blocks of audio data
 *
 *  @param   buf             : pointer to buffer to read data from
 *  @param   len             : length of buffer (in bytes)
 *  @param   num_bufs        : number of parts into which to split the buffer; calling the callback
 *                             after each is sent
 *
 *  @return
 *           - DRV_RC_OK on success
 *           - DRV_RC_FAIL otherwise
 */
DRIVER_API_RC soc_i2s_stream(void *buf, uint32_t len, uint32_t len_per_data, uint32_t num_bufs);

/**
 *  Function to stop a continuous audio data write
 *
 *  @return
 *           - DRV_RC_OK on success
 *           - DRV_RC_FAIL otherwise
 */
DRIVER_API_RC soc_i2s_stop_stream(void);

/**
 *  Function to receive a block of audio data
 *
 *  @param   buf             : pointer to buffer to fill with data
 *  @param   len             : length of buffer (in bytes)
 *
 *  @return
 *           - DRV_RC_OK on success
 *           - DRV_RC_FAIL otherwise
 */
DRIVER_API_RC soc_i2s_read(void *buf, uint32_t len, uint32_t len_per_data);

/**
 *  Function to continuously receive blocks of audio data
 *
 *  @param   buf             : pointer to buffer to fill with data
 *  @param   len             : length of buffer (in bytes)
 *  @param   num_bufs        : number of parts into which to split the buffer; calling the callback
 *                             after each is filled
 *
 *  @return
 *           - DRV_RC_OK on success
 *           - DRV_RC_FAIL otherwise
 */
DRIVER_API_RC soc_i2s_listen(void *buf, uint32_t len,  uint32_t len_per_data, uint8_t num_bufs);

/**
 *  Function to stop a continuous audio data read
 *
 *  @return
 *           - DRV_RC_OK on success
 *           - DRV_RC_FAIL otherwise
 */
DRIVER_API_RC soc_i2s_stop_listen(void);

DRIVER_API_RC soc_i2s_init();

uint32_t soc_i2s_get_rx_receive_address();
uint32_t soc_i2s_get_tx_transmit_address();

/** @} */

#ifdef __cplusplus
}
#endif

#endif

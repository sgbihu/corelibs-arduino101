/*
  Copyright (c) 2016 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>
#include "I2S.h"

#include "infra/log.h"

//I2S Arduino Pins
#define I2S_TXD     7
#define I2S_TWS     4
#define I2S_TSCK    2
#define I2S_RXD     5
#define I2S_RWS     3
#define I2S_RSCK    8

int I2SClass::_beginCount = 0;

I2SClass::I2SClass() :
  _state(I2S_STATE_IDLE),
  _bitsPerSample(32),
  _onTransmit(NULL),
  _onReceive(NULL)
{
    _transmitter_write_addr = (uint32_t) _dataBuff[0];
    _recevicer_read_addr  = (uint32_t) _dataBuff[0];
    memset(&_txcfg, 0, sizeof(_txcfg));
    memset(&_rxcfg, 0, sizeof(_rxcfg));
}

int I2SClass::begin(int mode, long sampleRate, int bitsPerSample)
{
    // master mode (driving clock and frame select pins - output)
    return begin(mode, sampleRate, bitsPerSample, true);
}

int I2SClass::begin(int mode, int bitsPerSample)
{
    // slave mode (not driving clock and frame select pin - input)
    return begin(mode, 0, bitsPerSample, false);
}

int I2SClass::begin(int mode, long sampleRate, int bitsPerSample, bool driveClock)
{
    if (_state != I2S_STATE_IDLE)
    {
        return 0;
    }
    uint8_t i2s_mode = I2S_MODE_PHILLIPS;
    switch (mode) 
    {
        case I2S_PHILIPS_MODE:
            i2s_mode = I2S_MODE_PHILLIPS;
            break;
        case I2S_RIGHT_JUSTIFIED_MODE:
            i2s_mode = I2S_MODE_RJ;
            break;
        case I2S_LEFT_JUSTIFIED_MODE:
            i2s_mode = I2S_MODE_LJ;
            break;
        break;

        default:
        // invalid mode
        return 0;
    }

    switch (bitsPerSample) 
    {
        case 8:
        case 16:
        case 32:
            _bitsPerSample = bitsPerSample;
            break;

        default:
            // invalid bits per sample
            return 0;
    }


    if (_beginCount == 0) 
    {
        initPinMux();
        soc_i2s_init();
        soc_dma_init();
    }
    
    setupReceiver(sampleRate, i2s_mode, 1);
    setupTransmitter(sampleRate, i2s_mode, 1);
    
    _beginCount++;

    return 1;
}

void I2SClass::setupReceiver(uint16_t sampleRate, uint8_t mode, uint8_t master_slave)
{
    _rxcfg.sample_rate = sampleRate;
    _rxcfg.resolution = _bitsPerSample;
    _rxcfg.mode = mode;
    _rxcfg.master = master_slave;
    _rxcfg.cb_done = onReceiverComplete;
    _rxcfg.cb_done_arg = this;
    _rxcfg.cb_err = onReceiverError; 
    _rxcfg.cb_err_arg = this;
    
    soc_i2s_config(I2S_CHANNEL_RX, &_rxcfg);
}

void I2SClass::setupTransmitter(uint16_t sampleRate, uint8_t mode, uint8_t master_slave)
{
    _txcfg.sample_rate = sampleRate;
    _txcfg.resolution = _bitsPerSample;
    _txcfg.mode = mode;
    _txcfg.master = master_slave;
    _txcfg.cb_done = onTransmitterComplete;
    _txcfg.cb_done_arg = this;
    _txcfg.cb_err = onTransmitterError;
    _txcfg.cb_err_arg = this;
    
    soc_i2s_config(I2S_CHANNEL_TX, &_txcfg);
}

void I2SClass::end()
{
    _beginCount--;

    if (_beginCount == 0) 
    {
        disableTransmitter();
        disableReceiver();
    }
}

int I2SClass::available()
{
    if ((_state & I2S_STATE_RECEIVER) == 0) 
    {
        enableReceiver();
    }
    
    size_t avail;
    uint32_t rx_address = soc_i2s_get_rx_receive_address();
    
    unsigned int flag = interrupt_lock();
    
    uint32_t read_index = addressToIndex((uint32_t)_dataBuff, 
                                          sizeof(_dataBuff[0]),
                                          _recevicer_read_addr);
    uint32_t start_address = (uint32_t)(_dataBuff[read_index]);
    uint32_t offset = rx_address - start_address;
    if (start_address < _recevicer_read_addr)
    {
        start_address += sizeof(_dataBuff);
    }
    offset += start_address - _recevicer_read_addr;
    interrupt_unlock(flag);
    avail = offset / (_bitsPerSample / 8);
    return avail;
}

union i2s_sample_t {
  uint8_t b8;
  int16_t b16;
  int32_t b32;
};

int I2SClass::read()
{
  i2s_sample_t sample;

  sample.b32 = 0;

  read(&sample, _bitsPerSample / 8);

  if (_bitsPerSample == 32) {
    return sample.b32;
  } else if (_bitsPerSample == 16) {
    return sample.b16;
  } else if (_bitsPerSample == 8) {
    return sample.b8;
  } else {
    return 0;
  }
}
/*
void I2SClass::copyMemToDma(void* dst, const void* src, size_t size)
{
    uint32_t* dma_addr = (uint32_t*)dst;
    int persize = _bitsPerSample / 8;
    int copy_len = size / persize;
    
    for (int i = 0; i < copy_len; i++)
    {
        memcpy(dma_addr, src, persize);
        dma_addr++;
        src = (uint8_t *)src + persize;
    }
}

void I2SClass::copyDmaToMem(void* dst, const void* src, size_t size)
{
    uint32_t* dma_addr = (uint32_t*)src;
    int persize = _bitsPerSample / 8;
    int copy_len = size / persize;
    
    for (int i = 0; i < copy_len; i++)
    {
        memcpy(dst, dma_addr, persize);
        dma_addr++;
        dst = (uint8_t *)dst + persize;
    }
}*/

int I2SClass::read(void* buffer, size_t size)
{
    if ((_state & I2S_STATE_RECEIVER) == 0)
    {
        enableReceiver();
    }
    unsigned int flag = interrupt_lock();
    uint32_t* start_addr = (uint32_t*)_recevicer_read_addr;
    uint32_t latest_addr = soc_i2s_get_rx_receive_address();
    uint32_t buffer_end_addr = sizeof(_dataBuff) + (uint32_t)_dataBuff[0];
    uint32_t available_len = available();
    uint32_t read_len = 0;
    if (size > available_len)
    {
        size = available_len;
    }
    
    if (_recevicer_read_addr > latest_addr)
    {
        uint32_t reserved_len = buffer_end_addr - _recevicer_read_addr;
        if (reserved_len < size)
        {
            memcpy(buffer, start_addr, reserved_len);
            size -= reserved_len;
            buffer = (void *)((uint32_t)buffer + reserved_len);
            _recevicer_read_addr = (uint32_t)_dataBuff[0];
            start_addr = (uint32_t*)_recevicer_read_addr;
            read_len += reserved_len;
        }
    }
    memcpy(buffer, start_addr, size);
    _recevicer_read_addr += size;
    read_len += size;
    if (_recevicer_read_addr >= buffer_end_addr)
    {
        _recevicer_read_addr = (uint32_t)_dataBuff[0];
    }
    interrupt_unlock(flag);
    
    return (int)read_len;
}

int I2SClass::peek()
{
    i2s_sample_t sample;

    sample.b32 = 0;

    unsigned int flag = interrupt_lock();
    uint32_t* start_addr = (uint32_t*)_recevicer_read_addr;
    memcpy(&sample, start_addr, _bitsPerSample / 8);
    interrupt_unlock(flag);

    if (_bitsPerSample == 32) 
    {
        return sample.b32;
    }
    else if (_bitsPerSample == 16)
    {
        return sample.b16;
    }
    else if (_bitsPerSample == 8)
    {
        return sample.b8;
    }
    else
    {
        return 0;
    }
}

void I2SClass::flush()
{
  // do nothing, writes are DMA triggered
}

size_t I2SClass::write(uint8_t data)
{
  return write((int32_t)data);
}

size_t I2SClass::write(const uint8_t *buffer, size_t size)
{
  return write((const void*)buffer, size);
}

size_t I2SClass::availableForWrite()
{
    size_t avail;
    uint32_t tx_address = soc_i2s_get_tx_transmit_address();
    if (0 == tx_address)
    {
        // The I2S not started and clear it.
        memset(_dataBuff, 0, sizeof(_dataBuff));
        tx_address = (uint32_t) _dataBuff[0];
    }
    unsigned int flag = interrupt_lock();
    
    uint32_t read_index = addressToIndex((uint32_t)_dataBuff, 
                                          sizeof(_dataBuff[0]),
                                          tx_address);
    
    //pr_debug(LOG_MODULE_I2S, "read_index: %d", 
    //        read_index);
    uint32_t start_address = (uint32_t)(_dataBuff[read_index]);
    uint32_t offset = tx_address - start_address;
    if (start_address < _transmitter_write_addr)
    {
        start_address += sizeof(_dataBuff);
    }
    offset += start_address - _transmitter_write_addr;
    interrupt_unlock(flag);
    avail = offset;

    return avail;
}

size_t I2SClass::write(int sample)
{
  return write((int32_t)sample);
}

size_t I2SClass::write(int32_t sample)
{
    unsigned int flag = interrupt_lock();
    uint32_t* start_addr = (uint32_t*)_transmitter_write_addr;
    memcpy(start_addr, &sample, sizeof (sample));
    _transmitter_write_addr += sizeof (sample);
    uint32_t buffer_end_addr = sizeof(_dataBuff) + (uint32_t)_dataBuff[0];
    if (_transmitter_write_addr >= buffer_end_addr)
    {
        _transmitter_write_addr = (uint32_t)_dataBuff[0];
    }
    interrupt_unlock(flag);

    if ((_state & I2S_STATE_TRANSMITTER) == 0) 
    {
        enableTransmitter();
    }
    
    return 1;
}

size_t I2SClass::write(const void *buffer, size_t size)
{
    #if 0
    pr_debug(LOG_MODULE_I2S, "Write addr: 0x%X, start: 0x%p, trans size: %d", 
            _transmitter_write_addr,
            _dataBuff, size);
    
    #endif
    unsigned int flag = interrupt_lock();
    _transmitter_write_addr += 4;
    _transmitter_write_addr &= ~0x07;
    
    uint32_t* start_addr = (uint32_t*)_transmitter_write_addr;
    uint32_t latest_addr = soc_i2s_get_tx_transmit_address();
    uint32_t buffer_end_addr = sizeof(_dataBuff) + (uint32_t)_dataBuff[0];
    uint32_t available_len = availableForWrite();
    uint32_t write_len = 0;
    if (size > available_len)
    {
        size = available_len;
    }
    
    if (_transmitter_write_addr > latest_addr)
    {
        uint32_t reserved_len = buffer_end_addr - _transmitter_write_addr;
        if (reserved_len < size)
        {
            memcpy(start_addr, buffer, reserved_len);
            size -= reserved_len;
            buffer = (void *)((uint32_t)buffer + reserved_len);
            _transmitter_write_addr = (uint32_t)_dataBuff[0];
            start_addr = (uint32_t*)_transmitter_write_addr;
            write_len += reserved_len;
        }
    }
    memcpy(start_addr, buffer, size);
    _transmitter_write_addr += size;
    write_len += size;
    if (_transmitter_write_addr >= buffer_end_addr)
    {
        _transmitter_write_addr = (uint32_t)_dataBuff[0];
    }
    interrupt_unlock(flag);
    
    if ((_state & I2S_STATE_TRANSMITTER) == 0) 
    {
        enableTransmitter();
    }
    return write_len;
}

void I2SClass::onTransmit(void(*function)(void))
{
    _onTransmit = function;
}

void I2SClass::onReceive(void(*function)(void))
{
    _onReceive = function;
}

void I2SClass::enableTransmitter()
{
    if (_state & I2S_STATE_TRANSMITTER)
    {
        return;
    }
    int status = soc_i2s_stream(_dataBuff, sizeof(_dataBuff), sizeof(uint32_t), 2); 
    if (status)
    {
        pr_debug(LOG_MODULE_I2S, "Start TX DMA failed [%d]", status);
        while(1){}
    }
    _transmitter_write_addr = (uint32_t) _dataBuff[1];
    _state |= I2S_STATE_TRANSMITTER;
}

void I2SClass::disableTransmitter()
{
    if ((_state & I2S_STATE_TRANSMITTER) == 0)
    {
        return;
    }
    int status = soc_i2s_stop_stream(); 
    if (status)
    {
        pr_debug(LOG_MODULE_I2S, "Stop TX DMA failed [%d]", status);
        while(1){}
    }
    _transmitter_write_addr = (uint32_t) _dataBuff[0];
    _state &= I2S_STATE_TRANSMITTER;
}

void I2SClass::enableReceiver()
{
    if (_state & I2S_STATE_RECEIVER)
    {
        return;
    }
    int status = soc_i2s_listen(_dataBuff, sizeof(_dataBuff), sizeof(uint32_t), 2); 
    if(status)
    {
        pr_debug(LOG_MODULE_I2S, "Start RX DMA failed [%d]", status);
        while(1){}
    }
    _recevicer_read_addr = (uint32_t) _dataBuff[0];
    _state |= I2S_STATE_RECEIVER;
}

void I2SClass::disableReceiver()
{
    if ((_state & I2S_STATE_RECEIVER) == 0)
    {
        return;
    }
    int status = soc_i2s_stop_listen(); 
    if(status)
    {
        pr_debug(LOG_MODULE_I2S, "Stop RX DMA failed [%d]", status);
        while(1){}
    }
    _state &= I2S_STATE_RECEIVER;
}

void I2SClass::initPinMux()
{
    int mux_mode = I2S_MUX_MODE;

    /* Set SoC pin mux configuration */
    SET_PIN_MODE(g_APinDescription[I2S_TXD].ulSocPin, mux_mode);
    SET_PIN_MODE(g_APinDescription[I2S_TWS].ulSocPin, mux_mode);
    SET_PIN_MODE(g_APinDescription[I2S_TSCK].ulSocPin, mux_mode);
    
    SET_PIN_MODE(49, mux_mode); //I2S_RXD
    SET_PIN_MODE(51, mux_mode); //I2S_RWS
    SET_PIN_MODE(50,  mux_mode); //I2S_RSCK
}

uint32_t I2SClass::addressToIndex(uint32_t start_addr,
                                  uint32_t item_size, 
                                  uint32_t address)
{
    // Address to index
    uint32_t offset = address - start_addr + item_size;
    offset /= item_size;
    offset--;
    return offset;
}

void I2SClass::onReceiverComplete(void* x)
{
    I2SClass* i2sObj = (I2SClass*) x;
    
    uint32_t write_index = i2sObj->addressToIndex((uint32_t)i2sObj->_dataBuff, 
                                                  sizeof(i2sObj->_dataBuff[0]),
                                                  soc_i2s_get_rx_receive_address());
    uint32_t read_index = i2sObj->addressToIndex((uint32_t)i2sObj->_dataBuff, 
                                                  sizeof(i2sObj->_dataBuff[0]),
                                                  i2sObj->_recevicer_read_addr);
    if (read_index == write_index)
    {
        read_index++;
        read_index %= ARRAY_SIZE(i2sObj->_dataBuff);
        i2sObj->_recevicer_read_addr = (uint32_t)i2sObj->_dataBuff[read_index];
    }
    
    // Callback
    if (i2sObj->_onReceive)
    {
        i2sObj->_onReceive();
    }
}

void I2SClass::onReceiverError(void* x)
{
    //I2SClass* i2sObj = (I2SClass*) x;
    UNUSED(x);
}

void I2SClass::onTransmitterComplete(void* x)
{
    I2SClass* i2sObj = (I2SClass*) x;
    
    uint32_t read_index = i2sObj->addressToIndex((uint32_t)i2sObj->_dataBuff, 
                                                  sizeof(i2sObj->_dataBuff[0]),
                                                  soc_i2s_get_tx_transmit_address());
    uint32_t write_index = i2sObj->addressToIndex((uint32_t)i2sObj->_dataBuff, 
                                                  sizeof(i2sObj->_dataBuff[0]),
                                                  i2sObj->_transmitter_write_addr);
    if (read_index == write_index)
    {
        write_index++;
        write_index %= ARRAY_SIZE(i2sObj->_dataBuff);
        i2sObj->_transmitter_write_addr = (uint32_t)i2sObj->_dataBuff[write_index];
    }
    //pr_debug(LOG_MODULE_I2S, "DMA current addr: 0x%X, start: 0x%p", 
    //        soc_i2s_get_tx_transmit_address(),
    //        i2sObj->_dataBuff);
    if (i2sObj->_onTransmit)
    {
        i2sObj->_onTransmit();
    }
}

void I2SClass::onTransmitterError(void* x)
{
    //I2SClass* i2sObj = (I2SClass*) x;
    UNUSED(x);
}

//#if I2S_INTERFACES_COUNT > 0
I2SClass I2S;
//#endif

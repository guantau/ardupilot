
#include <AP_HAL/AP_HAL.h>

#if CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_DM1

#include "RCOutput_DM1.h"

#include <cmath>
#include <dirent.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "pak_stm32.h"
#include "GPIO.h"

using namespace Linux;

#define PWM_CHAN_COUNT 4

static const AP_HAL::HAL& hal = AP_HAL::get_HAL();

void RCOutput_DM1::init()
{
		_frequency = 400;
    _dev = hal.spi->get_device("stm32");
    hal.scheduler->register_timer_process(FUNCTOR_BIND_MEMBER(&RCOutput_DM1::_update, void));
}

void RCOutput_DM1::set_freq(uint32_t chmask, uint16_t freq_hz)
{
		_frequency = freq_hz;
}

uint16_t RCOutput_DM1::get_freq(uint8_t ch)
{
    return _frequency;
}

void RCOutput_DM1::enable_ch(uint8_t ch)
{

}

void RCOutput_DM1::disable_ch(uint8_t ch)
{
    write(ch, 0);
}

void RCOutput_DM1::write(uint8_t ch, uint16_t period_us)
{
    if(ch >= PWM_CHAN_COUNT){
        return;
    }

    _period_us[ch] = period_us;
}

void RCOutput_DM1::write(uint8_t ch, uint16_t* period_us, uint8_t len)
{
    for (int i = 0; i < len; i++)
        write(ch + i, period_us[i]);
}

uint16_t RCOutput_DM1::read(uint8_t ch)
{
    if(ch >= PWM_CHAN_COUNT){
        return 0;
    }

    return _period_us[ch];
}

void RCOutput_DM1::read(uint16_t* period_us, uint8_t len)
{
    for (int i = 0; i < len; i++)
        period_us[i] = read(0 + i);
}

void RCOutput_DM1::_update(void)
{
    int i;

    if (AP_HAL::micros() - _last_update_timestamp < 10000) {
        return;
    }

    _last_update_timestamp = AP_HAL::micros();

    if (!_dev->get_semaphore()->take_nonblocking()) {
        return;
    }

    struct IOPacket _dma_packet_tx, _dma_packet_rx;

    _dma_packet_tx.option = 1;

    for (i=0; i<PWM_CHAN_COUNT; i++) {
        _dma_packet_tx.regs[i] = _period_us[i];
    }

    _dev->transfer((uint8_t *)&_dma_packet_tx, 20, (uint8_t *)&_dma_packet_rx, 20);
    _dev->get_semaphore()->give();
}

#endif

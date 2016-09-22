#include <AP_HAL/AP_HAL.h>

#if CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_DM1

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "RCInput_DM1.h"
#include "pak_stm32.h"

static const AP_HAL::HAL& hal = AP_HAL::get_HAL();

using namespace Linux;

void RCInput_DM1::init()
{
    _dev = hal.spi->get_device("stm32");

    // start the timer process to read samples
    hal.scheduler->register_timer_process(FUNCTOR_BIND_MEMBER(&RCInput_DM1::_poll_data, void));
}

void RCInput_DM1::_poll_data(void)
{
    // Throttle read rate to 100hz maximum.
    if (AP_HAL::micros() - _last_timer < 10000) {
        return;
    }

    _last_timer = AP_HAL::micros();

    if (!_dev->get_semaphore()->take_nonblocking()) {
        return;
    }

    struct IOPacket _dma_packet_tx, _dma_packet_rx;
    _dma_packet_tx.option = 2;

    _dev->transfer((uint8_t *)&_dma_packet_tx, 20, (uint8_t *)&_dma_packet_rx, 20);
    _update_periods(&_dma_packet_rx.regs[0], 4);
    _dev->get_semaphore()->give();
}

#endif

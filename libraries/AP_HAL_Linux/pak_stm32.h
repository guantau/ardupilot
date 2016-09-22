#include <inttypes.h>

struct IOPacket {
	uint16_t option;
	uint16_t regs[9];
};

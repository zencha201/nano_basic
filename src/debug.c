#include "nano_basic_local.h"
#ifdef __DEBUG__

void show_buf(const NB_I8 *buf, NB_SIZE size)
{
    NB_SIZE i = 0;
    for( i =  0 ; i < size ; i ++) {
        LOG("%02X ", (NB_U8)buf[i]);
        if((i + 1) % 16 == 0) {
            LOG("\r\n");
        }
    }
    LOG("\r\n");
}

#endif /* __DEBUG__ */

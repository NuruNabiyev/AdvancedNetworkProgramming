#ifndef ANP_DEV_MANAGEMENT_H
#define ANP_DEV_MANAGEMENT_H

#include "systems_headers.h"

struct tap_netdev {
    // tun device file descriptor
    int tun_fd;
    // device name
    char *devname;
};


char *get_tdev_name();
void tdev_init(void);
int tdev_read(char *buf, int len);
int tdev_write(char *buf, int len);

#endif // ANP_DEV_MANAGEMENT_H

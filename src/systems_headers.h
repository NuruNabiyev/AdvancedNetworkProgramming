#ifndef ANP_SYSTEMS_HEADERS_H
#define ANP_SYSTEMS_HEADERS_H

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>

// just a skeleton file to include all the basic systems headers in the code
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include <pthread.h>
#include <time.h>
#include <poll.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/capability.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <linux/if.h>
#include <linux/if_tun.h>

#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

#endif //ANP_SYSTEMS_HEADERS_H

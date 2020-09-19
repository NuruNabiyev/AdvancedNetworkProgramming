#ifndef ATR_TUNTAP_UTILS_H
#define ATR_TUNTAP_UTILS_H

#include "systems_headers.h"
#include <execinfo.h>

#define _clear_var(addr) memset(&(addr), 0, sizeof(addr))

#define CMDBUFLEN 128

int      run_bash_command(char *cmd, ...);
uint16_t do_csum(void *addr, int count, int start_sum);
uint32_t ip_str_to_n32(const char *addr);
uint32_t ip_str_to_h32(const char *addr);
void     u32_ip_to_str(char *, uint32_t daddr);
void     print_trace(void);
int      do_tcp_csum(uint8_t *data, int length, uint16_t protocol, uint32_t saddr, uint32_t daddr);

#define ANP_MIN(a, b) (a < b ? a : b)

#endif //ATR_TUNTAP_UTILS_H

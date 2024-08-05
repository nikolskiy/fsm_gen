#include "protocol.h"
#include "uart.h"
#include <stdio.h>

int parse_rply(const char* str, uint8_t* rt, uint32_t* rv)
{
  // str = "TTTT:VVVV:S\0";
  // T - number
  // VVVV - number in dec ascii
  // S - checksum: (int(T) + int(VVVV)) mod 10 + '0'
  *rt = 0;
  while (*str && *str != ':') {
    *rt *= 10;
    *rt += (*str++) - '0';
  }
  if (*str!=':') return 0;
  ++str;
  *rv = 0;
  while (*str && *str != ':') {
    *rv *= 10;
    *rv += (*str++) - '0';
  }
  if (*str!=':') return 0;
  ++str;
  if (*str - '0' != (*rt + *rv) %10) return 0;
  ++str;
  if (*str != 0) return 0;
  return 1;
}

void send_req(uint8_t req, uint32_t val)
{
  // RRR:VVV:S\n
  int cs = (req + val) % 10;
  char str[16];
  char *p=str, *p1=str, *r=str;
  do {
    *p++ = (req % 10) + '0';
    req /= 10;
  } while(req);
  r = p;
  p--;
  while( p - p1 > 0) {
    *p1 ^= *p;
    *p ^= *p1;
    *p1++ ^= *p--;
  }
  *r++ = ':';
  p1 = r;
  p=r;
  do {
    *p++ = (val % 10) + '0';
    val /= 10;
  } while(val);
  r = p;
  p--;
  while(p - p1 > 0) {
    *p1 ^= *p;
    *p ^= *p1;
    *p1++ ^= *p--;
  }
 *r++ = ':';
 *r++ = cs + '0';
 *r++ = '\n';
 *r = 0;
 uart_send(str, (((uint32_t) r) - ((uint32_t) str)));
}

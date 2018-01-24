#ifndef ZROUTE_H
#define ZROUTE_H

#include <quagga/config.h>
#include <quagga/thread.h>
#include <quagga/zclient.h>
#include <quagga/prefix.h>

int zroute_start ();

void zroute_stop ();

int zroute_route_to_host ( u_char cmd, struct in_addr*, const char* );

#endif

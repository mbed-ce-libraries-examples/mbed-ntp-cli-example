#pragma once

#include <mbed.h>
#include <NetworkInterface.h>

/// Global network interface for the rest of the code to use
extern NetworkInterface & netInterface;

#define ETHERNET 1
#define WIFI 2

// Command functions
#if MBED_CONF_TARGET_NETWORK_DEFAULT_INTERFACE_TYPE == WIFI
int wifi_scan(int argc, char *argv[]);
int wifi_connect(int argc, char *argv[]);
#elif MBED_CONF_TARGET_NETWORK_DEFAULT_INTERFACE_TYPE == ETHERNET
int eth_connect(int argc, char *argv[]);
#endif
int ipconfig(int argc, char *argv[]);
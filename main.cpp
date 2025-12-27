/*
 * Copyright (c) 2018-2019, Pelion and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cstdio>
#include <cstring>

#include "mbed.h"
#include "mbed-trace/mbed_trace.h"
#include "mbed-client-cli/ns_cmdline.h"

#include <EthernetInterface.h>
#include <WiFiInterface.h>

#define TRACE_GROUP   "main"

static Mutex SerialOutMutex;
void serial_out_mutex_wait()
{
    SerialOutMutex.lock();
}
void serial_out_mutex_release()
{
    SerialOutMutex.unlock();
}

// Network interface definition
#define ETHERNET 1
#define WIFI 2
#if MBED_CONF_TARGET_NETWORK_DEFAULT_INTERFACE_TYPE == ETHERNET
EthernetInterface netInterface;
#elif MBED_CONF_TARGET_NETWORK_DEFAULT_INTERFACE_TYPE == WIFI
WiFiInterface & netInterface = *WiFiInterface::get_default_instance();
#endif

#if MBED_CONF_TARGET_NETWORK_DEFAULT_INTERFACE_TYPE == WIFI
static int wifi_scan(int argc, char *argv[])
{
    constexpr size_t maxNetworks = 15; // each network uses 48 bytes RAM
    std::vector<WiFiAccessPoint> scannedAPs(maxNetworks);

    printf("Scanning for Wi-Fi networks...\n");
    auto ret = netInterface.scan(scannedAPs.data(), maxNetworks);
    if(ret < 0) {
        tr_error("Error performing wifi scan: %d", ret);
        return CMDLINE_RETCODE_FAIL;
    }

    if(ret == 0) {
        printf("No networks detected.\n");
    }
    else {
        printf("Detected %d access points: \n", ret);

        // Shrink the vector to fit the number of networks actually seen, then sort by RSSI from high
        // to low.
        scannedAPs.resize(ret);
        auto comparator = [](WiFiAccessPoint const & lhs, WiFiAccessPoint const & rhs) { return lhs.get_rssi() > rhs.get_rssi(); };
        std::sort(scannedAPs.begin(), scannedAPs.end(), comparator);

        for (WiFiAccessPoint& ap : scannedAPs) {
            printf("- SSID: \"%s\", security: %s, RSSI: %" PRIi8 " dBm, Ch: %" PRIu8 "\n", ap.get_ssid(),
               nsapi_security_to_string(ap.get_security()), ap.get_rssi(), ap.get_channel());
        }
    }

    return CMDLINE_RETCODE_SUCCESS;
}

static int wifi_connect_dhcp(int argc, char *argv[]) {
    // Validate arguments
    if (argc < 3) {
        return CMDLINE_RETCODE_INVALID_PARAMETERS;
    }
    const char* ssid = argv[1];
    const auto security = nsapi_string_to_security(argv[2]);
    char* password = nullptr;
    if (security == NSAPI_SECURITY_UNKNOWN) {
        // Invalid security value
        return CMDLINE_RETCODE_INVALID_PARAMETERS;
    }
    else if (security == NSAPI_SECURITY_NONE) {
        // No password parameter
        if (argc != 3) {
            return CMDLINE_RETCODE_INVALID_PARAMETERS;
        }
    }
    else {
        // Password parameter needed
        if (argc != 4) {
            return CMDLINE_RETCODE_INVALID_PARAMETERS;
        }
        password = argv[3];
    }

    if (const auto ret = netInterface.set_credentials(ssid, password, security); ret != NSAPI_ERROR_OK) {
        tr_error("Error setting Wi-Fi credentials: %d", ret);
        return CMDLINE_RETCODE_INVALID_PARAMETERS;
    }

    printf("Connecting to the Wi-Fi network...\n");
    if (const auto ret = netInterface.connect(); ret != NSAPI_ERROR_OK) {
        tr_error("Error setting Wi-Fi credentials: %d", ret);
        return CMDLINE_RETCODE_FAIL;
    }
    printf("Connected to \"%s\".\n", ssid);
    return CMDLINE_RETCODE_SUCCESS;
}
#endif

int main(void)
{
    // Initialize trace library
    mbed_trace_init();

    // Initialize Mbed CLI
    cmd_init(nullptr);

    // Set up Mbed Trace and Mbed CLI to use a mutex for the console (so they don't try to print stuff
    // at the same time)
    mbed_trace_mutex_wait_function_set(serial_out_mutex_wait);
    mbed_trace_mutex_release_function_set(serial_out_mutex_release);
    cmd_mutex_wait_func(serial_out_mutex_wait);
    cmd_mutex_release_func(serial_out_mutex_release);

    // Add commands
#if MBED_CONF_TARGET_NETWORK_DEFAULT_INTERFACE_TYPE == WIFI
    cmd_add("wifi-scan", wifi_scan, "Scan for Wi-Fi networks.", nullptr);
    cmd_add("wifi-connect-dhcp", wifi_connect_dhcp, "Connect to Wi-Fi. IP address will be obtained from DHCP.",
        "Usage: wifi-connect-dhcp <ssid> <security> [password]\n"
        "This will connect to the given wi-fi network with the given SSID, security type, and password.\n"
        "Available networks can be seen by running the `wifi-scan` command.");
#endif

    // Run the CLI

    cmd_init_screen();
    while (true) {
        int c = getchar();
        if (c != EOF) {
            cmd_char_input(c);
        }
    }
}
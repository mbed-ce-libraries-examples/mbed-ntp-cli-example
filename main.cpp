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

#include "network_connection_cmds.h"

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
    cmd_add("ipconfig", ipconfig, "Display network configuration.", nullptr);
#if MBED_CONF_TARGET_NETWORK_DEFAULT_INTERFACE_TYPE == WIFI
    cmd_add("wifi-scan", wifi_scan, "Scan for Wi-Fi networks.", nullptr);
    cmd_add("wifi-connect", wifi_connect, "Connect to Wi-Fi. IP address will be obtained from DHCP.",
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
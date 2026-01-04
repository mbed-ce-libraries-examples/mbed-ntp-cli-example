#pragma once

// Command to print the time to the console
int get_time(int argc, char *argv[]);

// Command to perform an NTP sync
int ntp_update(int argc, char *argv[]);

// Un-sync the time between the device and the NTP server
int unsync_time(int argc, char *argv[]);
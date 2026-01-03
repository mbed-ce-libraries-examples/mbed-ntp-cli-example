
#include "network_connection_cmds.h"
#include "ns_cmdline.h"

#include "ntp_cmds.h"

#include <cinttypes>

int get_time(int argc, char *argv[]) {
    // Read the RTC time via std::time() (which Mbed implements).
    // You could also do this via the RealTimeClock class but this way we get it as a time_t,
    // which is what we need.
    const std::time_t currTime = std::time(nullptr);

    // Format time as a string
    char timeString[std::size("yyyy-mm-dd hh:mm:ss")];
    std::strftime(timeString, sizeof(timeString), "%F %T", std::localtime(&currTime));

    // Print time to console
    printf("%s\n", timeString);

    return CMDLINE_RETCODE_SUCCESS;
}

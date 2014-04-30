set_global_assignment -entity "clock_11" -library "lib_clock_11" -name IP_TOOL_NAME "altera_pll"
set_global_assignment -entity "clock_11" -library "lib_clock_11" -name IP_TOOL_VERSION "13.1.1"
set_global_assignment -entity "clock_11" -library "lib_clock_11" -name IP_TOOL_ENV "mwpim"
set_global_assignment -library "lib_clock_11" -name SPD_FILE [file join $::quartus(sip_path) "clock_11.spd"]

set_global_assignment -library "lib_clock_11" -name MISC_FILE [file join $::quartus(sip_path) "clock_11_sim/clock_11.vo"]

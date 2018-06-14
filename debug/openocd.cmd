#/opt/OpenOCD/bin/openocd -f interface/jlink.cfg -c "transport select swd" -f board/atmel_samr21_xplained_pro.cfg

# 1. Run an OpenOCD Server
/opt/OpenOCD/bin/openocd -f openocd.cfg

#2. Run GDB and connect to it.

GDB Commands:
target extended-remote localhost:3333
monitor reset halt
info registers

-- DISPLAY PORT A REGISTERS
x/24x 0x41004400
-- DISPLAY PORT B REGISTERS
x/24x 0x41004480


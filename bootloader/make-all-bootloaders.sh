#!/bin/bash
ARM_GCC_PATH="$1"
shift 1

export PATH="$PATH":"$ARM_GCC_PATH" 
cd uf2-samdx1

make BOARD=circuitplay_m0 "$@"
read -n 1 -s
make BOARD=dadamachines-automat "$@"
read -n 1 -s
make BOARD=eitech-robotics "$@"
read -n 1 -s
make BOARD=feather_m0 "$@"
read -n 1 -s
make BOARD=feather_m4 "$@"
read -n 1 -s
make BOARD=gemma_m0 "$@"
read -n 1 -s
make BOARD=generic "$@"
read -n 1 -s
make BOARD=grove-zero "$@"
read -n 1 -s
make BOARD=itsybitsy_m0 "$@"
read -n 1 -s
make BOARD=metro_m0 "$@"
read -n 1 -s
make BOARD=metro_m4 "$@"
read -n 1 -s
make BOARD=metro_m4_revb "$@"
read -n 1 -s
make BOARD=mkr1000 "$@"
read -n 1 -s
make BOARD=pirkey "$@"
read -n 1 -s
make BOARD=sensebox-mcu "$@"
read -n 1 -s
make BOARD=sparkfun-samd21-dev "$@"
read -n 1 -s
make BOARD=sparkfun-samd21-mini "$@"
read -n 1 -s
make BOARD=trinket_m0 "$@"
read -n 1 -s
make BOARD=wattuino-rc "$@"
read -n 1 -s
make BOARD=zero "$@"

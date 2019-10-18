#!/bin/bash

openocd -f interface/cmsis-dap.cfg -f target/nrf51.cfg \
-c "init" \
-c "halt" \
-c "nrf51 mass_erase" \
-c "sleep 50" \
-c "program ../../../../../../components/softdevice/s130/hex/s130_nrf51_2.0.0_softdevice.hex" \
-c "sleep 50" \
-c "program ./_build/nrf51422_xxac_s130.hex" \ 
-c "sleep 50" \
-c "reset" \
-c "exit"

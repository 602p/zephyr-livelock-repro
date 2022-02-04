
# Reproduction for Zephyr livelock bug

## TO BUILD

```bash
export ZEPHYR_BASE=../path/to/zephyr/
west build .
west flash # onto an NRF52840DK
```

Observe output over SEGGER RTT log:

```bash
JLinkGDBServerCLExe -select USB -device nRF52840_xxAA -if SWD -endian little -speed 4000 -nohalt &
JLinkRTTClient
```

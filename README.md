
# Reproduction for Zephyr livelock bug

See https://github.com/zephyrproject-rtos/zephyr/issues/42496 .

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

## Configuration

Uncomment `#define ENABLE_MITIGATION` to detect and halt on livelock condition.

Uncomment `#define MODE_JOIN` to cause the main thread to join on the user thread instead of polling; changes exact livelock behaviour.

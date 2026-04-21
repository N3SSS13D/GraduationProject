# WS2812 Dev Cycle Tool

`tools/ws2812_dev_cycle.py` is the current integration entry for the XiaoZhi + AI8051 workflow on Windows.

This tool keeps the flow narrow on purpose: watch source changes, rebuild firmware, flash the XiaoZhi ESP32, and reopen the serial monitors needed for Bluetooth debugging.

## Success criteria

1. The script can complete a full `--dry-run` path without hardware access.
2. The script can resolve the required build tool paths before runtime work starts.
3. A single command can rerun build, flash, and serial-monitor steps after source changes.

## Covered flow

1. Build and flash the XiaoZhi ESP32 firmware.
2. Open a dedicated XiaoZhi monitor window.
3. Rebuild the AI8051 Keil project.
4. Wait for the AI8051 reconnect delay.
5. Open a dedicated AI8051 serial monitor window.
6. Optionally auto-send the HC-05 BT probe sequence after the AI8051 monitor opens.

## Quick start

Load the development environment into the current PowerShell session, optionally overriding the device ports, then run the tool:

```powershell
. .\tools\dev_env.ps1 -EspPort COM17 -Ai8051ComPort COM15
.\.venv\Scripts\python.exe .\tools\ws2812_dev_cycle.py
```

If you want the tool to keep rebuilding after each source change:

```powershell
.\.venv\Scripts\python.exe .\tools\ws2812_dev_cycle.py --watch
```

## Common options

- `--watch`: rerun the flow after watched source files change.
- `--skip-xiaozhi-monitor`: skip `idf.py monitor`.
- `--skip-ai8051-monitor`: skip reopening the AI8051 serial monitor.
- `--ai8051-reconnect-delay-seconds`: override the default 20-second reconnect delay.
- `--run-ai8051-bt-debug`: after reopening the AI8051 serial monitor, automatically send the configured `BT` test-command sequence.
- `--ai8051-bt-command-sequence`: BT command list separated by `|`.
- `--watch-paths <paths...>`: override the default watched directories.
- `--dry-run`: print the intended commands without touching hardware.

## Environment variables

- `WS2812_IDF_PATH`: ESP-IDF root. Default: `S:\Embedded\ESP\v5.4.3\esp-idf`
- `WS2812_KEIL_UV4_PATH`: Keil CLI path. Default: `S:\Embedded\Keil\UV4\uVision.com`
- `WS2812_ESP_PORT`: XiaoZhi flash/monitor port.
- `WS2812_AI8051_COM_PORT`: AI8051 serial monitor port.

## Required environment and tools

- Repository root: `D:\GraduationProject`
- XiaoZhi project root: `D:\GraduationProject\External\xiaozhi-esp32`
- ESP-IDF root: `S:\Embedded\ESP\v5.4.3\esp-idf`
- ESP-IDF export script: `S:\Embedded\ESP\v5.4.3\esp-idf\export.bat`
- Keil CLI: `S:\Embedded\Keil\UV4\uVision.com`
- Repository Python environment: `D:\GraduationProject\.venv\Scripts\python.exe`
- Required Python package: `pyserial`

Ports still need to be supplied per device connection:

- `WS2812_ESP_PORT`: XiaoZhi board port, for example `COM17`
- `WS2812_AI8051_COM_PORT`: AI8051 debug port, for example `COM15`

## Verified command

The current no-hardware verification entry is:

```powershell
.\.venv\Scripts\python.exe .\tools\ws2812_dev_cycle.py --dry-run --esp-port COM17 --ai8051-com-port COM15
```

## Notes

- In one-shot mode, the child monitor windows stay open after the build cycle finishes.
- In watch mode, the script closes and recreates the child monitor windows on each rebuild cycle.
- The AI8051 serial monitor now uses `pyserial`, so it no longer depends on the PowerShell `System.IO.Ports` assembly.
- The current recommended HC-05 debug sequence is `BT SEND AT`, `BT SEND AT+VERSION?`, `BT SEND AT+ADDR?`, `BT SEND AT+NAME?`, `BT SEND AT+PSWD?`, `BT SEND AT+UART?`, `BT STATUS`.
- HC-05 configuration now keeps all AT commands and queries at `38400`, then uses `AT+RESET` and finally switches the local baudrate to `115200` as the last two steps.
- The repository does not currently contain a verified AI8051 ISP CLI flow, so the Python tool rebuilds the AI8051 firmware and reopens its serial monitor but does not auto-flash the LED side.

## Verification status

- Python syntax validation should pass for `tools/ws2812_dev_cycle.py`.
- The repository Python environment contains `pyserial`.

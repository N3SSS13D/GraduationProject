# WS2812 Dev Cycle Tool

`tools/ws2812_dev_cycle.ps1` implements the prompt-defined XiaoZhi + MCP + AI8051 debug sequence as a reusable Windows tool.

This tool is now maintained under the repository-level `karpathy-guidelines` skill: keep assumptions explicit, prefer the smallest change that solves the task, and verify each stage with a concrete check.

## Success criteria

1. The script can complete a full `-DryRun` path without hardware access.
2. The script can resolve all required tool paths before runtime work starts.
3. The environment and tool entry points are documented in one place so they do not need to be rediscovered later.

## Covered flow

1. Build and flash the XiaoZhi ESP32 firmware.
2. Open a dedicated XiaoZhi monitor window.
3. Stop the old local MCP Python bridge and start a new one.
4. Confirm that the local MCP HTTP status endpoint is ready.
5. Optionally validate `/control/snapshot`.
6. Stop the AI8051 serial monitor, build the Keil project, wait 20 seconds, and reopen the serial monitor.
7. Optionally let the reopened AI8051 serial monitor auto-send BT debug commands and print both host transmit and device reply logs.

## Quick start

Load the development environment into the current PowerShell session, optionally overriding the device ports, then run the tool:

```powershell
. .\tools\dev_env.ps1 -EspPort COM17 -Ai8051ComPort COM15
powershell -ExecutionPolicy Bypass -File .\tools\ws2812_dev_cycle.ps1
```

If you do not want to set ports during environment loading, omit them and continue using `-EspPort` and `-Ai8051ComPort` directly on `ws2812_dev_cycle.ps1`.

If you want the tool to keep rebuilding after each source change:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\ws2812_dev_cycle.ps1 -Watch
```

## Common options

- `-Watch`: rerun the full flow after watched source files change.
- `-SkipMcp`: only run XiaoZhi build/flash/monitor and AI8051 build/serial monitor.
- `-SkipXiaozhiMonitor`: skip `idf.py monitor`.
- `-SkipAi8051Monitor`: skip reopening the AI8051 serial monitor.
- `-ValidateSnapshotControl`: after MCP restart, send one `POST /control/snapshot` validation request.
- `-Ai8051ReconnectDelaySeconds`: override the default 20-second reconnect delay.
- `-RunAi8051BtDebug`: after reopening the AI8051 serial monitor, automatically send the configured `BT` test-command sequence.
- `-Ai8051BtCommandSequence`: BT command list separated by `|`. Default: `BT SEND AT|BT SEND AT+VERSION?|BT SEND AT+ADDR?|BT SEND AT+NAME?|BT SEND AT+PSWD?|BT SEND AT+UART?|BT STATUS`
- `-WatchPaths <paths...>`: override the default watched directories.

## Environment variables

- `WS2812_IDF_PATH`: ESP-IDF root. Default: `S:\Embedded\ESP\v5.4.3\esp-idf`
- `WS2812_KEIL_UV4_PATH`: Keil UV4 executable. Default: `S:\Embedded\Keil\UV4\UV4.exe`
- `WS2812_ESP_PORT`: XiaoZhi flash/monitor port.
- `WS2812_AI8051_COM_PORT`: AI8051 serial monitor port.
- `WS2812_MCP_PYTHON`: Python used for `gp_mcp_endpoint_client.py`.
- `WS2812_MCP_URL`: optional MCP websocket URL override.

## Required environment and tools

Verified in the current workspace:

- Repository root: `D:\GraduationProject`
- XiaoZhi project root: `D:\GraduationProject\External\xiaozhi-esp32`
- ESP-IDF root: `S:\Embedded\ESP\v5.4.3\esp-idf`
- ESP-IDF export script: `S:\Embedded\ESP\v5.4.3\esp-idf\export.ps1`
- Keil executable used by the automation script: `S:\Embedded\Keil\UV4\UV4.exe`
- Verified Keil CLI alternative from repository memory: `S:\Embedded\Keil\UV4\uVision.com -r D:\GraduationProject\STC51\Project\ws2812_driver\ws2812_driver.uvproj -t ws2812_driver`
- MCP helper script: `D:\GraduationProject\External\xiaozhi-esp32\GP_Port\gp_mcp_endpoint_client.py`
- Verified Python interpreter for XiaoZhi-side tools: `D:\GraduationProject\External\xiaozhi-esp32\.venv\Scripts\python.exe`
- Verified Python packages in that environment: `pyserial 3.5`, `websockets 16.0`

Ports still need to be supplied per device connection:

- `WS2812_ESP_PORT`: XiaoZhi board port, for example `COM17`
- `WS2812_AI8051_COM_PORT`: AI8051 debug port, for example `COM15`

Recommended one-time shell setup:

```powershell
. .\tools\dev_env.ps1 -EspPort COM17 -Ai8051ComPort COM15
```

Environment-only setup without ports:

```powershell
. .\tools\dev_env.ps1
```

Then supply the ports at run time when needed:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\ws2812_dev_cycle.ps1 -EspPort COM17 -Ai8051ComPort COM15
```

To rebuild only the AI8051 side, reopen the monitor, and automatically run the HC-05 BT probe sequence:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\ws2812_dev_cycle.ps1 -Ai8051ComPort COM15 -SkipMcp -SkipXiaozhiMonitor -RunAi8051BtDebug
```

## Verified command

The following command is the current no-hardware verification entry for the full workflow:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\ws2812_dev_cycle.ps1 -DryRun -ValidateSnapshotControl -EspPort COM17 -Ai8051ComPort COM15
```

## Notes

- The script opens dedicated PowerShell windows for the XiaoZhi monitor, MCP bridge, and AI8051 serial monitor.
- In one-shot mode, those child windows stay open after the build cycle finishes.
- In watch mode, the script automatically closes and recreates those child windows on each rebuild cycle.
- The default AI8051U <-> HC-05 UART2 transport now starts at `9600 8N1`.
- When `-RunAi8051BtDebug` is enabled, the AI8051 monitor window prints host-side transmit lines as `[HOST_BT_TX] ...`, then keeps printing BT replies from firmware.
- The current recommended HC-05 debug sequence is `BT SEND AT`, `BT SEND AT+VERSION?`, `BT SEND AT+ADDR?`, `BT SEND AT+NAME?`, `BT SEND AT+PSWD?`, `BT SEND AT+UART?`, `BT STATUS`.
- The AI8051 USB monitor baudrate stays unchanged; when the HC-05 accepts `AT+UART=...`, firmware switches its local `UART2` baudrate only after the reply contains `OK`.
- Firmware now prints `[BT_CMD]`, `[BT_RSP]`, `[BT_STA]`, `[BT_MON]`, and `[BT_ACT]` lines over USB so you can inspect command flow, reply framing, UART2 status, monitor output, and LED debug actions.

## Verification status

- PowerShell syntax parsing passed for `tools/ws2812_dev_cycle.ps1`.
- The XiaoZhi Python environment is configured and contains both required packages: `pyserial` and `websockets`.
- The script is expected to complete the full `-DryRun` path, including the MCP branch, without waiting for a real `/status` endpoint.

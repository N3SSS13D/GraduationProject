# WS2812 Auto Debug Workflow

`Project/Script/tools/ws2812_auto_debug.py` is the unified automation entry for the current debug chain.

## Default flow

1. Verify Keil and ESP-IDF toolchain availability using:
   - `S:\Embedded\Keil`
   - `S:\Embedded\ESP\v5.4.3\esp-idf`
2. Build 51-side firmware with Keil only (no download step).
3. Wait `20s`, then open AI8051U serial monitor (`COM15` by default).
4. Run ESP-IDF `build flash monitor` for AI-side and keep monitor output in the foreground.

## Run

```powershell
.\.venv\Scripts\python.exe .\Project\Script\tools\ws2812_auto_debug.py
```

## Common options

```powershell
.\.venv\Scripts\python.exe .\Project\Script\tools\ws2812_auto_debug.py --stc-port COM15 --esp-port COM17
.\.venv\Scripts\python.exe .\Project\Script\tools\ws2812_auto_debug.py --check-tools
```

## Notes

- Keil build log is written to `Project/Debug/build/keil_build.log`.
- Use `Ctrl+]` to leave ESP-IDF monitor.
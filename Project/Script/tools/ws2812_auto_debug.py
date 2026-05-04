import argparse
import subprocess
import sys
import threading
import time
from pathlib import Path

import serial


DEFAULT_KEIL_ROOT = Path(r"S:\Embedded\Keil")
DEFAULT_IDF_ROOT = Path(r"S:\Embedded\ESP\v5.4.3\esp-idf")
DEFAULT_STC_PORT = "COM15"
DEFAULT_ESP_PORT = "COM17"
DEFAULT_STC_BAUD = 9600
DEFAULT_ESP_BAUD = 115200
DEFAULT_STC_DELAY_SECONDS = 20
DEFAULT_KEIL_TARGET = "ws2812_driver"


def log(message: str) -> None:
    now = time.strftime("%Y-%m-%d %H:%M:%S")
    print(f"[{now}] {message}", flush=True)


def normalize_com_port(port: str) -> str:
    if port.isdigit():
        return f"COM{port}"
    return port


def find_uv4_exe(keil_root: Path) -> Path:
    direct = keil_root / "UV4" / "UV4.exe"
    if direct.exists():
        return direct

    for candidate in keil_root.rglob("UV4.exe"):
        return candidate

    raise FileNotFoundError(f"Cannot find UV4.exe under {keil_root}")


def verify_toolchain(keil_root: Path, idf_root: Path) -> dict[str, Path]:
    uv4_path = find_uv4_exe(keil_root)
    export_bat = idf_root / "export.bat"
    idf_py = idf_root / "tools" / "idf.py"

    if not export_bat.exists():
        raise FileNotFoundError(f"Missing ESP-IDF export script: {export_bat}")
    if not idf_py.exists():
        raise FileNotFoundError(f"Missing ESP-IDF idf.py: {idf_py}")

    return {
        "uv4": uv4_path,
        "export_bat": export_bat,
        "idf_py": idf_py,
    }


def stream_process_output(process: subprocess.Popen, log_file: Path | None = None) -> int:
    if process.stdout is None:
        return process.wait()

    handle = None
    if log_file is not None:
        log_file.parent.mkdir(parents=True, exist_ok=True)
        handle = log_file.open("w", encoding="utf-8", newline="")

    try:
        for line in process.stdout:
            print(line, end="", flush=True)
            if handle is not None:
                handle.write(line)
        return process.wait()
    finally:
        if handle is not None:
            handle.close()


def run_keil_rebuild(uv4_path: Path, project_path: Path, target: str, build_log: Path) -> None:
    command = [
        str(uv4_path),
        "-b",
        str(project_path),
        "-t",
        target,
        "-j0",
        "-o",
        str(build_log),
    ]

    log("Starting Keil build (rebuild only, no download).")
    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    code = stream_process_output(process, build_log)
    if code != 0:
        raise RuntimeError(f"Keil build failed with exit code {code}")


def _pump_serial_text(prefix: str, text: str, carry: str) -> str:
    carry += text
    while "\n" in carry:
        line, carry = carry.split("\n", 1)
        print(f"{prefix}{line}", flush=True)
    return carry


def stc_monitor_worker(port: str, baud: int, delay_seconds: int, stop_event: threading.Event) -> None:
    log(f"STC monitor will open after {delay_seconds}s on {port} @ {baud}.")
    start = time.monotonic()
    while (time.monotonic() - start) < delay_seconds:
        if stop_event.is_set():
            return
        time.sleep(0.1)

    try:
        with serial.Serial(port, baudrate=baud, timeout=0.2) as ser:
            log(f"STC monitor opened: {port} @ {baud}")
            carry = ""
            while not stop_event.is_set():
                chunk = ser.read(ser.in_waiting or 1)
                if not chunk:
                    continue
                decoded = chunk.decode("utf-8", errors="replace")
                carry = _pump_serial_text("[STC_MON] ", decoded, carry)
    except Exception as exc:
        log(f"STC monitor error on {port}: {exc}")


def build_idf_command(export_bat: Path, project_path: Path, port: str, baud: int) -> list[str]:
    idf_segment = f'idf.py -p {port} -b {baud} build flash monitor'
    cmd_text = (
        f'call "{export_bat}" && '
        f'cd /d "{project_path}" && '
        f'echo Press Ctrl+] to exit ESP-IDF monitor. && '
        f"{idf_segment}"
    )
    return ["cmd.exe", "/d", "/c", cmd_text]


def run_esp_build_flash_monitor(export_bat: Path, project_path: Path, port: str, baud: int) -> int:
    command = build_idf_command(export_bat, project_path, port, baud)
    log("Starting ESP-IDF build flash monitor.")
    process = subprocess.Popen(command)
    try:
        return process.wait()
    except KeyboardInterrupt:
        process.terminate()
        return process.wait()


def parse_args() -> argparse.Namespace:
    repo_root = Path(__file__).resolve().parents[3]
    parser = argparse.ArgumentParser(description="Unified WS2812 auto-debug workflow.")
    parser.add_argument("--check-tools", action="store_true", help="Only check Keil/ESP-IDF tool availability.")
    parser.add_argument("--dry-run", action="store_true", help="Print resolved commands without running them.")
    parser.add_argument("--keil-root", type=Path, default=DEFAULT_KEIL_ROOT)
    parser.add_argument("--idf-root", type=Path, default=DEFAULT_IDF_ROOT)
    parser.add_argument("--keil-project", type=Path,
                        default=repo_root / "Project" / "STC51" / "ws2812_driver" / "ws2812_driver.uvproj")
    parser.add_argument("--keil-target", default=DEFAULT_KEIL_TARGET)
    parser.add_argument("--esp-project", type=Path, default=repo_root / "Project" / "xiaozhi-esp32")
    parser.add_argument("--stc-port", default=DEFAULT_STC_PORT)
    parser.add_argument("--stc-baud", type=int, default=DEFAULT_STC_BAUD)
    parser.add_argument("--stc-delay-seconds", type=int, default=DEFAULT_STC_DELAY_SECONDS)
    parser.add_argument("--esp-port", default=DEFAULT_ESP_PORT)
    parser.add_argument("--esp-baud", type=int, default=DEFAULT_ESP_BAUD)
    parser.add_argument("--keil-build-log", type=Path,
                        default=repo_root / "Project" / "Debug" / "build" / "keil_build.log")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    args.stc_port = normalize_com_port(args.stc_port)
    args.esp_port = normalize_com_port(args.esp_port)

    try:
        tools = verify_toolchain(args.keil_root, args.idf_root)
    except Exception as exc:
        log(f"Toolchain check failed: {exc}")
        return 2

    log(f"Keil UV4: {tools['uv4']}")
    log(f"ESP-IDF export: {tools['export_bat']}")
    log(f"ESP-IDF idf.py: {tools['idf_py']}")

    if args.check_tools:
        log("Toolchain check completed.")
        return 0

    if args.dry_run:
        log("Dry run mode enabled.")
        print("Keil:", [str(tools["uv4"]), "-b", str(args.keil_project), "-t", args.keil_target, "-j0"], flush=True)
        print("ESP-IDF:", build_idf_command(tools["export_bat"], args.esp_project, args.esp_port, args.esp_baud),
              flush=True)
        return 0

    stop_event = threading.Event()
    monitor_thread = None
    try:
        run_keil_rebuild(tools["uv4"], args.keil_project, args.keil_target, args.keil_build_log)

        monitor_thread = threading.Thread(
            target=stc_monitor_worker,
            args=(args.stc_port, args.stc_baud, args.stc_delay_seconds, stop_event),
            daemon=True,
        )
        monitor_thread.start()

        esp_exit = run_esp_build_flash_monitor(tools["export_bat"], args.esp_project, args.esp_port, args.esp_baud)
        if esp_exit != 0:
            log(f"ESP-IDF build/flash/monitor failed with exit code {esp_exit}")
            return esp_exit
        return 0
    except KeyboardInterrupt:
        log("Interrupted by user.")
        return 130
    except Exception as exc:
        log(f"Workflow failed: {exc}")
        return 1
    finally:
        stop_event.set()
        if monitor_thread is not None:
            monitor_thread.join(timeout=1.5)


if __name__ == "__main__":
    raise SystemExit(main())

import argparse
import os
import signal
import subprocess
import sys
import time
from pathlib import Path

import serial


DEFAULT_IDF_PATH = Path(os.environ.get("WS2812_IDF_PATH", r"S:\Embedded\ESP\v5.4.3\esp-idf"))
DEFAULT_KEIL_CLI = Path(os.environ.get("WS2812_KEIL_UV4_PATH", r"S:\Embedded\Keil\UV4\uVision.com"))
DEFAULT_ESP_MONITOR_BAUD = 115200
DEFAULT_AI8051_BAUD = 9600
DEFAULT_RECONNECT_DELAY_SECONDS = 20
DEFAULT_BT_SEQUENCE = "BT SEND AT|BT SEND AT+VERSION?|BT SEND AT+ADDR?|BT SEND AT+NAME?|BT SEND AT+PSWD?|BT SEND AT+UART?|BT STATUS"
DEFAULT_BT_INITIAL_DELAY_MS = 1500
DEFAULT_BT_COMMAND_DELAY_MS = 1200
DEFAULT_DEBOUNCE_SECONDS = 3
CREATE_NEW_CONSOLE = getattr(subprocess, "CREATE_NEW_CONSOLE", 0)


def log(message: str) -> None:
    timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
    print(f"[{timestamp}] {message}", flush=True)


def normalize_com_port(port: str) -> str:
    if not port:
        return port
    if port.isdigit():
        return f"COM{port}"
    return port


def quote_cmd_token(token: str) -> str:
    return subprocess.list2cmdline([token])


def join_cmd_tokens(tokens) -> str:
    return subprocess.list2cmdline([str(token) for token in tokens])


def run_command(command, cwd: Path, step_name: str, dry_run: bool) -> None:
    if dry_run:
        log(f"[dry-run] {step_name}")
        print(join_cmd_tokens(command), flush=True)
        return

    result = subprocess.run(command, cwd=str(cwd), check=False)
    if result.returncode != 0:
        raise RuntimeError(f"{step_name} failed with exit code {result.returncode}.")


def build_idf_command(export_bat: Path, project_root: Path, idf_args) -> list[str]:
    command_text = (
        f"call {quote_cmd_token(str(export_bat))} && "
        f"cd /d {quote_cmd_token(str(project_root))} && "
        f"{join_cmd_tokens(['idf.py'] + list(idf_args))}"
    )
    return ["cmd.exe", "/d", "/c", command_text]


def build_idf_monitor_command(export_bat: Path, project_root: Path, idf_args) -> list[str]:
    command_text = (
        f"call {quote_cmd_token(str(export_bat))} && "
        f"cd /d {quote_cmd_token(str(project_root))} && "
        f"echo Press Ctrl+] to exit the ESP-IDF monitor. && "
        f"{join_cmd_tokens(['idf.py'] + list(idf_args))}"
    )
    return ["cmd.exe", "/d", "/k", command_text]


def invoke_xiaozhi_build_and_flash(export_bat: Path, project_root: Path, esp_port: str, dry_run: bool) -> None:
    idf_args = []
    if esp_port:
        idf_args.extend(["-p", esp_port])
    idf_args.extend(["build", "flash"])
    run_command(build_idf_command(export_bat, project_root, idf_args), project_root, "XiaoZhi build/flash", dry_run)


def start_xiaozhi_monitor(export_bat: Path, project_root: Path, esp_port: str, monitor_baud: int, dry_run: bool):
    idf_args = []
    if monitor_baud > 0:
        idf_args.extend(["-b", str(monitor_baud)])
    if esp_port:
        idf_args.extend(["-p", esp_port])
    idf_args.append("monitor")
    command = build_idf_monitor_command(export_bat, project_root, idf_args)

    if dry_run:
        log("[dry-run] Start XiaoZhi monitor")
        print(join_cmd_tokens(command), flush=True)
        return None

    return subprocess.Popen(command, cwd=str(project_root), creationflags=CREATE_NEW_CONSOLE)


def invoke_keil_build(keil_cli: Path, project_path: Path, dry_run: bool) -> None:
    command = [str(keil_cli), "-r", str(project_path), "-t", "ws2812_driver"]
    run_command(command, project_path.parent, "Keil rebuild", dry_run)


def parse_bt_commands(sequence: str) -> list[str]:
    return [segment.strip() for segment in sequence.split("|") if segment.strip()]


def write_serial_chunk(serial_port: serial.Serial) -> None:
    chunk = serial_port.read(serial_port.in_waiting or 1)
    if chunk:
        sys.stdout.write(chunk.decode("utf-8", errors="replace"))
        sys.stdout.flush()


def invoke_ai8051_bt_debug_sequence(serial_port: serial.Serial,
                                    commands: list[str],
                                    initial_delay_ms: int,
                                    command_delay_ms: int) -> None:
    if not commands:
        return

    if initial_delay_ms > 0:
        time.sleep(initial_delay_ms / 1000.0)
        write_serial_chunk(serial_port)

    for command in commands:
        print(f"[HOST_BT_TX] {command}", flush=True)
        serial_port.write((command + "\r\n").encode("ascii", errors="ignore"))
        deadline = time.time() + (command_delay_ms / 1000.0)
        while time.time() < deadline:
            write_serial_chunk(serial_port)
            time.sleep(0.12)


def run_ai8051_monitor(args) -> None:
    com_port = normalize_com_port(args.ai8051_com_port)
    if not com_port:
        raise RuntimeError("AI8051 serial monitor requires --ai8051-com-port.")

    with serial.Serial(com_port, args.ai8051_baud_rate, timeout=0.25) as serial_port:
        print(f"Listening on {com_port} @ {args.ai8051_baud_rate} 8N1. Press Ctrl+C to close.", flush=True)
        if args.run_ai8051_bt_debug:
            invoke_ai8051_bt_debug_sequence(serial_port,
                                            parse_bt_commands(args.ai8051_bt_command_sequence),
                                            args.ai8051_bt_initial_delay_ms,
                                            args.ai8051_bt_command_delay_ms)
        while True:
            write_serial_chunk(serial_port)
            time.sleep(0.12)


def start_ai8051_monitor(script_path: Path, repo_root: Path, args, dry_run: bool):
    command = [
        sys.executable,
        str(script_path),
        "--mode",
        "ai8051-monitor",
        "--ai8051-com-port",
        normalize_com_port(args.ai8051_com_port),
        "--ai8051-baud-rate",
        str(args.ai8051_baud_rate),
        "--ai8051-bt-command-sequence",
        args.ai8051_bt_command_sequence,
        "--ai8051-bt-initial-delay-ms",
        str(args.ai8051_bt_initial_delay_ms),
        "--ai8051-bt-command-delay-ms",
        str(args.ai8051_bt_command_delay_ms),
    ]
    if args.run_ai8051_bt_debug:
        command.append("--run-ai8051-bt-debug")

    if dry_run:
        log("[dry-run] Start AI8051 serial monitor")
        print(join_cmd_tokens(command), flush=True)
        return None

    return subprocess.Popen(command, cwd=str(repo_root), creationflags=CREATE_NEW_CONSOLE)


def stop_process(process, name: str):
    if process is None:
        return None
    if process.poll() is not None:
        return None

    log(f"Stopping {name} (PID {process.pid}).")
    process.terminate()
    try:
        process.wait(timeout=5)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait(timeout=5)
    return None


def build_watch_targets(repo_root: Path, project_root: Path, custom_paths: list[str]) -> list[Path]:
    if custom_paths:
        return [Path(path) for path in custom_paths]

    return [
        project_root / "main",
        project_root / "GP_Port",
        project_root / "sdkconfig",
        project_root / "sdkconfig.defaults",
        project_root / "sdkconfig.defaults.esp32s3",
        project_root / "CMakeLists.txt",
        repo_root / "STC51" / "Project" / "ws2812_driver" / "Sources",
        repo_root / "STC51" / "Project" / "ws2812_driver" / "ws2812_driver.uvproj",
        repo_root / "tools" / "ws2812_dev_cycle.py",
    ]


def snapshot_paths(paths: list[Path]) -> dict[str, int]:
    snapshot = {}
    for path in paths:
        if not path.exists():
            continue
        if path.is_file():
            snapshot[str(path.resolve())] = path.stat().st_mtime_ns
            continue

        for root, dirnames, filenames in os.walk(path):
            dirnames[:] = [name for name in dirnames if name not in {"build", ".git", ".venv", "Objects"}]
            for filename in filenames:
                full_path = Path(root) / filename
                try:
                    snapshot[str(full_path.resolve())] = full_path.stat().st_mtime_ns
                except OSError:
                    continue
    return snapshot


def wait_for_changes(paths: list[Path], debounce_seconds: int) -> None:
    previous = snapshot_paths(paths)
    while True:
        time.sleep(1.0)
        current = snapshot_paths(paths)
        changed_paths = sorted({*previous.keys(), *current.keys()})
        changed_path = next((path for path in changed_paths if previous.get(path) != current.get(path)), "")
        if changed_path:
            log(f"Change detected at {changed_path}.")
            stable_deadline = time.time() + debounce_seconds
            latest_snapshot = current
            while time.time() < stable_deadline:
                time.sleep(0.5)
                current = snapshot_paths(paths)
                if current != latest_snapshot:
                    latest_snapshot = current
                    stable_deadline = time.time() + debounce_seconds
            return
        previous = current


def validate_paths(args, repo_root: Path, project_root: Path, keil_project: Path) -> tuple[Path, Path, Path]:
    export_bat = args.idf_path / "export.bat"
    if not export_bat.exists():
        raise RuntimeError(f"ESP-IDF export script not found: {export_bat}")
    if not project_root.exists():
        raise RuntimeError(f"XiaoZhi project not found: {project_root}")
    if not keil_project.exists():
        raise RuntimeError(f"Keil project not found: {keil_project}")
    if not args.keil_cli_path.exists():
        raise RuntimeError(f"Keil CLI executable not found: {args.keil_cli_path}")
    if not args.skip_ai8051_monitor and not args.ai8051_com_port:
        raise RuntimeError("AI8051 serial monitor requires --ai8051-com-port or WS2812_AI8051_COM_PORT.")
    return export_bat, project_root, keil_project


def invoke_development_cycle(script_path: Path,
                             repo_root: Path,
                             export_bat: Path,
                             project_root: Path,
                             keil_project: Path,
                             args,
                             state: dict) -> None:
    state["xiaozhi_monitor"] = stop_process(state.get("xiaozhi_monitor"), "XiaoZhi monitor")
    state["ai8051_monitor"] = stop_process(state.get("ai8051_monitor"), "AI8051 monitor")

    log("Step 1/4: Build and flash XiaoZhi firmware.")
    invoke_xiaozhi_build_and_flash(export_bat, project_root, args.esp_port, args.dry_run)

    if not args.skip_xiaozhi_monitor:
        log("Step 2/4: Start XiaoZhi monitor.")
        state["xiaozhi_monitor"] = start_xiaozhi_monitor(export_bat,
                                                          project_root,
                                                          args.esp_port,
                                                          args.esp_monitor_baud,
                                                          args.dry_run)

    log("Step 3/4: Rebuild AI8051 firmware.")
    invoke_keil_build(args.keil_cli_path, keil_project, args.dry_run)

    if args.ai8051_reconnect_delay_seconds > 0:
        log(f"Waiting {args.ai8051_reconnect_delay_seconds} seconds before reopening AI8051 serial monitor.")
        if not args.dry_run:
            time.sleep(args.ai8051_reconnect_delay_seconds)

    if not args.skip_ai8051_monitor:
        log("Step 4/4: Start AI8051 serial monitor.")
        state["ai8051_monitor"] = start_ai8051_monitor(script_path, repo_root, args, args.dry_run)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="WS2812 development cycle helper.")
    parser.add_argument("--mode", choices=["run", "ai8051-monitor"], default="run")
    parser.add_argument("--idf-path", type=Path, default=DEFAULT_IDF_PATH)
    parser.add_argument("--xiaozhi-project", type=Path)
    parser.add_argument("--esp-port", default=os.environ.get("WS2812_ESP_PORT", ""))
    parser.add_argument("--esp-monitor-baud", type=int, default=DEFAULT_ESP_MONITOR_BAUD)
    parser.add_argument("--keil-cli-path", type=Path, default=DEFAULT_KEIL_CLI)
    parser.add_argument("--keil-project", type=Path)
    parser.add_argument("--ai8051-com-port", default=os.environ.get("WS2812_AI8051_COM_PORT", ""))
    parser.add_argument("--ai8051-baud-rate", type=int, default=DEFAULT_AI8051_BAUD)
    parser.add_argument("--ai8051-reconnect-delay-seconds", type=int, default=DEFAULT_RECONNECT_DELAY_SECONDS)
    parser.add_argument("--run-ai8051-bt-debug", action="store_true")
    parser.add_argument("--ai8051-bt-command-sequence", default=DEFAULT_BT_SEQUENCE)
    parser.add_argument("--ai8051-bt-initial-delay-ms", type=int, default=DEFAULT_BT_INITIAL_DELAY_MS)
    parser.add_argument("--ai8051-bt-command-delay-ms", type=int, default=DEFAULT_BT_COMMAND_DELAY_MS)
    parser.add_argument("--skip-xiaozhi-monitor", action="store_true")
    parser.add_argument("--skip-ai8051-monitor", action="store_true")
    parser.add_argument("--watch", action="store_true")
    parser.add_argument("--debounce-seconds", type=int, default=DEFAULT_DEBOUNCE_SECONDS)
    parser.add_argument("--watch-paths", nargs="*", default=[])
    parser.add_argument("--dry-run", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    script_path = Path(__file__).resolve()
    repo_root = script_path.parent.parent.resolve()
    project_root = (args.xiaozhi_project or (repo_root / "External" / "xiaozhi-esp32")).resolve()
    keil_project = (args.keil_project or (repo_root / "STC51" / "Project" / "ws2812_driver" / "ws2812_driver.uvproj")).resolve()
    args.esp_port = normalize_com_port(args.esp_port)
    args.ai8051_com_port = normalize_com_port(args.ai8051_com_port)

    if args.mode == "ai8051-monitor":
        run_ai8051_monitor(args)
        return 0

    export_bat, project_root, keil_project = validate_paths(args, repo_root, project_root, keil_project)
    log(f"Repository root: {repo_root}")
    log(f"XiaoZhi project: {project_root}")
    log(f"Keil project: {keil_project}")

    watch_targets = build_watch_targets(repo_root, project_root, args.watch_paths)
    state = {
        "xiaozhi_monitor": None,
        "ai8051_monitor": None,
    }

    try:
        invoke_development_cycle(script_path, repo_root, export_bat, project_root, keil_project, args, state)
        if args.watch:
            log("Watch mode is active. Waiting for source changes.")
            while True:
                wait_for_changes(watch_targets, args.debounce_seconds)
                invoke_development_cycle(script_path, repo_root, export_bat, project_root, keil_project, args, state)
                log("Watch mode is active. Waiting for the next source change.")
        else:
            log("One-shot cycle finished. Child monitor windows stay open until you close them manually.")
    finally:
        if args.watch:
            state["xiaozhi_monitor"] = stop_process(state.get("xiaozhi_monitor"), "XiaoZhi monitor")
            state["ai8051_monitor"] = stop_process(state.get("ai8051_monitor"), "AI8051 monitor")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except KeyboardInterrupt:
        print("Interrupted.", flush=True)
        raise SystemExit(130)
    except Exception as exc:
        print(str(exc), file=sys.stderr, flush=True)
        raise SystemExit(1)
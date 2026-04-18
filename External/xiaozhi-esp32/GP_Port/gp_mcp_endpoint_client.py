#!/usr/bin/env python3
import argparse
import asyncio
import json
import os
import socket
import sys
import threading
from dataclasses import asdict, dataclass
from datetime import datetime
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from typing import Any, Dict, Optional
from urllib.parse import urlparse


try:
    import websockets
except ImportError as exc:  # pragma: no cover
    print(
        "Missing dependency: websockets\n"
        "Install it with: python -m pip install websockets",
        file=sys.stderr,
    )
    raise SystemExit(2) from exc


DEFAULT_MCP_URL = (
    "wss://api.xiaozhi.me/mcp/?token="
    "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VySWQiOjc3MTgyOCwiYWdlbnRJZCI6MTM0MTAxMywiZW5k"
    "cG9pbnRJZCI6ImFnZW50XzEzNDEwMTMiLCJwdXJwb3NlIjoibWNwLWVuZHBvaW50IiwiaWF0IjoxNzc2NDMzNjM5LCJl"
    "eHAiOjE4MDc5OTEyMzl9.impbDZxMTLobjxUqH2z-sGLfU5WcP2H6AY_bAZFxa84pgPxtkQhV_lrPKa789hznYCfAPhBB2"
    "eWl4aiThi7OFg"
)
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "..", "..", ".."))
DEFAULT_SNAPSHOT_DIR = os.path.join(PROJECT_ROOT, "debug_snapshots")
DEFAULT_HTTP_HOST = "0.0.0.0"
DEFAULT_HTTP_PORT = 8765
DEFAULT_CONTROL_TIMEOUT = 10.0


@dataclass
class ServerStatus:
    connected: bool = False
    initialized: bool = False
    tool_calls: int = 0
    control_calls: int = 0
    last_tool: str = "-"
    last_event: str = "idle"
    saved_path: str = "-"
    last_error: str = "-"
    snapshot_url: str = "-"
    control_url: str = "-"
    status_url: str = "-"


def print_status(status: ServerStatus) -> None:
    print(
        "[status] "
        f"connected={status.connected} "
        f"initialized={status.initialized} "
        f"tool_calls={status.tool_calls} "
        f"control_calls={status.control_calls} "
        f"last_tool={status.last_tool} "
        f"event={status.last_event} "
        f"snapshot_url={status.snapshot_url} "
        f"control_url={status.control_url} "
        f"saved={status.saved_path} "
        f"error={status.last_error}"
    )


def update_status(status: ServerStatus, **kwargs: Any) -> None:
    for key, value in kwargs.items():
        setattr(status, key, value)
    print_status(status)


class McpWebSocketClient:
    def __init__(self, url: str, timeout_seconds: float, verbose: bool):
        self.url = url
        self.timeout_seconds = timeout_seconds
        self.verbose = verbose
        self._websocket = None

    async def __aenter__(self) -> "McpWebSocketClient":
        open_timeout = None if self.timeout_seconds <= 0 else self.timeout_seconds
        self._websocket = await websockets.connect(self.url, open_timeout=open_timeout)
        self._log(f"Connected to {self.url}")
        return self

    async def __aexit__(self, exc_type, exc, tb) -> None:
        if self._websocket is not None:
            await self._websocket.close()

    def _log(self, message: str) -> None:
        if self.verbose:
            print(message)

    async def send_json(self, payload: Dict[str, Any]) -> None:
        self._log(f"-> {json.dumps(payload, ensure_ascii=False)}")
        await self._websocket.send(json.dumps(payload, ensure_ascii=False))

    async def receive_json(self) -> Dict[str, Any]:
        if self.timeout_seconds <= 0:
            raw_message = await self._websocket.recv()
        else:
            raw_message = await asyncio.wait_for(self._websocket.recv(), timeout=self.timeout_seconds)
        self._log(f"<- {raw_message}")
        return json.loads(raw_message)

    async def respond(
        self,
        request_id: int,
        result: Optional[Dict[str, Any]] = None,
        error: Optional[Dict[str, Any]] = None,
    ) -> None:
        payload: Dict[str, Any] = {
            "jsonrpc": "2.0",
            "id": request_id,
        }
        if error is not None:
            payload["error"] = error
        else:
            payload["result"] = result or {}
        await self.send_json(payload)


def parse_rgb888(text: str) -> int:
    cleaned = text.strip().lower()
    if cleaned.startswith("#"):
        cleaned = cleaned[1:]
    if cleaned.startswith("0x"):
        cleaned = cleaned[2:]
    if len(cleaned) != 6:
        raise ValueError("RGB888 must use #RRGGBB format")
    return int(cleaned, 16)


def calculate_result(operation: str, left: int, right: int) -> float:
    lowered = operation.strip().lower()
    if lowered in {"add", "+"}:
        return float(left + right)
    if lowered in {"subtract", "minus", "-"}:
        return float(left - right)
    if lowered in {"multiply", "times", "*"}:
        return float(left * right)
    if lowered in {"divide", "/"}:
        if right == 0:
            raise ValueError("Division by zero is not allowed")
        return float(left) / float(right)
    if lowered in {"mod", "%", "modulo"}:
        if right == 0:
            raise ValueError("Modulo by zero is not allowed")
        return float(left % right)
    raise ValueError(f"Unsupported calculator operation: {operation}")


def build_tool_list() -> list[Dict[str, Any]]:
    return [
        {
            "name": "self.calculator.calculate",
            "description": "A calculator example tool. Use it for deterministic arithmetic instead of mental math.",
            "inputSchema": {
                "type": "object",
                "properties": {
                    "operation": {"type": "string", "enum": ["add", "subtract", "multiply", "divide", "mod"]},
                    "left": {"type": "integer"},
                    "right": {"type": "integer"},
                },
                "required": ["operation", "left", "right"],
            },
        },
        {
            "name": "self.screen.debug_dot.show",
            "description": "Show or update the debug color dot on screen.",
            "inputSchema": {
                "type": "object",
                "properties": {
                    "primary_rgb888": {"type": "string", "pattern": "^#?[0-9A-Fa-f]{6}$"},
                    "secondary_rgb888": {"type": "string"},
                    "animation": {"type": "string", "enum": ["solid", "gradient", "pulse"]},
                    "size": {"type": "integer", "minimum": 12, "maximum": 58},
                    "duration_ms": {"type": "integer", "minimum": 300, "maximum": 4000},
                    "label": {"type": "string"},
                    "transcript": {"type": "string"},
                    "source": {"type": "string"},
                },
                "required": ["primary_rgb888"],
            },
        },
    ]


def tool_result_content(payload: Dict[str, Any]) -> Dict[str, Any]:
    return {
        "content": [
            {
                "type": "text",
                "text": json.dumps(payload, ensure_ascii=False),
            }
        ],
        "isError": False,
    }


def save_snapshot_bytes(
    image_bytes: bytes,
    mime_type: str,
    width: int,
    height: int,
    quality: int,
    sequence: int,
    output_dir: str,
    file_prefix: str = "xiaozhi_screen",
) -> str:
    file_extension = ".bin"
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

    if mime_type == "image/png":
        file_extension = ".png"
    elif mime_type == "image/jpeg":
        file_extension = ".jpg"
    else:
        raise ValueError(f"Unsupported image mime_type: {mime_type}")

    os.makedirs(output_dir, exist_ok=True)
    sequence_suffix = f"_seq{sequence}" if sequence > 0 else ""
    quality_suffix = f"_q{quality}" if (quality > 0 and mime_type == "image/jpeg") else ""
    file_name = f"{file_prefix}_{timestamp}{sequence_suffix}_{width}x{height}{quality_suffix}{file_extension}"
    file_path = os.path.abspath(os.path.join(output_dir, file_name))
    with open(file_path, "wb") as snapshot_file:
        snapshot_file.write(image_bytes)

    return file_path


def detect_local_ip() -> str:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    try:
        sock.connect(("8.8.8.8", 80))
        return str(sock.getsockname()[0])
    except OSError:
        return "127.0.0.1"
    finally:
        sock.close()


def extract_tool_payload(result: Dict[str, Any]) -> Dict[str, Any]:
    if not isinstance(result, dict):
        return {"raw_result": result}

    content = result.get("content")
    if isinstance(content, list) and content:
        first_item = content[0]
        if isinstance(first_item, dict) and first_item.get("type") == "text":
            text = first_item.get("text", "")
            if isinstance(text, str):
                try:
                    parsed = json.loads(text)
                    if isinstance(parsed, dict):
                        return parsed
                except json.JSONDecodeError:
                    return {"text": text}
    return result


def handle_local_tool_call(tool_name: str, arguments: Dict[str, Any]) -> Dict[str, Any]:
    if tool_name == "self.calculator.calculate":
        left = int(arguments["left"])
        right = int(arguments["right"])
        operation = str(arguments["operation"])
        result_value = calculate_result(operation, left, right)
        return {
            "operation": operation.lower(),
            "left": left,
            "right": right,
            "result": int(result_value) if result_value.is_integer() else result_value,
        }

    if tool_name == "self.screen.debug_dot.show":
        primary = str(arguments["primary_rgb888"])
        parse_rgb888(primary)
        secondary = str(arguments.get("secondary_rgb888", ""))
        if secondary:
            parse_rgb888(secondary)
        return {
            "primary_rgb888": primary,
            "secondary_rgb888": secondary,
            "animation": str(arguments.get("animation", "solid")),
            "size": int(arguments.get("size", 28)),
            "duration_ms": int(arguments.get("duration_ms", 1400)),
            "label": str(arguments.get("label", "mcp")),
            "transcript": str(arguments.get("transcript", "")),
            "source": str(arguments.get("source", "mcp")),
            "applied": True,
        }

    raise KeyError(f"Unknown tool: {tool_name}")


class McpBridgeServer:
    def __init__(self, client: McpWebSocketClient, output_dir: str, status: ServerStatus):
        self.client = client
        self.output_dir = output_dir
        self.status = status
        self._receiver_task: Optional[asyncio.Task[None]] = None
        self._next_id = 1
        self._pending_requests: Dict[int, asyncio.Future[Dict[str, Any]]] = {}
        self._send_lock = asyncio.Lock()

    async def __aenter__(self) -> "McpBridgeServer":
        await self.client.__aenter__()
        update_status(self.status, connected=True, last_event="connected", last_error="-")
        print(f"[server] connected url={self.client.url}")
        print(f"[server] output_dir={os.path.abspath(self.output_dir)}")
        print("[server] waiting for MCP messages and HTTP control requests")
        self._receiver_task = asyncio.create_task(self._receive_loop())
        return self

    async def __aexit__(self, exc_type, exc, tb) -> None:
        if self._receiver_task is not None:
            self._receiver_task.cancel()
            try:
                await self._receiver_task
            except asyncio.CancelledError:
                pass
        for pending in self._pending_requests.values():
            if not pending.done():
                pending.cancel()
        self._pending_requests.clear()
        update_status(self.status, connected=False)
        await self.client.__aexit__(exc_type, exc, tb)

    async def wait_closed(self) -> None:
        if self._receiver_task is not None:
            await self._receiver_task

    async def call_remote_tool(
        self,
        tool_name: str,
        arguments: Dict[str, Any],
        timeout_seconds: float,
    ) -> Dict[str, Any]:
        if not self.status.initialized:
            raise RuntimeError("MCP bridge has not completed initialization yet")

        request_id = self._next_id
        self._next_id += 1

        loop = asyncio.get_running_loop()
        pending: asyncio.Future[Dict[str, Any]] = loop.create_future()
        self._pending_requests[request_id] = pending

        payload = {
            "jsonrpc": "2.0",
            "id": request_id,
            "method": "tools/call",
            "params": {
                "name": tool_name,
                "arguments": arguments,
            },
        }

        async with self._send_lock:
            await self.client.send_json(payload)

        try:
            message = await asyncio.wait_for(pending, timeout=timeout_seconds)
        except asyncio.TimeoutError as exc:
            self._pending_requests.pop(request_id, None)
            raise TimeoutError("Timed out waiting for remote tool result") from exc

        error = message.get("error")
        if isinstance(error, dict):
            error_text = str(error.get("message", "Unknown MCP error"))
            raise RuntimeError(error_text)
        return extract_tool_payload(message.get("result", {}))

    async def _receive_loop(self) -> None:
        while True:
            message = await self.client.receive_json()
            await self._handle_message(message)

    async def _handle_message(self, message: Dict[str, Any]) -> None:
        request_id = message.get("id")
        method = message.get("method")
        params = message.get("params", {})

        if (request_id is not None) and (method is None) and (("result" in message) or ("error" in message)):
            pending = self._pending_requests.pop(int(request_id), None)
            if pending is not None and not pending.done():
                pending.set_result(message)
            return

        if method == "initialize" and request_id is not None:
            await self.client.respond(
                request_id,
                result={
                    "protocolVersion": "2024-11-05",
                    "capabilities": {"tools": {}},
                    "serverInfo": {"name": "gp-mcp-endpoint-client", "version": "1.0.0"},
                },
            )
            update_status(self.status, initialized=True, last_event="initialize", last_error="-")
            return

        if method == "notifications/initialized":
            update_status(self.status, initialized=True, last_event="notifications_initialized", last_error="-")
            return

        if method == "tools/list" and request_id is not None:
            await self.client.respond(request_id, result={"tools": build_tool_list(), "nextCursor": ""})
            update_status(self.status, last_event="tools_list", last_error="-")
            return

        if method == "ping" and request_id is not None:
            await self.client.respond(request_id, result={})
            update_status(self.status, last_event="ping", last_error="-")
            return

        if method == "tools/call":
            try:
                tool_name = str(params.get("name", ""))
                arguments = params.get("arguments", {})
                if not isinstance(arguments, dict):
                    raise TypeError("tools/call arguments must be an object")
                result_payload = handle_local_tool_call(tool_name, arguments)
                if request_id is not None:
                    await self.client.respond(request_id, result=tool_result_content(result_payload))
                self.status.tool_calls += 1
                update_status(self.status, last_tool=tool_name, last_event="tool_called", last_error="-")
                print(f"[tool] name={tool_name} result={json.dumps(result_payload, ensure_ascii=False)}")
                return
            except (KeyError, TypeError, ValueError) as exc:
                update_status(
                    self.status,
                    last_tool=str(params.get("name", "")),
                    last_event="error",
                    last_error=str(exc),
                )
                if request_id is not None:
                    await self.client.respond(request_id, error={"code": -32602, "message": str(exc)})
                print(f"[tool] error={exc}")
                return

        if request_id is not None:
            await self.client.respond(
                request_id,
                error={"code": -32601, "message": f"Unknown method: {method}"},
            )
        update_status(self.status, last_event=f"unknown_method:{method}")


class HttpSnapshotServer:
    def __init__(
        self,
        host: str,
        port: int,
        output_dir: str,
        status: ServerStatus,
        event_loop: asyncio.AbstractEventLoop,
        bridge: McpBridgeServer,
    ):
        self.host = host
        self.port = port
        self.output_dir = output_dir
        self.status = status
        self.event_loop = event_loop
        self.bridge = bridge
        self._server: Optional[ThreadingHTTPServer] = None
        self._thread: Optional[threading.Thread] = None

    def _build_handler(self):
        outer = self

        class SnapshotHandler(BaseHTTPRequestHandler):
            def _send_json(self, status_code: int, payload: Dict[str, Any]) -> None:
                response = json.dumps(payload, ensure_ascii=False).encode("utf-8")
                self.send_response(status_code)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Content-Length", str(len(response)))
                self.end_headers()
                self.wfile.write(response)

            def _read_request_body(self) -> bytes:
                transfer_encoding = str(self.headers.get("Transfer-Encoding", "")).strip().lower()
                content_length = int(self.headers.get("Content-Length", "0") or "0")

                if transfer_encoding == "chunked":
                    chunks = bytearray()

                    while True:
                        size_line = self.rfile.readline().strip()
                        if not size_line:
                            continue

                        chunk_size = int(size_line.split(b";", 1)[0], 16)
                        if chunk_size == 0:
                            while True:
                                trailer_line = self.rfile.readline()
                                if trailer_line in {b"\r\n", b"\n", b""}:
                                    break
                            break

                        chunks.extend(self.rfile.read(chunk_size))
                        self.rfile.read(2)

                    return bytes(chunks)

                if content_length <= 0:
                    return b""

                return self.rfile.read(content_length)

            def _read_json_body(self) -> Dict[str, Any]:
                body = self._read_request_body()
                if not body:
                    return {}
                try:
                    parsed = json.loads(body.decode("utf-8"))
                except (UnicodeDecodeError, json.JSONDecodeError) as exc:
                    raise ValueError(f"Invalid JSON body: {exc}") from exc
                if not isinstance(parsed, dict):
                    raise ValueError("JSON body must be an object")
                return parsed

            def do_GET(self) -> None:
                parsed = urlparse(self.path)
                if parsed.path != "/status":
                    self.send_error(404, "Unsupported path")
                    return
                self._send_json(200, asdict(outer.status))

            def do_POST(self) -> None:
                parsed = urlparse(self.path)

                if parsed.path == "/snapshot":
                    mime_type = str(self.headers.get("Content-Type", "")).split(";", 1)[0].strip().lower()
                    width = int(self.headers.get("X-Snapshot-Width", "0") or "0")
                    height = int(self.headers.get("X-Snapshot-Height", "0") or "0")
                    quality = int(self.headers.get("X-Snapshot-Quality", "0") or "0")
                    sequence = int(self.headers.get("X-Snapshot-Sequence", "0") or "0")
                    prefix = str(self.headers.get("X-Snapshot-Prefix", "xiaozhi_screen") or "xiaozhi_screen")

                    if mime_type not in {"image/png", "image/jpeg"}:
                        update_status(
                            outer.status,
                            last_event="http_error",
                            last_error=f"Unsupported mime_type: {mime_type}",
                        )
                        self.send_error(400, "Unsupported mime_type")
                        return

                    body = self._read_request_body()
                    if not body:
                        update_status(
                            outer.status,
                            last_event="http_error",
                            last_error="Empty HTTP snapshot payload",
                        )
                        self.send_error(400, "Empty payload")
                        return

                    try:
                        saved_path = save_snapshot_bytes(
                            body,
                            mime_type,
                            width,
                            height,
                            quality,
                            sequence,
                            outer.output_dir,
                            prefix,
                        )
                    except ValueError as exc:
                        update_status(outer.status, last_event="http_error", last_error=str(exc))
                        self.send_error(400, str(exc))
                        return

                    update_status(
                        outer.status,
                        last_tool="http.snapshot.upload",
                        last_event="http_saved",
                        saved_path=saved_path,
                        last_error="-",
                    )
                    self._send_json(
                        200,
                        {
                            "saved": True,
                            "path": saved_path,
                            "mime_type": mime_type,
                            "width": width,
                            "height": height,
                            "sequence": sequence,
                        },
                    )
                    return

                if parsed.path == "/control/snapshot":
                    try:
                        request_payload = self._read_json_body()
                        quality = int(request_payload.get("quality", 50))
                        timeout_seconds = float(request_payload.get("timeout", DEFAULT_CONTROL_TIMEOUT))
                    except ValueError as exc:
                        update_status(
                            outer.status,
                            last_tool="http.control.snapshot",
                            last_event="control_error",
                            last_error=str(exc),
                        )
                        self.send_error(400, str(exc))
                        return

                    if quality < 0 or quality > 95:
                        self.send_error(400, "quality must be between 0 and 95")
                        return
                    if timeout_seconds <= 0:
                        self.send_error(400, "timeout must be greater than 0")
                        return

                    try:
                        future = asyncio.run_coroutine_threadsafe(
                            outer.bridge.call_remote_tool(
                                "self.screen.debug_snapshot.capture",
                                {"quality": quality},
                                timeout_seconds,
                            ),
                            outer.event_loop,
                        )
                        result_payload = future.result(timeout=timeout_seconds + 1.0)
                    except TimeoutError as exc:
                        update_status(
                            outer.status,
                            last_tool="http.control.snapshot",
                            last_event="control_timeout",
                            last_error=str(exc),
                        )
                        self.send_error(504, str(exc))
                        return
                    except RuntimeError as exc:
                        update_status(
                            outer.status,
                            last_tool="http.control.snapshot",
                            last_event="control_error",
                            last_error=str(exc),
                        )
                        self.send_error(503, str(exc))
                        return
                    except Exception as exc:
                        update_status(
                            outer.status,
                            last_tool="http.control.snapshot",
                            last_event="control_error",
                            last_error=str(exc),
                        )
                        self.send_error(500, str(exc))
                        return

                    outer.status.control_calls += 1
                    update_status(
                        outer.status,
                        last_tool="http.control.snapshot",
                        last_event="control_sent",
                        last_error="-",
                    )
                    self._send_json(
                        200,
                        {
                            "requested": True,
                            "quality": quality,
                            "device_result": result_payload,
                        },
                    )
                    return

                self.send_error(404, "Unsupported path")

            def log_message(self, format: str, *args: object) -> None:
                print(f"[http] {self.address_string()} {format % args}")

        return SnapshotHandler

    def start(self) -> str:
        local_ip = detect_local_ip()
        public_host = local_ip if self.host == "0.0.0.0" else self.host
        snapshot_url = f"http://{public_host}:{self.port}/snapshot"
        control_url = f"http://{public_host}:{self.port}/control/snapshot"
        status_url = f"http://{public_host}:{self.port}/status"

        self._server = ThreadingHTTPServer((self.host, self.port), self._build_handler())
        self._thread = threading.Thread(target=self._server.serve_forever, name="gp_snapshot_http", daemon=True)
        self._thread.start()
        update_status(
            self.status,
            last_event="http_listening",
            snapshot_url=snapshot_url,
            control_url=control_url,
            status_url=status_url,
            last_error="-",
        )
        print(f"[http] listening host={self.host} port={self.port}")
        print(f"[http] snapshot upload url={snapshot_url}")
        print(f"[http] snapshot control url={control_url}")
        print(f"[http] status url={status_url}")
        return snapshot_url

    def stop(self) -> None:
        if self._server is None:
            return
        self._server.shutdown()
        self._server.server_close()
        if self._thread is not None:
            self._thread.join(timeout=2.0)
        self._server = None
        self._thread = None


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Expose a local HTTP receiver for XiaoZhi screenshots and a local HTTP control port that "
            "triggers device-side Snap through the MCP bridge."
        )
    )
    parser.add_argument(
        "--url",
        default=os.getenv("GP_MCP_URL", DEFAULT_MCP_URL),
        help="MCP WebSocket endpoint, or use GP_MCP_URL",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=0.0,
        help="Timeout in seconds for waiting on inbound messages. Use 0 to wait indefinitely.",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Print connection progress and raw responses",
    )
    parser.add_argument(
        "--output-dir",
        default=DEFAULT_SNAPSHOT_DIR,
        help="Directory used to save captured screenshots.",
    )
    parser.add_argument(
        "--http-host",
        default=DEFAULT_HTTP_HOST,
        help="HTTP host used for direct device snapshot uploads and local control.",
    )
    parser.add_argument(
        "--http-port",
        type=int,
        default=DEFAULT_HTTP_PORT,
        help="HTTP port used for direct device snapshot uploads and local control.",
    )
    parser.add_argument(
        "--disable-http",
        action="store_true",
        help="Disable the local HTTP snapshot receiver and HTTP control endpoint.",
    )
    return parser.parse_args()


async def run() -> int:
    args = parse_args()
    status = ServerStatus()
    http_server: Optional[HttpSnapshotServer] = None

    try:
        client = McpWebSocketClient(args.url, args.timeout, args.verbose)
        async with McpBridgeServer(client, args.output_dir, status) as bridge:
            if not args.disable_http:
                http_server = HttpSnapshotServer(
                    args.http_host,
                    args.http_port,
                    args.output_dir,
                    status,
                    asyncio.get_running_loop(),
                    bridge,
                )
                http_server.start()
            await bridge.wait_closed()
            return 0
    except TimeoutError:
        print(f"Timed out after {args.timeout:.1f}s while waiting for MCP endpoint response.", file=sys.stderr)
        return 1
    except websockets.exceptions.WebSocketException as exc:
        print(f"WebSocket error: {exc}", file=sys.stderr)
        return 1
    except OSError as exc:
        print(f"Network error: {exc}", file=sys.stderr)
        return 1
    finally:
        if http_server is not None:
            http_server.stop()

    return 0


def main() -> int:
    return asyncio.run(run())


if __name__ == "__main__":
    raise SystemExit(main())

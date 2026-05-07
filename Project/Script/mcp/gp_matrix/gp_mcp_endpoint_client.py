#!/usr/bin/env python3
import ast
import argparse
import asyncio
import json
import mimetypes
import os
import random
import re
import socket
import struct
import sys
import threading
import zlib
from dataclasses import asdict, dataclass, field
from datetime import datetime
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from typing import Any, Dict, Optional, Sequence
from urllib.parse import quote, unquote, urlparse
from urllib import error as urllib_error
from urllib import request as urllib_request

from gp_bridge_mcp_service import build_bitmap_ascii_schema, build_draw_python_ops_schema
from gp_bridge_transport_service import build_bridge_arg_parser
from gp_matrix_llm_inputs import apply_draw_ops, normalize_bitmap_ascii_value, normalize_draw_ops


try:
    import websockets
except ImportError as exc:  # pragma: no cover
    print(
        "Missing dependency: websockets\n"
        "Install it with: python -m pip install websockets",
        file=sys.stderr,
    )
    raise SystemExit(2) from exc

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError as exc:  # pragma: no cover
    print(
        "Missing dependency: Pillow\n"
        "Install it with: python -m pip install Pillow",
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
DEFAULT_DEBUG_WS_HOST = "0.0.0.0"
DEFAULT_DEBUG_WS_PORT = 8766
DEFAULT_DEBUG_WS_PATH = "/debug"
DEFAULT_CONTROL_TIMEOUT = 10.0
DEFAULT_DEVICE_PREVIEW_IP = "10.164.64.41"
DEFAULT_DEVICE_PREVIEW_PORT = 8781
DEFAULT_DEVICE_PREVIEW_PATH = "/debug/preview_image"
DEFAULT_DEVICE_PREVIEW_STATUS_PATH = "/debug/preview_status"
DEFAULT_STARTUP_PREVIEW_TIMEOUT = 12.0
DEFAULT_STARTUP_PREVIEW_READY_WAIT_SECONDS = 15.0
DEFAULT_STARTUP_PREVIEW_POLL_SECONDS = 1.0
DEFAULT_STARTUP_PREVIEW_RETRY_COUNT = 2
DEFAULT_TEXT_FRAME_INTERVAL_MS = 420
DEFAULT_ANIMATION_FRAME_INTERVAL_MS = 42
TEXT_FRAME_INTERVAL_MIN_MS = 20
TEXT_FRAME_INTERVAL_MAX_MS = 5000
ANIMATION_FRAME_INTERVAL_MIN_MS = 1
ANIMATION_FRAME_INTERVAL_MAX_MS = 65535
STATUS_RESULT_TEXT_LIMIT = 320
STATUS_HISTORY_LIMIT = 8
MATRIX_WIDTH = 16
MATRIX_HEIGHT = 16
MATRIX_FRAME_BYTES = MATRIX_WIDTH * MATRIX_HEIGHT
MATRIX_BITMAP_ROW_BYTES = MATRIX_WIDTH // 8
MATRIX_BITMAP_BYTES = MATRIX_BITMAP_ROW_BYTES * MATRIX_HEIGHT
MATRIX_BITMAP_HEX_CHARS = MATRIX_BITMAP_BYTES * 2
RGB888_BYTES = 3
MATRIX_COMPACT_FRAME_BYTES = MATRIX_BITMAP_BYTES + (RGB888_BYTES * 2)
MAX_DRAWING_SOURCE_CHARS = 4000
MAX_DRAWING_AST_NODES = 512
MAX_DRAWING_RANGE_STEPS = 256
MAX_TEXT_FRAME_COUNT = 48
MAX_ANIMATION_FRAME_COUNT = 24
LOCAL_MCP_SERVER_NAME = "gp-display-mcp-bridge"

DRAW_FRAME_TOOL_NAMES = frozenset({
    "self.screen.matrix_16x16.draw",
    "self.screen.matrix_16x16.draw_frame",
})
PROMPT_RENDER_TOOL_NAMES = frozenset({"self.screen.matrix_16x16.render_prompt"})
PYTHON_DRAW_TOOL_NAMES = frozenset({"self.screen.matrix_16x16.draw_python"})
TEXT_SEQUENCE_TOOL_NAMES = frozenset({"self.screen.matrix_16x16.show_text"})
ANIMATION_SEQUENCE_TOOL_NAMES = frozenset({"self.screen.matrix_16x16.draw_animation"})
DEBUG_WS_DELIVERY_TOOL_NAMES = (
    DRAW_FRAME_TOOL_NAMES | PYTHON_DRAW_TOOL_NAMES | TEXT_SEQUENCE_TOOL_NAMES | ANIMATION_SEQUENCE_TOOL_NAMES
)
HTTP_PREVIEW_FALLBACK_TOOL_NAMES = DRAW_FRAME_TOOL_NAMES | PYTHON_DRAW_TOOL_NAMES

ALLOWED_DRAW_HELPER_NAMES = frozenset({
    "range",
    "min",
    "max",
    "abs",
    "int",
    "eval",
    "clear",
    "point",
    "line",
    "rectangle",
    "fill_rectangle",
    "ellipse",
    "fill_ellipse",
    "circle",
    "fill_circle",
    "polygon",
})
ALLOWED_DRAW_METHOD_NAMES = frozenset({
    "point",
    "line",
    "rectangle",
    "ellipse",
    "polygon",
    "arc",
    "pieslice",
    "chord",
})
ALLOWED_DRAW_AST_NODE_TYPES = (
    ast.Module,
    ast.Expr,
    ast.Assign,
    ast.For,
    ast.If,
    ast.Call,
    ast.Name,
    ast.Load,
    ast.Store,
    ast.Constant,
    ast.Tuple,
    ast.List,
    ast.keyword,
    ast.BinOp,
    ast.Add,
    ast.Sub,
    ast.Mult,
    ast.Div,
    ast.FloorDiv,
    ast.Mod,
    ast.UnaryOp,
    ast.USub,
    ast.UAdd,
    ast.BoolOp,
    ast.And,
    ast.Or,
    ast.Not,
    ast.Compare,
    ast.Eq,
    ast.NotEq,
    ast.Lt,
    ast.LtE,
    ast.Gt,
    ast.GtE,
    ast.IfExp,
    ast.Attribute,
)
ALLOWED_DRAW_EVAL_AST_NODE_TYPES = (
    ast.Expression,
    ast.Call,
    ast.Name,
    ast.Load,
    ast.Store,
    ast.Constant,
    ast.Tuple,
    ast.List,
    ast.ListComp,
    ast.comprehension,
    ast.keyword,
    ast.BinOp,
    ast.Add,
    ast.Sub,
    ast.Mult,
    ast.Div,
    ast.FloorDiv,
    ast.Mod,
    ast.UnaryOp,
    ast.USub,
    ast.UAdd,
    ast.BoolOp,
    ast.And,
    ast.Or,
    ast.Not,
    ast.Compare,
    ast.Eq,
    ast.NotEq,
    ast.Lt,
    ast.LtE,
    ast.Gt,
    ast.GtE,
    ast.IfExp,
    ast.Attribute,
)
MATRIX_TEXT_FONT_CANDIDATES = (
    r"C:\Windows\Fonts\msyh.ttc",
    r"C:\Windows\Fonts\msyhbd.ttc",
    r"C:\Windows\Fonts\simhei.ttf",
    r"C:\Windows\Fonts\simsun.ttc",
    r"C:\Windows\Fonts\arial.ttf",
)

PROMPT_COLOR_KEYWORDS: dict[str, int] = {
    "red": 0xFF3030,
    "green": 0x30D158,
    "blue": 0x3A86FF,
    "yellow": 0xFFD60A,
    "orange": 0xFF9F0A,
    "purple": 0xBF5AF2,
    "pink": 0xFF5FA2,
    "cyan": 0x14B8A6,
    "white": 0xF5F5F5,
    "black": 0x101010,
}

PROMPT_PATTERN_TEMPLATES: dict[str, tuple[str, tuple[str, ...]]] = {
    "heart": (
        "heart",
        (
            "................",
            "..###....###....",
            ".#####..#####...",
            "############....",
            "############....",
            ".##########.....",
            "..########......",
            "...######.......",
            "....####........",
            ".....##.........",
            "................",
            "................",
            "................",
            "................",
            "................",
            "................",
        ),
    ),
    "smile": (
        "smile",
        (
            "................",
            "....########....",
            "..##........##..",
            ".#............#.",
            ".#..##....##..#.",
            "#...##....##...#",
            "#..............#",
            "#..............#",
            "#..#........#..#",
            "#...########...#",
            ".#............#.",
            ".##..........##.",
            "..##........##..",
            "....########....",
            "................",
            "................",
        ),
    ),
    "checker": (
        "checker",
        (
            "##..##..##..##..",
            "##..##..##..##..",
            "..##..##..##..##",
            "..##..##..##..##",
            "##..##..##..##..",
            "##..##..##..##..",
            "..##..##..##..##",
            "..##..##..##..##",
            "##..##..##..##..",
            "##..##..##..##..",
            "..##..##..##..##",
            "..##..##..##..##",
            "##..##..##..##..",
            "##..##..##..##..",
            "..##..##..##..##",
            "..##..##..##..##",
        ),
    ),
    "diamond": (
        "diamond",
        (
            ".......##.......",
            "......####......",
            ".....######.....",
            "....########....",
            "...##########...",
            "..############..",
            ".##############.",
            "################",
            "################",
            ".##############.",
            "..############..",
            "...##########...",
            "....########....",
            ".....######.....",
            "......####......",
            ".......##.......",
        ),
    ),
    "ring": (
        "ring",
        (
            ".....######.....",
            "...##########...",
            "..###......###..",
            ".##..........##.",
            ".##..........##.",
            "##............##",
            "##............##",
            "##............##",
            "##............##",
            "##............##",
            ".##..........##.",
            ".##..........##.",
            "..###......###..",
            "...##########...",
            ".....######.....",
            "................",
        ),
    ),
    "arrow_right": (
        "arrow_right",
        (
            "................",
            "........##......",
            "........####....",
            "##......######..",
            "####....########",
            "######..########",
            "################",
            "################",
            "################",
            "######..########",
            "####....########",
            "##......######..",
            "........####....",
            "........##......",
            "................",
            "................",
        ),
    ),
    "cross": (
        "cross",
        (
            "##............##",
            ".##..........##.",
            "..##........##..",
            "...##......##...",
            "....##....##....",
            ".....##..##.....",
            "......####......",
            ".......##.......",
            ".......##.......",
            "......####......",
            ".....##..##.....",
            "....##....##....",
            "...##......##...",
            "..##........##..",
            ".##..........##.",
            "##............##",
        ),
    ),
    "border": (
        "border",
        (
            "################",
            "################",
            "##............##",
            "##............##",
            "##............##",
            "##............##",
            "##............##",
            "##............##",
            "##............##",
            "##............##",
            "##............##",
            "##............##",
            "##............##",
            "##............##",
            "################",
            "################",
        ),
    ),
}


@dataclass
class ServerStatus:
    connected: bool = False
    initialized: bool = False
    connection_state: str = "disconnected"
    connection_detail: str = "-"
    tool_calls: int = 0
    control_calls: int = 0
    last_tool: str = "-"
    last_event: str = "idle"
    saved_path: str = "-"
    last_error: str = "-"
    snapshot_url: str = "-"
    control_url: str = "-"
    matrix_control_url: str = "-"
    matrix_prompt_control_url: str = "-"
    device_preview_control_url: str = "-"
    status_url: str = "-"
    debug_ws_url: str = "-"
    debug_ws_client_state: str = "idle"
    debug_ws_client_detail: str = "-"
    debug_ws_last_message: str = "-"
    last_call_ok: Optional[bool] = None
    last_result_summary: str = "-"
    device_preview_status: str = "idle"
    device_preview_detail: str = "-"
    device_preview_last_attempt_at: str = "-"
    last_matrix_status: str = "idle"
    last_matrix_summary: str = "-"
    last_matrix_sent_at: str = "-"
    recent_calls: list[Dict[str, Any]] = field(default_factory=list)


def print_status(status: ServerStatus) -> None:
    print(
        "[status.connection] "
        f"connected={status.connected} "
        f"initialized={status.initialized} "
        f"state={status.connection_state} "
        f"detail={status.connection_detail} "
        f"tool_calls={status.tool_calls} "
        f"control_calls={status.control_calls} "
        f"last_tool={status.last_tool} "
        f"event={status.last_event} "
        f"error={status.last_error}"
    )
    print(
        "[status.preview] "
        f"state={status.device_preview_status} "
        f"detail={status.device_preview_detail} "
        f"last_attempt={status.device_preview_last_attempt_at} "
        f"saved={status.saved_path}"
    )
    print(
        "[status.matrix] "
        f"state={status.last_matrix_status} "
        f"summary={status.last_matrix_summary} "
        f"sent_at={status.last_matrix_sent_at}"
    )
    if status.debug_ws_url != "-":
        print(
            "[status.debug_ws] "
            f"url={status.debug_ws_url} "
            f"client_state={status.debug_ws_client_state} "
            f"client_detail={status.debug_ws_client_detail} "
            f"last_message={status.debug_ws_last_message}"
        )
    if status.snapshot_url != "-":
        print(
            "[status.http] "
            f"snapshot={status.snapshot_url} "
            f"snapshot_control={status.control_url} "
            f"matrix_control={status.matrix_control_url} "
            f"matrix_prompt_control={status.matrix_prompt_control_url} "
            f"device_preview_control={status.device_preview_control_url} "
            f"status={status.status_url}"
        )


def update_status(status: ServerStatus, **kwargs: Any) -> None:
    for key, value in kwargs.items():
        setattr(status, key, value)
    print_status(status)


def update_device_preview_status(status: ServerStatus, preview_status: str, detail: str) -> None:
    update_status(
        status,
        device_preview_status=preview_status,
        device_preview_detail=detail,
        device_preview_last_attempt_at=datetime.now().isoformat(timespec="seconds"),
    )


def summarize_status_payload(payload: Any) -> str:
    if payload in (None, ""):
        return "-"

    if isinstance(payload, str):
        compact = " ".join(payload.split())
    else:
        compact = json.dumps(payload, ensure_ascii=False, sort_keys=True)

    if len(compact) > STATUS_RESULT_TEXT_LIMIT:
        return compact[: STATUS_RESULT_TEXT_LIMIT - 3] + "..."
    return compact


def summarize_matrix_payload(payload: Dict[str, Any]) -> str:
    frame_hex = str(payload.get("frame_rgb332_hex", ""))
    bitmap_rows_hex = str(payload.get("bitmap_rows_hex", ""))
    primary_rgb888 = str(payload.get("primary_rgb888", ""))
    prompt = str(payload.get("prompt", "")).strip()
    transcript = str(payload.get("transcript", "")).strip()
    source = str(payload.get("source", "")).strip()
    pattern = str(payload.get("pattern", "")).strip()
    preset = str(payload.get("preset", "")).strip()
    parts: list[str] = ["target=led_16x16"]

    if pattern:
        parts.append(f"pattern={pattern}")
    if preset:
        parts.append(f"preset={preset}")
    if prompt:
        parts.append(f"prompt={prompt}")
    elif transcript:
        parts.append(f"transcript={transcript}")
    if source:
        parts.append(f"source={source}")
    if primary_rgb888:
        parts.append(f"primary_rgb888={primary_rgb888}")
    if bitmap_rows_hex:
        bitmap_head = bitmap_rows_hex[:16]
        if len(bitmap_rows_hex) > 16:
            bitmap_head += "..."
        parts.append(f"bitmap_head={bitmap_head}")
    if frame_hex:
        try:
            frame_bytes = bytes.fromhex(frame_hex)
        except ValueError:
            parts.append("frame=invalid_hex")
        else:
            lit_pixels = sum(1 for pixel in frame_bytes if pixel != 0)
            frame_preview = frame_hex[:32]
            if len(frame_hex) > 32:
                frame_preview += "..."
            parts.append(f"lit_pixels={lit_pixels}/{MATRIX_FRAME_BYTES}")
            parts.append(f"frame_head={frame_preview}")

    return summarize_status_payload(" ".join(parts))


def update_matrix_status(status: ServerStatus, payload: Dict[str, Any], matrix_status: str) -> None:
    summary = summarize_matrix_payload(payload)
    timestamp = datetime.now().isoformat(timespec="seconds")

    update_status(
        status,
        last_matrix_status=matrix_status,
        last_matrix_summary=summary,
        last_matrix_sent_at=timestamp,
    )
    print(f"[matrix] {summary}")


def update_debug_ws_status(status: ServerStatus, client_state: str, detail: str, last_message: str = "-") -> None:
    update_status(
        status,
        debug_ws_client_state=client_state,
        debug_ws_client_detail=detail,
        debug_ws_last_message=summarize_status_payload(last_message),
    )


def record_call_status(
    status: ServerStatus,
    *,
    call_type: str,
    name: str,
    ok: bool,
    event: str,
    result: Any = None,
    error: str = "-",
) -> None:
    entry = {
        "time": datetime.now().isoformat(timespec="seconds"),
        "type": call_type,
        "name": name,
        "ok": ok,
        "event": event,
        "error": "-" if ok else error,
        "result": summarize_status_payload(result),
    }

    status.recent_calls.insert(0, entry)
    del status.recent_calls[STATUS_HISTORY_LIMIT:]
    update_status(
        status,
        last_tool=name,
        last_event=event,
        last_error="-" if ok else error,
        last_call_ok=ok,
        last_result_summary=entry["result"],
    )


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


def format_rgb888(rgb: int) -> str:
    return f"#{rgb & 0xFFFFFF:06X}"


def rgb888_to_rgb332(rgb: int) -> int:
    red = (rgb >> 16) & 0xFF
    green = (rgb >> 8) & 0xFF
    blue = rgb & 0xFF
    return (red & 0xE0) | ((green >> 3) & 0x1C) | ((blue >> 6) & 0x03)


def rgb332_to_rgb888(pixel: int) -> tuple[int, int, int]:
    red = (pixel >> 5) & 0x07
    green = (pixel >> 2) & 0x07
    blue = pixel & 0x03

    red = (red * 255) // 7
    green = (green * 255) // 7
    blue = (blue * 255) // 3
    return red, green, blue


def build_matrix_frame_payload_from_bitmap_rows(
    *,
    bitmap_rows: Sequence[int],
    primary_rgb888: str,
    background_rgb888: str,
    source: str,
    transcript: str,
    content_type: str,
    prompt: str = "",
    pattern: str = "",
    label: str = "",
    text: str = "",
    glyph: str = "",
    python_source: str = "",
    eval_source: str = "",
    frame_index: Optional[int] = None,
    frame_count: Optional[int] = None,
) -> Dict[str, Any]:
    if len(bitmap_rows) != MATRIX_HEIGHT:
        raise ValueError("bitmap_rows must contain exactly 16 rows")

    resolved_primary_rgb = parse_rgb888(primary_rgb888)
    resolved_background_rgb = parse_rgb888(background_rgb888)
    primary_rgb332 = rgb888_to_rgb332(resolved_primary_rgb)
    background_rgb332 = rgb888_to_rgb332(resolved_background_rgb)
    frame_bytes = bytearray(MATRIX_FRAME_BYTES)
    normalized_rows: list[int] = []

    for row_index, row_bits in enumerate(bitmap_rows):
        normalized_row_bits = int(row_bits) & 0xFFFF
        normalized_rows.append(normalized_row_bits)
        for column_index in range(MATRIX_WIDTH):
            pixel_offset = row_index * MATRIX_WIDTH + column_index
            enabled = ((normalized_row_bits >> (MATRIX_WIDTH - 1 - column_index)) & 0x0001) != 0
            frame_bytes[pixel_offset] = primary_rgb332 if enabled else background_rgb332

    payload: Dict[str, Any] = {
        "data_format": "matrix_frame_v1",
        "content_type": content_type,
        "frame_rgb332_hex": frame_bytes.hex(),
        "bitmap_rows_hex": "".join(f"{row:04x}" for row in normalized_rows),
        "compact_frame_format": build_compact_bitmap_format_metadata(),
        "width": MATRIX_WIDTH,
        "height": MATRIX_HEIGHT,
        "primary_rgb888": format_rgb888(resolved_primary_rgb),
        "background_rgb888": format_rgb888(resolved_background_rgb),
        "source": source,
        "transcript": transcript,
        "applied": True,
    }

    if prompt:
        payload["prompt"] = prompt
    if pattern:
        payload["pattern"] = pattern
    if label:
        payload["label"] = label
    if text:
        payload["text"] = text
    if glyph:
        payload["glyph"] = glyph
    if python_source:
        payload["python_source"] = python_source
    if eval_source:
        payload["eval_source"] = eval_source
    if frame_index is not None:
        payload["frame_index"] = frame_index
    if frame_count is not None:
        payload["frame_count"] = frame_count

    return payload


def build_bitmap_rows_from_mask_image(mask_image: Image.Image) -> list[int]:
    normalized_mask = mask_image.convert("1")

    if normalized_mask.size != (MATRIX_WIDTH, MATRIX_HEIGHT):
        normalized_mask = normalized_mask.resize((MATRIX_WIDTH, MATRIX_HEIGHT), Image.NEAREST)

    bitmap_rows: list[int] = []
    for row_index in range(MATRIX_HEIGHT):
        row_bits = 0
        for column_index in range(MATRIX_WIDTH):
            enabled = normalized_mask.getpixel((column_index, row_index)) != 0
            if enabled:
                row_bits |= 1 << (MATRIX_WIDTH - 1 - column_index)
        bitmap_rows.append(row_bits)

    return bitmap_rows


def normalize_bitmap_rows_hex_value(bitmap_rows_hex: Any, field_name: str) -> str:
    def normalize_bitmap_row_token(row_token: Any) -> str:
        if isinstance(row_token, int):
            row_value = row_token
        else:
            row_text = str(row_token).strip().lower()
            if row_text.startswith("0x"):
                row_text = row_text[2:]
            if (not row_text) or (len(row_text) > 4) or (re.fullmatch(r"[0-9a-f]+", row_text) is None):
                raise ValueError(
                    f"{field_name} row tokens must be 1-4 hex digits or 0x-prefixed 16-bit values"
                )
            row_value = int(row_text, 16)

        if row_value < 0 or row_value > 0xFFFF:
            raise ValueError(f"{field_name} row tokens must stay within 0x0000..0xFFFF")

        return f"{row_value:04x}"

    def normalize_bitmap_row_sequence(row_sequence: Sequence[Any]) -> str:
        if len(row_sequence) != MATRIX_HEIGHT:
            raise ValueError(f"{field_name} must contain exactly {MATRIX_HEIGHT} row values when row tokens are used")

        return "".join(normalize_bitmap_row_token(row_token) for row_token in row_sequence)

    def normalize_compact_hex_pattern(hex_pattern: str) -> Optional[str]:
        if (not hex_pattern) or (re.fullmatch(r"[0-9a-fA-F]+", hex_pattern) is None):
            return None

        if len(hex_pattern) >= MATRIX_BITMAP_HEX_CHARS:
            return None
        if (len(hex_pattern) % 4) != 0:
            return None

        row_tokens = [hex_pattern[index:index + 4] for index in range(0, len(hex_pattern), 4)]
        if (not row_tokens) or (len(row_tokens) > MATRIX_HEIGHT):
            return None

        if len(row_tokens) == MATRIX_HEIGHT:
            return "".join(token.lower() for token in row_tokens)

        if len(row_tokens) == 1:
            expanded_row_tokens = row_tokens * MATRIX_HEIGHT
            return "".join(token.lower() for token in expanded_row_tokens)

        expanded_row_tokens: list[str] = []
        while len(expanded_row_tokens) < MATRIX_HEIGHT:
            expanded_row_tokens.extend(row_tokens)
        expanded_row_tokens = expanded_row_tokens[:MATRIX_HEIGHT]
        return "".join(token.lower() for token in expanded_row_tokens)

    if isinstance(bitmap_rows_hex, (list, tuple)):
        return normalize_bitmap_row_sequence(bitmap_rows_hex)

    bitmap_rows_hex_text = str(bitmap_rows_hex).strip()
    normalized_bitmap_rows_hex = "".join(ch for ch in bitmap_rows_hex_text if not ch.isspace())

    if normalized_bitmap_rows_hex.lower().startswith("0x"):
        normalized_bitmap_rows_hex = normalized_bitmap_rows_hex[2:]

    if len(normalized_bitmap_rows_hex) == MATRIX_BITMAP_HEX_CHARS:
        int(normalized_bitmap_rows_hex, 16)
        return normalized_bitmap_rows_hex.lower()

    compact_hex_pattern = normalize_compact_hex_pattern(normalized_bitmap_rows_hex)
    if compact_hex_pattern:
        return compact_hex_pattern

    parsed_row_sequence: Optional[Sequence[Any]] = None

    # LLM-friendly ASCII mode: 16 lines x 16 chars using on/off symbols.
    try:
        normalized_ascii = normalize_bitmap_ascii_value(bitmap_rows_hex, field_name)
    except ValueError:
        normalized_ascii = ""

    if normalized_ascii:
        return normalized_ascii

    try:
        literal_value = ast.literal_eval(bitmap_rows_hex_text)
    except (SyntaxError, ValueError):
        literal_value = None

    if isinstance(literal_value, (list, tuple)):
        parsed_row_sequence = literal_value
    elif any(separator in bitmap_rows_hex_text for separator in (",", "[", "]", "(", ")", "{", "}", "|", "\n", "\r", "\t")) or ("0x" in bitmap_rows_hex_text.lower()):
        row_tokens = re.findall(r"0x[0-9a-fA-F]{1,4}|[0-9a-fA-F]{1,4}", bitmap_rows_hex_text)
        if len(row_tokens) == MATRIX_HEIGHT:
            parsed_row_sequence = row_tokens

    if parsed_row_sequence is not None:
        return normalize_bitmap_row_sequence(parsed_row_sequence)

    raise ValueError(
        f"{field_name} must contain either exactly {MATRIX_BITMAP_HEX_CHARS} hex characters "
        f"or {MATRIX_HEIGHT} row tokens of 1-4 hex digits (16 rows x 16 bits = {MATRIX_BITMAP_BYTES} bytes). "
        "Shorthand compact hex patterns are also accepted when length is a multiple of 4 and will be repeated to 16 rows"
    )


def normalize_animation_bitmap_rows_hex_list(
    bitmap_rows_hex_list: Sequence[Any],
    frame_interval_ms: int,
) -> tuple[list[str], int, Optional[int]]:
    def is_likely_row_token(token: Any) -> bool:
        if isinstance(token, int):
            return 0 <= token <= 0xFFFF

        token_text = str(token).strip().lower()
        if token_text.startswith("0x"):
            token_text = token_text[2:]

        return bool(token_text) and (len(token_text) <= 4) and (re.fullmatch(r"[0-9a-f]+", token_text) is not None)

    if len(bitmap_rows_hex_list) == MATRIX_HEIGHT and all(is_likely_row_token(row_token) for row_token in bitmap_rows_hex_list):
        normalized_frames = [normalize_bitmap_rows_hex_value(bitmap_rows_hex_list, "bitmap_rows_hex_list")]
    else:
        normalized_frames = [
            normalize_bitmap_rows_hex_value(bitmap_rows_hex, "bitmap_rows_hex_list[]")
            for bitmap_rows_hex in bitmap_rows_hex_list
        ]

    source_frame_count = len(normalized_frames)

    if source_frame_count <= MAX_ANIMATION_FRAME_COUNT:
        return normalized_frames, frame_interval_ms, None

    sampled_frames: list[str] = []

    # Preserve the first and last keyframes while collapsing long sequences to the LED buffer budget.
    for frame_index in range(MAX_ANIMATION_FRAME_COUNT):
        source_index = (frame_index * (source_frame_count - 1)) // (MAX_ANIMATION_FRAME_COUNT - 1)
        sampled_frames.append(normalized_frames[source_index])

    scaled_interval_ms = (frame_interval_ms * source_frame_count + MAX_ANIMATION_FRAME_COUNT - 1) // MAX_ANIMATION_FRAME_COUNT
    scaled_interval_ms = max(ANIMATION_FRAME_INTERVAL_MIN_MS, min(ANIMATION_FRAME_INTERVAL_MAX_MS, scaled_interval_ms))
    return sampled_frames, int(scaled_interval_ms), source_frame_count


def bitmap_rows_hex_to_rows(bitmap_rows_hex: str) -> list[int]:
    normalized_bitmap_rows_hex = normalize_bitmap_rows_hex_value(bitmap_rows_hex, "bitmap_rows_hex")
    return [int(normalized_bitmap_rows_hex[index:index + 4], 16) for index in range(0, 64, 4)]


def bitmap_rows_to_hex(bitmap_rows: Sequence[int]) -> str:
    if len(bitmap_rows) != MATRIX_HEIGHT:
        raise ValueError("bitmap_rows must contain exactly 16 rows")
    return "".join(f"{int(row_bits) & 0xFFFF:04x}" for row_bits in bitmap_rows)


def shift_bitmap_rows(bitmap_rows: Sequence[int], offset: int) -> list[int]:
    normalized_offset = int(offset) % MATRIX_WIDTH
    shifted_rows: list[int] = []

    for row_bits in bitmap_rows:
        normalized_row_bits = int(row_bits) & 0xFFFF
        if normalized_offset == 0:
            shifted_rows.append(normalized_row_bits)
            continue

        wrapped_row = ((normalized_row_bits << normalized_offset) | (normalized_row_bits >> (MATRIX_WIDTH - normalized_offset))) & 0xFFFF
        shifted_rows.append(wrapped_row)

    return shifted_rows


def apply_density_to_bitmap_rows(bitmap_rows: Sequence[int], density: float) -> list[int]:
    clamped_density = max(0.0, min(1.0, float(density)))
    threshold = int(clamped_density * 1000)
    sampled_rows: list[int] = []

    for row_index, row_bits in enumerate(bitmap_rows):
        sampled_row_bits = 0
        normalized_row_bits = int(row_bits) & 0xFFFF

        for column_index in range(MATRIX_WIDTH):
            enabled = ((normalized_row_bits >> (MATRIX_WIDTH - 1 - column_index)) & 0x0001) != 0
            if not enabled:
                continue

            # Deterministic hash sampling keeps frames stable without random flicker.
            pixel_hash = ((row_index * 131) + (column_index * 197) + (row_index * column_index * 17)) % 1000
            if pixel_hash < threshold:
                sampled_row_bits |= 1 << (MATRIX_WIDTH - 1 - column_index)

        sampled_rows.append(sampled_row_bits)

    return sampled_rows


def expand_effect_animation_bitmap_rows_hex_list(base_bitmap_rows_hex: str, effect: Dict[str, Any]) -> list[str]:
    effect_name = str(effect.get("name", "")).strip().lower()
    if not effect_name:
        raise ValueError("effect.name is required")

    base_rows = bitmap_rows_hex_to_rows(base_bitmap_rows_hex)
    frame_count = int(effect.get("frame_count", 0))
    if frame_count <= 0:
        frame_count = 16
    if frame_count < 2:
        raise ValueError("effect.frame_count must be >= 2")

    if effect_name == "blink":
        duty_cycle = float(effect.get("duty_cycle", 0.5))
        clamped_duty = max(0.0, min(1.0, duty_cycle))
        on_frames = max(1, min(frame_count - 1, int(round(frame_count * clamped_duty))))
        off_rows = [0] * MATRIX_HEIGHT
        return [bitmap_rows_to_hex(base_rows if index < on_frames else off_rows) for index in range(frame_count)]

    if effect_name in {"marquee", "marquee_left", "marquee_right"}:
        direction = str(effect.get("direction", "left")).strip().lower()
        if effect_name == "marquee_right":
            direction = "right"
        elif effect_name == "marquee_left":
            direction = "left"

        step = int(effect.get("step", 1))
        if step <= 0:
            raise ValueError("effect.step must be >= 1")

        frames: list[str] = []
        for frame_index in range(frame_count):
            offset = (frame_index * step) % MATRIX_WIDTH
            shifted_rows = shift_bitmap_rows(base_rows, -offset if direction == "right" else offset)
            frames.append(bitmap_rows_to_hex(shifted_rows))
        return frames

    if effect_name == "breathe":
        min_density = float(effect.get("min_density", 0.20))
        max_density = float(effect.get("max_density", 1.00))
        clamped_min_density = max(0.0, min(1.0, min_density))
        clamped_max_density = max(0.0, min(1.0, max_density))
        if clamped_min_density > clamped_max_density:
            clamped_min_density, clamped_max_density = clamped_max_density, clamped_min_density

        frames = []
        for frame_index in range(frame_count):
            phase = frame_index / max(1, frame_count - 1)
            triangle_wave = 1.0 - abs((phase * 2.0) - 1.0)
            density = clamped_min_density + (clamped_max_density - clamped_min_density) * triangle_wave
            density_rows = apply_density_to_bitmap_rows(base_rows, density)
            frames.append(bitmap_rows_to_hex(density_rows))
        return frames

    raise ValueError(f"Unsupported effect.name: {effect_name}")


def resolve_animation_bitmap_rows_hex_sources(
    *,
    bitmap_rows_hex_list: Any,
    frames: Any,
    image: Any,
    effect: Any,
    primary_rgb888: str,
    background_rgb888: str,
    source: str,
) -> list[Any]:
    def resolve_single_frame_source(frame: Any, frame_field_name: str) -> str:
        if not isinstance(frame, dict):
            return normalize_bitmap_rows_hex_value(frame, frame_field_name)

        frame_bitmap_rows_hex = frame.get("bitmap_rows_hex", "")
        frame_bitmap_rows = frame.get("bitmap_rows")
        frame_python_source = str(frame.get("python_source", ""))
        frame_eval_source = str(frame.get("eval_source", ""))
        frame_bitmap_ascii = frame.get("bitmap_ascii")
        frame_ops = frame.get("ops")
        has_bitmap_rows_hex = bool(str(frame_bitmap_rows_hex).strip())
        has_bitmap_rows = frame_bitmap_rows is not None
        has_bitmap_ascii = frame_bitmap_ascii not in (None, "", [])
        has_draw_code = bool(frame_python_source.strip() or frame_eval_source.strip() or frame_ops not in (None, "", []))
        selected_source_count = int(has_bitmap_rows_hex) + int(has_bitmap_rows) + int(has_bitmap_ascii) + int(has_draw_code)

        if selected_source_count != 1:
            raise ValueError(
                f"{frame_field_name} must provide exactly one of bitmap_rows_hex, bitmap_rows, bitmap_ascii, or python_source/eval_source/ops"
            )

        if has_bitmap_rows_hex:
            return normalize_bitmap_rows_hex_value(frame_bitmap_rows_hex, f"{frame_field_name}.bitmap_rows_hex")

        if has_bitmap_rows:
            return normalize_bitmap_rows_hex_value(frame_bitmap_rows, f"{frame_field_name}.bitmap_rows")

        if has_bitmap_ascii:
            return normalize_bitmap_ascii_value(frame_bitmap_ascii, f"{frame_field_name}.bitmap_ascii")

        rendered_frame_payload = render_python_source_to_matrix_frame(
            python_source=frame_python_source,
            eval_source=frame_eval_source,
            draw_ops=frame_ops,
            primary_rgb888=str(frame.get("primary_rgb888", primary_rgb888)),
            background_rgb888=str(frame.get("background_rgb888", background_rgb888)),
            source=source,
            transcript=str(frame.get("transcript", frame_field_name)),
        )
        return normalize_bitmap_rows_hex_value(
            rendered_frame_payload.get("bitmap_rows_hex", ""),
            f"{frame_field_name}.python_bitmap_rows_hex",
        )

    has_bitmap_rows_hex_list = isinstance(bitmap_rows_hex_list, (list, tuple)) and len(bitmap_rows_hex_list) > 0
    has_frames = isinstance(frames, (list, tuple)) and len(frames) > 0
    has_image_effect = (image not in (None, "", [])) or (effect not in (None, "", []))

    if sum((1 if has_bitmap_rows_hex_list else 0, 1 if has_frames else 0, 1 if has_image_effect else 0)) > 1:
        raise ValueError("Provide exactly one animation input mode: bitmap_rows_hex_list, frames, or image+effect")

    if has_image_effect:
        if image in (None, "", []):
            raise ValueError("image is required when effect is provided")
        if not isinstance(effect, dict):
            raise ValueError("effect must be an object")
        base_bitmap_rows_hex = resolve_single_frame_source(image, "image")
        return expand_effect_animation_bitmap_rows_hex_list(base_bitmap_rows_hex, effect)

    if has_bitmap_rows_hex_list and has_frames:
        raise ValueError("Provide either bitmap_rows_hex_list or frames, not both")

    if frames not in (None, "", []) and not isinstance(frames, (list, tuple)):
        raise ValueError("frames must be an array")

    if has_frames:
        normalized_sources: list[str] = []

        for frame_index, frame in enumerate(frames):
            frame_field_name = f"frames[{frame_index}]"
            normalized_sources.append(resolve_single_frame_source(frame, frame_field_name))

        return normalized_sources

    if isinstance(bitmap_rows_hex_list, str) and bitmap_rows_hex_list.strip():
        return [bitmap_rows_hex_list]

    if has_bitmap_rows_hex_list:
        return list(bitmap_rows_hex_list)

    raise ValueError("bitmap_rows_hex_list or frames is required")


def normalize_rgb888_color(color_value: Any, field_name: str) -> str:
    try:
        return format_rgb888(parse_rgb888(str(color_value)))
    except ValueError as exc:
        raise ValueError(f"{field_name} must be a #RRGGBB RGB888 color") from exc


def build_compact_bitmap_format_metadata() -> Dict[str, Any]:
    return {
        "encoding": "bitmap_1bit_rgb888_compact_v1",
        "bit_on": 1,
        "bit_off": 0,
        "row_order": "top_to_bottom",
        "bit_order": "msb_left_to_right",
        "row_count": MATRIX_HEIGHT,
        "row_bits": MATRIX_WIDTH,
        "row_bytes": MATRIX_BITMAP_ROW_BYTES,
        "bitmap_bytes": MATRIX_BITMAP_BYTES,
        "primary_rgb888_bytes": RGB888_BYTES,
        "background_rgb888_bytes": RGB888_BYTES,
        "compact_frame_bytes": MATRIX_COMPACT_FRAME_BYTES,
    }


def try_extract_binary_bitmap_fields_from_frame_hex(frame_hex: str) -> Optional[tuple[str, str, str]]:
    normalized_frame_rgb332_hex = "".join(ch for ch in frame_hex if ch.strip())

    if len(normalized_frame_rgb332_hex) != MATRIX_FRAME_BYTES * 2:
        return None

    frame_bytes = bytes.fromhex(normalized_frame_rgb332_hex)
    pixel_counts: dict[int, int] = {}

    for pixel in frame_bytes:
        pixel_counts[pixel] = pixel_counts.get(pixel, 0) + 1

    if len(pixel_counts) > 2:
        return None

    background_pixel = max(pixel_counts, key=pixel_counts.get)
    foreground_pixels = [pixel for pixel in pixel_counts if pixel != background_pixel]
    if not foreground_pixels:
        return None

    foreground_pixel = foreground_pixels[0]
    bitmap_rows: list[int] = []
    for row_index in range(MATRIX_HEIGHT):
        row_bits = 0
        for column_index in range(MATRIX_WIDTH):
            pixel_offset = row_index * MATRIX_WIDTH + column_index
            if frame_bytes[pixel_offset] != background_pixel:
                row_bits |= 1 << (MATRIX_WIDTH - 1 - column_index)
        bitmap_rows.append(row_bits)

    foreground_red, foreground_green, foreground_blue = rgb332_to_rgb888(foreground_pixel)
    background_red, background_green, background_blue = rgb332_to_rgb888(background_pixel)

    return (
        "".join(f"{row:04x}" for row in bitmap_rows),
        format_rgb888((foreground_red << 16) | (foreground_green << 8) | foreground_blue),
        format_rgb888((background_red << 16) | (background_green << 8) | background_blue),
    )


def safe_matrix_range(*args: int) -> range:
    normalized_args = tuple(int(arg) for arg in args)
    value_range = range(*normalized_args)

    if len(value_range) > MAX_DRAWING_RANGE_STEPS:
        raise ValueError(f"range(...) exceeds the maximum step count of {MAX_DRAWING_RANGE_STEPS}")

    return value_range


def validate_matrix_python_source(python_source: str) -> ast.Module:
    try:
        syntax_tree = ast.parse(python_source, mode="exec")
    except SyntaxError as exc:
        raise ValueError(f"python_source has invalid syntax: {exc.msg} at line {exc.lineno}") from exc

    node_count = 0
    for node in ast.walk(syntax_tree):
        node_count += 1
        if node_count > MAX_DRAWING_AST_NODES:
            raise ValueError(f"python_source is too complex; keep it under {MAX_DRAWING_AST_NODES} AST nodes")

        if not isinstance(node, ALLOWED_DRAW_AST_NODE_TYPES):
            raise ValueError(f"Unsupported Python construct: {type(node).__name__}")

        if isinstance(node, ast.Assign):
            if len(node.targets) != 1 or not isinstance(node.targets[0], ast.Name):
                raise ValueError("Only simple variable assignments are allowed in python_source")

        if isinstance(node, ast.For) and not isinstance(node.target, ast.Name):
            raise ValueError("Only simple for-loop targets are allowed in python_source")

        if isinstance(node, ast.Attribute):
            if not isinstance(node.value, ast.Name) or node.value.id != "draw":
                raise ValueError("Only draw.<method>(...) attribute access is allowed in python_source")
            if node.attr not in ALLOWED_DRAW_METHOD_NAMES:
                raise ValueError(f"Unsupported draw method: {node.attr}")

        if isinstance(node, ast.Call):
            if isinstance(node.func, ast.Name):
                if node.func.id not in ALLOWED_DRAW_HELPER_NAMES:
                    raise ValueError(f"Unsupported helper function: {node.func.id}")
            elif isinstance(node.func, ast.Attribute):
                if node.func.attr not in ALLOWED_DRAW_METHOD_NAMES:
                    raise ValueError(f"Unsupported draw method: {node.func.attr}")
            else:
                raise ValueError("Only helper calls and draw.<method>(...) calls are allowed in python_source")

    return syntax_tree


def validate_matrix_eval_source(eval_source: str) -> ast.Expression:
    try:
        syntax_tree = ast.parse(eval_source, mode="eval")
    except SyntaxError as exc:
        raise ValueError(f"eval_source has invalid syntax: {exc.msg} at line {exc.lineno}") from exc

    node_count = 0
    for node in ast.walk(syntax_tree):
        node_count += 1
        if node_count > MAX_DRAWING_AST_NODES:
            raise ValueError(f"eval_source is too complex; keep it under {MAX_DRAWING_AST_NODES} AST nodes")

        if not isinstance(node, ALLOWED_DRAW_EVAL_AST_NODE_TYPES):
            raise ValueError(f"Unsupported eval Python construct: {type(node).__name__}")

        if isinstance(node, ast.Attribute):
            if not isinstance(node.value, ast.Name) or node.value.id != "draw":
                raise ValueError("Only draw.<method>(...) attribute access is allowed in eval_source")
            if node.attr not in ALLOWED_DRAW_METHOD_NAMES:
                raise ValueError(f"Unsupported draw method: {node.attr}")

        if isinstance(node, ast.Call):
            if isinstance(node.func, ast.Name):
                if node.func.id not in ALLOWED_DRAW_HELPER_NAMES:
                    raise ValueError(f"Unsupported helper function: {node.func.id}")
            elif isinstance(node.func, ast.Attribute):
                if node.func.attr not in ALLOWED_DRAW_METHOD_NAMES:
                    raise ValueError(f"Unsupported draw method: {node.func.attr}")
            else:
                raise ValueError("Only helper calls and draw.<method>(...) calls are allowed in eval_source")

        if isinstance(node, ast.comprehension):
            if node.is_async:
                raise ValueError("Async comprehensions are not allowed in eval_source")
            if not isinstance(node.target, ast.Name):
                raise ValueError("Only simple comprehension targets are allowed in eval_source")

    return syntax_tree


def execute_matrix_eval_source(eval_source: str, execution_scope: Dict[str, Any]) -> Any:
    normalized_eval_source = eval_source.strip()

    if not normalized_eval_source:
        raise ValueError("eval_source is required")
    if len(normalized_eval_source) > MAX_DRAWING_SOURCE_CHARS:
        raise ValueError(f"eval_source is too long; keep it under {MAX_DRAWING_SOURCE_CHARS} characters")

    syntax_tree = validate_matrix_eval_source(normalized_eval_source)
    try:
        return eval(compile(syntax_tree, "<matrix_draw_eval>", "eval"), {"__builtins__": {}}, execution_scope)
    except Exception as exc:
        raise ValueError(f"eval_source execution failed: {exc}") from exc


def render_python_source_to_matrix_frame(
    python_source: str = "",
    eval_source: str = "",
    draw_ops: Any = None,
    primary_rgb888: str = "",
    background_rgb888: str = "",
    source: str = "mcp_python",
    transcript: str = "",
) -> Dict[str, Any]:
    normalized_python_source = python_source.strip()
    normalized_eval_source = eval_source.strip()
    normalized_draw_ops = normalize_draw_ops(draw_ops, "ops")

    if not normalized_python_source and not normalized_eval_source and not normalized_draw_ops:
        raise ValueError("python_source, eval_source, or ops is required")
    if len(normalized_python_source) > MAX_DRAWING_SOURCE_CHARS:
        raise ValueError(f"python_source is too long; keep it under {MAX_DRAWING_SOURCE_CHARS} characters")

    syntax_tree = None
    if normalized_python_source:
        syntax_tree = validate_matrix_python_source(normalized_python_source)

    mask_image = Image.new("1", (MATRIX_WIDTH, MATRIX_HEIGHT), 0)
    draw_context = ImageDraw.Draw(mask_image)

    def clear(fill: int = 0) -> None:
        draw_context.rectangle((0, 0, MATRIX_WIDTH - 1, MATRIX_HEIGHT - 1), fill=1 if fill else 0)

    def point(x: int, y: int, fill: int = 1) -> None:
        draw_context.point((int(x), int(y)), fill=1 if fill else 0)

    def line(x0: int, y0: int, x1: int, y1: int, fill: int = 1, width: int = 1) -> None:
        draw_context.line((int(x0), int(y0), int(x1), int(y1)), fill=1 if fill else 0, width=max(1, int(width)))

    def rectangle(
        x0: int,
        y0: int,
        x1: int,
        y1: int,
        *,
        fill: Optional[int] = None,
        outline: int = 1,
        width: int = 1,
    ) -> None:
        draw_context.rectangle(
            (int(x0), int(y0), int(x1), int(y1)),
            outline=1 if outline else 0,
            fill=None if fill is None else (1 if fill else 0),
            width=max(1, int(width)),
        )

    def fill_rectangle(x0: int, y0: int, x1: int, y1: int) -> None:
        draw_context.rectangle((int(x0), int(y0), int(x1), int(y1)), fill=1)

    def ellipse(
        x0: int,
        y0: int,
        x1: int,
        y1: int,
        *,
        fill: Optional[int] = None,
        outline: int = 1,
        width: int = 1,
    ) -> None:
        draw_context.ellipse(
            (int(x0), int(y0), int(x1), int(y1)),
            outline=1 if outline else 0,
            fill=None if fill is None else (1 if fill else 0),
            width=max(1, int(width)),
        )

    def fill_ellipse(x0: int, y0: int, x1: int, y1: int) -> None:
        draw_context.ellipse((int(x0), int(y0), int(x1), int(y1)), fill=1)

    def circle(cx: int, cy: int, radius: int, *, fill: Optional[int] = None, outline: int = 1, width: int = 1) -> None:
        center_x = int(cx)
        center_y = int(cy)
        radius_value = max(0, int(radius))
        ellipse(
            center_x - radius_value,
            center_y - radius_value,
            center_x + radius_value,
            center_y + radius_value,
            fill=fill,
            outline=outline,
            width=width,
        )

    def fill_circle(cx: int, cy: int, radius: int) -> None:
        circle(int(cx), int(cy), int(radius), fill=1, outline=1)

    def polygon(points: Sequence[Sequence[int]], *, fill: Optional[int] = None, outline: int = 1) -> None:
        normalized_points = [(int(point_pair[0]), int(point_pair[1])) for point_pair in points]
        draw_context.polygon(
            normalized_points,
            outline=1 if outline else 0,
            fill=None if fill is None else (1 if fill else 0),
        )

    execution_scope: Dict[str, Any] = {}

    def safe_eval(expression: str) -> Any:
        if not isinstance(expression, str):
            raise ValueError("eval(...) requires a string expression")
        return execute_matrix_eval_source(expression, execution_scope)

    execution_scope.update({
        "draw": draw_context,
        "range": safe_matrix_range,
        "min": min,
        "max": max,
        "abs": abs,
        "int": int,
        "eval": safe_eval,
        "clear": clear,
        "point": point,
        "line": line,
        "rectangle": rectangle,
        "fill_rectangle": fill_rectangle,
        "ellipse": ellipse,
        "fill_ellipse": fill_ellipse,
        "circle": circle,
        "fill_circle": fill_circle,
        "polygon": polygon,
    })

    try:
        if syntax_tree is not None:
            exec(compile(syntax_tree, "<matrix_draw_python>", "exec"), {"__builtins__": {}}, execution_scope)
        if normalized_eval_source:
            execute_matrix_eval_source(normalized_eval_source, execution_scope)
        if normalized_draw_ops:
            apply_draw_ops(draw_context, normalized_draw_ops)
    except Exception as exc:
        if isinstance(exc, ValueError):
            raise
        raise ValueError(f"python_source execution failed: {exc}") from exc

    bitmap_rows = build_bitmap_rows_from_mask_image(mask_image)
    frame_payload = build_matrix_frame_payload_from_bitmap_rows(
        bitmap_rows=bitmap_rows,
        primary_rgb888=primary_rgb888 or "#F5F5F5",
        background_rgb888=background_rgb888 or "#000000",
        source=source,
        transcript=transcript or normalized_python_source or normalized_eval_source or "ops_draw",
        content_type="python_draw",
        python_source=normalized_python_source,
        eval_source=normalized_eval_source,
        label="python_draw",
    )
    if normalized_draw_ops:
        frame_payload["ops"] = normalized_draw_ops
    frame_payload["tool_name"] = "self.screen.matrix_16x16.draw_python"
    return frame_payload


def render_bitmap_animation_frame_sequence(
    bitmap_rows_hex_list: Sequence[str],
    primary_rgb888: str = "",
    background_rgb888: str = "",
    frame_interval_ms: int = DEFAULT_ANIMATION_FRAME_INTERVAL_MS,
    source: str = "mcp_animation",
    transcript: str = "",
) -> Dict[str, Any]:
    if not bitmap_rows_hex_list:
        raise ValueError("bitmap_rows_hex_list is required")
    if frame_interval_ms < ANIMATION_FRAME_INTERVAL_MIN_MS or frame_interval_ms > ANIMATION_FRAME_INTERVAL_MAX_MS:
        raise ValueError(
            f"frame_interval_ms must be between {ANIMATION_FRAME_INTERVAL_MIN_MS} and {ANIMATION_FRAME_INTERVAL_MAX_MS}"
        )

    resolved_primary_rgb888 = primary_rgb888 or "#F5F5F5"
    resolved_background_rgb888 = background_rgb888 or "#000000"
    parse_rgb888(resolved_primary_rgb888)
    parse_rgb888(resolved_background_rgb888)

    frames: list[Dict[str, Any]] = []
    normalized_bitmap_rows_hex_list, normalized_frame_interval_ms, source_frame_count = normalize_animation_bitmap_rows_hex_list(
        bitmap_rows_hex_list,
        frame_interval_ms,
    )
    total_frames = len(normalized_bitmap_rows_hex_list)
    transcript_text = transcript or f"play {len(bitmap_rows_hex_list)}-frame animation"

    for frame_index, normalized_bitmap_rows_hex in enumerate(normalized_bitmap_rows_hex_list):
        bitmap_rows = [int(normalized_bitmap_rows_hex[index:index + 4], 16) for index in range(0, 64, 4)]
        frame_payload = build_matrix_frame_payload_from_bitmap_rows(
            bitmap_rows=bitmap_rows,
            primary_rgb888=resolved_primary_rgb888,
            background_rgb888=resolved_background_rgb888,
            source=source,
            transcript=transcript_text,
            content_type="animation",
            frame_index=frame_index,
            frame_count=total_frames,
            label="animation_frame",
        )
        frames.append(frame_payload)

    result = {
        "data_format": "matrix_frame_sequence_v1",
        "content_type": "animation",
        "compact_frame_format": build_compact_bitmap_format_metadata(),
        "width": MATRIX_WIDTH,
        "height": MATRIX_HEIGHT,
        "frame_interval_ms": int(normalized_frame_interval_ms),
        "frame_count": total_frames,
        "primary_rgb888": format_rgb888(parse_rgb888(resolved_primary_rgb888)),
        "background_rgb888": format_rgb888(parse_rgb888(resolved_background_rgb888)),
        "frames": frames,
        "source": source,
        "transcript": transcript_text,
        "applied": True,
        "tool_name": "self.screen.matrix_16x16.draw_animation",
    }

    if source_frame_count is not None:
        result["source_frame_count"] = int(source_frame_count)
        result["source_frame_interval_ms"] = int(frame_interval_ms)
        result["frame_sampling_applied"] = True

    return result


def render_text_glyph_to_mask_image(glyph: str) -> Image.Image:
    if len(glyph) != 1:
        raise ValueError("Each glyph must be exactly one character long")

    glyph_image = Image.new("1", (MATRIX_WIDTH, MATRIX_HEIGHT), 0)
    if glyph.isspace():
        return glyph_image

    draw_context = ImageDraw.Draw(glyph_image)
    font_paths = [font_path for font_path in MATRIX_TEXT_FONT_CANDIDATES if os.path.isfile(font_path)]

    for font_size in range(16, 7, -1):
        for font_path in font_paths:
            try:
                font = ImageFont.truetype(font_path, size=font_size)
            except OSError:
                continue

            bbox = draw_context.textbbox((0, 0), glyph, font=font)
            if bbox is None:
                continue

            glyph_width = bbox[2] - bbox[0]
            glyph_height = bbox[3] - bbox[1]
            if glyph_width <= 0 or glyph_height <= 0 or glyph_width > MATRIX_WIDTH or glyph_height > MATRIX_HEIGHT:
                continue

            x_pos = (MATRIX_WIDTH - glyph_width) / 2 - bbox[0]
            y_pos = (MATRIX_HEIGHT - glyph_height) / 2 - bbox[1]
            draw_context.text((x_pos, y_pos), glyph, fill=1, font=font)
            if glyph_image.getbbox() is not None:
                return glyph_image
            draw_context.rectangle((0, 0, MATRIX_WIDTH - 1, MATRIX_HEIGHT - 1), fill=0)

    fallback_font = ImageFont.load_default()
    fallback_bbox = draw_context.textbbox((0, 0), glyph, font=fallback_font)
    if fallback_bbox is not None:
        glyph_width = fallback_bbox[2] - fallback_bbox[0]
        glyph_height = fallback_bbox[3] - fallback_bbox[1]
        x_pos = (MATRIX_WIDTH - glyph_width) / 2 - fallback_bbox[0]
        y_pos = (MATRIX_HEIGHT - glyph_height) / 2 - fallback_bbox[1]
        draw_context.text((x_pos, y_pos), glyph, fill=1, font=fallback_font)

    return glyph_image


def render_text_to_matrix_frame_sequence(
    text: str,
    primary_rgb888: str = "",
    background_rgb888: str = "",
    frame_interval_ms: int = DEFAULT_TEXT_FRAME_INTERVAL_MS,
    source: str = "mcp_text",
    transcript: str = "",
) -> Dict[str, Any]:
    text_input = text

    if not text_input.strip():
        raise ValueError("text is required")
    if len(text_input) > MAX_TEXT_FRAME_COUNT:
        raise ValueError(f"text is too long; keep it under {MAX_TEXT_FRAME_COUNT} characters")
    if frame_interval_ms < TEXT_FRAME_INTERVAL_MIN_MS or frame_interval_ms > TEXT_FRAME_INTERVAL_MAX_MS:
        raise ValueError(
            f"frame_interval_ms must be between {TEXT_FRAME_INTERVAL_MIN_MS} and {TEXT_FRAME_INTERVAL_MAX_MS}"
        )

    frames: list[Dict[str, Any]] = []
    total_frames = len(text_input)
    resolved_primary_rgb888 = primary_rgb888 or "#F5F5F5"
    resolved_background_rgb888 = background_rgb888 or "#000000"
    transcript_text = transcript or text_input

    for frame_index, glyph in enumerate(text_input):
        glyph_mask_image = render_text_glyph_to_mask_image(glyph)
        bitmap_rows = build_bitmap_rows_from_mask_image(glyph_mask_image)
        frame_payload = build_matrix_frame_payload_from_bitmap_rows(
            bitmap_rows=bitmap_rows,
            primary_rgb888=resolved_primary_rgb888,
            background_rgb888=resolved_background_rgb888,
            source=source,
            transcript=transcript_text,
            content_type="text",
            text=text_input,
            glyph=glyph,
            frame_index=frame_index,
            frame_count=total_frames,
            label="text_frame",
        )
        frames.append(frame_payload)

    return {
        "data_format": "matrix_frame_sequence_v1",
        "content_type": "text",
        "text": text_input,
        "width": MATRIX_WIDTH,
        "height": MATRIX_HEIGHT,
        "frame_interval_ms": int(frame_interval_ms),
        "frame_count": total_frames,
        "primary_rgb888": format_rgb888(parse_rgb888(resolved_primary_rgb888)),
        "background_rgb888": format_rgb888(parse_rgb888(resolved_background_rgb888)),
        "frames": frames,
        "source": source,
        "transcript": transcript_text,
        "applied": True,
        "tool_name": "self.screen.matrix_16x16.show_text",
    }


def build_png_chunk(chunk_type: bytes, chunk_data: bytes) -> bytes:
    return (
        struct.pack(">I", len(chunk_data))
        + chunk_type
        + chunk_data
        + struct.pack(">I", zlib.crc32(chunk_type + chunk_data) & 0xFFFFFFFF)
    )


def encode_png_rgb_image(width: int, height: int, pixel_rows: list[bytes]) -> bytes:
    if width <= 0 or height <= 0:
        raise ValueError("PNG width and height must be positive")
    if len(pixel_rows) != height:
        raise ValueError("PNG pixel row count does not match image height")

    image_data = bytearray()
    for row in pixel_rows:
        if len(row) != width * 3:
            raise ValueError("PNG pixel row width does not match image width")
        image_data.append(0)
        image_data.extend(row)

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    idat = zlib.compress(bytes(image_data), level=9)
    return b"\x89PNG\r\n\x1a\n" + build_png_chunk(b"IHDR", ihdr) + build_png_chunk(b"IDAT", idat) + build_png_chunk(b"IEND", b"")


def build_matrix_preview_png_bytes(frame_hex: str, pixel_size: int = 12) -> bytes:
    compact_hex = "".join(ch for ch in frame_hex if ch.strip())

    if len(compact_hex) != MATRIX_FRAME_BYTES * 2:
        raise ValueError("frame_rgb332_hex must contain exactly 512 hex characters")
    if pixel_size <= 0:
        raise ValueError("pixel_size must be greater than 0")

    frame_bytes = bytes.fromhex(compact_hex)
    image_width = MATRIX_WIDTH * pixel_size
    image_height = MATRIX_HEIGHT * pixel_size
    pixel_rows: list[bytes] = []

    for row_index in range(MATRIX_HEIGHT):
        row_bytes = bytearray()
        for column_index in range(MATRIX_WIDTH):
            red, green, blue = rgb332_to_rgb888(frame_bytes[row_index * MATRIX_WIDTH + column_index])
            row_bytes.extend(bytes((red, green, blue)) * pixel_size)
        expanded_row = bytes(row_bytes)
        for _ in range(pixel_size):
            pixel_rows.append(expanded_row)

    return encode_png_rgb_image(image_width, image_height, pixel_rows)


def normalize_prompt_text(text: str) -> str:
    return " ".join(text.strip().lower().split())


def find_inline_rgb888(text: str) -> Optional[int]:
    match = re.search(r"#?[0-9a-fA-F]{6}", text)
    if match is None:
        return None
    return parse_rgb888(match.group(0))


def resolve_prompt_color(prompt: str, explicit_rgb888: str, default_rgb888: int) -> int:
    if explicit_rgb888:
        return parse_rgb888(explicit_rgb888)

    inline_rgb = find_inline_rgb888(prompt)
    if inline_rgb is not None:
        return inline_rgb

    lowered = normalize_prompt_text(prompt)
    keyword_groups = {
        "red": ("红", "red", "crimson", "scarlet"),
        "green": ("绿", "绿色", "green", "lime", "emerald"),
        "blue": ("蓝", "蓝色", "blue", "azure"),
        "yellow": ("黄", "黄色", "yellow", "gold"),
        "orange": ("橙", "橙色", "orange"),
        "purple": ("紫", "紫色", "purple", "violet"),
        "pink": ("粉", "粉色", "pink", "rose"),
        "cyan": ("青", "青色", "cyan", "teal", "turquoise", "蓝绿"),
        "white": ("白", "白色", "white", "silver"),
        "black": ("黑", "黑色", "black", "dark"),
    }

    for color_name, keywords in keyword_groups.items():
        if any(keyword in prompt or keyword in lowered for keyword in keywords):
            return PROMPT_COLOR_KEYWORDS[color_name]

    return default_rgb888


def resolve_prompt_template(prompt: str) -> tuple[str, tuple[str, ...]]:
    lowered = normalize_prompt_text(prompt)
    template_keywords = (
        (("爱心", "心形", "heart", "love"), "heart"),
        (("笑脸", "笑", "smile", "happy"), "smile"),
        (("棋盘", "格子", "checker", "grid"), "checker"),
        (("菱形", "diamond", "rhombus"), "diamond"),
        (("圆", "circle", "ring", "环"), "ring"),
        (("箭头", "arrow", "右", "right"), "arrow_right"),
        (("十字", "叉", "cross", "x shape"), "cross"),
        (("边框", "相框", "border", "frame"), "border"),
    )

    for keywords, template_name in template_keywords:
        if any(keyword in prompt or keyword in lowered for keyword in keywords):
            return PROMPT_PATTERN_TEMPLATES[template_name]

    return PROMPT_PATTERN_TEMPLATES["diamond"]


def render_prompt_to_matrix_frame(
    prompt: str,
    primary_rgb888: str = "",
    background_rgb888: str = "",
    source: str = "mcp_prompt",
    transcript: str = "",
) -> Dict[str, Any]:
    prompt_text = prompt.strip() or transcript.strip()

    if not prompt_text:
        raise ValueError("prompt is required")

    resolved_primary_rgb = resolve_prompt_color(prompt_text, primary_rgb888, PROMPT_COLOR_KEYWORDS["cyan"])
    resolved_background_rgb = resolve_prompt_color(prompt_text, background_rgb888, PROMPT_COLOR_KEYWORDS["black"])
    pattern_name, template = resolve_prompt_template(prompt_text)
    bitmap_rows: list[int] = []

    for row_index, row_text in enumerate(template):
        row_bits = 0
        for column_index, pixel_char in enumerate(row_text[:MATRIX_WIDTH]):
            enabled = pixel_char == "#"
            if enabled:
                row_bits |= 1 << (MATRIX_WIDTH - 1 - column_index)
        bitmap_rows.append(row_bits)

    prompt_payload = build_matrix_frame_payload_from_bitmap_rows(
        bitmap_rows=bitmap_rows,
        primary_rgb888=format_rgb888(resolved_primary_rgb),
        background_rgb888=format_rgb888(resolved_background_rgb),
        source=source,
        transcript=transcript or prompt_text,
        content_type="prompt_template",
        prompt=prompt_text,
        pattern=pattern_name,
    )
    prompt_payload["tool_name"] = "self.screen.matrix_16x16.render_prompt"
    return prompt_payload


def build_random_matrix_frame_payload(transcript: str, source: str = "debug_ws") -> Dict[str, Any]:
    pattern_name = random.choice(tuple(PROMPT_PATTERN_TEMPLATES.keys()))
    primary_rgb888 = format_rgb888(random.choice(tuple(PROMPT_COLOR_KEYWORDS.values())))

    return render_prompt_to_matrix_frame(
        prompt=pattern_name,
        primary_rgb888=primary_rgb888,
        background_rgb888="#000000",
        source=source,
        transcript=transcript or f"random pattern {pattern_name}",
    )


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
        {
            "name": "self.screen.matrix_16x16.draw_frame",
            "description": "Low-level tool. Return or deliver one 16x16 matrix frame when you already have frame_rgb332_hex or the compact 38-byte LED bitmap format.",
            "inputSchema": {
                "type": "object",
                "properties": {
                    "preset": {
                        "type": "string",
                        "enum": ["", "python_demo"],
                        "description": "Optional built-in preset. Prefer empty when you provide explicit frame data.",
                    },
                    "frame_rgb332_hex": {
                        "type": "string",
                        "description": "Exactly 512 hex characters. One RGB332 byte per pixel for the full 16x16 frame.",
                    },
                    "bitmap_rows_hex": {
                        "type": "string",
                        "description": "Optional compact bitmap field only. Exactly 64 hex characters = 16 rows x 2 bytes = 32 bitmap bytes. Bit 1 means LED on, bit 0 means off; rows are top to bottom, and the high bit in each 16-bit row is the leftmost LED. Together with primary_rgb888 and background_rgb888 it forms one 38-byte LED-ready frame.",
                    },
                    "bitmap_ascii": build_bitmap_ascii_schema(),
                    "primary_rgb888": {
                        "type": "string",
                        "description": "Required with bitmap_rows_hex. Foreground color in #RRGGBB. Stored as RGB888 (3 bytes) after the 32-byte bitmap.",
                    },
                    "background_rgb888": {
                        "type": "string",
                        "description": "Optional background color for bitmap_rows_hex. Defaults to #000000. Stored as RGB888 (3 bytes) after the foreground color, so bitmap_rows_hex + primary_rgb888 + background_rgb888 = 38 bytes.",
                    },
                    "source": {"type": "string"},
                    "transcript": {"type": "string"},
                },
            },
        },
        {
            "name": "self.screen.matrix_16x16.draw_python",
            "description": "Primary LLM drawing tool. Execute restricted Pillow ImageDraw-style Python statements, evaluate restricted Python expressions with eval(), and return one unified 16x16 frame payload.",
            "inputSchema": {
                "type": "object",
                "properties": {
                    "python_source": {
                        "type": "string",
                        "description": "Restricted Python drawing statements. Use draw.point, draw.line, draw.rectangle, draw.ellipse, draw.polygon, helper functions like line(...), fill_rectangle(...), fill_circle(...), or eval(\"<expression>\") for nested dynamic drawing.",
                    },
                    "eval_source": {
                        "type": "string",
                        "description": "Restricted Python expression evaluated with eval(). Prefer this for comprehensions, conditional expressions, or other dynamic draw patterns. If both python_source and eval_source are provided, python_source runs first.",
                    },
                    "ops": build_draw_python_ops_schema(),
                    "primary_rgb888": {
                        "type": "string",
                        "description": "Foreground color in #RRGGBB. All painted pixels use this color.",
                    },
                    "background_rgb888": {
                        "type": "string",
                        "description": "Background color in #RRGGBB. Empty pixels use this color.",
                    },
                    "source": {"type": "string"},
                    "transcript": {"type": "string"},
                },
                "anyOf": [
                    {"required": ["python_source"]},
                    {"required": ["eval_source"]},
                    {"required": ["ops"]}
                ],
            },
        },
        {
            "name": "self.screen.matrix_16x16.show_text",
            "description": "Convert text into a 16x16 frame sequence and deliver each frame to the AI preview through the debug websocket when available.",
            "inputSchema": {
                "type": "object",
                "properties": {
                    "text": {
                        "type": "string",
                        "description": "The text content to render. Each character becomes one 16x16 frame.",
                    },
                    "frame_interval_ms": {
                        "type": "integer",
                        "minimum": 20,
                        "maximum": 5000,
                        "description": "Delay between consecutive frames sent to the AI preview sequence.",
                    },
                    "primary_rgb888": {"type": "string"},
                    "background_rgb888": {"type": "string"},
                    "source": {"type": "string"},
                    "transcript": {"type": "string"},
                },
                "required": ["text"],
            },
        },
        {
            "name": "self.screen.matrix_16x16.draw_animation",
            "description": "Transmit a compact 16x16 bitmap animation sequence for LED-side buffered playback. Each frame is a 32-byte 1-bit bitmap plus foreground/background RGB888, for a total compact frame size of 38 bytes. The LED side buffers 24 frames; if more frames are supplied, the bridge resamples them to 24 and scales frame_interval_ms to preserve the overall duration. Do not implement timing in python_source with sleep/yield style logic; animation timing is controlled only by frame_interval_ms.",
            "inputSchema": {
                "type": "object",
                "properties": {
                    "bitmap_rows_hex_list": {
                        "type": "array",
                        "description": "An array of animation frame masks. Preferred: each item is 64 hex characters = 32 bitmap bytes. Compatibility: if this field itself is exactly 16 row tokens (1-4 hex digits), it will be treated as one frame rather than 16 frames. Bit 1 means LED on, bit 0 means off; rows are top to bottom, and the high bit in each 16-bit row is the leftmost LED. Together with primary_rgb888 and background_rgb888, each frame becomes one 38-byte LED-ready compact frame.",
                        "minItems": 1,
                        "maxItems": 96,
                        "items": {
                            "type": "string"
                        }
                    },
                    "frames": {
                        "type": "array",
                        "description": "Alternative animation input. Each frame object must provide exactly one source: bitmap_rows_hex (64 hex), bitmap_rows (16 row values), bitmap_ascii (16x16 ASCII), or python_source/eval_source/ops. This mode is recommended when LLM output is easier to express as row arrays, ASCII patterns, structured ops, or restricted Python drawing snippets.",
                        "minItems": 1,
                        "maxItems": 96,
                        "items": {
                            "type": "object",
                            "properties": {
                                "bitmap_rows_hex": {"type": "string"},
                                "bitmap_rows": {
                                    "type": "array",
                                    "items": {
                                        "type": "string"
                                    }
                                },
                                "bitmap_ascii": build_bitmap_ascii_schema(),
                                "python_source": {"type": "string"},
                                "eval_source": {"type": "string"},
                                "ops": build_draw_python_ops_schema(),
                                "primary_rgb888": {"type": "string"},
                                "background_rgb888": {"type": "string"},
                                "transcript": {"type": "string"}
                            }
                        }
                    },
                    "image": {
                        "type": "object",
                        "description": "Base image source for effect-driven animation mode. Provide exactly one source: bitmap_rows_hex, bitmap_rows, bitmap_ascii, or python_source/eval_source/ops.",
                        "properties": {
                            "bitmap_rows_hex": {"type": "string"},
                            "bitmap_rows": {
                                "type": "array",
                                "items": {"type": "string"}
                            },
                            "bitmap_ascii": build_bitmap_ascii_schema(),
                            "python_source": {"type": "string"},
                            "eval_source": {"type": "string"},
                            "ops": build_draw_python_ops_schema(),
                            "primary_rgb888": {"type": "string"},
                            "background_rgb888": {"type": "string"},
                            "transcript": {"type": "string"}
                        }
                    },
                    "effect": {
                        "type": "object",
                        "description": "Effect parameters to synthesize animation from one base image. Supported names: blink, breathe, marquee, marquee_left, marquee_right.",
                        "properties": {
                            "name": {"type": "string"},
                            "frame_count": {"type": "integer", "minimum": 2, "maximum": 96},
                            "duty_cycle": {"type": "number"},
                            "step": {"type": "integer", "minimum": 1, "maximum": 16},
                            "direction": {"type": "string"},
                            "min_density": {"type": "number"},
                            "max_density": {"type": "number"}
                        },
                        "required": ["name"]
                    },
                    "frame_interval_ms": {
                        "type": "integer",
                        "minimum": 1,
                        "maximum": 65535,
                        "description": "Delay between consecutive frames in milliseconds. Use 42 for the default near-24 fps playback.",
                    },
                    "primary_rgb888": {
                        "type": "string",
                        "description": "Foreground color in #RRGGBB. All set bits use this color, stored as 3 RGB888 bytes after the 32-byte bitmap.",
                    },
                    "background_rgb888": {
                        "type": "string",
                        "description": "Background color in #RRGGBB. Empty pixels use this color, stored as 3 RGB888 bytes after the foreground color.",
                    },
                    "source": {"type": "string"},
                    "transcript": {"type": "string"},
                },
                "anyOf": [
                    {"required": ["bitmap_rows_hex_list"]},
                    {"required": ["frames"]},
                    {"required": ["image", "effect"]}
                ],
            },
        },
        {
            "name": "self.screen.matrix_16x16.render_prompt",
            "description": "Legacy helper. Render a free-text prompt into one template-based 16x16 frame payload.",
            "inputSchema": {
                "type": "object",
                "properties": {
                    "prompt": {"type": "string"},
                    "primary_rgb888": {"type": "string"},
                    "background_rgb888": {"type": "string"},
                    "source": {"type": "string"},
                    "transcript": {"type": "string"},
                },
                "required": ["prompt"],
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


def build_device_preview_url(device_ip: str, device_port: int = DEFAULT_DEVICE_PREVIEW_PORT) -> str:
    normalized_ip = device_ip.strip()

    if not normalized_ip:
        raise ValueError("device_ip is required")
    if device_port <= 0 or device_port > 65535:
        raise ValueError("device_port must be between 1 and 65535")

    return f"http://{normalized_ip}:{device_port}{DEFAULT_DEVICE_PREVIEW_PATH}"


def build_device_preview_status_url(device_ip: str, device_port: int = DEFAULT_DEVICE_PREVIEW_PORT) -> str:
    normalized_ip = device_ip.strip()

    if not normalized_ip:
        raise ValueError("device_ip is required")
    if device_port <= 0 or device_port > 65535:
        raise ValueError("device_port must be between 1 and 65535")

    return f"http://{normalized_ip}:{device_port}{DEFAULT_DEVICE_PREVIEW_STATUS_PATH}"


def fetch_device_preview_status(
    *,
    device_ip: str,
    device_port: int = DEFAULT_DEVICE_PREVIEW_PORT,
    timeout_seconds: float = DEFAULT_CONTROL_TIMEOUT,
) -> Dict[str, Any]:
    request_url = build_device_preview_status_url(device_ip, device_port)

    if timeout_seconds <= 0:
        raise ValueError("timeout must be greater than 0")

    request = urllib_request.Request(request_url, method="GET")

    try:
        with urllib_request.urlopen(request, timeout=timeout_seconds) as response:
            response_text = response.read().decode("utf-8")
            response_payload = json.loads(response_text) if response_text else {}
    except urllib_error.HTTPError as exc:
        error_text = exc.read().decode("utf-8", errors="replace")
        raise RuntimeError(f"device preview status failed: HTTP {exc.code} {error_text}") from exc
    except urllib_error.URLError as exc:
        raise RuntimeError(f"device preview status failed: {exc.reason}") from exc

    if not isinstance(response_payload, dict):
        raise RuntimeError("device preview status returned invalid JSON")

    response_payload["request_url"] = request_url
    return response_payload


def upload_image_to_device_preview(
    *,
    device_ip: str,
    image_path: str,
    device_port: int = DEFAULT_DEVICE_PREVIEW_PORT,
    timeout_seconds: float = DEFAULT_CONTROL_TIMEOUT,
) -> Dict[str, Any]:
    normalized_path = os.path.abspath(image_path)
    content_type, _ = mimetypes.guess_type(normalized_path)

    if timeout_seconds <= 0:
        raise ValueError("timeout must be greater than 0")
    if not os.path.isfile(normalized_path):
        raise ValueError(f"image_path does not exist: {normalized_path}")
    if content_type not in {"image/png", "image/jpeg"}:
        raise ValueError("image_path must point to a PNG or JPEG file")

    with open(normalized_path, "rb") as image_file:
        image_bytes = image_file.read()

    return upload_bytes_to_device_preview(
        device_ip=device_ip,
        image_bytes=image_bytes,
        content_type=content_type,
        device_port=device_port,
        timeout_seconds=timeout_seconds,
        image_path=normalized_path,
    )


def upload_bytes_to_device_preview(
    *,
    device_ip: str,
    image_bytes: bytes,
    content_type: str,
    device_port: int = DEFAULT_DEVICE_PREVIEW_PORT,
    timeout_seconds: float = DEFAULT_CONTROL_TIMEOUT,
    image_path: str = "<memory>",
) -> Dict[str, Any]:
    request_url = build_device_preview_url(device_ip, device_port)

    if timeout_seconds <= 0:
        raise ValueError("timeout must be greater than 0")
    if content_type not in {"image/png", "image/jpeg"}:
        raise ValueError("content_type must be image/png or image/jpeg")
    if not image_bytes:
        raise ValueError("image_bytes must not be empty")

    request = urllib_request.Request(
        request_url,
        data=image_bytes,
        headers={"Content-Type": content_type},
        method="POST",
    )

    try:
        with urllib_request.urlopen(request, timeout=timeout_seconds) as response:
            response_text = response.read().decode("utf-8")
            response_payload = json.loads(response_text) if response_text else {}
    except urllib_error.HTTPError as exc:
        error_text = exc.read().decode("utf-8", errors="replace")
        raise RuntimeError(f"device preview upload failed: HTTP {exc.code} {error_text}") from exc
    except urllib_error.URLError as exc:
        raise RuntimeError(f"device preview upload failed: {exc.reason}") from exc

    return {
        "device_ip": device_ip,
        "device_port": device_port,
        "image_path": image_path,
        "content_type": content_type,
        "bytes": len(image_bytes),
        "request_url": request_url,
        "device_response": response_payload,
    }


def upload_matrix_preview_frame(
    *,
    frame_hex: str,
    output_dir: str,
    label: str = "matrix_preview",
) -> str:
    png_bytes = build_matrix_preview_png_bytes(frame_hex)
    return save_snapshot_bytes(
        png_bytes,
        "image/png",
        MATRIX_WIDTH,
        MATRIX_HEIGHT,
        100,
        0,
        output_dir,
        label,
    )


def build_generated_image_url(status: ServerStatus, image_path: str) -> str:
    snapshot_url = status.snapshot_url.strip()
    parsed = urlparse(snapshot_url)

    if not parsed.scheme or not parsed.netloc:
        raise RuntimeError("HTTP server is not ready to serve generated images")

    return f"{parsed.scheme}://{parsed.netloc}/generated/{quote(os.path.basename(image_path))}"


def find_latest_preview_image(image_dir: str) -> Optional[str]:
    candidate_paths: list[str] = []

    if not os.path.isdir(image_dir):
        return None

    for entry_name in os.listdir(image_dir):
        entry_path = os.path.join(image_dir, entry_name)
        if not os.path.isfile(entry_path):
            continue
        if mimetypes.guess_type(entry_path)[0] not in {"image/png", "image/jpeg"}:
            continue
        candidate_paths.append(entry_path)

    if not candidate_paths:
        return None

    return max(candidate_paths, key=os.path.getmtime)


def upload_startup_preview_image(
    image_dir: str,
    timeout_seconds: float = DEFAULT_STARTUP_PREVIEW_TIMEOUT,
) -> Optional[Dict[str, Any]]:
    image_path = find_latest_preview_image(image_dir)

    if image_path is None:
        print(f"[startup] skip auto preview upload: no PNG/JPEG found under {image_dir}")
        return None

    result_payload = upload_image_to_device_preview(
        device_ip=DEFAULT_DEVICE_PREVIEW_IP,
        image_path=image_path,
        device_port=DEFAULT_DEVICE_PREVIEW_PORT,
        timeout_seconds=timeout_seconds,
    )
    print(
        "[startup] auto preview upload sent "
        f"device_ip={DEFAULT_DEVICE_PREVIEW_IP} "
        f"image_path={result_payload['image_path']}"
    )
    return result_payload


async def wait_for_device_preview_ready() -> Dict[str, Any]:
    waited_seconds = 0.0
    last_error = "device preview status was not ready"

    # Wait for the AI-side preview HTTP server before sending the startup image.
    while waited_seconds < DEFAULT_STARTUP_PREVIEW_READY_WAIT_SECONDS:
        try:
            status_payload = await asyncio.to_thread(
                fetch_device_preview_status,
                device_ip=DEFAULT_DEVICE_PREVIEW_IP,
                device_port=DEFAULT_DEVICE_PREVIEW_PORT,
                timeout_seconds=DEFAULT_STARTUP_PREVIEW_POLL_SECONDS,
            )
        except Exception as exc:
            last_error = str(exc)
        else:
            if bool(status_payload.get("ready", False)):
                return status_payload
            last_error = str(status_payload.get("status_text", "device preview status was not ready"))

        await asyncio.sleep(DEFAULT_STARTUP_PREVIEW_POLL_SECONDS)
        waited_seconds += DEFAULT_STARTUP_PREVIEW_POLL_SECONDS

    raise RuntimeError(last_error)


async def run_startup_preview_upload(status: ServerStatus, image_dir: str) -> None:
    image_path = find_latest_preview_image(image_dir)

    if image_path is None:
        update_device_preview_status(status, "skipped", f"no PNG/JPEG found under {image_dir}")
        print(f"[startup] skip auto preview upload: no PNG/JPEG found under {image_dir}")
        return

    update_device_preview_status(
        status,
        "pending",
        f"waiting for device preview HTTP ready; image_path={image_path}",
    )

    try:
        status_payload = await wait_for_device_preview_ready()
    except Exception as exc:
        update_device_preview_status(status, "error", str(exc))
        print(f"[startup] auto preview upload failed before send: {exc}")
        return

    update_device_preview_status(
        status,
        "ready",
        str(status_payload.get("status_text", "device preview HTTP ready")),
    )

    # Retry the first automatic upload because Wi-Fi preview can come up slightly before the receiver settles.
    for attempt_index in range(DEFAULT_STARTUP_PREVIEW_RETRY_COUNT):
        try:
            result_payload = await asyncio.to_thread(
                upload_startup_preview_image,
                image_dir,
                DEFAULT_STARTUP_PREVIEW_TIMEOUT,
            )
        except Exception as exc:
            update_device_preview_status(
                status,
                "error",
                f"attempt={attempt_index + 1}/{DEFAULT_STARTUP_PREVIEW_RETRY_COUNT} {exc}",
            )
            print(f"[startup] auto preview upload failed: {exc}")
            if attempt_index + 1 >= DEFAULT_STARTUP_PREVIEW_RETRY_COUNT:
                return
            await asyncio.sleep(DEFAULT_STARTUP_PREVIEW_POLL_SECONDS)
            continue
        break
    else:
        return

    if result_payload is None:
        update_device_preview_status(status, "skipped", f"no PNG/JPEG found under {image_dir}")
        return

    status.control_calls += 1
    update_device_preview_status(
        status,
        "sent",
        f"device_ip={DEFAULT_DEVICE_PREVIEW_IP} image_path={result_payload['image_path']}",
    )


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

    if tool_name in DRAW_FRAME_TOOL_NAMES:
        preset_name = str(arguments.get("preset", ""))
        matrix_frame_rgb332_hex = str(arguments.get("frame_rgb332_hex", ""))
        bitmap_rows_hex = str(arguments.get("bitmap_rows_hex", ""))
        bitmap_ascii = arguments.get("bitmap_ascii")
        primary_rgb888 = str(arguments.get("primary_rgb888", ""))
        background_rgb888 = str(arguments.get("background_rgb888", "#000000"))
        normalized_frame_rgb332_hex = "".join(ch for ch in matrix_frame_rgb332_hex if ch.strip())
        normalized_bitmap_rows_hex = ""
        tool_source = str(arguments.get("source", "mcp"))
        transcript_text = str(arguments.get("transcript", ""))

        if str(bitmap_rows_hex).strip():
            normalized_bitmap_rows_hex = normalize_bitmap_rows_hex_value(bitmap_rows_hex, "bitmap_rows_hex")
        elif bitmap_ascii not in (None, "", []):
            normalized_bitmap_rows_hex = normalize_bitmap_ascii_value(bitmap_ascii, "bitmap_ascii")

        if not preset_name and not normalized_frame_rgb332_hex and not normalized_bitmap_rows_hex:
            raise ValueError("Either preset, frame_rgb332_hex, bitmap_rows_hex, or bitmap_ascii is required")
        if preset_name and preset_name != "python_demo":
            raise ValueError(f"Unsupported preset: {preset_name}")
        if normalized_frame_rgb332_hex:
            if len(normalized_frame_rgb332_hex) != 512:
                raise ValueError("frame_rgb332_hex must contain 512 hex characters")
            int(normalized_frame_rgb332_hex, 16)
        if normalized_bitmap_rows_hex:
            parse_rgb888(primary_rgb888)

        if background_rgb888:
            parse_rgb888(background_rgb888)

        if normalized_bitmap_rows_hex:
            bitmap_rows = [int(normalized_bitmap_rows_hex[index:index + 4], 16) for index in range(0, 64, 4)]
            frame_payload = build_matrix_frame_payload_from_bitmap_rows(
                bitmap_rows=bitmap_rows,
                primary_rgb888=primary_rgb888,
                background_rgb888=background_rgb888 or "#000000",
                source=tool_source,
                transcript=transcript_text,
                content_type="frame",
                label="draw_frame",
            )
            frame_payload["tool_name"] = "self.screen.matrix_16x16.draw_frame"
            if preset_name:
                frame_payload["preset"] = preset_name
            return frame_payload

        result_payload: Dict[str, Any] = {
            "data_format": "matrix_frame_v1",
            "content_type": "frame",
            "preset": preset_name,
            "frame_rgb332_hex": normalized_frame_rgb332_hex,
            "bitmap_rows_hex": normalized_bitmap_rows_hex,
            "primary_rgb888": primary_rgb888,
            "background_rgb888": background_rgb888,
            "width": 16,
            "height": 16,
            "source": tool_source,
            "transcript": transcript_text,
            "applied": True,
            "tool_name": "self.screen.matrix_16x16.draw_frame",
        }

        if normalized_frame_rgb332_hex and not normalized_bitmap_rows_hex:
            compact_fields = try_extract_binary_bitmap_fields_from_frame_hex(normalized_frame_rgb332_hex)
            if compact_fields is not None:
                compact_bitmap_rows_hex, compact_primary_rgb888, compact_background_rgb888 = compact_fields
                result_payload["bitmap_rows_hex"] = compact_bitmap_rows_hex
                result_payload["compact_frame_format"] = build_compact_bitmap_format_metadata()
                if not result_payload["primary_rgb888"]:
                    result_payload["primary_rgb888"] = compact_primary_rgb888
                if not result_payload["background_rgb888"]:
                    result_payload["background_rgb888"] = compact_background_rgb888

        return result_payload

    if tool_name in PYTHON_DRAW_TOOL_NAMES:
        return render_python_source_to_matrix_frame(
            python_source=str(arguments.get("python_source", "")),
            eval_source=str(arguments.get("eval_source", "")),
            draw_ops=arguments.get("ops"),
            primary_rgb888=str(arguments.get("primary_rgb888", "#F5F5F5")),
            background_rgb888=str(arguments.get("background_rgb888", "#000000")),
            source=str(arguments.get("source", "mcp_python")),
            transcript=str(arguments.get("transcript", "")),
        )

    if tool_name in TEXT_SEQUENCE_TOOL_NAMES:
        return render_text_to_matrix_frame_sequence(
            text=str(arguments.get("text", "")),
            primary_rgb888=str(arguments.get("primary_rgb888", "#F5F5F5")),
            background_rgb888=str(arguments.get("background_rgb888", "#000000")),
            frame_interval_ms=int(arguments.get("frame_interval_ms", DEFAULT_TEXT_FRAME_INTERVAL_MS)),
            source=str(arguments.get("source", "mcp_text")),
            transcript=str(arguments.get("transcript", "")),
        )

    if tool_name in ANIMATION_SEQUENCE_TOOL_NAMES:
        resolved_primary_rgb888 = str(arguments.get("primary_rgb888", "#F5F5F5"))
        resolved_background_rgb888 = str(arguments.get("background_rgb888", "#000000"))
        resolved_source = str(arguments.get("source", "mcp_animation"))
        animation_sources = resolve_animation_bitmap_rows_hex_sources(
            bitmap_rows_hex_list=arguments.get("bitmap_rows_hex_list", []),
            frames=arguments.get("frames", []),
            image=arguments.get("image"),
            effect=arguments.get("effect"),
            primary_rgb888=resolved_primary_rgb888,
            background_rgb888=resolved_background_rgb888,
            source=resolved_source,
        )

        animation_result = render_bitmap_animation_frame_sequence(
            bitmap_rows_hex_list=animation_sources,
            primary_rgb888=resolved_primary_rgb888,
            background_rgb888=resolved_background_rgb888,
            frame_interval_ms=int(arguments.get("frame_interval_ms", DEFAULT_ANIMATION_FRAME_INTERVAL_MS)),
            source=resolved_source,
            transcript=str(arguments.get("transcript", "")),
        )

        if isinstance(arguments.get("effect"), dict):
            animation_result["effect"] = arguments.get("effect")
        if arguments.get("image") not in (None, "", []):
            animation_result["effect_mode"] = "image_plus_effect"

        return animation_result

    if tool_name in PROMPT_RENDER_TOOL_NAMES:
        return render_prompt_to_matrix_frame(
            prompt=str(arguments.get("prompt", "")),
            primary_rgb888=str(arguments.get("primary_rgb888", "")),
            background_rgb888=str(arguments.get("background_rgb888", "")),
            source=str(arguments.get("source", "mcp_prompt")),
            transcript=str(arguments.get("transcript", "")),
        )

    raise KeyError(f"Unknown tool: {tool_name}")


class McpBridgeServer:
    def __init__(
        self,
        client: McpWebSocketClient,
        output_dir: str,
        status: ServerStatus,
        debug_ws_server: Optional["LocalDebugWebSocketServer"] = None,
    ):
        self.client = client
        self.output_dir = output_dir
        self.status = status
        self.debug_ws_server = debug_ws_server
        self._receiver_task: Optional[asyncio.Task[None]] = None
        self._next_id = 1
        self._pending_requests: Dict[int, asyncio.Future[Dict[str, Any]]] = {}
        self._send_lock = asyncio.Lock()

    async def __aenter__(self) -> "McpBridgeServer":
        await self.client.__aenter__()
        update_status(
            self.status,
            connected=True,
            connection_state="websocket_connected",
            connection_detail=self.client.url,
            last_event="connected",
            last_error="-",
        )
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
        update_status(
            self.status,
            connected=False,
            initialized=False,
            connection_state="disconnected",
            connection_detail="websocket closed",
        )
        await self.client.__aexit__(exc_type, exc, tb)

    async def dispatch_remote_preview_fetch(
        self,
        *,
        image_url: str,
        transcript: str,
        timeout_seconds: float,
    ) -> None:
        try:
            result_payload = await self.call_remote_tool(
                "self.screen.preview_image.fetch_http",
                {
                    "url": image_url,
                    "source": "host_http",
                    "transcript": transcript,
                },
                timeout_seconds,
            )
        except Exception as exc:
            update_device_preview_status(self.status, "error", f"AI fetch failed: {exc}")
            print(f"[preview] fetch error url={image_url} error={exc}")
            return

        update_device_preview_status(
            self.status,
            "fetched",
            f"url={image_url} bytes={result_payload.get('bytes', 0)}",
        )
        print(f"[preview] fetched url={image_url} bytes={result_payload.get('bytes', 0)}")

    async def wait_closed(self) -> None:
        if self._receiver_task is not None:
            await self._receiver_task

    async def deliver_matrix_payload_via_debug_ws(self, payload: Dict[str, Any]) -> Dict[str, Any]:
        if self.debug_ws_server is None:
            raise RuntimeError("AI debug websocket server is not configured")

        frames = payload.get("frames")
        if isinstance(frames, list) and frames:
            frame_interval_ms = int(payload.get("frame_interval_ms", DEFAULT_TEXT_FRAME_INTERVAL_MS))
            frame_interval_ms = max(1, frame_interval_ms)

            await self.debug_ws_server.send_json({
                "type": "matrix_animation_start",
                "content_type": payload.get("content_type", "animation"),
                "frame_count": len(frames),
                "frame_interval_ms": frame_interval_ms,
                "source": payload.get("source", ""),
                "transcript": payload.get("transcript", ""),
            })

            for frame_payload in frames:
                if not isinstance(frame_payload, dict):
                    raise ValueError("frames must contain JSON objects")

                normalized_frame_payload = dict(frame_payload)
                normalized_frame_payload["bitmap_rows_hex"] = normalize_bitmap_rows_hex_value(
                    frame_payload.get("bitmap_rows_hex", ""),
                    "frames[].bitmap_rows_hex",
                )
                normalized_frame_payload["primary_rgb888"] = normalize_rgb888_color(
                    frame_payload.get("primary_rgb888", ""),
                    "frames[].primary_rgb888",
                )
                normalized_frame_payload["background_rgb888"] = normalize_rgb888_color(
                    frame_payload.get("background_rgb888", ""),
                    "frames[].background_rgb888",
                )
                normalized_frame_payload["compact_frame_format"] = build_compact_bitmap_format_metadata()

                await self.debug_ws_server.send_json({
                    "type": "matrix_pattern_result",
                    "frame_interval_ms": frame_interval_ms,
                    **normalized_frame_payload,
                })
                update_matrix_status(self.status, normalized_frame_payload, "debug_ws_sent")

            await self.debug_ws_server.send_json({
                "type": "matrix_animation_end",
                "content_type": payload.get("content_type", "animation"),
                "frame_count": len(frames),
                "frame_interval_ms": frame_interval_ms,
                "source": payload.get("source", ""),
                "transcript": payload.get("transcript", ""),
            })

            return {
                "requested": True,
                "transport": "debug_ws",
                "sent": True,
                "frame_count": len(frames),
                "frame_interval_ms": frame_interval_ms,
            }

        normalized_payload = dict(payload)
        normalized_payload["bitmap_rows_hex"] = normalize_bitmap_rows_hex_value(
            payload.get("bitmap_rows_hex", ""),
            "bitmap_rows_hex",
        )
        normalized_payload["primary_rgb888"] = normalize_rgb888_color(
            payload.get("primary_rgb888", ""),
            "primary_rgb888",
        )
        normalized_payload["background_rgb888"] = normalize_rgb888_color(
            payload.get("background_rgb888", ""),
            "background_rgb888",
        )
        normalized_payload["compact_frame_format"] = build_compact_bitmap_format_metadata()

        await self.debug_ws_server.send_json({
            "type": "matrix_pattern_result",
            **normalized_payload,
        })
        update_matrix_status(self.status, normalized_payload, "debug_ws_sent")
        return {
            "requested": True,
            "transport": "debug_ws",
            "sent": True,
            "frame_count": 1,
        }



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
            record_call_status(
                self.status,
                call_type="remote_tool",
                name=tool_name,
                ok=False,
                event="remote_tool_timeout",
                error="Timed out waiting for remote tool result",
                result={"arguments": arguments},
            )
            raise TimeoutError("Timed out waiting for remote tool result") from exc

        error = message.get("error")
        if isinstance(error, dict):
            error_text = str(error.get("message", "Unknown MCP error"))
            record_call_status(
                self.status,
                call_type="remote_tool",
                name=tool_name,
                ok=False,
                event="remote_tool_error",
                error=error_text,
                result={"arguments": arguments},
            )
            raise RuntimeError(error_text)

        result_payload = extract_tool_payload(message.get("result", {}))
        record_call_status(
            self.status,
            call_type="remote_tool",
            name=tool_name,
            ok=True,
            event="remote_tool_result",
            result=result_payload,
        )
        return result_payload

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
                    "serverInfo": {"name": LOCAL_MCP_SERVER_NAME, "version": "1.0.0"},
                },
            )
            update_status(
                self.status,
                initialized=True,
                connection_state="ready",
                connection_detail="initialize acknowledged",
                last_event="initialize",
                last_error="-",
            )
            return

        if method == "notifications/initialized":
            update_status(
                self.status,
                initialized=True,
                connection_state="ready",
                connection_detail="notifications initialized received",
                last_event="notifications_initialized",
                last_error="-",
            )
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
                delivery_sent = False
                delivery_warning = ""
                if not isinstance(arguments, dict):
                    raise TypeError("tools/call arguments must be an object")
                result_payload = handle_local_tool_call(tool_name, arguments)

                if tool_name in DEBUG_WS_DELIVERY_TOOL_NAMES:
                    try:
                        result_payload["delivery"] = await self.deliver_matrix_payload_via_debug_ws(result_payload)
                        delivery_sent = True
                    except Exception as exc:
                        delivery_warning = str(exc)
                        result_payload["delivery"] = {
                            "requested": True,
                            "transport": "debug_ws",
                            "sent": False,
                            "error": delivery_warning,
                        }

                if tool_name in HTTP_PREVIEW_FALLBACK_TOOL_NAMES and not delivery_sent:
                    update_matrix_status(self.status, result_payload, "generated_http_preview")
                    frame_hex = str(result_payload.get("frame_rgb332_hex", ""))
                    if frame_hex:
                        try:
                            saved_path = upload_matrix_preview_frame(
                                frame_hex=frame_hex,
                                output_dir=self.output_dir,
                                label="matrix_http_preview",
                            )
                            preview_url = build_generated_image_url(self.status, saved_path)
                            result_payload["preview_image_url"] = preview_url
                            result_payload["saved_path"] = saved_path
                            update_status(self.status, saved_path=saved_path)
                            update_device_preview_status(
                                self.status,
                                "pending",
                                f"waiting for AI fetch url={preview_url}",
                            )
                            asyncio.create_task(
                                self.dispatch_remote_preview_fetch(
                                    image_url=preview_url,
                                    transcript=str(result_payload.get("transcript", "")),
                                    timeout_seconds=DEFAULT_CONTROL_TIMEOUT,
                                )
                            )
                            result_payload["delivery"] = {
                                "requested": True,
                                "transport": "ai_pull_http",
                                "sent": True,
                                "preview_image_url": preview_url,
                                "saved_path": saved_path,
                            }
                        except Exception as exc:
                            result_payload["preview_error"] = str(exc)
                            update_device_preview_status(self.status, "error", str(exc))
                    else:
                        result_payload["preview_note"] = "HTTP preview skipped because frame_rgb332_hex is empty"
                        if delivery_warning:
                            result_payload["delivery_note"] = delivery_warning

                if delivery_warning and delivery_sent:
                    result_payload["delivery_warning"] = delivery_warning
                if request_id is not None:
                    await self.client.respond(request_id, result=tool_result_content(result_payload))
                self.status.tool_calls += 1
                record_call_status(
                    self.status,
                    call_type="local_tool",
                    name=tool_name,
                    ok=True,
                    event="tool_called",
                    result=result_payload,
                )
                print(f"[tool] name={tool_name} result={json.dumps(result_payload, ensure_ascii=False)}")
                return
            except (KeyError, TypeError, ValueError) as exc:
                record_call_status(
                    self.status,
                    call_type="local_tool",
                    name=str(params.get("name", "")),
                    ok=False,
                    event="error",
                    error=str(exc),
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


class LocalDebugWebSocketServer:
    def __init__(self, host: str, port: int, path: str, status: ServerStatus):
        self.host = host
        self.port = port
        self.path = path
        self.status = status
        self._server = None
        self._client = None
        self._client_lock = asyncio.Lock()

    async def start(self) -> None:
        self._server = await websockets.serve(self._handle_client, self.host, self.port)
        public_host = detect_local_ip() if self.host == "0.0.0.0" else self.host
        debug_ws_url = f"ws://{public_host}:{self.port}{self.path}"

        update_status(
            self.status,
            debug_ws_url=debug_ws_url,
            debug_ws_client_state="listening",
            debug_ws_client_detail="waiting for AI client",
            debug_ws_last_message="-",
        )
        print(f"[debug_ws] listening url={debug_ws_url}")

    async def stop(self) -> None:
        if self._server is None:
            return

        self._server.close()
        await self._server.wait_closed()
        self._server = None
        async with self._client_lock:
            self._client = None
        update_debug_ws_status(self.status, "stopped", "server closed")

    async def send_json(self, payload: Dict[str, Any]) -> None:
        async with self._client_lock:
            if self._client is None:
                raise RuntimeError("AI debug websocket client is not connected")
            await self._client.send(json.dumps(payload, ensure_ascii=False))
        update_debug_ws_status(self.status, "sent", "message delivered to AI client", json.dumps(payload, ensure_ascii=False))
        print(f"[debug_ws] tx {json.dumps(payload, ensure_ascii=False)}")

    async def _handle_client(self, websocket) -> None:
        request_path = getattr(websocket, "path", self.path)

        if request_path != self.path:
            await websocket.close(code=1008, reason="Unsupported path")
            return

        remote_address = getattr(websocket, "remote_address", None)
        remote_text = str(remote_address) if remote_address is not None else "<unknown>"

        async with self._client_lock:
            self._client = websocket

        update_debug_ws_status(self.status, "connected", remote_text, "client connected")
        print(f"[debug_ws] client connected remote={remote_text}")

        try:
            async for raw_message in websocket:
                print(f"[debug_ws] rx {raw_message}")
                update_debug_ws_status(self.status, "received", remote_text, raw_message)
                await self._handle_message(websocket, raw_message)
        except websockets.ConnectionClosed as exc:
            print(f"[debug_ws] client disconnected code={exc.code} reason={exc.reason}")
        finally:
            async with self._client_lock:
                if self._client is websocket:
                    self._client = None
            update_debug_ws_status(self.status, "disconnected", remote_text, "client disconnected")

    async def _handle_message(self, websocket, raw_message: str) -> None:
        try:
            payload = json.loads(raw_message)
        except json.JSONDecodeError as exc:
            error_payload = {"type": "error", "message": f"invalid_json: {exc}"}
            await websocket.send(json.dumps(error_payload, ensure_ascii=False))
            print(f"[debug_ws] tx {json.dumps(error_payload, ensure_ascii=False)}")
            return

        if not isinstance(payload, dict):
            error_payload = {"type": "error", "message": "payload must be a JSON object"}
            await websocket.send(json.dumps(error_payload, ensure_ascii=False))
            print(f"[debug_ws] tx {json.dumps(error_payload, ensure_ascii=False)}")
            return

        message_type = str(payload.get("type", "")).strip()

        if message_type == "hello":
            await websocket.send(json.dumps({
                "type": "hello",
                "role": "host_debug_server",
                "transport": "websocket",
                "path": self.path,
                "mcp_connected": self.status.connected,
            }, ensure_ascii=False))
            return

        if message_type == "touch_state_update":
            await websocket.send(json.dumps({
                "type": "ack",
                "request_type": "touch_state_update",
                "accepted": True,
            }, ensure_ascii=False))
            return

        if message_type == "draw_random_pattern_request":
            result_payload = build_random_matrix_frame_payload(
                transcript=str(payload.get("transcript", "")),
                source="debug_ws_random",
            )
            response_payload = {
                "type": "matrix_pattern_result",
                **result_payload,
            }
            await websocket.send(json.dumps(response_payload, ensure_ascii=False))
            update_matrix_status(self.status, result_payload, "debug_ws_sent")
            print(f"[debug_ws] tx {json.dumps(response_payload, ensure_ascii=False)}")
            return

        await websocket.send(json.dumps({
            "type": "error",
            "message": f"unsupported_type: {message_type or '<empty>'}",
        }, ensure_ascii=False))


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
                if parsed.path == "/status":
                    self._send_json(200, asdict(outer.status))
                    return

                if parsed.path.startswith("/generated/"):
                    file_name = os.path.basename(unquote(parsed.path[len("/generated/"):]))
                    file_path = os.path.abspath(os.path.join(outer.output_dir, file_name))
                    mime_type, _ = mimetypes.guess_type(file_path)

                    if not file_name or os.path.dirname(file_path) != os.path.abspath(outer.output_dir):
                        self.send_error(400, "Invalid file path")
                        return
                    if not os.path.isfile(file_path):
                        self.send_error(404, "Image not found")
                        return

                    with open(file_path, "rb") as image_file:
                        body = image_file.read()

                    self.send_response(200)
                    self.send_header("Content-Type", mime_type or "application/octet-stream")
                    self.send_header("Content-Length", str(len(body)))
                    self.end_headers()
                    self.wfile.write(body)
                    return

                self.send_error(404, "Unsupported path")

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
                    record_call_status(
                        outer.status,
                        call_type="http_control",
                        name="http.control.snapshot",
                        ok=True,
                        event="control_success",
                        result={"quality": quality, "device_result": result_payload},
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

                if parsed.path == "/control/matrix_16x16":
                    try:
                        request_payload = self._read_json_body()
                        timeout_seconds = float(request_payload.get("timeout", DEFAULT_CONTROL_TIMEOUT))
                        preset = str(request_payload.get("preset", ""))
                        frame_hex = str(request_payload.get("frame_rgb332_hex", ""))
                        source = str(request_payload.get("source", "http"))
                        transcript = str(request_payload.get("transcript", ""))
                    except ValueError as exc:
                        update_status(
                            outer.status,
                            last_tool="http.control.matrix_16x16",
                            last_event="control_error",
                            last_error=str(exc),
                        )
                        self.send_error(400, str(exc))
                        return

                    if timeout_seconds <= 0:
                        self.send_error(400, "timeout must be greater than 0")
                        return
                    if (not preset) and (not frame_hex):
                        self.send_error(400, "Either preset or frame_rgb332_hex is required")
                        return

                    try:
                        result_payload = handle_local_tool_call(
                            "self.screen.matrix_16x16.draw",
                            {
                                "preset": preset,
                                "frame_rgb332_hex": frame_hex,
                                "source": source,
                                "transcript": transcript,
                            },
                        )
                        preview_url = ""
                        saved_path = ""
                        if str(result_payload.get("frame_rgb332_hex", "")):
                            saved_path = upload_matrix_preview_frame(
                                frame_hex=str(result_payload.get("frame_rgb332_hex", "")),
                                output_dir=outer.output_dir,
                                label="matrix_control_preview",
                            )
                            preview_url = build_generated_image_url(outer.status, saved_path)
                            future = asyncio.run_coroutine_threadsafe(
                                outer.bridge.dispatch_remote_preview_fetch(
                                    image_url=preview_url,
                                    transcript=transcript,
                                    timeout_seconds=timeout_seconds,
                                ),
                                outer.event_loop,
                            )
                            future.result(timeout=timeout_seconds + 1.0)
                    except TimeoutError as exc:
                        update_status(
                            outer.status,
                            last_tool="http.control.matrix_16x16",
                            last_event="control_timeout",
                            last_error=str(exc),
                        )
                        self.send_error(504, str(exc))
                        return
                    except RuntimeError as exc:
                        update_status(
                            outer.status,
                            last_tool="http.control.matrix_16x16",
                            last_event="control_error",
                            last_error=str(exc),
                        )
                        self.send_error(503, str(exc))
                        return
                    except Exception as exc:
                        update_status(
                            outer.status,
                            last_tool="http.control.matrix_16x16",
                            last_event="control_error",
                            last_error=str(exc),
                        )
                        self.send_error(500, str(exc))
                        return

                    outer.status.control_calls += 1
                    record_call_status(
                        outer.status,
                        call_type="http_control",
                        name="http.control.matrix_16x16",
                        ok=True,
                        event="control_success",
                        result={"device_result": result_payload},
                    )
                    update_matrix_status(outer.status, result_payload, "http_fetched")
                    preview_payload = {
                        "requested": bool(preview_url),
                        "transport": "ai_pull_http",
                        "preview_image_url": preview_url,
                        "saved_path": saved_path,
                        "reason": "frame_rgb332_hex is required for HTTP preview" if not preview_url else "",
                    }
                    if preview_url:
                        update_device_preview_status(outer.status,
                                                     "fetched",
                                                     f"AI fetched {preview_url}")
                    self._send_json(
                        200,
                        {
                            "requested": True,
                            "device_result": result_payload,
                            "preview_result": preview_payload,
                        },
                    )
                    return

                if parsed.path == "/control/matrix_prompt_16x16":
                    try:
                        request_payload = self._read_json_body()
                        timeout_seconds = float(request_payload.get("timeout", DEFAULT_CONTROL_TIMEOUT))
                        prompt = str(request_payload.get("prompt", ""))
                        primary_rgb888 = str(request_payload.get("primary_rgb888", ""))
                        background_rgb888 = str(request_payload.get("background_rgb888", ""))
                        source = str(request_payload.get("source", "http_prompt"))
                        transcript = str(request_payload.get("transcript", prompt))
                    except ValueError as exc:
                        update_status(
                            outer.status,
                            last_tool="http.control.matrix_prompt_16x16",
                            last_event="control_error",
                            last_error=str(exc),
                        )
                        self.send_error(400, str(exc))
                        return

                    if timeout_seconds <= 0:
                        self.send_error(400, "timeout must be greater than 0")
                        return
                    if not prompt.strip():
                        self.send_error(400, "prompt is required")
                        return

                    try:
                        rendered_payload = render_prompt_to_matrix_frame(
                            prompt=prompt,
                            primary_rgb888=primary_rgb888,
                            background_rgb888=background_rgb888,
                            source=source,
                            transcript=transcript,
                        )
                        saved_path = upload_matrix_preview_frame(
                            frame_hex=str(rendered_payload.get("frame_rgb332_hex", "")),
                            output_dir=outer.output_dir,
                            label="matrix_prompt_preview",
                        )
                        preview_url = build_generated_image_url(outer.status, saved_path)
                        future = asyncio.run_coroutine_threadsafe(
                            outer.bridge.dispatch_remote_preview_fetch(
                                image_url=preview_url,
                                transcript=transcript,
                                timeout_seconds=timeout_seconds,
                            ),
                            outer.event_loop,
                        )
                        result_payload = {
                            "fetched": True,
                            "preview_image_url": preview_url,
                            "saved_path": saved_path,
                        }
                        future.result(timeout=timeout_seconds + 1.0)
                    except TimeoutError as exc:
                        update_status(
                            outer.status,
                            last_tool="http.control.matrix_prompt_16x16",
                            last_event="control_timeout",
                            last_error=str(exc),
                        )
                        self.send_error(504, str(exc))
                        return
                    except RuntimeError as exc:
                        update_status(
                            outer.status,
                            last_tool="http.control.matrix_prompt_16x16",
                            last_event="control_error",
                            last_error=str(exc),
                        )
                        self.send_error(503, str(exc))
                        return
                    except Exception as exc:
                        update_status(
                            outer.status,
                            last_tool="http.control.matrix_prompt_16x16",
                            last_event="control_error",
                            last_error=str(exc),
                        )
                        self.send_error(500, str(exc))
                        return

                    outer.status.control_calls += 1
                    record_call_status(
                        outer.status,
                        call_type="http_control",
                        name="http.control.matrix_prompt_16x16",
                        ok=True,
                        event="control_success",
                        result={
                            "render_result": rendered_payload,
                            "device_result": result_payload,
                        },
                    )
                    update_matrix_status(outer.status, rendered_payload, "http_fetched")
                    preview_payload = {
                        "requested": True,
                        "transport": "ai_pull_http",
                        "preview_image_url": preview_url,
                        "saved_path": saved_path,
                    }
                    update_device_preview_status(outer.status,
                                                 "fetched",
                                                 f"AI fetched {preview_url}")
                    self._send_json(
                        200,
                        {
                            "requested": True,
                            "render_result": rendered_payload,
                            "device_result": result_payload,
                            "preview_result": preview_payload,
                        },
                    )
                    return

                if parsed.path == "/control/device_preview":
                    try:
                        request_payload = self._read_json_body()
                        device_ip = str(request_payload.get("device_ip", ""))
                        image_path = str(request_payload.get("image_path", ""))
                        device_port = int(request_payload.get("device_port", DEFAULT_DEVICE_PREVIEW_PORT))
                        timeout_seconds = float(request_payload.get("timeout", DEFAULT_CONTROL_TIMEOUT))
                        result_payload = upload_image_to_device_preview(
                            device_ip=device_ip,
                            image_path=image_path,
                            device_port=device_port,
                            timeout_seconds=timeout_seconds,
                        )
                    except ValueError as exc:
                        update_status(
                            outer.status,
                            last_tool="http.control.device_preview",
                            last_event="control_error",
                            last_error=str(exc),
                        )
                        self.send_error(400, str(exc))
                        return
                    except RuntimeError as exc:
                        update_status(
                            outer.status,
                            last_tool="http.control.device_preview",
                            last_event="control_error",
                            last_error=str(exc),
                        )
                        self.send_error(502, str(exc))
                        return
                    except Exception as exc:
                        update_status(
                            outer.status,
                            last_tool="http.control.device_preview",
                            last_event="control_error",
                            last_error=str(exc),
                        )
                        self.send_error(500, str(exc))
                        return

                    outer.status.control_calls += 1
                    record_call_status(
                        outer.status,
                        call_type="http_control",
                        name="http.control.device_preview",
                        ok=True,
                        event="control_success",
                        result=result_payload,
                    )
                    self._send_json(
                        200,
                        {
                            "requested": True,
                            **result_payload,
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
        matrix_control_url = f"http://{public_host}:{self.port}/control/matrix_16x16"
        matrix_prompt_control_url = f"http://{public_host}:{self.port}/control/matrix_prompt_16x16"
        device_preview_control_url = f"http://{public_host}:{self.port}/control/device_preview"
        status_url = f"http://{public_host}:{self.port}/status"

        self._server = ThreadingHTTPServer((self.host, self.port), self._build_handler())
        self._thread = threading.Thread(target=self._server.serve_forever, name="gp_snapshot_http", daemon=True)
        self._thread.start()
        update_status(
            self.status,
            last_event="http_listening",
            snapshot_url=snapshot_url,
            control_url=control_url,
            matrix_control_url=matrix_control_url,
            matrix_prompt_control_url=matrix_prompt_control_url,
            device_preview_control_url=device_preview_control_url,
            status_url=status_url,
            last_error="-",
        )
        print(f"[http] listening host={self.host} port={self.port}")
        print(f"[http] snapshot upload url={snapshot_url}")
        print(f"[http] snapshot control url={control_url}")
        print(f"[http] matrix control url={matrix_control_url}")
        print(f"[http] matrix prompt control url={matrix_prompt_control_url}")
        print(f"[http] device preview control url={device_preview_control_url}")
        print(f"[http] generated image base url=http://{public_host}:{self.port}/generated/")
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
    parser = build_bridge_arg_parser({
        "url": os.getenv("GP_MCP_URL", DEFAULT_MCP_URL),
        "timeout": 0.0,
        "output_dir": DEFAULT_SNAPSHOT_DIR,
        "http_host": DEFAULT_HTTP_HOST,
        "http_port": DEFAULT_HTTP_PORT,
        "ws_host": DEFAULT_DEBUG_WS_HOST,
        "ws_port": DEFAULT_DEBUG_WS_PORT,
    })
    return parser.parse_args()


async def run() -> int:
    args = parse_args()
    status = ServerStatus()
    http_server: Optional[HttpSnapshotServer] = None
    debug_ws_server = LocalDebugWebSocketServer(args.ws_host, args.ws_port, DEFAULT_DEBUG_WS_PATH, status)
    startup_preview_task: Optional[asyncio.Task[None]] = None

    try:
        client = McpWebSocketClient(args.url, args.timeout, args.verbose)
        async with McpBridgeServer(client, args.output_dir, status, debug_ws_server) as bridge:
            await debug_ws_server.start()
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
                startup_preview_task = asyncio.create_task(
                    run_startup_preview_upload(status, args.output_dir)
                )
            await bridge.wait_closed()
            if startup_preview_task is not None:
                await startup_preview_task
            if debug_ws_server is not None:
                await debug_ws_server.stop()
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
    try:
        return asyncio.run(run())
    except KeyboardInterrupt:
        print("Interrupted by user.", file=sys.stderr)
        return 130


if __name__ == "__main__":
    raise SystemExit(main())

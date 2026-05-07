#!/usr/bin/env python3

from __future__ import annotations

import argparse
from typing import Any, Dict


def build_bridge_arg_parser(defaults: Dict[str, Any]) -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description=(
            "Run the local display MCP bridge: screenshot receiver, 16x16 drawing tools, debug websocket "
            "transport, and AI-side preview helpers."
        )
    )
    parser.add_argument(
        "--url",
        default=defaults["url"],
        help="MCP WebSocket endpoint, or use GP_MCP_URL",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=defaults["timeout"],
        help="Timeout in seconds for waiting on inbound messages. Use 0 to wait indefinitely.",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Print connection progress and raw responses",
    )
    parser.add_argument(
        "--output-dir",
        default=defaults["output_dir"],
        help="Directory used to save captured screenshots.",
    )
    parser.add_argument(
        "--http-host",
        default=defaults["http_host"],
        help="HTTP host used for direct device snapshot uploads and local control.",
    )
    parser.add_argument(
        "--http-port",
        type=int,
        default=defaults["http_port"],
        help="HTTP port used for direct device snapshot uploads and local control.",
    )
    parser.add_argument(
        "--disable-http",
        action="store_true",
        help="Disable the local HTTP snapshot receiver and HTTP control endpoint.",
    )
    parser.add_argument(
        "--ws-host",
        default=defaults["ws_host"],
        help="WebSocket host used for AI-side debug data transport.",
    )
    parser.add_argument(
        "--ws-port",
        type=int,
        default=defaults["ws_port"],
        help="WebSocket port used for AI-side debug data transport.",
    )
    return parser

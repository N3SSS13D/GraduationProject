#!/usr/bin/env python3
import argparse
import asyncio
import json
import math
import os
import sys
from dataclasses import dataclass
from typing import Any, Dict, Optional


try:
    import websockets
except ImportError as exc:  # pragma: no cover
    print(
        "Missing dependency: websockets\n"
        "Install it with: python -m pip install websockets",
        file=sys.stderr,
    )
    raise SystemExit(2) from exc


@dataclass
class JsonRpcResponse:
    response_id: int
    result: Optional[Dict[str, Any]] = None
    error: Optional[Dict[str, Any]] = None


class McpWebSocketClient:
    def __init__(self, url: str, timeout_seconds: float, verbose: bool):
        self.url = url
        self.timeout_seconds = timeout_seconds
        self.verbose = verbose
        self._next_id = 1
        self._websocket = None

    async def __aenter__(self) -> "McpWebSocketClient":
        self._websocket = await websockets.connect(self.url, open_timeout=self.timeout_seconds)
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
        raw_message = await asyncio.wait_for(self._websocket.recv(), timeout=self.timeout_seconds)
        self._log(f"<- {raw_message}")
        return json.loads(raw_message)

    async def request(self, method: str, params: Optional[Dict[str, Any]] = None) -> JsonRpcResponse:
        request_id = self._next_id
        self._next_id += 1

        payload = {
            "jsonrpc": "2.0",
            "id": request_id,
            "method": method,
            "params": params or {},
        }
        self._log(f"-> {method} #{request_id}")
        await self.send_json(payload)

        while True:
            message = await self.receive_json()
            if "id" not in message:
                print(
                    json.dumps(
                        {"notification": message},
                        ensure_ascii=False,
                        indent=2,
                    )
                )
                continue
            if message["id"] != request_id:
                continue
            return JsonRpcResponse(
                response_id=request_id,
                result=message.get("result"),
                error=message.get("error"),
            )

    async def notify(self, method: str, params: Optional[Dict[str, Any]] = None) -> None:
        payload = {
            "jsonrpc": "2.0",
            "method": method,
            "params": params or {},
        }
        self._log(f"-> notification {method}")
        await self.send_json(payload)

    async def respond(self, request_id: int, result: Optional[Dict[str, Any]] = None,
        error: Optional[Dict[str, Any]] = None) -> None:
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


async def run_client_mode(client: McpWebSocketClient, args: argparse.Namespace) -> int:
    initialize_response = await client.request(
        "initialize",
        {
            "capabilities": {
                "sampling": {},
                "elicitation": {},
            },
            "clientInfo": {
                "name": "gp-mcp-endpoint-client",
                "version": "1.0.0",
            },
            "protocolVersion": "2024-11-05",
        },
    )
    dump_response("initialize", initialize_response)
    if initialize_response.error is not None:
        return 1

    await client.notify("notifications/initialized", {})
    if args.verbose:
        print("\n=== notifications/initialized ===")
        print(json.dumps({"sent": True}, ensure_ascii=False, indent=2))

    if not args.skip_tools_list:
        tools_response = await client.request("tools/list", {"cursor": ""})
        dump_response("tools/list", tools_response)
        if tools_response.error is not None:
            return 1

    if args.calculator_operation:
        calculator_response = await client.request(
            "tools/call",
            {
                "name": "self.calculator.calculate",
                "arguments": {
                    "operation": args.calculator_operation,
                    "left": args.calculator_left,
                    "right": args.calculator_right,
                },
            },
        )
        dump_response("tools/call self.calculator.calculate", calculator_response)
        if calculator_response.error is not None:
            return 1

    if args.dot_primary:
        dot_arguments = {
            "primary_rgb888": args.dot_primary,
            "secondary_rgb888": args.dot_secondary,
            "animation": args.dot_animation,
            "size": args.dot_size,
            "duration_ms": args.dot_duration_ms,
            "label": args.dot_label,
            "transcript": args.dot_transcript,
            "source": args.dot_source,
        }
        dot_response = await client.request(
            "tools/call",
            {
                "name": "self.screen.debug_dot.show",
                "arguments": dot_arguments,
            },
        )
        dump_response("tools/call self.screen.debug_dot.show", dot_response)
        if dot_response.error is not None:
            return 1

    return 0


async def run_server_mode(client: McpWebSocketClient, args: argparse.Namespace) -> int:
    initialized = False
    tool_calls = 0

    while True:
        try:
            message = await client.receive_json()
        except TimeoutError:
            if initialized:
                print("\n=== server idle timeout ===")
                print(json.dumps({"initialized": True, "tool_calls": tool_calls}, ensure_ascii=False, indent=2))
                return 0
            raise

        method = message.get("method")
        request_id = message.get("id")
        params = message.get("params", {})

        if method == "initialize" and request_id is not None:
            result = {
                "protocolVersion": "2024-11-05",
                "capabilities": {
                    "tools": {},
                },
                "serverInfo": {
                    "name": "gp-mcp-endpoint-client",
                    "version": "1.0.0",
                },
            }
            await client.respond(request_id, result=result)
            print("\n=== initialize ===")
            print(json.dumps(result, ensure_ascii=False, indent=2))
            initialized = True
            continue

        if method == "notifications/initialized":
            print("\n=== notifications/initialized ===")
            print(json.dumps({"received": True}, ensure_ascii=False, indent=2))
            continue

        if method == "tools/list" and request_id is not None:
            result = {
                "tools": build_tool_list(),
                "nextCursor": "",
            }
            await client.respond(request_id, result=result)
            print("\n=== tools/list ===")
            print(json.dumps(result, ensure_ascii=False, indent=2))
            continue

        if method == "ping" and request_id is not None:
            result = {}
            await client.respond(request_id, result=result)
            print("\n=== ping ===")
            print(json.dumps({"pong": True}, ensure_ascii=False, indent=2))
            continue

        if method == "tools/call" and request_id is not None:
            tool_calls += 1
            try:
                tool_name = params.get("name", "")
                arguments = params.get("arguments", {})
                if tool_name == "self.calculator.calculate":
                    left = int(arguments["left"])
                    right = int(arguments["right"])
                    operation = str(arguments["operation"])
                    result_value = calculate_result(operation, left, right)
                    result_payload = {
                        "operation": operation.lower(),
                        "left": left,
                        "right": right,
                        "result": int(result_value) if math.isfinite(result_value) and result_value.is_integer() else result_value,
                    }
                    await client.respond(request_id, result=tool_result_content(result_payload))
                    print("\n=== tools/call self.calculator.calculate ===")
                    print(json.dumps(result_payload, ensure_ascii=False, indent=2))
                    continue

                if tool_name == "self.screen.debug_dot.show":
                    primary = str(arguments["primary_rgb888"])
                    parse_rgb888(primary)
                    secondary = str(arguments.get("secondary_rgb888", ""))
                    if secondary:
                        parse_rgb888(secondary)
                    result_payload = {
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
                    await client.respond(request_id, result=tool_result_content(result_payload))
                    print("\n=== tools/call self.screen.debug_dot.show ===")
                    print(json.dumps(result_payload, ensure_ascii=False, indent=2))
                    continue

                await client.respond(
                    request_id,
                    error={"code": -32601, "message": f"Unknown tool: {tool_name}"},
                )
            except (KeyError, TypeError, ValueError) as exc:
                await client.respond(
                    request_id,
                    error={"code": -32602, "message": str(exc)},
                )
            continue

        if request_id is not None:
            await client.respond(
                request_id,
                error={"code": -32601, "message": f"Unknown method: {method}"},
            )

        if initialized and tool_calls >= args.server_exit_after_tool_calls:
            return 0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Connect to an MCP WebSocket endpoint and validate calculator/debug-dot tools."
    )
    parser.add_argument(
        "--url",
        default=os.getenv("GP_MCP_URL", ""),
        help="MCP WebSocket endpoint, or use GP_MCP_URL",
    )
    parser.add_argument(
        "--mode",
        choices=["client", "server"],
        default="client",
        help="Use client mode for calling an MCP server, or server mode for endpoints that initiate initialize themselves.",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=15.0,
        help="Timeout in seconds for connect and response wait",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Print connection progress and raw responses",
    )
    parser.add_argument(
        "--skip-tools-list",
        action="store_true",
        help="Skip tools/list after initialize",
    )
    parser.add_argument(
        "--calculator-operation",
        choices=["add", "subtract", "multiply", "divide", "mod"],
        help="Invoke self.calculator.calculate with the given operation",
    )
    parser.add_argument("--calculator-left", type=int, default=12)
    parser.add_argument("--calculator-right", type=int, default=30)
    parser.add_argument(
        "--dot-primary",
        help="Invoke self.screen.debug_dot.show with the given primary RGB888 color",
    )
    parser.add_argument("--dot-secondary", default="")
    parser.add_argument(
        "--dot-animation",
        choices=["solid", "gradient", "pulse"],
        default="solid",
    )
    parser.add_argument("--dot-size", type=int, default=42)
    parser.add_argument("--dot-duration-ms", type=int, default=1800)
    parser.add_argument("--dot-label", default="mcp-client")
    parser.add_argument("--dot-transcript", default="")
    parser.add_argument("--dot-source", default="mcp-client")
    parser.add_argument(
        "--server-exit-after-tool-calls",
        type=int,
        default=1,
        help="In server mode, exit after handling this many tools/call requests.",
    )
    return parser.parse_args()


def dump_response(title: str, response: JsonRpcResponse) -> None:
    body: Dict[str, Any] = {"id": response.response_id}
    if response.error is not None:
        body["error"] = response.error
    else:
        body["result"] = response.result
    print(f"\n=== {title} ===")
    print(json.dumps(body, ensure_ascii=False, indent=2))


async def run() -> int:
    args = parse_args()
    if not args.url:
        print("Missing MCP endpoint URL. Pass --url or set GP_MCP_URL.", file=sys.stderr)
        return 2

    try:
        async with McpWebSocketClient(args.url, args.timeout, args.verbose) as client:
            if args.mode == "server":
                return await run_server_mode(client, args)
            return await run_client_mode(client, args)
    except TimeoutError:
        print(f"Timed out after {args.timeout:.1f}s while waiting for MCP endpoint response.", file=sys.stderr)
        return 1
    except websockets.exceptions.WebSocketException as exc:
        print(f"WebSocket error: {exc}", file=sys.stderr)
        return 1
    except OSError as exc:
        print(f"Network error: {exc}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(asyncio.run(run()))
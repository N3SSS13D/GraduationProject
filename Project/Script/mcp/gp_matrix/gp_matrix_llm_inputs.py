#!/usr/bin/env python3

from __future__ import annotations

from typing import Any, Dict, Sequence

MATRIX_WIDTH = 16
MATRIX_HEIGHT = 16

_BITMAP_ASCII_ON = frozenset({"1", "#", "X", "x", "*", "@"})
_BITMAP_ASCII_OFF = frozenset({"0", ".", "_", "-", " "})
_ALLOWED_DRAW_OPS = frozenset({
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


def _normalize_int(value: Any, field_name: str) -> int:
    if isinstance(value, bool):
        raise ValueError(f"{field_name} must be an integer")
    try:
        return int(value)
    except (TypeError, ValueError) as exc:
        raise ValueError(f"{field_name} must be an integer") from exc


def normalize_bitmap_ascii_value(bitmap_ascii: Any, field_name: str) -> str:
    if isinstance(bitmap_ascii, str):
        lines = [line.strip() for line in bitmap_ascii.strip().splitlines() if line.strip()]
    elif isinstance(bitmap_ascii, (list, tuple)):
        lines = [str(line).strip() for line in bitmap_ascii if str(line).strip()]
    else:
        raise ValueError(f"{field_name} must be a multi-line string or an array of 16 row strings")

    if len(lines) != MATRIX_HEIGHT:
        raise ValueError(f"{field_name} must contain exactly {MATRIX_HEIGHT} rows")

    row_values: list[str] = []
    for row_index, line in enumerate(lines):
        if len(line) != MATRIX_WIDTH:
            raise ValueError(f"{field_name}[{row_index}] must contain exactly {MATRIX_WIDTH} characters")

        row_bits = 0
        for column_index, char_value in enumerate(line):
            if char_value in _BITMAP_ASCII_ON:
                row_bits |= 1 << (MATRIX_WIDTH - 1 - column_index)
                continue
            if char_value in _BITMAP_ASCII_OFF:
                continue
            raise ValueError(
                f"{field_name}[{row_index}] has unsupported character '{char_value}'. Use only 1/#/X/*/@ and 0/./_/-/space"
            )

        row_values.append(f"{row_bits:04x}")

    return "".join(row_values)


def normalize_draw_ops(draw_ops: Any, field_name: str = "ops") -> list[Dict[str, Any]]:
    if draw_ops in (None, ""):
        return []
    if not isinstance(draw_ops, (list, tuple)):
        raise ValueError(f"{field_name} must be an array")

    normalized_ops: list[Dict[str, Any]] = []
    for op_index, op_item in enumerate(draw_ops):
        op_field_name = f"{field_name}[{op_index}]"
        if not isinstance(op_item, dict):
            raise ValueError(f"{op_field_name} must be an object")

        op_name = str(op_item.get("op", "")).strip().lower()
        if op_name not in _ALLOWED_DRAW_OPS:
            raise ValueError(f"{op_field_name}.op is unsupported: {op_name}")

        normalized_op = dict(op_item)
        normalized_op["op"] = op_name
        normalized_ops.append(normalized_op)

    return normalized_ops


def apply_draw_ops(draw_context: Any, draw_ops: Sequence[Dict[str, Any]]) -> None:
    for op_index, op_item in enumerate(draw_ops):
        op_name = str(op_item.get("op", "")).strip().lower()
        field_prefix = f"ops[{op_index}]"

        if op_name == "clear":
            fill_value = 1 if bool(op_item.get("fill", 0)) else 0
            draw_context.rectangle((0, 0, MATRIX_WIDTH - 1, MATRIX_HEIGHT - 1), fill=fill_value)
            continue

        if op_name == "point":
            x_value = _normalize_int(op_item.get("x"), f"{field_prefix}.x")
            y_value = _normalize_int(op_item.get("y"), f"{field_prefix}.y")
            fill_value = 1 if bool(op_item.get("fill", 1)) else 0
            draw_context.point((x_value, y_value), fill=fill_value)
            continue

        if op_name == "line":
            x0_value = _normalize_int(op_item.get("x0"), f"{field_prefix}.x0")
            y0_value = _normalize_int(op_item.get("y0"), f"{field_prefix}.y0")
            x1_value = _normalize_int(op_item.get("x1"), f"{field_prefix}.x1")
            y1_value = _normalize_int(op_item.get("y1"), f"{field_prefix}.y1")
            width_value = max(1, _normalize_int(op_item.get("width", 1), f"{field_prefix}.width"))
            fill_value = 1 if bool(op_item.get("fill", 1)) else 0
            draw_context.line((x0_value, y0_value, x1_value, y1_value), fill=fill_value, width=width_value)
            continue

        if op_name in {"rectangle", "fill_rectangle", "ellipse", "fill_ellipse"}:
            x0_value = _normalize_int(op_item.get("x0"), f"{field_prefix}.x0")
            y0_value = _normalize_int(op_item.get("y0"), f"{field_prefix}.y0")
            x1_value = _normalize_int(op_item.get("x1"), f"{field_prefix}.x1")
            y1_value = _normalize_int(op_item.get("y1"), f"{field_prefix}.y1")
            width_value = max(1, _normalize_int(op_item.get("width", 1), f"{field_prefix}.width"))

            if op_name in {"fill_rectangle", "fill_ellipse"}:
                fill_value = 1
                outline_value = 1
            else:
                fill_raw = op_item.get("fill")
                fill_value = None if fill_raw is None else (1 if bool(fill_raw) else 0)
                outline_value = 1 if bool(op_item.get("outline", 1)) else 0

            if op_name in {"rectangle", "fill_rectangle"}:
                draw_context.rectangle(
                    (x0_value, y0_value, x1_value, y1_value),
                    fill=fill_value,
                    outline=outline_value,
                    width=width_value,
                )
            else:
                draw_context.ellipse(
                    (x0_value, y0_value, x1_value, y1_value),
                    fill=fill_value,
                    outline=outline_value,
                    width=width_value,
                )
            continue

        if op_name in {"circle", "fill_circle"}:
            cx_value = _normalize_int(op_item.get("cx"), f"{field_prefix}.cx")
            cy_value = _normalize_int(op_item.get("cy"), f"{field_prefix}.cy")
            radius_value = max(0, _normalize_int(op_item.get("radius"), f"{field_prefix}.radius"))
            width_value = max(1, _normalize_int(op_item.get("width", 1), f"{field_prefix}.width"))

            if op_name == "fill_circle":
                fill_value = 1
                outline_value = 1
            else:
                fill_raw = op_item.get("fill")
                fill_value = None if fill_raw is None else (1 if bool(fill_raw) else 0)
                outline_value = 1 if bool(op_item.get("outline", 1)) else 0

            draw_context.ellipse(
                (cx_value - radius_value, cy_value - radius_value, cx_value + radius_value, cy_value + radius_value),
                fill=fill_value,
                outline=outline_value,
                width=width_value,
            )
            continue

        if op_name == "polygon":
            points = op_item.get("points")
            if not isinstance(points, (list, tuple)) or len(points) < 2:
                raise ValueError(f"{field_prefix}.points must contain at least 2 points")

            normalized_points = []
            for point_index, point_pair in enumerate(points):
                point_field_name = f"{field_prefix}.points[{point_index}]"
                if not isinstance(point_pair, (list, tuple)) or len(point_pair) != 2:
                    raise ValueError(f"{point_field_name} must contain exactly 2 values")
                normalized_points.append((
                    _normalize_int(point_pair[0], f"{point_field_name}[0]"),
                    _normalize_int(point_pair[1], f"{point_field_name}[1]"),
                ))

            fill_raw = op_item.get("fill")
            fill_value = None if fill_raw is None else (1 if bool(fill_raw) else 0)
            outline_value = 1 if bool(op_item.get("outline", 1)) else 0
            draw_context.polygon(normalized_points, fill=fill_value, outline=outline_value)
            continue

        raise ValueError(f"ops[{op_index}].op is unsupported: {op_name}")

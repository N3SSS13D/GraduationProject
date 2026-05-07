#!/usr/bin/env python3

from __future__ import annotations

from typing import Any, Dict


def build_draw_python_ops_schema() -> Dict[str, Any]:
    return {
        "type": "array",
        "description": (
            "Structured draw operations for LLM-safe input. Each item must contain op and matching fields. "
            "Supported ops: clear, point, line, rectangle, fill_rectangle, ellipse, fill_ellipse, circle, fill_circle, polygon."
        ),
        "items": {
            "type": "object",
            "properties": {
                "op": {"type": "string"},
                "x": {"type": "integer"},
                "y": {"type": "integer"},
                "x0": {"type": "integer"},
                "y0": {"type": "integer"},
                "x1": {"type": "integer"},
                "y1": {"type": "integer"},
                "cx": {"type": "integer"},
                "cy": {"type": "integer"},
                "radius": {"type": "integer"},
                "width": {"type": "integer"},
                "fill": {},
                "outline": {},
                "points": {
                    "type": "array",
                    "items": {
                        "type": "array",
                        "items": {"type": "integer"},
                        "minItems": 2,
                        "maxItems": 2,
                    },
                },
            },
            "required": ["op"],
        },
    }


def build_bitmap_ascii_schema() -> Dict[str, Any]:
    return {
        "description": (
            "ASCII bitmap rows for 16x16. Provide exactly 16 row strings, each 16 chars. "
            "On pixels: 1/#/X/*/@. Off pixels: 0/./_/-/space."
        ),
        "anyOf": [
            {
                "type": "array",
                "minItems": 16,
                "maxItems": 16,
                "items": {"type": "string"},
            },
            {
                "type": "string",
            },
        ],
    }

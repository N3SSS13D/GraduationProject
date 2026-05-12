from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


DEFAULT_CHAR = "\u5409"
TARGET_SIZE = 16
DEFAULT_OUTPUT_NAME = "ji_bitmap_demo.png"
DEFAULT_TEXT_NAME = "ji_bitmap_demo_bitmap.txt"
DEFAULT_BG = "#000000"
DEFAULT_ON = "#1677ff"
DEFAULT_OFF = "#0f0f0f"
DEFAULT_GRID = "#2a2a2a"
DEFAULT_LABEL = "#d7e3ff"
DEFAULT_ACCENT = "#4ea1ff"
DEFAULT_COLOR_FG = (0, 0, 255)
DEFAULT_COLOR_BG = (0, 0, 0)
DEFAULT_SEQUENCE = 0x00

WINDOWS_FONT_CANDIDATES = (
    r"C:\Windows\Fonts\simhei.ttf",
    r"C:\Windows\Fonts\msyh.ttc",
    r"C:\Windows\Fonts\msyh.ttf",
    r"C:\Windows\Fonts\simsun.ttc",
    r"C:\Windows\Fonts\simfang.ttf",
    r"C:\Windows\Fonts\mingliu.ttc",
)


def parse_args() -> argparse.Namespace:
    repo_root = Path(__file__).resolve().parents[3]
    output_dir = repo_root / "Doc" / "毕业论文" / "图片"
    parser = argparse.ArgumentParser(description="Generate a 16x16 glyph demo image and bitmap listing.")
    parser.add_argument("--char", default=DEFAULT_CHAR, help="Single Chinese character to render.")
    parser.add_argument("--font-path", type=Path, default=None, help="Optional font file path.")
    parser.add_argument("--font-size", type=int, default=128, help="Font size used on the source canvas.")
    parser.add_argument("--threshold", type=int, default=96, help="Threshold used to binarize the glyph.")
    parser.add_argument("--output-dir", type=Path, default=output_dir, help="Directory for generated outputs.")
    parser.add_argument("--output-name", default=DEFAULT_OUTPUT_NAME, help="PNG file name.")
    parser.add_argument("--text-name", default=DEFAULT_TEXT_NAME, help="Bitmap text file name.")
    parser.add_argument("--scale", type=int, default=24, help="Pixel cell size for the left preview.")
    parser.add_argument("--arrow-width", type=int, default=320, help="Width reserved for the center arrow.")
    return parser.parse_args()


def find_font_path(font_path: Path | None) -> Path:
    if font_path is not None:
        if font_path.exists():
            return font_path
        raise FileNotFoundError(f"Font file not found: {font_path}")

    for candidate in WINDOWS_FONT_CANDIDATES:
        path = Path(candidate)
        if path.exists():
            return path

    raise FileNotFoundError("No usable Chinese font found. Pass --font-path to a CJK font file.")


def render_glyph_matrix(char: str, font_path: Path, font_size: int, threshold: int) -> list[list[int]]:
    canvas_size = 128
    canvas = Image.new("L", (canvas_size, canvas_size), color=0)
    draw = ImageDraw.Draw(canvas)
    font = ImageFont.truetype(str(font_path), font_size)
    draw.text((canvas_size // 2, canvas_size // 2), char, fill=255, font=font, anchor="mm")

    bbox = canvas.getbbox()
    if bbox is None:
        frame = Image.new("L", (TARGET_SIZE, TARGET_SIZE), color=0)
    else:
        cropped = canvas.crop(bbox)
        inner_size = TARGET_SIZE - 2
        scale = min(inner_size / max(1, cropped.width), inner_size / max(1, cropped.height))
        resized = cropped.resize(
            (max(1, int(round(cropped.width * scale))), max(1, int(round(cropped.height * scale)))),
            Image.Resampling.LANCZOS,
        )
        frame = Image.new("L", (TARGET_SIZE, TARGET_SIZE), color=0)
        paste_x = (TARGET_SIZE - resized.width) // 2
        paste_y = (TARGET_SIZE - resized.height) // 2
        frame.paste(resized, (paste_x, paste_y))

    matrix: list[list[int]] = []
    for row in range(TARGET_SIZE):
        row_pixels = []
        for col in range(TARGET_SIZE):
            row_pixels.append(1 if frame.getpixel((col, row)) >= threshold else 0)
        matrix.append(row_pixels)
    return matrix


def matrix_to_rows(matrix: list[list[int]]) -> list[int]:
    rows: list[int] = []
    for row_pixels in matrix:
        value = 0
        for bit in row_pixels:
            value = (value << 1) | (1 if bit else 0)
        rows.append(value)
    return rows


def row_to_le_bytes(row_value: int) -> tuple[int, int]:
    return row_value & 0xFF, (row_value >> 8) & 0xFF


def row_to_be_bytes(row_value: int) -> tuple[int, int]:
    hi = (row_value >> 8) & 0xFF
    lo = row_value & 0xFF
    return hi, lo


def rows_to_bitmap_bytes(rows: list[int]) -> list[int]:
    bitmap_bytes: list[int] = []
    for row_value in rows:
        lo, hi = row_to_le_bytes(row_value)
        bitmap_bytes.append(lo)
        bitmap_bytes.append(hi)
    return bitmap_bytes


def format_bitmap_text(char: str, rows: list[int], sequence: int) -> str:
    lines = []
    lines.append(f"/* glyph demo for {char} */")
    lines.append("/* real app frame format: 32-byte bitmap + 3-byte RGB + 1-byte sequence */")
    lines.append("/* bitmap display order: 16 rows x 2 bytes, hi byte first, top row first */")
    lines.append("static const uint8_t code g_demoLayeredFrame[36] = {")
    for idx, row_value in enumerate(rows):
        hi, lo = row_to_be_bytes(row_value)
        lines.append(f"    /* row {idx:02d} */ 0x{hi:02X}, 0x{lo:02X},")
    lines.append(f"    /* rgb888 */ 0x{DEFAULT_COLOR_FG[0]:02X}, 0x{DEFAULT_COLOR_FG[1]:02X}, 0x{DEFAULT_COLOR_FG[2]:02X},")
    lines.append(f"    /* seq    */ 0x{sequence & 0xFF:02X}")
    lines.append("};")
    lines.append("")
    lines.append("static const uint8_t code g_demoPrimaryRgb888[3] = { 0x00, 0x00, 0xFF };")
    lines.append("")
    lines.append(f"/* primary_rgb888 = #{DEFAULT_COLOR_FG[0]:02X}{DEFAULT_COLOR_FG[1]:02X}{DEFAULT_COLOR_FG[2]:02X} */")
    lines.append(f"/* sequence = 0x{sequence & 0xFF:02X} */")
    return "\n".join(lines)


def load_text_font(font_path: Path, size: int) -> ImageFont.FreeTypeFont:
    return ImageFont.truetype(str(font_path), size)


def draw_grid(draw: ImageDraw.ImageDraw, origin: tuple[int, int], matrix: list[list[int]], cell: int, on_color: str, off_color: str,
              grid_color: str) -> None:
    left, top = origin
    for row_idx, row_pixels in enumerate(matrix):
        for col_idx, bit in enumerate(row_pixels):
            x0 = left + col_idx * cell
            y0 = top + row_idx * cell
            x1 = x0 + cell
            y1 = y0 + cell
            draw.rectangle((x0, y0, x1, y1), fill=on_color if bit else off_color, outline=grid_color)


def draw_arrow(draw: ImageDraw.ImageDraw, box: tuple[int, int, int, int], accent: str) -> None:
    x0, y0, x1, y1 = box
    mid_y = (y0 + y1) // 2
    head = max(28, (x1 - x0) // 5)
    shaft_end = x1 - head
    draw.line((x0, mid_y, shaft_end, mid_y), fill=accent, width=8)
    draw.polygon(
        [
            (shaft_end, y0 + 8),
            (x1, mid_y),
            (shaft_end, y1 - 8),
        ],
        fill=accent,
    )


def draw_demo_image(char: str, matrix: list[list[int]], rows: list[int], font_path: Path, output_path: Path,
                    scale: int, arrow_width: int, sequence: int) -> None:
    cell = max(18, scale)
    grid_size = cell * TARGET_SIZE
    margin = 36
    title_font = load_text_font(font_path, 30)
    label_font = load_text_font(font_path, 22)
    mono_font = ImageFont.truetype(str(font_path), 18)

    bitmap_lines = []
    for idx, row_value in enumerate(rows):
        hi, lo = row_to_be_bytes(row_value)
        bitmap_lines.append(f"row {idx:02d}   0x{hi:02X} 0x{lo:02X}")

    bitmap_line_height = 24
    right_panel_width = 480
    width = margin * 2 + grid_size + arrow_width + right_panel_width
    list_y = 126
    color_block_top = list_y + 28 + len(bitmap_lines) * bitmap_line_height + 24
    sequence_y = color_block_top + 36 + 36
    panel_bottom = sequence_y + 36
    content_height = max(grid_size, panel_bottom - margin)
    height = margin * 2 + content_height

    image = Image.new("RGB", (width, height), color=DEFAULT_BG)
    draw = ImageDraw.Draw(image)

    draw.text((margin, 14), "Glyph Pixels", fill=DEFAULT_LABEL, font=title_font)
    draw.text((margin, 52), f"Character: {char}", fill=DEFAULT_ACCENT, font=label_font)
    draw.text((margin, 82), "Black background, blue on-pixels, 16x16 grid", fill=DEFAULT_LABEL, font=label_font)

    grid_origin = (margin, 126)
    draw_grid(draw, grid_origin, matrix, cell, DEFAULT_ON, DEFAULT_OFF, DEFAULT_GRID)

    grid_box = (grid_origin[0], grid_origin[1], grid_origin[0] + grid_size, grid_origin[1] + grid_size)
    arrow_x0 = grid_box[2] + 40
    arrow_x1 = arrow_x0 + arrow_width - 80
    arrow_y0 = grid_box[1] + grid_size // 2 - 30
    arrow_y1 = arrow_y0 + 60
    draw_arrow(draw, (arrow_x0, arrow_y0, arrow_x1, arrow_y1), DEFAULT_ACCENT)
    draw.text((arrow_x0 + 20, arrow_y0 - 34), "encode", fill=DEFAULT_LABEL, font=label_font)

    right_x = arrow_x0 + arrow_width
    draw.text((right_x, 14), "Bitmap Encoding", fill=DEFAULT_LABEL, font=title_font)
    draw.text((right_x, 52), "16 rows x 2 bytes + 3-byte RGB + 1-byte seq", fill=DEFAULT_ACCENT, font=label_font)
    draw.text((right_x, 82), "bitmap rows: hi byte first, top row first", fill=DEFAULT_LABEL, font=label_font)

    draw.rounded_rectangle((right_x - 12, list_y - 12, width - margin, panel_bottom),
                           radius=18, outline=DEFAULT_GRID, width=2, fill="#050505")
    header = "row      hi    lo"
    draw.text((right_x + 12, list_y), header, fill=DEFAULT_ACCENT, font=mono_font)
    for idx, line in enumerate(bitmap_lines):
        y = list_y + 28 + idx * bitmap_line_height
        draw.text((right_x + 12, y), line, fill=DEFAULT_LABEL, font=mono_font)

    draw.text((right_x + 12, color_block_top), "Foreground RGB + sequence", fill=DEFAULT_ACCENT, font=label_font)
    swatch_y = color_block_top + 36
    swatch_size = 28
    draw.rectangle((right_x + 12, swatch_y, right_x + 12 + swatch_size, swatch_y + swatch_size), fill=DEFAULT_COLOR_FG)
    draw.text((right_x + 54, swatch_y + 2), "primary_rgb888  = 00 00 FF", fill=DEFAULT_LABEL, font=mono_font)
    draw.text((right_x + 12, sequence_y), f"sequence byte       = 0x{sequence & 0xFF:02X}", fill=DEFAULT_LABEL, font=mono_font)

    image.save(output_path)


def save_text_output(output_path: Path, content: str) -> None:
    output_path.write_text(content, encoding="utf-8", newline="\n")


def main() -> int:
    args = parse_args()
    if len(args.char) != 1:
        raise ValueError("--char must contain exactly one character.")

    threshold = min(max(args.threshold, 0), 255)
    font_path = find_font_path(args.font_path)
    matrix = render_glyph_matrix(args.char, font_path, args.font_size, threshold)
    rows = matrix_to_rows(matrix)

    args.output_dir.mkdir(parents=True, exist_ok=True)
    image_path = args.output_dir / args.output_name
    text_path = args.output_dir / args.text_name

    draw_demo_image(args.char, matrix, rows, font_path, image_path, args.scale, args.arrow_width, DEFAULT_SEQUENCE)
    save_text_output(text_path, format_bitmap_text(args.char, rows, DEFAULT_SEQUENCE))

    print(f"Saved image: {image_path}")
    print(f"Saved bitmap text: {text_path}")
    print(f"Font used: {font_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
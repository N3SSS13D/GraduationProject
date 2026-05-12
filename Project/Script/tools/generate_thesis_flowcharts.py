from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

from PIL import Image, ImageDraw, ImageFont


DEFAULT_BG = "#ffffff"
DEFAULT_PANEL = "#ffffff"
DEFAULT_BORDER = "#1f1f1f"
DEFAULT_TEXT = "#111111"
DEFAULT_MUTED = "#666666"
DEFAULT_ACCENT = "#0f172a"
DEFAULT_ACCENT_2 = "#1f2937"
DEFAULT_WARN = "#111111"
DEFAULT_ERROR = "#7a1f1f"
DEFAULT_OK = "#0f5132"
DEFAULT_NODE_FILL = "#ffffff"
DEFAULT_DECISION_FILL = "#ffffff"
DEFAULT_SHADOW = "#e5e7eb"
DEFAULT_ARROW = "#111111"
DEFAULT_FONT_CANDIDATES = (
    r"C:\Windows\Fonts\simhei.ttf",
    r"C:\Windows\Fonts\msyh.ttc",
    r"C:\Windows\Fonts\msyh.ttf",
    r"C:\Windows\Fonts\simsun.ttc",
)


@dataclass(frozen=True)
class Node:
    text: str
    kind: str = "process"
    width: int = 0
    height: int = 0


@dataclass(frozen=True)
class FlowSpec:
    name: str
    title: str
    subtitle: str
    nodes: tuple[Node, ...]
    branches: tuple[tuple[str, str], ...] = ()
    footer: str = ""


FLOW_42 = FlowSpec(
    name="4_2_led_driver",
    title="4.2 LED显示驱动总体流程图",
    subtitle="真实路径：协议接收 -> 图像渲染 -> 行扫描 -> PWM+DMA输出",
    nodes=(
        Node("上电 / 外设初始化\n定时器、PWM、DMA、74HC595、图像缓冲区", kind="terminal", width=420, height=96),
        Node("UART接收协议数据\n图像 / 动画 / 亮度 / 颜色参数", width=420, height=96),
        Node("更新待显示内容\n写入图像存储区与动作参数", width=360, height=90),
        Node("DrawDrv_RebuildFrame()\n前景色 / 动画效果 / 亮度调节", width=400, height=110),
        Node("WS2812DRV_EncodeAllRows()\n将图像缓冲区编码到PWM缓冲区", width=430, height=104),
        Node("PWM + DMA自动输出\n驱动WS2812B逐行点亮", width=380, height=96),
        Node("定时器1行扫描切换\n74HC595选择行电源", width=380, height=96),
        Node("完成一帧显示\n循环等待下一次刷新", kind="terminal", width=320, height=86),
    ),
    footer="适用于固定帧刷新与行扫描显示，通信层只负责更新渲染输入。",
)

FLOW_43 = FlowSpec(
    name="4_3_ai_interface",
    title="4.3 小智AI接口总体流程图",
    subtitle="真实路径：AI端连接PC绘图服务，再经蓝牙转发至LED端",
    nodes=(
        Node("ESP32-S3 / 小智AI启动\n初始化Wi-Fi、UART、蓝牙封装模块", kind="terminal", width=390, height=100),
        Node("建立WebSocket连接\n连接PC端绘图脚本桥接服务", width=390, height=96),
        Node("接收绘图结果\n单帧位图 / 动画序列 / 控制参数", width=380, height=100),
        Node("协议封装\n补充包头、序列号、长度、CRC", width=360, height=96),
        Node("UART发送到HC-05\n转发至LED端", width=330, height=86),
        Node("ACK是否成功", kind="decision", width=260, height=96),
        Node("LED端解析并显示\n执行图像渲染与行扫描", kind="terminal", width=370, height=92),
    ),
    branches=(
        ("ACK成功", "继续下一帧 / 下一包"),
        ("ACK失败", "重试发送"),
    ),
    footer="AI端负责中转与可靠发送，不直接操作LED扫描细节。",
)

FLOW_45 = FlowSpec(
    name="4_5_drawing_script",
    title="4.5 智能绘图脚本总体流程图",
    subtitle="真实路径：MCP请求 -> 选择绘图模式 -> 生成位图 / 动画 -> 输出规范负载",
    nodes=(
        Node("MCP客户端请求工具\nAI大模型选择绘图任务", kind="terminal", width=390, height=100),
        Node("桥接服务接收指令\n选择 draw_frame / draw_python\nshow_text / draw_animation / render_prompt", width=540, height=132),
        Node("输入类型判断\n位图 / 文字 / 代码绘图 / 动画 / 模糊描述", kind="decision", width=420, height=108),
        Node("位图规范化\nbitmap_rows_hex = 64\nhex chars", width=340, height=106),
        Node("文字渲染\n字体加载 + 字符栅格化\n+ 16x16取样", width=360, height=110),
        Node("Python绘图\n执行绘图库语句\n并转位图", width=340, height=102),
        Node("动画序列\n多帧生成 + 帧序号\n+ 帧间隔", width=340, height=110),
        Node("封装前景色编码\nbitmap_rows_hex + primary_rgb888 + sequence", width=390, height=98),
        Node("返回AI端预览 / 转发\nWebSocket输出标准结果", kind="terminal", width=360, height=92),
    ),
    footer="脚本侧只输出项目约定的标准图像负载，不直接处理LED端串口细节。",
)


def parse_args() -> argparse.Namespace:
    repo_root = Path(__file__).resolve().parents[3]
    default_output_dir = repo_root / "Doc" / "毕业论文" / "图片"
    parser = argparse.ArgumentParser(description="Generate thesis flowchart images for sections 4.2, 4.3, and 4.5.")
    parser.add_argument("--output-dir", type=Path, default=default_output_dir, help="Directory for the PNG outputs.")
    parser.add_argument("--font-path", type=Path, default=None, help="Optional font file path.")
    parser.add_argument("--scale", type=float, default=1.0, help="Global scale factor for the diagram canvas.")
    return parser.parse_args()


def find_font_path(font_path: Path | None) -> Path:
    if font_path is not None:
        if font_path.exists():
            return font_path
        raise FileNotFoundError(f"Font file not found: {font_path}")

    for candidate in DEFAULT_FONT_CANDIDATES:
        candidate_path = Path(candidate)
        if candidate_path.exists():
            return candidate_path

    raise FileNotFoundError("No usable Chinese font found. Pass --font-path to a CJK font file.")


def load_font(font_path: Path, size: int) -> ImageFont.FreeTypeFont:
    return ImageFont.truetype(str(font_path), size)


def text_size(draw: ImageDraw.ImageDraw, font: ImageFont.FreeTypeFont, text: str) -> tuple[int, int]:
    bbox = draw.multiline_textbbox((0, 0), text, font=font, spacing=8, align="center")
    return bbox[2] - bbox[0], bbox[3] - bbox[1]


def wrap_text(text: str) -> str:
    return text


def build_layout(spec: FlowSpec, draw: ImageDraw.ImageDraw, title_font: ImageFont.FreeTypeFont,
                 subtitle_font: ImageFont.FreeTypeFont, node_font: ImageFont.FreeTypeFont,
                 footer_font: ImageFont.FreeTypeFont, scale: float) -> dict[str, object]:
    left = int(64 * scale)
    top = int(84 * scale)
    h_gap = int(54 * scale)
    v_gap = int(22 * scale)
    connector_gap = int(52 * scale)

    node_sizes = []
    for node in spec.nodes:
        w = node.width if node.width else int(text_size(draw, node_font, node.text)[0] + 44 * scale)
        h = node.height if node.height else int(text_size(draw, node_font, node.text)[1] + 34 * scale)
        node_sizes.append((w, h))

    width = 0
    for w, _ in node_sizes:
        width = max(width, w)
    canvas_width = max(int(1260 * scale), left * 2 + width + int(140 * scale))

    stacked_height = sum(h for _, h in node_sizes) + v_gap * (len(node_sizes) - 1)
    title_height = text_size(draw, title_font, spec.title)[1]
    subtitle_height = text_size(draw, subtitle_font, spec.subtitle)[1]
    footer_height = text_size(draw, footer_font, spec.footer)[1] if spec.footer else 0

    canvas_height = top + title_height + int(36 * scale) + subtitle_height + int(38 * scale) + stacked_height + int(70 * scale)
    if spec.footer:
        canvas_height += footer_height + int(24 * scale)

    x = left
    y = top + title_height + int(52 * scale)
    positions = []
    for (w, h), node in zip(node_sizes, spec.nodes):
        positions.append((x, y, x + w, y + h, node))
        y += h + v_gap

    return {
        "canvas_width": canvas_width,
        "canvas_height": canvas_height,
        "left": left,
        "top": top,
        "v_gap": v_gap,
        "connector_gap": connector_gap,
        "positions": positions,
        "node_sizes": node_sizes,
        "title_height": title_height,
        "subtitle_height": subtitle_height,
        "footer_height": footer_height,
    }


def draw_rounded_box(draw: ImageDraw.ImageDraw, box: tuple[int, int, int, int], fill: str, outline: str, radius: int,
                     width: int = 2, shadow: bool = True) -> None:
    x0, y0, x1, y1 = box
    if shadow:
        shadow_box = (x0 + 6, y0 + 8, x1 + 6, y1 + 8)
        draw.rounded_rectangle(shadow_box, radius=radius, fill=DEFAULT_SHADOW)
    draw.rounded_rectangle(box, radius=radius, fill=fill, outline=outline, width=width)


def draw_ellipse_box(draw: ImageDraw.ImageDraw, box: tuple[int, int, int, int], fill: str, outline: str,
                     width: int = 2, shadow: bool = True) -> None:
    x0, y0, x1, y1 = box
    if shadow:
        shadow_box = (x0 + 6, y0 + 8, x1 + 6, y1 + 8)
        draw.ellipse(shadow_box, fill=DEFAULT_SHADOW)
    draw.ellipse(box, fill=fill, outline=outline, width=width)


def draw_diamond(draw: ImageDraw.ImageDraw, center: tuple[int, int], size: tuple[int, int], fill: str,
                 outline: str, width: int = 2) -> tuple[tuple[int, int], ...]:
    cx, cy = center
    w, h = size
    points = ((cx, cy - h // 2), (cx + w // 2, cy), (cx, cy + h // 2), (cx - w // 2, cy))
    draw.polygon(points, fill=fill, outline=outline)
    return points


def draw_process_node(draw: ImageDraw.ImageDraw, box: tuple[int, int, int, int], node: Node,
                      font: ImageFont.FreeTypeFont, scale: float) -> None:
    fill = DEFAULT_DECISION_FILL if node.kind == "decision" else DEFAULT_NODE_FILL
    outline = DEFAULT_WARN if node.kind == "decision" else DEFAULT_BORDER
    if node.kind == "terminal":
        draw_ellipse_box(draw, box, fill=fill, outline=outline, width=max(2, int(2 * scale)))
        draw_centered_text(draw, box, node.text, font, DEFAULT_TEXT)
        return

    if node.kind == "decision":
        x0, y0, x1, y1 = box
        draw_diamond(draw, ((x0 + x1) // 2, (y0 + y1) // 2), (x1 - x0, y1 - y0), fill=fill, outline=outline)
        draw_centered_text(draw, box, node.text, font, DEFAULT_TEXT)
        return

    draw_rounded_box(draw, box, fill=fill, outline=outline, radius=int(16 * scale))
    draw_centered_text(draw, box, node.text, font, DEFAULT_TEXT)


def draw_centered_text(draw: ImageDraw.ImageDraw, box: tuple[int, int, int, int], text: str,
                       font: ImageFont.FreeTypeFont, fill: str, spacing: int = 8) -> None:
    x0, y0, x1, y1 = box
    bbox = draw.multiline_textbbox((0, 0), text, font=font, spacing=spacing, align="center")
    text_w = bbox[2] - bbox[0]
    text_h = bbox[3] - bbox[1]
    x = x0 + (x1 - x0 - text_w) // 2
    y = y0 + (y1 - y0 - text_h) // 2 - 2
    draw.multiline_text((x, y), text, font=font, fill=fill, spacing=spacing, align="center")


def draw_line_arrow(draw: ImageDraw.ImageDraw, start: tuple[int, int], end: tuple[int, int], color: str,
                    width: int = 6, head: int = 18) -> None:
    x0, y0 = start
    x1, y1 = end
    draw.line((x0, y0, x1, y1), fill=color, width=width)
    dx = x1 - x0
    dy = y1 - y0
    length = max((dx * dx + dy * dy) ** 0.5, 1.0)
    ux = dx / length
    uy = dy / length
    px = -uy
    py = ux
    left = (x1 - ux * head + px * head * 0.55, y1 - uy * head + py * head * 0.55)
    right = (x1 - ux * head - px * head * 0.55, y1 - uy * head - py * head * 0.55)
    draw.polygon([end, left, right], fill=color)


def draw_connector(draw: ImageDraw.ImageDraw, upper_box: tuple[int, int, int, int], lower_box: tuple[int, int, int, int],
                   color: str, width: int = 5) -> None:
    ux0, uy0, ux1, uy1 = upper_box
    lx0, ly0, lx1, ly1 = lower_box
    start = ((ux0 + ux1) // 2, uy1)
    end = ((lx0 + lx1) // 2, ly0)
    draw_line_arrow(draw, start, end, color, width=width, head=16)


def draw_branch_merge_connector(draw: ImageDraw.ImageDraw, source_box: tuple[int, int, int, int], target_box: tuple[int, int, int, int],
                                color: str, width: int = 4) -> None:
    sx0, sy0, sx1, sy1 = source_box
    tx0, ty0, tx1, ty1 = target_box
    start = ((sx0 + sx1) // 2, sy1)
    end = ((tx0 + tx1) // 2, ty0)
    draw_line_arrow(draw, start, end, color, width=width, head=14)


def measure_node(draw: ImageDraw.ImageDraw, font: ImageFont.FreeTypeFont, node: Node, scale: float) -> tuple[int, int]:
    width = node.width if node.width else int(text_size(draw, font, node.text)[0] + 44 * scale)
    height = node.height if node.height else int(text_size(draw, font, node.text)[1] + 34 * scale)
    return width, height


def draw_branching_flow45(draw: ImageDraw.ImageDraw, spec: FlowSpec, layout: dict[str, object], font: ImageFont.FreeTypeFont,
                          footer_font: ImageFont.FreeTypeFont, scale: float) -> None:
    left = layout["left"]
    top = layout["top"]
    title_height = layout["title_height"]

    node0 = spec.nodes[0]
    node1 = spec.nodes[1]
    node2 = spec.nodes[2]
    branch_nodes = spec.nodes[3:7]
    merge_node = spec.nodes[7]
    final_node = spec.nodes[8]

    node0_w, node0_h = measure_node(draw, font, node0, scale)
    node1_w, node1_h = measure_node(draw, font, node1, scale)
    node2_w, node2_h = measure_node(draw, font, node2, scale)
    branch_width = int(290 * scale)
    branch_height = int(100 * scale)
    merge_w, merge_h = measure_node(draw, font, merge_node, scale)
    final_w, final_h = measure_node(draw, font, final_node, scale)

    center_x = layout["canvas_width"] // 2
    current_y = top + title_height + int(48 * scale)

    box0 = (center_x - node0_w // 2, current_y, center_x + node0_w // 2, current_y + node0_h)
    current_y = box0[3] + int(28 * scale)
    box1 = (center_x - node1_w // 2, current_y, center_x + node1_w // 2, current_y + node1_h)
    current_y = box1[3] + int(34 * scale)
    box2 = (center_x - node2_w // 2, current_y, center_x + node2_w // 2, current_y + node2_h)
    branch_y = box2[3] + int(62 * scale)

    column_gap = int(46 * scale)
    total_branch_width = branch_width * 4 + column_gap * 3
    branch_start_x = center_x - total_branch_width // 2
    branch_boxes = []
    for index, node in enumerate(branch_nodes):
        x0 = branch_start_x + index * (branch_width + column_gap)
        y0 = branch_y
        branch_boxes.append((x0, y0, x0 + branch_width, y0 + branch_height, node))

    merge_y = branch_y + branch_height + int(54 * scale)
    merge_box = (center_x - merge_w // 2, merge_y, center_x + merge_w // 2, merge_y + merge_h)
    final_y = merge_box[3] + int(34 * scale)
    final_box = (center_x - final_w // 2, final_y, center_x + final_w // 2, final_y + final_h)

    draw_process_node(draw, box0, node0, font, scale)
    draw_process_node(draw, box1, node1, font, scale)
    draw_process_node(draw, box2, node2, font, scale)
    for box in branch_boxes:
        draw_process_node(draw, box[:4], box[4], font, scale)
    draw_process_node(draw, merge_box, merge_node, font, scale)
    draw_process_node(draw, final_box, final_node, font, scale)

    draw_connector(draw, box0, box1, DEFAULT_ARROW, width=max(3, int(4 * scale)))
    draw_connector(draw, box1, box2, DEFAULT_ARROW, width=max(3, int(4 * scale)))

    decision_bottom = ((box2[0] + box2[2]) // 2, box2[3])
    for box in branch_boxes:
        branch_top = ((box[0] + box[2]) // 2, box[1])
        draw_line_arrow(draw, decision_bottom, branch_top, DEFAULT_ARROW, width=max(3, int(4 * scale)), head=14)

    for box in branch_boxes:
        draw_branch_merge_connector(draw, box[:4], merge_box, DEFAULT_ARROW, width=max(3, int(3 * scale)))

    draw_connector(draw, merge_box, final_box, DEFAULT_ARROW, width=max(3, int(4 * scale)))

    if spec.footer:
        footer_y = layout["canvas_height"] - int(42 * scale)
        draw.text((left, footer_y), spec.footer, font=footer_font, fill=DEFAULT_MUTED)


def draw_flowchart(spec: FlowSpec, output_path: Path, font_path: Path, scale: float = 1.0) -> None:
    title_font = load_font(font_path, int(32 * scale))
    subtitle_font = load_font(font_path, int(22 * scale))
    node_font = load_font(font_path, int(20 * scale))
    footer_font = load_font(font_path, int(18 * scale))

    probe = Image.new("RGB", (20, 20), color=DEFAULT_BG)
    probe_draw = ImageDraw.Draw(probe)
    layout = build_layout(spec, probe_draw, title_font, subtitle_font, node_font, footer_font, scale)

    canvas_width = layout["canvas_width"]
    canvas_height = layout["canvas_height"]
    positions = layout["positions"]

    if spec.name == "4_5_drawing_script":
        canvas_width = max(canvas_width, int(1700 * scale))

    image = Image.new("RGB", (canvas_width, canvas_height), color=DEFAULT_BG)
    draw = ImageDraw.Draw(image)

    title_bbox = draw.textbbox((0, 0), spec.title, font=title_font)
    title_width = title_bbox[2] - title_bbox[0]
    draw.text(((canvas_width - title_width) // 2, layout["top"]), spec.title, font=title_font, fill=DEFAULT_TEXT)
    subtitle_y = layout["top"] + layout["title_height"] + int(12 * scale)
    subtitle_bbox = draw.textbbox((0, 0), spec.subtitle, font=subtitle_font)
    subtitle_width = subtitle_bbox[2] - subtitle_bbox[0]
    draw.text(((canvas_width - subtitle_width) // 2, subtitle_y), spec.subtitle, font=subtitle_font, fill=DEFAULT_ACCENT_2)

    if spec.name == "4_5_drawing_script":
        draw_branching_flow45(draw, spec, layout, node_font, footer_font, scale)
        image.save(output_path)
        return

    # Draw nodes.
    for idx, (x0, y0, x1, y1, node) in enumerate(positions):
        box = (x0, y0, x1, y1)
        draw_process_node(draw, box, node, node_font, scale)

    # Primary vertical connectors.
    for idx in range(len(positions) - 1):
        upper = positions[idx][:4]
        lower = positions[idx + 1][:4]
        draw_connector(draw, upper, lower, DEFAULT_ARROW, width=max(4, int(5 * scale)))

    # Branch annotations for flow 4.3.
    if spec.name == "4_3_ai_interface":
        ack_box = positions[5][:4]
        retry_width = int(190 * scale)
        retry_box = (ack_box[2] + int(34 * scale), ack_box[1], ack_box[2] + int(34 * scale) + retry_width, ack_box[3])
        draw_rounded_box(draw, retry_box, fill=DEFAULT_NODE_FILL, outline=DEFAULT_BORDER, radius=int(12 * scale))
        draw_centered_text(draw, retry_box, "重试发送", footer_font, DEFAULT_ERROR)
        success_target = positions[6][:4]
        send_box = positions[4][:4]
        draw_line_arrow(draw, ((ack_box[0] + ack_box[2]) // 2, ack_box[3]), ((success_target[0] + success_target[2]) // 2, success_target[1]), DEFAULT_ARROW, width=4, head=14)
        draw_line_arrow(draw, (retry_box[0], (retry_box[1] + retry_box[3]) // 2), ((send_box[0] + send_box[2]) // 2, send_box[1]), DEFAULT_ARROW, width=4, head=14)
        draw.text((retry_box[0], retry_box[3] + int(8 * scale)), "ACK失败", font=footer_font, fill=DEFAULT_ERROR)
        draw.text((ack_box[2] + int(18 * scale), ack_box[3] + int(4 * scale)), "ACK成功", font=footer_font, fill=DEFAULT_OK)

    if spec.footer:
        footer_y = canvas_height - int(42 * scale)
        draw.text((layout["left"], footer_y), spec.footer, font=footer_font, fill=DEFAULT_MUTED)

    image.save(output_path)


def main() -> int:
    args = parse_args()
    font_path = find_font_path(args.font_path)
    args.output_dir.mkdir(parents=True, exist_ok=True)

    outputs = [
        (FLOW_42, args.output_dir / "flow_4_2_led_driver.png"),
        (FLOW_43, args.output_dir / "flow_4_3_ai_interface.png"),
        (FLOW_45, args.output_dir / "flow_4_5_drawing_script.png"),
    ]

    for spec, output_path in outputs:
        draw_flowchart(spec, output_path, font_path, args.scale)
        print(f"Saved {spec.name}: {output_path}")

    print(f"Font used: {font_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

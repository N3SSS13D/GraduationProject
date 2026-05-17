const pptxgen = require("pptxgenjs");

const pres = new pptxgen();
pres.layout = "LAYOUT_16x9"; // 10" x 5.625"
pres.author = "聂俊宇";
pres.title = "基于MCU的智能LED图示条幅的设计 - 毕业答辩";

// === Color Palette (LED Tech Theme) ===
const C = {
  bg:      "0F1923", // deep dark blue-black
  bg2:     "1A2A3A", // slightly lighter bg for cards
  primary: "00A8CC", // cyan-teal (LED glow)
  accent:  "FF6B35", // warm amber-orange
  white:   "FFFFFF",
  light:   "B0C4DE", // light steel blue for body text
  muted:   "607080", // muted gray-blue
  green:   "00CC88", // green for successes
  red:     "FF4466", // red accent
  card:    "162231", // card background
};

// === Helpers ===
function darkSlide() {
  let s = pres.addSlide();
  s.background = { color: C.bg };
  return s;
}

function addTitle(s, text, y) {
  s.addText(text, {
    x: 0.7, y: y || 0.3, w: 8.6, h: 0.6,
    fontSize: 32, fontFace: "Arial", bold: true,
    color: C.white, margin: 0,
  });
}

function addSubtitle(s, text, y) {
  s.addText(text, {
    x: 0.7, y: y || 0.85, w: 8.6, h: 0.4,
    fontSize: 16, fontFace: "Arial",
    color: C.primary, margin: 0,
  });
}

function addBody(s, items, x, y, w) {
  s.addText(items.map((t, i) => ({
    text: t,
    options: { bullet: true, breakLine: i < items.length - 1, fontSize: 15, color: C.light }
  })), {
    x: x || 0.7, y: y || 1.5, w: w || 4.5, h: 3.5,
    valign: "top", margin: 0, paraSpaceAfter: 8,
  });
}

function addPlaceholder(s, label, x, y, w, h) {
  s.addShape(pres.shapes.RECTANGLE, {
    x: x, y: y, w: w, h: h,
    fill: { color: C.card }, line: { color: C.primary, width: 1.5, dashType: "dash" },
  });
  s.addText("[ " + label + " ]", {
    x: x, y: y, w: w, h: h,
    fontSize: 13, fontFace: "Arial", color: C.muted,
    align: "center", valign: "middle", margin: 0,
  });
}

function addGlowBar(s, y) {
  s.addShape(pres.shapes.RECTANGLE, {
    x: 0, y: y, w: 10, h: 0.04,
    fill: { color: C.primary }, line: { color: C.primary, width: 0 },
  });
}

function addFooter(s, text) {
  s.addText(text || "毕业答辩 — 基于MCU的智能LED图示条幅的设计", {
    x: 0.5, y: 5.15, w: 9, h: 0.3,
    fontSize: 9, fontFace: "Arial", color: C.muted, margin: 0,
  });
  s.addText("聂俊宇 | 电子科学与工程学院", {
    x: 0.5, y: 5.35, w: 9, h: 0.25,
    fontSize: 8, fontFace: "Arial", color: C.muted, margin: 0,
  });
}

function addPageNum(s, n, total) {
  s.addText(n + " / " + total, {
    x: 8.8, y: 5.2, w: 1, h: 0.3,
    fontSize: 9, fontFace: "Arial", color: C.muted, align: "right", margin: 0,
  });
}

const TOTAL = 13;

// ============================================================
// Slide 1: COVER
// ============================================================
let s1 = darkSlide();
s1.addShape(pres.shapes.RECTANGLE, {
  x: 0, y: 0, w: 10, h: 5.625, fill: { color: C.bg },
});
// accent bar
s1.addShape(pres.shapes.RECTANGLE, {
  x: 0.7, y: 1.8, w: 0.06, h: 1.8, fill: { color: C.primary },
});
s1.addText("基于MCU的\n智能LED图示条幅的设计", {
  x: 1.1, y: 1.8, w: 8, h: 1.8,
  fontSize: 36, fontFace: "Arial", bold: true,
  color: C.white, margin: 0, lineSpacingMultiple: 1.2,
});
s1.addText("Design of an Intelligent LED Graphic Banner Based on MCU", {
  x: 1.1, y: 3.5, w: 8, h: 0.4,
  fontSize: 14, fontFace: "Arial", italic: true,
  color: C.primary, margin: 0,
});
s1.addText("聂俊宇  19220309", {
  x: 1.1, y: 4.2, w: 5, h: 0.3,
  fontSize: 16, fontFace: "Arial", color: C.light, margin: 0,
});
s1.addText("电子科学与工程学院  电子信息工程", {
  x: 1.1, y: 4.5, w: 5, h: 0.3,
  fontSize: 12, fontFace: "Arial", color: C.muted, margin: 0,
});
s1.addText("指导教师：李秀英 副教授", {
  x: 1.1, y: 4.8, w: 5, h: 0.3,
  fontSize: 12, fontFace: "Arial", color: C.muted, margin: 0,
});
addPlaceholder(s1, "设备工作照片", 7.0, 1.2, 2.5, 2.8);

// ============================================================
// Slide 2: OUTLINE
// ============================================================
let s2 = darkSlide();
addTitle(s2, "汇报提纲", 0.3);
addGlowBar(s2, 0.95);
let outlineItems = [
  "1. 研究背景与意义",
  "2. 总体设计方案",
  "3. 硬件组成",
  "4. 软件设计 — LED驱动 | AI接口 | 通信协议 | 绘图脚本",
  "5. 功能验证",
  "6. 创新点总结与展望",
];
s2.addText(outlineItems.map((t, i) => ({
  text: t,
  options: { bullet: true, breakLine: i < outlineItems.length - 1, fontSize: 16, color: C.light, paraSpaceAfter: 10 }
})), {
  x: 1.5, y: 1.3, w: 7, h: 3.8, valign: "top", margin: 0,
});
addFooter(s2); addPageNum(s2, 2, TOTAL);

// ============================================================
// Slide 3: BACKGROUND & GOALS
// ============================================================
let s3 = darkSlide();
addTitle(s3, "研究背景与设计目标");
addGlowBar(s3, 0.95);
// Left column: background
s3.addShape(pres.shapes.RECTANGLE, {
  x: 0.5, y: 1.3, w: 4.3, h: 3.8, fill: { color: C.card },
});
s3.addText("研究背景", {
  x: 0.7, y: 1.4, w: 4, h: 0.35,
  fontSize: 16, fontFace: "Arial", bold: true, color: C.primary, margin: 0,
});
addBody(s3, [
  "LED图示条幅广泛应用，传统产品显示固定、修改不便",
  "AI技术（ChatGPT、DeepSeek等）快速发展",
  "\"AI+\" 浪潮推动LED显示从单向显示转向智能交互",
  "小智AI + MCP协议为嵌入式设备提供AI扩展能力",
], 0.7, 1.85, 3.9);

// Right column: goals
s3.addShape(pres.shapes.RECTANGLE, {
  x: 5.2, y: 1.3, w: 4.3, h: 3.8, fill: { color: C.card },
});
s3.addText("设计目标", {
  x: 5.4, y: 1.4, w: 4, h: 0.35,
  fontSize: 16, fontFace: "Arial", bold: true, color: C.accent, margin: 0,
});
addBody(s3, [
  "低功耗、模块化的WS2812B LED矩阵显示",
  "高帧率标准化LED驱动（30fps，PWM+DMA）",
  "蓝牙无线数据传输与控制",
  "语音交互 + AI智能绘图（MCP集成）",
], 5.4, 1.85, 3.9);

addFooter(s3); addPageNum(s3, 3, TOTAL);

// ============================================================
// Slide 4: SYSTEM ARCHITECTURE
// ============================================================
let s4 = darkSlide();
addTitle(s4, "系统总体架构");
addGlowBar(s4, 0.95);
// 4-layer diagram placeholder
addPlaceholder(s4, "系统4层架构框图", 0.5, 1.3, 9.0, 2.8);
// Two-column description
s4.addShape(pres.shapes.RECTANGLE, {
  x: 0.5, y: 4.3, w: 4.3, h: 0.85, fill: { color: C.card },
});
s4.addText("LED端：AI8051U主控 + WS2812矩阵\n  行扫描驱动 + UART协议接收", {
  x: 0.7, y: 4.35, w: 3.9, h: 0.75,
  fontSize: 12, fontFace: "Arial", color: C.light, margin: 0, lineSpacingMultiple: 1.3,
});
s4.addShape(pres.shapes.RECTANGLE, {
  x: 5.2, y: 4.3, w: 4.3, h: 0.85, fill: { color: C.card },
});
s4.addText("AI端：ESP32-S3 + 小智AI\n  语音交互 + 蓝牙传输 + MCP绘图", {
  x: 5.4, y: 4.35, w: 3.9, h: 0.75,
  fontSize: 12, fontFace: "Arial", color: C.light, margin: 0, lineSpacingMultiple: 1.3,
});
addFooter(s4); addPageNum(s4, 4, TOTAL);

// ============================================================
// Slide 5: HARDWARE OVERVIEW
// ============================================================
let s5 = darkSlide();
addTitle(s5, "硬件组成");
addGlowBar(s5, 0.95);
let hwCards = [
  { label: "LED矩阵", desc: "WS2812B × 256\n16×16 模块化\nSMD 5050封装", x: 0.5 },
  { label: "LED控制电路", desc: "AI8051U 主控\nPWM+DMA 驱动\n74HC595 行选", x: 2.6 },
  { label: "蓝牙模块", desc: "HC-05 ×2\n主从配对\n460800bps", x: 4.7 },
  { label: "AI开发板", desc: "ESP32-S3\n立创实战派\n语音+触摸", x: 6.8 },
];
hwCards.forEach(c => {
  s5.addShape(pres.shapes.RECTANGLE, {
    x: c.x, y: 1.3, w: 1.9, h: 2.2, fill: { color: C.card },
  });
  s5.addShape(pres.shapes.RECTANGLE, {
    x: c.x, y: 1.3, w: 1.9, h: 0.04, fill: { color: C.primary },
  });
  s5.addText(c.label, {
    x: c.x + 0.1, y: 1.4, w: 1.7, h: 0.3,
    fontSize: 13, fontFace: "Arial", bold: true, color: C.white, margin: 0,
  });
  s5.addText(c.desc, {
    x: c.x + 0.1, y: 1.75, w: 1.7, h: 1.6,
    fontSize: 11, fontFace: "Arial", color: C.light, margin: 0, lineSpacingMultiple: 1.4,
  });
});
// Hardware photos row
addPlaceholder(s5, "AI8051U开发板照片", 0.5, 3.8, 2.0, 1.2);
addPlaceholder(s5, "LED矩阵PCB照片", 2.75, 3.8, 2.0, 1.2);
addPlaceholder(s5, "HC-05蓝牙模块照片", 5.0, 3.8, 2.0, 1.2);
addPlaceholder(s5, "ESP32-S3开发板照片", 7.25, 3.8, 2.0, 1.2);
addFooter(s5); addPageNum(s5, 5, TOTAL);

// ============================================================
// Slide 6: LED DRIVER (Key Slide)
// ============================================================
let s6 = darkSlide();
addTitle(s6, "LED显示驱动方案");
addGlowBar(s6, 0.95);
// Left: text description
addBody(s6, [
  "PWM+DMA自动生成WS2812时序波形",
  "256×8查找表实时编码，无需CPU干预",
  "双通道双行交织扫描（8步/帧）",
  "双缓冲机制，防止画面撕裂",
  "1ms定时器驱动行扫描 + 32ms帧刷新",
  "协作式任务调度器管理多任务",
], 0.5, 1.3, 4.8);
// Right: encoding pipeline placeholder
addPlaceholder(s6, "PWM编码管线流程图", 5.5, 1.3, 4.0, 2.5);
// Bottom: key specs
s6.addShape(pres.shapes.RECTANGLE, {
  x: 5.5, y: 4.0, w: 4.0, h: 1.1, fill: { color: C.card },
});
s6.addText([
  { text: "PWM: ~691kHz  ", options: { bold: true, color: C.primary, fontSize: 11 } },
  { text: "MCU: 33.18MHz\n", options: { color: C.light, fontSize: 11, breakLine: true } },
  { text: "扫描: 8行对  ", options: { bold: true, color: C.primary, fontSize: 11 } },
  { text: "帧率: ~30fps\n", options: { color: C.light, fontSize: 11, breakLine: true } },
  { text: "缓冲: 32帧×36B  ", options: { bold: true, color: C.primary, fontSize: 11 } },
  { text: "DMA: CH1+CH2", options: { color: C.light, fontSize: 11 } },
], { x: 5.7, y: 4.1, w: 3.6, h: 0.9, margin: 0 });
addFooter(s6); addPageNum(s6, 6, TOTAL);

// ============================================================
// Slide 7: AI INTERFACE (Key Slide)
// ============================================================
let s7 = darkSlide();
addTitle(s7, "AI端接口调度方案");
addGlowBar(s7, 0.95);
// Two control paths
let makeCard7 = () => ({ fill: { color: C.card }, shadow: { type: "outer", blur: 4, offset: 2, angle: 135, color: "000000", opacity: 0.3 } });
s7.addShape(pres.shapes.RECTANGLE, { x: 0.5, y: 1.3, w: 4.3, h: 3.5, ...makeCard7() });
s7.addShape(pres.shapes.RECTANGLE, { x: 0.5, y: 1.3, w: 4.3, h: 0.04, fill: { color: C.green } });
s7.addText("路径A：本地触摸/语音关键词", {
  x: 0.7, y: 1.4, w: 3.9, h: 0.3,
  fontSize: 14, fontFace: "Arial", bold: true, color: C.green, margin: 0,
});
addBody(s7, [
  "触摸UI / 关键词检测",
  "→ SetAction 直发蓝牙",
  "延迟 < 20ms",
  "无需网络往返",
  "适用：图案切换、时钟、效果",
], 0.7, 1.8, 3.9);

s7.addShape(pres.shapes.RECTANGLE, { x: 5.2, y: 1.3, w: 4.3, h: 3.5, ...makeCard7() });
s7.addShape(pres.shapes.RECTANGLE, { x: 5.2, y: 1.3, w: 4.3, h: 0.04, fill: { color: C.accent } });
s7.addText("路径B：自由语音/MCP绘图", {
  x: 5.4, y: 1.4, w: 3.9, h: 0.3,
  fontSize: 14, fontFace: "Arial", bold: true, color: C.accent, margin: 0,
});
addBody(s7, [
  "语音 → MCP工具调用",
  "→ 主机Python绘图",
  "→ WebSocket回传结果",
  "→ 蓝牙上传到LED",
  "适用：自由绘图、动画、字幕",
], 5.4, 1.8, 3.9);

addPlaceholder(s7, "双路径控制流图", 8.0, 5.05, 1.5, 0); // hidden
addFooter(s7); addPageNum(s7, 7, TOTAL);

// ============================================================
// Slide 8: PROTOCOL DESIGN
// ============================================================
let s8 = darkSlide();
addTitle(s8, "蓝牙通信协议设计");
addGlowBar(s8, 0.95);
// Packet format
s8.addShape(pres.shapes.RECTANGLE, {
  x: 0.5, y: 1.3, w: 4.3, h: 1.2, fill: { color: C.card },
});
s8.addText("包格式（V3紧凑版，6字节包头）", {
  x: 0.7, y: 1.35, w: 3.9, h: 0.25,
  fontSize: 13, fontFace: "Arial", bold: true, color: C.primary, margin: 0,
});
s8.addText("Magic(47) | Flags | Seq | Cmd | Len | CRC8 | Payload... | CRC16", {
  x: 0.7, y: 1.65, w: 3.9, h: 0.6,
  fontSize: 11, fontFace: "Consolas", color: C.light, margin: 0, lineSpacingMultiple: 1.5,
});
s8.addShape(pres.shapes.RECTANGLE, {
  x: 5.2, y: 1.3, w: 4.3, h: 1.2, fill: { color: C.card },
});
s8.addText("三种图像格式", {
  x: 5.4, y: 1.35, w: 3.9, h: 0.25,
  fontSize: 13, fontFace: "Arial", bold: true, color: C.primary, margin: 0,
});
s8.addText([
  { text: "RGB332: 256B 全色任意帧\n", options: { color: C.light, fontSize: 11, breakLine: true } },
  { text: "紧凑位图: 38B 单色图案\n", options: { color: C.light, fontSize: 11, breakLine: true } },
  { text: "分层位图: 36B/层 多色叠加 (≤4层144B单包)", options: { color: C.light, fontSize: 11 } },
], { x: 5.4, y: 1.65, w: 3.9, h: 0.7, margin: 0 });
// Features
addBody(s8, [
  "双CRC校验（header_crc8 + packet_crc16）抗噪",
  "Request/Reply事务模型，ACK精准匹配",
  "轻量命令单包直传（≤144B无需分包）",
  "动画最多32帧，帧间隔1~65535ms可调",
  "4类命令：参数控制、整帧图像、动画、轻量图像",
], 0.5, 2.8, 9.0);
addFooter(s8); addPageNum(s8, 8, TOTAL);

// ============================================================
// Slide 9: MCP DRAWING SCRIPTS
// ============================================================
let s9 = darkSlide();
addTitle(s9, "智能绘图脚本与MCP集成");
addGlowBar(s9, 0.95);
s9.addShape(pres.shapes.RECTANGLE, {
  x: 0.5, y: 1.3, w: 5.5, h: 3.8, fill: { color: C.card },
});
s9.addText("MCP绘图工具集", {
  x: 0.7, y: 1.4, w: 5, h: 0.3,
  fontSize: 15, fontFace: "Arial", bold: true, color: C.primary, margin: 0,
});
let tools = [
  "draw_python — AST白名单安全绘图（Pillow受限执行）",
  "render_prompt — 自然语言→图案模板",
  "show_text — 文本→字形→16×16帧序列",
  "show_scroll_subtitle — 长文本→离屏位图→滚动分块",
  "show_effect — 原生效果直连（无需逐帧渲染）",
  "draw_animation — 多帧动画+自动重采样",
];
addBody(s9, tools, 0.7, 1.8, 5.0);
// MCP flow placeholder
addPlaceholder(s9, "MCP架构流程图\nLLM↔MCP↔Bridge↔ESP32↔LED", 6.3, 1.3, 3.2, 3.8);
addFooter(s9); addPageNum(s9, 9, TOTAL);

// ============================================================
// Slide 10: DATA FLOW
// ============================================================
let s10 = darkSlide();
addTitle(s10, "端到端数据流：\"画一个红心\"");
addGlowBar(s10, 0.95);
s10.addText("用户语音 → MCP工具 → Python绘图 → JSON结果 → WebSocket → ESP32 → 蓝牙 → AI8051U → WS2812显示", {
  x: 0.5, y: 1.3, w: 9, h: 0.5,
  fontSize: 13, fontFace: "Arial", bold: true, color: C.white, margin: 0, align: "center",
});
// Data format stages
let stages = [
  { stage: "① 语音", desc: "\"画一个红心\"", color: C.accent },
  { stage: "② 脚本", desc: "bitmap_rows_hex\n[0x0000,0x07E0...]\nrgb:[255,0,0]", color: C.primary },
  { stage: "③ 封包", desc: "47 01 05 18 24 XX\n[36B分层数据]\nYY YY", color: C.green },
  { stage: "④ 显示", desc: "LED矩阵\n显示红心♥", color: C.red },
];
stages.forEach((st, i) => {
  let sx = 0.5 + i * 2.3;
  s10.addShape(pres.shapes.RECTANGLE, {
    x: sx, y: 2.1, w: 2.1, h: 2.8, fill: { color: C.card },
  });
  s10.addShape(pres.shapes.RECTANGLE, {
    x: sx, y: 2.1, w: 2.1, h: 0.04, fill: { color: st.color },
  });
  s10.addText(st.stage, {
    x: sx + 0.1, y: 2.2, w: 1.9, h: 0.3,
    fontSize: 14, fontFace: "Arial", bold: true, color: st.color, margin: 0,
  });
  s10.addText(st.desc, {
    x: sx + 0.1, y: 2.55, w: 1.9, h: 2.2,
    fontSize: 11, fontFace: "Arial", color: C.light, margin: 0, lineSpacingMultiple: 1.4,
  });
  // Arrow between stages
  if (i < stages.length - 1) {
    s10.addText("→", {
      x: sx + 2.0, y: 3.2, w: 0.4, h: 0.4,
      fontSize: 20, fontFace: "Arial", color: C.muted, align: "center", margin: 0,
    });
  }
});
addFooter(s10); addPageNum(s10, 10, TOTAL);

// ============================================================
// Slide 11: TESTING
// ============================================================
let s11 = darkSlide();
addTitle(s11, "功能验证");
addGlowBar(s11, 0.95);
// 2x2 grid
let tests = [
  { title: "显示效果验证", items: ["纯色/渐变/图案/动画", "30fps流畅播放", "多种效果切换正常"] },
  { title: "通信协议验证", items: ["CRC校验通过率100%", "ACK匹配准确", "分包重组正确"] },
  { title: "智能绘图验证", items: ["自然语言→图案→显示", "AST安全沙箱有效", "show_text/scroll正常"] },
  { title: "AI语音控制验证", items: ["唤醒词检测准确", "关键词→对应动作", "自由绘图端到端通"] },
];
tests.forEach((t, i) => {
  let tx = 0.5 + (i % 2) * 4.7;
  let ty = 1.3 + Math.floor(i / 2) * 1.9;
  s11.addShape(pres.shapes.RECTANGLE, {
    x: tx, y: ty, w: 4.4, h: 1.7, fill: { color: C.card },
  });
  s11.addShape(pres.shapes.RECTANGLE, {
    x: tx, y: ty, w: 0.06, h: 1.7, fill: { color: C.primary },
  });
  s11.addText(t.title, {
    x: tx + 0.2, y: ty + 0.1, w: 4.0, h: 0.3,
    fontSize: 14, fontFace: "Arial", bold: true, color: C.white, margin: 0,
  });
  addBody(s11, t.items, tx + 0.2, ty + 0.45, 4.0);
});
addFooter(s11); addPageNum(s11, 11, TOTAL);

// ============================================================
// Slide 12: INNOVATION & RESULTS
// ============================================================
let s12 = darkSlide();
addTitle(s12, "创新点总结");
addGlowBar(s12, 0.95);
let innovations = [
  { num: "1", title: "双CRC轻量级协议", desc: "header_crc8+packet_crc16\n双层校验保障蓝牙传输可靠性" },
  { num: "2", title: "MCP+LLM智能绘图", desc: "标准化MCP协议集成LLM\n自然语言控制LED显示" },
  { num: "3", title: "分层位图压缩", desc: "36B/层灵活多色叠加\n最高5倍传输压缩比" },
  { num: "4", title: "PWM+DMA波形生成", desc: "256×8查找表实时编码\nCPU零开销时序输出" },
  { num: "5", title: "统一显示配置架构", desc: "显示配置文件解耦\n本地/远程渲染统一调度" },
];
innovations.forEach((inv, i) => {
  let ix = 0.3 + i * 1.9;
  s12.addShape(pres.shapes.RECTANGLE, {
    x: ix, y: 1.3, w: 1.75, h: 3.5, fill: { color: C.card },
  });
  // Number circle
  s12.addShape(pres.shapes.OVAL, {
    x: ix + 0.55, y: 1.4, w: 0.65, h: 0.65,
    fill: { color: C.primary },
  });
  s12.addText(inv.num, {
    x: ix + 0.55, y: 1.4, w: 0.65, h: 0.65,
    fontSize: 22, fontFace: "Arial", bold: true, color: C.white, align: "center", valign: "middle", margin: 0,
  });
  s12.addText(inv.title, {
    x: ix + 0.1, y: 2.2, w: 1.55, h: 0.5,
    fontSize: 12, fontFace: "Arial", bold: true, color: C.white, align: "center", margin: 0,
  });
  s12.addText(inv.desc, {
    x: ix + 0.1, y: 2.8, w: 1.55, h: 1.5,
    fontSize: 10, fontFace: "Arial", color: C.light, align: "center", margin: 0, lineSpacingMultiple: 1.4,
  });
});
addFooter(s12); addPageNum(s12, 12, TOTAL);

// ============================================================
// Slide 13: THANKS
// ============================================================
let s13 = darkSlide();
s13.addShape(pres.shapes.RECTANGLE, {
  x: 0, y: 0, w: 10, h: 5.625, fill: { color: C.bg },
});
s13.addText("感谢聆听", {
  x: 1, y: 1.5, w: 8, h: 1.2,
  fontSize: 52, fontFace: "Arial", bold: true, color: C.white, align: "center", margin: 0,
});
s13.addText("请各位老师批评指正", {
  x: 1, y: 2.8, w: 8, h: 0.6,
  fontSize: 20, fontFace: "Arial", color: C.primary, align: "center", margin: 0,
});
s13.addText("聂俊宇  电子科学与工程学院  电子信息工程", {
  x: 1, y: 4.0, w: 8, h: 0.4,
  fontSize: 14, fontFace: "Arial", color: C.light, align: "center", margin: 0,
});
s13.addText("指导教师：李秀英 副教授", {
  x: 1, y: 4.4, w: 8, h: 0.4,
  fontSize: 14, fontFace: "Arial", color: C.muted, align: "center", margin: 0,
});
addPageNum(s13, 13, TOTAL);

// ============================================================
// OUTPUT
// ============================================================
pres.writeFile({ fileName: "D:/GraduationProject/Doc/PPT/毕业答辩-聂俊宇.pptx" })
  .then(() => console.log("PPT generated successfully: 毕业答辩-聂俊宇.pptx (13 slides)"))
  .catch(err => console.error("Error:", err));

#!/usr/bin/env python3
# -*- coding: ascii -*-
"""LED image converter GUI.

Features:
1. Load image and convert to 16x16.
2. Pixel-sampling downscale mode to preserve contour.
3. Crop selectable region with preview before conversion.
4. GUI with original, cropped, 16x16, and LED simulation previews.
5. Extract 16x16 glyphs from Chinese text for firmware arrays.
"""

import tkinter as tk
from tkinter import colorchooser, filedialog, messagebox, ttk

try:
    from PIL import Image, ImageDraw, ImageFilter, ImageFont, ImageOps, ImageTk
except Exception as exc:  # pragma: no cover
    raise SystemExit(
        "Pillow is required. Install with: pip install pillow\n"
        f"Import error: {exc}"
    )


PREVIEW_SIZE = 320
TARGET_SIZE = 16


class LedImageConverterApp:
    def __init__(self, root):
        self.root = root
        self.root.title("LED 16x16 Image Converter")
        self.root.geometry("1680x860")
        self.root.minsize(1360, 760)

        self.original_image = None
        self.display_image = None
        self.crop_image = None
        self.result_image = None

        self.tk_original = None
        self.tk_crop = None
        self.tk_result = None

        self.scale_x = 1.0
        self.scale_y = 1.0

        self.dragging = False
        self.drag_mode = "new"
        self.drag_start_x = 0
        self.drag_start_y = 0
        self.drag_offset_x = 0
        self.drag_offset_y = 0
        self.crop_rect_id = None
        self.last_painted_pixel = (-1, -1)

        self.image_path_var = tk.StringVar(value="No image loaded")
        self.crop_x_var = tk.IntVar(value=0)
        self.crop_y_var = tk.IntVar(value=0)
        self.crop_w_var = tk.IntVar(value=64)
        self.crop_h_var = tk.IntVar(value=64)
        self.method_var = tk.StringVar(value="feature")
        self.color_order_var = tk.StringVar(value="RGB")
        self.array_name_var = tk.StringVar(value="g_ledImage16x16")
        self.pattern_name_var = tk.StringVar(value="custom0")
        self.edit_enable_var = tk.BooleanVar(value=True)
        self.primary_color_var = tk.StringVar(value="#FF4040")
        self.secondary_color_var = tk.StringVar(value="#2080FF")
        self.binary_enable_var = tk.BooleanVar(value=False)
        self.binary_threshold_var = tk.IntVar(value=128)
        self.binary_dark_color_var = tk.StringVar(value="#000000")
        self.binary_light_color_var = tk.StringVar(value="#FFFFFF")
        self.edge_gradient_enable_var = tk.BooleanVar(value=True)
        self.edge_gradient_strength_var = tk.IntVar(value=96)
        self.glyph_text_var = tk.StringVar(value="JilinUniversity")
        self.glyph_font_path_var = tk.StringVar(value="C:/Windows/Fonts/simhei.ttf")
        self.glyph_font_size_var = tk.IntVar(value=16)
        self.glyph_threshold_var = tk.IntVar(value=96)
        self.glyph_name_var = tk.StringVar(value="jilin_university")

        self._build_layout()
        self._bind_events()

    def _build_layout(self):
        top_frame = ttk.Frame(self.root, padding=8)
        top_frame.pack(fill=tk.X)

        ttk.Button(top_frame, text="Open Image", command=self.open_image).pack(side=tk.LEFT)
        ttk.Label(top_frame, textvariable=self.image_path_var, width=90).pack(side=tk.LEFT, padx=8)

        control_frame = ttk.LabelFrame(self.root, text="Crop and Convert Controls", padding=10)
        control_frame.pack(fill=tk.X, padx=8, pady=(0, 8))

        ttk.Label(control_frame, text="X:").grid(row=0, column=0, sticky=tk.W, padx=(0, 4))
        self.crop_x_spin = ttk.Spinbox(
            control_frame,
            from_=0,
            to=99999,
            textvariable=self.crop_x_var,
            width=8,
            command=self.update_all_previews,
        )
        self.crop_x_spin.grid(row=0, column=1, sticky=tk.W, padx=(0, 10))

        ttk.Label(control_frame, text="Y:").grid(row=0, column=2, sticky=tk.W, padx=(0, 4))
        self.crop_y_spin = ttk.Spinbox(
            control_frame,
            from_=0,
            to=99999,
            textvariable=self.crop_y_var,
            width=8,
            command=self.update_all_previews,
        )
        self.crop_y_spin.grid(row=0, column=3, sticky=tk.W, padx=(0, 10))

        ttk.Label(control_frame, text="Width:").grid(row=0, column=4, sticky=tk.W, padx=(0, 4))
        self.crop_w_spin = ttk.Spinbox(
            control_frame,
            from_=1,
            to=99999,
            textvariable=self.crop_w_var,
            width=8,
            command=self.update_all_previews,
        )
        self.crop_w_spin.grid(row=0, column=5, sticky=tk.W, padx=(0, 10))

        ttk.Label(control_frame, text="Height:").grid(row=0, column=6, sticky=tk.W, padx=(0, 4))
        self.crop_h_spin = ttk.Spinbox(
            control_frame,
            from_=1,
            to=99999,
            textvariable=self.crop_h_var,
            width=8,
            command=self.update_all_previews,
        )
        self.crop_h_spin.grid(row=0, column=7, sticky=tk.W, padx=(0, 12))

        ttk.Label(control_frame, text="Mode:").grid(row=0, column=8, sticky=tk.W, padx=(0, 4))
        mode_combo = ttk.Combobox(
            control_frame,
            width=22,
            state="readonly",
            textvariable=self.method_var,
            values=["feature", "sample", "nearest"],
        )
        mode_combo.grid(row=0, column=9, sticky=tk.W, padx=(0, 10))

        ttk.Label(control_frame, text="Color Order:").grid(row=0, column=10, sticky=tk.W, padx=(0, 4))
        order_combo = ttk.Combobox(
            control_frame,
            width=8,
            state="readonly",
            textvariable=self.color_order_var,
            values=["RGB", "GRB", "BGR"],
        )
        order_combo.grid(row=0, column=11, sticky=tk.W, padx=(0, 10))

        ttk.Label(control_frame, text="Array Name:").grid(row=0, column=12, sticky=tk.W, padx=(0, 4))
        ttk.Entry(control_frame, textvariable=self.array_name_var, width=20).grid(
            row=0, column=13, sticky=tk.W, padx=(0, 10)
        )

        ttk.Label(control_frame, text="Pattern Name:").grid(row=0, column=14, sticky=tk.W, padx=(0, 4))
        ttk.Entry(control_frame, textvariable=self.pattern_name_var, width=14).grid(
            row=0, column=15, sticky=tk.W, padx=(0, 10)
        )

        ttk.Button(control_frame, text="Convert", command=self.update_all_previews).grid(
            row=0, column=16, padx=(0, 8)
        )
        ttk.Button(control_frame, text="Copy Array", command=self.copy_array).grid(row=0, column=17)

        ttk.Checkbutton(control_frame, text="Binary", variable=self.binary_enable_var).grid(
            row=1, column=0, sticky=tk.W, pady=(8, 0)
        )
        ttk.Label(control_frame, text="Threshold:").grid(row=1, column=1, sticky=tk.W, pady=(8, 0))
        ttk.Spinbox(
            control_frame,
            from_=0,
            to=255,
            textvariable=self.binary_threshold_var,
            width=6,
            command=self.update_all_previews,
        ).grid(row=1, column=2, sticky=tk.W, pady=(8, 0))
        ttk.Label(control_frame, text="Dark:").grid(row=1, column=3, sticky=tk.W, pady=(8, 0))
        ttk.Entry(control_frame, textvariable=self.binary_dark_color_var, width=9).grid(
            row=1, column=4, sticky=tk.W, pady=(8, 0)
        )
        ttk.Label(control_frame, text="Light:").grid(row=1, column=5, sticky=tk.W, pady=(8, 0))
        ttk.Entry(control_frame, textvariable=self.binary_light_color_var, width=9).grid(
            row=1, column=6, sticky=tk.W, pady=(8, 0)
        )
        ttk.Checkbutton(
            control_frame,
            text="Edge Gradient",
            variable=self.edge_gradient_enable_var,
            command=self.update_all_previews,
        ).grid(row=1, column=7, sticky=tk.W, pady=(8, 0), padx=(8, 0))
        ttk.Label(control_frame, text="Strength:").grid(row=1, column=8, sticky=tk.W, pady=(8, 0))
        ttk.Spinbox(
            control_frame,
            from_=0,
            to=255,
            textvariable=self.edge_gradient_strength_var,
            width=6,
            command=self.update_all_previews,
        ).grid(row=1, column=9, sticky=tk.W, pady=(8, 0))

        ttk.Checkbutton(control_frame, text="Enable Pixel Edit", variable=self.edit_enable_var).grid(
            row=2, column=0, columnspan=2, sticky=tk.W, pady=(8, 0)
        )
        ttk.Button(control_frame, text="Primary Color", command=self.choose_primary_color).grid(
            row=2, column=2, columnspan=2, sticky=tk.W, pady=(8, 0)
        )
        ttk.Entry(control_frame, textvariable=self.primary_color_var, width=10).grid(
            row=2, column=4, sticky=tk.W, pady=(8, 0)
        )
        ttk.Button(control_frame, text="Secondary Color", command=self.choose_secondary_color).grid(
            row=2, column=5, columnspan=2, sticky=tk.W, pady=(8, 0)
        )
        ttk.Entry(control_frame, textvariable=self.secondary_color_var, width=10).grid(
            row=2, column=7, sticky=tk.W, pady=(8, 0)
        )
        ttk.Label(
            control_frame,
            text="Left paint: primary, Right paint: secondary, drag inside red box on Original to move selection",
        ).grid(row=2, column=8, columnspan=8, sticky=tk.W, pady=(8, 0))

        ttk.Label(control_frame, text="Glyph Text:").grid(row=3, column=0, sticky=tk.W, pady=(8, 0))
        ttk.Entry(control_frame, textvariable=self.glyph_text_var, width=18).grid(
            row=3, column=1, columnspan=2, sticky=tk.W, pady=(8, 0)
        )
        ttk.Label(control_frame, text="Glyph Name:").grid(row=3, column=3, sticky=tk.W, pady=(8, 0))
        ttk.Entry(control_frame, textvariable=self.glyph_name_var, width=18).grid(
            row=3, column=4, columnspan=2, sticky=tk.W, pady=(8, 0)
        )
        ttk.Label(control_frame, text="Font Size:").grid(row=3, column=6, sticky=tk.W, pady=(8, 0))
        ttk.Spinbox(
            control_frame,
            from_=8,
            to=96,
            textvariable=self.glyph_font_size_var,
            width=6,
        ).grid(row=3, column=7, sticky=tk.W, pady=(8, 0))
        ttk.Label(control_frame, text="Threshold:").grid(row=3, column=8, sticky=tk.W, pady=(8, 0))
        ttk.Spinbox(
            control_frame,
            from_=0,
            to=255,
            textvariable=self.glyph_threshold_var,
            width=6,
        ).grid(row=3, column=9, sticky=tk.W, pady=(8, 0))
        ttk.Entry(control_frame, textvariable=self.glyph_font_path_var, width=40).grid(
            row=3, column=10, columnspan=4, sticky=tk.W, pady=(8, 0), padx=(0, 6)
        )
        ttk.Button(control_frame, text="Font...", command=self.select_glyph_font).grid(
            row=3, column=14, sticky=tk.W, pady=(8, 0)
        )
        ttk.Button(control_frame, text="Extract Glyph", command=self.extract_glyph_arrays).grid(
            row=3, column=15, columnspan=2, sticky=tk.W, pady=(8, 0), padx=(8, 0)
        )

        self.method_var.trace_add("write", lambda *_: self.update_all_previews())
        self.color_order_var.trace_add("write", lambda *_: self.update_led_array_text())
        self.array_name_var.trace_add("write", lambda *_: self.update_led_array_text())
        self.pattern_name_var.trace_add("write", lambda *_: self.update_led_array_text())
        self.binary_enable_var.trace_add("write", lambda *_: self.update_all_previews())
        self.binary_threshold_var.trace_add("write", lambda *_: self.update_all_previews())
        self.binary_dark_color_var.trace_add("write", lambda *_: self.update_all_previews())
        self.binary_light_color_var.trace_add("write", lambda *_: self.update_all_previews())
        self.edge_gradient_enable_var.trace_add("write", lambda *_: self.update_all_previews())
        self.edge_gradient_strength_var.trace_add("write", lambda *_: self.update_all_previews())

        view_frame = ttk.Frame(self.root, padding=8)
        view_frame.pack(fill=tk.BOTH, expand=True)

        left = ttk.LabelFrame(view_frame, text="Original (drag outside: new, drag inside box: move)", padding=8)
        left.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 6))

        middle = ttk.LabelFrame(view_frame, text="Selected Crop", padding=8)
        middle.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=6)

        right = ttk.LabelFrame(view_frame, text="16x16 Preview (Editable)", padding=8)
        right.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=6)

        led = ttk.LabelFrame(view_frame, text="LED Simulation", padding=8)
        led.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(6, 0))

        self.original_canvas = tk.Canvas(left, bg="#111111", width=PREVIEW_SIZE, height=PREVIEW_SIZE)
        self.original_canvas.pack(fill=tk.BOTH, expand=True)

        self.crop_canvas = tk.Canvas(middle, bg="#111111", width=PREVIEW_SIZE, height=PREVIEW_SIZE)
        self.crop_canvas.pack(fill=tk.BOTH, expand=True)

        self.result_canvas = tk.Canvas(right, bg="#111111", width=PREVIEW_SIZE, height=PREVIEW_SIZE)
        self.result_canvas.pack(fill=tk.BOTH, expand=True)

        self.led_canvas = tk.Canvas(led, bg="#080808", width=PREVIEW_SIZE, height=PREVIEW_SIZE)
        self.led_canvas.pack(fill=tk.BOTH, expand=True)

        output_frame = ttk.LabelFrame(self.root, text="LED C Array Output", padding=8)
        output_frame.pack(fill=tk.BOTH, expand=False, padx=8, pady=(0, 8))

        self.output_text = tk.Text(output_frame, height=13, wrap=tk.NONE)
        self.output_text.pack(fill=tk.BOTH, expand=True)

    def _bind_events(self):
        self.original_canvas.bind("<Button-1>", self.on_mouse_down)
        self.original_canvas.bind("<B1-Motion>", self.on_mouse_drag)
        self.original_canvas.bind("<ButtonRelease-1>", self.on_mouse_up)
        self.result_canvas.bind("<Button-1>", self.on_result_paint_primary)
        self.result_canvas.bind("<B1-Motion>", self.on_result_paint_primary)
        self.result_canvas.bind("<Button-3>", self.on_result_paint_secondary)
        self.result_canvas.bind("<B3-Motion>", self.on_result_paint_secondary)
        self.result_canvas.bind("<ButtonRelease-1>", self.on_result_paint_release)
        self.result_canvas.bind("<ButtonRelease-3>", self.on_result_paint_release)

        self.crop_x_spin.bind("<KeyRelease>", lambda *_: self.update_all_previews())
        self.crop_y_spin.bind("<KeyRelease>", lambda *_: self.update_all_previews())
        self.crop_w_spin.bind("<KeyRelease>", lambda *_: self.update_all_previews())
        self.crop_h_spin.bind("<KeyRelease>", lambda *_: self.update_all_previews())

    def open_image(self):
        file_path = filedialog.askopenfilename(
            title="Select image",
            filetypes=[
                ("Image files", "*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.webp"),
                ("All files", "*.*"),
            ],
        )
        if not file_path:
            return

        try:
            image = Image.open(file_path).convert("RGB")
        except Exception as exc:
            messagebox.showerror("Open error", f"Failed to load image:\n{exc}")
            return

        self.original_image = image
        self.image_path_var.set(file_path)

        init_w = min(64, image.width)
        init_h = min(64, image.height)
        self.crop_w_var.set(max(1, init_w))
        self.crop_h_var.set(max(1, init_h))
        self.crop_x_var.set(max(0, (image.width - init_w) // 2))
        self.crop_y_var.set(max(0, (image.height - init_h) // 2))

        self.update_all_previews()

    def update_all_previews(self):
        if self.original_image is None:
            return

        self._normalize_crop_rect()
        self._draw_original_with_rect()
        self._update_crop_preview()
        self._update_result_preview()
        self.update_led_array_text()

    def _normalize_crop_rect(self):
        img_w, img_h = self.original_image.size

        crop_w = max(1, int(self.crop_w_var.get()))
        crop_h = max(1, int(self.crop_h_var.get()))

        crop_w = min(crop_w, img_w)
        crop_h = min(crop_h, img_h)

        crop_x = max(0, int(self.crop_x_var.get()))
        crop_y = max(0, int(self.crop_y_var.get()))

        if crop_x + crop_w > img_w:
            crop_x = img_w - crop_w
        if crop_y + crop_h > img_h:
            crop_y = img_h - crop_h

        self.crop_w_var.set(crop_w)
        self.crop_h_var.set(crop_h)
        self.crop_x_var.set(crop_x)
        self.crop_y_var.set(crop_y)

    def _draw_original_with_rect(self):
        img_w, img_h = self.original_image.size

        display = self.original_image.copy()
        display.thumbnail((PREVIEW_SIZE, PREVIEW_SIZE), Image.Resampling.LANCZOS)
        self.display_image = display

        self.scale_x = img_w / display.width
        self.scale_y = img_h / display.height

        self.tk_original = ImageTk.PhotoImage(display)
        self.original_canvas.delete("all")
        self.original_canvas.create_image(PREVIEW_SIZE // 2, PREVIEW_SIZE // 2, image=self.tk_original)

        crop_x = self.crop_x_var.get()
        crop_y = self.crop_y_var.get()
        crop_w = self.crop_w_var.get()
        crop_h = self.crop_h_var.get()

        left = int((PREVIEW_SIZE - display.width) / 2 + crop_x / self.scale_x)
        top = int((PREVIEW_SIZE - display.height) / 2 + crop_y / self.scale_y)
        right = int((PREVIEW_SIZE - display.width) / 2 + (crop_x + crop_w) / self.scale_x)
        bottom = int((PREVIEW_SIZE - display.height) / 2 + (crop_y + crop_h) / self.scale_y)

        self.crop_rect_id = self.original_canvas.create_rectangle(
            left,
            top,
            right,
            bottom,
            outline="#ff2e2e",
            width=2,
        )

    def _update_crop_preview(self):
        x = self.crop_x_var.get()
        y = self.crop_y_var.get()
        w = self.crop_w_var.get()
        h = self.crop_h_var.get()

        self.crop_image = self.original_image.crop((x, y, x + w, y + h))
        crop_show = self.crop_image.resize((PREVIEW_SIZE, PREVIEW_SIZE), Image.Resampling.NEAREST)

        self.tk_crop = ImageTk.PhotoImage(crop_show)
        self.crop_canvas.delete("all")
        self.crop_canvas.create_image(PREVIEW_SIZE // 2, PREVIEW_SIZE // 2, image=self.tk_crop)

    def _update_result_preview(self):
        if self.crop_image is None:
            return

        method = self.method_var.get()
        if method == "sample":
            base = self._sample_to_16(self.crop_image)
        elif method == "nearest":
            base = self.crop_image.resize((TARGET_SIZE, TARGET_SIZE), Image.Resampling.NEAREST)
        else:
            base = self._feature_to_16(self.crop_image)

        if self.binary_enable_var.get():
            self.result_image = self._apply_binarization(base)
        else:
            self.result_image = base

        if self.edge_gradient_enable_var.get():
            self.result_image = self._apply_edge_gradient(self.result_image)

        self._refresh_result_views()

    def _refresh_result_views(self):
        if self.result_image is None:
            return

        enlarged = self.result_image.resize((PREVIEW_SIZE, PREVIEW_SIZE), Image.Resampling.NEAREST)
        self.tk_result = ImageTk.PhotoImage(enlarged)

        self.result_canvas.delete("all")
        self.result_canvas.create_image(PREVIEW_SIZE // 2, PREVIEW_SIZE // 2, image=self.tk_result)

        # Draw grid to show each LED pixel block clearly.
        grid_color = "#2f2f2f"
        cell = PREVIEW_SIZE / TARGET_SIZE
        for i in range(TARGET_SIZE + 1):
            p = i * cell
            self.result_canvas.create_line(p, 0, p, PREVIEW_SIZE, fill=grid_color)
            self.result_canvas.create_line(0, p, PREVIEW_SIZE, p, fill=grid_color)

        self._draw_led_simulation()

    def _draw_led_simulation(self):
        if self.result_image is None:
            return

        self.led_canvas.delete("all")
        self.led_canvas.create_rectangle(0, 0, PREVIEW_SIZE, PREVIEW_SIZE, fill="#101010", outline="")

        cell = PREVIEW_SIZE / TARGET_SIZE
        outer_radius = cell * 0.40
        inner_radius = cell * 0.26

        for y in range(TARGET_SIZE):
            for x in range(TARGET_SIZE):
                r, g, b = self.result_image.getpixel((x, y))
                cx = x * cell + cell / 2
                cy = y * cell + cell / 2

                glow_color = "#" + f"{max(r // 2, 20):02x}{max(g // 2, 20):02x}{max(b // 2, 20):02x}"
                led_color = "#" + f"{r:02x}{g:02x}{b:02x}"

                self.led_canvas.create_oval(
                    cx - outer_radius,
                    cy - outer_radius,
                    cx + outer_radius,
                    cy + outer_radius,
                    fill=glow_color,
                    outline="#1a1a1a",
                )
                self.led_canvas.create_oval(
                    cx - inner_radius,
                    cy - inner_radius,
                    cx + inner_radius,
                    cy + inner_radius,
                    fill=led_color,
                    outline="",
                )

    def choose_primary_color(self):
        rgb, hex_color = colorchooser.askcolor(
            color=self.primary_color_var.get(), title="Select primary paint color"
        )
        if rgb is None or not hex_color:
            return

        self.primary_color_var.set(hex_color.upper())

    def choose_secondary_color(self):
        rgb, hex_color = colorchooser.askcolor(
            color=self.secondary_color_var.get(), title="Select secondary paint color"
        )
        if rgb is None or not hex_color:
            return

        self.secondary_color_var.set(hex_color.upper())

    def select_glyph_font(self):
        file_path = filedialog.askopenfilename(
            title="Select TrueType/OpenType font",
            filetypes=[
                ("Font files", "*.ttf;*.ttc;*.otf"),
                ("All files", "*.*"),
            ],
        )
        if not file_path:
            return

        self.glyph_font_path_var.set(file_path)

    @staticmethod
    def _parse_hex_color(hex_text):
        value = hex_text.strip().lstrip("#")
        if len(value) != 6:
            return None

        try:
            r = int(value[0:2], 16)
            g = int(value[2:4], 16)
            b = int(value[4:6], 16)
        except ValueError:
            return None

        return (r, g, b)

    @staticmethod
    def _preview_coord_to_pixel(event_x, event_y):
        x = int(event_x * TARGET_SIZE / PREVIEW_SIZE)
        y = int(event_y * TARGET_SIZE / PREVIEW_SIZE)

        x = min(max(x, 0), TARGET_SIZE - 1)
        y = min(max(y, 0), TARGET_SIZE - 1)
        return x, y

    def on_result_paint_primary(self, event):
        self._paint_result_pixel(event, self.primary_color_var.get())

    def on_result_paint_secondary(self, event):
        self._paint_result_pixel(event, self.secondary_color_var.get())

    def _paint_result_pixel(self, event, color_text):
        if self.result_image is None or not self.edit_enable_var.get():
            return

        pixel = self._preview_coord_to_pixel(event.x, event.y)
        if pixel == self.last_painted_pixel and event.type != tk.EventType.ButtonPress:
            return

        paint_color = self._parse_hex_color(color_text)
        if paint_color is None:
            return

        self.result_image.putpixel(pixel, paint_color)
        self.last_painted_pixel = pixel
        self._refresh_result_views()
        self.update_led_array_text()

    def on_result_paint_release(self, _event):
        self.last_painted_pixel = (-1, -1)

    @staticmethod
    def _sample_to_16(image):
        src = image.convert("RGB")
        src_w, src_h = src.size
        dst = Image.new("RGB", (TARGET_SIZE, TARGET_SIZE))

        for y in range(TARGET_SIZE):
            src_y = int(y * src_h / TARGET_SIZE)
            if src_y >= src_h:
                src_y = src_h - 1
            for x in range(TARGET_SIZE):
                src_x = int(x * src_w / TARGET_SIZE)
                if src_x >= src_w:
                    src_x = src_w - 1
                dst.putpixel((x, y), src.getpixel((src_x, src_y)))

        return dst

    @staticmethod
    def _feature_to_16(image):
        src = image.convert("RGB")

        # Keep composition by fitting without geometric distortion first.
        fitted = ImageOps.fit(
            src,
            (TARGET_SIZE * 4, TARGET_SIZE * 4),
            method=Image.Resampling.LANCZOS,
            centering=(0.5, 0.5),
        )

        # Enhance local structure so details survive when shrinking to 16x16.
        enhanced = ImageOps.autocontrast(fitted, cutoff=1)
        enhanced = enhanced.filter(ImageFilter.UnsharpMask(radius=1.0, percent=180, threshold=2))
        enhanced = enhanced.filter(ImageFilter.EDGE_ENHANCE_MORE)

        return enhanced.resize((TARGET_SIZE, TARGET_SIZE), Image.Resampling.BOX)

    def _apply_binarization(self, image):
        dark = self._parse_hex_color(self.binary_dark_color_var.get())
        light = self._parse_hex_color(self.binary_light_color_var.get())
        if dark is None:
            dark = (0, 0, 0)
        if light is None:
            light = (255, 255, 255)

        threshold = int(self.binary_threshold_var.get())
        threshold = min(max(threshold, 0), 255)

        src = image.convert("RGB")
        dst = Image.new("RGB", (TARGET_SIZE, TARGET_SIZE))
        for y in range(TARGET_SIZE):
            for x in range(TARGET_SIZE):
                r, g, b = src.getpixel((x, y))
                luma = int((299 * r + 587 * g + 114 * b) / 1000)
                if luma >= threshold:
                    dst.putpixel((x, y), light)
                else:
                    dst.putpixel((x, y), dark)

        return dst

    def _apply_edge_gradient(self, image):
        src = image.convert("RGB")
        dst = src.copy()
        strength = int(self.edge_gradient_strength_var.get())
        strength = min(max(strength, 0), 255)

        for y in range(TARGET_SIZE):
            for x in range(TARGET_SIZE):
                center = src.getpixel((x, y))
                total_w = 0
                acc_r = 0
                acc_g = 0
                acc_b = 0

                for dy in (-1, 0, 1):
                    ny = y + dy
                    if ny < 0 or ny >= TARGET_SIZE:
                        continue
                    for dx in (-1, 0, 1):
                        nx = x + dx
                        if nx < 0 or nx >= TARGET_SIZE:
                            continue
                        if dx == 0 and dy == 0:
                            continue

                        nr, ng, nb = src.getpixel((nx, ny))
                        diff = abs(center[0] - nr) + abs(center[1] - ng) + abs(center[2] - nb)
                        if diff == 0:
                            continue

                        weight = min(255, (diff * strength) // 255)
                        acc_r += nr * weight
                        acc_g += ng * weight
                        acc_b += nb * weight
                        total_w += weight

                if total_w != 0:
                    blend = min(160, strength)
                    avg_r = acc_r // total_w
                    avg_g = acc_g // total_w
                    avg_b = acc_b // total_w
                    out_r = (center[0] * (255 - blend) + avg_r * blend) // 255
                    out_g = (center[1] * (255 - blend) + avg_g * blend) // 255
                    out_b = (center[2] * (255 - blend) + avg_b * blend) // 255
                    dst.putpixel((x, y), (out_r, out_g, out_b))

        return dst

    @staticmethod
    def _rgb_to_rgb332(r, g, b):
        return ((r & 0xE0) | ((g >> 3) & 0x1C) | (b >> 6))

    @staticmethod
    def _sanitize_c_identifier(name):
        text = (name or "custom0").strip()
        if not text:
            text = "custom0"

        chars = []
        for idx, ch in enumerate(text):
            if ("a" <= ch <= "z") or ("A" <= ch <= "Z") or ch == "_":
                chars.append(ch)
            elif ("0" <= ch <= "9") and idx > 0:
                chars.append(ch)
            else:
                chars.append("_")

        if not chars or ("0" <= chars[0] <= "9"):
            chars.insert(0, "p")

        return "".join(chars)

    @staticmethod
    def _pack_bits_to_row(bits):
        row = 0
        for idx, bit in enumerate(bits):
            if bit != 0:
                row |= (1 << (15 - idx))

        return row

    @staticmethod
    def _glyph_rows_to_rgb332_frame(rows):
        frame = []
        for row in rows:
            for col in range(TARGET_SIZE):
                if (row & (1 << (15 - col))) != 0:
                    frame.append(0xFF)
                else:
                    frame.append(0x00)

        return frame

    def _load_glyph_font(self):
        font_path = self.glyph_font_path_var.get().strip()
        font_size = int(self.glyph_font_size_var.get())
        if font_size < 8:
            font_size = 8

        if not font_path:
            raise ValueError("Please select a font file for Chinese glyph extraction.")

        try:
            return ImageFont.truetype(font_path, font_size)
        except Exception as exc:
            raise ValueError(f"Failed to load font: {exc}")

    def _render_char_rows_16x16(self, char, font, threshold):
        # Render on a larger canvas first, then crop and scale to keep 16x16 details stable.
        canvas_size = 128
        canvas = Image.new("L", (canvas_size, canvas_size), color=0)
        draw = ImageDraw.Draw(canvas)

        draw.text((canvas_size // 2, canvas_size // 2), char, fill=255, font=font, anchor="mm")

        bbox = canvas.getbbox()
        if bbox is None:
            cropped = Image.new("L", (1, 1), color=0)
        else:
            cropped = canvas.crop(bbox)

        target_inner = TARGET_SIZE - 2
        src_w, src_h = cropped.size
        if src_w <= 0:
            src_w = 1
        if src_h <= 0:
            src_h = 1

        scale = min(target_inner / src_w, target_inner / src_h)
        if scale <= 0:
            scale = 1.0

        out_w = max(1, int(round(src_w * scale)))
        out_h = max(1, int(round(src_h * scale)))
        resized = cropped.resize((out_w, out_h), Image.Resampling.LANCZOS)

        frame = Image.new("L", (TARGET_SIZE, TARGET_SIZE), color=0)
        paste_x = (TARGET_SIZE - out_w) // 2
        paste_y = (TARGET_SIZE - out_h) // 2
        frame.paste(resized, (paste_x, paste_y))

        rows = []
        for row in range(TARGET_SIZE):
            bits = []
            for col in range(TARGET_SIZE):
                pixel = frame.getpixel((col, row))
                bits.append(1 if pixel >= threshold else 0)
            rows.append(self._pack_bits_to_row(bits))

        return rows

    def extract_glyph_arrays(self):
        text = self.glyph_text_var.get().strip()
        if not text:
            messagebox.showwarning("No text", "Please input Chinese text first.")
            return

        safe_name = self._sanitize_c_identifier(self.glyph_name_var.get())
        threshold = int(self.glyph_threshold_var.get())
        threshold = min(max(threshold, 0), 255)

        try:
            font = self._load_glyph_font()
        except ValueError as exc:
            messagebox.showerror("Font error", str(exc))
            return

        glyph_rows = []
        for ch in text:
            glyph_rows.append((ch, self._render_char_rows_16x16(ch, font, threshold)))

        lines = []
        lines.append("/* 16x16 glyph extraction output */")
        lines.append(f"/* source_text: {text} */")
        lines.append(f"#define TEST_SCROLL_GLYPH_COUNT          {len(glyph_rows)}U")
        lines.append("#define TEST_SCROLL_GLYPH_WIDTH          16U")
        lines.append("#define TEST_SCROLL_GLYPH_SPACING        1U")
        lines.append("")
        lines.append("static const uint16_t code g_testScrollGlyphRows[TEST_SCROLL_GLYPH_COUNT][TEST_IMAGE_ROWS] =")
        lines.append("{")
        for idx, (ch, rows) in enumerate(glyph_rows):
            lines.append(f"    /* {idx}: {ch} */")
            lines.append("    {")
            for row_idx, row in enumerate(rows):
                suffix = "," if row_idx < (TARGET_SIZE - 1) else ""
                lines.append(f"        0x{row:04X}{suffix}")
            block_suffix = "," if idx < (len(glyph_rows) - 1) else ""
            lines.append(f"    }}{block_suffix}")
        lines.append("};")
        lines.append("")
        lines.append("/* Optional RGB332 frame arrays, fg=0xFF bg=0x00 */")
        for idx, (ch, rows) in enumerate(glyph_rows):
            frame = self._glyph_rows_to_rgb332_frame(rows)
            lines.append(
                f"static const uint8_t code g_testPattern_{safe_name}_{idx}[TEST_IMAGE_PIXELS_PER_FRAME] ="
            )
            lines.append("{")
            for row in range(TARGET_SIZE):
                vals = frame[row * TARGET_SIZE : (row + 1) * TARGET_SIZE]
                suffix = "," if row < (TARGET_SIZE - 1) else ""
                lines.append("    " + ",".join(f"0x{v:02X}" for v in vals) + suffix)
            lines.append("};")
            lines.append(f"/* pattern index {idx}, char: {ch} */")
            lines.append("")

        content = "\n".join(lines)
        self.output_text.delete("1.0", tk.END)
        self.output_text.insert(tk.END, content)

    def update_led_array_text(self):
        if self.result_image is None:
            return

        order = self.color_order_var.get().strip().upper()
        if order not in ("RGB", "GRB", "BGR"):
            order = "RGB"

        var_name = self.array_name_var.get().strip()
        if not var_name:
            var_name = "g_ledImage16x16"

        pattern_name = self._sanitize_c_identifier(self.pattern_name_var.get())

        lines = []
        lines.append(f"const unsigned char {var_name}[16][16][3] = {{")

        for y in range(TARGET_SIZE):
            row_items = []
            for x in range(TARGET_SIZE):
                r, g, b = self.result_image.getpixel((x, y))
                if order == "RGB":
                    vals = (r, g, b)
                elif order == "GRB":
                    vals = (g, r, b)
                else:  # BGR
                    vals = (b, g, r)

                row_items.append("{" + f"{vals[0]:3d}, {vals[1]:3d}, {vals[2]:3d}" + "}")

            row_text = "    { " + ", ".join(row_items) + " }"
            if y < TARGET_SIZE - 1:
                row_text += ","
            lines.append(row_text)

        lines.append("};")

        lines.append("")
        lines.append("/* Optional 24-bit packed format (0xRRGGBB): */")
        lines.append(f"const unsigned int {var_name}_PACKED[16][16] = {{")
        for y in range(TARGET_SIZE):
            packed_row = []
            for x in range(TARGET_SIZE):
                r, g, b = self.result_image.getpixel((x, y))
                packed_row.append(f"0x{r:02X}{g:02X}{b:02X}")

            packed_text = "    { " + ", ".join(packed_row) + " }"
            if y < TARGET_SIZE - 1:
                packed_text += ","
            lines.append(packed_text)

        lines.append("};")

        lines.append("")
        lines.append("/* RGB332 flat frame format for test_image.h: */")
        lines.append(f"static const uint8_t code g_testPattern_{pattern_name}[TEST_IMAGE_PIXELS_PER_FRAME] =")
        lines.append("{")
        for y in range(TARGET_SIZE):
            row_vals = []
            for x in range(TARGET_SIZE):
                r, g, b = self.result_image.getpixel((x, y))
                row_vals.append(f"0x{self._rgb_to_rgb332(r, g, b):02X}")
            suffix = "," if y < TARGET_SIZE - 1 else ""
            lines.append("    " + ",".join(row_vals) + suffix)
        lines.append("};")

        lines.append("")
        lines.append("/* Direct insert snippet (name + frame table entry): */")
        lines.append(f"static const char code g_testPatternName_{pattern_name}[] = \"{pattern_name}\";")
        lines.append(f"/* Add to g_testImageNames: g_testPatternName_{pattern_name} */")
        lines.append(f"/* Add to g_testImageFrames: g_testPattern_{pattern_name} */")

        content = "\n".join(lines)
        self.output_text.delete("1.0", tk.END)
        self.output_text.insert(tk.END, content)

    def copy_array(self):
        text = self.output_text.get("1.0", tk.END).strip()
        if not text:
            messagebox.showwarning("No output", "No generated array to copy.")
            return

        self.root.clipboard_clear()
        self.root.clipboard_append(text)
        self.root.update()
        messagebox.showinfo("Copied", "LED array text copied to clipboard.")

    def _canvas_to_image_coord(self, event_x, event_y):
        if self.display_image is None:
            return 0, 0

        left = (PREVIEW_SIZE - self.display_image.width) / 2
        top = (PREVIEW_SIZE - self.display_image.height) / 2

        # Clamp to displayed image area.
        px = min(max(event_x, left), left + self.display_image.width - 1)
        py = min(max(event_y, top), top + self.display_image.height - 1)

        img_x = int((px - left) * self.scale_x)
        img_y = int((py - top) * self.scale_y)

        img_x = min(max(img_x, 0), self.original_image.width - 1)
        img_y = min(max(img_y, 0), self.original_image.height - 1)
        return img_x, img_y

    def on_mouse_down(self, event):
        if self.original_image is None:
            return

        self.dragging = True
        click_x, click_y = self._canvas_to_image_coord(event.x, event.y)

        crop_x = self.crop_x_var.get()
        crop_y = self.crop_y_var.get()
        crop_w = self.crop_w_var.get()
        crop_h = self.crop_h_var.get()

        inside_rect = (
            click_x >= crop_x
            and click_x < (crop_x + crop_w)
            and click_y >= crop_y
            and click_y < (crop_y + crop_h)
        )

        if inside_rect:
            self.drag_mode = "move"
            self.drag_offset_x = click_x - crop_x
            self.drag_offset_y = click_y - crop_y
        else:
            self.drag_mode = "new"
            self.drag_start_x = click_x
            self.drag_start_y = click_y

    def on_mouse_drag(self, event):
        if self.original_image is None or not self.dragging:
            return

        cur_x, cur_y = self._canvas_to_image_coord(event.x, event.y)

        if self.drag_mode == "move":
            self.crop_x_var.set(cur_x - self.drag_offset_x)
            self.crop_y_var.set(cur_y - self.drag_offset_y)
        else:
            x0 = min(self.drag_start_x, cur_x)
            y0 = min(self.drag_start_y, cur_y)
            x1 = max(self.drag_start_x, cur_x)
            y1 = max(self.drag_start_y, cur_y)

            w = max(1, x1 - x0 + 1)
            h = max(1, y1 - y0 + 1)

            self.crop_x_var.set(x0)
            self.crop_y_var.set(y0)
            self.crop_w_var.set(w)
            self.crop_h_var.set(h)

        self.update_all_previews()

    def on_mouse_up(self, _event):
        self.dragging = False
        self.last_painted_pixel = (-1, -1)


def main():
    root = tk.Tk()
    style = ttk.Style(root)
    try:
        style.theme_use("clam")
    except Exception:
        pass

    app = LedImageConverterApp(root)
    app.update_led_array_text()
    root.mainloop()


if __name__ == "__main__":
    main()

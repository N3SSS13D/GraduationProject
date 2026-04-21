#include "gp_debug_display.h"

#include "lvgl_theme.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <esp_timer.h>

namespace {
constexpr int kDebugDotDefaultSize = 24;
constexpr int kDebugDotMinSize = 10;
constexpr int kDebugDotMaxSize = 44;
constexpr uint32_t kDebugAnimationTickMs = 33;
constexpr uint32_t kAiLinkOnlineColor = 0x22C55E;
constexpr uint32_t kAiLinkOfflineColor = 0xEF4444;
constexpr uint32_t kAiLinkBusyColor = 0xF59E0B;
constexpr uint32_t kAiLinkNeutralColor = 0x64748B;
constexpr int kAiLinkPanelHeight = 78;
constexpr int kDebugMenuMargin = 8;
constexpr int kDebugMenuHeaderHeight = 30;
constexpr int kDebugMenuContentPadding = 8;
constexpr int kDebugMenuContentGap = 8;
constexpr int kDebugPreviewHeight = 48;
constexpr int kTouchPanelMinHeight = 100;
constexpr int kMenuEntryWidth = 42;
constexpr int kMenuEntryHeight = 28;
constexpr int kMenuEntryMargin = 10;
constexpr int kDebugMenuButtonWidth = 24;
constexpr int kDebugMenuButtonHeight = 20;
constexpr int kDebugPreviewTopInset = 8;
constexpr int kDebugSnapshotButtonSize = 24;
constexpr int kTouchControlRowHeight = 40;
constexpr int kDebugPageCount = 2;

struct DebugMenuLayout {
    int menu_width = 0;
    int menu_height = 0;
    int content_width = 0;
    int content_height = 0;
    int content_top = 0;
    int left_width = 0;
    int right_width = 0;
    int left_x = 0;
    int right_x = 0;
};

void ApplyDebugTextStyle(lv_obj_t* obj, uint32_t color,
    lv_text_align_t align = LV_TEXT_ALIGN_LEFT, uint16_t zoom = 220) {
    lv_obj_set_style_text_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_text_line_space(obj, 0, 0);
    lv_obj_set_style_text_align(obj, align, 0);

    /* Avoid transform-layer text rendering here. LVGL routes scaled labels through
     * an intermediate image layer, which is the crash path seen when opening DBG. */
    (void)zoom;
}

void ClearDebugImageStyle(lv_obj_t* obj) {
    if (obj == nullptr) {
        return;
    }

    /* Keep debug widgets away from background-image and mask style paths.
     * The current crash reaches lv_draw_rect() while probing bg_image_src. */
    lv_obj_set_style_bg_image_src(obj, nullptr, 0);
    lv_obj_set_style_bitmap_mask_src(obj, nullptr, 0);
}

DebugMenuLayout ComputeDebugMenuLayout(int width, int height) {
    DebugMenuLayout layout;

    layout.menu_width = std::max(width - kDebugMenuMargin * 2, 140);
    layout.menu_height = std::max(height - kDebugMenuMargin * 2, 180);
    layout.content_width = layout.menu_width - kDebugMenuContentPadding * 2;
    layout.content_height = layout.menu_height - kDebugMenuHeaderHeight - kDebugMenuContentPadding - 8;
    layout.content_top = kDebugMenuHeaderHeight + 8;
    layout.right_width = std::clamp((layout.content_width * 2) / 5, 124, 140);
    layout.left_width = std::max(layout.content_width - layout.right_width - kDebugMenuContentGap, 120);
    layout.left_x = kDebugMenuContentPadding;
    layout.right_x = layout.left_x + layout.left_width + kDebugMenuContentGap;

    return layout;
}

int ClampDotSizeToPreview(lv_obj_t* panel, int dot_size) {
    int width_limit;
    int height_limit;

    if (panel == nullptr) {
        return std::clamp(dot_size, kDebugDotMinSize, kDebugDotMaxSize);
    }

    width_limit = lv_obj_get_content_width(panel) - kDebugMenuContentPadding;
    height_limit = lv_obj_get_content_height(panel) - kDebugPreviewTopInset - kDebugMenuContentPadding;
    width_limit = std::max(width_limit, kDebugDotMinSize);
    height_limit = std::max(height_limit, kDebugDotMinSize);

    /* Keep the preview fully inside the submenu's reduced content area. */
    return std::clamp(dot_size, kDebugDotMinSize, std::min({kDebugDotMaxSize, width_limit, height_limit}));
}

const char* GetPresetName(GpColorDebugPreset preset) {
    switch (preset) {
    case GpColorDebugPreset::kDiamond:
        return "diamond";
    case GpColorDebugPreset::kCross:
        return "cross";
    case GpColorDebugPreset::kJluEmblem:
        return "JLU_emblem";
    case GpColorDebugPreset::kScrollSubtitle:
        return "scroll";
    case GpColorDebugPreset::kSolid:
    default:
        return "solid";
    }
}

const char* GetAnimationName(GpColorDebugAnimation animation) {
    switch (animation) {
    case GpColorDebugAnimation::kGradient:
        return "gradient";
    case GpColorDebugAnimation::kPulse:
        return "pulse";
    case GpColorDebugAnimation::kSolid:
    default:
        return "solid";
    }
}

GpColorDebugPreset CyclePreset(GpColorDebugPreset preset, int delta) {
    static constexpr std::array<GpColorDebugPreset, 4> kPresets = {
        GpColorDebugPreset::kSolid,
        GpColorDebugPreset::kDiamond,
        GpColorDebugPreset::kCross,
        GpColorDebugPreset::kJluEmblem,
    };

    size_t index = 0;
    for (size_t i = 0; i < kPresets.size(); ++i) {
        if (kPresets[i] == preset) {
            index = i;
            break;
        }
    }

    const int size = static_cast<int>(kPresets.size());
    const int next = (static_cast<int>(index) + delta + size) % size;
    return kPresets[static_cast<size_t>(next)];
}

GpColorDebugAnimation CycleAnimation(GpColorDebugAnimation animation, int delta) {
    static constexpr std::array<GpColorDebugAnimation, 3> kAnimations = {
        GpColorDebugAnimation::kSolid,
        GpColorDebugAnimation::kGradient,
        GpColorDebugAnimation::kPulse,
    };

    size_t index = 0;
    for (size_t i = 0; i < kAnimations.size(); ++i) {
        if (kAnimations[i] == animation) {
            index = i;
            break;
        }
    }

    const int size = static_cast<int>(kAnimations.size());
    const int next = (static_cast<int>(index) + delta + size) % size;
    return kAnimations[static_cast<size_t>(next)];
}

uint8_t MixChannel(uint8_t first, uint8_t second, float ratio) {
    const float clamped = std::clamp(ratio, 0.0f, 1.0f);
    return static_cast<uint8_t>(first + static_cast<float>(second - first) * clamped);
}

uint32_t MixRgb888(uint32_t first, uint32_t second, float ratio) {
    const uint8_t first_r = static_cast<uint8_t>((first >> 16) & 0xFFU);
    const uint8_t first_g = static_cast<uint8_t>((first >> 8) & 0xFFU);
    const uint8_t first_b = static_cast<uint8_t>(first & 0xFFU);
    const uint8_t second_r = static_cast<uint8_t>((second >> 16) & 0xFFU);
    const uint8_t second_g = static_cast<uint8_t>((second >> 8) & 0xFFU);
    const uint8_t second_b = static_cast<uint8_t>(second & 0xFFU);

    return (static_cast<uint32_t>(MixChannel(first_r, second_r, ratio)) << 16) |
        (static_cast<uint32_t>(MixChannel(first_g, second_g, ratio)) << 8) |
        static_cast<uint32_t>(MixChannel(first_b, second_b, ratio));
}

float TriangleWave(uint64_t elapsed_us, uint32_t period_ms) {
    const uint64_t period_us = static_cast<uint64_t>(std::max<uint32_t>(period_ms, 300U)) * 1000ULL;
    const float progress = static_cast<float>(elapsed_us % period_us) / static_cast<float>(period_us);
    return progress < 0.5f ? progress * 2.0f : (1.0f - progress) * 2.0f;
}

bool ContainsAsciiNoCase(const std::string& text, const char* keyword) {
    const size_t keyword_length = std::strlen(keyword);

    if (keyword_length == 0U || text.size() < keyword_length) {
        return false;
    }

    for (size_t offset = 0; offset + keyword_length <= text.size(); ++offset) {
        bool match = true;

        for (size_t index = 0; index < keyword_length; ++index) {
            const char left = static_cast<char>(std::tolower(static_cast<unsigned char>(text[offset + index])));
            const char right = static_cast<char>(std::tolower(static_cast<unsigned char>(keyword[index])));

            if (left != right) {
                match = false;
                break;
            }
        }

        if (match) {
            return true;
        }
    }

    return false;
}
}

GpDebugLcdDisplay::GpDebugLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
    int width, int height, int offset_x, int offset_y,
    bool mirror_x, bool mirror_y, bool swap_xy)
    : SpiLcdDisplay(panel_io, panel, width, height, offset_x, offset_y, mirror_x, mirror_y, swap_xy) {
    current_state_.dot_size_px = kDebugDotDefaultSize;
    current_state_.label = "touch";
    current_state_.source = "touch";
    ai_link_status_text_ = "Bluetooth init";
    menu_button_contexts_[0] = {this, MenuAction::kOpenDebug};
    menu_button_contexts_[1] = {this, MenuAction::kCloseDebug};
    touch_button_contexts_[0] = {this, TouchAdjust::kPrevPreset};
    touch_button_contexts_[1] = {this, TouchAdjust::kNextPreset};
    touch_button_contexts_[2] = {this, TouchAdjust::kPrevEffect};
    touch_button_contexts_[3] = {this, TouchAdjust::kNextEffect};
    touch_button_contexts_[4] = {this, TouchAdjust::kCaptureSnapshot};
    CreateMenuEntryButton();
    CreateDebugMenuOverlay();
    CreateTouchOverlay();
    CreateDebugOverlay();
    CreateLinkStatusOverlay();
    SetDebugMenuVisible(false);
}

GpDebugLcdDisplay::~GpDebugLcdDisplay() {
    if (debug_animation_timer_ != nullptr) {
        lv_timer_delete(debug_animation_timer_);
        debug_animation_timer_ = nullptr;
    }
}

void GpDebugLcdDisplay::CreateMenuEntryButton() {
    DisplayLockGuard lock(this);
    if (display_ == nullptr || menu_entry_button_ != nullptr) {
        return;
    }

    auto* screen = lv_display_get_screen_active(display_);

    menu_entry_button_ = lv_button_create(screen);
    ClearDebugImageStyle(menu_entry_button_);
    lv_obj_set_size(menu_entry_button_, kMenuEntryWidth, kMenuEntryHeight);
    lv_obj_set_style_radius(menu_entry_button_, 12, 0);
    lv_obj_set_style_bg_color(menu_entry_button_, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_bg_opa(menu_entry_button_, LV_OPA_70, 0);
    lv_obj_set_style_border_width(menu_entry_button_, 1, 0);
    lv_obj_set_style_border_color(menu_entry_button_, lv_color_hex(0x334155), 0);
    lv_obj_align(menu_entry_button_, LV_ALIGN_BOTTOM_RIGHT, -kMenuEntryMargin, -kMenuEntryMargin);
    lv_obj_add_event_cb(menu_entry_button_, &GpDebugLcdDisplay::OnMenuButtonEvent, LV_EVENT_CLICKED, &menu_button_contexts_[0]);

    lv_obj_t* label = lv_label_create(menu_entry_button_);
    ApplyDebugTextStyle(label, 0xE2E8F0, LV_TEXT_ALIGN_CENTER);
    lv_label_set_text(label, "DBG");
    lv_obj_center(label);
}

void GpDebugLcdDisplay::CreateDebugMenuOverlay() {
    DisplayLockGuard lock(this);
    if (display_ == nullptr || debug_menu_panel_ != nullptr) {
        return;
    }

    auto* screen = lv_display_get_screen_active(display_);
    const DebugMenuLayout layout = ComputeDebugMenuLayout(width_, height_);

    debug_menu_panel_ = lv_obj_create(screen);
    lv_obj_remove_style_all(debug_menu_panel_);
    ClearDebugImageStyle(debug_menu_panel_);
    lv_obj_set_size(debug_menu_panel_, layout.menu_width, layout.menu_height);
    lv_obj_set_style_radius(debug_menu_panel_, 18, 0);
    lv_obj_set_style_bg_opa(debug_menu_panel_, LV_OPA_90, 0);
    lv_obj_set_style_bg_color(debug_menu_panel_, lv_color_hex(0x020617), 0);
    lv_obj_set_style_border_width(debug_menu_panel_, 1, 0);
    lv_obj_set_style_border_color(debug_menu_panel_, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_pad_all(debug_menu_panel_, 0, 0);
    lv_obj_align(debug_menu_panel_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(debug_menu_panel_, LV_OBJ_FLAG_SCROLLABLE);

    debug_header_panel_ = lv_obj_create(debug_menu_panel_);
    lv_obj_remove_style_all(debug_header_panel_);
    ClearDebugImageStyle(debug_header_panel_);
    lv_obj_set_size(debug_header_panel_, layout.content_width, kDebugMenuHeaderHeight);
    lv_obj_align(debug_header_panel_, LV_ALIGN_TOP_MID, 0, 4);
    lv_obj_set_style_pad_left(debug_header_panel_, 0, 0);
    lv_obj_set_style_pad_right(debug_header_panel_, 0, 0);
    lv_obj_set_style_pad_top(debug_header_panel_, 0, 0);
    lv_obj_set_style_pad_bottom(debug_header_panel_, 0, 0);
    lv_obj_clear_flag(debug_header_panel_, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* title = lv_label_create(debug_header_panel_);
    ApplyDebugTextStyle(title, 0xF8FAFC, LV_TEXT_ALIGN_CENTER);
    lv_label_set_text(title, "Debug Menu");
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* back_button = lv_button_create(debug_header_panel_);
    ClearDebugImageStyle(back_button);
    lv_obj_set_size(back_button, 44, 24);
    lv_obj_set_style_radius(back_button, 12, 0);
    lv_obj_set_style_bg_color(back_button, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_bg_opa(back_button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(back_button, 1, 0);
    lv_obj_set_style_border_color(back_button, lv_color_hex(0x475569), 0);
    lv_obj_align(back_button, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_event_cb(back_button, &GpDebugLcdDisplay::OnMenuButtonEvent, LV_EVENT_CLICKED, &menu_button_contexts_[1]);

    lv_obj_t* back_label = lv_label_create(back_button);
    ApplyDebugTextStyle(back_label, 0xE2E8F0, LV_TEXT_ALIGN_CENTER);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);

    snapshot_button_ = lv_button_create(debug_header_panel_);
    ClearDebugImageStyle(snapshot_button_);
    lv_obj_set_size(snapshot_button_, kDebugSnapshotButtonSize, kDebugSnapshotButtonSize);
    lv_obj_set_style_radius(snapshot_button_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(snapshot_button_, lv_color_hex(0x1D4ED8), 0);
    lv_obj_set_style_bg_opa(snapshot_button_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(snapshot_button_, 1, 0);
    lv_obj_set_style_border_color(snapshot_button_, lv_color_hex(0x60A5FA), 0);
    lv_obj_align(snapshot_button_, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event_cb(snapshot_button_, &GpDebugLcdDisplay::OnTouchButtonEvent, LV_EVENT_CLICKED, &touch_button_contexts_[4]);

    lv_obj_t* snapshot_button_label = lv_label_create(snapshot_button_);
    ApplyDebugTextStyle(snapshot_button_label, 0xEFF6FF, LV_TEXT_ALIGN_CENTER, 175);
    lv_label_set_text(snapshot_button_label, "S");
    lv_obj_center(snapshot_button_label);

    debug_menu_content_ = nullptr;
    debug_page_strip_ = nullptr;
    debug_control_page_ = nullptr;
    debug_info_page_ = nullptr;
    debug_side_panel_ = nullptr;
    debug_page_indicator_label_ = nullptr;

    lv_obj_add_flag(debug_menu_panel_, LV_OBJ_FLAG_HIDDEN);
}

void GpDebugLcdDisplay::CreateDebugOverlay() {
    DisplayLockGuard lock(this);
    if (display_ == nullptr || debug_dot_ != nullptr) {
        return;
    }

    if (debug_menu_panel_ == nullptr) {
        return;
    }

    const DebugMenuLayout layout = ComputeDebugMenuLayout(width_, height_);

    debug_preview_panel_ = lv_obj_create(debug_menu_panel_);
    lv_obj_remove_style_all(debug_preview_panel_);
    ClearDebugImageStyle(debug_preview_panel_);
    lv_obj_set_size(debug_preview_panel_, layout.right_width, kDebugPreviewHeight);
    lv_obj_set_height(debug_preview_panel_, kDebugPreviewHeight);
    lv_obj_set_style_radius(debug_preview_panel_, 14, 0);
    lv_obj_set_style_bg_color(debug_preview_panel_, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_bg_opa(debug_preview_panel_, LV_OPA_70, 0);
    lv_obj_set_style_border_width(debug_preview_panel_, 1, 0);
    lv_obj_set_style_border_color(debug_preview_panel_, lv_color_hex(0x1E293B), 0);
    lv_obj_clear_flag(debug_preview_panel_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(debug_preview_panel_, layout.right_x, layout.content_top);

    debug_dot_ = lv_obj_create(debug_preview_panel_);
    lv_obj_remove_style_all(debug_dot_);
    ClearDebugImageStyle(debug_dot_);
    lv_obj_set_size(debug_dot_, current_state_.dot_size_px, current_state_.dot_size_px);
    lv_obj_set_style_radius(debug_dot_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(debug_dot_, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(debug_dot_, lv_color_hex(0x808080), 0);
    UpdateDotPlacement(current_state_.dot_size_px);
    RefreshAnimatedDot();
}

void GpDebugLcdDisplay::CreateLinkStatusOverlay() {
    DisplayLockGuard lock(this);
    if (display_ == nullptr || link_status_panel_ != nullptr) {
        return;
    }

    if (debug_menu_panel_ == nullptr) {
        return;
    }

    const DebugMenuLayout layout = ComputeDebugMenuLayout(width_, height_);

    link_status_panel_ = lv_obj_create(debug_menu_panel_);
    lv_obj_remove_style_all(link_status_panel_);
    ClearDebugImageStyle(link_status_panel_);
    lv_obj_set_size(link_status_panel_, layout.right_width, kAiLinkPanelHeight);
    lv_obj_set_style_radius(link_status_panel_, 14, 0);
    lv_obj_set_style_bg_opa(link_status_panel_, LV_OPA_80, 0);
    lv_obj_set_style_bg_color(link_status_panel_, lv_color_hex(0x101418), 0);
    lv_obj_set_style_pad_top(link_status_panel_, 10, 0);
    lv_obj_set_style_pad_bottom(link_status_panel_, 10, 0);
    lv_obj_set_style_pad_left(link_status_panel_, 10, 0);
    lv_obj_set_style_pad_right(link_status_panel_, 10, 0);
    lv_obj_set_style_pad_row(link_status_panel_, 6, 0);
    lv_obj_clear_flag(link_status_panel_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(link_status_panel_, layout.right_x, layout.content_top + kDebugPreviewHeight + kDebugMenuContentGap);

    lv_obj_t* status_title_row = lv_obj_create(link_status_panel_);
    lv_obj_remove_style_all(status_title_row);
    ClearDebugImageStyle(status_title_row);
    lv_obj_set_size(status_title_row, layout.right_width - 20, 16);
    lv_obj_clear_flag(status_title_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(status_title_row, 10, 8);

    lv_obj_t* status_title = lv_label_create(status_title_row);
    ApplyDebugTextStyle(status_title, 0x94A3B8, LV_TEXT_ALIGN_LEFT, 180);
    lv_label_set_text(status_title, "Bluetooth");
    lv_obj_align(status_title, LV_ALIGN_LEFT_MID, 0, 0);

    link_status_dot_ = lv_obj_create(status_title_row);
    lv_obj_remove_style_all(link_status_dot_);
    ClearDebugImageStyle(link_status_dot_);
    lv_obj_set_size(link_status_dot_, 12, 12);
    lv_obj_set_style_radius(link_status_dot_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(link_status_dot_, LV_OPA_COVER, 0);
    lv_obj_align(link_status_dot_, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_obj_t* status_row = lv_obj_create(link_status_panel_);
    lv_obj_remove_style_all(status_row);
    ClearDebugImageStyle(status_row);
    lv_obj_set_size(status_row, layout.right_width - 20, 50);
    lv_obj_clear_flag(status_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(status_row, 10, 28);

    link_status_label_ = lv_label_create(status_row);
    lv_obj_set_width(link_status_label_, layout.right_width - 20);
    lv_label_set_long_mode(link_status_label_, LV_LABEL_LONG_WRAP);
    ApplyDebugTextStyle(link_status_label_, 0xF8FAFC, LV_TEXT_ALIGN_LEFT, 155);

    RefreshAiLinkStatus();
}

void GpDebugLcdDisplay::CreateTouchOverlay() {
    DisplayLockGuard lock(this);
    if (display_ == nullptr || touch_panel_ != nullptr) {
        return;
    }

    if (debug_menu_panel_ == nullptr) {
        return;
    }

    const DebugMenuLayout layout = ComputeDebugMenuLayout(width_, height_);
    const int inner_width = layout.left_width - 20;
    const int row_width = inner_width;

    auto create_touch_button = [](lv_obj_t* parent, const char* text, TouchButtonContext* context, int x, int y) {
        lv_obj_t* button = lv_button_create(parent);
        ClearDebugImageStyle(button);
        lv_obj_set_size(button, kDebugMenuButtonWidth, kDebugMenuButtonHeight);
        lv_obj_set_pos(button, x, y);
        lv_obj_set_style_radius(button, 12, 0);
        lv_obj_set_style_bg_color(button, lv_color_hex(0x1E293B), 0);
        lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(button, 1, 0);
        lv_obj_set_style_border_color(button, lv_color_hex(0x475569), 0);
        lv_obj_add_event_cb(button, &GpDebugLcdDisplay::OnTouchButtonEvent, LV_EVENT_CLICKED, context);

        lv_obj_t* label = lv_label_create(button);
        ApplyDebugTextStyle(label, 0xF8FAFC, LV_TEXT_ALIGN_CENTER);
        lv_label_set_text(label, text);
        lv_obj_center(label);

        return button;
    };

    touch_panel_ = lv_obj_create(debug_menu_panel_);
    lv_obj_remove_style_all(touch_panel_);
    ClearDebugImageStyle(touch_panel_);
    lv_obj_set_size(touch_panel_, layout.left_width, layout.content_height);
    lv_obj_set_style_radius(touch_panel_, 14, 0);
    lv_obj_set_style_bg_opa(touch_panel_, LV_OPA_80, 0);
    lv_obj_set_style_bg_color(touch_panel_, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_pad_top(touch_panel_, 8, 0);
    lv_obj_set_style_pad_bottom(touch_panel_, 8, 0);
    lv_obj_set_style_pad_left(touch_panel_, 10, 0);
    lv_obj_set_style_pad_right(touch_panel_, 10, 0);
    lv_obj_clear_flag(touch_panel_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(touch_panel_, layout.left_x, layout.content_top);

    lv_obj_t* title_label = lv_label_create(touch_panel_);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_CLIP);
    ApplyDebugTextStyle(title_label, 0xCBD5E1, LV_TEXT_ALIGN_LEFT, 170);
    lv_label_set_text(title_label, "LEDmatrix");
    lv_obj_set_width(title_label, inner_width);
    lv_obj_set_pos(title_label, 0, 4);

    auto create_control_row = [&](const char* name, TouchButtonContext* prev_context,
                                  TouchButtonContext* next_context, lv_obj_t** value_label, int y, bool with_buttons) {
        lv_obj_t* row = lv_obj_create(touch_panel_);
        lv_obj_remove_style_all(row);
        ClearDebugImageStyle(row);
        lv_obj_set_size(row, row_width, kTouchControlRowHeight);
        lv_obj_set_pos(row, 0, y);
        lv_obj_set_height(row, kTouchControlRowHeight);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* label = lv_label_create(row);
        lv_obj_set_width(label, row_width);
        lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
        ApplyDebugTextStyle(label, 0x94A3B8, LV_TEXT_ALIGN_LEFT, 150);
        lv_label_set_text(label, name);
        lv_obj_set_pos(label, 0, 0);

        if (with_buttons) {
            create_touch_button(row, "<", prev_context, 0, 18);

            *value_label = lv_label_create(row);
            lv_obj_set_width(*value_label, row_width - (kDebugMenuButtonWidth * 2 + 12));
            lv_label_set_long_mode(*value_label, LV_LABEL_LONG_CLIP);
            ApplyDebugTextStyle(*value_label, 0xF8FAFC, LV_TEXT_ALIGN_CENTER, 150);
            lv_obj_set_pos(*value_label, kDebugMenuButtonWidth + 6, 20);

            create_touch_button(row, ">", next_context, row_width - kDebugMenuButtonWidth, 18);
        } else {
            *value_label = lv_label_create(row);
            lv_obj_set_width(*value_label, row_width);
            lv_label_set_long_mode(*value_label, LV_LABEL_LONG_CLIP);
            ApplyDebugTextStyle(*value_label, 0xF8FAFC, LV_TEXT_ALIGN_LEFT, 150);
            lv_obj_set_pos(*value_label, 0, 20);
        }
    };

    create_control_row("Pattern", &touch_button_contexts_[0], &touch_button_contexts_[1], &preset_value_label_, 28, true);
    create_control_row("Effect", &touch_button_contexts_[2], &touch_button_contexts_[3], &effect_value_label_, 72, true);
    create_control_row("Color", nullptr, nullptr, &color_value_label_, 116, false);

    debug_info_label_ = lv_label_create(touch_panel_);
    lv_obj_set_width(debug_info_label_, inner_width);
    lv_label_set_long_mode(debug_info_label_, LV_LABEL_LONG_WRAP);
    ApplyDebugTextStyle(debug_info_label_, 0xE2E8F0, LV_TEXT_ALIGN_LEFT, 145);
    lv_obj_set_pos(debug_info_label_, 0, 162);

    RefreshTouchState();
    RefreshInfoPage();
}

void GpDebugLcdDisplay::ApplyColorDebugState(const GpColorDebugState& state) {
    if (debug_dot_ == nullptr) {
        CreateDebugOverlay();
    }
    if (touch_panel_ == nullptr) {
        CreateTouchOverlay();
    }

    DisplayLockGuard lock(this);
    if (debug_dot_ == nullptr) {
        return;
    }

    current_state_ = state;
    current_state_.dot_size_px = static_cast<uint16_t>(std::clamp<int>(current_state_.dot_size_px, kDebugDotMinSize, kDebugDotMaxSize));
    if (current_state_.rgb888_text.empty()) {
        char rgb_text[16] = {0};
        std::snprintf(rgb_text, sizeof(rgb_text), "#%06X", static_cast<unsigned int>(current_state_.primary_rgb888 & 0xFFFFFFU));
        current_state_.rgb888_text = rgb_text;
    }
    if (current_state_.label.empty()) {
        current_state_.label = "custom";
    }
    if (current_state_.source.empty()) {
        current_state_.source = "custom";
    }

    animation_start_us_ = static_cast<uint64_t>(esp_timer_get_time());
    RefreshTouchState();
    RefreshInfoPage();
    if (debug_menu_visible_) {
        RefreshAnimatedDot();
    }
}

void GpDebugLcdDisplay::ApplyAiLinkStatus(bool online, const std::string& status_text) {
    if (link_status_panel_ == nullptr) {
        CreateLinkStatusOverlay();
    }

    DisplayLockGuard lock(this);
    if (link_status_panel_ == nullptr) {
        return;
    }

    ai_link_online_ = online;
    ai_link_status_text_ = status_text.empty() ? "Bluetooth waiting" : status_text;
    RefreshAiLinkStatus();
    RefreshInfoPage();
}

void GpDebugLcdDisplay::SetMatrixDebugStateCallback(MatrixDebugStateCallback callback) {
    matrix_debug_state_callback_ = std::move(callback);
}

void GpDebugLcdDisplay::SetDebugSnapshotCallback(DebugSnapshotCallback callback) {
    debug_snapshot_callback_ = std::move(callback);
}

void GpDebugLcdDisplay::ScheduleDebugMenuVisible(bool visible) {
    pending_menu_visible_ = visible;
    lv_async_call_cancel(&GpDebugLcdDisplay::OnAsyncMenuVisibility, this);
    lv_async_call(&GpDebugLcdDisplay::OnAsyncMenuVisibility, this);
}

void GpDebugLcdDisplay::SetDebugMenuVisible(bool visible) {
    if (debug_menu_panel_ == nullptr) {
        return;
    }

    debug_menu_visible_ = visible;
    if (visible) {
        lv_obj_remove_flag(debug_menu_panel_, LV_OBJ_FLAG_HIDDEN);
        if (menu_entry_button_ != nullptr) {
            lv_obj_add_flag(menu_entry_button_, LV_OBJ_FLAG_HIDDEN);
        }

        /* Avoid large state churn on the first visible frame. The menu content
         * is initialized during construction, so showing the panel can stay
         * close to a pure visibility toggle. */
    } else {
        lv_obj_add_flag(debug_menu_panel_, LV_OBJ_FLAG_HIDDEN);
        if (menu_entry_button_ != nullptr) {
            lv_obj_remove_flag(menu_entry_button_, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void GpDebugLcdDisplay::EnsureAnimationTimer() {
    /* Disabled while stabilizing DBG open/close. Timer-driven UI updates add
     * another re-entrant source during the first frame after the menu opens. */
}

void GpDebugLcdDisplay::OnAnimationTimer(lv_timer_t* timer) {
    auto* self = static_cast<GpDebugLcdDisplay*>(lv_timer_get_user_data(timer));
    if (self != nullptr) {
        self->RefreshAnimatedDot();
    }
}

void GpDebugLcdDisplay::OnAsyncMenuVisibility(void* user_data) {
    auto* self = static_cast<GpDebugLcdDisplay*>(user_data);

    if (self != nullptr) {
        self->SetDebugMenuVisible(self->pending_menu_visible_);
    }
}

void GpDebugLcdDisplay::RefreshAnimatedDot() {
    if ((debug_dot_ == nullptr) || !debug_menu_visible_) {
        return;
    }

    const uint64_t now_us = static_cast<uint64_t>(esp_timer_get_time());
    const float wave = TriangleWave(now_us - animation_start_us_, current_state_.animation_period_ms);

    uint32_t rgb = current_state_.primary_rgb888;
    int dot_size = current_state_.dot_size_px;

    if (current_state_.animation == GpColorDebugAnimation::kGradient && current_state_.has_secondary) {
        rgb = MixRgb888(current_state_.primary_rgb888, current_state_.secondary_rgb888, wave);
    } else if (current_state_.animation == GpColorDebugAnimation::kPulse) {
        if (current_state_.has_secondary) {
            rgb = MixRgb888(current_state_.primary_rgb888, current_state_.secondary_rgb888, wave);
        }
        dot_size = static_cast<int>(current_state_.dot_size_px * (0.72f + 0.40f * wave));
    }

    dot_size = ClampDotSizeToPreview(debug_preview_panel_, dot_size);
    lv_obj_set_size(debug_dot_, dot_size, dot_size);
    UpdateDotPlacement(dot_size);
    lv_obj_set_style_bg_color(debug_dot_, lv_color_hex(rgb), 0);
}

void GpDebugLcdDisplay::RefreshAiLinkStatus() {
    uint32_t dot_color;

    if ((link_status_panel_ == nullptr) || (link_status_dot_ == nullptr) || (link_status_label_ == nullptr)) {
        return;
    }

    dot_color = ai_link_online_ ? kAiLinkOnlineColor : kAiLinkOfflineColor;
    if (ContainsAsciiNoCase(ai_link_status_text_, "test")) {
        dot_color = kAiLinkBusyColor;
    } else if (ContainsAsciiNoCase(ai_link_status_text_, "init") || ContainsAsciiNoCase(ai_link_status_text_, "waiting")) {
        dot_color = kAiLinkNeutralColor;
    }

    lv_obj_set_style_bg_color(link_status_dot_, lv_color_hex(dot_color), 0);
    lv_label_set_text(link_status_label_, ai_link_status_text_.c_str());
}

void GpDebugLcdDisplay::RefreshTouchState() {
    if ((preset_value_label_ == nullptr) || (effect_value_label_ == nullptr) || (color_value_label_ == nullptr)) {
        return;
    }

    lv_label_set_text(preset_value_label_, GetPresetName(current_state_.preset));
    lv_label_set_text(effect_value_label_, GetAnimationName(current_state_.animation));
    lv_label_set_text(color_value_label_, current_state_.rgb888_text.empty() ? "#808080" : current_state_.rgb888_text.c_str());
}

void GpDebugLcdDisplay::RefreshInfoPage() {
    char buffer[224] = {0};

    if (debug_info_label_ == nullptr) {
        return;
    }

    std::snprintf(buffer,
        sizeof(buffer),
        "Source  %s\nStatus  right panel",
        current_state_.source.empty() ? "touch" : current_state_.source.c_str());
    lv_label_set_text(debug_info_label_, buffer);
}

void GpDebugLcdDisplay::RefreshPageIndicator() {
    (void)current_page_index_;
}

void GpDebugLcdDisplay::SetActivePage(int page_index) {
    current_page_index_ = std::clamp(page_index, 0, kDebugPageCount - 1);
    RefreshPageIndicator();
}

void GpDebugLcdDisplay::HandleTouchAdjust(TouchAdjust adjust) {
    if (adjust == TouchAdjust::kCaptureSnapshot) {
        if (!debug_snapshot_callback_) {
            return;
        }

        debug_snapshot_callback_();
        return;
    }

    switch (adjust) {
    case TouchAdjust::kPrevPreset:
        current_state_.preset = CyclePreset(current_state_.preset, -1);
        break;
    case TouchAdjust::kNextPreset:
        current_state_.preset = CyclePreset(current_state_.preset, 1);
        break;
    case TouchAdjust::kPrevEffect:
        current_state_.animation = CycleAnimation(current_state_.animation, -1);
        break;
    case TouchAdjust::kNextEffect:
        current_state_.animation = CycleAnimation(current_state_.animation, 1);
        break;
    case TouchAdjust::kCaptureSnapshot:
    default:
        break;
    }

    animation_start_us_ = static_cast<uint64_t>(esp_timer_get_time());
    RefreshAnimatedDot();

    if (matrix_debug_state_callback_) {
        matrix_debug_state_callback_(current_state_);
    }

    RefreshTouchState();
}

void GpDebugLcdDisplay::UpdateDotPlacement(int dot_size) {
    int available_height;

    if (debug_dot_ == nullptr) {
        return;
    }

    if (debug_preview_panel_ == nullptr) {
        lv_obj_align(debug_dot_, LV_ALIGN_CENTER, 0, 0);
        return;
    }

    available_height = lv_obj_get_content_height(debug_preview_panel_) - kDebugPreviewTopInset;
    available_height = std::max(available_height, dot_size);
    lv_obj_align(debug_dot_, LV_ALIGN_TOP_MID, 0, kDebugPreviewTopInset + (available_height - dot_size) / 2);
}

void GpDebugLcdDisplay::OnMenuButtonEvent(lv_event_t* event) {
    auto* context = static_cast<MenuButtonContext*>(lv_event_get_user_data(event));

    if ((context == nullptr) || (context->self == nullptr)) {
        return;
    }

    context->self->ScheduleDebugMenuVisible(context->action == MenuAction::kOpenDebug);
}

void GpDebugLcdDisplay::OnTouchButtonEvent(lv_event_t* event) {
    auto* context = static_cast<TouchButtonContext*>(lv_event_get_user_data(event));

    if ((context != nullptr) && (context->self != nullptr)) {
        context->self->HandleTouchAdjust(context->adjust);
    }
}

void GpDebugLcdDisplay::OnPageStripEvent(lv_event_t* event) {
    (void)event;
}
#include "gp_debug_display.h"

#include "lvgl_theme.h"

#include <algorithm>
#include <cstdio>

#include <esp_timer.h>

namespace {
constexpr int kDebugDotDefaultSize = 28;
constexpr int kDebugDotMinSize = 12;
constexpr int kDebugDotMaxSize = 58;
constexpr uint32_t kDebugAnimationTickMs = 33;
constexpr uint32_t kAiLinkOnlineColor = 0x22C55E;
constexpr uint32_t kAiLinkOfflineColor = 0xEF4444;
constexpr uint32_t kAiLinkBusyColor = 0xF59E0B;
constexpr uint32_t kAiLinkNeutralColor = 0x64748B;
constexpr int kAiLinkPanelWidth = 142;
constexpr int kAiLinkPanelHeight = 74;
constexpr int kAiLinkCenterXNumerator = 1;
constexpr int kAiLinkCenterXDenominator = 6;
constexpr int kDotCenterXNumerator = 5;
constexpr int kDotCenterXDenominator = 6;
constexpr int kDotCenterYOffset = 0;

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
}

GpDebugLcdDisplay::GpDebugLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
    int width, int height, int offset_x, int offset_y,
    bool mirror_x, bool mirror_y, bool swap_xy)
    : SpiLcdDisplay(panel_io, panel, width, height, offset_x, offset_y, mirror_x, mirror_y, swap_xy) {
    current_state_.dot_size_px = kDebugDotDefaultSize;
    ai_link_status_text_ = "AI8051U INIT";
    CreateDebugOverlay();
    CreateLinkStatusOverlay();
}

GpDebugLcdDisplay::~GpDebugLcdDisplay() {
    if (debug_animation_timer_ != nullptr) {
        lv_timer_delete(debug_animation_timer_);
        debug_animation_timer_ = nullptr;
    }
}

void GpDebugLcdDisplay::CreateDebugOverlay() {
    DisplayLockGuard lock(this);
    if (display_ == nullptr || debug_dot_ != nullptr) {
        return;
    }

    auto* screen = lv_display_get_screen_active(display_);

    debug_dot_ = lv_obj_create(screen);
    lv_obj_remove_style_all(debug_dot_);
    lv_obj_set_size(debug_dot_, current_state_.dot_size_px, current_state_.dot_size_px);
    lv_obj_set_style_radius(debug_dot_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(debug_dot_, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(debug_dot_, lv_color_hex(0x808080), 0);
    UpdateDotPlacement(current_state_.dot_size_px);
    lv_obj_move_foreground(debug_dot_);

    EnsureAnimationTimer();
    RefreshAnimatedDot();
}

void GpDebugLcdDisplay::CreateLinkStatusOverlay() {
    DisplayLockGuard lock(this);
    if (display_ == nullptr || link_status_panel_ != nullptr) {
        return;
    }

    auto* screen = lv_display_get_screen_active(display_);
    const int target_center_x = (width_ * kAiLinkCenterXNumerator) / kAiLinkCenterXDenominator;
    const int x_offset = target_center_x - (width_ / 2);

    link_status_panel_ = lv_obj_create(screen);
    lv_obj_remove_style_all(link_status_panel_);
    lv_obj_set_size(link_status_panel_, kAiLinkPanelWidth, kAiLinkPanelHeight);
    lv_obj_set_style_radius(link_status_panel_, 18, 0);
    lv_obj_set_style_bg_opa(link_status_panel_, LV_OPA_80, 0);
    lv_obj_set_style_bg_color(link_status_panel_, lv_color_hex(0x101418), 0);
    lv_obj_set_style_pad_all(link_status_panel_, 0, 0);
    lv_obj_align(link_status_panel_, LV_ALIGN_CENTER, x_offset, 0);

    link_status_dot_ = lv_obj_create(link_status_panel_);
    lv_obj_remove_style_all(link_status_dot_);
    lv_obj_set_size(link_status_dot_, 12, 12);
    lv_obj_set_style_radius(link_status_dot_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(link_status_dot_, LV_OPA_COVER, 0);
    lv_obj_align(link_status_dot_, LV_ALIGN_LEFT_MID, 10, 0);

    link_status_label_ = lv_label_create(link_status_panel_);
    lv_obj_set_size(link_status_label_, kAiLinkPanelWidth - 36, kAiLinkPanelHeight - 10);
    lv_label_set_long_mode(link_status_label_, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(link_status_label_, lv_color_hex(0xF8FAFC), 0);
    lv_obj_align(link_status_label_, LV_ALIGN_LEFT_MID, 28, 0);

    RefreshAiLinkStatus();
    lv_obj_move_foreground(link_status_panel_);
}

void GpDebugLcdDisplay::ApplyColorDebugState(const GpColorDebugState& state) {
    if (debug_dot_ == nullptr) {
        CreateDebugOverlay();
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
    RefreshAnimatedDot();
    lv_obj_move_foreground(debug_dot_);
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
    ai_link_status_text_ = status_text.empty() ? "AI8051U" : status_text;
    RefreshAiLinkStatus();
    lv_obj_move_foreground(link_status_panel_);
}

void GpDebugLcdDisplay::EnsureAnimationTimer() {
    if (debug_animation_timer_ == nullptr) {
        debug_animation_timer_ = lv_timer_create(&GpDebugLcdDisplay::OnAnimationTimer, kDebugAnimationTickMs, this);
    }
}

void GpDebugLcdDisplay::OnAnimationTimer(lv_timer_t* timer) {
    auto* self = static_cast<GpDebugLcdDisplay*>(lv_timer_get_user_data(timer));
    if (self != nullptr) {
        self->RefreshAnimatedDot();
    }
}

void GpDebugLcdDisplay::RefreshAnimatedDot() {
    if (debug_dot_ == nullptr) {
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

    dot_size = std::clamp(dot_size, kDebugDotMinSize, kDebugDotMaxSize);
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
    if (ai_link_status_text_.find("TEST") != std::string::npos) {
        dot_color = kAiLinkBusyColor;
    } else if (ai_link_status_text_.find("INIT") != std::string::npos) {
        dot_color = kAiLinkNeutralColor;
    }

    lv_obj_set_style_bg_color(link_status_dot_, lv_color_hex(dot_color), 0);
    lv_label_set_text(link_status_label_, ai_link_status_text_.c_str());
}

void GpDebugLcdDisplay::UpdateDotPlacement(int dot_size) {
    if (debug_dot_ == nullptr) {
        return;
    }

    const int target_center_x = (width_ * kDotCenterXNumerator) / kDotCenterXDenominator;
    const int x_offset = target_center_x - (width_ / 2);
    (void)dot_size;
    lv_obj_align(debug_dot_, LV_ALIGN_CENTER, x_offset, kDotCenterYOffset);
}
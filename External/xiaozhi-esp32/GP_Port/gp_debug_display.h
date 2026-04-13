#ifndef GP_DEBUG_DISPLAY_H
#define GP_DEBUG_DISPLAY_H

#include "display/lcd_display.h"

#include <cstdint>
#include <string>

enum class GpColorDebugAnimation : uint8_t {
    kSolid = 0,
    kGradient = 1,
    kPulse = 2,
};

enum class GpColorDebugPreset : uint8_t {
    kSolid = 0,
    kDiamond = 1,
    kCross = 2,
    kPythonDemo = 3,
    kScrollSubtitle = 4,
};

struct GpColorDebugState {
    uint32_t primary_rgb888 = 0x808080;
    uint32_t secondary_rgb888 = 0x202020;
    uint32_t matrix_background_rgb888 = 0x000000;
    bool has_secondary = false;
    bool has_matrix_background_rgb888 = false;
    uint16_t dot_size_px = 28;
    uint16_t animation_period_ms = 1400;
    GpColorDebugAnimation animation = GpColorDebugAnimation::kSolid;
    GpColorDebugPreset preset = GpColorDebugPreset::kSolid;
    std::string label;
    std::string rgb888_text;
    std::string source;
    std::string transcript;
};

class GpDebugLcdDisplay : public SpiLcdDisplay {
public:
    GpDebugLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
        int width, int height, int offset_x, int offset_y,
        bool mirror_x, bool mirror_y, bool swap_xy);
    ~GpDebugLcdDisplay() override;

    void ApplyColorDebugState(const GpColorDebugState& state);
    void ApplyAiLinkStatus(bool online, const std::string& status_text);

private:
    lv_obj_t* debug_dot_ = nullptr;
    lv_obj_t* link_status_panel_ = nullptr;
    lv_obj_t* link_status_dot_ = nullptr;
    lv_obj_t* link_status_label_ = nullptr;
    lv_timer_t* debug_animation_timer_ = nullptr;
    GpColorDebugState current_state_;
    uint64_t animation_start_us_ = 0;
    bool ai_link_online_ = false;
    std::string ai_link_status_text_;

    void CreateDebugOverlay();
    void CreateLinkStatusOverlay();
    void UpdateDotPlacement(int dot_size);
    void RefreshAnimatedDot();
    void RefreshAiLinkStatus();
    void EnsureAnimationTimer();
    static void OnAnimationTimer(lv_timer_t* timer);
};

#endif
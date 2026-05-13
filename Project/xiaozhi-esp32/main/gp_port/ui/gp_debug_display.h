#ifndef GP_DEBUG_DISPLAY_H
#define GP_DEBUG_DISPLAY_H

#include "gp_led_matrix_protocol.h"
#include "display/lcd_display.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>

enum class GpColorDebugAnimation : uint8_t {
    kSolid = 0,
    kGradient = 1,
    kPulse = 2,
    kScrollLeft = 3,
    kScrollRight = 4,
    kFadeIn = 5,
    kFadeOut = 6,
    kColorCycle = 7,
    kRowReveal = 8,
    kRowHide = 9,
    kGradientReveal = 10,
};

enum class GpColorDebugPreset : uint8_t {
    kSolid = 0,
    kDiamond = 1,
    kCross = 2,
    kJluEmblem = 3,
    kScrollSubtitle = 5,
    kChecker = 6,
    kBorder = 7,
    kDiagonalX = 8,
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
    using MatrixDebugStateCallback = std::function<bool(const GpColorDebugState& state)>;
    using LocalControlActionCallback = std::function<bool(GpMatrixLocalControlAction action)>;
    using DebugSnapshotCallback = std::function<std::string()>;
    using TouchCommandCallback = std::function<void(const GpColorDebugState& state, bool request_pattern_draw)>;

    /* Create the custom LVGL debug display wrapper used by the lichuang-dev board. */
    GpDebugLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
        int width, int height, int offset_x, int offset_y,
        bool mirror_x, bool mirror_y, bool swap_xy);
    ~GpDebugLcdDisplay() override;
    void SetPreviewImage(std::unique_ptr<LvglImage> image) override;

    /* Apply one debug-menu color state to the local UI preview and status widgets. */
    void ApplyColorDebugState(const GpColorDebugState& state);

    /* Refresh the AI link indicator shown inside the custom debug menu. */
    void ApplyAiLinkStatus(bool online, const std::string& status_text);

    /* Draw one compact 16x16 bitmap preview directly in the Preview card. */
    void ApplyMatrixBitmapPreview(const std::array<uint16_t, 16>& bitmap_rows,
        uint32_t primary_rgb888,
        uint32_t background_rgb888 = 0x000000);

    /* Start one buffered matrix preview sequence that can loop locally while frames arrive. */
    void BeginMatrixAnimationPreview(size_t frame_count, uint16_t frame_interval_ms);

    /* Store one preview frame inside the local animation buffer. */
    void ApplyMatrixAnimationPreviewFrame(size_t frame_index,
        const std::array<uint16_t, 16>& bitmap_rows,
        uint32_t primary_rgb888,
        uint32_t background_rgb888 = 0x000000);

    /* Mark the buffered preview sequence complete and keep looping it locally. */
    void EndMatrixAnimationPreview();

    /* Register the callback used to push debug-menu color changes to the matrix layer. */
    void SetMatrixDebugStateCallback(MatrixDebugStateCallback callback);

    /* Register the callback used by the local Snap button in the custom menu. */
    void SetDebugSnapshotCallback(DebugSnapshotCallback callback);

    /* Register the callback used to trigger LED-side local/offline scheme actions. */
    void SetLocalControlActionCallback(LocalControlActionCallback callback);

    /* Register the callback used to forward touch-generated state changes to the board transport path. */
    void SetTouchCommandCallback(TouchCommandCallback callback);

private:
    enum class MenuAction : uint8_t {
        kOpenDebug = 0,
        kCloseDebug = 1,
    };

    enum class TouchAdjust : uint8_t {
        kPrevPreset = 0,
        kNextPreset = 1,
        kPrevEffect = 2,
        kNextEffect = 3,
        kCaptureSnapshot = 4,
        kRequestPatternDraw = 5,
        kLocalNextPattern = 6,
        kLocalShowTextScroll = 7,
        kLocalShowClock = 8,
        kLocalToggleTextClock = 9,
        kLocalNextEffect = 10,
        kLocalNextColor = 11,
    };

    struct MenuButtonContext {
        GpDebugLcdDisplay* self = nullptr;
        MenuAction action = MenuAction::kOpenDebug;
    };

    struct TouchButtonContext {
        GpDebugLcdDisplay* self = nullptr;
        TouchAdjust adjust = TouchAdjust::kPrevPreset;
    };

    struct ColorSliderContext {
        GpDebugLcdDisplay* self = nullptr;
        uint8_t channel = 0U;
    };

    struct MatrixPreviewFrame {
        std::array<uint16_t, 16> bitmap_rows = {};
        uint32_t primary_rgb888 = 0xF5F5F5U;
        uint32_t background_rgb888 = 0x000000U;
        bool valid = false;
    };

    lv_obj_t* menu_entry_button_ = nullptr;
    lv_obj_t* debug_menu_panel_ = nullptr;
    lv_obj_t* debug_header_panel_ = nullptr;
    lv_obj_t* debug_page_strip_ = nullptr;
    lv_obj_t* debug_page_indicator_label_ = nullptr;
    lv_obj_t* debug_menu_content_ = nullptr;
    lv_obj_t* debug_control_page_ = nullptr;
    lv_obj_t* debug_info_page_ = nullptr;
    lv_obj_t* debug_side_panel_ = nullptr;
    lv_obj_t* debug_preview_panel_ = nullptr;
    lv_obj_t* debug_dot_ = nullptr;
    lv_obj_t* debug_image_preview_ = nullptr;
    lv_obj_t* debug_matrix_preview_ = nullptr;
    lv_obj_t* link_status_panel_ = nullptr;
    lv_obj_t* link_status_dot_ = nullptr;
    lv_obj_t* link_status_label_ = nullptr;
    lv_obj_t* debug_info_label_ = nullptr;
    lv_obj_t* touch_panel_ = nullptr;
    lv_obj_t* preset_value_label_ = nullptr;
    lv_obj_t* effect_value_label_ = nullptr;
    lv_obj_t* color_value_label_ = nullptr;
    lv_obj_t* red_slider_ = nullptr;
    lv_obj_t* green_slider_ = nullptr;
    lv_obj_t* blue_slider_ = nullptr;
    lv_obj_t* red_value_label_ = nullptr;
    lv_obj_t* green_value_label_ = nullptr;
    lv_obj_t* blue_value_label_ = nullptr;
    lv_obj_t* snapshot_button_ = nullptr;
    lv_timer_t* debug_animation_timer_ = nullptr;
    GpColorDebugState current_state_;
    uint64_t animation_start_us_ = 0;
    int current_page_index_ = 0;
    bool ai_link_online_ = false;
    bool debug_menu_visible_ = false;
    bool pending_menu_visible_ = false;
    bool has_matrix_bitmap_preview_ = false;
    bool has_matrix_animation_preview_ = false;
    bool refreshing_color_controls_ = false;
    std::string ai_link_status_text_;
    MatrixDebugStateCallback matrix_debug_state_callback_;
    LocalControlActionCallback local_control_action_callback_;
    DebugSnapshotCallback debug_snapshot_callback_;
    TouchCommandCallback touch_command_callback_;
    std::array<MenuButtonContext, 2> menu_button_contexts_;
    std::array<TouchButtonContext, 12> touch_button_contexts_;
    std::array<ColorSliderContext, 3> color_slider_contexts_;
    std::array<MatrixPreviewFrame, GP_MATRIX_ANIMATION_MAX_FRAMES> matrix_preview_frames_ = {};
    std::array<lv_obj_t*, 256> debug_matrix_pixels_ = {};
    size_t matrix_preview_frame_count_ = 0U;
    size_t matrix_preview_available_frames_ = 0U;
    size_t matrix_preview_rendered_index_ = GP_MATRIX_ANIMATION_MAX_FRAMES;
    uint16_t matrix_preview_frame_interval_ms_ = 42U;
    uint64_t matrix_preview_animation_start_us_ = 0U;

    void CreateMenuEntryButton();
    void CreateDebugMenuOverlay();
    void CreateDebugOverlay();
    void CreateLinkStatusOverlay();
    void CreateTouchOverlay();
    void SetDebugMenuVisible(bool visible);
    void ScheduleDebugMenuVisible(bool visible);
    void UpdateDotPlacement(int dot_size);
    void UpdatePreviewImagePlacement();
    void RefreshAnimatedDot();
    void RefreshAiLinkStatus();
    void RefreshTouchState();
    void RefreshInfoPage();
    void RelayoutDebugMenuSections();
    void RefreshPageIndicator();
    void SetActivePage(int page_index);
    void HandleTouchAdjust(TouchAdjust adjust);
    std::string BuildTouchCommandTranscript(TouchAdjust adjust) const;
    std::string BuildColorSliderTranscript() const;
    void EnsureAnimationTimer();
    void RenderMatrixPreviewFrame(const MatrixPreviewFrame& frame);
    void RefreshMatrixAnimationPreview();
    static void OnAnimationTimer(lv_timer_t* timer);
    static void OnAsyncMenuVisibility(void* user_data);
    static void OnMenuButtonEvent(lv_event_t* event);
    static void OnTouchButtonEvent(lv_event_t* event);
    static void OnColorSliderEvent(lv_event_t* event);
    static void OnPageStripEvent(lv_event_t* event);
};

#endif
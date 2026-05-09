#include "application.h"
#include "board.h"
#include "display.h"
#include "system_info.h"
#include "audio_codec.h"
#include "mqtt_protocol.h"
#include "websocket_protocol.h"
#include "assets/lang_config.h"
#include "mcp_server.h"
#include "assets.h"
#include "ui/gp_debug_display.h"
#include "gp_led_matrix_esp32.h"
#include "settings.h"

#include <array>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <optional>
#include <string_view>
#include <esp_log.h>
#include <cJSON.h>
#include <driver/gpio.h>
#include <arpa/inet.h>
#include <font_awesome.h>

#define TAG "Application"

namespace {

struct DebugColorKeyword {
    const char* label;
    uint32_t primary_rgb888;
    uint32_t secondary_rgb888;
    bool has_secondary;
    std::array<const char*, 6> keywords;
};

struct DebugPresetKeyword {
    const char* label;
    GpColorDebugPreset preset;
    std::array<const char*, 6> keywords;
};

const DebugColorKeyword* MatchDebugColorKeyword(const std::string& text);
bool HasBackgroundColorKeyword(const std::string& text);

constexpr std::string_view kVoiceColorAnalyzeAction = "voice_color_analyze";
constexpr std::string_view kVoiceColorResultAction = "voice_color_result";
constexpr std::string_view kMatrixPatternRequestAction = "matrix_pattern_request";
constexpr uint16_t kDefaultDotSize = 28;
constexpr uint16_t kLargeDotSize = 42;
constexpr uint16_t kSmallDotSize = 18;
constexpr uint32_t kMatrixDefaultRgb888 = 0xF5F5F5;
constexpr uint32_t kMatrixDefaultBackgroundRgb888 = 0x000000;

uint32_t g_matrixBackgroundRgb888 = kMatrixDefaultBackgroundRgb888;
std::optional<GpColorDebugState> g_lastAppliedColorState;

std::string ToAsciiLower(const std::string& text) {
    std::string lowered = text;

    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });

    return lowered;
}

std::string EscapeJsonString(const std::string& text) {
    std::string escaped;

    escaped.reserve(text.size() + 8);
    for (char ch : text) {
        switch (ch) {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\b':
            escaped += "\\b";
            break;
        case '\f':
            escaped += "\\f";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }

    return escaped;
}

const char* GetPresetLabel(GpColorDebugPreset preset) {
    switch (preset) {
    case GpColorDebugPreset::kDiamond:
        return "diamond";
    case GpColorDebugPreset::kCross:
        return "cross";
    case GpColorDebugPreset::kJluEmblem:
        return "JLU_emblem";
    case GpColorDebugPreset::kPythonDemo:
        return "python_demo";
    case GpColorDebugPreset::kScrollSubtitle:
        return "scroll_subtitle";
    case GpColorDebugPreset::kSolid:
    default:
        return "solid";
    }
}

const DebugPresetKeyword* MatchDebugPresetKeyword(const std::string& text) {
    static const std::array<DebugPresetKeyword, 5> kPresets{{
        {"diamond", GpColorDebugPreset::kDiamond, {"菱形", "diamond", "rhombus", "钻石", "菱", ""}},
        {"cross", GpColorDebugPreset::kCross, {"十字", "cross", "plus", "叉", "crosshair", ""}},
        {"JLU_emblem", GpColorDebugPreset::kJluEmblem, {"吉林大学校徽", "吉大校徽", "校徽", "jlu_emblem", "jlu emblem", "emblem"}},
        {"python_demo", GpColorDebugPreset::kPythonDemo, {"python_demo", "python demo", "像素图", "16x16", "16*16", "图案画板"}},
        {"scroll_subtitle", GpColorDebugPreset::kScrollSubtitle, {"字幕", "滚动字幕", "scroll", "text", "marquee", "滚动"}},
    }};

    const std::string lowered = ToAsciiLower(text);
    for (const auto& preset : kPresets) {
        for (const char* keyword : preset.keywords) {
            if (keyword == nullptr || keyword[0] == '\0') {
                continue;
            }
            if (text.find(keyword) != std::string::npos || lowered.find(keyword) != std::string::npos) {
                return &preset;
            }
        }
    }
    return nullptr;
}

void ApplyColorDebugToMatrix(const GpColorDebugState& state) {
    auto* matrix_led = dynamic_cast<GpLedMatrixEsp32*>(Board::GetInstance().GetLed());
    if (matrix_led == nullptr) {
        return;
    }

    (void)matrix_led->ShowDebugState(state);
}

std::string FormatRgb888(uint32_t rgb) {
    char buffer[16] = {0};
    std::snprintf(buffer, sizeof(buffer), "#%06X", static_cast<unsigned int>(rgb & 0xFFFFFFU));
    return buffer;
}

bool TryApplyBackgroundColorCommand(Display* display, const std::string& transcript) {
    const auto* keyword = MatchDebugColorKeyword(transcript);
    GpColorDebugState refreshed_state;

    if ((keyword == nullptr) || !HasBackgroundColorKeyword(transcript) || (MatchDebugPresetKeyword(transcript) != nullptr)) {
        return false;
    }

    g_matrixBackgroundRgb888 = keyword->primary_rgb888;
    if (g_lastAppliedColorState.has_value()) {
        refreshed_state = *g_lastAppliedColorState;
        refreshed_state.matrix_background_rgb888 = g_matrixBackgroundRgb888;
        refreshed_state.has_matrix_background_rgb888 = true;
        g_lastAppliedColorState = refreshed_state;

        if (refreshed_state.preset != GpColorDebugPreset::kSolid) {
            ApplyColorDebugToMatrix(refreshed_state);
        }
    }

    if (display != nullptr) {
        const std::string notification = std::string("Matrix background: ") + FormatRgb888(g_matrixBackgroundRgb888);
        display->ShowNotification(notification.c_str(), 1800);
    }

    return true;
}

const DebugColorKeyword* MatchDebugColorKeyword(const std::string& text) {
    static const std::array<DebugColorKeyword, 10> kCommands{{
        {"red", 0xFF3030, 0xFF7B54, true, {"红色", "红", "red", "crimson", "scarlet", ""}},
        {"green", 0x30D158, 0x9EF01A, true, {"绿色", "绿", "green", "lime", "emerald", ""}},
        {"blue", 0x3A86FF, 0x00BBF9, true, {"蓝色", "蓝", "blue", "azure", "cyan blue", ""}},
        {"yellow", 0xFFD60A, 0xFFB703, true, {"黄色", "黄", "yellow", "gold", "amber", ""}},
        {"purple", 0xBF5AF2, 0x7B2CBF, true, {"紫色", "紫", "purple", "violet", "magenta", ""}},
        {"white", 0xF5F5F5, 0xD9D9D9, true, {"白色", "白", "white", "silver", "gray", "grey"}},
        {"orange", 0xFF9F0A, 0xFB8500, true, {"橙色", "橙", "orange", "warm", "sunset", ""}},
        {"pink", 0xFF5FA2, 0xFF8CC6, true, {"粉色", "粉", "pink", "rose", "" , ""}},
        {"cyan", 0x14B8A6, 0x60A5FA, true, {"青色", "青", "cyan", "teal", "turquoise", "蓝绿"}},
        {"black", 0x101010, 0x404040, true, {"黑色", "黑", "black", "dark", "charcoal", ""}},
    }};

    const std::string lowered = ToAsciiLower(text);
    for (const auto& command : kCommands) {
        for (const char* keyword : command.keywords) {
            if (keyword == nullptr || keyword[0] == '\0') {
                continue;
            }
            if (text.find(keyword) != std::string::npos || lowered.find(keyword) != std::string::npos) {
                return &command;
            }
        }
    }
    return nullptr;
}

bool ContainsAnyKeyword(const std::string& text, std::initializer_list<const char*> keywords) {
    const std::string lowered = ToAsciiLower(text);
    for (const char* keyword : keywords) {
        if (keyword == nullptr || keyword[0] == '\0') {
            continue;
        }
        if (text.find(keyword) != std::string::npos || lowered.find(keyword) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool HasBackgroundColorKeyword(const std::string& text) {
    return ContainsAnyKeyword(text, {"背景", "底色", "背景色", "background", "backdrop", "bg"});
}

bool IsVoiceColorIntent(const std::string& transcript) {
    if (MatchDebugColorKeyword(transcript) != nullptr) {
        return true;
    }
    if (MatchDebugPresetKeyword(transcript) != nullptr) {
        return true;
    }
    return ContainsAnyKeyword(transcript, {"圆点", "颜色", "渐变", "动画", "背景", "底色", "rgb", "dot", "color", "gradient", "pulse", "breath", "background", "菱形", "十字", "字幕", "图案", "scroll", "pattern", "python", "16x16", "16*16", "像素图"});
}

bool IsMatrixPatternIntent(const std::string& transcript) {
    const bool has_draw_verb = ContainsAnyKeyword(transcript,
                                                  {"画", "绘", "draw", "render", "show", "显示", "生成", "写"});
    const bool has_matrix_target = ContainsAnyKeyword(transcript,
                                                      {"图案", "图形", "像素", "像素图", "pixel", "matrix", "16x16", "16*16", "bitmap", "文字", "文本", "字母", "text", "glyph", "logo", "图标", "笑脸", "心形", "箭头"});

    if (!has_draw_verb) {
        return false;
    }
    if (MatchDebugPresetKeyword(transcript) != nullptr) {
        return false;
    }
    if (HasBackgroundColorKeyword(transcript)) {
        return false;
    }
    if (ContainsAnyKeyword(transcript, {"圆点", "dot"}) && !has_matrix_target) {
        return false;
    }
    return has_matrix_target || (MatchDebugColorKeyword(transcript) != nullptr);
}

uint16_t MatchDotSize(const std::string& transcript) {
    if (ContainsAnyKeyword(transcript, {"最大", "很大", "biggest", "huge"})) {
        return 52;
    }
    if (ContainsAnyKeyword(transcript, {"变大", "放大", "大一点", "larger", "bigger", "large"})) {
        return kLargeDotSize;
    }
    if (ContainsAnyKeyword(transcript, {"变小", "缩小", "小一点", "smaller", "small"})) {
        return kSmallDotSize;
    }
    return kDefaultDotSize;
}

GpColorDebugAnimation MatchAnimation(const std::string& transcript, bool has_secondary) {
    if (ContainsAnyKeyword(transcript, {"渐变", "gradient", "彩虹", "过渡"})) {
        return GpColorDebugAnimation::kGradient;
    }
    if (ContainsAnyKeyword(transcript, {"呼吸", "脉冲", "闪动", "pulse", "breath", "animate"})) {
        return GpColorDebugAnimation::kPulse;
    }
    return has_secondary ? GpColorDebugAnimation::kGradient : GpColorDebugAnimation::kSolid;
}

std::optional<uint32_t> ParseRgb888String(const char* text) {
    if (text == nullptr) {
        return std::nullopt;
    }

    unsigned int rgb = 0;
    if (std::sscanf(text, "#%06x", &rgb) == 1 || std::sscanf(text, "0x%06x", &rgb) == 1 || std::sscanf(text, "%06x", &rgb) == 1) {
        return static_cast<uint32_t>(rgb & 0xFFFFFFU);
    }
    return std::nullopt;
}

std::optional<GpColorDebugState> BuildLocalColorState(const std::string& transcript, std::string source) {
    const auto* keyword = MatchDebugColorKeyword(transcript);
    const auto* preset_keyword = MatchDebugPresetKeyword(transcript);

    if ((keyword == nullptr) && (preset_keyword == nullptr)) {
        return std::nullopt;
    }

    GpColorDebugState state;
    if (keyword != nullptr) {
        state.primary_rgb888 = keyword->primary_rgb888;
        state.secondary_rgb888 = keyword->secondary_rgb888;
        state.has_secondary = keyword->has_secondary;
        state.label = keyword->label;
    } else {
        state.primary_rgb888 = kMatrixDefaultRgb888;
        state.secondary_rgb888 = 0x202020;
        state.has_secondary = false;
        state.label = "white";
    }

    if (preset_keyword != nullptr) {
        state.preset = preset_keyword->preset;
    }
    state.dot_size_px = MatchDotSize(transcript);
    state.animation = MatchAnimation(transcript, state.has_secondary);
    state.animation_period_ms = state.animation == GpColorDebugAnimation::kPulse ? 1100 : 1600;
    state.rgb888_text = FormatRgb888(state.primary_rgb888);
    state.source = std::move(source);
    state.transcript = transcript;
    if (preset_keyword != nullptr) {
        state.label += std::string(" ") + GetPresetLabel(state.preset);
    }

    if (state.preset == GpColorDebugPreset::kSolid) {
        state.animation = GpColorDebugAnimation::kSolid;
    }

    return state;
}

std::optional<GpColorDebugState> ParseRemoteColorState(const cJSON* payload) {
    if (!cJSON_IsObject(payload)) {
        return std::nullopt;
    }

    const auto* action = cJSON_GetObjectItem(payload, "action");
    if (!cJSON_IsString(action) || std::string_view(action->valuestring) != kVoiceColorResultAction) {
        return std::nullopt;
    }

    const auto* primary = cJSON_GetObjectItem(payload, "primary_rgb888");
    if (!cJSON_IsString(primary)) {
        return std::nullopt;
    }
    const auto primary_rgb = ParseRgb888String(primary->valuestring);
    if (!primary_rgb.has_value()) {
        return std::nullopt;
    }

    GpColorDebugState state;
    state.primary_rgb888 = *primary_rgb;
    state.rgb888_text = primary->valuestring;

    const auto* secondary = cJSON_GetObjectItem(payload, "secondary_rgb888");
    if (cJSON_IsString(secondary)) {
        const auto secondary_rgb = ParseRgb888String(secondary->valuestring);
        if (secondary_rgb.has_value()) {
            state.secondary_rgb888 = *secondary_rgb;
            state.has_secondary = true;
        }
    }

    const auto* label = cJSON_GetObjectItem(payload, "label");
    state.label = cJSON_IsString(label) ? label->valuestring : "llm";

    const auto* transcript = cJSON_GetObjectItem(payload, "transcript");
    state.transcript = cJSON_IsString(transcript) ? transcript->valuestring : "";

    const auto* source = cJSON_GetObjectItem(payload, "source");
    state.source = cJSON_IsString(source) ? source->valuestring : "llm";

    const auto* animation = cJSON_GetObjectItem(payload, "animation");
    if (cJSON_IsString(animation)) {
        const std::string animation_name = ToAsciiLower(animation->valuestring);
        if (animation_name == "gradient") {
            state.animation = GpColorDebugAnimation::kGradient;
        } else if (animation_name == "pulse") {
            state.animation = GpColorDebugAnimation::kPulse;
        } else {
            state.animation = GpColorDebugAnimation::kSolid;
        }
    }

    const auto* size = cJSON_GetObjectItem(payload, "size");
    state.dot_size_px = cJSON_IsNumber(size) ? static_cast<uint16_t>(size->valueint) : kDefaultDotSize;

    const auto* duration = cJSON_GetObjectItem(payload, "duration_ms");
    state.animation_period_ms = cJSON_IsNumber(duration) ? static_cast<uint16_t>(duration->valueint) : 1400;

    const auto* background = cJSON_GetObjectItem(payload, "background_rgb888");
    if (cJSON_IsString(background)) {
        const auto background_rgb = ParseRgb888String(background->valuestring);
        if (background_rgb.has_value()) {
            state.matrix_background_rgb888 = *background_rgb;
            state.has_matrix_background_rgb888 = true;
        }
    }

    const auto* preset = cJSON_GetObjectItem(payload, "preset");
    if (cJSON_IsString(preset)) {
        const std::string preset_name = ToAsciiLower(preset->valuestring);
        if (preset_name == "diamond") {
            state.preset = GpColorDebugPreset::kDiamond;
        } else if (preset_name == "cross") {
            state.preset = GpColorDebugPreset::kCross;
        } else if (preset_name == "jlu_emblem" || preset_name == "jlu emblem" || preset_name == "jilin university emblem") {
            state.preset = GpColorDebugPreset::kJluEmblem;
        } else if (preset_name == "python_demo" || preset_name == "python demo" || preset_name == "16x16") {
            state.preset = GpColorDebugPreset::kPythonDemo;
        } else if (preset_name == "scroll_subtitle" || preset_name == "scroll") {
            state.preset = GpColorDebugPreset::kScrollSubtitle;
        }
    }

    if (state.preset == GpColorDebugPreset::kSolid) {
        state.animation = GpColorDebugAnimation::kSolid;
    }
    return state;
}

bool ApplyColorDebugState(Display* display, const GpColorDebugState& state, bool notify) {
    auto* debug_display = dynamic_cast<GpDebugLcdDisplay*>(display);
    GpColorDebugState matrix_state = state;

    if (debug_display == nullptr) {
        return false;
    }

    if (matrix_state.has_matrix_background_rgb888) {
        g_matrixBackgroundRgb888 = matrix_state.matrix_background_rgb888;
    } else {
        matrix_state.matrix_background_rgb888 = g_matrixBackgroundRgb888;
    }
    matrix_state.has_matrix_background_rgb888 = true;
    g_lastAppliedColorState = matrix_state;

    debug_display->ApplyColorDebugState(state);
    ApplyColorDebugToMatrix(matrix_state);
    if (notify) {
        const std::string notification = std::string("Color debug: ") + state.label + " " + state.rgb888_text;
        display->ShowNotification(notification.c_str(), 1800);
    }
    return true;
}

std::string BuildColorAnalyzePayload(const std::string& transcript, std::string_view source) {
    cJSON* payload = cJSON_CreateObject();
    cJSON_AddStringToObject(payload, "action", std::string(kVoiceColorAnalyzeAction).c_str());
    cJSON_AddStringToObject(payload, "source", std::string(source).c_str());
    cJSON_AddStringToObject(payload, "transcript", transcript.c_str());

    cJSON* format = cJSON_CreateObject();
    cJSON_AddStringToObject(format, "action", std::string(kVoiceColorResultAction).c_str());
    cJSON_AddStringToObject(format, "primary_rgb888", "#RRGGBB");
    cJSON_AddStringToObject(format, "secondary_rgb888", "#RRGGBB or empty");
    cJSON_AddStringToObject(format, "background_rgb888", "#RRGGBB or empty");
    cJSON_AddStringToObject(format, "animation", "solid|gradient|pulse");
    cJSON_AddStringToObject(format, "preset", "solid|diamond|cross|python_demo|scroll_subtitle");
    cJSON_AddNumberToObject(format, "size", 36);
    cJSON_AddNumberToObject(format, "duration_ms", 1400);
    cJSON_AddStringToObject(format, "label", "teal");
    cJSON_AddStringToObject(format, "source", "llm");
    cJSON_AddStringToObject(format, "transcript", transcript.c_str());
    cJSON_AddItemToObject(payload, "response_format", format);

    char* json_text = cJSON_PrintUnformatted(payload);
    std::string result = json_text != nullptr ? json_text : "{}";
    if (json_text != nullptr) {
        cJSON_free(json_text);
    }
    cJSON_Delete(payload);
    return result;
}

std::string BuildMatrixPatternRequestPayload(const std::string& transcript, std::string_view source) {
    cJSON* payload = cJSON_CreateObject();
    cJSON* target = cJSON_CreateObject();
    cJSON* transport = cJSON_CreateObject();
    cJSON* service_hints = cJSON_CreateObject();
    char* json_text = nullptr;
    std::string result = "{}";

    cJSON_AddStringToObject(payload, "action", std::string(kMatrixPatternRequestAction).c_str());
    cJSON_AddStringToObject(payload, "source", std::string(source).c_str());
    cJSON_AddStringToObject(payload, "transcript", transcript.c_str());

    cJSON_AddStringToObject(target, "type", "led_matrix");
    cJSON_AddNumberToObject(target, "width", GP_MATRIX_WIDTH);
    cJSON_AddNumberToObject(target, "height", GP_MATRIX_HEIGHT);
    cJSON_AddStringToObject(target, "pixel_format", "rgb332");
    cJSON_AddItemToObject(payload, "target", target);

    cJSON_AddStringToObject(transport, "link", "bluetooth_frame_upload");
    cJSON_AddNumberToObject(transport, "chunk_bytes", GP_MATRIX_MAX_CHUNK_DATA);
    cJSON_AddBoolToObject(transport, "ack_required", true);
    cJSON_AddItemToObject(payload, "transport", transport);

    cJSON_AddStringToObject(service_hints, "mcp_render_tool", "self.screen.matrix_16x16.render_prompt");
    cJSON_AddStringToObject(service_hints, "mcp_frame_tool", "self.screen.matrix_16x16.draw_frame");
    cJSON_AddStringToObject(service_hints, "mcp_drawing_tool", "self.screen.matrix_16x16.draw_python");
    cJSON_AddStringToObject(service_hints, "mcp_text_tool", "self.screen.matrix_16x16.show_text");
    cJSON_AddStringToObject(service_hints, "http_control_endpoint", "/control/matrix_prompt_16x16");
    cJSON_AddItemToObject(payload, "service_hints", service_hints);

    json_text = cJSON_PrintUnformatted(payload);
    if (json_text != nullptr) {
        result = json_text;
        cJSON_free(json_text);
    }
    cJSON_Delete(payload);
    return result;
}

void HandleVoiceColorDebugFromStt(Application* app, Display* display, const std::string& transcript) {
    const bool is_color_intent = IsVoiceColorIntent(transcript);
    const bool is_matrix_pattern_intent = IsMatrixPatternIntent(transcript);

    if (!is_color_intent && !is_matrix_pattern_intent) {
        return;
    }

    if (TryApplyBackgroundColorCommand(display, transcript)) {
        return;
    }

    const auto local_state = BuildLocalColorState(transcript, "local-fallback");
    if (is_matrix_pattern_intent && (!local_state.has_value() || (local_state->preset == GpColorDebugPreset::kSolid))) {
        app->SendMatrixPatternRequest(transcript, "stt");
        return;
    }

    if (local_state.has_value()) {
        ApplyColorDebugState(display, *local_state, true);
        app->SendColorDebugAnalyze(transcript, "stt");
        return;
    }

    if (is_matrix_pattern_intent) {
        app->SendMatrixPatternRequest(transcript, "stt");
        return;
    }

    if (is_color_intent) {
        app->SendColorDebugAnalyze(transcript, "stt");
    }
}

bool HandleVoiceColorDebugFromCustom(Display* display, const cJSON* payload) {
    const auto remote_state = ParseRemoteColorState(payload);
    if (!remote_state.has_value()) {
        return false;
    }
    return ApplyColorDebugState(display, *remote_state, true);
}

}


Application::Application() {
    event_group_ = xEventGroupCreate();

#if CONFIG_USE_DEVICE_AEC && CONFIG_USE_SERVER_AEC
#error "CONFIG_USE_DEVICE_AEC and CONFIG_USE_SERVER_AEC cannot be enabled at the same time"
#elif CONFIG_USE_DEVICE_AEC
    aec_mode_ = kAecOnDeviceSide;
#elif CONFIG_USE_SERVER_AEC
    aec_mode_ = kAecOnServerSide;
#else
    aec_mode_ = kAecOff;
#endif

    esp_timer_create_args_t clock_timer_args = {
        .callback = [](void* arg) {
            Application* app = (Application*)arg;
            xEventGroupSetBits(app->event_group_, MAIN_EVENT_CLOCK_TICK);
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "clock_timer",
        .skip_unhandled_events = true
    };
    esp_timer_create(&clock_timer_args, &clock_timer_handle_);
}

Application::~Application() {
    if (clock_timer_handle_ != nullptr) {
        esp_timer_stop(clock_timer_handle_);
        esp_timer_delete(clock_timer_handle_);
    }
    vEventGroupDelete(event_group_);
}

bool Application::SetDeviceState(DeviceState state) {
    return state_machine_.TransitionTo(state);
}

void Application::Initialize() {
    auto& board = Board::GetInstance();
    SetDeviceState(kDeviceStateStarting);

    // Setup the display
    auto display = board.GetDisplay();

    // Print board name/version info
    display->SetChatMessage("system", SystemInfo::GetUserAgent().c_str());

    // Setup the audio service
    auto codec = board.GetAudioCodec();
    audio_service_.Initialize(codec);
    audio_service_.Start();

    AudioServiceCallbacks callbacks;
    callbacks.on_send_queue_available = [this]() {
        xEventGroupSetBits(event_group_, MAIN_EVENT_SEND_AUDIO);
    };
    callbacks.on_wake_word_detected = [this](const std::string& wake_word) {
        xEventGroupSetBits(event_group_, MAIN_EVENT_WAKE_WORD_DETECTED);
    };
    callbacks.on_vad_change = [this](bool speaking) {
        xEventGroupSetBits(event_group_, MAIN_EVENT_VAD_CHANGE);
    };
    audio_service_.SetCallbacks(callbacks);

    // Add state change listeners
    state_machine_.AddStateChangeListener([this](DeviceState old_state, DeviceState new_state) {
        xEventGroupSetBits(event_group_, MAIN_EVENT_STATE_CHANGED);
    });

    // Start the clock timer to update the status bar
    esp_timer_start_periodic(clock_timer_handle_, 1000000);

    // Add MCP common tools (only once during initialization)
    auto& mcp_server = McpServer::GetInstance();
    mcp_server.AddCommonTools();
    mcp_server.AddUserOnlyTools();

    // Set network event callback for UI updates and network state handling
    board.SetNetworkEventCallback([this](NetworkEvent event, const std::string& data) {
        auto display = Board::GetInstance().GetDisplay();
        
        switch (event) {
            case NetworkEvent::Scanning:
                display->ShowNotification(Lang::Strings::SCANNING_WIFI, 30000);
                xEventGroupSetBits(event_group_, MAIN_EVENT_NETWORK_DISCONNECTED);
                break;
            case NetworkEvent::Connecting: {
                if (data.empty()) {
                    // Cellular network - registering without carrier info yet
                    display->SetStatus(Lang::Strings::REGISTERING_NETWORK);
                } else {
                    // WiFi or cellular with carrier info
                    std::string msg = Lang::Strings::CONNECT_TO;
                    msg += data;
                    msg += "...";
                    display->ShowNotification(msg.c_str(), 30000);
                }
                break;
            }
            case NetworkEvent::Connected: {
                std::string msg = Lang::Strings::CONNECTED_TO;
                msg += data;
                display->ShowNotification(msg.c_str(), 30000);
                xEventGroupSetBits(event_group_, MAIN_EVENT_NETWORK_CONNECTED);
                break;
            }
            case NetworkEvent::Disconnected:
                xEventGroupSetBits(event_group_, MAIN_EVENT_NETWORK_DISCONNECTED);
                break;
            case NetworkEvent::WifiConfigModeEnter:
                // WiFi config mode enter is handled by WifiBoard internally
                break;
            case NetworkEvent::WifiConfigModeExit:
                // WiFi config mode exit is handled by WifiBoard internally
                break;
            // Cellular modem specific events
            case NetworkEvent::ModemDetecting:
                display->SetStatus(Lang::Strings::DETECTING_MODULE);
                break;
            case NetworkEvent::ModemErrorNoSim:
                Alert(Lang::Strings::ERROR, Lang::Strings::PIN_ERROR, "triangle_exclamation", Lang::Sounds::OGG_ERR_PIN);
                break;
            case NetworkEvent::ModemErrorRegDenied:
                Alert(Lang::Strings::ERROR, Lang::Strings::REG_ERROR, "triangle_exclamation", Lang::Sounds::OGG_ERR_REG);
                break;
            case NetworkEvent::ModemErrorInitFailed:
                display->SetStatus(Lang::Strings::DETECTING_MODULE);
                display->SetChatMessage("system", Lang::Strings::DETECTING_MODULE);
                break;
            case NetworkEvent::ModemErrorTimeout:
                display->SetStatus(Lang::Strings::REGISTERING_NETWORK);
                break;
        }
    });

    // Start network asynchronously
    board.StartNetwork();

    // Update the status bar immediately to show the network state
    display->UpdateStatusBar(true);
}

void Application::Run() {
    const EventBits_t ALL_EVENTS = 
        MAIN_EVENT_SCHEDULE |
        MAIN_EVENT_SEND_AUDIO |
        MAIN_EVENT_WAKE_WORD_DETECTED |
        MAIN_EVENT_VAD_CHANGE |
        MAIN_EVENT_CLOCK_TICK |
        MAIN_EVENT_ERROR |
        MAIN_EVENT_NETWORK_CONNECTED |
        MAIN_EVENT_NETWORK_DISCONNECTED |
        MAIN_EVENT_TOGGLE_CHAT |
        MAIN_EVENT_START_LISTENING |
        MAIN_EVENT_STOP_LISTENING |
        MAIN_EVENT_ACTIVATION_DONE |
        MAIN_EVENT_STATE_CHANGED;

    while (true) {
        auto bits = xEventGroupWaitBits(event_group_, ALL_EVENTS, pdTRUE, pdFALSE, portMAX_DELAY);

        if (bits & MAIN_EVENT_ERROR) {
            SetDeviceState(kDeviceStateIdle);
            Alert(Lang::Strings::ERROR, last_error_message_.c_str(), "circle_xmark", Lang::Sounds::OGG_EXCLAMATION);
        }

        if (bits & MAIN_EVENT_NETWORK_CONNECTED) {
            HandleNetworkConnectedEvent();
        }

        if (bits & MAIN_EVENT_NETWORK_DISCONNECTED) {
            HandleNetworkDisconnectedEvent();
        }

        if (bits & MAIN_EVENT_ACTIVATION_DONE) {
            HandleActivationDoneEvent();
        }

        if (bits & MAIN_EVENT_STATE_CHANGED) {
            HandleStateChangedEvent();
        }

        if (bits & MAIN_EVENT_TOGGLE_CHAT) {
            HandleToggleChatEvent();
        }

        if (bits & MAIN_EVENT_START_LISTENING) {
            HandleStartListeningEvent();
        }

        if (bits & MAIN_EVENT_STOP_LISTENING) {
            HandleStopListeningEvent();
        }

        if ((bits & MAIN_EVENT_SEND_AUDIO) && protocol_) {
            bool rearmed = false;
            while (auto packet = audio_service_.PopPacketFromSendQueue()) {
                if (!protocol_->SendAudio(std::move(packet))) {
                    if (!rearmed && audio_service_.HasPendingSendPackets()) {
                        xEventGroupSetBits(event_group_, MAIN_EVENT_SEND_AUDIO);
                        rearmed = true;
                    }
                    break;
                }
            }
        }

        if (bits & MAIN_EVENT_SCHEDULE) {
            std::unique_lock<std::mutex> lock(mutex_);
            auto tasks = std::move(main_tasks_);
            lock.unlock();
            for (auto& task : tasks) {
                task();
            }
        }

        if (bits & MAIN_EVENT_WAKE_WORD_DETECTED) {
            HandleWakeWordDetectedEvent();
        }

        if (bits & MAIN_EVENT_VAD_CHANGE) {
            if (GetDeviceState() == kDeviceStateListening) {
                auto led = Board::GetInstance().GetLed();
                led->OnStateChanged();
            }
        }

        if (bits & MAIN_EVENT_CLOCK_TICK) {
            clock_ticks_++;
            auto display = Board::GetInstance().GetDisplay();
            display->UpdateStatusBar();

            // Print debug info every 10 seconds
            if (clock_ticks_ % 10 == 0) {
                SystemInfo::PrintHeapStats();
            }

            if (protocol_ && audio_service_.HasPendingSendPackets()) {
                xEventGroupSetBits(event_group_, MAIN_EVENT_SEND_AUDIO);
            }
        }
    }
}

void Application::HandleNetworkConnectedEvent() {
    ESP_LOGI(TAG, "Network connected");
    auto state = GetDeviceState();

    if (state == kDeviceStateStarting || state == kDeviceStateWifiConfiguring) {
        // Network is ready, start activation
        SetDeviceState(kDeviceStateActivating);
        if (activation_task_handle_ != nullptr) {
            ESP_LOGW(TAG, "Activation task already running");
            return;
        }

        xTaskCreate([](void* arg) {
            Application* app = static_cast<Application*>(arg);
            app->ActivationTask();
            app->activation_task_handle_ = nullptr;
            vTaskDelete(NULL);
        }, "activation", 4096 * 2, this, 2, &activation_task_handle_);
    }

    // Update the status bar immediately to show the network state
    auto display = Board::GetInstance().GetDisplay();
    display->UpdateStatusBar(true);
}

void Application::HandleNetworkDisconnectedEvent() {
    // Close current conversation when network disconnected
    auto state = GetDeviceState();
    if (state == kDeviceStateConnecting || state == kDeviceStateListening || state == kDeviceStateSpeaking) {
        ESP_LOGI(TAG, "Closing audio channel due to network disconnection");
        protocol_->CloseAudioChannel();
    }

    // Update the status bar immediately to show the network state
    auto display = Board::GetInstance().GetDisplay();
    display->UpdateStatusBar(true);
}

void Application::HandleActivationDoneEvent() {
    ESP_LOGI(TAG, "Activation done");

    SystemInfo::PrintHeapStats();
    SetDeviceState(kDeviceStateIdle);

    has_server_time_ = ota_->HasServerTime();

    auto display = Board::GetInstance().GetDisplay();
    std::string message = std::string(Lang::Strings::VERSION) + ota_->GetCurrentVersion();
    display->ShowNotification(message.c_str());
    display->SetChatMessage("system", "");

    // Play the success sound to indicate the device is ready
    audio_service_.PlaySound(Lang::Sounds::OGG_SUCCESS);

    // Release OTA object after activation is complete
    ota_.reset();
    auto& board = Board::GetInstance();
    board.SetPowerSaveLevel(PowerSaveLevel::LOW_POWER);
}

void Application::ActivationTask() {
    // Create OTA object for activation process
    ota_ = std::make_unique<Ota>();

    // Check for new assets version
    CheckAssetsVersion();

    // Check for new firmware version
    CheckNewVersion();

    // Initialize the protocol
    InitializeProtocol();

    // Signal completion to main loop
    xEventGroupSetBits(event_group_, MAIN_EVENT_ACTIVATION_DONE);
}

void Application::CheckAssetsVersion() {
    // Only allow CheckAssetsVersion to be called once
    if (assets_version_checked_) {
        return;
    }
    assets_version_checked_ = true;

    auto& board = Board::GetInstance();
    auto display = board.GetDisplay();
    auto& assets = Assets::GetInstance();

    if (!assets.partition_valid()) {
        ESP_LOGW(TAG, "Assets partition is disabled for board %s", BOARD_NAME);
        return;
    }
    
    Settings settings("assets", true);
    // Check if there is a new assets need to be downloaded
    std::string download_url = settings.GetString("download_url");

    if (!download_url.empty()) {
        settings.EraseKey("download_url");

        char message[256];
        snprintf(message, sizeof(message), Lang::Strings::FOUND_NEW_ASSETS, download_url.c_str());
        Alert(Lang::Strings::LOADING_ASSETS, message, "cloud_arrow_down", Lang::Sounds::OGG_UPGRADE);
        
        // Wait for the audio service to be idle for 3 seconds
        vTaskDelay(pdMS_TO_TICKS(3000));
        SetDeviceState(kDeviceStateUpgrading);
        board.SetPowerSaveLevel(PowerSaveLevel::PERFORMANCE);
        display->SetChatMessage("system", Lang::Strings::PLEASE_WAIT);

        bool success = assets.Download(download_url, [display](int progress, size_t speed) -> void {
            std::thread([display, progress, speed]() {
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "%d%% %uKB/s", progress, speed / 1024);
                display->SetChatMessage("system", buffer);
            }).detach();
        });

        board.SetPowerSaveLevel(PowerSaveLevel::LOW_POWER);
        vTaskDelay(pdMS_TO_TICKS(1000));

        if (!success) {
            Alert(Lang::Strings::ERROR, Lang::Strings::DOWNLOAD_ASSETS_FAILED, "circle_xmark", Lang::Sounds::OGG_EXCLAMATION);
            vTaskDelay(pdMS_TO_TICKS(2000));
            SetDeviceState(kDeviceStateActivating);
            return;
        }
    }

    // Apply assets
    assets.Apply();
    display->SetChatMessage("system", "");
    display->SetEmotion("microchip_ai");
}

void Application::CheckNewVersion() {
    const int MAX_RETRY = 10;
    int retry_count = 0;
    int retry_delay = 10; // Initial retry delay in seconds

    auto& board = Board::GetInstance();
    while (true) {
        auto display = board.GetDisplay();
        display->SetStatus(Lang::Strings::CHECKING_NEW_VERSION);

        esp_err_t err = ota_->CheckVersion();
        if (err != ESP_OK) {
            retry_count++;
            if (retry_count >= MAX_RETRY) {
                ESP_LOGE(TAG, "Too many retries, exit version check");
                return;
            }

            char error_message[128];
            snprintf(error_message, sizeof(error_message), "code=%d, url=%s", err, ota_->GetCheckVersionUrl().c_str());
            char buffer[256];
            snprintf(buffer, sizeof(buffer), Lang::Strings::CHECK_NEW_VERSION_FAILED, retry_delay, error_message);
            Alert(Lang::Strings::ERROR, buffer, "cloud_slash", Lang::Sounds::OGG_EXCLAMATION);

            ESP_LOGW(TAG, "Check new version failed, retry in %d seconds (%d/%d)", retry_delay, retry_count, MAX_RETRY);
            for (int i = 0; i < retry_delay; i++) {
                vTaskDelay(pdMS_TO_TICKS(1000));
                if (GetDeviceState() == kDeviceStateIdle) {
                    break;
                }
            }
            retry_delay *= 2; // Double the retry delay
            continue;
        }
        retry_count = 0;
        retry_delay = 10; // Reset retry delay

        if (ota_->HasNewVersion()) {
            if (UpgradeFirmware(ota_->GetFirmwareUrl(), ota_->GetFirmwareVersion())) {
                return; // This line will never be reached after reboot
            }
            // If upgrade failed, continue to normal operation
        }

        // No new version, mark the current version as valid
        ota_->MarkCurrentVersionValid();
        if (!ota_->HasActivationCode() && !ota_->HasActivationChallenge()) {
            // Exit the loop if done checking new version
            break;
        }

        display->SetStatus(Lang::Strings::ACTIVATION);
        // Activation code is shown to the user and waiting for the user to input
        if (ota_->HasActivationCode()) {
            ShowActivationCode(ota_->GetActivationCode(), ota_->GetActivationMessage());
        }

        // This will block the loop until the activation is done or timeout
        for (int i = 0; i < 10; ++i) {
            ESP_LOGI(TAG, "Activating... %d/%d", i + 1, 10);
            esp_err_t err = ota_->Activate();
            if (err == ESP_OK) {
                break;
            } else if (err == ESP_ERR_TIMEOUT) {
                vTaskDelay(pdMS_TO_TICKS(3000));
            } else {
                vTaskDelay(pdMS_TO_TICKS(10000));
            }
            if (GetDeviceState() == kDeviceStateIdle) {
                break;
            }
        }
    }
}

void Application::InitializeProtocol() {
    auto& board = Board::GetInstance();
    auto display = board.GetDisplay();
    auto codec = board.GetAudioCodec();

    display->SetStatus(Lang::Strings::LOADING_PROTOCOL);

    if (ota_->HasMqttConfig()) {
        protocol_ = std::make_unique<MqttProtocol>();
    } else if (ota_->HasWebsocketConfig()) {
        protocol_ = std::make_unique<WebsocketProtocol>();
    } else {
        ESP_LOGW(TAG, "No protocol specified in the OTA config, using MQTT");
        protocol_ = std::make_unique<MqttProtocol>();
    }

    protocol_->OnConnected([this]() {
        DismissAlert();
    });

    protocol_->OnNetworkError([this](const std::string& message) {
        last_error_message_ = message;
        xEventGroupSetBits(event_group_, MAIN_EVENT_ERROR);
    });
    
    protocol_->OnIncomingAudio([this](std::unique_ptr<AudioStreamPacket> packet) {
        if (GetDeviceState() == kDeviceStateSpeaking) {
            audio_service_.PushPacketToDecodeQueue(std::move(packet));
        }
    });
    
    protocol_->OnAudioChannelOpened([this, codec, &board]() {
        board.SetPowerSaveLevel(PowerSaveLevel::PERFORMANCE);
        if (protocol_->server_sample_rate() != codec->output_sample_rate()) {
            ESP_LOGW(TAG, "Server sample rate %d does not match device output sample rate %d, resampling may cause distortion",
                protocol_->server_sample_rate(), codec->output_sample_rate());
        }
    });
    
    protocol_->OnAudioChannelClosed([this, &board]() {
        board.SetPowerSaveLevel(PowerSaveLevel::LOW_POWER);
        Schedule([this]() {
            auto display = Board::GetInstance().GetDisplay();
            display->SetChatMessage("system", "");
            SetDeviceState(kDeviceStateIdle);
        });
    });
    
    protocol_->OnIncomingJson([this, display](const cJSON* root) {
        // Parse JSON data
        auto type = cJSON_GetObjectItem(root, "type");
        if (strcmp(type->valuestring, "tts") == 0) {
            auto state = cJSON_GetObjectItem(root, "state");
            if (strcmp(state->valuestring, "start") == 0) {
                Schedule([this]() {
                    aborted_ = false;
                    SetDeviceState(kDeviceStateSpeaking);
                });
            } else if (strcmp(state->valuestring, "stop") == 0) {
                Schedule([this]() {
                    if (GetDeviceState() == kDeviceStateSpeaking) {
                        if (listening_mode_ == kListeningModeManualStop) {
                            SetDeviceState(kDeviceStateIdle);
                        } else {
                            SetDeviceState(kDeviceStateListening);
                        }
                    }
                });
            } else if (strcmp(state->valuestring, "sentence_start") == 0) {
                auto text = cJSON_GetObjectItem(root, "text");
                if (cJSON_IsString(text)) {
                    ESP_LOGI(TAG, "<< %s", text->valuestring);
                    Schedule([this, display, message = std::string(text->valuestring)]() {
                        display->SetChatMessage("assistant", message.c_str());
                    });
                }
            }
        } else if (strcmp(type->valuestring, "stt") == 0) {
            auto text = cJSON_GetObjectItem(root, "text");
            if (cJSON_IsString(text)) {
                ESP_LOGI(TAG, ">> %s", text->valuestring);
                Schedule([this, display, message = std::string(text->valuestring)]() {
                    display->SetChatMessage("user", message.c_str());
                    HandleVoiceColorDebugFromStt(this, display, message);
                });
            }
        } else if (strcmp(type->valuestring, "llm") == 0) {
            auto emotion = cJSON_GetObjectItem(root, "emotion");
            if (cJSON_IsString(emotion)) {
                Schedule([this, display, emotion_str = std::string(emotion->valuestring)]() {
                    display->SetEmotion(emotion_str.c_str());
                });
            }
        } else if (strcmp(type->valuestring, "mcp") == 0) {
            auto payload = cJSON_GetObjectItem(root, "payload");
            if (cJSON_IsObject(payload)) {
                if (HandlePendingMcpResponse(payload)) {
                    return;
                }
                McpServer::GetInstance().ParseMessage(payload);
            }
        } else if (strcmp(type->valuestring, "system") == 0) {
            auto command = cJSON_GetObjectItem(root, "command");
            if (cJSON_IsString(command)) {
                ESP_LOGI(TAG, "System command: %s", command->valuestring);
                if (strcmp(command->valuestring, "reboot") == 0) {
                    // Do a reboot if user requests a OTA update
                    Schedule([this]() {
                        Reboot();
                    });
                } else {
                    ESP_LOGW(TAG, "Unknown system command: %s", command->valuestring);
                }
            }
        } else if (strcmp(type->valuestring, "alert") == 0) {
            auto status = cJSON_GetObjectItem(root, "status");
            auto message = cJSON_GetObjectItem(root, "message");
            auto emotion = cJSON_GetObjectItem(root, "emotion");
            if (cJSON_IsString(status) && cJSON_IsString(message) && cJSON_IsString(emotion)) {
                Alert(status->valuestring, message->valuestring, emotion->valuestring, Lang::Sounds::OGG_VIBRATION);
            } else {
                ESP_LOGW(TAG, "Alert command requires status, message and emotion");
            }
        } else if (strcmp(type->valuestring, "custom") == 0) {
            auto payload = cJSON_GetObjectItem(root, "payload");
            char* root_dump = cJSON_PrintUnformatted(root);
            ESP_LOGI(TAG, "Received custom message: %s", root_dump != nullptr ? root_dump : "{}");
            if (root_dump != nullptr) {
                cJSON_free(root_dump);
            }
            if (cJSON_IsObject(payload)) {
                if (HandleVoiceColorDebugFromCustom(display, payload)) {
                    return;
                }
                char* payload_dump = cJSON_PrintUnformatted(payload);
                const std::string payload_str = payload_dump != nullptr ? payload_dump : "{}";
                if (payload_dump != nullptr) {
                    cJSON_free(payload_dump);
                }
                Schedule([this, display, payload_str]() {
                    display->SetChatMessage("system", payload_str.c_str());
                });
            } else {
                ESP_LOGW(TAG, "Invalid custom message format: missing payload");
            }
        } else {
            ESP_LOGW(TAG, "Unknown message type: %s", type->valuestring);
        }
    });
    
    protocol_->Start();
}

void Application::ShowActivationCode(const std::string& code, const std::string& message) {
    struct digit_sound {
        char digit;
        const std::string_view& sound;
    };
    static const std::array<digit_sound, 10> digit_sounds{{
        digit_sound{'0', Lang::Sounds::OGG_0},
        digit_sound{'1', Lang::Sounds::OGG_1}, 
        digit_sound{'2', Lang::Sounds::OGG_2},
        digit_sound{'3', Lang::Sounds::OGG_3},
        digit_sound{'4', Lang::Sounds::OGG_4},
        digit_sound{'5', Lang::Sounds::OGG_5},
        digit_sound{'6', Lang::Sounds::OGG_6},
        digit_sound{'7', Lang::Sounds::OGG_7},
        digit_sound{'8', Lang::Sounds::OGG_8},
        digit_sound{'9', Lang::Sounds::OGG_9}
    }};

    // This sentence uses 9KB of SRAM, so we need to wait for it to finish
    Alert(Lang::Strings::ACTIVATION, message.c_str(), "link", Lang::Sounds::OGG_ACTIVATION);

    for (const auto& digit : code) {
        auto it = std::find_if(digit_sounds.begin(), digit_sounds.end(),
            [digit](const digit_sound& ds) { return ds.digit == digit; });
        if (it != digit_sounds.end()) {
            audio_service_.PlaySound(it->sound);
        }
    }
}

void Application::Alert(const char* status, const char* message, const char* emotion, const std::string_view& sound) {
    ESP_LOGW(TAG, "Alert [%s] %s: %s", emotion, status, message);
    auto display = Board::GetInstance().GetDisplay();
    display->SetStatus(status);
    display->SetEmotion(emotion);
    display->SetChatMessage("system", message);
    if (!sound.empty()) {
        audio_service_.PlaySound(sound);
    }
}

void Application::DismissAlert() {
    if (GetDeviceState() == kDeviceStateIdle) {
        auto display = Board::GetInstance().GetDisplay();
        display->SetStatus(Lang::Strings::STANDBY);
        display->SetEmotion("neutral");
        display->SetChatMessage("system", "");
    }
}

void Application::ToggleChatState() {
    xEventGroupSetBits(event_group_, MAIN_EVENT_TOGGLE_CHAT);
}

void Application::StartListening() {
    xEventGroupSetBits(event_group_, MAIN_EVENT_START_LISTENING);
}

void Application::StopListening() {
    xEventGroupSetBits(event_group_, MAIN_EVENT_STOP_LISTENING);
}

void Application::HandleToggleChatEvent() {
    auto state = GetDeviceState();
    
    if (state == kDeviceStateActivating) {
        SetDeviceState(kDeviceStateIdle);
        return;
    } else if (state == kDeviceStateWifiConfiguring) {
        audio_service_.EnableAudioTesting(true);
        SetDeviceState(kDeviceStateAudioTesting);
        return;
    } else if (state == kDeviceStateAudioTesting) {
        audio_service_.EnableAudioTesting(false);
        SetDeviceState(kDeviceStateWifiConfiguring);
        return;
    }

    if (!protocol_) {
        ESP_LOGE(TAG, "Protocol not initialized");
        return;
    }

    if (state == kDeviceStateIdle) {
        if (!protocol_->IsAudioChannelOpened()) {
            SetDeviceState(kDeviceStateConnecting);
            if (!protocol_->OpenAudioChannel()) {
                return;
            }
        }

        SetListeningMode(aec_mode_ == kAecOff ? kListeningModeAutoStop : kListeningModeRealtime);
    } else if (state == kDeviceStateSpeaking) {
        AbortSpeaking(kAbortReasonNone);
    } else if (state == kDeviceStateListening) {
        protocol_->CloseAudioChannel();
    }
}

void Application::HandleStartListeningEvent() {
    auto state = GetDeviceState();
    
    if (state == kDeviceStateActivating) {
        SetDeviceState(kDeviceStateIdle);
        return;
    } else if (state == kDeviceStateWifiConfiguring) {
        audio_service_.EnableAudioTesting(true);
        SetDeviceState(kDeviceStateAudioTesting);
        return;
    }

    if (!protocol_) {
        ESP_LOGE(TAG, "Protocol not initialized");
        return;
    }
    
    if (state == kDeviceStateIdle) {
        if (!protocol_->IsAudioChannelOpened()) {
            SetDeviceState(kDeviceStateConnecting);
            if (!protocol_->OpenAudioChannel()) {
                return;
            }
        }

        SetListeningMode(kListeningModeManualStop);
    } else if (state == kDeviceStateSpeaking) {
        AbortSpeaking(kAbortReasonNone);
        SetListeningMode(kListeningModeManualStop);
    }
}

void Application::HandleStopListeningEvent() {
    auto state = GetDeviceState();
    
    if (state == kDeviceStateAudioTesting) {
        audio_service_.EnableAudioTesting(false);
        SetDeviceState(kDeviceStateWifiConfiguring);
        return;
    } else if (state == kDeviceStateListening) {
        if (protocol_) {
            protocol_->SendStopListening();
        }
        SetDeviceState(kDeviceStateIdle);
    }
}

void Application::HandleWakeWordDetectedEvent() {
    if (!protocol_) {
        return;
    }

    auto state = GetDeviceState();
    
    if (state == kDeviceStateIdle) {
        audio_service_.EncodeWakeWord();

        if (!protocol_->IsAudioChannelOpened()) {
            SetDeviceState(kDeviceStateConnecting);
            if (!protocol_->OpenAudioChannel()) {
                audio_service_.EnableWakeWordDetection(true);
                return;
            }
        }

        auto wake_word = audio_service_.GetLastWakeWord();
        ESP_LOGI(TAG, "Wake word detected: %s", wake_word.c_str());
#if CONFIG_SEND_WAKE_WORD_DATA
        // Encode and send the wake word data to the server
        while (auto packet = audio_service_.PopWakeWordPacket()) {
            protocol_->SendAudio(std::move(packet));
        }
        // Set the chat state to wake word detected
        protocol_->SendWakeWordDetected(wake_word);
        SetListeningMode(aec_mode_ == kAecOff ? kListeningModeAutoStop : kListeningModeRealtime);
#else
        // Set flag to play popup sound after state changes to listening
        // (PlaySound here would be cleared by ResetDecoder in EnableVoiceProcessing)
        play_popup_on_listening_ = true;
        SetListeningMode(aec_mode_ == kAecOff ? kListeningModeAutoStop : kListeningModeRealtime);
#endif
    } else if (state == kDeviceStateSpeaking) {
        AbortSpeaking(kAbortReasonWakeWordDetected);
    } else if (state == kDeviceStateActivating) {
        // Restart the activation check if the wake word is detected during activation
        SetDeviceState(kDeviceStateIdle);
    }
}

void Application::HandleStateChangedEvent() {
    DeviceState new_state = state_machine_.GetState();
    clock_ticks_ = 0;

    auto& board = Board::GetInstance();
    auto display = board.GetDisplay();
    auto led = board.GetLed();
    led->OnStateChanged();
    
    switch (new_state) {
        case kDeviceStateUnknown:
        case kDeviceStateIdle:
            display->SetStatus(Lang::Strings::STANDBY);
            display->SetEmotion("neutral");
            audio_service_.EnableVoiceProcessing(false);
            audio_service_.EnableWakeWordDetection(true);
            break;
        case kDeviceStateConnecting:
            display->SetStatus(Lang::Strings::CONNECTING);
            display->SetEmotion("neutral");
            display->SetChatMessage("system", "");
            break;
        case kDeviceStateListening:
            display->SetStatus(Lang::Strings::LISTENING);
            display->SetEmotion("neutral");

            // Make sure the audio processor is running
            if (!audio_service_.IsAudioProcessorRunning()) {
                // Send the start listening command
                protocol_->SendStartListening(listening_mode_);
                audio_service_.EnableVoiceProcessing(true);
                audio_service_.EnableWakeWordDetection(false);
            }

            // Play popup sound after ResetDecoder (in EnableVoiceProcessing) has been called
            if (play_popup_on_listening_) {
                play_popup_on_listening_ = false;
                audio_service_.PlaySound(Lang::Sounds::OGG_POPUP);
            }
            break;
        case kDeviceStateSpeaking:
            display->SetStatus(Lang::Strings::SPEAKING);

            if (listening_mode_ != kListeningModeRealtime) {
                audio_service_.EnableVoiceProcessing(false);
                // Only AFE wake word can be detected in speaking mode
                audio_service_.EnableWakeWordDetection(audio_service_.IsAfeWakeWord());
            }
            audio_service_.ResetDecoder();
            break;
        case kDeviceStateWifiConfiguring:
            audio_service_.EnableVoiceProcessing(false);
            audio_service_.EnableWakeWordDetection(false);
            break;
        default:
            // Do nothing
            break;
    }
}

void Application::Schedule(std::function<void()>&& callback) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        main_tasks_.push_back(std::move(callback));
    }
    xEventGroupSetBits(event_group_, MAIN_EVENT_SCHEDULE);
}

void Application::AbortSpeaking(AbortReason reason) {
    ESP_LOGI(TAG, "Abort speaking");
    aborted_ = true;
    if (protocol_) {
        protocol_->SendAbortSpeaking(reason);
    }
}

void Application::SetListeningMode(ListeningMode mode) {
    listening_mode_ = mode;
    SetDeviceState(kDeviceStateListening);
}

void Application::Reboot() {
    ESP_LOGI(TAG, "Rebooting...");
    // Disconnect the audio channel
    if (protocol_ && protocol_->IsAudioChannelOpened()) {
        protocol_->CloseAudioChannel();
    }
    protocol_.reset();
    audio_service_.Stop();

    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
}

bool Application::UpgradeFirmware(const std::string& url, const std::string& version) {
    auto& board = Board::GetInstance();
    auto display = board.GetDisplay();

    std::string upgrade_url = url;
    std::string version_info = version.empty() ? "(Manual upgrade)" : version;

    // Close audio channel if it's open
    if (protocol_ && protocol_->IsAudioChannelOpened()) {
        ESP_LOGI(TAG, "Closing audio channel before firmware upgrade");
        protocol_->CloseAudioChannel();
    }
    ESP_LOGI(TAG, "Starting firmware upgrade from URL: %s", upgrade_url.c_str());

    Alert(Lang::Strings::OTA_UPGRADE, Lang::Strings::UPGRADING, "download", Lang::Sounds::OGG_UPGRADE);
    vTaskDelay(pdMS_TO_TICKS(3000));

    SetDeviceState(kDeviceStateUpgrading);

    std::string message = std::string(Lang::Strings::NEW_VERSION) + version_info;
    display->SetChatMessage("system", message.c_str());

    board.SetPowerSaveLevel(PowerSaveLevel::PERFORMANCE);
    audio_service_.Stop();
    vTaskDelay(pdMS_TO_TICKS(1000));

    bool upgrade_success = Ota::Upgrade(upgrade_url, [display](int progress, size_t speed) {
        std::thread([display, progress, speed]() {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%d%% %uKB/s", progress, speed / 1024);
            display->SetChatMessage("system", buffer);
        }).detach();
    });

    if (!upgrade_success) {
        // Upgrade failed, restart audio service and continue running
        ESP_LOGE(TAG, "Firmware upgrade failed, restarting audio service and continuing operation...");
        audio_service_.Start(); // Restart audio service
        board.SetPowerSaveLevel(PowerSaveLevel::LOW_POWER); // Restore power save level
        Alert(Lang::Strings::ERROR, Lang::Strings::UPGRADE_FAILED, "circle_xmark", Lang::Sounds::OGG_EXCLAMATION);
        vTaskDelay(pdMS_TO_TICKS(3000));
        return false;
    } else {
        // Upgrade success, reboot immediately
        ESP_LOGI(TAG, "Firmware upgrade successful, rebooting...");
        display->SetChatMessage("system", "Upgrade successful, rebooting...");
        vTaskDelay(pdMS_TO_TICKS(1000)); // Brief pause to show message
        Reboot();
        return true;
    }
}

void Application::WakeWordInvoke(const std::string& wake_word) {
    if (!protocol_) {
        return;
    }

    auto state = GetDeviceState();
    
    if (state == kDeviceStateIdle) {
        audio_service_.EncodeWakeWord();

        if (!protocol_->IsAudioChannelOpened()) {
            SetDeviceState(kDeviceStateConnecting);
            if (!protocol_->OpenAudioChannel()) {
                audio_service_.EnableWakeWordDetection(true);
                return;
            }
        }

        ESP_LOGI(TAG, "Wake word detected: %s", wake_word.c_str());
#if CONFIG_USE_AFE_WAKE_WORD || CONFIG_USE_CUSTOM_WAKE_WORD
        // Encode and send the wake word data to the server
        while (auto packet = audio_service_.PopWakeWordPacket()) {
            protocol_->SendAudio(std::move(packet));
        }
        // Set the chat state to wake word detected
        protocol_->SendWakeWordDetected(wake_word);
        SetListeningMode(aec_mode_ == kAecOff ? kListeningModeAutoStop : kListeningModeRealtime);
#else
        // Set flag to play popup sound after state changes to listening
        // (PlaySound here would be cleared by ResetDecoder in EnableVoiceProcessing)
        play_popup_on_listening_ = true;
        SetListeningMode(aec_mode_ == kAecOff ? kListeningModeAutoStop : kListeningModeRealtime);
#endif
    } else if (state == kDeviceStateSpeaking) {
        Schedule([this]() {
            AbortSpeaking(kAbortReasonNone);
        });
    } else if (state == kDeviceStateListening) {   
        Schedule([this]() {
            if (protocol_) {
                protocol_->CloseAudioChannel();
            }
        });
    }
}

bool Application::CanEnterSleepMode() {
    if (GetDeviceState() != kDeviceStateIdle) {
        return false;
    }

    if (protocol_ && protocol_->IsAudioChannelOpened()) {
        return false;
    }

    if (!audio_service_.IsIdle()) {
        return false;
    }

    // Now it is safe to enter sleep mode
    return true;
}

bool Application::CallMcpTool(const std::string& tool_name, const std::string& arguments_json,
    std::string* result_json, std::string* error_message, int timeout_ms) {
    std::shared_ptr<PendingMcpCall> pending_call;
    std::string payload;
    int request_id;

    if (timeout_ms <= 0) {
        if (error_message != nullptr) {
            *error_message = "timeout_ms must be greater than 0";
        }
        return false;
    }

    if (!protocol_) {
        if (error_message != nullptr) {
            *error_message = "Protocol is not initialized";
        }
        return false;
    }

    request_id = next_mcp_call_id_.fetch_add(1);
    pending_call = std::make_shared<PendingMcpCall>();
    {
        std::lock_guard<std::mutex> lock(pending_mcp_calls_mutex_);
        pending_mcp_calls_[request_id] = pending_call;
    }

    payload = "{\"jsonrpc\":\"2.0\",\"id\":" + std::to_string(request_id) +
        ",\"method\":\"tools/call\",\"params\":{\"name\":\"" + EscapeJsonString(tool_name) +
        "\",\"arguments\":" + (arguments_json.empty() ? "{}" : arguments_json) + "}}";
    SendMcpMessage(payload);

    {
        std::unique_lock<std::mutex> lock(pending_call->mutex);
        if (!pending_call->condition.wait_for(lock, std::chrono::milliseconds(timeout_ms), [&pending_call]() {
            return pending_call->completed;
        })) {
            lock.unlock();
            std::lock_guard<std::mutex> map_lock(pending_mcp_calls_mutex_);
            pending_mcp_calls_.erase(request_id);
            if (error_message != nullptr) {
                *error_message = "Timed out waiting for MCP tool result";
            }
            return false;
        }

        if (result_json != nullptr) {
            *result_json = pending_call->result_json;
        }
        if (error_message != nullptr) {
            *error_message = pending_call->error_message;
        }
    }

    {
        std::lock_guard<std::mutex> lock(pending_mcp_calls_mutex_);
        pending_mcp_calls_.erase(request_id);
    }

    return pending_call->success;
}

bool Application::SendMcpToolCallAsync(const std::string& tool_name, const std::string& arguments_json,
    std::string* error_message) {
    cJSON* root = nullptr;
    cJSON* params = nullptr;
    cJSON* arguments = nullptr;
    char* payload_text = nullptr;
    bool ok = false;
    int request_id = 0;

    if (!protocol_) {
        if (error_message != nullptr) {
            *error_message = "Protocol is not initialized";
        }
        return false;
    }

    request_id = next_mcp_call_id_.fetch_add(1);

    root = cJSON_CreateObject();
    params = cJSON_CreateObject();
    if ((root == nullptr) || (params == nullptr)) {
        if (error_message != nullptr) {
            *error_message = "Failed to allocate MCP notification JSON";
        }
        goto cleanup;
    }

    arguments = arguments_json.empty() ? cJSON_CreateObject() : cJSON_Parse(arguments_json.c_str());
    if (arguments == nullptr) {
        if (error_message != nullptr) {
            *error_message = "Failed to parse MCP tool arguments JSON";
        }
        goto cleanup;
    }

    cJSON_AddStringToObject(root, "jsonrpc", "2.0");
    cJSON_AddNumberToObject(root, "id", request_id);
    cJSON_AddStringToObject(root, "method", "tools/call");
    cJSON_AddStringToObject(params, "name", tool_name.c_str());
    cJSON_AddItemToObject(params, "arguments", arguments);
    arguments = nullptr;
    cJSON_AddItemToObject(root, "params", params);
    params = nullptr;

    payload_text = cJSON_PrintUnformatted(root);
    if (payload_text == nullptr) {
        if (error_message != nullptr) {
            *error_message = "Failed to serialize MCP notification JSON";
        }
        goto cleanup;
    }

    try {
        {
            std::lock_guard<std::mutex> lock(pending_mcp_calls_mutex_);
            async_mcp_call_ids_.insert(request_id);
        }
        SendMcpMessage(payload_text);
        ok = true;
    } catch (const std::exception& exception) {
        std::lock_guard<std::mutex> lock(pending_mcp_calls_mutex_);
        async_mcp_call_ids_.erase(request_id);
        if (error_message != nullptr) {
            *error_message = exception.what();
        }
    } catch (...) {
        std::lock_guard<std::mutex> lock(pending_mcp_calls_mutex_);
        async_mcp_call_ids_.erase(request_id);
        if (error_message != nullptr) {
            *error_message = "Unexpected MCP notification failure";
        }
    }

cleanup:
    if (payload_text != nullptr) {
        cJSON_free(payload_text);
    }
    if (arguments != nullptr) {
        cJSON_Delete(arguments);
    }
    if (params != nullptr) {
        cJSON_Delete(params);
    }
    if (root != nullptr) {
        cJSON_Delete(root);
    }
    return ok;
}

bool Application::HandlePendingMcpResponse(const cJSON* payload) {
    const cJSON* id = cJSON_GetObjectItem(payload, "id");
    const cJSON* method = cJSON_GetObjectItem(payload, "method");
    const cJSON* result = cJSON_GetObjectItem(payload, "result");
    const cJSON* error = cJSON_GetObjectItem(payload, "error");
    std::shared_ptr<PendingMcpCall> pending_call;
    bool is_async_call = false;

    if (!cJSON_IsNumber(id) || cJSON_IsString(method) || ((result == nullptr) && (error == nullptr))) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(pending_mcp_calls_mutex_);
        auto iter = pending_mcp_calls_.find(id->valueint);
        if (iter == pending_mcp_calls_.end()) {
            auto async_iter = async_mcp_call_ids_.find(id->valueint);
            if (async_iter == async_mcp_call_ids_.end()) {
                return false;
            }
            async_mcp_call_ids_.erase(async_iter);
            is_async_call = true;
        } else {
            pending_call = iter->second;
        }
    }

    if (is_async_call) {
        if (error != nullptr) {
            const cJSON* message = cJSON_GetObjectItem(error, "message");
            if (cJSON_IsString(message) && (message->valuestring != nullptr)) {
                ESP_LOGW(TAG, "Async MCP tool call failed: %s", message->valuestring);
            } else {
                ESP_LOGW(TAG, "Async MCP tool call failed with unknown error");
            }
        } else {
            ESP_LOGI(TAG, "Async MCP tool call response received: id=%d", id->valueint);
        }
        return true;
    }

    {
        std::lock_guard<std::mutex> lock(pending_call->mutex);
        pending_call->completed = true;
        pending_call->success = (error == nullptr);
        if (result != nullptr) {
            char* result_text = cJSON_PrintUnformatted(result);
            pending_call->result_json = result_text != nullptr ? result_text : "{}";
            if (result_text != nullptr) {
                cJSON_free(result_text);
            }
        }
        if (error != nullptr) {
            const cJSON* message = cJSON_GetObjectItem(error, "message");
            if (cJSON_IsString(message)) {
                pending_call->error_message = message->valuestring;
            } else {
                char* error_text = cJSON_PrintUnformatted(error);
                pending_call->error_message = error_text != nullptr ? error_text : "Unknown MCP error";
                if (error_text != nullptr) {
                    cJSON_free(error_text);
                }
            }
        }
    }

    pending_call->condition.notify_all();
    return true;
}

void Application::SendMcpMessage(const std::string& payload) {
    // Always schedule to run in main task for thread safety
    Schedule([this, payload = std::move(payload)]() {
        if (protocol_) {
            protocol_->SendMcpMessage(payload);
        }
    });
}

void Application::SendCustomMessage(const std::string& payload) {
    Schedule([this, payload = std::move(payload)]() {
        if (protocol_) {
            protocol_->SendCustomMessage(payload);
        }
    });
}

void Application::SendColorDebugAnalyze(const std::string& transcript, const std::string& source) {
    if (transcript.empty()) {
        return;
    }

    Schedule([transcript]() {
        auto* display = Board::GetInstance().GetDisplay();

        if (display != nullptr) {
            display->SetChatMessage("user", transcript.c_str());
            display->ShowNotification("Touch debug request queued", 1500);
        }
    });

    SendCustomMessage(BuildColorAnalyzePayload(transcript, source));
}

void Application::SendMatrixPatternRequest(const std::string& transcript, const std::string& source) {
    if (transcript.empty()) {
        return;
    }

    Schedule([transcript]() {
        auto* display = Board::GetInstance().GetDisplay();

        if (display != nullptr) {
            display->SetChatMessage("user", transcript.c_str());
            display->ShowNotification("Matrix draw request queued", 1500);
        }
    });
    SendCustomMessage(BuildMatrixPatternRequestPayload(transcript, source));
}

void Application::SetAecMode(AecMode mode) {
    aec_mode_ = mode;
    Schedule([this]() {
        auto& board = Board::GetInstance();
        auto display = board.GetDisplay();
        switch (aec_mode_) {
        case kAecOff:
            audio_service_.EnableDeviceAec(false);
            display->ShowNotification(Lang::Strings::RTC_MODE_OFF);
            break;
        case kAecOnServerSide:
            audio_service_.EnableDeviceAec(false);
            display->ShowNotification(Lang::Strings::RTC_MODE_ON);
            break;
        case kAecOnDeviceSide:
            audio_service_.EnableDeviceAec(true);
            display->ShowNotification(Lang::Strings::RTC_MODE_ON);
            break;
        }

        // If the AEC mode is changed, close the audio channel
        if (protocol_ && protocol_->IsAudioChannelOpened()) {
            protocol_->CloseAudioChannel();
        }
    });
}

void Application::PlaySound(const std::string_view& sound) {
    audio_service_.PlaySound(sound);
}

void Application::ResetProtocol() {
    Schedule([this]() {
        // Close audio channel if opened
        if (protocol_ && protocol_->IsAudioChannelOpened()) {
            protocol_->CloseAudioChannel();
        }
        // Reset protocol
        protocol_.reset();
    });
}


#include "gp_matrix_local_tools.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <optional>
#include <string>

#include "display/display.h"
#include "mcp_server.h"
#include "gp_led_matrix_esp32.h"
#include "ui/gp_debug_display.h"

namespace {

std::string ToAsciiLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

std::optional<uint32_t> ParseRgb888(const std::string& text) {
    unsigned int rgb = 0;
    if (std::sscanf(text.c_str(), "#%06x", &rgb) == 1
        || std::sscanf(text.c_str(), "0x%06x", &rgb) == 1
        || std::sscanf(text.c_str(), "%06x", &rgb) == 1) {
        return static_cast<uint32_t>(rgb & 0xFFFFFFU);
    }
    return std::nullopt;
}

std::optional<std::array<uint16_t, GP_MATRIX_HEIGHT>> ParseMatrixBitmapRowsHex(const std::string& text) {
    std::array<uint16_t, GP_MATRIX_HEIGHT> rows = {};
    std::string hex_digits;

    hex_digits.reserve(text.size());
    for (char ch : text) {
        if (std::isxdigit(static_cast<unsigned char>(ch)) != 0) {
            hex_digits.push_back(ch);
        }
    }

    if (hex_digits.size() != (GP_MATRIX_HEIGHT * 4U)) {
        return std::nullopt;
    }

    for (size_t index = 0; index < rows.size(); ++index) {
        unsigned int value = 0;

        if (std::sscanf(hex_digits.substr(index * 4U, 4U).c_str(), "%4x", &value) != 1) {
            return std::nullopt;
        }
        rows[index] = static_cast<uint16_t>(value & 0xFFFFU);
    }

    return rows;
}

std::string EncodeMatrixBitmapRowsHex(const std::array<uint16_t, GP_MATRIX_HEIGHT>& rows) {
    std::string result;
    char row_hex[5] = {0};

    result.reserve(GP_MATRIX_HEIGHT * 4U);
    for (size_t row_index = 0; row_index < rows.size(); ++row_index) {
        std::snprintf(row_hex, sizeof(row_hex), "%04X", static_cast<unsigned int>(rows[row_index]));
        result.append(row_hex);
    }
    return result;
}

void SetMatrixPixel(std::array<uint16_t, GP_MATRIX_HEIGHT>& rows, int x, int y, bool on) {
    uint16_t mask;

    if ((x < 0) || (x >= static_cast<int>(GP_MATRIX_WIDTH))
        || (y < 0) || (y >= static_cast<int>(GP_MATRIX_HEIGHT))) {
        return;
    }

    mask = static_cast<uint16_t>(1U << (GP_MATRIX_WIDTH - 1U - static_cast<size_t>(x)));
    if (on) {
        rows[static_cast<size_t>(y)] |= mask;
    } else {
        rows[static_cast<size_t>(y)] &= static_cast<uint16_t>(~mask);
    }
}

void DrawMatrixLine(std::array<uint16_t, GP_MATRIX_HEIGHT>& rows, int x0, int y0, int x1, int y1) {
    const int dx = std::abs(x1 - x0);
    const int sx = (x0 < x1) ? 1 : -1;
    const int dy = -std::abs(y1 - y0);
    const int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true) {
        SetMatrixPixel(rows, x0, y0, true);
        if ((x0 == x1) && (y0 == y1)) {
            break;
        }

        const int e2 = err * 2;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void DrawMatrixRect(std::array<uint16_t, GP_MATRIX_HEIGHT>& rows,
                    int x,
                    int y,
                    int width,
                    int height,
                    bool filled) {
    const int x_end = x + width - 1;
    const int y_end = y + height - 1;

    if ((width <= 0) || (height <= 0)) {
        return;
    }

    if (filled) {
        for (int py = y; py <= y_end; ++py) {
            for (int px = x; px <= x_end; ++px) {
                SetMatrixPixel(rows, px, py, true);
            }
        }
        return;
    }

    DrawMatrixLine(rows, x, y, x_end, y);
    DrawMatrixLine(rows, x, y_end, x_end, y_end);
    DrawMatrixLine(rows, x, y, x, y_end);
    DrawMatrixLine(rows, x_end, y, x_end, y_end);
}

bool BuildMatrixPattern(const std::string& pattern_name,
                        std::array<uint16_t, GP_MATRIX_HEIGHT>* rows,
                        std::string* normalized_name) {
    const std::string lowered = ToAsciiLower(pattern_name);
    size_t x;
    size_t y;

    if ((rows == nullptr) || (normalized_name == nullptr)) {
        return false;
    }

    rows->fill(0U);
    if (lowered == "cross" || lowered == "x") {
        for (y = 0U; y < GP_MATRIX_HEIGHT; ++y) {
            SetMatrixPixel(*rows, static_cast<int>(y), static_cast<int>(y), true);
            SetMatrixPixel(*rows,
                           static_cast<int>(GP_MATRIX_WIDTH - 1U - y),
                           static_cast<int>(y),
                           true);
        }
        *normalized_name = "cross";
        return true;
    }

    if (lowered == "plus") {
        for (x = 0U; x < GP_MATRIX_WIDTH; ++x) {
            SetMatrixPixel(*rows, static_cast<int>(x), 7, true);
            SetMatrixPixel(*rows, static_cast<int>(x), 8, true);
        }
        for (y = 0U; y < GP_MATRIX_HEIGHT; ++y) {
            SetMatrixPixel(*rows, 7, static_cast<int>(y), true);
            SetMatrixPixel(*rows, 8, static_cast<int>(y), true);
        }
        *normalized_name = "plus";
        return true;
    }

    if (lowered == "border" || lowered == "frame") {
        DrawMatrixRect(*rows, 0, 0, GP_MATRIX_WIDTH, GP_MATRIX_HEIGHT, false);
        *normalized_name = "border";
        return true;
    }

    if (lowered == "checker" || lowered == "checkerboard") {
        for (y = 0U; y < GP_MATRIX_HEIGHT; ++y) {
            for (x = 0U; x < GP_MATRIX_WIDTH; ++x) {
                if (((x + y) & 0x1U) == 0U) {
                    SetMatrixPixel(*rows, static_cast<int>(x), static_cast<int>(y), true);
                }
            }
        }
        *normalized_name = "checker";
        return true;
    }

    if (lowered == "diamond") {
        for (y = 0U; y < GP_MATRIX_HEIGHT; ++y) {
            const int distance = std::abs(static_cast<int>(y) - 7);
            const int left = distance;
            const int right = static_cast<int>(GP_MATRIX_WIDTH - 1U - static_cast<size_t>(distance));
            SetMatrixPixel(*rows, left, static_cast<int>(y), true);
            SetMatrixPixel(*rows, right, static_cast<int>(y), true);
        }
        *normalized_name = "diamond";
        return true;
    }

    return false;
}

cJSON* BuildMatrixFrameResultJson(const char* preset,
                                  const char* frame_hex,
                                  const char* bitmap_rows_hex,
                                  const char* primary_rgb888,
                                  bool applied,
                                  const char* source,
                                  const char* transcript) {
    cJSON* json = cJSON_CreateObject();

    cJSON_AddStringToObject(json, "preset", preset != nullptr ? preset : "");
    cJSON_AddStringToObject(json, "frame_rgb332_hex", frame_hex != nullptr ? frame_hex : "");
    cJSON_AddStringToObject(json, "bitmap_rows_hex", bitmap_rows_hex != nullptr ? bitmap_rows_hex : "");
    cJSON_AddStringToObject(json, "primary_rgb888", primary_rgb888 != nullptr ? primary_rgb888 : "");
    cJSON_AddNumberToObject(json, "width", GP_MATRIX_WIDTH);
    cJSON_AddNumberToObject(json, "height", GP_MATRIX_HEIGHT);
    cJSON_AddBoolToObject(json, "applied", applied);
    cJSON_AddStringToObject(json, "source", source != nullptr ? source : "mcp");
    cJSON_AddStringToObject(json, "transcript", transcript != nullptr ? transcript : "");
    return json;
}

bool SendBitmapFrame(GpLedMatrixEsp32* matrix_led,
                     GpDebugLcdDisplay* debug_display,
                     const std::array<uint16_t, GP_MATRIX_HEIGHT>& rows,
                     uint32_t primary_rgb888,
                     uint32_t background_rgb888) {
    if (matrix_led == nullptr) {
        return false;
    }

    if (debug_display != nullptr) {
        debug_display->ApplyMatrixBitmapPreview(rows, primary_rgb888, background_rgb888);
    }

    return matrix_led->ShowBitmapFrame(rows.data(),
                                       rows.size(),
                                       primary_rgb888,
                                       background_rgb888,
                                       kGpMatrixModeSolidFrame);
}

}  // namespace

void RegisterGpMatrixLocalMcpTools(McpServer& server,
                                   GpLedMatrixEsp32* matrix_led,
                                   Display* display) {
    auto* debug_display = dynamic_cast<GpDebugLcdDisplay*>(display);

    auto register_matrix_frame_tool = [matrix_led, debug_display, &server](const char* tool_name) {
        server.AddTool(tool_name,
            "Draw one 16x16 matrix frame on the LED side via layered bitmap format. "
            "Requires bitmap_rows_hex (64 hex chars, 16x16 bitmap) plus primary_rgb888 and optional background_rgb888.",
            PropertyList({
                Property("bitmap_rows_hex", kPropertyTypeString, std::string("")),
                Property("primary_rgb888", kPropertyTypeString, std::string("")),
                Property("background_rgb888", kPropertyTypeString, std::string("#000000")),
                Property("source", kPropertyTypeString, std::string("mcp")),
                Property("transcript", kPropertyTypeString, std::string(""))
            }),
            [matrix_led, debug_display](const PropertyList& properties) -> ReturnValue {
                const std::string bitmap_rows_hex = properties["bitmap_rows_hex"].value<std::string>();
                const std::string source = properties["source"].value<std::string>();
                const std::string transcript = properties["transcript"].value<std::string>();
                const auto bitmap_rows = ParseMatrixBitmapRowsHex(bitmap_rows_hex);
                const auto primary_rgb = ParseRgb888(properties["primary_rgb888"].value<std::string>());
                const auto background_rgb = ParseRgb888(properties["background_rgb888"].value<std::string>());
                const uint32_t resolved_background_rgb = background_rgb.value_or(0x000000U);
                std::string encoded_rows;
                char rgb_text[16] = {0};
                bool applied;

                if (matrix_led == nullptr) {
                    throw std::runtime_error("LED matrix transport is not initialized");
                }
                if (bitmap_rows_hex.empty()) {
                    throw std::runtime_error("bitmap_rows_hex is required");
                }
                if (!bitmap_rows.has_value()) {
                    throw std::runtime_error("bitmap_rows_hex must contain exactly 16 rows encoded as 64 hex characters");
                }
                if (!primary_rgb.has_value()) {
                    throw std::runtime_error("primary_rgb888 must be a RGB888 string like #RRGGBB");
                }

                applied = SendBitmapFrame(matrix_led,
                                          debug_display,
                                          *bitmap_rows,
                                          *primary_rgb,
                                          resolved_background_rgb);
                if (!applied) {
                    throw std::runtime_error("16x16 frame draw failed");
                }

                encoded_rows = EncodeMatrixBitmapRowsHex(*bitmap_rows);
                std::snprintf(rgb_text,
                              sizeof(rgb_text),
                              "#%06X",
                              static_cast<unsigned int>(*primary_rgb & 0xFFFFFFU));
                return BuildMatrixFrameResultJson("",
                                                  "",
                                                  encoded_rows.c_str(),
                                                  rgb_text,
                                                  applied,
                                                  source.c_str(),
                                                  transcript.c_str());
            });
    };

    register_matrix_frame_tool("self.screen.matrix_16x16.draw_frame");
    register_matrix_frame_tool("self.screen.matrix_16x16.draw");

    server.AddTool("self.screen.matrix_16x16.local.pattern",
        "Draw a built-in local 16x16 pattern directly on AI side without external script. "
        "Supported patterns: cross, plus, border, checker, diamond.",
        PropertyList({
            Property("pattern", kPropertyTypeString),
            Property("primary_rgb888", kPropertyTypeString, std::string("#00FF66")),
            Property("background_rgb888", kPropertyTypeString, std::string("#000000")),
            Property("source", kPropertyTypeString, std::string("local_tool")),
            Property("transcript", kPropertyTypeString, std::string(""))
        }),
        [matrix_led, debug_display](const PropertyList& properties) -> ReturnValue {
            const std::string pattern_text = properties["pattern"].value<std::string>();
            const auto primary_rgb = ParseRgb888(properties["primary_rgb888"].value<std::string>());
            const auto background_rgb = ParseRgb888(properties["background_rgb888"].value<std::string>());
            std::array<uint16_t, GP_MATRIX_HEIGHT> rows = {};
            std::string normalized_pattern;
            std::string encoded_rows;
            char rgb_text[16] = {0};

            if (!primary_rgb.has_value()) {
                throw std::runtime_error("primary_rgb888 must be a RGB888 string like #RRGGBB");
            }
            if (!background_rgb.has_value()) {
                throw std::runtime_error("background_rgb888 must be a RGB888 string like #RRGGBB");
            }
            if (!BuildMatrixPattern(pattern_text, &rows, &normalized_pattern)) {
                throw std::runtime_error("pattern must be one of: cross, plus, border, checker, diamond");
            }
            if (!SendBitmapFrame(matrix_led, debug_display, rows, *primary_rgb, *background_rgb)) {
                throw std::runtime_error("16x16 local draw failed");
            }

            encoded_rows = EncodeMatrixBitmapRowsHex(rows);
            std::snprintf(rgb_text,
                          sizeof(rgb_text),
                          "#%06X",
                          static_cast<unsigned int>(*primary_rgb & 0xFFFFFFU));
            return BuildMatrixFrameResultJson(normalized_pattern.c_str(),
                                              "",
                                              encoded_rows.c_str(),
                                              rgb_text,
                                              true,
                                              properties["source"].value<std::string>().c_str(),
                                              properties["transcript"].value<std::string>().c_str());
        });

    server.AddTool("self.screen.matrix_16x16.local.line",
        "Draw a 1-pixel line in a local 16x16 bitmap on AI side and send it to LED side.",
        PropertyList({
            Property("x0", kPropertyTypeInteger, 0, 15),
            Property("y0", kPropertyTypeInteger, 0, 15),
            Property("x1", kPropertyTypeInteger, 0, 15),
            Property("y1", kPropertyTypeInteger, 0, 15),
            Property("primary_rgb888", kPropertyTypeString, std::string("#FFFFFF")),
            Property("background_rgb888", kPropertyTypeString, std::string("#000000")),
            Property("source", kPropertyTypeString, std::string("local_tool")),
            Property("transcript", kPropertyTypeString, std::string(""))
        }),
        [matrix_led, debug_display](const PropertyList& properties) -> ReturnValue {
            std::array<uint16_t, GP_MATRIX_HEIGHT> rows = {};
            const auto primary_rgb = ParseRgb888(properties["primary_rgb888"].value<std::string>());
            const auto background_rgb = ParseRgb888(properties["background_rgb888"].value<std::string>());
            std::string encoded_rows;
            char rgb_text[16] = {0};

            if (!primary_rgb.has_value()) {
                throw std::runtime_error("primary_rgb888 must be a RGB888 string like #RRGGBB");
            }
            if (!background_rgb.has_value()) {
                throw std::runtime_error("background_rgb888 must be a RGB888 string like #RRGGBB");
            }

            DrawMatrixLine(rows,
                           properties["x0"].value<int>(),
                           properties["y0"].value<int>(),
                           properties["x1"].value<int>(),
                           properties["y1"].value<int>());
            if (!SendBitmapFrame(matrix_led, debug_display, rows, *primary_rgb, *background_rgb)) {
                throw std::runtime_error("16x16 local draw failed");
            }

            encoded_rows = EncodeMatrixBitmapRowsHex(rows);
            std::snprintf(rgb_text,
                          sizeof(rgb_text),
                          "#%06X",
                          static_cast<unsigned int>(*primary_rgb & 0xFFFFFFU));
            return BuildMatrixFrameResultJson("line",
                                              "",
                                              encoded_rows.c_str(),
                                              rgb_text,
                                              true,
                                              properties["source"].value<std::string>().c_str(),
                                              properties["transcript"].value<std::string>().c_str());
        });

    server.AddTool("self.screen.matrix_16x16.local.rect",
        "Draw one local rectangle in a 16x16 bitmap on AI side and send it to LED side. "
        "Set filled=true for solid rectangle.",
        PropertyList({
            Property("x", kPropertyTypeInteger, 0, 15),
            Property("y", kPropertyTypeInteger, 0, 15),
            Property("width", kPropertyTypeInteger, 1, 16),
            Property("height", kPropertyTypeInteger, 1, 16),
            Property("filled", kPropertyTypeBoolean, false),
            Property("primary_rgb888", kPropertyTypeString, std::string("#FFFFFF")),
            Property("background_rgb888", kPropertyTypeString, std::string("#000000")),
            Property("source", kPropertyTypeString, std::string("local_tool")),
            Property("transcript", kPropertyTypeString, std::string(""))
        }),
        [matrix_led, debug_display](const PropertyList& properties) -> ReturnValue {
            std::array<uint16_t, GP_MATRIX_HEIGHT> rows = {};
            const auto primary_rgb = ParseRgb888(properties["primary_rgb888"].value<std::string>());
            const auto background_rgb = ParseRgb888(properties["background_rgb888"].value<std::string>());
            std::string encoded_rows;
            char rgb_text[16] = {0};

            if (!primary_rgb.has_value()) {
                throw std::runtime_error("primary_rgb888 must be a RGB888 string like #RRGGBB");
            }
            if (!background_rgb.has_value()) {
                throw std::runtime_error("background_rgb888 must be a RGB888 string like #RRGGBB");
            }

            DrawMatrixRect(rows,
                           properties["x"].value<int>(),
                           properties["y"].value<int>(),
                           properties["width"].value<int>(),
                           properties["height"].value<int>(),
                           properties["filled"].value<bool>());
            if (!SendBitmapFrame(matrix_led, debug_display, rows, *primary_rgb, *background_rgb)) {
                throw std::runtime_error("16x16 local draw failed");
            }

            encoded_rows = EncodeMatrixBitmapRowsHex(rows);
            std::snprintf(rgb_text,
                          sizeof(rgb_text),
                          "#%06X",
                          static_cast<unsigned int>(*primary_rgb & 0xFFFFFFU));
            return BuildMatrixFrameResultJson("rect",
                                              "",
                                              encoded_rows.c_str(),
                                              rgb_text,
                                              true,
                                              properties["source"].value<std::string>().c_str(),
                                              properties["transcript"].value<std::string>().c_str());
        });
}

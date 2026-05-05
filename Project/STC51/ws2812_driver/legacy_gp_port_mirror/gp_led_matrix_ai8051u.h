#ifndef GP_LED_MATRIX_AI8051U_H_
#define GP_LED_MATRIX_AI8051U_H_

#include "AI8051U.H"
#include "ai8051u_def.h"
#include "gp_led_matrix_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GpMatrixAi8051uContext {
    unsigned char rx_buffer[GP_MATRIX_RGB332_FRAME_SIZE + 32U];
    unsigned char tx_buffer[32U];
    unsigned char frame_buffer[GP_MATRIX_RGB332_FRAME_SIZE];
    unsigned char brightness;
    unsigned char sequence;
    unsigned char dirty;
    unsigned char mode;
    unsigned int expected_bytes;
    unsigned int received_bytes;
} GpMatrixAi8051uContext;

void GpMatrixAi8051u_Init(GpMatrixAi8051uContext* context, unsigned char i2c_address);
void GpMatrixAi8051u_OnI2cReceive(GpMatrixAi8051uContext* context, const unsigned char* data, unsigned char length);
unsigned char GpMatrixAi8051u_PrepareTx(GpMatrixAi8051uContext* context, unsigned char* out_data, unsigned char max_length);
void GpMatrixAi8051u_Poll(GpMatrixAi8051uContext* context);
void GpMatrixAi8051u_RenderFrame(const GpMatrixAi8051uContext* context);
void GpMatrixAi8051u_LoadGlyphRows(GpMatrixAi8051uContext* context, const unsigned int* rows, unsigned char glyph_count, unsigned char glyph_width, unsigned char glyph_spacing);

#ifdef __cplusplus
}
#endif

#endif
#ifndef __GP_LED_MATRIX_AI8051U_H__
#define __GP_LED_MATRIX_AI8051U_H__

#include "AI8051U.H"
#include "ai8051u_def.h"
#include "gp_led_matrix_protocol.h"

#define GP_MATRIX_AI8051U_RX_BUFFER_SIZE (GP_MATRIX_PACKET_HEADER_SIZE + GP_MATRIX_MAX_CHUNK_DATA + 8U)
#define GP_MATRIX_AI8051U_TX_BUFFER_SIZE 32U

typedef struct
{
    uint8_t rxBuffer[GP_MATRIX_AI8051U_RX_BUFFER_SIZE];
    uint8_t txBuffer[GP_MATRIX_AI8051U_TX_BUFFER_SIZE];
    uint8_t frameBuffer[GP_MATRIX_RGB332_FRAME_SIZE];
    uint8_t glyphBuffer[GP_MATRIX_MAX_GLYPH_TRANSFER_BYTES];
    uint8_t i2cAddress;
    uint8_t brightness;
    uint8_t mode;
    uint8_t lastSequence;
    uint8_t lastCommand;
    uint8_t lastStatus;
    uint8_t packetLength;
    uint8_t packetPending;
    uint8_t packetReplyPrepared;
    uint8_t txLength;
    uint8_t txPending;
    uint8_t dmaRxEnabled;
    uint8_t dmaTxEnabled;
    uint8_t dmaRxActive;
    uint8_t dmaTxActive;
    uint8_t glyphCount;
    uint8_t glyphWidth;
    uint8_t glyphSpacing;
    uint16_t expectedBytes;
    uint16_t receivedBytes;
    uint16_t glyphExpectedBytes;
    uint16_t glyphReceivedBytes;
    uint16_t dmaLastRxDone;
    uint16_t dmaLastTxDone;
} GpLedMatrixAi8051uContext;

void GpLedMatrixAi8051u_Init(GpLedMatrixAi8051uContext xdata *context, uint8_t i2cAddress);
void GpLedMatrixAi8051u_SetDmaMode(GpLedMatrixAi8051uContext xdata *context, uint8_t enableRx, uint8_t enableTx);
void GpLedMatrixAi8051u_OnI2cReceive(GpLedMatrixAi8051uContext xdata *context, const uint8_t *rxBytes, uint8_t length);
uint8_t GpLedMatrixAi8051u_PrepareTx(GpLedMatrixAi8051uContext xdata *context, uint8_t *outData, uint8_t maxLength);
void GpLedMatrixAi8051u_Poll(GpLedMatrixAi8051uContext xdata *context);
void GpLedMatrixAi8051u_RenderFrame(GpLedMatrixAi8051uContext xdata *context);
void GpLedMatrixAi8051u_LoadGlyphRows(GpLedMatrixAi8051uContext xdata *context,
                                      const uint16_t *rows,
                                      uint8_t glyphCount,
                                      uint8_t glyphWidth,
                                      uint8_t glyphSpacing);

#endif
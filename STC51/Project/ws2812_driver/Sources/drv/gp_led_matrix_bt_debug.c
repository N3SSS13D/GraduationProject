/*
 * @file gp_led_matrix_bt_debug.c
 * @brief Minimal USB-to-HC05 bridge for UART2 and AT-mode setup.
 */

#include "config.h"
#include "gp_led_matrix_bt_debug.h"
#include "port.h"

#define GP_MATRIX_BT_COMMAND_BUFFER_SIZE 64U
#define GP_MATRIX_BT_REPLY_BUFFER_SIZE   96U
#define GP_MATRIX_BT_HEX_BUFFER_SIZE     (GP_MATRIX_BT_REPLY_BUFFER_SIZE * 3U + 1U)
#define GP_MATRIX_BT_SNIFF_BUFFER_SIZE   48U
#define GP_MATRIX_BT_REPLY_TIMEOUT_MS    800U
#define GP_MATRIX_BT_REPLY_IDLE_MS       12U
#define GP_MATRIX_BT_TASK_IDLE_TICKS     2U
#define GP_MATRIX_BT_BAUD_SWITCH_DELAY_MS 120U
#define GP_MATRIX_BT_COMMAND_GAP_MS      80U

static char xdata g_gpMatrixBtCommand[GP_MATRIX_BT_COMMAND_BUFFER_SIZE];
static char xdata g_gpMatrixBtReply[GP_MATRIX_BT_REPLY_BUFFER_SIZE];
static char xdata g_gpMatrixBtAscii[GP_MATRIX_BT_REPLY_BUFFER_SIZE];
static char xdata g_gpMatrixBtHex[GP_MATRIX_BT_HEX_BUFFER_SIZE];
static uint8_t xdata g_gpMatrixBtSniff[GP_MATRIX_BT_SNIFF_BUFFER_SIZE];
static uint8_t xdata g_gpMatrixBtLastTxOk = 0U;
static uint8_t xdata g_gpMatrixBtLastRxBytes = 0U;
static uint8_t xdata g_gpMatrixBtReady = 0U;
static uint16_t xdata g_gpMatrixBtLastTotalRx = 0U;
static uint8_t xdata g_gpMatrixBtRxIdleTicks = 0U;

static uint8_t GpLedMatrixBtDebug_IsSpace(uint8_t value);
static const char *GpLedMatrixBtDebug_SkipSpace(const char *text);
static uint8_t GpLedMatrixBtDebug_MatchKeyword(const char *text, const char *keyword);
static uint8_t GpLedMatrixBtDebug_CopyText(char *buffer, uint8_t bufferSize, const char *text, uint8_t reserveLength);
static uint8_t GpLedMatrixBtDebug_AppendCrLf(char *buffer, uint8_t bufferSize, uint8_t textLength);
static uint8_t GpLedMatrixBtDebug_IsLedDigit(const char *text);
static void GpLedMatrixBtDebug_TrimTrailingSpace(char *text);
static uint8_t GpLedMatrixBtDebug_IsAtCommand(const char *text);
static uint8_t GpLedMatrixBtDebug_ParseTargetBaud(const char *text, uint32_t *baudrate);
static void GpLedMatrixBtDebug_PrintUsage(void);
static void GpLedMatrixBtDebug_PrintStatus(const char *tag);
static void GpLedMatrixBtDebug_FormatAscii(const char *replyBuffer, uint8_t replyLength);
static void GpLedMatrixBtDebug_FormatHex(const char *replyBuffer, uint8_t replyLength);
static uint8_t GpLedMatrixBtDebug_ReplyContainsOk(const char *replyBuffer, uint8_t replyLength);
static void GpLedMatrixBtDebug_HandleReceivedText(char *text);
static uint8_t GpLedMatrixBtDebug_ReadReply(char *replyBuffer, uint8_t maxLength, uint16_t timeoutMs);
static void GpLedMatrixBtDebug_SendText(const char *payload);
static void GpLedMatrixBtDebug_RunAutoSetup(void);

static uint8_t GpLedMatrixBtDebug_IsSpace(uint8_t value)
{
    if ((value == ' ') || (value == '\r') || (value == '\n') || (value == '\t'))
    {
        return 1U;
    }

    return 0U;
}

static const char *GpLedMatrixBtDebug_SkipSpace(const char *text)
{
    while ((*text != '\0') && (GpLedMatrixBtDebug_IsSpace((uint8_t)(*text)) != 0U))
    {
        text++;
    }

    return text;
}

static uint8_t GpLedMatrixBtDebug_MatchKeyword(const char *text, const char *keyword)
{
    while ((*text != '\0') && (*keyword != '\0'))
    {
        if (*text != *keyword)
        {
            return 0U;
        }

        text++;
        keyword++;
    }

    if (*keyword != '\0')
    {
        return 0U;
    }

    if ((*text == '\0') || (GpLedMatrixBtDebug_IsSpace((uint8_t)(*text)) != 0U))
    {
        return 1U;
    }

    return 0U;
}

static uint8_t GpLedMatrixBtDebug_CopyText(char *buffer, uint8_t bufferSize, const char *text, uint8_t reserveLength)
{
    uint8_t textLength;

    if ((buffer == 0) || (text == 0) || (bufferSize <= (uint8_t)(reserveLength + 1U)))
    {
        return 0U;
    }

    textLength = 0U;
    while ((text[textLength] != '\0')
           && (textLength < (uint8_t)(bufferSize - reserveLength - 1U)))
    {
        buffer[textLength] = text[textLength];
        textLength++;
    }

    buffer[textLength] = '\0';
    return textLength;
}

static uint8_t GpLedMatrixBtDebug_AppendCrLf(char *buffer, uint8_t bufferSize, uint8_t textLength)
{
    if ((buffer == 0) || (textLength >= bufferSize))
    {
        return textLength;
    }

    if ((textLength >= 1U) && (buffer[textLength - 1U] == '\n'))
    {
        return textLength;
    }

    if ((textLength + 2U) >= bufferSize)
    {
        return textLength;
    }

    buffer[textLength] = '\r';
    textLength++;
    buffer[textLength] = '\n';
    textLength++;
    buffer[textLength] = '\0';

    return textLength;
}

static uint8_t GpLedMatrixBtDebug_IsLedDigit(const char *text)
{
    if ((text[0] >= '0') && (text[0] <= '7') && (text[1] == '\0'))
    {
        return 1U;
    }

    return 0U;
}

static void GpLedMatrixBtDebug_TrimTrailingSpace(char *text)
{
    uint8_t textLength;

    textLength = 0U;
    while (text[textLength] != '\0')
    {
        textLength++;
    }

    while (textLength > 0U)
    {
        if (GpLedMatrixBtDebug_IsSpace((uint8_t)text[textLength - 1U]) == 0U)
        {
            break;
        }

        textLength--;
        text[textLength] = '\0';
    }
}

static uint8_t GpLedMatrixBtDebug_IsAtCommand(const char *text)
{
    return (uint8_t)((text[0] == 'A') && (text[1] == 'T'));
}

static uint8_t GpLedMatrixBtDebug_ParseTargetBaud(const char *text, uint32_t *baudrate)
{
    uint32_t value;
    uint8_t index;

    if ((baudrate == 0)
        || (text[0] != 'A')
        || (text[1] != 'T')
        || (text[2] != '+')
        || (text[3] != 'U')
        || (text[4] != 'A')
        || (text[5] != 'R')
        || (text[6] != 'T')
        || (text[7] != '='))
    {
        return 0U;
    }

    value = 0UL;
    index = 8U;
    if ((text[index] < '0') || (text[index] > '9'))
    {
        return 0U;
    }

    while ((text[index] >= '0') && (text[index] <= '9'))
    {
        value = value * 10UL + (uint32_t)(text[index] - '0');
        index++;
    }

    if ((text[index] != ',') || (value == 0UL))
    {
        return 0U;
    }

    *baudrate = value;
    return 1U;
}

static void GpLedMatrixBtDebug_PrintUsage(void)
{
    printf("[BT_HELP] BT SEND <text>\r\n");
    printf("[BT_HELP] BT STATUS\r\n");
    printf("[BT_HELP] startup auto setup uses 38400 -> role/name/pswd -> 115200\r\n");
    printf("[BT_HELP] BT AT+UART=115200,0,0\r\n");
}

static void GpLedMatrixBtDebug_PrintStatus(const char *tag)
{
    printf("[BT_STA] tag=%s baud=%lu at=%u tx=%u rx=%u ovf=%u total=%u\r\n",
           tag,
           (unsigned long)UART2_GetBaudrate(),
           (unsigned int)UART2_GetBtAtMode(),
           (unsigned int)g_gpMatrixBtLastTxOk,
           (unsigned int)g_gpMatrixBtLastRxBytes,
           (unsigned int)UART2_GetRxOverflow(),
           (unsigned int)UART2_GetRxTotalCount());
}

static void GpLedMatrixBtDebug_FormatAscii(const char *replyBuffer, uint8_t replyLength)
{
    uint8_t index;
    uint8_t displayLength;
    uint8_t asciiChar;

    displayLength = replyLength;
    while (displayLength > 0U)
    {
        asciiChar = (uint8_t)replyBuffer[displayLength - 1U];
        if ((asciiChar != '\r') && (asciiChar != '\n'))
        {
            break;
        }

        displayLength--;
    }

    for (index = 0U; index < displayLength; ++index)
    {
        asciiChar = (uint8_t)replyBuffer[index];
        if ((asciiChar < 0x20U) || (asciiChar > 0x7EU))
        {
            g_gpMatrixBtAscii[index] = '.';
        }
        else
        {
            g_gpMatrixBtAscii[index] = (char)asciiChar;
        }
    }

    g_gpMatrixBtAscii[displayLength] = '\0';
}

static void GpLedMatrixBtDebug_FormatHex(const char *replyBuffer, uint8_t replyLength)
{
    static const char code hexDigits[] = "0123456789ABCDEF";
    uint8_t index;
    uint8_t hexOffset;
    uint8_t replyByte;

    hexOffset = 0U;
    for (index = 0U; index < replyLength; ++index)
    {
        replyByte = (uint8_t)replyBuffer[index];
        g_gpMatrixBtHex[hexOffset] = hexDigits[replyByte >> 4];
        hexOffset++;
        g_gpMatrixBtHex[hexOffset] = hexDigits[replyByte & 0x0FU];
        hexOffset++;
        if (index + 1U < replyLength)
        {
            g_gpMatrixBtHex[hexOffset] = ' ';
            hexOffset++;
        }
    }

    g_gpMatrixBtHex[hexOffset] = '\0';
}

static uint8_t GpLedMatrixBtDebug_ReplyContainsOk(const char *replyBuffer, uint8_t replyLength)
{
    uint8_t index;

    if ((replyBuffer == 0) || (replyLength < 2U))
    {
        return 0U;
    }

    for (index = 0U; index + 1U < replyLength; ++index)
    {
        if ((replyBuffer[index] == 'O') && (replyBuffer[index + 1U] == 'K'))
        {
            return 1U;
        }
    }

    return 0U;
}

static void GpLedMatrixBtDebug_HandleReceivedText(char *text)
{
    const char *payload;

    payload = GpLedMatrixBtDebug_SkipSpace(text);
    if (*payload == '\0')
    {
        return;
    }

    if ((payload[0] != 'L') || (payload[1] != 'E') || (payload[2] != 'D'))
    {
        return;
    }

    payload += 3;
    payload = GpLedMatrixBtDebug_SkipSpace(payload);
    if ((payload[0] == 'C')
        && (payload[1] == 'L')
        && (payload[2] == 'E')
        && (payload[3] == 'A')
        && (payload[4] == 'R')
        && (payload[5] == '\0'))
    {
        PORT2_ClearDebugLeds();
        printf("[BT_ACT] led=clear\r\n");
        return;
    }

    if (GpLedMatrixBtDebug_IsLedDigit(payload) != 0U)
    {
        PORT2_SetDebugLedDigit((uint8_t)(payload[0] - '0'));
        printf("[BT_ACT] led=%c\r\n", payload[0]);
        return;
    }

    printf("[BT_ACT] led_err=%s\r\n", payload);
}

static uint8_t GpLedMatrixBtDebug_ReadReply(char *replyBuffer, uint8_t maxLength, uint16_t timeoutMs)
{
    uint8_t replyLength;
    uint8_t rxByte;
    uint16_t waitMs;
    uint16_t idleMs;
    uint8_t receivedAny;
    uint8_t receivedThisTick;

    if ((replyBuffer == 0) || (maxLength == 0U))
    {
        return 0U;
    }

    replyLength = 0U;
    idleMs = 0U;
    receivedAny = 0U;
    for (waitMs = 0U; waitMs < timeoutMs; ++waitMs)
    {
        receivedThisTick = 0U;
        while (UART2_TryPopByte(&rxByte) != 0U)
        {
            if (replyLength < (uint8_t)(maxLength - 1U))
            {
                replyBuffer[replyLength] = (char)rxByte;
                replyLength++;
            }

            receivedAny = 1U;
            receivedThisTick = 1U;
        }

        if (replyLength >= (uint8_t)(maxLength - 1U))
        {
            break;
        }

        if (receivedAny != 0U)
        {
            if (receivedThisTick != 0U)
            {
                idleMs = 0U;
            }
            else
            {
                idleMs++;
                if (idleMs >= GP_MATRIX_BT_REPLY_IDLE_MS)
                {
                    break;
                }
            }
        }

        delay_ms(1U);
    }

    replyBuffer[replyLength] = '\0';
    return replyLength;
}

static void GpLedMatrixBtDebug_SendText(const char *payload)
{
    uint8_t textLength;
    uint8_t isAtCommand;
    uint32_t nextBaudrate;
    uint8_t hasNextBaudrate;
    uint8_t canSwitchLocalBaud;
    uint8_t resetReplyLength;

    textLength = GpLedMatrixBtDebug_CopyText(g_gpMatrixBtCommand,
                                             GP_MATRIX_BT_COMMAND_BUFFER_SIZE,
                                             payload,
                                             2U);

    if (textLength == 0U)
    {
        printf("[BT_USB] empty payload\r\n");
        return;
    }

    isAtCommand = GpLedMatrixBtDebug_IsAtCommand(g_gpMatrixBtCommand);
    UART2_SetBtAtMode(0U);

    printf("[BT_CMD] mode=%s text=%s\r\n",
           (isAtCommand != 0U) ? "AT" : "DATA",
           g_gpMatrixBtCommand);

    textLength = GpLedMatrixBtDebug_AppendCrLf(g_gpMatrixBtCommand,
                                               GP_MATRIX_BT_COMMAND_BUFFER_SIZE,
                                               textLength);

    UART2_ResetRxRing();
    UART2_DebugResetRecentRx();
    g_gpMatrixBtLastTxOk = UART2_SendText(g_gpMatrixBtCommand);
    g_gpMatrixBtLastRxBytes = 0U;
    if (g_gpMatrixBtLastTxOk == 0U)
    {
        GpLedMatrixBtDebug_PrintStatus("tx-timeout");
        return;
    }

    hasNextBaudrate = GpLedMatrixBtDebug_ParseTargetBaud(payload, &nextBaudrate);

    g_gpMatrixBtLastRxBytes = GpLedMatrixBtDebug_ReadReply(g_gpMatrixBtReply,
                                                           GP_MATRIX_BT_REPLY_BUFFER_SIZE,
                                                           GP_MATRIX_BT_REPLY_TIMEOUT_MS);
    if (g_gpMatrixBtLastRxBytes == 0U)
    {
        printf("[BT_RSP] timeout=%u\r\n", (unsigned int)GP_MATRIX_BT_REPLY_TIMEOUT_MS);
        GpLedMatrixBtDebug_PrintStatus("no-reply");
        return;
    }

    GpLedMatrixBtDebug_FormatAscii(g_gpMatrixBtReply, g_gpMatrixBtLastRxBytes);
    GpLedMatrixBtDebug_FormatHex(g_gpMatrixBtReply, g_gpMatrixBtLastRxBytes);
    printf("[BT_RSP] len=%u ascii=%s\r\n",
           (unsigned int)g_gpMatrixBtLastRxBytes,
           g_gpMatrixBtAscii);
    printf("[BT_RSP] hex=%s\r\n", g_gpMatrixBtHex);

    canSwitchLocalBaud = 0U;
    if (hasNextBaudrate != 0U)
    {
        canSwitchLocalBaud = GpLedMatrixBtDebug_ReplyContainsOk(g_gpMatrixBtReply,
                                                                g_gpMatrixBtLastRxBytes);
        if (canSwitchLocalBaud != 0U)
        {
            delay_ms(GP_MATRIX_BT_COMMAND_GAP_MS);
            UART2_ResetRxRing();
            UART2_DebugResetRecentRx();
            g_gpMatrixBtLastTxOk = UART2_SendText("AT+RESET\r\n");
            if (g_gpMatrixBtLastTxOk == 0U)
            {
                GpLedMatrixBtDebug_PrintStatus("reset-tx-timeout");
                return;
            }

            resetReplyLength = GpLedMatrixBtDebug_ReadReply(g_gpMatrixBtReply,
                                                            GP_MATRIX_BT_REPLY_BUFFER_SIZE,
                                                            GP_MATRIX_BT_REPLY_TIMEOUT_MS);
            if (resetReplyLength == 0U)
            {
                printf("[BT_RSP] reset_timeout=%u\r\n", (unsigned int)GP_MATRIX_BT_REPLY_TIMEOUT_MS);
                GpLedMatrixBtDebug_PrintStatus("reset-no-reply");
                return;
            }

            GpLedMatrixBtDebug_FormatAscii(g_gpMatrixBtReply, resetReplyLength);
            GpLedMatrixBtDebug_FormatHex(g_gpMatrixBtReply, resetReplyLength);
            printf("[BT_CMD] mode=AT text=AT+RESET\r\n");
            printf("[BT_RSP] len=%u ascii=%s\r\n", (unsigned int)resetReplyLength, g_gpMatrixBtAscii);
            printf("[BT_RSP] hex=%s\r\n", g_gpMatrixBtHex);
            if (GpLedMatrixBtDebug_ReplyContainsOk(g_gpMatrixBtReply, resetReplyLength) == 0U)
            {
                printf("[BT_CFG] local_baud_hold=%lu reason=reset_reply_not_ok\r\n",
                       (unsigned long)UART2_GetBaudrate());
                GpLedMatrixBtDebug_PrintStatus("reset-reply-not-ok");
                return;
            }

            UART2_ResetRxRing();
            delay_ms(GP_MATRIX_BT_BAUD_SWITCH_DELAY_MS);
            if (UART2_SetBaudrate(nextBaudrate) != 0U)
            {
                printf("[BT_CFG] local_baud=%lu\r\n", (unsigned long)nextBaudrate);
            }
            else
            {
                printf("[BT_CFG] local_baud_switch_failed=%lu\r\n", (unsigned long)nextBaudrate);
            }
        }
        else
        {
            printf("[BT_CFG] local_baud_hold=%lu reason=reply_not_ok\r\n",
                   (unsigned long)UART2_GetBaudrate());
        }
    }

    GpLedMatrixBtDebug_PrintStatus("reply-ok");
    UART2_ResetRxRing();
    UART2_DebugResetRecentRx();
}

static void GpLedMatrixBtDebug_RunAutoSetup(void)
{
    printf("[BT_CFG] startup=begin\r\n");

    UART2_SetBtAtMode(0U);
    UART2_ResetRxRing();
    UART2_DebugResetRecentRx();
    if (UART2_SetBaudrate(38400UL) != 0U)
    {
        printf("[BT_CFG] local_baud=38400\r\n");
    }
    else
    {
        printf("[BT_CFG] local_baud_switch_failed=38400\r\n");
    }

    GpLedMatrixBtDebug_SendText("AT");
    delay_ms(GP_MATRIX_BT_COMMAND_GAP_MS);
    GpLedMatrixBtDebug_SendText("AT+VERSION?");
    delay_ms(GP_MATRIX_BT_COMMAND_GAP_MS);
    GpLedMatrixBtDebug_SendText("AT+ROLE=0");
    delay_ms(GP_MATRIX_BT_COMMAND_GAP_MS);
    GpLedMatrixBtDebug_SendText("AT+ROLE?");
    delay_ms(GP_MATRIX_BT_COMMAND_GAP_MS);
    GpLedMatrixBtDebug_SendText("AT+NAME=WS2812");
    delay_ms(GP_MATRIX_BT_COMMAND_GAP_MS);
    GpLedMatrixBtDebug_SendText("AT+NAME?");
    delay_ms(GP_MATRIX_BT_COMMAND_GAP_MS);
    GpLedMatrixBtDebug_SendText("AT+PSWD=19220309");
    delay_ms(GP_MATRIX_BT_COMMAND_GAP_MS);
    GpLedMatrixBtDebug_SendText("AT+PSWD?");
    delay_ms(GP_MATRIX_BT_COMMAND_GAP_MS);
    GpLedMatrixBtDebug_SendText("AT+UART=115200,0,0");
    delay_ms(GP_MATRIX_BT_COMMAND_GAP_MS);
    GpLedMatrixBtDebug_SendText("AT+UART?");
    printf("[BT_CFG] startup=end\r\n");
}

void GpLedMatrixBtDebug_SetReady(uint8_t ready)
{
    g_gpMatrixBtReady = ready;
}

void GpLedMatrixBtDebug_PrintInit(void)
{
    PORT2_ClearDebugLeds();
    UART2_SetBtAtMode(0U);
    printf("[BT_INIT] uart2=P42(RX),P43(TX) at=P41 baud=%lu startup_at=38400 data=115200\r\n",
           (unsigned long)UART2_GetBaudrate());
    GpLedMatrixBtDebug_PrintStatus("init");
    GpLedMatrixBtDebug_RunAutoSetup();
}

void GpLedMatrixBtDebug_Task(void)
{
    uint16_t totalRx;
    uint8_t sniffLength;
    uint8_t copyIndex;

    if (g_gpMatrixBtReady == 0U)
    {
        return;
    }

    if (UART2_DebugHasRecentRx() == 0U)
    {
        g_gpMatrixBtRxIdleTicks = 0U;
        return;
    }

    totalRx = UART2_GetRxTotalCount();
    if (totalRx != g_gpMatrixBtLastTotalRx)
    {
        g_gpMatrixBtLastTotalRx = totalRx;
        g_gpMatrixBtRxIdleTicks = 0U;
        return;
    }

    if (g_gpMatrixBtRxIdleTicks < GP_MATRIX_BT_TASK_IDLE_TICKS)
    {
        g_gpMatrixBtRxIdleTicks++;
        return;
    }

    sniffLength = UART2_DebugCopyRecentRx(g_gpMatrixBtSniff,
                                          GP_MATRIX_BT_SNIFF_BUFFER_SIZE,
                                          1U);
    g_gpMatrixBtRxIdleTicks = 0U;
    if (sniffLength == 0U)
    {
        return;
    }

    GpLedMatrixBtDebug_FormatAscii((const char *)g_gpMatrixBtSniff, sniffLength);
    GpLedMatrixBtDebug_FormatHex((const char *)g_gpMatrixBtSniff, sniffLength);
    printf("[BT_MON] len=%u ascii=%s\r\n", (unsigned int)sniffLength, g_gpMatrixBtAscii);
    printf("[BT_MON] hex=%s\r\n", g_gpMatrixBtHex);

    if (sniffLength >= GP_MATRIX_BT_COMMAND_BUFFER_SIZE)
    {
        sniffLength = (uint8_t)(GP_MATRIX_BT_COMMAND_BUFFER_SIZE - 1U);
    }

    for (copyIndex = 0U; copyIndex < sniffLength; ++copyIndex)
    {
        g_gpMatrixBtCommand[copyIndex] = (char)g_gpMatrixBtSniff[copyIndex];
    }
    g_gpMatrixBtCommand[sniffLength] = '\0';
    GpLedMatrixBtDebug_TrimTrailingSpace(g_gpMatrixBtCommand);
    GpLedMatrixBtDebug_HandleReceivedText(g_gpMatrixBtCommand);
}

void GpLedMatrixBtDebug_HandleUsbCommand(const uint8_t *commandBytes, uint8_t length)
{
    uint8_t copyIndex;
    const char *payload;

    if (g_gpMatrixBtReady == 0U)
    {
        printf("[BT_STA] init=off\r\n");
        return;
    }

    if ((commandBytes == 0) || (length == 0U))
    {
        GpLedMatrixBtDebug_PrintUsage();
        return;
    }

    if (length >= GP_MATRIX_BT_COMMAND_BUFFER_SIZE)
    {
        length = (uint8_t)(GP_MATRIX_BT_COMMAND_BUFFER_SIZE - 1U);
    }

    for (copyIndex = 0U; copyIndex < length; ++copyIndex)
    {
        g_gpMatrixBtCommand[copyIndex] = (char)commandBytes[copyIndex];
    }
    g_gpMatrixBtCommand[length] = '\0';

    payload = GpLedMatrixBtDebug_SkipSpace(g_gpMatrixBtCommand);
    if (*payload == '\0')
    {
        GpLedMatrixBtDebug_PrintUsage();
        return;
    }

    if (GpLedMatrixBtDebug_MatchKeyword(payload, "HELP") != 0U)
    {
        GpLedMatrixBtDebug_PrintUsage();
        return;
    }

    if (GpLedMatrixBtDebug_MatchKeyword(payload, "STATUS") != 0U)
    {
        GpLedMatrixBtDebug_PrintStatus("manual");
        return;
    }

    if (GpLedMatrixBtDebug_MatchKeyword(payload, "SEND") != 0U)
    {
        payload += 4;
        payload = GpLedMatrixBtDebug_SkipSpace(payload);
        if (*payload == '\0')
        {
            printf("[BT_HELP] empty_send_payload\r\n");
            return;
        }
    }

    GpLedMatrixBtDebug_SendText(payload);
}
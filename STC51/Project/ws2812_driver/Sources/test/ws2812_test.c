#include "ws2812_test.h"

static sbit HC595_SER = P0^0;
static sbit HC595_RCLK = P0^1;
static sbit HC595_SRCLK = P0^2;

#define WS_OUT_P10          0
#define WS_OUT_P11          1

#define TEST_LED_PER_ROW    8
#define TEST_SHIFT_BITS     16

#define WS2812_PWM_0_DUTY   12
#define WS2812_PWM_1_DUTY   37
#define WS2812_PWM_PERIOD   50
#define WS2812_PWM_NUM      (TEST_LED_PER_ROW * 24 + 2)

static uint8_t shift_index = 0;
static uint8_t light_count = 1;
static uint8_t ws2812_output_pin = WS_OUT_P10;
static bit ws2812_dma_busy = 0;

static uint8_t xdata ws_rgb[TEST_LED_PER_ROW][3];
static uint8_t xdata ws_pwm[WS2812_PWM_NUM];

static void ws2812_apply_output_pin(uint8_t output_pin)
{
    if (output_pin == WS_OUT_P11)
    {
        PWMA_CCER1 = 0x04;
        PWMA_ENO = 0x02;
        PWMA_DBA = 0x0D;
        P10 = 0;
    }
    else
    {
        PWMA_CCER1 = 0x01;
        PWMA_ENO = 0x01;
        PWMA_DBA = 0x0D;
        P11 = 0;
    }
}

static void hc595_pulse_srclk(void)
{
    HC595_SRCLK = 1;
    _nop_();
    HC595_SRCLK = 0;
}

static void hc595_pulse_rclk(void)
{
    HC595_RCLK = 1;
    _nop_();
    HC595_RCLK = 0;
}

static void hc595_write16(uint16_t value)
{
    int8_t bit_index;

    for (bit_index = 15; bit_index >= 0; bit_index--)
    {
        HC595_SER = (value >> bit_index) & 0x01;
        hc595_pulse_srclk();
    }

    hc595_pulse_rclk();
}

static void ws2812_pwm_config(void)
{
    PWMA_ENO = 0;
    PWMA_IER = 0;
    PWMA_SR1 = 0;
    PWMA_SR2 = 0;

    PWMA_PSCRH = 0;
    PWMA_PSCRL = 0;
    PWMA_DTR = 24;
    PWMA_ARRH = 0;
    PWMA_ARRL = WS2812_PWM_PERIOD - 1;

    PWMA_CCMR1 = 0x68;
    PWMA_CCR1H = 0;
    PWMA_CCR1L = 0;

    PWMA_CCER1 = 0x01;
    PWMA_CCER2 = 0x00;
    PWMA_PS = 0x00;
    PWMA_ENO = 0x01;

    P1M1 &= ~(BIT0 | BIT1);
    P1M0 |= (BIT0 | BIT1);

    PWMA_BKR = 0x80;
    PWMA_CR1 = 0x81;
    PWMA_EGR = 0x01;
}

static void ws2812_build_pwm_data(uint8_t on_count)
{
    uint8_t led;
    uint8_t color_idx;
    uint8_t bit_idx;
    uint16_t pwm_idx;
    uint8_t color_val;

    for (led = 0; led < TEST_LED_PER_ROW; led++)
    {
        if (led < on_count)
        {
            ws_rgb[led][0] = 16;
            ws_rgb[led][1] = 16;
            ws_rgb[led][2] = 16;
        }
        else
        {
            ws_rgb[led][0] = 0;
            ws_rgb[led][1] = 0;
            ws_rgb[led][2] = 0;
        }
    }

    ws_pwm[0] = 0;
    pwm_idx = 1;
    for (led = 0; led < TEST_LED_PER_ROW; led++)
    {
        for (color_idx = 0; color_idx < 3; color_idx++)
        {
            color_val = ws_rgb[led][color_idx];
            for (bit_idx = 0; bit_idx < 8; bit_idx++)
            {
                if (color_val & 0x80)
                {
                    ws_pwm[pwm_idx] = WS2812_PWM_1_DUTY;
                }
                else
                {
                    ws_pwm[pwm_idx] = WS2812_PWM_0_DUTY;
                }
                color_val <<= 1;
                pwm_idx++;
            }
        }
    }
    ws_pwm[pwm_idx] = 0;
}

static void ws2812_dma_start(uint8_t xdata *tx_buf, uint16_t num, uint8_t output_pin)
{
    uint16_t addr;

    ws2812_apply_output_pin(output_pin);

    PWMA_DBL = 0x00;
    PWMA_DER = 0x01;
    PWMA_DMACR = 0x14;

    addr = (uint16_t)tx_buf;
    DMA_PWMAT_TXAH = (uint8_t)(addr >> 8);
    DMA_PWMAT_TXAL = (uint8_t)addr;
    DMA_PWMAT_AMTH = (uint8_t)((num - 1) >> 8);
    DMA_PWMAT_AMT = (uint8_t)(num - 1);
    DMA_PWMAT_STA = 0x00;
    DMA_PWMAT_CFG = 0x80;
    ws2812_dma_busy = 1;
    DMA_PWMAT_CR = 0xC0;
}

void ws2812_test_init(void)
{
    P0M1 &= ~(BIT0 | BIT1 | BIT2);
    P0M0 |= (BIT0 | BIT1 | BIT2);

    HC595_SER = 0;
    HC595_RCLK = 0;
    HC595_SRCLK = 0;

    ws2812_pwm_config();
    shift_index = 0;
    light_count = 1;
    ws2812_output_pin = WS_OUT_P10;
    ws2812_dma_busy = 0;
}

void ws2812_test_step(void)
{
    uint16_t shift_data;

    while (ws2812_dma_busy);

    shift_data = (uint16_t)(~((uint16_t)0x0001 << shift_index));
    hc595_write16(shift_data);

    ws2812_build_pwm_data(light_count);
    ws2812_dma_start(ws_pwm, WS2812_PWM_NUM, ws2812_output_pin);
    while (ws2812_dma_busy);
    delay_ms(100);

    light_count++;
    if (light_count > TEST_LED_PER_ROW)
    {
        light_count = 1;
        shift_index++;

        if (shift_index >= TEST_SHIFT_BITS)
        {
            shift_index = 0;
            if (ws2812_output_pin == WS_OUT_P10)
            {
                ws2812_output_pin = WS_OUT_P11;
            }
            else
            {
                ws2812_output_pin = WS_OUT_P10;
            }
        }
    }
}

void PWMAT_DMA_ISR(void) interrupt DMA_PWMAT_VECTOR
{
    DMA_PWMAT_STA = 0;
    ws2812_dma_busy = 0;
}

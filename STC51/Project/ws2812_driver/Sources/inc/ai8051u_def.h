#ifndef __AI8051U_DEF_H__
#define __AI8051U_DEF_H__

/////////////////////////////////////////////////

#include "ai8051u.h"
#include "def.h"

/////////////////////////////////////////////////

#define PORT_SetInitLevelLow(p, b)      CLR_REG_BIT(P##p, (b))
#define PORT_SetInitLevelHigh(p, b)     SET_REG_BIT(P##p, (b))

#define PORT_SetQuasiMode(p, b)         CLR_REG_BIT(P##p##M0, (b)); \
                                        CLR_REG_BIT(P##p##M1, (b))

#define PORT_SetPushPullMode(p, b)      SET_REG_BIT(P##p##M0, (b)); \
                                        CLR_REG_BIT(P##p##M1, (b))

#define PORT_SetHighZInputMode(p, b)    CLR_REG_BIT(P##p##M0, (b)); \
                                        SET_REG_BIT(P##p##M1, (b))

#define PORT_SetOpenDrainMode(p, b)     SET_REG_BIT(P##p##M0, (b)); \
                                        SET_REG_BIT(P##p##M1, (b))

#define PORT_SetAutoMode(p, b)          CLR_REG_BIT(P##p##BP, (b))
#define PORT_SetManualMode(p, b)        SET_REG_BIT(P##p##BP, (b))

#define PORT_EnablePullUp(p, b)         SET_REG_BIT(P##p##PU, (b))
#define PORT_DisablePullUp(p, b)        CLR_REG_BIT(P##p##PU, (b))

#define PORT_EnablePullDown(p, b)       SET_REG_BIT(P##p##PD, (b))
#define PORT_DisablePullDown(p, b)      CLR_REG_BIT(P##p##PD, (b))

#define PORT_EnableSchmitt(p, b)        CLR_REG_BIT(P##p##NCS, (b))
#define PORT_DisableSchmitt(p, b)       SET_REG_BIT(P##p##NCS, (b))

#define PORT_SetSlewRateFast(p, b)      CLR_REG_BIT(P##p##SR, (b))
#define PORT_SetSlewRateNormal(p, b)    SET_REG_BIT(P##p##SR, (b))

#define PORT_SetDrivingStrong(p, b)     CLR_REG_BIT(P##p##DR, (b))
#define PORT_SetDrivingNormal(p, b)     SET_REG_BIT(P##p##DR, (b))

#define PORT_SetAnalogInput(p, b)       CLR_REG_BIT(P##p##IE, (b))
#define PORT_SetDigitalInput(p, b)      SET_REG_BIT(P##p##IE, (b))

#define SetP0nInitLevelLow(b)           PORT_SetInitLevelLow(0, (b))
#define SetP1nInitLevelLow(b)           PORT_SetInitLevelLow(1, (b))
#define SetP2nInitLevelLow(b)           PORT_SetInitLevelLow(2, (b))
#define SetP3nInitLevelLow(b)           PORT_SetInitLevelLow(3, (b))
#define SetP4nInitLevelLow(b)           PORT_SetInitLevelLow(4, (b))
#define SetP5nInitLevelLow(b)           PORT_SetInitLevelLow(5, (b))
#define SetP6nInitLevelLow(b)           PORT_SetInitLevelLow(6, (b))
#define SetP7nInitLevelLow(b)           PORT_SetInitLevelLow(7, (b))

#define SetP0nInitLevelHigh(b)          PORT_SetInitLevelHigh(0, (b))
#define SetP1nInitLevelHigh(b)          PORT_SetInitLevelHigh(1, (b))
#define SetP2nInitLevelHigh(b)          PORT_SetInitLevelHigh(2, (b))
#define SetP3nInitLevelHigh(b)          PORT_SetInitLevelHigh(3, (b))
#define SetP4nInitLevelHigh(b)          PORT_SetInitLevelHigh(4, (b))
#define SetP5nInitLevelHigh(b)          PORT_SetInitLevelHigh(5, (b))
#define SetP6nInitLevelHigh(b)          PORT_SetInitLevelHigh(6, (b))
#define SetP7nInitLevelHigh(b)          PORT_SetInitLevelHigh(7, (b))

#define SetP0nQuasiMode(b)              PORT_SetQuasiMode(0, (b))
#define SetP1nQuasiMode(b)              PORT_SetQuasiMode(1, (b))
#define SetP2nQuasiMode(b)              PORT_SetQuasiMode(2, (b))
#define SetP3nQuasiMode(b)              PORT_SetQuasiMode(3, (b))
#define SetP4nQuasiMode(b)              PORT_SetQuasiMode(4, (b))
#define SetP5nQuasiMode(b)              PORT_SetQuasiMode(5, (b))
#define SetP6nQuasiMode(b)              PORT_SetQuasiMode(6, (b))
#define SetP7nQuasiMode(b)              PORT_SetQuasiMode(7, (b))

#define SetP0nPushPullMode(b)           PORT_SetPushPullMode(0, (b))
#define SetP1nPushPullMode(b)           PORT_SetPushPullMode(1, (b))
#define SetP2nPushPullMode(b)           PORT_SetPushPullMode(2, (b))
#define SetP3nPushPullMode(b)           PORT_SetPushPullMode(3, (b))
#define SetP4nPushPullMode(b)           PORT_SetPushPullMode(4, (b))
#define SetP5nPushPullMode(b)           PORT_SetPushPullMode(5, (b))
#define SetP6nPushPullMode(b)           PORT_SetPushPullMode(6, (b))
#define SetP7nPushPullMode(b)           PORT_SetPushPullMode(7, (b))

#define SetP0nHighZInputMode(b)         PORT_SetHighZInputMode(0, (b))
#define SetP1nHighZInputMode(b)         PORT_SetHighZInputMode(1, (b))
#define SetP2nHighZInputMode(b)         PORT_SetHighZInputMode(2, (b))
#define SetP3nHighZInputMode(b)         PORT_SetHighZInputMode(3, (b))
#define SetP4nHighZInputMode(b)         PORT_SetHighZInputMode(4, (b))
#define SetP5nHighZInputMode(b)         PORT_SetHighZInputMode(5, (b))
#define SetP6nHighZInputMode(b)         PORT_SetHighZInputMode(6, (b))
#define SetP7nHighZInputMode(b)         PORT_SetHighZInputMode(7, (b))

#define SetP0nOpenDrainMode(b)          PORT_SetOpenDrainMode(0, (b))
#define SetP1nOpenDrainMode(b)          PORT_SetOpenDrainMode(1, (b))
#define SetP2nOpenDrainMode(b)          PORT_SetOpenDrainMode(2, (b))
#define SetP3nOpenDrainMode(b)          PORT_SetOpenDrainMode(3, (b))
#define SetP4nOpenDrainMode(b)          PORT_SetOpenDrainMode(4, (b))
#define SetP5nOpenDrainMode(b)          PORT_SetOpenDrainMode(5, (b))
#define SetP6nOpenDrainMode(b)          PORT_SetOpenDrainMode(6, (b))
#define SetP7nOpenDrainMode(b)          PORT_SetOpenDrainMode(7, (b))

#define SetP0nAutoMode(b)               PORT_SetAutoMode(0, (b))
#define SetP1nAutoMode(b)               PORT_SetAutoMode(1, (b))
#define SetP2nAutoMode(b)               PORT_SetAutoMode(2, (b))
#define SetP3nAutoMode(b)               PORT_SetAutoMode(3, (b))
#define SetP4nAutoMode(b)               PORT_SetAutoMode(4, (b))
#define SetP5nAutoMode(b)               PORT_SetAutoMode(5, (b))
#define SetP6nAutoMode(b)               PORT_SetAutoMode(6, (b))
#define SetP7nAutoMode(b)               PORT_SetAutoMode(7, (b))

#define SetP0nManualMode(b)             PORT_SetManualMode(0, (b))
#define SetP1nManualMode(b)             PORT_SetManualMode(1, (b))
#define SetP2nManualMode(b)             PORT_SetManualMode(2, (b))
#define SetP3nManualMode(b)             PORT_SetManualMode(3, (b))
#define SetP4nManualMode(b)             PORT_SetManualMode(4, (b))
#define SetP5nManualMode(b)             PORT_SetManualMode(5, (b))
#define SetP6nManualMode(b)             PORT_SetManualMode(6, (b))
#define SetP7nManualMode(b)             PORT_SetManualMode(7, (b))

#define EnableP0nPullUp(b)              PORT_EnablePullUp(0, (b))
#define EnableP1nPullUp(b)              PORT_EnablePullUp(1, (b))
#define EnableP2nPullUp(b)              PORT_EnablePullUp(2, (b))
#define EnableP3nPullUp(b)              PORT_EnablePullUp(3, (b))
#define EnableP4nPullUp(b)              PORT_EnablePullUp(4, (b))
#define EnableP5nPullUp(b)              PORT_EnablePullUp(5, (b))
#define EnableP6nPullUp(b)              PORT_EnablePullUp(6, (b))
#define EnableP7nPullUp(b)              PORT_EnablePullUp(7, (b))

#define DisableP0nPullUp(b)             PORT_DisablePullUp(0, (b))
#define DisableP1nPullUp(b)             PORT_DisablePullUp(1, (b))
#define DisableP2nPullUp(b)             PORT_DisablePullUp(2, (b))
#define DisableP3nPullUp(b)             PORT_DisablePullUp(3, (b))
#define DisableP4nPullUp(b)             PORT_DisablePullUp(4, (b))
#define DisableP5nPullUp(b)             PORT_DisablePullUp(5, (b))
#define DisableP6nPullUp(b)             PORT_DisablePullUp(6, (b))
#define DisableP7nPullUp(b)             PORT_DisablePullUp(7, (b))

#define EnableP0nPullDown(b)            PORT_EnablePullDown(0, (b))
#define EnableP1nPullDown(b)            PORT_EnablePullDown(1, (b))
#define EnableP2nPullDown(b)            PORT_EnablePullDown(2, (b))
#define EnableP3nPullDown(b)            PORT_EnablePullDown(3, (b))
#define EnableP4nPullDown(b)            PORT_EnablePullDown(4, (b))
#define EnableP5nPullDown(b)            PORT_EnablePullDown(5, (b))
#define EnableP6nPullDown(b)            PORT_EnablePullDown(6, (b))
#define EnableP7nPullDown(b)            PORT_EnablePullDown(7, (b))

#define DisableP0nPullDown(b)           PORT_DisablePullDown(0, (b))
#define DisableP1nPullDown(b)           PORT_DisablePullDown(1, (b))
#define DisableP2nPullDown(b)           PORT_DisablePullDown(2, (b))
#define DisableP3nPullDown(b)           PORT_DisablePullDown(3, (b))
#define DisableP4nPullDown(b)           PORT_DisablePullDown(4, (b))
#define DisableP5nPullDown(b)           PORT_DisablePullDown(5, (b))
#define DisableP6nPullDown(b)           PORT_DisablePullDown(6, (b))
#define DisableP7nPullDown(b)           PORT_DisablePullDown(7, (b))

#define EnableP0nSchmitt(b)             PORT_EnableSchmitt(0, (b))
#define EnableP1nSchmitt(b)             PORT_EnableSchmitt(1, (b))
#define EnableP2nSchmitt(b)             PORT_EnableSchmitt(2, (b))
#define EnableP3nSchmitt(b)             PORT_EnableSchmitt(3, (b))
#define EnableP4nSchmitt(b)             PORT_EnableSchmitt(4, (b))
#define EnableP5nSchmitt(b)             PORT_EnableSchmitt(5, (b))
#define EnableP6nSchmitt(b)             PORT_EnableSchmitt(6, (b))
#define EnableP7nSchmitt(b)             PORT_EnableSchmitt(7, (b))

#define DisableP0nSchmitt(b)            PORT_DisableSchmitt(0, (b))
#define DisableP1nSchmitt(b)            PORT_DisableSchmitt(1, (b))
#define DisableP2nSchmitt(b)            PORT_DisableSchmitt(2, (b))
#define DisableP3nSchmitt(b)            PORT_DisableSchmitt(3, (b))
#define DisableP4nSchmitt(b)            PORT_DisableSchmitt(4, (b))
#define DisableP5nSchmitt(b)            PORT_DisableSchmitt(5, (b))
#define DisableP6nSchmitt(b)            PORT_DisableSchmitt(6, (b))
#define DisableP7nSchmitt(b)            PORT_DisableSchmitt(7, (b))

#define SetP0nSlewRateFast(b)           PORT_SetSlewRateFast(0, (b))
#define SetP1nSlewRateFast(b)           PORT_SetSlewRateFast(1, (b))
#define SetP2nSlewRateFast(b)           PORT_SetSlewRateFast(2, (b))
#define SetP3nSlewRateFast(b)           PORT_SetSlewRateFast(3, (b))
#define SetP4nSlewRateFast(b)           PORT_SetSlewRateFast(4, (b))
#define SetP5nSlewRateFast(b)           PORT_SetSlewRateFast(5, (b))
#define SetP6nSlewRateFast(b)           PORT_SetSlewRateFast(6, (b))
#define SetP7nSlewRateFast(b)           PORT_SetSlewRateFast(7, (b))

#define SetP0nSlewRateNormal(b)         PORT_SetSlewRateNormal(0, (b))
#define SetP1nSlewRateNormal(b)         PORT_SetSlewRateNormal(1, (b))
#define SetP2nSlewRateNormal(b)         PORT_SetSlewRateNormal(2, (b))
#define SetP3nSlewRateNormal(b)         PORT_SetSlewRateNormal(3, (b))
#define SetP4nSlewRateNormal(b)         PORT_SetSlewRateNormal(4, (b))
#define SetP5nSlewRateNormal(b)         PORT_SetSlewRateNormal(5, (b))
#define SetP6nSlewRateNormal(b)         PORT_SetSlewRateNormal(6, (b))
#define SetP7nSlewRateNormal(b)         PORT_SetSlewRateNormal(7, (b))

#define SetP0nDrivingStrong(b)          PORT_SetDrivingStrong(0, (b))
#define SetP1nDrivingStrong(b)          PORT_SetDrivingStrong(1, (b))
#define SetP2nDrivingStrong(b)          PORT_SetDrivingStrong(2, (b))
#define SetP3nDrivingStrong(b)          PORT_SetDrivingStrong(3, (b))
#define SetP4nDrivingStrong(b)          PORT_SetDrivingStrong(4, (b))
#define SetP5nDrivingStrong(b)          PORT_SetDrivingStrong(5, (b))
#define SetP6nDrivingStrong(b)          PORT_SetDrivingStrong(6, (b))
#define SetP7nDrivingStrong(b)          PORT_SetDrivingStrong(7, (b))

#define SetP0nDrivingNormal(b)          PORT_SetDrivingNormal(0, (b))
#define SetP1nDrivingNormal(b)          PORT_SetDrivingNormal(1, (b))
#define SetP2nDrivingNormal(b)          PORT_SetDrivingNormal(2, (b))
#define SetP3nDrivingNormal(b)          PORT_SetDrivingNormal(3, (b))
#define SetP4nDrivingNormal(b)          PORT_SetDrivingNormal(4, (b))
#define SetP5nDrivingNormal(b)          PORT_SetDrivingNormal(5, (b))
#define SetP6nDrivingNormal(b)          PORT_SetDrivingNormal(6, (b))
#define SetP7nDrivingNormal(b)          PORT_SetDrivingNormal(7, (b))

#define SetP0nAnalogInput(b)            PORT_SetAnalogInput(0, (b))
#define SetP1nAnalogInput(b)            PORT_SetAnalogInput(1, (b))
#define SetP2nAnalogInput(b)            PORT_SetAnalogInput(2, (b))
#define SetP3nAnalogInput(b)            PORT_SetAnalogInput(3, (b))
#define SetP4nAnalogInput(b)            PORT_SetAnalogInput(4, (b))
#define SetP5nAnalogInput(b)            PORT_SetAnalogInput(5, (b))
#define SetP6nAnalogInput(b)            PORT_SetAnalogInput(6, (b))
#define SetP7nAnalogInput(b)            PORT_SetAnalogInput(7, (b))

#define SetP0nDigitalInput(b)           PORT_SetDigitalInput(0, (b))
#define SetP1nDigitalInput(b)           PORT_SetDigitalInput(1, (b))
#define SetP2nDigitalInput(b)           PORT_SetDigitalInput(2, (b))
#define SetP3nDigitalInput(b)           PORT_SetDigitalInput(3, (b))
#define SetP4nDigitalInput(b)           PORT_SetDigitalInput(4, (b))
#define SetP5nDigitalInput(b)           PORT_SetDigitalInput(5, (b))
#define SetP6nDigitalInput(b)           PORT_SetDigitalInput(6, (b))
#define SetP7nDigitalInput(b)           PORT_SetDigitalInput(7, (b))

/////////////////////////////////////////////////

#define EnableGlobalInt()               (EA = 1)
#define DisableGlobalInt()              (EA = 0)

#define INT0_EnableInt()                (EX0 = 1)
#define INT0_DisableInt()               (EX0 = 0)

#define INT1_EnableInt()                (EX1 = 1)
#define INT1_DisableInt()               (EX1 = 0)

#define INTCLKO_EX2_MSK                 BIT4
#define INT2_EnableInt()                SET_REG_BIT(INTCLKO, INTCLKO_EX2_MSK)
#define INT2_DisableInt()               CLR_REG_BIT(INTCLKO, INTCLKO_EX2_MSK)

#define INTCLKO_EX3_MSK                 BIT5
#define INT3_EnableInt()                SET_REG_BIT(INTCLKO, INTCLKO_EX3_MSK)
#define INT3_DisableInt()               CLR_REG_BIT(INTCLKO, INTCLKO_EX3_MSK)

#define INTCLKO_EX4_MSK                 BIT6
#define INT4_EnableInt()                SET_REG_BIT(INTCLKO, INTCLKO_EX4_MSK)
#define INT4_DisableInt()               CLR_REG_BIT(INTCLKO, INTCLKO_EX4_MSK)

#define TIMER0_EnableInt()              (ET0 = 1)
#define TIMER0_DisableInt()             (ET0 = 0)

#define TIMER1_EnableInt()              (ET1 = 1)
#define TIMER1_DisableInt()             (ET1 = 0)

#define IE2_ET2_MSK                     BIT2
#define TIMER2_EnableInt()              SET_REG_BIT(IE2, IE2_ET2_MSK)
#define TIMER2_DisableInt()             CLR_REG_BIT(IE2, IE2_ET2_MSK)

#define IE2_ET3_MSK                     BIT5
#define TIMER3_EnableInt()              SET_REG_BIT(IE2, IE2_ET3_MSK)
#define TIMER3_DisableInt()             CLR_REG_BIT(IE2, IE2_ET3_MSK)

#define IE2_ET4_MSK                     BIT6
#define TIMER4_EnableInt()              SET_REG_BIT(IE2, IE2_ET4_MSK)
#define TIMER4_DisableInt()             CLR_REG_BIT(IE2, IE2_ET4_MSK)

#define T11CR_ET11I_MSK                 BIT1
#define TIMER11_EnableInt()             SET_REG_BIT(T11CR, T11CR_ET11I_MSK)
#define TIMER11_DisableInt()            CLR_REG_BIT(T11CR, T11CR_ET11I_MSK)

#define UART1_EnableInt()               (ES = 1)
#define UART1_DisableInt()              (ES = 0)

#define IE2_ES2_MSK                     BIT0
#define UART2_EnableInt()               SET_REG_BIT(IE2, IE2_ES2_MSK)
#define UART2_DisableInt()              CLR_REG_BIT(IE2, IE2_ES2_MSK)

#define IE2_ES3_MSK                     BIT3
#define UART3_EnableInt()               SET_REG_BIT(IE2, IE2_ES3_MSK)
#define UART3_DisableInt()              CLR_REG_BIT(IE2, IE2_ES3_MSK)

#define IE2_ES4_MSK                     BIT4
#define UART4_EnableInt()               SET_REG_BIT(IE2, IE2_ES4_MSK)
#define UART4_DisableInt()              CLR_REG_BIT(IE2, IE2_ES4_MSK)

#define LVD_EnableInt()                 (ELVD = 1)
#define LVD_DisableInt()                (ELVD = 0)

#define ADC_EnableInt()                 (EADC = 1)
#define ADC_DisableInt()                (EADC = 0)

#define IE2_ESPI_MSK                    BIT1
#define SPI_EnableInt()                 SET_REG_BIT(IE2, IE2_ESPI_MSK)
#define SPI_DisableInt()                CLR_REG_BIT(IE2, IE2_ESPI_MSK)

#define IE2_EUSB_MSK                    BIT7
#define USB_EnableInt()                 SET_REG_BIT(IE2, IE2_EUSB_MSK)
#define USB_DisableInt()                CLR_REG_BIT(IE2, IE2_EUSB_MSK)

#define CMPCR1_PIE_MSK                  BIT5
#define CMPCR1_NIE_MSK                  BIT4
#define CMP_EnablePosedgeInt()          SET_REG_BIT(CMPCR1, CMPCR1_PIE_MSK)
#define CMP_EnableNegedgeInt()          SET_REG_BIT(CMPCR1, CMPCR1_NIE_MSK)
#define CMP_EnableEdgeInt()             SET_REG_BIT(CMPCR1, (CMPCR1_PIE_MSK | CMPCR1_NIE_MSK))
#define CMP_DisableInt()                CLR_REG_BIT(CMPCR1, (CMPCR1_PIE_MSK | CMPCR1_NIE_MSK))

#define RTCIEN_ALARM_MSK                BIT7
#define RTCIEN_DAY_MSK                  BIT6
#define RTCIEN_HOUR_MSK                 BIT5
#define RTCIEN_MIN_MSK                  BIT4
#define RTCIEN_SEC_MSK                  BIT3
#define RTCIEN_SEC2_MSK                 BIT2
#define RTCIEN_SEC8_MSK                 BIT1
#define RTCIEN_SEC32_MSK                BIT0
#define RTC_EnableAlarmInt()            SET_REG_BIT(RTCIEN, RTCIEN_ALARM_MSK)
#define RTC_EnableDayInt()              SET_REG_BIT(RTCIEN, RTCIEN_DAY_MSK)
#define RTC_EnableHourInt()             SET_REG_BIT(RTCIEN, RTCIEN_HOUR_MSK)
#define RTC_EnableMinuteInt()           SET_REG_BIT(RTCIEN, RTCIEN_MIN_MSK)
#define RTC_EnableSecondInt()           SET_REG_BIT(RTCIEN, RTCIEN_SEC_MSK)
#define RTC_EnableSecondD2Int()         SET_REG_BIT(RTCIEN, RTCIEN_SEC2_MSK)
#define RTC_EnableSecondD8Int()         SET_REG_BIT(RTCIEN, RTCIEN_SEC8_MSK)
#define RTC_EnableSecondD32Int()        SET_REG_BIT(RTCIEN, RTCIEN_SEC32_MSK)
#define RTC_DisableAlarmInt()           CLR_REG_BIT(RTCIEN, RTCIEN_ALARM_MSK)
#define RTC_DisableDayInt()             CLR_REG_BIT(RTCIEN, RTCIEN_DAY_MSK)
#define RTC_DisableHourInt()            CLR_REG_BIT(RTCIEN, RTCIEN_HOUR_MSK)
#define RTC_DisableMinuteInt()          CLR_REG_BIT(RTCIEN, RTCIEN_MIN_MSK)
#define RTC_DisableSecondInt()          CLR_REG_BIT(RTCIEN, RTCIEN_SEC_MSK)
#define RTC_DisableSecondD2Int()        CLR_REG_BIT(RTCIEN, RTCIEN_SEC2_MSK)
#define RTC_DisableSecondD8Int()        CLR_REG_BIT(RTCIEN, RTCIEN_SEC8_MSK)
#define RTC_DisableSecondD32Int()       CLR_REG_BIT(RTCIEN, RTCIEN_SEC32_MSK)

#define LCMIFIE_MSK                     BIT7
#define LCM_EnableInt()                 SET_REG_BIT(LCMIFCFG, LCMIFIE_MSK)
#define LCM_DisableInt()                CLR_REG_BIT(LCMIFCFG, LCMIFIE_MSK)

#define I2CMSCR_EMSI_MSK                BIT7
#define I2C_EnableMasterInt()           SET_REG_BIT(I2CMSCR, I2CMSCR_EMSI_MSK)
#define I2C_DisableMasterInt()          CLR_REG_BIT(I2CMSCR, I2CMSCR_EMSI_MSK)

#define I2CSLCR_ESTAI_MSK               BIT6
#define I2CSLCR_ERXI_MSK                BIT5
#define I2CSLCR_ETXI_MSK                BIT4
#define I2CSLCR_ESTOI_MSK               BIT3
#define I2C_EnableSlaveSTAInt()         SET_REG_BIT(I2CSLCR, I2CSLCR_ESTAI_MSK)
#define I2C_EnableSlaveSTOInt()         SET_REG_BIT(I2CSLCR, I2CSLCR_ESTOI_MSK)
#define I2C_EnableSlaveRXInt()          SET_REG_BIT(I2CSLCR, I2CSLCR_ERXI_MSK)
#define I2C_EnableSlaveTXInt()          SET_REG_BIT(I2CSLCR, I2CSLCR_ETXI_MSK)
#define I2C_EnableSlaveAllInt()         SET_REG_BIT(I2CSLCR, I2CSLCR_ESTAI_MSK | I2CSLCR_ESTOI_MSK | I2CSLCR_ERXI_MSK | I2CSLCR_ETXI_MSK)
#define I2C_DisableSlaveSTAInt()        CLR_REG_BIT(I2CSLCR, I2CSLCR_ESTAI_MSK)
#define I2C_DisableSlaveSTOInt()        CLR_REG_BIT(I2CSLCR, I2CSLCR_ESTOI_MSK)
#define I2C_DisableSlaveRXInt()         CLR_REG_BIT(I2CSLCR, I2CSLCR_ERXI_MSK)
#define I2C_DisableSlaveTXInt()         CLR_REG_BIT(I2CSLCR, I2CSLCR_ETXI_MSK)
#define I2C_DisableSlaveAllInt()        CLR_REG_BIT(I2CSLCR, I2CSLCR_ESTAI_MSK | I2CSLCR_ESTOI_MSK | I2CSLCR_ERXI_MSK | I2CSLCR_ETXI_MSK)

#define PWMAIER_BIE_MSK                 BIT7
#define PWMAIER_TIE_MSK                 BIT6
#define PWMAIER_COMIE_MSK               BIT5
#define PWMAIER_CC4IE_MSK               BIT4
#define PWMAIER_CC3IE_MSK               BIT3
#define PWMAIER_CC2IE_MSK               BIT2
#define PWMAIER_CC1IE_MSK               BIT1
#define PWMAIER_UIE_MSK                 BIT0
#define PWMA_EnableBreakInt()           SET_REG_BIT(PWMA_IER, PWMAIER_BIE_MSK)
#define PWMA_EnableTriggerInt()         SET_REG_BIT(PWMA_IER, PWMAIER_TIE_MSK)
#define PWMA_EnableCOMInt()             SET_REG_BIT(PWMA_IER, PWMAIER_COMIE_MSK)
#define PWMA_EnableCC4Int()             SET_REG_BIT(PWMA_IER, PWMAIER_CC4IE_MSK)
#define PWMA_EnableCC3Int()             SET_REG_BIT(PWMA_IER, PWMAIER_CC3IE_MSK)
#define PWMA_EnableCC2Int()             SET_REG_BIT(PWMA_IER, PWMAIER_CC2IE_MSK)
#define PWMA_EnableCC1Int()             SET_REG_BIT(PWMA_IER, PWMAIER_CC1IE_MSK)
#define PWMA_EnableUpdateInt()          SET_REG_BIT(PWMA_IER, PWMAIER_UIE_MSK)
#define PWMA_DisableBreakInt()          CLR_REG_BIT(PWMA_IER, PWMAIER_BIE_MSK)
#define PWMA_DisableTriggerInt()        CLR_REG_BIT(PWMA_IER, PWMAIER_TIE_MSK)
#define PWMA_DisableCOMInt()            CLR_REG_BIT(PWMA_IER, PWMAIER_COMIE_MSK)
#define PWMA_DisableCC4Int()            CLR_REG_BIT(PWMA_IER, PWMAIER_CC4IE_MSK)
#define PWMA_DisableCC3Int()            CLR_REG_BIT(PWMA_IER, PWMAIER_CC3IE_MSK)
#define PWMA_DisableCC2Int()            CLR_REG_BIT(PWMA_IER, PWMAIER_CC2IE_MSK)
#define PWMA_DisableCC1Int()            CLR_REG_BIT(PWMA_IER, PWMAIER_CC1IE_MSK)
#define PWMA_DisableUpdateInt()         CLR_REG_BIT(PWMA_IER, PWMAIER_UIE_MSK)

#define HSPWMA_EnableBreakInt()         SET_HSPWMA_BIT(PWMA_IER, PWMAIER_BIE_MSK)
#define HSPWMA_EnableTriggerInt()       SET_HSPWMA_BIT(PWMA_IER, PWMAIER_TIE_MSK)
#define HSPWMA_EnableCOMInt()           SET_HSPWMA_BIT(PWMA_IER, PWMAIER_COMIE_MSK)
#define HSPWMA_EnableCC4Int()           SET_HSPWMA_BIT(PWMA_IER, PWMAIER_CC4IE_MSK)
#define HSPWMA_EnableCC3Int()           SET_HSPWMA_BIT(PWMA_IER, PWMAIER_CC3IE_MSK)
#define HSPWMA_EnableCC2Int()           SET_HSPWMA_BIT(PWMA_IER, PWMAIER_CC2IE_MSK)
#define HSPWMA_EnableCC1Int()           SET_HSPWMA_BIT(PWMA_IER, PWMAIER_CC1IE_MSK)
#define HSPWMA_EnableUpdateInt()        SET_HSPWMA_BIT(PWMA_IER, PWMAIER_UIE_MSK)
#define HSPWMA_DisableBreakInt()        CLR_HSPWMA_BIT(PWMA_IER, PWMAIER_BIE_MSK)
#define HSPWMA_DisableTriggerInt()      CLR_HSPWMA_BIT(PWMA_IER, PWMAIER_TIE_MSK)
#define HSPWMA_DisableCOMInt()          CLR_HSPWMA_BIT(PWMA_IER, PWMAIER_COMIE_MSK)
#define HSPWMA_DisableCC4Int()          CLR_HSPWMA_BIT(PWMA_IER, PWMAIER_CC4IE_MSK)
#define HSPWMA_DisableCC3Int()          CLR_HSPWMA_BIT(PWMA_IER, PWMAIER_CC3IE_MSK)
#define HSPWMA_DisableCC2Int()          CLR_HSPWMA_BIT(PWMA_IER, PWMAIER_CC2IE_MSK)
#define HSPWMA_DisableCC1Int()          CLR_HSPWMA_BIT(PWMA_IER, PWMAIER_CC1IE_MSK)
#define HSPWMA_DisableUpdateInt()       CLR_HSPWMA_BIT(PWMA_IER, PWMAIER_UIE_MSK)

#define PWMBIER_BIE_MSK                 BIT7
#define PWMBIER_TIE_MSK                 BIT6
#define PWMBIER_COMIE_MSK               BIT5
#define PWMBIER_CC8IE_MSK               BIT4
#define PWMBIER_CC7IE_MSK               BIT3
#define PWMBIER_CC6IE_MSK               BIT2
#define PWMBIER_CC5IE_MSK               BIT1
#define PWMBIER_UIE_MSK                 BIT0
#define PWMB_EnableBreakInt()           SET_REG_BIT(PWMB_IER, PWMBIER_BIE_MSK)
#define PWMB_EnableTriggerInt()         SET_REG_BIT(PWMB_IER, PWMBIER_TIE_MSK)
#define PWMB_EnableCOMInt()             SET_REG_BIT(PWMB_IER, PWMBIER_COMIE_MSK)
#define PWMB_EnableCC8Int()             SET_REG_BIT(PWMB_IER, PWMBIER_CC8IE_MSK)
#define PWMB_EnableCC7Int()             SET_REG_BIT(PWMB_IER, PWMBIER_CC7IE_MSK)
#define PWMB_EnableCC6Int()             SET_REG_BIT(PWMB_IER, PWMBIER_CC6IE_MSK)
#define PWMB_EnableCC5Int()             SET_REG_BIT(PWMB_IER, PWMBIER_CC5IE_MSK)
#define PWMB_EnableUpdateInt()          SET_REG_BIT(PWMB_IER, PWMBIER_UIE_MSK)
#define PWMB_DisableBreakInt()          CLR_REG_BIT(PWMB_IER, PWMBIER_BIE_MSK)
#define PWMB_DisableTriggerInt()        CLR_REG_BIT(PWMB_IER, PWMBIER_TIE_MSK)
#define PWMB_DisableCOMInt()            CLR_REG_BIT(PWMB_IER, PWMBIER_COMIE_MSK)
#define PWMB_DisableCC8Int()            CLR_REG_BIT(PWMB_IER, PWMBIER_CC8IE_MSK)
#define PWMB_DisableCC7Int()            CLR_REG_BIT(PWMB_IER, PWMBIER_CC7IE_MSK)
#define PWMB_DisableCC6Int()            CLR_REG_BIT(PWMB_IER, PWMBIER_CC6IE_MSK)
#define PWMB_DisableCC5Int()            CLR_REG_BIT(PWMB_IER, PWMBIER_CC5IE_MSK)
#define PWMB_DisableUpdateInt()         CLR_REG_BIT(PWMB_IER, PWMBIER_UIE_MSK)

#define HSPWMB_EnableBreakInt()         SET_HSPWMB_BIT(PWMB_IER, PWMBIER_BIE_MSK)
#define HSPWMB_EnableTriggerInt()       SET_HSPWMB_BIT(PWMB_IER, PWMBIER_TIE_MSK)
#define HSPWMB_EnableCOMInt()           SET_HSPWMB_BIT(PWMB_IER, PWMBIER_COMIE_MSK)
#define HSPWMB_EnableCC8Int()           SET_HSPWMB_BIT(PWMB_IER, PWMBIER_CC8IE_MSK)
#define HSPWMB_EnableCC7Int()           SET_HSPWMB_BIT(PWMB_IER, PWMBIER_CC7IE_MSK)
#define HSPWMB_EnableCC6Int()           SET_HSPWMB_BIT(PWMB_IER, PWMBIER_CC6IE_MSK)
#define HSPWMB_EnableCC5Int()           SET_HSPWMB_BIT(PWMB_IER, PWMBIER_CC5IE_MSK)
#define HSPWMB_EnableUpdateInt()        SET_HSPWMB_BIT(PWMB_IER, PWMBIER_UIE_MSK)
#define HSPWMB_DisableBreakInt()        CLR_HSPWMB_BIT(PWMB_IER, PWMBIER_BIE_MSK)
#define HSPWMB_DisableTriggerInt()      CLR_HSPWMB_BIT(PWMB_IER, PWMBIER_TIE_MSK)
#define HSPWMB_DisableCOMInt()          CLR_HSPWMB_BIT(PWMB_IER, PWMBIER_COMIE_MSK)
#define HSPWMB_DisableCC8Int()          CLR_HSPWMB_BIT(PWMB_IER, PWMBIER_CC8IE_MSK)
#define HSPWMB_DisableCC7Int()          CLR_HSPWMB_BIT(PWMB_IER, PWMBIER_CC7IE_MSK)
#define HSPWMB_DisableCC6Int()          CLR_HSPWMB_BIT(PWMB_IER, PWMBIER_CC6IE_MSK)
#define HSPWMB_DisableCC5Int()          CLR_HSPWMB_BIT(PWMB_IER, PWMBIER_CC5IE_MSK)
#define HSPWMB_DisableUpdateInt()       CLR_HSPWMB_BIT(PWMB_IER, PWMBIER_UIE_MSK)

#define PORT_EnableInt(p, b)            SET_REG_BIT(P##p##INTE, (b))
#define PORT_DisableInt(p, b)           CLR_REG_BIT(P##p##INTE, (b))

#define EnableP0nInt(b)                 PORT_EnableInt(0, (b))
#define EnableP1nInt(b)                 PORT_EnableInt(1, (b))
#define EnableP2nInt(b)                 PORT_EnableInt(2, (b))
#define EnableP3nInt(b)                 PORT_EnableInt(3, (b))
#define EnableP4nInt(b)                 PORT_EnableInt(4, (b))
#define EnableP5nInt(b)                 PORT_EnableInt(5, (b))
#define EnableP6nInt(b)                 PORT_EnableInt(6, (b))
#define EnableP7nInt(b)                 PORT_EnableInt(7, (b))

#define DisableP0nInt(b)                PORT_DisableInt(0, (b))
#define DisableP1nInt(b)                PORT_DisableInt(1, (b))
#define DisableP2nInt(b)                PORT_DisableInt(2, (b))
#define DisableP3nInt(b)                PORT_DisableInt(3, (b))
#define DisableP4nInt(b)                PORT_DisableInt(4, (b))
#define DisableP5nInt(b)                PORT_DisableInt(5, (b))
#define DisableP6nInt(b)                PORT_DisableInt(6, (b))
#define DisableP7nInt(b)                PORT_DisableInt(7, (b))

#define CMOD_ECF_MSK                    BIT0
#define PCA_EnableCounterInt()          SET_REG_BIT(CMOD, CMOD_ECF_MSK)
#define PCA_DisableCounterInt()         CLR_REG_BIT(CMOD, CMOD_ECF_MSK)

#define CCAPM0_ECCF0_MSK                BIT0
#define CCAPM1_ECCF1_MSK                BIT0
#define CCAPM2_ECCF2_MSK                BIT0
#define PCA_EnableCCF0Int()             SET_REG_BIT(CCAPM0, CCAPM0_ECCF0_MSK)
#define PCA_EnableCCF1Int()             SET_REG_BIT(CCAPM1, CCAPM1_ECCF1_MSK)
#define PCA_EnableCCF2Int()             SET_REG_BIT(CCAPM2, CCAPM2_ECCF2_MSK)
#define PCA_DisableCCF0Int()            CLR_REG_BIT(CCAPM0, CCAPM0_ECCF0_MSK)
#define PCA_DisableCCF1Int()            CLR_REG_BIT(CCAPM1, CCAPM1_ECCF1_MSK)
#define PCA_DisableCCF2Int()            CLR_REG_BIT(CCAPM2, CCAPM2_ECCF2_MSK)

#define UR1TOCR_ENTOI_MSK               BIT6
#define UART1_EnableTimeoutInt()        SET_REG_BIT(UR1TOCR, UR1TOCR_ENTOI_MSK)
#define UART1_DisableTimeoutInt()       CLR_REG_BIT(UR1TOCR, UR1TOCR_ENTOI_MSK)

#define UR2TOCR_ENTOI_MSK               BIT6
#define UART2_EnableTimeoutInt()        SET_REG_BIT(UR2TOCR, UR2TOCR_ENTOI_MSK)
#define UART2_DisableTimeoutInt()       CLR_REG_BIT(UR2TOCR, UR2TOCR_ENTOI_MSK)

#define UR3TOCR_ENTOI_MSK               BIT6
#define UART3_EnableTimeoutInt()        SET_REG_BIT(UR3TOCR, UR3TOCR_ENTOI_MSK)
#define UART3_DisableTimeoutInt()       CLR_REG_BIT(UR3TOCR, UR3TOCR_ENTOI_MSK)

#define UR4TOCR_ENTOI_MSK               BIT6
#define UART4_EnableTimeoutInt()        SET_REG_BIT(UR4TOCR, UR4TOCR_ENTOI_MSK)
#define UART4_DisableTimeoutInt()       CLR_REG_BIT(UR4TOCR, UR4TOCR_ENTOI_MSK)

#define SPITOCR_ENTOI_MSK               BIT6
#define SPI_EnableTimeoutInt()          SET_REG_BIT(SPITOCR, SPITOCR_ENTOI_MSK)
#define SPI_DisableTimeoutInt()         CLR_REG_BIT(SPITOCR, SPITOCR_ENTOI_MSK)

#define I2CTOCR_ENTOI_MSK               BIT6
#define I2C_EnableTimeoutInt()          SET_REG_BIT(I2CTOCR, I2CTOCR_ENTOI_MSK)
#define I2C_DisableTimeoutInt()         CLR_REG_BIT(I2CTOCR, I2CTOCR_ENTOI_MSK)

#define I2SCR_TXEIE_MSK                 BIT7
#define I2SCR_RXNEIE_MSK                BIT6
#define I2SCR_ERRIE_MSK                 BIT5
#define I2S_EnableTXEInt()              SET_REG_BIT(I2SCR, I2SCR_TXEIE_MSK)
#define I2S_EnableRXNEInt()             SET_REG_BIT(I2SCR, I2SCR_RXNEIE_MSK)
#define I2S_EnableErrorInt()            SET_REG_BIT(I2SCR, I2SCR_ERRIE_MSK)
#define I2S_DisableTXEInt()             CLR_REG_BIT(I2SCR, I2SCR_TXEIE_MSK)
#define I2S_DisableRXNEInt()            CLR_REG_BIT(I2SCR, I2SCR_RXNEIE_MSK)
#define I2S_DisableErrorInt()           CLR_REG_BIT(I2SCR, I2SCR_ERRIE_MSK)

#define QSPICR3_SMIE_MSK                BIT3
#define QSPICR3_FTIE_MSK                BIT2
#define QSPICR3_TCIE_MSK                BIT1
#define QSPICR3_TEIE_MSK                BIT0
#define QSPI_EnableMatchInt()           SET_REG_BIT(QSPI_CR3, QSPICR3_SMIE_MSK)
#define QSPI_EnableFIFOInt()            SET_REG_BIT(QSPI_CR3, QSPICR3_FTIE_MSK)
#define QSPI_EnableTransferInt()        SET_REG_BIT(QSPI_CR3, QSPICR3_TCIE_MSK)
#define QSPI_EnableErrorInt()           SET_REG_BIT(QSPI_CR3, QSPICR3_TEIE_MSK)
#define QSPI_DisableMatchInt()          CLR_REG_BIT(QSPI_CR3, QSPICR3_SMIE_MSK)
#define QSPI_DisableFIFOInt()           CLR_REG_BIT(QSPI_CR3, QSPICR3_FTIE_MSK)
#define QSPI_DisableTransferInt()       CLR_REG_BIT(QSPI_CR3, QSPICR3_TCIE_MSK)
#define QSPI_DisableErrorInt()          CLR_REG_BIT(QSPI_CR3, QSPICR3_TEIE_MSK)

#define DMAM2MCFG_M2MIE_MSK             BIT7
#define DMAADCCFG_ADCIE_MSK             BIT7
#define DMASPICFG_SPIIE_MSK             BIT7
#define DMAQSPICFG_QSPIIE_MSK           BIT7
#define DMALCMCFG_LCMIE_MSK             BIT7
#define DMAUR1TCFG_UR1TIE_MSK           BIT7
#define DMAUR1RCFG_UR1RIE_MSK           BIT7
#define DMAUR2TCFG_UR2TIE_MSK           BIT7
#define DMAUR2RCFG_UR2RIE_MSK           BIT7
#define DMAUR3TCFG_UR3TIE_MSK           BIT7
#define DMAUR3RCFG_UR3RIE_MSK           BIT7
#define DMAUR4TCFG_UR4TIE_MSK           BIT7
#define DMAUR4RCFG_UR4RIE_MSK           BIT7
#define DMAI2CTCFG_I2CTIE_MSK           BIT7
#define DMAI2CRCFG_I2CRIE_MSK           BIT7
#define DMAI2STCFG_I2STIE_MSK           BIT7
#define DMAI2SRCFG_I2SRIE_MSK           BIT7
#define DMAPWMATCFG_PWMATIE_MSK         BIT7
#define DMAPWMARCFG_PWMARIE_MSK         BIT7
#define DMA_M2M_EnableInt()             SET_REG_BIT(DMA_M2M_CFG,   DMAM2MCFG_M2MIE_MSK)
#define DMA_ADC_EnableInt()             SET_REG_BIT(DMA_ADC_CFG,   DMAADCCFG_ADCIE_MSK)
#define DMA_SPI_EnableInt()             SET_REG_BIT(DMA_SPI_CFG,   DMASPICFG_SPIIE_MSK)
#define DMA_QSPI_EnableInt()            SET_REG_BIT(DMA_QSPI_CFG,  DMAQSPICFG_QSPIIE_MSK)
#define DMA_LCM_EnableInt()             SET_REG_BIT(DMA_LCM_CFG,   DMALCMCFG_LCMIE_MSK)
#define DMA_UART1_EnableTxInt()         SET_REG_BIT(DMA_UR1T_CFG,  DMAUR1TCFG_UR1TIE_MSK)
#define DMA_UART1_EnableRxInt()         SET_REG_BIT(DMA_UR1R_CFG,  DMAUR1RCFG_UR1RIE_MSK)
#define DMA_UART2_EnableTxInt()         SET_REG_BIT(DMA_UR2T_CFG,  DMAUR2TCFG_UR2TIE_MSK)
#define DMA_UART2_EnableRxInt()         SET_REG_BIT(DMA_UR2R_CFG,  DMAUR2RCFG_UR2RIE_MSK)
#define DMA_UART3_EnableTxInt()         SET_REG_BIT(DMA_UR3T_CFG,  DMAUR3TCFG_UR3TIE_MSK)
#define DMA_UART3_EnableRxInt()         SET_REG_BIT(DMA_UR3R_CFG,  DMAUR3RCFG_UR3RIE_MSK)
#define DMA_UART4_EnableTxInt()         SET_REG_BIT(DMA_UR4T_CFG,  DMAUR4TCFG_UR4TIE_MSK)
#define DMA_UART4_EnableRxInt()         SET_REG_BIT(DMA_UR4R_CFG,  DMAUR4RCFG_UR4RIE_MSK)
#define DMA_I2C_EnableTxInt()           SET_REG_BIT(DMA_I2CT_CFG,  DMAI2CTCFG_I2CTIE_MSK)
#define DMA_I2C_EnableRxInt()           SET_REG_BIT(DMA_I2CR_CFG,  DMAI2CRCFG_I2CRIE_MSK)
#define DMA_I2S_EnableTxInt()           SET_REG_BIT(DMA_I2ST_CFG,  DMAI2STCFG_I2STIE_MSK)
#define DMA_I2S_EnableRxInt()           SET_REG_BIT(DMA_I2SR_CFG,  DMAI2SRCFG_I2SRIE_MSK)
#define DMA_PWMA_EnableTxInt()          SET_REG_BIT(DMA_PWMAT_CFG, DMAPWMATCFG_PWMATIE_MSK)
#define DMA_PWMA_EnableRxInt()          SET_REG_BIT(DMA_PWMAR_CFG, DMAPWMARCFG_PWMARIE_MSK)
#define DMA_M2M_DisableInt()            CLR_REG_BIT(DMA_M2M_CFG,   DMAM2MCFG_M2MIE_MSK)
#define DMA_ADC_DisableInt()            CLR_REG_BIT(DMA_ADC_CFG,   DMAADCCFG_ADCIE_MSK)
#define DMA_SPI_DisableInt()            CLR_REG_BIT(DMA_SPI_CFG,   DMASPICFG_SPIIE_MSK)
#define DMA_QSPI_DisableInt()           CLR_REG_BIT(DMA_QSPI_CFG,  DMAQSPICFG_QSPIIE_MSK)
#define DMA_LCM_DisableInt()            CLR_REG_BIT(DMA_LCM_CFG,   DMALCMCFG_LCMIE_MSK)
#define DMA_UART1_DisableTxInt()        CLR_REG_BIT(DMA_UR1T_CFG,  DMAUR1TCFG_UR1TIE_MSK)
#define DMA_UART1_DisableRxInt()        CLR_REG_BIT(DMA_UR1R_CFG,  DMAUR1RCFG_UR1RIE_MSK)
#define DMA_UART2_DisableTxInt()        CLR_REG_BIT(DMA_UR2T_CFG,  DMAUR2TCFG_UR2TIE_MSK)
#define DMA_UART2_DisableRxInt()        CLR_REG_BIT(DMA_UR2R_CFG,  DMAUR2RCFG_UR2RIE_MSK)
#define DMA_UART3_DisableTxInt()        CLR_REG_BIT(DMA_UR3T_CFG,  DMAUR3TCFG_UR3TIE_MSK)
#define DMA_UART3_DisableRxInt()        CLR_REG_BIT(DMA_UR3R_CFG,  DMAUR3RCFG_UR3RIE_MSK)
#define DMA_UART4_DisableTxInt()        CLR_REG_BIT(DMA_UR4T_CFG,  DMAUR4TCFG_UR4TIE_MSK)
#define DMA_UART4_DisableRxInt()        CLR_REG_BIT(DMA_UR4R_CFG,  DMAUR4RCFG_UR4RIE_MSK)
#define DMA_I2C_DisableTxInt()          CLR_REG_BIT(DMA_I2CT_CFG,  DMAI2CTCFG_I2CTIE_MSK)
#define DMA_I2C_DisableRxInt()          CLR_REG_BIT(DMA_I2CR_CFG,  DMAI2CRCFG_I2CRIE_MSK)
#define DMA_I2S_DisableTxInt()          CLR_REG_BIT(DMA_I2ST_CFG,  DMAI2STCFG_I2STIE_MSK)
#define DMA_I2S_DisableRxInt()          CLR_REG_BIT(DMA_I2SR_CFG,  DMAI2SRCFG_I2SRIE_MSK)
#define DMA_PWMA_DisableTxInt()         CLR_REG_BIT(DMA_PWMAT_CFG, DMAPWMATCFG_PWMATIE_MSK)
#define DMA_PWMA_DisableRxInt()         CLR_REG_BIT(DMA_PWMAR_CFG, DMAPWMARCFG_PWMARIE_MSK)

#define INT0_FallingRisingInt()         (IT0 = 0)
#define INT0_FallingInt()               (IT0 = 1)

#define INT0_CheckFlag()                (IE0)
#define INT0_ClearFlag()                (IE0 = 0)

#define INT1_FallingRisingInt()         (IT1 = 0)
#define INT1_FallingInt()               (IT1 = 1)

#define INT1_CheckFlag()                (IE1)
#define INT1_ClearFlag()                (IE1 = 0)

#define AUXINTIF_INT2IF_MSK             BIT4
#define INT2_CheckFlag()                READ_REG_BIT(AUXINTIF, AUXINTIF_INT2IF_MSK)
#define INT2_ClearFlag()                CLR_REG_BIT(AUXINTIF, AUXINTIF_INT2IF_MSK)

#define AUXINTIF_INT3IF_MSK             BIT5
#define INT3_CheckFlag()                READ_REG_BIT(AUXINTIF, AUXINTIF_INT3IF_MSK)
#define INT3_ClearFlag()                CLR_REG_BIT(AUXINTIF, AUXINTIF_INT3IF_MSK)

#define AUXINTIF_INT4IF_MSK             BIT6
#define INT4_CheckFlag()                READ_REG_BIT(AUXINTIF, AUXINTIF_INT4IF_MSK)
#define INT4_ClearFlag()                CLR_REG_BIT(AUXINTIF, AUXINTIF_INT4IF_MSK)

#define TIMER0_CheckFlag()              (TF0)
#define TIMER0_ClearFlag()              (TF0 = 0)

#define TIMER1_CheckFlag()              (TF1)
#define TIMER1_ClearFlag()              (TF1 = 0)

#define AUXINTIF_T2IF_MSK               BIT0
#define TIMER2_CheckFlag()              READ_REG_BIT(AUXINTIF, AUXINTIF_T2IF_MSK)
#define TIMER2_ClearFlag()              CLR_REG_BIT(AUXINTIF, AUXINTIF_T2IF_MSK)

#define AUXINTIF_T3IF_MSK               BIT1
#define TIMER3_CheckFlag()              READ_REG_BIT(AUXINTIF, AUXINTIF_T3IF_MSK)
#define TIMER3_ClearFlag()              CLR_REG_BIT(AUXINTIF, AUXINTIF_T3IF_MSK)

#define AUXINTIF_T4IF_MSK               BIT2
#define TIMER4_CheckFlag()              READ_REG_BIT(AUXINTIF, AUXINTIF_T4IF_MSK)
#define TIMER4_ClearFlag()              CLR_REG_BIT(AUXINTIF, AUXINTIF_T4IF_MSK)

#define T11CR_T11IF_MSK                 BIT0
#define TIMER11_CheckFlag()             READ_REG_BIT(T11CR, T11CR_T11IF_MSK)
#define TIMER11_ClearFlag()             CLR_REG_BIT(T11CR, T11CR_T11IF_MSK)

#define UART1_CheckRxFlag()             (RI)
#define UART1_CheckTxFlag()             (TI)
#define UART1_ClearRxFlag()             (RI = 0)
#define UART1_ClearTxFlag()             (TI = 0)
#define UART1_SetRxFlag()               (RI = 1)
#define UART1_SetTxFlag()               (TI = 1)

#define S2CON_S2TI_MSK                  BIT1
#define S2CON_S2RI_MSK                  BIT0
#define UART2_CheckRxFlag()             READ_REG_BIT(S2CON, S2CON_S2RI_MSK)
#define UART2_CheckTxFlag()             READ_REG_BIT(S2CON, S2CON_S2TI_MSK)
#define UART2_ClearRxFlag()             CLR_REG_BIT(S2CON, S2CON_S2RI_MSK)
#define UART2_ClearTxFlag()             CLR_REG_BIT(S2CON, S2CON_S2TI_MSK)
#define UART2_SetRxFlag()               SET_REG_BIT(S2CON, S2CON_S2RI_MSK)
#define UART2_SetTxFlag()               SET_REG_BIT(S2CON, S2CON_S2TI_MSK)

#define S3CON_S3TI_MSK                  BIT1
#define S3CON_S3RI_MSK                  BIT0
#define UART3_CheckRxFlag()             READ_REG_BIT(S3CON, S3CON_S3RI_MSK)
#define UART3_CheckTxFlag()             READ_REG_BIT(S3CON, S3CON_S3TI_MSK)
#define UART3_ClearRxFlag()             CLR_REG_BIT(S3CON, S3CON_S3RI_MSK)
#define UART3_ClearTxFlag()             CLR_REG_BIT(S3CON, S3CON_S3TI_MSK)
#define UART3_SetRxFlag()               SET_REG_BIT(S3CON, S3CON_S3RI_MSK)
#define UART3_SetTxFlag()               SET_REG_BIT(S3CON, S3CON_S3TI_MSK)

#define S4CON_S4TI_MSK                  BIT1
#define S4CON_S4RI_MSK                  BIT0
#define UART4_CheckRxFlag()             READ_REG_BIT(S4CON, S4CON_S4RI_MSK)
#define UART4_CheckTxFlag()             READ_REG_BIT(S4CON, S4CON_S4TI_MSK)
#define UART4_ClearRxFlag()             CLR_REG_BIT(S4CON, S4CON_S4RI_MSK)
#define UART4_ClearTxFlag()             CLR_REG_BIT(S4CON, S4CON_S4TI_MSK)
#define UART4_SetRxFlag()               SET_REG_BIT(S4CON, S4CON_S4RI_MSK)
#define UART4_SetTxFlag()               SET_REG_BIT(S4CON, S4CON_S4TI_MSK)

#define PCON_LVDF_MSK                   BIT5
#define LVD_CheckFlag()                 READ_REG_BIT(PCON, PCON_LVDF_MSK)
#define LVD_ClearFlag()                 CLR_REG_BIT(PCON, PCON_LVDF_MSK)

#define ADCCONTR_ADCFLAG_MSK            BIT5
#define ADC_CheckFlag()                 READ_REG_BIT(ADC_CONTR, ADCCONTR_ADCFLAG_MSK)
#define ADC_ClearFlag()                 CLR_REG_BIT(ADC_CONTR, ADCCONTR_ADCFLAG_MSK)

#define SPSTAT_SPIF_MSK                 BIT7
#define SPSTAT_WCOL_MSK                 BIT6
#define SPI_CheckFlag()                 READ_REG_BIT(SPSTAT, SPSTAT_SPIF_MSK)
#define SPI_ClearFlag()                 SET_REG_BIT(SPSTAT, SPSTAT_SPIF_MSK | SPSTAT_WCOL_MSK)

#define CMPCR1_CMPIF_MSK                BIT6
#define CMP_CheckFlag()                 READ_REG_BIT(CMPCR1, CMPCR1_CMPIF_MSK)
#define CMP_ClearFlag()                 CLR_REG_BIT(CMPCR1, CMPCR1_CMPIF_MSK)

#define RTCIF_ALARM_MSK                 BIT7
#define RTCIF_DAY_MSK                   BIT6
#define RTCIF_HOUR_MSK                  BIT5
#define RTCIF_MIN_MSK                   BIT4
#define RTCIF_SEC_MSK                   BIT3
#define RTCIF_SEC2_MSK                  BIT2
#define RTCIF_SEC8_MSK                  BIT1
#define RTCIF_SEC32_MSK                 BIT0
#define RTC_CheckAlarmFlag()            READ_REG_BIT(RTCIF, RTCIF_ALARM_MSK)
#define RTC_CheckDayFlag()              READ_REG_BIT(RTCIF, RTCIF_DAY_MSK)
#define RTC_CheckHourFlag()             READ_REG_BIT(RTCIF, RTCIF_HOUR_MSK)
#define RTC_CheckMinuteFlag()           READ_REG_BIT(RTCIF, RTCIF_MIN_MSK)
#define RTC_CheckSecondFlag()           READ_REG_BIT(RTCIF, RTCIF_SEC_MSK)
#define RTC_CheckSecondD2Flag()         READ_REG_BIT(RTCIF, RTCIF_SEC2_MSK)
#define RTC_CheckSecondD8Flag()         READ_REG_BIT(RTCIF, RTCIF_SEC8_MSK)
#define RTC_CheckSecondD32Flag()        READ_REG_BIT(RTCIF, RTCIF_SEC32_MSK)
#define RTC_ClearAlarmFlag()            CLR_REG_BIT(RTCIF, RTCIF_ALARM_MSK)
#define RTC_ClearDayFlag()              CLR_REG_BIT(RTCIF, RTCIF_DAY_MSK)
#define RTC_ClearHourFlag()             CLR_REG_BIT(RTCIF, RTCIF_HOUR_MSK)
#define RTC_ClearMinuteFlag()           CLR_REG_BIT(RTCIF, RTCIF_MIN_MSK)
#define RTC_ClearSecondFlag()           CLR_REG_BIT(RTCIF, RTCIF_SEC_MSK)
#define RTC_ClearSecondD2Flag()         CLR_REG_BIT(RTCIF, RTCIF_SEC2_MSK)
#define RTC_ClearSecondD8Flag()         CLR_REG_BIT(RTCIF, RTCIF_SEC8_MSK)
#define RTC_ClearSecondD32Flag()        CLR_REG_BIT(RTCIF, RTCIF_SEC32_MSK)

#define LCMIFIF_MSK                     BIT0
#define LCM_CheckFlag()                 READ_REG_BIT(LCMIFSTA, LCMIFIF_MSK)
#define LCM_ClearFlag()                 CLR_REG_BIT(LCMIFSTA, LCMIFIF_MSK)

#define I2CMSST_MSIF_MSK                BIT6
#define I2C_CheckMasterFlag()           READ_REG_BIT(I2CMSST, I2CMSST_MSIF_MSK)
#define I2C_ClearMasterFlag()           CLR_REG_BIT(I2CMSST, I2CMSST_MSIF_MSK)

#define I2CSLST_STAIF_MSK               BIT6
#define I2CSLST_RXIF_MSK                BIT5
#define I2CSLST_TXIF_MSK                BIT4
#define I2CSLST_STOIF_MSK               BIT3
#define I2C_CheckSlaveSTAFlag()         READ_REG_BIT(I2CSLST, I2CSLST_STAIF_MSK)
#define I2C_CheckSlaveSTOFlag()         READ_REG_BIT(I2CSLST, I2CSLST_STOIF_MSK)
#define I2C_CheckSlaveRXFlag()          READ_REG_BIT(I2CSLST, I2CSLST_RXIF_MSK)
#define I2C_CheckSlaveTXFlag()          READ_REG_BIT(I2CSLST, I2CSLST_TXIF_MSK)
#define I2C_CheckSlaveAllFlag()         READ_REG_BIT(I2CSLST, I2CSLST_STAIF_MSK | I2CSLST_STOIT_MSK | I2CSLST_RXIT_MSK | I2CSLST_TXIF_MSK)
#define I2C_ClearSlaveSTAFlag()         CLR_REG_BIT(I2CSLST, I2CSLST_STAIF_MSK)
#define I2C_ClearSlaveSTOFlag()         CLR_REG_BIT(I2CSLST, I2CSLST_STOIF_MSK)
#define I2C_ClearSlaveRXFlag()          CLR_REG_BIT(I2CSLST, I2CSLST_RXIF_MSK)
#define I2C_ClearSlaveTXFlag()          CLR_REG_BIT(I2CSLST, I2CSLST_TXIF_MSK)
#define I2C_ClearSlaveAllFlag()         CLR_REG_BIT(I2CSLST, I2CSLST_STAIF_MSK | I2CSLST_STOIT_MSK | I2CSLST_RXIT_MSK | I2CSLST_TXIF_MSK)

#define PWMASR1_BIF_MSK                 BIT7
#define PWMASR1_TIF_MSK                 BIT6
#define PWMASR1_COMIF_MSK               BIT5
#define PWMASR1_CC4IF_MSK               BIT4
#define PWMASR1_CC3IF_MSK               BIT3
#define PWMASR1_CC2IF_MSK               BIT2
#define PWMASR1_CC1IF_MSK               BIT1
#define PWMASR1_UIF_MSK                 BIT0
#define PWMA_CheckBreakFlag()           READ_REG_BIT(PWMA_SR1, PWMASR1_BIF_MSK)
#define PWMA_CheckTriggerFlag()         READ_REG_BIT(PWMA_SR1, PWMASR1_TIF_MSK)
#define PWMA_CheckCOMFlag()             READ_REG_BIT(PWMA_SR1, PWMASR1_COMIF_MSK)
#define PWMA_CheckCC4Flag()             READ_REG_BIT(PWMA_SR1, PWMASR1_CC4IF_MSK)
#define PWMA_CheckCC3Flag()             READ_REG_BIT(PWMA_SR1, PWMASR1_CC3IF_MSK)
#define PWMA_CheckCC2Flag()             READ_REG_BIT(PWMA_SR1, PWMASR1_CC2IF_MSK)
#define PWMA_CheckCC1Flag()             READ_REG_BIT(PWMA_SR1, PWMASR1_CC1IF_MSK)
#define PWMA_CheckUpdateFlag()          READ_REG_BIT(PWMA_SR1, PWMASR1_UIF_MSK)
#define PWMA_ClearBreakFlag()           CLR_REG_BIT(PWMA_SR1, PWMASR1_BIF_MSK)
#define PWMA_ClearTriggerFlag()         CLR_REG_BIT(PWMA_SR1, PWMASR1_TIF_MSK)
#define PWMA_ClearCOMFlag()             CLR_REG_BIT(PWMA_SR1, PWMASR1_COMIF_MSK)
#define PWMA_ClearCC4Flag()             CLR_REG_BIT(PWMA_SR1, PWMASR1_CC4IF_MSK)
#define PWMA_ClearCC3Flag()             CLR_REG_BIT(PWMA_SR1, PWMASR1_CC3IF_MSK)
#define PWMA_ClearCC2Flag()             CLR_REG_BIT(PWMA_SR1, PWMASR1_CC2IF_MSK)
#define PWMA_ClearCC1Flag()             CLR_REG_BIT(PWMA_SR1, PWMASR1_CC1IF_MSK)
#define PWMA_ClearUpdateFlag()          CLR_REG_BIT(PWMA_SR1, PWMASR1_UIF_MSK)

#define HSPWMA_CheckBreakFlag()         READ_HSPWMA_BIT(PWMA_SR1, PWMASR1_BIF_MSK)
#define HSPWMA_CheckTriggerFlag()       READ_HSPWMA_BIT(PWMA_SR1, PWMASR1_TIF_MSK)
#define HSPWMA_CheckCOMFlag()           READ_HSPWMA_BIT(PWMA_SR1, PWMASR1_COMIF_MSK)
#define HSPWMA_CheckCC4Flag()           READ_HSPWMA_BIT(PWMA_SR1, PWMASR1_CC4IF_MSK)
#define HSPWMA_CheckCC3Flag()           READ_HSPWMA_BIT(PWMA_SR1, PWMASR1_CC3IF_MSK)
#define HSPWMA_CheckCC2Flag()           READ_HSPWMA_BIT(PWMA_SR1, PWMASR1_CC2IF_MSK)
#define HSPWMA_CheckCC1Flag()           READ_HSPWMA_BIT(PWMA_SR1, PWMASR1_CC1IF_MSK)
#define HSPWMA_CheckUpdateFlag()        READ_HSPWMA_BIT(PWMA_SR1, PWMASR1_UIF_MSK)
#define HSPWMA_ClearBreakFlag()         CLR_HSPWMA_BIT(PWMA_SR1, PWMASR1_BIF_MSK)
#define HSPWMA_ClearTriggerFlag()       CLR_HSPWMA_BIT(PWMA_SR1, PWMASR1_TIF_MSK)
#define HSPWMA_ClearCOMFlag()           CLR_HSPWMA_BIT(PWMA_SR1, PWMASR1_COMIF_MSK)
#define HSPWMA_ClearCC4Flag()           CLR_HSPWMA_BIT(PWMA_SR1, PWMASR1_CC4IF_MSK)
#define HSPWMA_ClearCC3Flag()           CLR_HSPWMA_BIT(PWMA_SR1, PWMASR1_CC3IF_MSK)
#define HSPWMA_ClearCC2Flag()           CLR_HSPWMA_BIT(PWMA_SR1, PWMASR1_CC2IF_MSK)
#define HSPWMA_ClearCC1Flag()           CLR_HSPWMA_BIT(PWMA_SR1, PWMASR1_CC1IF_MSK)
#define HSPWMA_ClearUpdateFlag()        CLR_HSPWMA_BIT(PWMA_SR1, PWMASR1_UIF_MSK)

#define PWMBSR1_BIF_MSK                 BIT7
#define PWMBSR1_TIF_MSK                 BIT6
#define PWMBSR1_COMIF_MSK               BIT5
#define PWMBSR1_CC8IF_MSK               BIT4
#define PWMBSR1_CC7IF_MSK               BIT3
#define PWMBSR1_CC6IF_MSK               BIT2
#define PWMBSR1_CC5IF_MSK               BIT1
#define PWMBSR1_UIF_MSK                 BIT0
#define PWMB_CheckBreakFlag()           READ_REG_BIT(PWMB_SR1, PWMBSR1_BIF_MSK)
#define PWMB_CheckTriggerFlag()         READ_REG_BIT(PWMB_SR1, PWMBSR1_TIF_MSK)
#define PWMB_CheckCOMFlag()             READ_REG_BIT(PWMB_SR1, PWMBSR1_COMIF_MSK)
#define PWMB_CheckCC8Flag()             READ_REG_BIT(PWMB_SR1, PWMBSR1_CC8IF_MSK)
#define PWMB_CheckCC7Flag()             READ_REG_BIT(PWMB_SR1, PWMBSR1_CC7IF_MSK)
#define PWMB_CheckCC6Flag()             READ_REG_BIT(PWMB_SR1, PWMBSR1_CC6IF_MSK)
#define PWMB_CheckCC5Flag()             READ_REG_BIT(PWMB_SR1, PWMBSR1_CC5IF_MSK)
#define PWMB_CheckUpdateFlag()          READ_REG_BIT(PWMB_SR1, PWMBSR1_UIF_MSK)
#define PWMB_ClearBreakFlag()           CLR_REG_BIT(PWMB_SR1, PWMBSR1_BIF_MSK)
#define PWMB_ClearTriggerFlag()         CLR_REG_BIT(PWMB_SR1, PWMBSR1_TIF_MSK)
#define PWMB_ClearCOMFlag()             CLR_REG_BIT(PWMB_SR1, PWMBSR1_COMIF_MSK)
#define PWMB_ClearCC8Flag()             CLR_REG_BIT(PWMB_SR1, PWMBSR1_CC8IF_MSK)
#define PWMB_ClearCC7Flag()             CLR_REG_BIT(PWMB_SR1, PWMBSR1_CC7IF_MSK)
#define PWMB_ClearCC6Flag()             CLR_REG_BIT(PWMB_SR1, PWMBSR1_CC6IF_MSK)
#define PWMB_ClearCC5Flag()             CLR_REG_BIT(PWMB_SR1, PWMBSR1_CC5IF_MSK)
#define PWMB_ClearUpdateFlag()          CLR_REG_BIT(PWMB_SR1, PWMBSR1_UIF_MSK)

#define HSPWMB_CheckBreakFlag()         READ_HSPWMB_BIT(PWMB_SR1, PWMBSR1_BIF_MSK)
#define HSPWMB_CheckTriggerFlag()       READ_HSPWMB_BIT(PWMB_SR1, PWMBSR1_TIF_MSK)
#define HSPWMB_CheckCOMFlag()           READ_HSPWMB_BIT(PWMB_SR1, PWMBSR1_COMIF_MSK)
#define HSPWMB_CheckCC8Flag()           READ_HSPWMB_BIT(PWMB_SR1, PWMBSR1_CC8IF_MSK)
#define HSPWMB_CheckCC7Flag()           READ_HSPWMB_BIT(PWMB_SR1, PWMBSR1_CC7IF_MSK)
#define HSPWMB_CheckCC6Flag()           READ_HSPWMB_BIT(PWMB_SR1, PWMBSR1_CC6IF_MSK)
#define HSPWMB_CheckCC5Flag()           READ_HSPWMB_BIT(PWMB_SR1, PWMBSR1_CC5IF_MSK)
#define HSPWMB_CheckUpdateFlag()        READ_HSPWMB_BIT(PWMB_SR1, PWMBSR1_UIF_MSK)
#define HSPWMB_ClearBreakFlag()         CLR_HSPWMB_BIT(PWMB_SR1, PWMBSR1_BIF_MSK)
#define HSPWMB_ClearTriggerFlag()       CLR_HSPWMB_BIT(PWMB_SR1, PWMBSR1_TIF_MSK)
#define HSPWMB_ClearCOMFlag()           CLR_HSPWMB_BIT(PWMB_SR1, PWMBSR1_COMIF_MSK)
#define HSPWMB_ClearCC8Flag()           CLR_HSPWMB_BIT(PWMB_SR1, PWMBSR1_CC8IF_MSK)
#define HSPWMB_ClearCC7Flag()           CLR_HSPWMB_BIT(PWMB_SR1, PWMBSR1_CC7IF_MSK)
#define HSPWMB_ClearCC6Flag()           CLR_HSPWMB_BIT(PWMB_SR1, PWMBSR1_CC6IF_MSK)
#define HSPWMB_ClearCC5Flag()           CLR_HSPWMB_BIT(PWMB_SR1, PWMBSR1_CC5IF_MSK)
#define HSPWMB_ClearUpdateFlag()        CLR_HSPWMB_BIT(PWMB_SR1, PWMBSR1_UIF_MSK)

#define PWMASR2_CC4OF_MSK               BIT4
#define PWMASR2_CC3OF_MSK               BIT3
#define PWMASR2_CC2OF_MSK               BIT2
#define PWMASR2_CC1OF_MSK               BIT1
#define PWMA_CheckCC4OverFlag()         READ_REG_BIT(PWMA_SR2, PWMASR1_CC4OF_MSK)
#define PWMA_CheckCC3OverFlag()         READ_REG_BIT(PWMA_SR2, PWMASR1_CC3OF_MSK)
#define PWMA_CheckCC2OverFlag()         READ_REG_BIT(PWMA_SR2, PWMASR1_CC2OF_MSK)
#define PWMA_CheckCC1OverFlag()         READ_REG_BIT(PWMA_SR2, PWMASR1_CC1OF_MSK)
#define PWMA_ClearCC4OverFlag()         CLR_REG_BIT(PWMA_SR2, PWMASR1_CC4OF_MSK)
#define PWMA_ClearCC3OverFlag()         CLR_REG_BIT(PWMA_SR2, PWMASR1_CC3OF_MSK)
#define PWMA_ClearCC2OverFlag()         CLR_REG_BIT(PWMA_SR2, PWMASR1_CC2OF_MSK)
#define PWMA_ClearCC1OverFlag()         CLR_REG_BIT(PWMA_SR2, PWMASR1_CC1OF_MSK)

#define HSPWMA_CheckCC4OverFlag()       READ_HSPWMA_BIT(PWMA_SR2, PWMASR1_CC4OF_MSK)
#define HSPWMA_CheckCC3OverFlag()       READ_HSPWMA_BIT(PWMA_SR2, PWMASR1_CC3OF_MSK)
#define HSPWMA_CheckCC2OverFlag()       READ_HSPWMA_BIT(PWMA_SR2, PWMASR1_CC2OF_MSK)
#define HSPWMA_CheckCC1OverFlag()       READ_HSPWMA_BIT(PWMA_SR2, PWMASR1_CC1OF_MSK)
#define HSPWMA_ClearCC4OverFlag()       CLR_HSPWMA_BIT(PWMA_SR2, PWMASR1_CC4OF_MSK)
#define HSPWMA_ClearCC3OverFlag()       CLR_HSPWMA_BIT(PWMA_SR2, PWMASR1_CC3OF_MSK)
#define HSPWMA_ClearCC2OverFlag()       CLR_HSPWMA_BIT(PWMA_SR2, PWMASR1_CC2OF_MSK)
#define HSPWMA_ClearCC1OverFlag()       CLR_HSPWMA_BIT(PWMA_SR2, PWMASR1_CC1OF_MSK)

#define PWMBSR2_CC8OF_MSK               BIT4
#define PWMBSR2_CC7OF_MSK               BIT3
#define PWMBSR2_CC6OF_MSK               BIT2
#define PWMBSR2_CC5OF_MSK               BIT1
#define PWMB_CheckCC8OverFlag()         READ_REG_BIT(PWMB_SR2, PWMASR1_CC8OF_MSK)
#define PWMB_CheckCC7OverFlag()         READ_REG_BIT(PWMB_SR2, PWMASR1_CC7OF_MSK)
#define PWMB_CheckCC6OverFlag()         READ_REG_BIT(PWMB_SR2, PWMASR1_CC6OF_MSK)
#define PWMB_CheckCC5OverFlag()         READ_REG_BIT(PWMB_SR2, PWMASR1_CC5OF_MSK)
#define PWMB_ClearCC8OverFlag()         CLR_REG_BIT(PWMB_SR2, PWMASR1_CC8OF_MSK)
#define PWMB_ClearCC7OverFlag()         CLR_REG_BIT(PWMB_SR2, PWMASR1_CC7OF_MSK)
#define PWMB_ClearCC6OverFlag()         CLR_REG_BIT(PWMB_SR2, PWMASR1_CC6OF_MSK)
#define PWMB_ClearCC5OverFlag()         CLR_REG_BIT(PWMB_SR2, PWMASR1_CC5OF_MSK)

#define HSPWMB_CheckCC8OverFlag()       READ_HSPWMB_BIT(PWMB_SR2, PWMASR1_CC8OF_MSK)
#define HSPWMB_CheckCC7OverFlag()       READ_HSPWMB_BIT(PWMB_SR2, PWMASR1_CC7OF_MSK)
#define HSPWMB_CheckCC6OverFlag()       READ_HSPWMB_BIT(PWMB_SR2, PWMASR1_CC6OF_MSK)
#define HSPWMB_CheckCC5OverFlag()       READ_HSPWMB_BIT(PWMB_SR2, PWMASR1_CC5OF_MSK)
#define HSPWMB_ClearCC8OverFlag()       CLR_HSPWMB_BIT(PWMB_SR2, PWMASR1_CC8OF_MSK)
#define HSPWMB_ClearCC7OverFlag()       CLR_HSPWMB_BIT(PWMB_SR2, PWMASR1_CC7OF_MSK)
#define HSPWMB_ClearCC6OverFlag()       CLR_HSPWMB_BIT(PWMB_SR2, PWMASR1_CC6OF_MSK)
#define HSPWMB_ClearCC5OverFlag()       CLR_HSPWMB_BIT(PWMB_SR2, PWMASR1_CC5OF_MSK)

#define PORT_CheckFlag(p, b)            READ_REG_BIT(P##p##INTF, (b))
#define PORT_ClearFlag(p, b)            CLR_REG_BIT(P##p##INTF, (b))

#define CheckP0nFlag(b)                 PORT_CheckFlag(0, (b))
#define CheckP1nFlag(b)                 PORT_CheckFlag(1, (b))
#define CheckP2nFlag(b)                 PORT_CheckFlag(2, (b))
#define CheckP3nFlag(b)                 PORT_CheckFlag(3, (b))
#define CheckP4nFlag(b)                 PORT_CheckFlag(4, (b))
#define CheckP5nFlag(b)                 PORT_CheckFlag(5, (b))
#define CheckP6nFlag(b)                 PORT_CheckFlag(6, (b))
#define CheckP7nFlag(b)                 PORT_CheckFlag(7, (b))
#define ClearP0nFlag(b)                 PORT_ClearFlag(0, (b))
#define ClearP1nFlag(b)                 PORT_ClearFlag(1, (b))
#define ClearP2nFlag(b)                 PORT_ClearFlag(2, (b))
#define ClearP3nFlag(b)                 PORT_ClearFlag(3, (b))
#define ClearP4nFlag(b)                 PORT_ClearFlag(4, (b))
#define ClearP5nFlag(b)                 PORT_ClearFlag(5, (b))
#define ClearP6nFlag(b)                 PORT_ClearFlag(6, (b))
#define ClearP7nFlag(b)                 PORT_ClearFlag(7, (b))

#define CCON_CF_MSK                     BIT7
#define PCA_CheckCounterFlag()          READ_REG_BIT(CCON, CCON_CF_MSK)
#define PCA_ClearCounterFlag()          CLR_REG_BIT(CCON, CCON_CF_MSK)

#define CCON_CCF0_MSK                   BIT0
#define CCON_CCF1_MSK                   BIT1
#define CCON_CCF2_MSK                   BIT2
#define PCA_CheckCCF0Flag()             READ_REG_BIT(CCON, CCON_CCF0_MSK)
#define PCA_CheckCCF1Flag()             READ_REG_BIT(CCON, CCON_CCF1_MSK)
#define PCA_CheckCCF2Flag()             READ_REG_BIT(CCON, CCON_CCF2_MSK)
#define PCA_ClearCCF0Flag()             CLR_REG_BIT(CCON, CCON_CCF0_MSK)
#define PCA_ClearCCF1Flag()             CLR_REG_BIT(CCON, CCON_CCF1_MSK)
#define PCA_ClearCCF2Flag()             CLR_REG_BIT(CCON, CCON_CCF2_MSK)

#define UR1TOSR_TOIF_MSK                BIT0
#define UR2TOSR_TOIF_MSK                BIT0
#define UR3TOSR_TOIF_MSK                BIT0
#define UR4TOSR_TOIF_MSK                BIT0
#define SPITOSR_TOIF_MSK                BIT0
#define I2CTOSR_TOIF_MSK                BIT0
#define UART1_CheckTimeoutFlag()        READ_REG_BIT(UR1TOSR, UR1TOSR_TOIF_MSK)
#define UART2_CheckTimeoutFlag()        READ_REG_BIT(UR2TOSR, UR2TOSR_TOIF_MSK)
#define UART3_CheckTimeoutFlag()        READ_REG_BIT(UR3TOSR, UR3TOSR_TOIF_MSK)
#define UART4_CheckTimeoutFlag()        READ_REG_BIT(UR4TOSR, UR4TOSR_TOIF_MSK)
#define SPI_CheckTimeoutFlag()          READ_REG_BIT(SPITOSR, SPITOSR_TOIF_MSK)
#define I2C_CheckTimeoutFlag()          READ_REG_BIT(I2CTOSR, I2CTOSR_TOIF_MSK)

#define UR1TOSR_CTOIF_MSK               BIT7
#define UR2TOSR_CTOIF_MSK               BIT7
#define UR3TOSR_CTOIF_MSK               BIT7
#define UR4TOSR_CTOIF_MSK               BIT7
#define SPITOSR_CTOIF_MSK               BIT7
#define I2CTOSR_CTOIF_MSK               BIT7
#define UART1_ClearTimeoutFlag()        SET_REG_BIT(UR1TOSR, UR1TOSR_CTOIF_MSK)
#define UART2_ClearTimeoutFlag()        SET_REG_BIT(UR2TOSR, UR2TOSR_CTOIF_MSK)
#define UART3_ClearTimeoutFlag()        SET_REG_BIT(UR3TOSR, UR3TOSR_CTOIF_MSK)
#define UART4_ClearTimeoutFlag()        SET_REG_BIT(UR4TOSR, UR4TOSR_CTOIF_MSK)
#define SPI_ClearTimeoutFlag()          SET_REG_BIT(SPITOSR, SPITOSR_CTOIF_MSK)
#define I2C_ClearTimeoutFlag()          SET_REG_BIT(I2CTOSR, I2CTOSR_CTOIF_MSK)

#define I2SSR_TXE_MSK                   BIT1
#define I2SSR_RXNE_MSK                  BIT0
#define I2S_CheckTXEFlag()              READ_REG_BIT(I2SSR, I2SSR_TXE_MSK)
#define I2S_CheckRXNEFlag()             READ_REG_BIT(I2SSR, I2SSR_RXNE_MSK)
#define I2S_ClearTXEFlag()              CLR_REG_BIT(I2SSR, I2SSR_TXE_MSK)
#define I2S_ClearRXNEFlag()             CLR_REG_BIT(I2SSR, I2SSR_RXNE_MSK)

#define QSPISR1_TOF_MSK                 BIT4
#define QSPISR1_SMF_MSK                 BIT3
#define QSPISR1_FTF_MSK                 BIT2
#define QSPISR1_TCF_MSK                 BIT1
#define QSPISR1_TEF_MSK                 BIT0
#define QSPI_CheckTimeout()             READ_REG_BIT(QSPI_SR1, QSPISR1_TOF_MSK)
#define QSPI_CheckMatch()               READ_REG_BIT(QSPI_SR1, QSPISR1_SMF_MSK)
#define QSPI_CheckFIFO()                READ_REG_BIT(QSPI_SR1, QSPISR1_FTF_MSK)
#define QSPI_CheckTransfer()            READ_REG_BIT(QSPI_SR1, QSPISR1_TCF_MSK)
#define QSPI_CheckError()               READ_REG_BIT(QSPI_SR1, QSPISR1_TEF_MSK)

#define QSPIFCR_CTOF_MSK                BIT4
#define QSPIFCR_CSMF_MSK                BIT3
#define QSPIFCR_CTCF_MSK                BIT1
#define QSPIFCR_CTEF_MSK                BIT0
#define QSPI_ClearTimeout()             SET_REG_BIT(QSPI_FCR, QSPIFCR_CTOF_MSK)
#define QSPI_ClearMatch()               SET_REG_BIT(QSPI_FCR, QSPIFCR_CSMF_MSK)
#define QSPI_ClearTransfer()            SET_REG_BIT(QSPI_FCR, QSPIFCR_CTCF_MSK)
#define QSPI_ClearError()               SET_REG_BIT(QSPI_FCR, QSPIFCR_CTEF_MSK)

#define DMAM2MSTA_M2MIF_MSK             BIT0
#define DMAADCSTA_ADCIF_MSK             BIT0
#define DMASPISTA_SPIIF_MSK             BIT0
#define DMAQSPISTA_QSPIIF_MSK           BIT0
#define DMALCMSTA_LCMIF_MSK             BIT0
#define DMAUR1TSTA_UR1TIF_MSK           BIT0
#define DMAUR1RSTA_UR1RIF_MSK           BIT0
#define DMAUR2TSTA_UR2TIF_MSK           BIT0
#define DMAUR2RSTA_UR2RIF_MSK           BIT0
#define DMAUR3TSTA_UR3TIF_MSK           BIT0
#define DMAUR3RSTA_UR3RIF_MSK           BIT0
#define DMAUR4TSTA_UR4TIF_MSK           BIT0
#define DMAUR4RSTA_UR4RIF_MSK           BIT0
#define DMAI2CTSTA_I2CTIF_MSK           BIT0
#define DMAI2CRSTA_I2CRIF_MSK           BIT0
#define DMAI2STSTA_I2STIF_MSK           BIT0
#define DMAI2SRSTA_I2SRIF_MSK           BIT0
#define DMAPWMATSTA_PWMATIF_MSK         BIT0
#define DMAPWMARSTA_PWMARIF_MSK         BIT0
#define DMA_M2M_CheckFlag()             READ_REG_BIT(DMA_M2M_STA,   DMAM2MSTA_M2MIF_MSK)
#define DMA_ADC_CheckFlag()             READ_REG_BIT(DMA_ADC_STA,   DMAADCSTA_ADCIF_MSK)
#define DMA_SPI_CheckFlag()             READ_REG_BIT(DMA_SPI_STA,   DMASPISTA_SPIIF_MSK)
#define DMA_QSPI_CheckFlag()            READ_REG_BIT(DMA_QSPI_STA,  DMAQSPISTA_QSPIIF_MSK)
#define DMA_LCM_CheckFlag()             READ_REG_BIT(DMA_LCM_STA,   DMALCMSTA_LCMIF_MSK)
#define DMA_UART1_CheckTxFlag()         READ_REG_BIT(DMA_UR1T_STA,  DMAUR1TSTA_UR1TIF_MSK)
#define DMA_UART1_CheckRxFlag()         READ_REG_BIT(DMA_UR1R_STA,  DMAUR1RSTA_UR1RIF_MSK)
#define DMA_UART2_CheckTxFlag()         READ_REG_BIT(DMA_UR2T_STA,  DMAUR2TSTA_UR2TIF_MSK)
#define DMA_UART2_CheckRxFlag()         READ_REG_BIT(DMA_UR2R_STA,  DMAUR2RSTA_UR2RIF_MSK)
#define DMA_UART3_CheckTxFlag()         READ_REG_BIT(DMA_UR3T_STA,  DMAUR3TSTA_UR3TIF_MSK)
#define DMA_UART3_CheckRxFlag()         READ_REG_BIT(DMA_UR3R_STA,  DMAUR3RSTA_UR3RIF_MSK)
#define DMA_UART4_CheckTxFlag()         READ_REG_BIT(DMA_UR4T_STA,  DMAUR4TSTA_UR4TIF_MSK)
#define DMA_UART4_CheckRxFlag()         READ_REG_BIT(DMA_UR4R_STA,  DMAUR4RSTA_UR4RIF_MSK)
#define DMA_I2C_CheckTxFlag()           READ_REG_BIT(DMA_I2CT_STA,  DMAI2CTSTA_I2CTIF_MSK)
#define DMA_I2C_CheckRxFlag()           READ_REG_BIT(DMA_I2CR_STA,  DMAI2CRSTA_I2CRIF_MSK)
#define DMA_I2S_CheckTxFlag()           READ_REG_BIT(DMA_I2ST_STA,  DMAI2STSTA_I2STIF_MSK)
#define DMA_I2S_CheckRxFlag()           READ_REG_BIT(DMA_I2SR_STA,  DMAI2SRSTA_I2SRIF_MSK)
#define DMA_PWMA_CheckTxFlag()          READ_REG_BIT(DMA_PWMAT_STA, DMAPWMATSTA_PWMATIF_MSK)
#define DMA_PWMA_CheckRxFlag()          READ_REG_BIT(DMA_PWMAR_STA, DMAPWMARSTA_PWMARIF_MSK)
#define DMA_M2M_ClearFlag()             CLR_REG_BIT(DMA_M2M_STA,    DMAM2MSTA_M2MIF_MSK)
#define DMA_ADC_ClearFlag()             CLR_REG_BIT(DMA_ADC_STA,    DMAADCSTA_ADCIF_MSK)
#define DMA_SPI_ClearFlag()             CLR_REG_BIT(DMA_SPI_STA,    DMASPISTA_SPIIF_MSK)
#define DMA_QSPI_ClearFlag()            CLR_REG_BIT(DMA_QSPI_STA,   DMAQSPISTA_QSPIIF_MSK)
#define DMA_LCM_ClearFlag()             CLR_REG_BIT(DMA_LCM_STA,    DMALCMSTA_LCMIF_MSK)
#define DMA_UART1_ClearTxFlag()         CLR_REG_BIT(DMA_UR1T_STA,   DMAUR1TSTA_UR1TIF_MSK)
#define DMA_UART1_ClearRxFlag()         CLR_REG_BIT(DMA_UR1R_STA,   DMAUR1RSTA_UR1RIF_MSK)
#define DMA_UART2_ClearTxFlag()         CLR_REG_BIT(DMA_UR2T_STA,   DMAUR2TSTA_UR2TIF_MSK)
#define DMA_UART2_ClearRxFlag()         CLR_REG_BIT(DMA_UR2R_STA,   DMAUR2RSTA_UR2RIF_MSK)
#define DMA_UART3_ClearTxFlag()         CLR_REG_BIT(DMA_UR3T_STA,   DMAUR3TSTA_UR3TIF_MSK)
#define DMA_UART3_ClearRxFlag()         CLR_REG_BIT(DMA_UR3R_STA,   DMAUR3RSTA_UR3RIF_MSK)
#define DMA_UART4_ClearTxFlag()         CLR_REG_BIT(DMA_UR4T_STA,   DMAUR4TSTA_UR4TIF_MSK)
#define DMA_UART4_ClearRxFlag()         CLR_REG_BIT(DMA_UR4R_STA,   DMAUR4RSTA_UR4RIF_MSK)
#define DMA_I2C_ClearTxFlag()           CLR_REG_BIT(DMA_I2CT_STA,   DMAI2CTSTA_I2CTIF_MSK)
#define DMA_I2C_ClearRxFlag()           CLR_REG_BIT(DMA_I2CR_STA,   DMAI2CRSTA_I2CRIF_MSK)
#define DMA_I2S_ClearTxFlag()           CLR_REG_BIT(DMA_I2ST_STA,   DMAI2STSTA_I2STIF_MSK)
#define DMA_I2S_ClearRxFlag()           CLR_REG_BIT(DMA_I2SR_STA,   DMAI2SRSTA_I2SRIF_MSK)
#define DMA_PWMA_ClearTxFlag()          CLR_REG_BIT(DMA_PWMAT_STA,  DMAPWMATSTA_PWMATIF_MSK)
#define DMA_PWMA_ClearRxFlag()          CLR_REG_BIT(DMA_PWMAR_STA,  DMAPWMARSTA_PWMARIF_MSK)

#define IPH_PX0H_MSK                    BIT0
#define INT0_SetIntPriority(n)          PX0 = ((n) & 1); \
                                        MODIFY_REG(IPH, IPH_PX0H_MSK, ((((n) >> 1) & 1) << 0))

#define IPH_PX1H_MSK                    BIT2
#define INT1_SetIntPriority(n)          PX1 = ((n) & 1); \
                                        MODIFY_REG(IPH, IPH_PX1H_MSK, ((((n) >> 1) & 1) << 2))

#define IP2_PX4_MSK                     BIT4
#define IP2H_PX4H_MSK                   BIT4
#define INT4_SetIntPriority(n)          MODIFY_REG(IP2, IP2_PX4_MSK, (((n) & 1) << 4)); \
                                        MODIFY_REG(IP2H, IP2H_PX4H_MSK, ((((n) >> 1) & 1) << 4))

#define IPH_PT0H_MSK                    BIT1
#define TIMER0_SetIntPriority(n)        PT0 = ((n) & 1); \
                                        MODIFY_REG(IPH, IPH_PT0H_MSK, ((((n) >> 1) & 1) << 1))

#define IPH_PT1H_MSK                    BIT3
#define TIMER1_SetIntPriority(n)        PT1 = ((n) & 1); \
                                        MODIFY_REG(IPH, IPH_PT1H_MSK, ((((n) >> 1) & 1) << 3))

#define IPH_PSH_MSK                     BIT4
#define UART1_SetIntPriority(n)         PS = ((n) & 1); \
                                        MODIFY_REG(IPH, IPH_PSH_MSK, ((((n) >> 1) & 1) << 4))

#define IP2_PS2_MSK                     BIT0
#define IP2H_PS2H_MSK                   BIT0
#define UART2_SetIntPriority(n)         MODIFY_REG(IP2, IP2_PS2_MSK, (((n) & 1) << 0)); \
                                        MODIFY_REG(IP2H, IP2H_PS2H_MSK, ((((n) >> 1) & 1) << 0))

#define IP3_PS3_MSK                     BIT0
#define IP3H_PS3H_MSK                   BIT0
#define UART3_SetIntPriority(n)         MODIFY_REG(IP3, IP3_PS3_MSK, (((n) & 1) << 0)); \
                                        MODIFY_REG(IP3H, IP3H_PS3H_MSK, ((((n) >> 1) & 1) << 0))

#define IP3_PS4_MSK                     BIT1
#define IP3H_PS4H_MSK                   BIT1
#define UART4_SetIntPriority(n)         MODIFY_REG(IP3, IP3_PS4_MSK, (((n) & 1) << 1)); \
                                        MODIFY_REG(IP3H, IP3H_PS4H_MSK, ((((n) >> 1) & 1) << 1))

#define IPH_PLVDH_MSK                   BIT6
#define LVD_SetIntPriority(n)           PLVD = ((n) & 1); \
                                        MODIFY_REG(IPH, IPH_PLVDH_MSK, ((((n) >> 1) & 1) << 6))

#define IPH_PADCH_MSK                   BIT5
#define ADC_SetIntPriority(n)           PADC = ((n) & 1); \
                                        MODIFY_REG(IPH, IPH_PADCH_MSK, ((((n) >> 1) & 1) << 5))

#define IP2_PSPI_MSK                    BIT1
#define IP2H_PSPIH_MSK                  BIT1
#define SPI_SetIntPriority(n)           MODIFY_REG(IP2, IP2_PSPI_MSK, (((n) & 1) << 1)); \
                                        MODIFY_REG(IP2H, IP2H_PSPIH_MSK, ((((n) >> 1) & 1) << 1))

#define IP2_PCMP_MSK                    BIT5
#define IP2H_PCMPH_MSK                  BIT5
#define CMP_SetIntPriority(n)           MODIFY_REG(IP2, IP2_PCMP_MSK, (((n) & 1) << 5)); \
                                        MODIFY_REG(IP2H, IP2H_PCMPH_MSK, ((((n) >> 1) & 1) << 5))

#define IP3_PRTC_MSK                    BIT2
#define IP3H_PRTCH_MSK                  BIT2
#define RTC_SetIntPriority(n)           MODIFY_REG(IP3, IP3_PRTC_MSK, (((n) & 1) << 2)); \
                                        MODIFY_REG(IP3H, IP3H_PRTCH_MSK, ((((n) >> 1) & 1) << 2))

#define IP2_PI2C_MSK                    BIT6
#define IP2H_PI2CH_MSK                  BIT6
#define I2C_SetIntPriority(n)           MODIFY_REG(IP2, IP2_PI2C_MSK, (((n) & 1) << 6)); \
                                        MODIFY_REG(IP2H, IP2H_PI2CH_MSK, ((((n) >> 1) & 1) << 6))

#define IP3_PI2S_MSK                    BIT3
#define IP3H_PI2SH_MSK                  BIT3
#define I2S_SetIntPriority(n)           MODIFY_REG(IP3, IP3_PI2S_MSK, (((n) & 1) << 3)); \
                                        MODIFY_REG(IP3H, IP3H_PI2SH_MSK, ((((n) >> 1) & 1) << 3))

#define IPH_PPCAH_MSK                   BIT7
#define PCA_SetIntPriority(n)           PPCA = ((n) & 1); \
                                        MODIFY_REG(IPH, IPH_PPCAH_MSK, ((((n) >> 1) & 1) << 7))

#define IP2_PUSB_MSK                    BIT7
#define IP2H_PUSBH_MSK                  BIT7
#define USB_SetIntPriority(n)           MODIFY_REG(IP2, IP2_PUSB_MSK, (((n) & 1) << 7)); \
                                        MODIFY_REG(IP2H, IP2H_PUSBH_MSK, ((((n) >> 1) & 1) << 7))

#define IP2_PPWMA_MSK                   BIT2
#define IP2H_PPWMAH_MSK                 BIT2
#define PWMA_SetIntPriority(n)          MODIFY_REG(IP2, IP2_PPWMA_MSK, (((n) & 1) << 2)); \
                                        MODIFY_REG(IP2H, IP2H_PPWMAH_MSK, ((((n) >> 1) & 1) << 2))
#define HSPWMA_SetIntPriority(n)        PWMA_SetIntPriority(n)

#define IP2_PPWMB_MSK                   BIT3
#define IP2H_PPWMBH_MSK                 BIT3
#define PWMB_SetIntPriority(n)          MODIFY_REG(IP2, IP2_PPWMB_MSK, (((n) & 1) << 3)); \
                                        MODIFY_REG(IP2H, IP2H_PPWMBH_MSK, ((((n) >> 1) & 1) << 3))
#define HSPWMB_SetIntPriority(n)        PWMB_SetIntPriority(n)

#define LCMIFIP_MSK                     (BIT5 | BIT4)
#define LCM_SetIntPriority(n)           MODIFY_REG(LCMIFCFG, LCMIFIP_MSK, ((n) << 4))

#define PORT_SetIntPriority(p, n)       MODIFY_REG(PINIPL, BIT(p), (((n) & 0x01) << (p))); \
                                        MODIFY_REG(PINIPH, BIT(p), ((((n) >> 1) & 0x01) << (p)))

#define SetP0IntPriority(n)             PORT_SetIntPriority(0, (n))
#define SetP1IntPriority(n)             PORT_SetIntPriority(1, (n))
#define SetP2IntPriority(n)             PORT_SetIntPriority(2, (n))
#define SetP3IntPriority(n)             PORT_SetIntPriority(3, (n))
#define SetP4IntPriority(n)             PORT_SetIntPriority(4, (n))
#define SetP5IntPriority(n)             PORT_SetIntPriority(5, (n))
#define SetP6IntPriority(n)             PORT_SetIntPriority(6, (n))
#define SetP7IntPriority(n)             PORT_SetIntPriority(7, (n))

#define QSPIIP_MSK                      (BIT3 | BIT2)
#define QSPI_SetIntPriority(n)          MODIFY_REG(QSPI_DCR1, QSPIIP_MSK, ((n) << 2))

#define DMAM2MCFG_M2MIP_MSK             (BIT3 | BIT2)
#define DMAADCCFG_ADCIP_MSK             (BIT3 | BIT2)
#define DMASPICFG_SPIIP_MSK             (BIT3 | BIT2)
#define DMAQSPICFG_QSPIIP_MSK           (BIT3 | BIT2)
#define DMALCMCFG_LCMIP_MSK             (BIT3 | BIT2)
#define DMAUR1TCFG_UR1TIP_MSK           (BIT3 | BIT2)
#define DMAUR1RCFG_UR1RIP_MSK           (BIT3 | BIT2)
#define DMAUR2TCFG_UR2TIP_MSK           (BIT3 | BIT2)
#define DMAUR2RCFG_UR2RIP_MSK           (BIT3 | BIT2)
#define DMAUR3TCFG_UR3TIP_MSK           (BIT3 | BIT2)
#define DMAUR3RCFG_UR3RIP_MSK           (BIT3 | BIT2)
#define DMAUR4TCFG_UR4TIP_MSK           (BIT3 | BIT2)
#define DMAUR4RCFG_UR4RIP_MSK           (BIT3 | BIT2)
#define DMAI2CTCFG_I2CTIP_MSK           (BIT3 | BIT2)
#define DMAI2CRCFG_I2CRIP_MSK           (BIT3 | BIT2)
#define DMAI2STCFG_I2STIP_MSK           (BIT3 | BIT2)
#define DMAI2SRCFG_I2SRIP_MSK           (BIT3 | BIT2)
#define DMAPWMATCFG_PWMATIP_MSK         (BIT3 | BIT2)
#define DMAPWMARCFG_PWMARIP_MSK         (BIT3 | BIT2)
#define DMA_M2M_SetIntPriority(n)       MODIFY_REG(DMA_M2M_CFG,   DMAM2MCFG_M2MIP_MSK, ((n) << 2))
#define DMA_ADC_SetIntPriority(n)       MODIFY_REG(DMA_ADC_CFG,   DMAADCCFG_ADCIP_MSK, ((n) << 2))
#define DMA_SPI_SetIntPriority(n)       MODIFY_REG(DMA_SPI_CFG,   DMASPICFG_SPIIP_MSK, ((n) << 2))
#define DMA_QSPI_SetIntPriority(n)      MODIFY_REG(DMA_QSPI_CFG,  DMAQSPICFG_QSPIIP_MSK, ((n) << 2))
#define DMA_LCM_SetIntPriority(n)       MODIFY_REG(DMA_LCM_CFG,   DMALCMCFG_LCMIP_MSK, ((n) << 2))
#define DMA_UART1_SetTxIntPriority(n)   MODIFY_REG(DMA_UR1T_CFG,  DMAUR1TCFG_UR1TIP_MSK, ((n) << 2))
#define DMA_UART1_SetRxIntPriority(n)   MODIFY_REG(DMA_UR1R_CFG,  DMAUR1RCFG_UR1RIP_MSK, ((n) << 2))
#define DMA_UART2_SetTxIntPriority(n)   MODIFY_REG(DMA_UR2T_CFG,  DMAUR2TCFG_UR2TIP_MSK, ((n) << 2))
#define DMA_UART2_SetRxIntPriority(n)   MODIFY_REG(DMA_UR2R_CFG,  DMAUR2RCFG_UR2RIP_MSK, ((n) << 2))
#define DMA_UART3_SetTxIntPriority(n)   MODIFY_REG(DMA_UR3T_CFG,  DMAUR3TCFG_UR3TIP_MSK, ((n) << 2))
#define DMA_UART3_SetRxIntPriority(n)   MODIFY_REG(DMA_UR3R_CFG,  DMAUR3RCFG_UR3RIP_MSK, ((n) << 2))
#define DMA_UART4_SetTxIntPriority(n)   MODIFY_REG(DMA_UR4T_CFG,  DMAUR4TCFG_UR4TIP_MSK, ((n) << 2))
#define DMA_UART4_SetRxIntPriority(n)   MODIFY_REG(DMA_UR4R_CFG,  DMAUR4RCFG_UR4RIP_MSK, ((n) << 2))
#define DMA_I2C_SetTxIntPriority(n)     MODIFY_REG(DMA_I2CT_CFG,  DMAI2CTCFG_I2CTIP_MSK, ((n) << 2))
#define DMA_I2C_SetRxIntPriority(n)     MODIFY_REG(DMA_I2CR_CFG,  DMAI2CRCFG_I2CRIP_MSK, ((n) << 2))
#define DMA_I2S_SetTxIntPriority(n)     MODIFY_REG(DMA_I2ST_CFG,  DMAI2STCFG_I2STIP_MSK, ((n) << 2))
#define DMA_I2S_SetRxIntPriority(n)     MODIFY_REG(DMA_I2SR_CFG,  DMAI2SRCFG_I2SRIP_MSK, ((n) << 2))
#define DMA_PWMA_SetTxIntPriority(n)    MODIFY_REG(DMA_PWMAT_CFG, DMAPWMATCFG_PWMATIP_MSK, ((n) << 2))
#define DMA_PWMA_SetRxIntPriority(n)    MODIFY_REG(DMA_PWMAR_CFG, DMAPWMARCFG_PWMARIP_MSK, ((n) << 2))

/////////////////////////////////////////////////

#define EAXFR_MSK                       BIT7
#define EnableAccessXFR()               SET_REG_BIT(P_SW2, EAXFR_MSK)

#define AccessCodeSpeed(n)              WRITE_REG(WTST, (n)) 
#define AccessCodeFastest()             AccessCodeSpeed(0)

#define ACCIXRAM_MSK                    (BIT2 | BIT1 | BIT0)
#define AccessIXramSpeed(n)             MODIFY_REG(CKCON, ACCIXRAM_MSK, ((n) << 0))
#define AccessIXramFastest()            AccessIXramSpeed(0)

#define ACCEXRAM_MSK                    (BIT2 | BIT1 | BIT0)
#define AccessEXramSpeed(n)             MODIFY_REG(BUS_SPEED, ACCEXRAM_MSK, ((n) << 0))
#define AccessEXramFastest()            AccessEXramSpeed(0)

#define UART1_PS_MSK                    (BIT7 | BIT6)
#define UART1_SwitchP3031()             MODIFY_REG(P_SW1, UART1_PS_MSK, ((0) << 6))
#define UART1_SwitchP3637()             MODIFY_REG(P_SW1, UART1_PS_MSK, ((1) << 6))
#define UART1_SwitchP1617()             MODIFY_REG(P_SW1, UART1_PS_MSK, ((2) << 6))
#define UART1_SwitchP4344()             MODIFY_REG(P_SW1, UART1_PS_MSK, ((3) << 6))

#define UART2_PS_MSK                    (BIT0)
#define UART2_SwitchP1213()             CLR_REG_BIT(P_SW2, UART2_PS_MSK)
#define UART2_SwitchP4243()             SET_REG_BIT(P_SW2, UART2_PS_MSK)

#define UART3_PS_MSK                    (BIT1)
#define UART3_SwitchP0001()             CLR_REG_BIT(P_SW2, UART3_PS_MSK)
#define UART3_SwitchP5051()             SET_REG_BIT(P_SW2, UART3_PS_MSK)

#define UART4_PS_MSK                    (BIT2)
#define UART4_SwitchP0203()             CLR_REG_BIT(P_SW2, UART4_PS_MSK)
#define UART4_SwitchP5253()             SET_REG_BIT(P_SW2, UART4_PS_MSK)

#define I2C_PS_MSK                      (BIT5 | BIT4)
#define I2C_SwitchP2324()               MODIFY_REG(P_SW2, I2C_PS_MSK, ((0) << 4))
#define I2C_SwitchP1415()               MODIFY_REG(P_SW2, I2C_PS_MSK, ((1) << 4))
#define I2C_SwitchP3332()               MODIFY_REG(P_SW2, I2C_PS_MSK, ((3) << 4))

#define SPI_PS_MSK                      (BIT3 | BIT2)
#define SPI_SwitchP1n()                 MODIFY_REG(P_SW1, SPI_PS_MSK, ((0) << 2))
#define SPI_SwitchP2n()                 MODIFY_REG(P_SW1, SPI_PS_MSK, ((1) << 2))
#define SPI_SwitchP4n()                 MODIFY_REG(P_SW1, SPI_PS_MSK, ((2) << 2))
#define SPI_SwitchP3n()                 MODIFY_REG(P_SW1, SPI_PS_MSK, ((3) << 2))

#define USART1_PS_MSK                   (BIT3 | BIT2)
#define USART1_SwitchP1n()              MODIFY_REG(P_SW3, USART1_PS_MSK, ((0) << 2))
#define USART1_SwitchP2n()              MODIFY_REG(P_SW3, USART1_PS_MSK, ((1) << 2))
#define USART1_SwitchP4n()              MODIFY_REG(P_SW3, USART1_PS_MSK, ((2) << 2))
#define USART1_SwitchP3n()              MODIFY_REG(P_SW3, USART1_PS_MSK, ((3) << 2))

#define USART2_PS_MSK                   (BIT5 | BIT4)
#define USART2_SwitchP1n()              MODIFY_REG(P_SW3, USART2_PS_MSK, ((0) << 4))
#define USART2_SwitchP2n()              MODIFY_REG(P_SW3, USART2_PS_MSK, ((1) << 4))
#define USART2_SwitchP4n()              MODIFY_REG(P_SW3, USART2_PS_MSK, ((2) << 4))
#define USART2_SwitchP3n()              MODIFY_REG(P_SW3, USART2_PS_MSK, ((3) << 4))

#define I2S_PS_MSK                      (BIT7 | BIT6)
#define I2S_SwitchP3n()                 MODIFY_REG(P_SW3, I2S_PS_MSK, ((0) << 6))
#define I2S_SwitchP1n()                 MODIFY_REG(P_SW3, I2S_PS_MSK, ((1) << 6))
#define I2S_SwitchP2n()                 MODIFY_REG(P_SW3, I2S_PS_MSK, ((2) << 6))
#define I2S_SwitchP4n()                 MODIFY_REG(P_SW3, I2S_PS_MSK, ((3) << 6))

#define QSPI_PS_MSK                     (BIT1 | BIT0)
#define QSPI_SwitchP1n()                MODIFY_REG(P_SW4, QSPI_PS_MSK, ((0) << 0))
#define QSPI_SwitchP4nP5n()             MODIFY_REG(P_SW4, QSPI_PS_MSK, ((1) << 0))
#define QSPI_SwitchP2nP4n()             MODIFY_REG(P_SW4, QSPI_PS_MSK, ((2) << 0))

#define CMPO_PS_MSK                     (BIT3)
#define CMPO_SwitchP45()                CLR_REG_BIT(P_SW2, CMPO_PS_MSK)
#define CMPO_SwitchP41()                SET_REG_BIT(P_SW2, CMPO_PS_MSK)

#define CMPNS_MSK                       BIT2
#define CMPN_SwitchP44()                CLR_REG_BIT(CMPEXCFG, CMPNS_MSK)
#define CMPN_SwitchREFV()               SET_REG_BIT(CMPEXCFG, CMPNS_MSK)

#define CMPPS_MSK                       (BIT1 | BIT0)
#define CMPP_SwitchP46()                 MODIFY_REG(CMPEXCFG, CMPPS_MSK, ((0) << 0))
#define CMPP_SwitchP50()                 MODIFY_REG(CMPEXCFG, CMPPS_MSK, ((1) << 0))
#define CMPP_SwitchP51()                 MODIFY_REG(CMPEXCFG, CMPPS_MSK, ((2) << 0))
#define CMPP_SwitchADCIN()               MODIFY_REG(CMPEXCFG, CMPPS_MSK, ((3) << 0))

#define MCLKO_PS_MSK                    (BIT7)
#define CLK_SYSCLKO_SwitchP47()         CLR_REG_BIT(MCLKOCR, MCLKO_PS_MSK)
#define CLK_SYSCLKO_SwitchP56()         SET_REG_BIT(MCLKOCR, MCLKO_PS_MSK)

#define PCA_PS_MSK                      (BIT6 | BIT5)
#define PCA_SwitchP1n()                 MODIFY_REG(CMOD, PCA_PS_MSK, ((0) << 5))
#define PCA_SwitchP4n()                 MODIFY_REG(CMOD, PCA_PS_MSK, ((1) << 5))
#define PCA_SwitchP2n()                 MODIFY_REG(CMOD, PCA_PS_MSK, ((2) << 5))

#define PWMA_C1PS_MSK                   (BIT1 | BIT0)
#define PWMA_C1SwitchP1011()            MODIFY_REG(PWMA_PS, PWMA_C1PS_MSK, ((0) << 0))
#define PWMA_C1SwitchP0001()            MODIFY_REG(PWMA_PS, PWMA_C1PS_MSK, ((1) << 0))
#define PWMA_C1SwitchP2021()            MODIFY_REG(PWMA_PS, PWMA_C1PS_MSK, ((2) << 0))

#define HSPWMA_C1SwitchP1011()          PWMA_C1SwitchP1011()
#define HSPWMA_C1SwitchP0001()          PWMA_C1SwitchP0001()
#define HSPWMA_C1SwitchP2021()          PWMA_C1SwitchP2021()

#define PWMA_C2PS_MSK                   (BIT3 | BIT2)
#define PWMA_C2SwitchP1213()            MODIFY_REG(PWMA_PS, PWMA_C2PS_MSK, ((0) << 2))
#define PWMA_C2SwitchP0203()            MODIFY_REG(PWMA_PS, PWMA_C2PS_MSK, ((1) << 2))
#define PWMA_C2SwitchP2223()            MODIFY_REG(PWMA_PS, PWMA_C2PS_MSK, ((2) << 2))

#define HSPWMA_C2SwitchP1213()          PWMA_C2SwitchP1213()
#define HSPWMA_C2SwitchP0203()          PWMA_C2SwitchP0203()
#define HSPWMA_C2SwitchP2223()          PWMA_C2SwitchP2223()

#define PWMA_C3PS_MSK                   (BIT5 | BIT4)
#define PWMA_C3SwitchP1415()            MODIFY_REG(PWMA_PS, PWMA_C3PS_MSK, ((0) << 4))
#define PWMA_C3SwitchP0405()            MODIFY_REG(PWMA_PS, PWMA_C3PS_MSK, ((1) << 4))
#define PWMA_C3SwitchP2425()            MODIFY_REG(PWMA_PS, PWMA_C3PS_MSK, ((2) << 4))

#define HSPWMA_C3SwitchP1415()          PWMA_C3SwitchP1415()
#define HSPWMA_C3SwitchP0405()          PWMA_C3SwitchP0405()
#define HSPWMA_C3SwitchP2425()          PWMA_C3SwitchP2425()

#define PWMA_C4PS_MSK                   (BIT7 | BIT6)
#define PWMA_C4SwitchP1617()            MODIFY_REG(PWMA_PS, PWMA_C4PS_MSK, ((0) << 6))
#define PWMA_C4SwitchP0607()            MODIFY_REG(PWMA_PS, PWMA_C4PS_MSK, ((1) << 6))
#define PWMA_C4SwitchP2627()            MODIFY_REG(PWMA_PS, PWMA_C4PS_MSK, ((2) << 6))

#define HSPWMA_C4SwitchP1617()          PWMA_C4SwitchP1617()
#define HSPWMA_C4SwitchP0607()          PWMA_C4SwitchP0607()
#define HSPWMA_C4SwitchP2627()          PWMA_C4SwitchP2627()

#define PWMB_C5PS_MSK                   (BIT1 | BIT0)
#define PWMB_C5SwitchP01()              MODIFY_REG(PWMB_PS, PWMB_C5PS_MSK, ((0) << 0))
#define PWMB_C5SwitchP11()              MODIFY_REG(PWMB_PS, PWMB_C5PS_MSK, ((1) << 0))
#define PWMB_C5SwitchP21()              MODIFY_REG(PWMB_PS, PWMB_C5PS_MSK, ((2) << 0))
#define PWMB_C5SwitchP50()              MODIFY_REG(PWMB_PS, PWMB_C5PS_MSK, ((3) << 0))

#define HSPWMB_C5SwitchP01()            PWMB_C5SwitchP01()
#define HSPWMB_C5SwitchP11()            PWMB_C5SwitchP11()
#define HSPWMB_C5SwitchP21()            PWMB_C5SwitchP21()
#define HSPWMB_C5SwitchP50()            PWMB_C5SwitchP50()

#define PWMB_C6PS_MSK                   (BIT3 | BIT2)
#define PWMB_C6SwitchP03()              MODIFY_REG(PWMB_PS, PWMB_C6PS_MSK, ((0) << 2))
#define PWMB_C6SwitchP13()              MODIFY_REG(PWMB_PS, PWMB_C6PS_MSK, ((1) << 2))
#define PWMB_C6SwitchP23()              MODIFY_REG(PWMB_PS, PWMB_C6PS_MSK, ((2) << 2))
#define PWMB_C6SwitchP51()              MODIFY_REG(PWMB_PS, PWMB_C6PS_MSK, ((3) << 2))

#define HSPWMB_C6SwitchP03()            PWMB_C6SwitchP03()
#define HSPWMB_C6SwitchP13()            PWMB_C6SwitchP13()
#define HSPWMB_C6SwitchP23()            PWMB_C6SwitchP23()
#define HSPWMB_C6SwitchP51()            PWMB_C6SwitchP51()

#define PWMB_C7PS_MSK                   (BIT5 | BIT4)
#define PWMB_C7SwitchP05()              MODIFY_REG(PWMB_PS, PWMB_C7PS_MSK, ((0) << 4))
#define PWMB_C7SwitchP15()              MODIFY_REG(PWMB_PS, PWMB_C7PS_MSK, ((1) << 4))
#define PWMB_C7SwitchP25()              MODIFY_REG(PWMB_PS, PWMB_C7PS_MSK, ((2) << 4))
#define PWMB_C7SwitchP52()              MODIFY_REG(PWMB_PS, PWMB_C7PS_MSK, ((3) << 4))

#define HSPWMB_C7SwitchP05()            PWMB_C7SwitchP05()
#define HSPWMB_C7SwitchP15()            PWMB_C7SwitchP15()
#define HSPWMB_C7SwitchP25()            PWMB_C7SwitchP25()
#define HSPWMB_C7SwitchP52()            PWMB_C7SwitchP52()

#define PWMB_C8PS_MSK                   (BIT7 | BIT6)
#define PWMB_C8SwitchP07()              MODIFY_REG(PWMB_PS, PWMB_C8PS_MSK, ((0) << 6))
#define PWMB_C8SwitchP17()              MODIFY_REG(PWMB_PS, PWMB_C8PS_MSK, ((1) << 6))
#define PWMB_C8SwitchP27()              MODIFY_REG(PWMB_PS, PWMB_C8PS_MSK, ((2) << 6))
#define PWMB_C8SwitchP53()              MODIFY_REG(PWMB_PS, PWMB_C8PS_MSK, ((3) << 6))

#define HSPWMB_C8SwitchP07()            PWMB_C8SwitchP07()
#define HSPWMB_C8SwitchP17()            PWMB_C8SwitchP17()
#define HSPWMB_C8SwitchP27()            PWMB_C8SwitchP27()
#define HSPWMB_C8SwitchP53()            PWMB_C8SwitchP53()

#define PWMA_ETRPS_MSK                  (BIT1 | BIT0)
#define PWMA_ETRSwitchP32()             MODIFY_REG(PWMA_ETRPS, PWMA_ETRPS_MSK, ((0) << 0))
#define PWMA_ETRSwitchP41()             MODIFY_REG(PWMA_ETRPS, PWMA_ETRPS_MSK, ((1) << 0))
#define PWMA_ETRSwitchP23()             MODIFY_REG(PWMA_ETRPS, PWMA_ETRPS_MSK, ((2) << 0))
#define PWMA_ETRSwitchP12()             MODIFY_REG(PWMA_ETRPS, PWMA_ETRPS_MSK, ((3) << 0))

#define HSPWMA_ETRSwitchP32()           PWMA_ETRSwitchP32()
#define HSPWMA_ETRSwitchP41()           PWMA_ETRSwitchP41()
#define HSPWMA_ETRSwitchP23()           PWMA_ETRSwitchP23()
#define HSPWMA_ETRSwitchP12()           PWMA_ETRSwitchP12()

#define PWMB_ETRPS_MSK                  (BIT1 | BIT0)
#define PWMB_ETRSwitchP32()             MODIFY_REG(PWMB_ETRPS, PWMB_ETRPS_MSK, ((0) << 0))
#define PWMB_ETRSwitchP41()             MODIFY_REG(PWMB_ETRPS, PWMB_ETRPS_MSK, ((1) << 0))
#define PWMB_ETRSwitchP23()             MODIFY_REG(PWMB_ETRPS, PWMB_ETRPS_MSK, ((2) << 0))
#define PWMB_ETRSwitchP12()             MODIFY_REG(PWMB_ETRPS, PWMB_ETRPS_MSK, ((3) << 0))

#define HSPWMB_ETRSwitchP32()           PWMB_ETRSwitchP32()
#define HSPWMB_ETRSwitchP41()           PWMB_ETRSwitchP41()
#define HSPWMB_ETRSwitchP23()           PWMB_ETRSwitchP23()
#define HSPWMB_ETRSwitchP12()           PWMB_ETRSwitchP12()

#define PWMA_BRKPS_MSK                  (BIT3 | BIT2)
#define PWMA_BRKSwitchP35()             MODIFY_REG(PWMA_ETRPS, PWMA_BRKPS_MSK, ((0) << 2))
#define PWMA_BRKSwitchCMPO()            MODIFY_REG(PWMA_ETRPS, PWMA_BRKPS_MSK, ((1) << 2))
#define PWMA_BRKSwitchP06()             MODIFY_REG(PWMA_ETRPS, PWMA_BRKPS_MSK, ((2) << 2))

#define HSPWMA_BRKSwitchP35()           PWMA_BRKSwitchP35() 
#define HSPWMA_BRKSwitchCMPO()          PWMA_BRKSwitchCMPO()
#define HSPWMA_BRKSwitchP06()           PWMA_BRKSwitchP06() 

#define PWMB_BRKPS_MSK                  (BIT3 | BIT2)
#define PWMB_BRKSwitchP35()             MODIFY_REG(PWMB_ETRPS, PWMB_BRKPS_MSK, ((0) << 2))
#define PWMB_BRKSwitchCMPO()            MODIFY_REG(PWMB_ETRPS, PWMB_BRKPS_MSK, ((1) << 2))
#define PWMB_BRKSwitchP06()             MODIFY_REG(PWMB_ETRPS, PWMB_BRKPS_MSK, ((2) << 2))

#define HSPWMB_BRKSwitchP35()           PWMB_BRKSwitchP35() 
#define HSPWMB_BRKSwitchCMPO()          PWMB_BRKSwitchCMPO()
#define HSPWMB_BRKSwitchP06()           PWMB_BRKSwitchP06() 

#define ADC_ETRPS_MSK                   (BIT7 | BIT6)
#define ADC_ETRSwitchP47()              MODIFY_REG(ADCEXCFG, ADC_ETRPS_MSK, ((0) << 6))
#define ADC_ETRSwitchP40()              MODIFY_REG(ADCEXCFG, ADC_ETRPS_MSK, ((1) << 6))
#define ADC_ETRSwitchP20()              MODIFY_REG(ADCEXCFG, ADC_ETRPS_MSK, ((2) << 6))

#define LCM_DPS_MSK                     (BIT3 | BIT2)
#define LCM_DATA8BSwitchP2()            MODIFY_REG(LCMIFCFG, LCM_DPS_MSK, ((0) << 2))
#define LCM_DATA16BSwitchP2P0()         MODIFY_REG(LCMIFCFG, LCM_DPS_MSK, ((0) << 2))
#define LCM_DATA16BSwitchP2P4()         MODIFY_REG(LCMIFCFG, LCM_DPS_MSK, ((2) << 2))

#define LCM_RSPS_MSK                    (BIT6)
#define LCM_RSSwitchP45()               CLR_REG_BIT(LCMIFCFG2, LCM_RSPS_MSK)
#define LCM_RSSwitchP40()               SET_REG_BIT(LCMIFCFG2, LCM_RSPS_MSK)

#define LCM_RWPS_MSK                    (BIT5)
#define LCM_CTRLSwitchP4442()           CLR_REG_BIT(LCMIFCFG2, LCM_RWPS_MSK)
#define LCM_CTRLSwitchP3736()           SET_REG_BIT(LCMIFCFG2, LCM_RWPS_MSK)

/////////////////////////////////////////////////

#define CLKSEL_MCKSEL_MSK               (BIT1 | BIT0)
#define CLK_MCLK_HIRC()                 MODIFY_REG(CLKSEL, CLKSEL_MCKSEL_MSK, ((0x00) << 0))
#define CLK_MCLK_XTAL()                 MODIFY_REG(CLKSEL, CLKSEL_MCKSEL_MSK, ((0x01) << 0))
#define CLK_MCLK_X32K()                 MODIFY_REG(CLKSEL, CLKSEL_MCKSEL_MSK, ((0x02) << 0))
#define CLK_MCLK_LIRC()                 MODIFY_REG(CLKSEL, CLKSEL_MCKSEL_MSK, ((0x03) << 0))

#define CLKSEL_MCK2SEL_MSK              (BIT3 | BIT2)
#define CLK_MCLK2_BYPASS()              MODIFY_REG(CLKSEL, CLKSEL_MCK2SEL_MSK, ((0x00) << 2))
#define CLK_MCLK2_PLL()                 MODIFY_REG(CLKSEL, CLKSEL_MCK2SEL_MSK, ((0x01) << 2))
#define CLK_MCLK2_PLLD2()               MODIFY_REG(CLKSEL, CLKSEL_MCK2SEL_MSK, ((0x02) << 2))
#define CLK_MCLK2_IRC48M()              MODIFY_REG(CLKSEL, CLKSEL_MCK2SEL_MSK, ((0x03) << 2))

#define USBCLK_ENCKM_MSK                (BIT7)
#define CLK_PLL_Enable()                SET_REG_BIT(USBCLK, USBCLK_ENCKM_MSK)
#define CLK_PLL_Disable()               CLR_REG_BIT(USBCLK, USBCLK_ENCKM_MSK)

#define USBCLK_PCKI_MSK                 (BIT6 | BIT5)
#define CLK_PLL_PreDivider1()           MODIFY_REG(USBCLK, USBCLK_PCKI_MSK, ((0) << 5))
#define CLK_PLL_PreDivider2()           MODIFY_REG(USBCLK, USBCLK_PCKI_MSK, ((1) << 5))
#define CLK_PLL_PreDivider4()           MODIFY_REG(USBCLK, USBCLK_PCKI_MSK, ((2) << 5))
#define CLK_PLL_PreDivider8()           MODIFY_REG(USBCLK, USBCLK_PCKI_MSK, ((3) << 5))

#define CKSEL_CKMS_MSK                  (BIT7)
#define CLK_PLL_Output96MHz()           CLR_REG_BIT(CLKSEL, CKSEL_CKMS_MSK)
#define CLK_PLL_Output144MHz()          SET_REG_BIT(CLKSEL, CKSEL_CKMS_MSK)

#define CKSEL_HSIOCK_MSK                (BIT6)
#define CLK_HSIOCK_MCLK()               CLR_REG_BIT(CLKSEL, CKSEL_HSIOCK_MSK)
#define CLK_HSIOCK_PLL()                SET_REG_BIT(CLKSEL, CKSEL_HSIOCK_MSK)

#define CLK_HSIOCK_Divider(n)           WRITE_REG(HSCLKDIV, (n))

#define CLK_SYSCLK_Divider(n)           WRITE_REG(CLKDIV, (n))

#define MCLKODIV_MSK                    (~(BIT7))
#define CLK_SYSCLKO_Divider(n)          MODIFY_REG(MCLKOCR, MCLKODIV_MSK, (n))

#define HIRCCR_EN_MSK                   (BIT7)
#define HIRCCR_ST_MSK                   (BIT0)
#define CLK_HIRC_Enable()               SET_REG_BIT(HIRCCR, HIRCCR_EN_MSK)
#define CLK_HIRC_WaitStable()           while (READ_REG_BIT(HIRCCR, HIRCCR_ST_MSK) == 0)

#define IRCBAND_SEL_MSK                 (BIT1 | BIT0)
#define CLK_HIRC_6MHzBand()             MODIFY_REG(IRCBAND, IRCBAND_SEL_MSK, ((0) << 0))
#define CLK_HIRC_10MHzBand()            MODIFY_REG(IRCBAND, IRCBAND_SEL_MSK, ((1) << 0))
#define CLK_HIRC_27MHzBand()            MODIFY_REG(IRCBAND, IRCBAND_SEL_MSK, ((2) << 0))
#define CLK_HIRC_44MHzBand()            MODIFY_REG(IRCBAND, IRCBAND_SEL_MSK, ((3) << 0))

#define IRC48MCR_EN_MSK                 (BIT7)
#define IRC48MCR_ST_MSK                 (BIT0)
#define CLK_IRC48M_Enable()             SET_REG_BIT(IRC48MCR, IRC48MCR_EN_MSK)
#define CLK_IRC48M_WaitStable()         while (READ_REG_BIT(IRC48MCR, IRC48MCR_ST_MSK) == 0)

#define LIRCCR_EN_MSK                   (BIT7)
#define LIRCCR_ST_MSK                   (BIT0)
#define CLK_LIRC_Enable()               SET_REG_BIT(IRC32KCR, LIRCCR_EN_MSK)
#define CLK_LIRC_WaitStable()           while (READ_REG_BIT(IRC32KCR, LIRCCR_ST_MSK) == 0)

#define XOSCCR_EN_MSK                   (BIT7)
#define XOSCCR_XITYPE_MSK               (BIT6)
#define XOSCCR_GAIN_MSK                 (BIT5)
#define XOSCCR_FILTER_MSK               (BIT3 | BIT2)
#define XOSCCR_ST_MSK                   (BIT0)

#define CLK_XTAL_Enable()               SET_REG_BIT(XOSCCR, XOSCCR_EN_MSK | XOSCCR_XITYPE_MSK)
#define CLK_XOSC_Enable()               SET_REG_BIT(XOSCCR, XOSCCR_EN_MSK)
#define CLK_XOSC_WaitStable()           while (READ_REG_BIT(XOSCCR, XOSCCR_ST_MSK) == 0)

#define CLK_XOSC_LowGain()              CLR_REG_BIT(XOSCCR, XOSCCR_GAIN_MSK)
#define CLK_XOSC_FullGain()             SET_REG_BIT(XOSCCR, XOSCCR_GAIN_MSK)

#define CLK_XOSC_HighFilter()           MODIFY_REG(CLKSEL, XOSCCR_FILTER_MSK, ((0) << 2))
#define CLK_XOSC_MiddleFilter()         MODIFY_REG(CLKSEL, XOSCCR_FILTER_MSK, ((1) << 2))
#define CLK_XOSC_LowFilter()            MODIFY_REG(CLKSEL, XOSCCR_FILTER_MSK, ((2) << 2))

#define X32KCR_EN_MSK                   (BIT7)
#define X32KCR_GAIN_MSK                 (BIT6)
#define X32KCR_ST_MSK                   (BIT0)

#define CLK_X32K_Enable()               SET_REG_BIT(X32KCR, X32KCR_EN_MSK)
#define CLK_X32K_WaitStable()           while (READ_REG_BIT(X32KCR, X32KCR_ST_MSK) == 0)

#define CLK_X32K_LowGain()              CLR_REG_BIT(X32KCR, X32KCR_GAIN_MSK)
#define CLK_X32K_FullGain()             SET_REG_BIT(X32KCR, X32KCR_GAIN_MSK)

#define CLK_SPICLK_Divider(n)           WRITE_REG(SPI_CLKDIV, (n))
#define CLK_I2SCLK_Divider(n)           WRITE_REG(I2S_CLKDIV, (n))
#define CLK_PWMACLK_Divider(n)          WRITE_REG(PWMA_CLKDIV, (n))
#define CLK_PWMBCLK_Divider(n)          WRITE_REG(PWMB_CLKDIV, (n))
#define CLK_TFPUCLK_Divider(n)          WRITE_REG(TFPU_CLKDIV, (n))

#define IRCBAND_USBCKS_MSK              (BIT7 | BIT6)
#define USB_CLK_PLLD2()                 MODIFY_REG(IRCBAND, IRCBAND_USBCKS_MSK, ((0) << 6))
#define USB_CLK_IRC48M()                MODIFY_REG(IRCBAND, IRCBAND_USBCKS_MSK, ((2) << 6))
#define USB_CLK_SYSCLK()                MODIFY_REG(IRCBAND, IRCBAND_USBCKS_MSK, ((1) << 6))

/////////////////////////////////////////////////

#define CRECR_ENCRE_MSK                 (BIT7)
#define CRECR_MONO_MSK                  (BIT6)
#define CRECR_UPT_MSK                   (BIT5 | BIT4)
#define CRECR_CREHF_MSK                 (BIT3)
#define CRECR_CREINC_MSK                (BIT2)
#define CRECR_CREDEC_MSK                (BIT1)
#define CRECR_CRERDY_MSK                (BIT0)

#define CRE_Enable()                    SET_REG_BIT(CRECR, CRECR_ENCRE_MSK)
#define CRE_Disable()                   CLR_REG_BIT(CRECR, CRECR_ENCRE_MSK)

#define CRE_OneStepMode()               CLR_REG_BIT(CRECR, CRECR_MONO_MSK)
#define CRE_TwoStepsMode()              SET_REG_BIT(CRECR, CRECR_MONO_MSK)

#define CRE_CalibCycle_1ms()            MODIFY_REG(CRECR, CRECR_UPT_MSK, ((0) << 4))
#define CRE_CalibCycle_4ms()            MODIFY_REG(CRECR, CRECR_UPT_MSK, ((1) << 4))
#define CRE_CalibCycle_32ms()           MODIFY_REG(CRECR, CRECR_UPT_MSK, ((2) << 4))
#define CRE_CalibCycle_64ms()           MODIFY_REG(CRECR, CRECR_UPT_MSK, ((3) << 4))

#define CRE_LowFrequency()              CLR_REG_BIT(CRECR, CRECR_CREHF_MSK)
#define CRE_HighFrequency()             SET_REG_BIT(CRECR, CRECR_CREHF_MSK)

#define CRE_CalibReady()                READ_REG_BIT(CRECR, CRECR_CRERDY_MSK)

#define CRE_LFCNT(freq)                 ((16UL * (freq)) / 32768)
#define CRE_HFCNT(freq)                 ((8UL * (freq)) / 32768)

#define CRE_SetCalibCounter(n)          WRITE_REG(CRECNTH, HIBYTE(n)); \
                                        WRITE_REG(CRECNTL, LOBYTE(n))

#define CRE_LFERROR(freq, err)          ((BYTE)(CRE_LFCNT(freq) * (err) / 1000))
#define CRE_HFERROR(freq, err)          ((BYTE)(CRE_HFCNT(freq) * (err) / 1000))

#define CRE_SetCalibError(n)            WRITE_REG(CRERES, (n))

/////////////////////////////////////////////////

#define WDTCONTR_FLAG_MSK               BIT7
#define WDTCONTR_EN_MSK                 BIT5
#define WDTCONTR_CLR_MSK                BIT4
#define WDTCONTR_IDL_MSK                BIT3
#define WDTCONTR_PS_MSK                 (BIT2 | BIT1 | BIT0)

#define WDT_Enable()                    SET_REG_BIT(WDT_CONTR, WDTCONTR_EN_MSK)
#define WDT_Clear()                     SET_REG_BIT(WDT_CONTR, WDTCONTR_CLR_MSK)
#define WDT_CheckWDTReset()             READ_REG_BIT(WDT_CONTR, WDTCONTR_FLAG_MSK)
#define WDT_IdlePause()                 CLR_REG_BIT(WDT_CONTR, WDTCONTR_IDL_MSK)
#define WDT_IdleContinue()              SET_REG_BIT(WDT_CONTR, WDTCONTR_IDL_MSK)

#define WDT_SetPrescale(n)              MODIFY_REG(WDT_CONTR, WDTCONTR_PS_MSK, ((n) << 0))
#define WDT_SetPrescale2()              WDT_SetPrescale(0)
#define WDT_SetPrescale4()              WDT_SetPrescale(1)
#define WDT_SetPrescale8()              WDT_SetPrescale(2)
#define WDT_SetPrescale16()             WDT_SetPrescale(3)
#define WDT_SetPrescale32()             WDT_SetPrescale(4)
#define WDT_SetPrescale64()             WDT_SetPrescale(5)
#define WDT_SetPrescale128()            WDT_SetPrescale(6)
#define WDT_SetPrescale256()            WDT_SetPrescale(7)

/////////////////////////////////////////////////

#define CHIPID_ReadID0()                CHIPID0
#define CHIPID_ReadID1()                CHIPID1
#define CHIPID_ReadID2()                CHIPID2
#define CHIPID_ReadID3()                CHIPID3
#define CHIPID_ReadID4()                CHIPID4
#define CHIPID_ReadID5()                CHIPID5
#define CHIPID_ReadID6()                CHIPID6
#define CHIPID_ReadREFVoltage()         MAKEWORD(CHIPID8, CHIPID7)
#define CHIPID_ReadLIRCFrequency()      MAKEWORD(CHIPID10, CHIPID9)
#define CHIPID_Read22MIRCTrim()         CHIPID11
#define CHIPID_Read24MIRCTrim()         CHIPID12
#define CHIPID_Read27MIRCTrim()         CHIPID13
#define CHIPID_Read30MIRCTrim()         CHIPID14
#define CHIPID_Read33MIRCTrim()         CHIPID15
#define CHIPID_Read35MIRCTrim()         CHIPID16
#define CHIPID_Read36MIRCTrim()         CHIPID17
#define CHIPID_Read40MIRCTrim()         CHIPID18
#define CHIPID_Read44MIRCTrim()         CHIPID19
#define CHIPID_Read45MIRCTrim()         CHIPID20
#define CHIPID_Read6MVRTrim()           CHIPID21
#define CHIPID_Read10MVRTrim()          CHIPID22
#define CHIPID_Read27MVRTrim()          CHIPID23
#define CHIPID_Read44MVRTrim()          CHIPID24
#define CHIPID_ReadTag()                CHIPID31

#define CHIPIDX_Read33MIRCTrim()        CHIPIDX0
#define CHIPIDX_Read33MVRTrim()         CHIPIDX1
#define CHIPIDX_Read40MIRCTrim()        CHIPIDX2
#define CHIPIDX_Read40MVRTrim()         CHIPIDX3
#define CHIPIDX_Read42MIRCTrim()        CHIPIDX4
#define CHIPIDX_Read42MVRTrim()         CHIPIDX5

#define HIRC_22M1184()                          \
{                                               \
    /* CLKDIV = 0x04; */                        \
    IRTRIM = CHIPID_Read22MIRCTrim();           \
    VRTRIM = CHIPID_Read27MVRTrim();            \
    CLK_HIRC_27MHzBand();                       \
    /* CLKDIV = 0x00; */                        \
}

#define HIRC_24M()                              \
{                                               \
    /* CLKDIV = 0x04; */                        \
    IRTRIM = CHIPID_Read24MIRCTrim();           \
    VRTRIM = CHIPID_Read27MVRTrim();            \
    CLK_HIRC_27MHzBand();                       \
    /* CLKDIV = 0x00; */                        \
}

#define HIRC_27M()                              \
{                                               \
    /* CLKDIV = 0x04; */                        \
    IRTRIM = CHIPID_Read27MIRCTrim();           \
    if ((CHIPID_ReadTag() == 0x5b) &&           \
        (READ_REG_BIT(CHIPID25, BIT0)))         \
    {                                           \
        VRTRIM = CHIPID_Read44MVRTrim();        \
        CLK_HIRC_44MHzBand();                   \
    }                                           \
    else                                        \
    {                                           \
        VRTRIM = CHIPID_Read27MVRTrim();        \
        CLK_HIRC_27MHzBand();                   \
    }                                           \
    /* CLKDIV = 0x00; */                        \
}

#define HIRC_30M()                              \
{                                               \
    /* CLKDIV = 0x04; */                        \
    IRTRIM = CHIPID_Read30MIRCTrim();           \
    if ((CHIPID_ReadTag() == 0x5b) &&           \
        (READ_REG_BIT(CHIPID25, BIT1)))         \
    {                                           \
        VRTRIM = CHIPID_Read44MVRTrim();        \
        CLK_HIRC_44MHzBand();                   \
    }                                           \
    else                                        \
    {                                           \
        VRTRIM = CHIPID_Read27MVRTrim();        \
        CLK_HIRC_27MHzBand();                   \
    }                                           \
    /* CLKDIV = 0x00; */                        \
}

#define HIRC_33M1776()                          \
{                                               \
    /* CLKDIV = 0x04; */                        \
    IRTRIM = CHIPID_Read33MIRCTrim();           \
    if ((CHIPID_ReadTag() == 0x5b) &&           \
        (READ_REG_BIT(CHIPID25, BIT2)))         \
    {                                           \
        VRTRIM = CHIPID_Read44MVRTrim();        \
        CLK_HIRC_44MHzBand();                   \
    }                                           \
    else                                        \
    {                                           \
        VRTRIM = CHIPID_Read27MVRTrim();        \
        CLK_HIRC_27MHzBand();                   \
    }                                           \
    /* CLKDIV = 0x00; */                        \
}

#define HIRC_35M()                              \
{                                               \
    /* CLKDIV = 0x04; */                        \
    IRTRIM = CHIPID_Read35MIRCTrim();           \
    if ((CHIPID_ReadTag() == 0x5b) &&           \
        (READ_REG_BIT(CHIPID25, BIT3)))         \
    {                                           \
        VRTRIM = CHIPID_Read44MVRTrim();        \
        CLK_HIRC_44MHzBand();                   \
    }                                           \
    else                                        \
    {                                           \
        VRTRIM = CHIPID_Read27MVRTrim();        \
        CLK_HIRC_27MHzBand();                   \
    }                                           \
    /* CLKDIV = 0x00; */                        \
}

#define HIRC_36M864()                           \
{                                               \
    /* CLKDIV = 0x04; */                        \
    IRTRIM = CHIPID_Read36MIRCTrim();           \
    if ((CHIPID_ReadTag() == 0x5b) &&           \
        (READ_REG_BIT(CHIPID25, BIT4)))         \
    {                                           \
        VRTRIM = CHIPID_Read44MVRTrim();        \
        CLK_HIRC_44MHzBand();                   \
    }                                           \
    else                                        \
    {                                           \
        VRTRIM = CHIPID_Read27MVRTrim();        \
        CLK_HIRC_27MHzBand();                   \
    }                                           \
    /* CLKDIV = 0x00; */                        \
}

#define HIRC_40M()                              \
{                                               \
    /* CLKDIV = 0x04; */                        \
    IRTRIM = CHIPID_Read40MIRCTrim();           \
    if ((CHIPID_ReadTag() == 0x5b) &&           \
        (READ_REG_BIT(CHIPID25, BIT5)))         \
    {                                           \
        VRTRIM = CHIPID_Read44MVRTrim();        \
        CLK_HIRC_44MHzBand();                   \
    }                                           \
    else                                        \
    {                                           \
        VRTRIM = CHIPID_Read27MVRTrim();        \
        CLK_HIRC_27MHzBand();                   \
    }                                           \
    /* CLKDIV = 0x00; */                        \
}

#define HIRC_44M2368()                          \
{                                               \
    /* CLKDIV = 0x04; */                        \
    IRTRIM = CHIPID_Read44MIRCTrim();           \
    if ((CHIPID_ReadTag() == 0x5b) &&           \
        (READ_REG_BIT(CHIPID25, BIT6)))         \
    {                                           \
        VRTRIM = CHIPID_Read44MVRTrim();        \
        CLK_HIRC_44MHzBand();                   \
    }                                           \
    else                                        \
    {                                           \
        VRTRIM = CHIPID_Read27MVRTrim();        \
        CLK_HIRC_27MHzBand();                   \
    }                                           \
    /* CLKDIV = 0x00; */                        \
}

#define HIRC_45M1584()                          \
{                                               \
    /* CLKDIV = 0x04; */                        \
    IRTRIM = CHIPID_Read45MIRCTrim();           \
    if ((CHIPID_ReadTag() == 0x5b) &&           \
        (READ_REG_BIT(CHIPID25, BIT7)))         \
    {                                           \
        VRTRIM = CHIPID_Read44MVRTrim();        \
        CLK_HIRC_44MHzBand();                   \
    }                                           \
    else                                        \
    {                                           \
        VRTRIM = CHIPID_Read27MVRTrim();        \
        CLK_HIRC_27MHzBand();                   \
    }                                           \
    /* CLKDIV = 0x00; */                        \
}

#define HIRC_33M8688()                          \
{                                               \
    /* CLKDIV = 0x04; */                        \
    IRTRIM = CHIPIDX_Read33MIRCTrim();          \
    if (CHIPIDX_Read33MVRTrim() == 3)           \
    {                                           \
        VRTRIM = CHIPID_Read44MVRTrim();        \
        CLK_HIRC_44MHzBand();                   \
    }                                           \
    else                                        \
    {                                           \
        VRTRIM = CHIPID_Read27MVRTrim();        \
        CLK_HIRC_27MHzBand();                   \
    }                                           \
    /* CLKDIV = 0x00; */                        \
}

#define HIRC_40M96()                            \
{                                               \
    /* CLKDIV = 0x04; */                        \
    IRTRIM = CHIPIDX_Read40MIRCTrim();          \
    if (CHIPIDX_Read40MVRTrim() == 3)           \
    {                                           \
        VRTRIM = CHIPID_Read44MVRTrim();        \
        CLK_HIRC_44MHzBand();                   \
    }                                           \
    else                                        \
    {                                           \
        VRTRIM = CHIPID_Read27MVRTrim();        \
        CLK_HIRC_27MHzBand();                   \
    }                                           \
    /* CLKDIV = 0x00; */                        \
}

#define HIRC_42M()                              \
{                                               \
    /* CLKDIV = 0x04; */                        \
    IRTRIM = CHIPIDX_Read42MIRCTrim();          \
    if (CHIPIDX_Read42MVRTrim() == 3)           \
    {                                           \
        VRTRIM = CHIPID_Read44MVRTrim();        \
        CLK_HIRC_44MHzBand();                   \
    }                                           \
    else                                        \
    {                                           \
        VRTRIM = CHIPID_Read27MVRTrim();        \
        CLK_HIRC_27MHzBand();                   \
    }                                           \
    /* CLKDIV = 0x00; */                        \
}

/////////////////////////////////////////////////

#define PORT_SetFallingInt(p, b)        CLR_REG_BIT(P##p##IM0, (b)); \
                                        CLR_REG_BIT(P##p##IM1, (b))

#define PORT_SetRisingInt(p, b)         SET_REG_BIT(P##p##IM0, (b)); \
                                        CLR_REG_BIT(P##p##IM1, (b))

#define PORT_SetLowLevelInt(p, b)       CLR_REG_BIT(P##p##IM0, (b)); \
                                        SET_REG_BIT(P##p##IM1, (b))

#define PORT_SetHighLevelInt(p, b)      SET_REG_BIT(P##p##IM0, (b)); \
                                        SET_REG_BIT(P##p##IM1, (b))

#define SetP0nFallingInt(b)             PORT_SetFallingInt(0, (b))
#define SetP1nFallingInt(b)             PORT_SetFallingInt(1, (b))
#define SetP2nFallingInt(b)             PORT_SetFallingInt(2, (b))
#define SetP3nFallingInt(b)             PORT_SetFallingInt(3, (b))
#define SetP4nFallingInt(b)             PORT_SetFallingInt(4, (b))
#define SetP5nFallingInt(b)             PORT_SetFallingInt(5, (b))
#define SetP6nFallingInt(b)             PORT_SetFallingInt(6, (b))
#define SetP7nFallingInt(b)             PORT_SetFallingInt(7, (b))

#define SetP0nRisingInt(b)              PORT_SetRisingInt(0, (b))
#define SetP1nRisingInt(b)              PORT_SetRisingInt(1, (b))
#define SetP2nRisingInt(b)              PORT_SetRisingInt(2, (b))
#define SetP3nRisingInt(b)              PORT_SetRisingInt(3, (b))
#define SetP4nRisingInt(b)              PORT_SetRisingInt(4, (b))
#define SetP5nRisingInt(b)              PORT_SetRisingInt(5, (b))
#define SetP6nRisingInt(b)              PORT_SetRisingInt(6, (b))
#define SetP7nRisingInt(b)              PORT_SetRisingInt(7, (b))

#define SetP0nLowLevelInt(b)            PORT_SetLowLevelInt(0, (b))
#define SetP1nLowLevelInt(b)            PORT_SetLowLevelInt(1, (b))
#define SetP2nLowLevelInt(b)            PORT_SetLowLevelInt(2, (b))
#define SetP3nLowLevelInt(b)            PORT_SetLowLevelInt(3, (b))
#define SetP4nLowLevelInt(b)            PORT_SetLowLevelInt(4, (b))
#define SetP5nLowLevelInt(b)            PORT_SetLowLevelInt(5, (b))
#define SetP6nLowLevelInt(b)            PORT_SetLowLevelInt(6, (b))
#define SetP7nLowLevelInt(b)            PORT_SetLowLevelInt(7, (b))

#define SetP0nHighLevelInt(b)           PORT_SetHighLevelInt(0, (b))
#define SetP1nHighLevelInt(b)           PORT_SetHighLevelInt(1, (b))
#define SetP2nHighLevelInt(b)           PORT_SetHighLevelInt(2, (b))
#define SetP3nHighLevelInt(b)           PORT_SetHighLevelInt(3, (b))
#define SetP4nHighLevelInt(b)           PORT_SetHighLevelInt(4, (b))
#define SetP5nHighLevelInt(b)           PORT_SetHighLevelInt(5, (b))
#define SetP6nHighLevelInt(b)           PORT_SetHighLevelInt(6, (b))
#define SetP7nHighLevelInt(b)           PORT_SetHighLevelInt(7, (b))

#define PORT_EnableIntWaieup(p, b)      SET_REG_BIT(P##p##WKUE, (b))

#define SetP0nIntWaieup(b)              PORT_EnableIntWaieup(0, (b))
#define SetP1nIntWaieup(b)              PORT_EnableIntWaieup(1, (b))
#define SetP2nIntWaieup(b)              PORT_EnableIntWaieup(2, (b))
#define SetP3nIntWaieup(b)              PORT_EnableIntWaieup(3, (b))
#define SetP4nIntWaieup(b)              PORT_EnableIntWaieup(4, (b))
#define SetP5nIntWaieup(b)              PORT_EnableIntWaieup(5, (b))
#define SetP6nIntWaieup(b)              PORT_EnableIntWaieup(6, (b))
#define SetP7nIntWaieup(b)              PORT_EnableIntWaieup(7, (b))

/////////////////////////////////////////////////

#define TIMER0_Run()                    (TR0 = 1)
#define TIMER0_Stop()                   (TR0 = 0)

#define TIMER0_SetReload8(n)            (TH0 = LOBYTE(n), TL0 = LOBYTE(n))
#define TIMER0_SetReload16(n)           (TH0 = HIBYTE(n), TL0 = LOBYTE(n))
#define TIMER0_SetPrescale(n)           (TM0PS = (n))

#define TMOD_T0M_MSK                    (BIT1 | BIT0)
#define TIMER0_Mode0()                  MODIFY_REG(TMOD, TMOD_T0M_MSK, ((0) << 0))
#define TIMER0_Mode1()                  MODIFY_REG(TMOD, TMOD_T0M_MSK, ((1) << 0))
#define TIMER0_Mode2()                  MODIFY_REG(TMOD, TMOD_T0M_MSK, ((2) << 0))
#define TIMER0_Mode3()                  MODIFY_REG(TMOD, TMOD_T0M_MSK, ((3) << 0))

#define AUXR_T0X12_MSK                  BIT7
#define TIMER0_1TMode()                 SET_REG_BIT(AUXR, AUXR_T0X12_MSK)
#define TIMER0_12TMode()                CLR_REG_BIT(AUXR, AUXR_T0X12_MSK)

#define TMOD_T0CT_MSK                   BIT2
#define TIMER0_TimerMode()              CLR_REG_BIT(TMOD, TMOD_T0CT_MSK)
#define TIMER0_CounterMode()            SET_REG_BIT(TMOD, TMOD_T0CT_MSK)

#define TMOD_T0GATE_MSK                 BIT3
#define TIMER0_EnableGateINT0()         SET_REG_BIT(TMOD, TMOD_T0GATE_MSK)
#define TIMER0_DisableGateINT0()        CLR_REG_BIT(TMOD, TMOD_T0GATE_MSK)

#define INTCLKO_T0CLKO_MSK              BIT0
#define TIMER0_EnableCLKO()             SET_REG_BIT(INTCLKO, INTCLKO_T0CLKO_MSK)
#define TIMER0_DisableCLKO()            CLR_REG_BIT(INTCLKO, INTCLKO_T0CLKO_MSK)

////////////////////////

#define TIMER1_Run()                    (TR1 = 1)
#define TIMER1_Stop()                   (TR1 = 0)

#define TIMER1_SetReload8(n)            (TH1 = LOBYTE(n), TL1 = LOBYTE(n))
#define TIMER1_SetReload16(n)           (TH1 = HIBYTE(n), TL1 = LOBYTE(n))
#define TIMER1_SetPrescale(n)           (TM1PS = (n))

#define TMOD_T1M_MSK                    (BIT5 | BIT4)
#define TIMER1_Mode0()                  MODIFY_REG(TMOD, TMOD_T1M_MSK, ((0) << 0))
#define TIMER1_Mode1()                  MODIFY_REG(TMOD, TMOD_T1M_MSK, ((1) << 0))
#define TIMER1_Mode2()                  MODIFY_REG(TMOD, TMOD_T1M_MSK, ((2) << 0))

#define AUXR_T1X12_MSK                  BIT6
#define TIMER1_1TMode()                 SET_REG_BIT(AUXR, AUXR_T1X12_MSK)
#define TIMER1_12TMode()                CLR_REG_BIT(AUXR, AUXR_T1X12_MSK)

#define TMOD_T1CT_MSK                   BIT6
#define TIMER1_TimerMode()              CLR_REG_BIT(TMOD, TMOD_T1CT_MSK)
#define TIMER1_CounterMode()            SET_REG_BIT(TMOD, TMOD_T1CT_MSK)

#define TMOD_T1GATE_MSK                 BIT7
#define TIMER1_EnableGateINT1()         SET_REG_BIT(TMOD, TMOD_T1GATE_MSK)
#define TIMER1_DisableGateINT1()        CLR_REG_BIT(TMOD, TMOD_T1GATE_MSK)

#define INTCLKO_T1CLKO_MSK              BIT1
#define TIMER1_EnableCLKO()             SET_REG_BIT(INTCLKO, INTCLKO_T1CLKO_MSK)
#define TIMER1_DisableCLKO()            CLR_REG_BIT(INTCLKO, INTCLKO_T1CLKO_MSK)

////////////////////////

#define AUXR_T2R_MSK                    BIT4
#define TIMER2_Run()                    SET_REG_BIT(AUXR, AUXR_T2R_MSK)
#define TIMER2_Stop()                   CLR_REG_BIT(AUXR, AUXR_T2R_MSK)

#define TIMER2_SetReload16(n)           (T2H = HIBYTE(n), T2L = LOBYTE(n))
#define TIMER2_SetPrescale(n)           (TM2PS = (n))

#define AUXR_T2X12_MSK                  BIT2
#define TIMER2_1TMode()                 SET_REG_BIT(AUXR, AUXR_T2X12_MSK)
#define TIMER2_12TMode()                CLR_REG_BIT(AUXR, AUXR_T2X12_MSK)

#define AUXR_T2CT_MSK                   BIT3
#define TIMER2_TimerMode()              CLR_REG_BIT(AUXR, AUXR_T2CT_MSK)
#define TIMER2_CounterMode()            SET_REG_BIT(AUXR, AUXR_T2CT_MSK)

#define INTCLKO_T2CLKO_MSK              BIT2
#define TIMER2_EnableCLKO()             SET_REG_BIT(INTCLKO, INTCLKO_T2CLKO_MSK)
#define TIMER2_DisableCLKO()            CLR_REG_BIT(INTCLKO, INTCLKO_T2CLKO_MSK)

////////////////////////

#define T4T3M_T3R_MSK                   BIT3
#define TIMER3_Run()                    SET_REG_BIT(T4T3M, T4T3M_T3R_MSK)
#define TIMER3_Stop()                   CLR_REG_BIT(T4T3M, T4T3M_T3R_MSK)

#define TIMER3_SetReload16(n)           (T3H = HIBYTE(n), T3L = LOBYTE(n))
#define TIMER3_SetPrescale(n)           (TM3PS = (n))

#define T4T3M_T3X12_MSK                 BIT1
#define TIMER3_1TMode()                 SET_REG_BIT(T4T3M, T4T3M_T3X12_MSK)
#define TIMER3_12TMode()                CLR_REG_BIT(T4T3M, T4T3M_T3X12_MSK)

#define T4T3M_T3CT_MSK                  BIT2
#define TIMER3_TimerMode()              CLR_REG_BIT(T4T3M, T4T3M_T3CT_MSK)
#define TIMER3_CounterMode()            SET_REG_BIT(T4T3M, T4T3M_T3CT_MSK)

#define T4T3M_T3CLKO_MSK                BIT0
#define TIMER3_EnableCLKO()             SET_REG_BIT(T4T3M, T4T3M_T3CLKO_MSK)
#define TIMER3_DisableCLKO()            CLR_REG_BIT(T4T3M, T4T3M_T3CLKO_MSK)

////////////////////////

#define T4T3M_T4R_MSK                   BIT7
#define TIMER4_Run()                    SET_REG_BIT(T4T3M, T4T3M_T4R_MSK)
#define TIMER4_Stop()                   CLR_REG_BIT(T4T3M, T4T3M_T4R_MSK)

#define TIMER4_SetReload16(n)           (T4H = HIBYTE(n), T4L = LOBYTE(n))
#define TIMER4_SetPrescale(n)           (TM4PS = (n))

#define T4T3M_T4X12_MSK                 BIT5
#define TIMER4_1TMode()                 SET_REG_BIT(T4T3M, T4T3M_T4X12_MSK)
#define TIMER4_12TMode()                CLR_REG_BIT(T4T3M, T4T3M_T4X12_MSK)

#define T4T3M_T4CT_MSK                  BIT6
#define TIMER4_TimerMode()              CLR_REG_BIT(T4T3M, T4T3M_T4CT_MSK)
#define TIMER4_CounterMode()            SET_REG_BIT(T4T3M, T4T3M_T4CT_MSK)

#define T4T3M_T4CLKO_MSK                BIT4
#define TIMER4_EnableCLKO()             SET_REG_BIT(T4T3M, T4T3M_T4CLKO_MSK)
#define TIMER4_DisableCLKO()            CLR_REG_BIT(T4T3M, T4T3M_T4CLKO_MSK)

////////////////////////

#define T11CR_T11R_MSK                  BIT7
#define TIMER11_Run()                   SET_REG_BIT(T11CR, T11CR_T11R_MSK)
#define TIMER11_Stop()                  CLR_REG_BIT(T11CR, T11CR_T11R_MSK)

#define TIMER11_CheckFlag()             READ_REG_BIT(T11CR, T11CR_T11IF_MSK)
#define TIMER11_ClearFlag()             CLR_REG_BIT(T11CR, T11CR_T11IF_MSK)

#define TIMER11_SetReload16(n)          (T11H = HIBYTE(n), T11L = LOBYTE(n))
#define TIMER11_SetPrescale(n)          (T11PS = (n))

#define T11CR_T11X12_MSK                BIT4
#define TIMER11_1TMode()                SET_REG_BIT(T11CR, T11CR_T11X12_MSK)
#define TIMER11_12TMode()               CLR_REG_BIT(T11CR, T11CR_T11X12_MSK)

#define T11CR_T11CT_MSK                 BIT6
#define TIMER11_TimerMode()             CLR_REG_BIT(T11CR, T11CR_T11CT_MSK)
#define TIMER11_CounterMode()           SET_REG_BIT(T11CR, T11CR_T11CT_MSK)

#define T11CR_T11CLKO_MSK               BIT5
#define TIMER11_EnableCLKO()            SET_REG_BIT(T11CR, T11CR_T11CLKO_MSK)
#define TIMER11_DisableCLKO()           CLR_REG_BIT(T11CR, T11CR_T11CLKO_MSK)

#define T11CR_T11CS_MSK                 (BIT3 | BIT2)
#define TIMER11_ClockSource(n)          MODIFY_REG(T11CR, T11CR_T11CS_MSK, ((n) << 2))
#define TIMER11_CLK_SYSCLK()            TIMER11_ClockSource(0)
#define TIMER11_CLK_HIRC()              TIMER11_ClockSource(1)
#define TIMER11_CLK_X32K()              TIMER11_ClockSource(2)
#define TIMER11_CLK_LIRC()              TIMER11_ClockSource(3)

/////////////////////////////////////////////////

#define SCON_SM_MSK                     (BIT7 | BIT6)
#define UART1_SetMode(n)                MODIFY_REG(SCON, SCON_SM_MSK, ((n) << 6))
#define UART1_Mode0()                   UART1_SetMode(0)
#define UART1_Mode1()                   UART1_SetMode(1)
#define UART1_Mode2()                   UART1_SetMode(2)
#define UART1_Mode3()                   UART1_SetMode(3)

#define UART1_EnableRx()                (REN = 1)
#define UART1_DisableRx()               (REN = 0)
#define UART1_SetTB8(b)                 (TB8 = (b))
#define UART1_ReadRB8()                 (RB8)

#define UART1_SendData(d)               (SBUF = (d))
#define UART1_ReadData()                (SBUF)

#define PCON_SMOD_MSK                   BIT7
#define UART1_BaudrateX2()              SET_REG_BIT(PCON, PCON_SMOD_MSK)

#define AUXR_M0X6_MSK                   BIT5
#define UART1_Mode0BaudrateX6()         SET_REG_BIT(AUXR, AUXR_M0X6_MSK)

#define USARTCR2_PCEN_MSK               BIT2
#define USARTCR2_PS_MSK                 BIT1
#define UART1_NoneParity()              CLR_REG_BIT(USARTCR2, USARTCR2_PCEN_MSK)
#define UART1_OddParity()               SET_REG_BIT(USARTCR2, USARTCR2_PCEN_MSK); \
                                        SET_REG_BIT(USARTCR2, USARTCR2_PS_MSK)
#define UART1_EvenParity()              SET_REG_BIT(USARTCR2, USARTCR2_PCEN_MSK); \
                                        CLR_REG_BIT(USARTCR2, USARTCR2_PS_MSK)
#define UART1_MarkParity()              CLR_REG_BIT(USARTCR2, USARTCR2_PCEN_MSK); \
                                        UART1_SetTB8(1)
#define UART1_SpaceParity()             CLR_REG_BIT(USARTCR2, USARTCR2_PCEN_MSK); \
                                        UART1_SetTB8(0)

#define UR1TOCR_ENTO_MSK                BIT7
#define UART1_EnableTimeout()           SET_REG_BIT(UR1TOCR, UR1TOCR_ENTO_MSK)
#define UART1_DisableTimeout()          CLR_REG_BIT(UR1TOCR, UR1TOCR_ENTO_MSK)

#define UR1TOCR_SCALE_MSK               BIT5
#define UART1_TimeoutScale_SYSCLK()     SET_REG_BIT(UR1TOCR, UR1TOCR_SCALE_MSK)
#define UART1_TimeoutScale_BRT()        CLR_REG_BIT(UR1TOCR, UR1TOCR_SCALE_MSK)

#define UART1_SetTimeoutInterval(n)     UR1TOTL = BYTE0(n); \
                                        UR1TOTH = BYTE1(n); \
                                        UR1TOTE = BYTE2(n)

#define AUXR_S1BRT_MSK                  BIT0
#define UART1_Timer1BRT()               CLR_REG_BIT(AUXR, AUXR_S1BRT_MSK)
#define UART1_Timer2BRT()               SET_REG_BIT(AUXR, AUXR_S1BRT_MSK)

////////////////////////

#define S2CON_S2SM_MSK                  (BIT7 | BIT6)
#define UART2_SetMode(n)                MODIFY_REG(S2CON, S2CON_S2SM_MSK, ((n) << 6))
#define UART2_Mode0()                   UART2_SetMode(0)
#define UART2_Mode1()                   UART2_SetMode(1)
#define UART2_Mode2()                   UART2_SetMode(2)
#define UART2_Mode3()                   UART2_SetMode(3)

#define S2CON_S2REN_MSK                 BIT4
#define UART2_EnableRx()                SET_REG_BIT(S2CON, S2CON_S2REN_MSK)
#define UART2_DisableRx()               CLR_REG_BIT(S2CON, S2CON_S2REN_MSK)

#define S2CON_S2TB8_MSK                 BIT3
#define UART2_SetTB8(b)                 MODIFY_REG(S2CON, S2CON_S2TB8_MSK, ((b) << 3))

#define S2CON_S2RB8_MSK                 BIT2
#define UART2_ReadRB8()                 READ_REG_BIT(S2CON, S2CON_S2RB8_MSK)

#define UART2_SendData(d)               (S2BUF = (d))
#define UART2_ReadData()                (S2BUF)

#define S2CFG_S2M0X6_MSK                BIT5
#define UART2_Mode0BaudrateX6()         SET_REG_BIT(S2CFG, S2CFG_S2M0X6_MSK)

#define USART2CR2_PCEN_MSK              BIT2
#define USART2CR2_PS_MSK                BIT1
#define UART2_NoneParity()              CLR_REG_BIT(USART2CR2, USART2CR2_PCEN_MSK)
#define UART2_OddParity()               SET_REG_BIT(USART2CR2, USART2CR2_PCEN_MSK); \
                                        SET_REG_BIT(USART2CR2, USART2CR2_PS_MSK)
#define UART2_EvenParity()              SET_REG_BIT(USART2CR2, USART2CR2_PCEN_MSK); \
                                        CLR_REG_BIT(USART2CR2, USART2CR2_PS_MSK)
#define UART2_MarkParity()              CLR_REG_BIT(USART2CR2, USART2CR2_PCEN_MSK); \
                                        UART2_SetTB8(1)
#define UART2_SpaceParity()             CLR_REG_BIT(USART2CR2, USART2CR2_PCEN_MSK); \
                                        UART2_SetTB8(0)

#define UR2TOCR_ENTO_MSK                BIT7
#define UART2_EnableTimeout()           SET_REG_BIT(UR2TOCR, UR2TOCR_ENTO_MSK)
#define UART2_DisableTimeout()          CLR_REG_BIT(UR2TOCR, UR2TOCR_ENTO_MSK)

#define UR2TOCR_SCALE_MSK               BIT5
#define UART2_TimeoutScale_SYSCLK()     SET_REG_BIT(UR2TOCR, UR2TOCR_SCALE_MSK)
#define UART2_TimeoutScale_BRT()        CLR_REG_BIT(UR2TOCR, UR2TOCR_SCALE_MSK)

#define UART2_SetTimeoutInterval(n)     UR2TOTL = BYTE0(n); \
                                        UR2TOTH = BYTE1(n); \
                                        UR2TOTE = BYTE2(n)

////////////////////////

#define S3CON_S3SM_MSK                  BIT7
#define UART3_SetMode(n)                MODIFY_REG(S3CON, S3CON_S3SM_MSK, ((n) << 7))
#define UART3_Mode0()                   CLR_REG_BIT(S3CON, S3CON_S3SM_MSK)
#define UART3_Mode1()                   SET_REG_BIT(S3CON, S3CON_S3SM_MSK)

#define S3CON_S3REN_MSK                 BIT4
#define UART3_EnableRx()                SET_REG_BIT(S3CON, S3CON_S3REN_MSK)
#define UART3_DisableRx()               CLR_REG_BIT(S3CON, S3CON_S3REN_MSK)

#define S3CON_S3TB8_MSK                 BIT3
#define UART3_SetTB8(b)                 MODIFY_REG(S3CON, S3CON_S3TB8_MSK, ((b) << 3))

#define S3CON_S3RB8_MSK                 BIT2
#define UART3_ReadRB8()                 READ_REG_BIT(S3CON, S3CON_S3RB8_MSK)

#define UART3_SendData(d)               (S3BUF = (d))
#define UART3_ReadData()                (S3BUF)

#define UR3TOCR_ENTO_MSK                BIT7
#define UART3_EnableTimeout()           SET_REG_BIT(UR3TOCR, UR3TOCR_ENTO_MSK)
#define UART3_DisableTimeout()          CLR_REG_BIT(UR3TOCR, UR3TOCR_ENTO_MSK)

#define UR3TOCR_SCALE_MSK               BIT5
#define UART3_TimeoutScale_SYSCLK()     SET_REG_BIT(UR3TOCR, UR3TOCR_SCALE_MSK)
#define UART3_TimeoutScale_BRT()        CLR_REG_BIT(UR3TOCR, UR3TOCR_SCALE_MSK)

#define UART3_SetTimeoutInterval(n)     UR3TOTL = BYTE0(n); \
                                        UR3TOTH = BYTE1(n); \
                                        UR3TOTE = BYTE2(n)

#define S3CON_S3BRT_MSK                 BIT6
#define UART3_Timer2BRT()               CLR_REG_BIT(S3CON, S3CON_S3BRT_MSK)
#define UART3_Timer3BRT()               SET_REG_BIT(S3CON, S3CON_S3BRT_MSK)

////////////////////////

#define S4CON_S4SM_MSK                  BIT7
#define UART4_SetMode(n)                MODIFY_REG(S4CON, S4CON_S4SM_MSK, ((n) << 7))
#define UART4_Mode0()                   CLR_REG_BIT(S4CON, S4CON_S4SM_MSK)
#define UART4_Mode1()                   SET_REG_BIT(S4CON, S4CON_S4SM_MSK)

#define S4CON_S4REN_MSK                 BIT4
#define UART4_EnableRx()                SET_REG_BIT(S4CON, S4CON_S4REN_MSK)
#define UART4_DisableRx()               CLR_REG_BIT(S4CON, S4CON_S4REN_MSK)

#define S4CON_S4TB8_MSK                 BIT3
#define UART4_SetTB8(b)                 MODIFY_REG(S4CON, S4CON_S4TB8_MSK, ((b) << 3))

#define S4CON_S4RB8_MSK                 BIT2
#define UART4_ReadRB8()                 READ_REG_BIT(S4CON, S4CON_S4RB8_MSK)

#define UART4_SendData(d)               (S4BUF = (d))
#define UART4_ReadData()                (S4BUF)

#define UR4TOCR_ENTO_MSK                BIT7
#define UART4_EnableTimeout()           SET_REG_BIT(UR4TOCR, UR4TOCR_ENTO_MSK)
#define UART4_DisableTimeout()          CLR_REG_BIT(UR4TOCR, UR4TOCR_ENTO_MSK)

#define UR4TOCR_SCALE_MSK               BIT5
#define UART4_TimeoutScale_SYSCLK()     SET_REG_BIT(UR4TOCR, UR4TOCR_SCALE_MSK)
#define UART4_TimeoutScale_BRT()        CLR_REG_BIT(UR4TOCR, UR4TOCR_SCALE_MSK)

#define UART4_SetTimeoutInterval(n)     UR4TOTL = BYTE0(n); \
                                        UR4TOTH = BYTE1(n); \
                                        UR4TOTE = BYTE2(n)

#define S4CON_S4BRT_MSK                 BIT6
#define UART4_Timer2BRT()               CLR_REG_BIT(S4CON, S4CON_S4BRT_MSK)
#define UART4_Timer4BRT()               SET_REG_BIT(S4CON, S4CON_S4BRT_MSK)

/////////////////////////////////////////////////

#define CMPCR1_CMPEN_MSK                BIT7
#define CMP_Enable()                    SET_REG_BIT(CMPCR1, CMPCR1_CMPEN_MSK)
#define CMP_Disable()                   CLR_REG_BIT(CMPCR1, CMPCR1_CMPEN_MSK)

#define CMPCR1_CMPOE_MSK                BIT1
#define CMP_EnableOutput()              SET_REG_BIT(CMPCR1, CMPCR1_CMPOE_MSK)
#define CMP_DisableOutput()             CLR_REG_BIT(CMPCR1, CMPCR1_CMPOE_MSK)

#define CMPCR1_CMPRES_MSK               BIT0
#define CMP_ReadResult()                READ_REG_BIT(CMPCR1, CMPCR1_CMPRES_MSK)

#define CMPCR2_INVO_MSK                 BIT7
#define CMP_OutputInvert()              SET_REG_BIT(CMPCR2, CMPCR2_INVO_MSK)

#define CMPCR2_DISFLT_MSK               BIT6
#define CMP_EnableAnalogFilter()        CLR_REG_BIT(CMPCR2, CMPCR2_DISFLT_MSK)
#define CMP_DisableAnalogFilter()       SET_REG_BIT(CMPCR2, CMPCR2_DISFLT_MSK)

#define CMPCR2_LCDTY_MSK                (BIT5 | BIT4 | BIT_LN)
#define CMP_SetDigitalFilter(n)         MODIFY_REG(CMPCR2, CMPCR2_LCDTY_MSK, ((n) << 0))

#define CMPEXCFG_CHYS_MSK               (BIT7 | BIT6)
#define CMP_SetHysteresis(n)            MODIFY_REG(CMPEXCFG, CMPEXCFG_CHYS_MSK, ((n) << 6))
#define CMP_DisableHysteresis()         CMP_SetHysteresis(0)
#define CMP_SetHysteresis_10mV()        CMP_SetHysteresis(1)
#define CMP_SetHysteresis_20mV()        CMP_SetHysteresis(2)
#define CMP_SetHysteresis_30mV()        CMP_SetHysteresis(3)

/////////////////////////////////////////////////

#define IAPCON_IAPEN_MSK                BIT7
#define IAP_Enable()                    SET_REG_BIT(IAP_CONTR, IAPCON_IAPEN_MSK)
#define IAP_Disable()                   CLR_REG_BIT(IAP_CONTR, IAPCON_IAPEN_MSK)

#define IAP_SetData(d)                  (IAP_DATA = (d))
#define IAP_ReadData()                  (IAP_DATA)

#define IAP_SetAddress(n)               IAP_ADDRL = BYTE0(n); \
                                        IAP_ADDRH = BYTE1(n); \
                                        IAP_ADDRE = BYTE2(n)

#define IAP_Trigger()                   IAP_TRIG = 0x5a; \
                                        IAP_TRIG = 0xa5; \
                                        _nop_();         \
                                        _nop_();         \
                                        _nop_();         \
                                        _nop_()

#define IAP_Idle()                      IAP_CMD = 0
#define IAP_TriggerRead()               IAP_CMD = 1; IAP_Trigger()
#define IAP_TriggerProgram()            IAP_CMD = 2; IAP_Trigger()
#define IAP_TriggerErase()              IAP_CMD = 3; IAP_Trigger()

#define IAP_SetTimeBase()               IAP_TPS = ((SYSCLK) / 1000000)

#define IAPCON_FAIL_MSK                 BIT4
#define IAP_CheckErrorFlag()            READ_REG_BIT(IAP_CONTR, IAPCON_FAIL_MSK)
#define IAP_ClearErrorFlag()            CLR_REG_BIT(IAP_CONTR, IAPCON_FAIL_MSK)

/////////////////////////////////////////////////

#define ADCCONTR_ADCPOWER_MSK           BIT7
#define ADC_Enable()                    SET_REG_BIT(ADC_CONTR, ADCCONTR_ADCPOWER_MSK)
#define ADC_Disable()                   CLR_REG_BIT(ADC_CONTR, ADCCONTR_ADCPOWER_MSK)

#define ADC_ReadResult()                MAKEWORD(ADC_RESL, ADC_RES)

#define ADCCONTR_ADCSTART_MSK           BIT6
#define ADC_Start()                     SET_REG_BIT(ADC_CONTR, ADCCONTR_ADCSTART_MSK)

#define ADCCONTR_ADCCHS_MSK             (BIT_LN)
#define ADC_ActiveChannel(n)            MODIFY_REG(ADC_CONTR, ADCCONTR_ADCCHS_MSK, ((n) << 0))

#define ADCCONTR_ADCEPWMT_MSK           BIT4
#define ADC_EnablePWMTrig()             SET_REG_BIT(ADC_CONTR, ADCCONTR_ADCEPWMT_MSK)

#define ADCCFG_RESFMT_MSK               BIT5
#define ADC_ResultLeftAlign()           CLR_REG_BIT(ADCCFG, ADCCFG_RESFMT_MSK)
#define ADC_ResultRightAlign()          SET_REG_BIT(ADCCFG, ADCCFG_RESFMT_MSK)

#define ADCCFG_SPEED_MSK                (BIT_LN)
#define ADC_SetClockDivider(n)          MODIFY_REG(ADCCFG, ADCCFG_SPEED_MSK, ((n) << 0))

#define ADCEXCFG_ETRE_MSK               BIT5
#define ADC_EnableETR()                 SET_REG_BIT(ADCEXCFG, ADCEXCFG_ETRE_MSK)
#define ADC_DisableETR()                CLR_REG_BIT(ADCEXCFG, ADCEXCFG_ETRE_MSK)

#define ADCEXCFG_ETRP_MSK               BIT4
#define ADC_ETRRising()                 CLR_REG_BIT(ADCEXCFG, ADCEXCFG_ETRP_MSK)
#define ADC_ETRFalling()                SET_REG_BIT(ADCEXCFG, ADCEXCFG_ETRP_MSK)

#define ADCEXCFG_CVT_MSK                (BIT2 | BIT1 | BIT0)
#define ADC_SetRepeatTimes(n)           MODIFY_REG(ADCEXCFG, ADCEXCFG_CVT_MSK, ((n) << 0))

#define ADC_DisableRepeatConv()         ADC_SetRepeatTimes(0)
#define ADC_SetRepeat2Times()           ADC_SetRepeatTimes(4)
#define ADC_SetRepeat4Times()           ADC_SetRepeatTimes(5)
#define ADC_SetRepeat8Times()           ADC_SetRepeatTimes(6)
#define ADC_SetRepeat16Times()          ADC_SetRepeatTimes(7)

#define ADCTIM_CSSETUP_MSK              BIT7
#define ADCTIM_CSHOLD_MSK               (BIT6 | BIT5)
#define ADCTIM_SMPDUTY_MSK              (BIT4 | BIT_LN)
#define ADC_SetCSSetupCycles(n)         MODIFY_REG(ADCTIM, ADCTIM_CSSETUP_MSK, (((n) - 1) << 7))
#define ADC_SetCSHoldCycles(n)          MODIFY_REG(ADCTIM, ADCTIM_CSHOLD_MSK, (((n) - 1) << 5))
#define ADC_SetSampleDutyCycles(n)      MODIFY_REG(ADCTIM, ADCTIM_SMPDUTY_MSK, (((n) - 1) << 0))

/////////////////////////////////////////////////

#define CCON_CR_MSK                     BIT6
#define PCA_Run()                       SET_REG_BIT(CCON, CCON_CR_MSK)
#define PCA_Stop()                      CLR_REG_BIT(CCON, CCON_CR_MSK)

#define CMOD_CIDL_MSK                   BIT7
#define PCA_IdlePause()                 SET_REG_BIT(CMOD, CMOD_CIDL_MSK)
#define PCA_IdleContinue()              CLR_REG_BIT(CMOD, CMOD_CIDL_MSK)

#define CMOD_CPS_MSK                    (BIT4 | BIT3 |BIT2 |BIT1)
#define PCA_SetClockSource(n)           MODIFY_REG(CMOD, CMOD_CPS_MSK, ((n) << 1))
#define PCA_CLK_SYSCLKD12()             PCA_SetClockSource(0)
#define PCA_CLK_SYSCLKD2()              PCA_SetClockSource(1)
#define PCA_CLK_TIMER0OV()              PCA_SetClockSource(2)
#define PCA_CLK_ECI()                   PCA_SetClockSource(3)
#define PCA_CLK_SYSCLK()                PCA_SetClockSource(4)
#define PCA_CLK_SYSCLKD4()              PCA_SetClockSource(5)
#define PCA_CLK_SYSCLKD6()              PCA_SetClockSource(6)
#define PCA_CLK_SYSCLKD8()              PCA_SetClockSource(7)
#define PCA_CLK_SYSCLKD3()              PCA_SetClockSource(8)

#define PCA_SetCounter(n)               (CH = HIBYTE(n), CL = LOBYTE(n))
#define PCA_InitCounter()               PCA_SetCounter(0)

#define PCAPWMn_XCCAPH_MSK              (BIT5 | BIT4)
#define PCAPWMn_XCCAPL_MSK              (BIT3 | BIT2)
#define PCA_SetCnReload(n, v)           MODIFY_REG(PCA_PWM##n, PCAPWMn_XCCAPH_MSK, (HIBYTE(v) << 4)); \
                                        MODIFY_REG(PCA_PWM##n, PCAPWMn_XCCAPL_MSK, (HIBYTE(v) << 2)); \
                                        CCAP##n##L = LOBYTE(v); \
                                        CCAP##n##H = LOBYTE(v)

#define PCA_SetCnMatch(n, v)            (CCAP##n##L = LOBYTE(v), CCAP##n##H = HIBYTE(v))

#define PCA_ReadCnCapture(n)            MAKEWORD(CCAP##n##L, CCAP##n##H)

#define CCAPMn_ECOM_MSK                 BIT6
#define CCAPMn_CCAPP_MSK                BIT5
#define CCAPMn_CCAPN_MSK                BIT4
#define CCAPMn_MAT_MSK                  BIT3
#define CCAPMn_TOG_MSK                  BIT2
#define CCAPMn_PWM_MSK                  BIT1
#define PCAPWMn_EBS_MSK                 (BIT7 | BIT6)

#define PCA_DisableCn(n)                CCAPM##n = 0

#define PCA_SetCnAs6BitPWMMode(n)       CCAPM##n = (CCAPMn_ECOM_MSK | CCAPMn_PWM_MSK); \
                                        MODIFY_REG(PCA_PWM##n, PCAPWMn_EBS_MSK, ((2) << 6))
#define PCA_SetCnAs7BitPWMMode(n)       CCAPM##n = (CCAPMn_ECOM_MSK | CCAPMn_PWM_MSK); \
                                        MODIFY_REG(PCA_PWM##n, PCAPWMn_EBS_MSK, ((1) << 6))
#define PCA_SetCnAs8BitPWMMode(n)       CCAPM##n = (CCAPMn_ECOM_MSK | CCAPMn_PWM_MSK); \
                                        MODIFY_REG(PCA_PWM##n, PCAPWMn_EBS_MSK, ((0) << 6))
#define PCA_SetCnAs10BitPWMMode(n)      CCAPM##n = (CCAPMn_ECOM_MSK | CCAPMn_PWM_MSK); \
                                        MODIFY_REG(PCA_PWM##n, PCAPWMn_EBS_MSK, ((3) << 6))

#define PCA_EnableCnCapturePWMPos(n)    MODIFY_REG(CCAPM##n, CCAPMn_CCAPP_MSK | CCAPMn_CCAPN_MSK, (CCAPMn_CCAPP_MSK))
#define PCA_EnableCnCapturePWMNeg(n)    MODIFY_REG(CCAPM##n, CCAPMn_CCAPP_MSK | CCAPMn_CCAPN_MSK, (CCAPMn_CCAPN_MSK))
#define PCA_EnableCnCapturePWMEdge(n)   CCAPM##n |= (CCAPMn_CCAPP_MSK | CCAPMn_CCAPN_MSK)

#define PCA_SetCnAsCapturePosMode(n)    CCAPM##n = (CCAPMn_CCAPP_MSK)
#define PCA_SetCnAsCaptureNegMode(n)    CCAPM##n = (CCAPMn_CCAPN_MSK)
#define PCA_SetCnAsCaptureEdgeMode(n)   CCAPM##n = (CCAPMn_CCAPP_MSK | CCAPMn_CCAPN_MSK)

#define PCA_SetCnAs16BitTimerMode(n)    CCAPM##n = (CCAPMn_ECOM_MSK | CCAPMn_MAT_MSK)

#define PCA_SetCnAsHighSpeedPulseMode(n) CCAPM##n = (CCAPMn_ECOM_MSK | CCAPMn_MAT_MSK | CCAPMn_TOG_MSK)

#define PCA_DisableC0()                 PCA_DisableCn(0)
#define PCA_DisableC1()                 PCA_DisableCn(1)
#define PCA_DisableC2()                 PCA_DisableCn(2)

#define PCA_SetC0Reload(v)              PCA_SetCnReload(0, (v))
#define PCA_SetC1Reload(v)              PCA_SetCnReload(1, (v))
#define PCA_SetC2Reload(v)              PCA_SetCnReload(2, (v))

#define PCA_SetC0Match(v)               PCA_SetCnMatch(0, (v))
#define PCA_SetC1Match(v)               PCA_SetCnMatch(1, (v))
#define PCA_SetC2Match(v)               PCA_SetCnMatch(2, (v))

#define PCA_ReadC0Capture()             PCA_ReadCnCapture(0)
#define PCA_ReadC1Capture()             PCA_ReadCnCapture(1)
#define PCA_ReadC2Capture()             PCA_ReadCnCapture(2)

#define PCA_SetC0As6BitPWMMode()        PCA_SetCnAs6BitPWMMode(0)
#define PCA_SetC1As6BitPWMMode()        PCA_SetCnAs6BitPWMMode(1)
#define PCA_SetC2As6BitPWMMode()        PCA_SetCnAs6BitPWMMode(2)

#define PCA_SetC0As7BitPWMMode()        PCA_SetCnAs7BitPWMMode(0)
#define PCA_SetC1As7BitPWMMode()        PCA_SetCnAs7BitPWMMode(1)
#define PCA_SetC2As7BitPWMMode()        PCA_SetCnAs7BitPWMMode(2)

#define PCA_SetC0As8BitPWMMode()        PCA_SetCnAs8BitPWMMode(0)
#define PCA_SetC1As8BitPWMMode()        PCA_SetCnAs8BitPWMMode(1)
#define PCA_SetC2As8BitPWMMode()        PCA_SetCnAs8BitPWMMode(2)

#define PCA_SetC0As10BitPWMMode()       PCA_SetCnAs10BitPWMMode(0)
#define PCA_SetC1As10BitPWMMode()       PCA_SetCnAs10BitPWMMode(1)
#define PCA_SetC2As10BitPWMMode()       PCA_SetCnAs10BitPWMMode(2)

#define PCA_EnableC0CapturePWMPos()     PCA_EnableCnCapturePWMPos(0)
#define PCA_EnableC1CapturePWMPos()     PCA_EnableCnCapturePWMPos(1)
#define PCA_EnableC2CapturePWMPos()     PCA_EnableCnCapturePWMPos(2)

#define PCA_EnableC0CapturePWMNeg()     PCA_EnableCnCapturePWMNeg(0)
#define PCA_EnableC1CapturePWMNeg()     PCA_EnableCnCapturePWMNeg(1)
#define PCA_EnableC2CapturePWMNeg()     PCA_EnableCnCapturePWMNeg(2)

#define PCA_EnableC0CapturePWMEdge()    PCA_EnableCnCapturePWMEdge(0)
#define PCA_EnableC1CapturePWMEdge()    PCA_EnableCnCapturePWMEdge(1)
#define PCA_EnableC2CapturePWMEdge()    PCA_EnableCnCapturePWMEdge(2)

#define PCA_SetC0AsCapturePosMode()     PCA_SetCnAsCapturePosMode(0)
#define PCA_SetC1AsCapturePosMode()     PCA_SetCnAsCapturePosMode(1)
#define PCA_SetC2AsCapturePosMode()     PCA_SetCnAsCapturePosMode(2)

#define PCA_SetC0AsCaptureNegMode()     PCA_SetCnAsCaptureNegMode(0)
#define PCA_SetC1AsCaptureNegMode()     PCA_SetCnAsCaptureNegMode(1)
#define PCA_SetC2AsCaptureNegMode()     PCA_SetCnAsCaptureNegMode(2)

#define PCA_SetC0AsCaptureEdgeMode()    PCA_SetCnAsCaptureEdgeMode(0)
#define PCA_SetC1AsCaptureEdgeMode()    PCA_SetCnAsCaptureEdgeMode(1)
#define PCA_SetC2AsCaptureEdgeMode()    PCA_SetCnAsCaptureEdgeMode(2)

#define PCA_SetC0As16BitTimerMode()     PCA_SetCnAs16BitTimerMode(0)
#define PCA_SetC1As16BitTimerMode()     PCA_SetCnAs16BitTimerMode(1)
#define PCA_SetC2As16BitTimerMode()     PCA_SetCnAs16BitTimerMode(2)

#define PCA_SetC0AsHighSpeedPulseMode() PCA_SetCnAsHighSpeedPulseMode(0)
#define PCA_SetC1AsHighSpeedPulseMode() PCA_SetCnAsHighSpeedPulseMode(1)
#define PCA_SetC2AsHighSpeedPulseMode() PCA_SetCnAsHighSpeedPulseMode(2)

/////////////////////////////////////////////////

#define SPCTL_SPEN_MSK                  BIT6
#define SPI_Enable()                    SET_REG_BIT(SPCTL, SPCTL_SPEN_MSK)
#define SPI_Disable()                   CLR_REG_BIT(SPCTL, SPCTL_SPEN_MSK)

#define SPCTL_DORD_MSK                  BIT5
#define SPI_DataLSB()                   SET_REG_BIT(SPCTL, SPCTL_DORD_MSK)
#define SPI_DataMSB()                   CLR_REG_BIT(SPCTL, SPCTL_DORD_MSK)

#define SPCTL_MSTR_MSK                  BIT4
#define SPI_MasterMode()                SET_REG_BIT(SPCTL, SPCTL_MSTR_MSK)
#define SPI_SlaveMode()                 CLR_REG_BIT(SPCTL, SPCTL_MSTR_MSK)

#define SPCTL_SSIG_MSK                  BIT7
#define SPI_IgnoreSS()                  SET_REG_BIT(SPCTL, SPCTL_SSIG_MSK)
#define SPI_UnignoreSS()                CLR_REG_BIT(SPCTL, SPCTL_SSIG_MSK)

#define SPCTL_CPOL_MSK                  BIT3
#define SPCTL_CPHA_MSK                  BIT2
#define SPIMODE                         (SPCTL_CPOL_MSK | SPCTL_CPHA_MSK)
#define SPI_SetMode0()                  MODIFY_REG(SPCTL, SPIMODE, ((0) << 2))
#define SPI_SetMode1()                  MODIFY_REG(SPCTL, SPIMODE, ((1) << 2))
#define SPI_SetMode2()                  MODIFY_REG(SPCTL, SPIMODE, ((2) << 2))
#define SPI_SetMode3()                  MODIFY_REG(SPCTL, SPIMODE, ((3) << 2))

#define SPCTL_SPR_MSK                   (BIT1 | BIT0)
#define SPI_SetClockDivider(n)          MODIFY_REG(SPCTL, SPCTL_SPR_MSK, ((n) << 0))
#define SPI_SetClockDivider2()          SPI_SetClockDivider(3)
#define SPI_SetClockDivider4()          SPI_SetClockDivider(0)
#define SPI_SetClockDivider8()          SPI_SetClockDivider(1)
#define SPI_SetClockDivider16()         SPI_SetClockDivider(2)

#define SPI_SendData(d)                 (SPDAT = (d))
#define SPI_ReadData()                  (SPDAT)

#define HSSPICFG2_IOSW_MSK              BIT6
#define SPI_SwapMosiMiso()              SET_REG_BIT(HSSPI_CFG2, HSSPICFG2_IOSW_MSK)

#define HSSPICFG2_HSSPIEN_MSK           BIT5
#define HSSPI_Enable()                  SET_REG_BIT(HSSPI_CFG2, HSSPICFG2_HSSPIEN_MSK)
#define HSSPI_Disable()                 CLR_REG_BIT(HSSPI_CFG2, HSSPICFG2_HSSPIEN_MSK)

#define HSSPICFG_SSHLD_MSK              BIT_HN
#define HSSPI_SetSSHoldTime(n)          MODIFY_REG(HSSPI_CFG, HSSPICFG_SSHLD_MSK, ((n) << 4))

#define HSSPICFG_SSSETUP_MSK            BIT_LN
#define HSSPI_SetSSSetupTime(n)         MODIFY_REG(HSSPI_CFG, HSSPICFG_SSSETUP_MSK, ((n) << 0))

#define HSSPICFG2_SSDACT_MSK            BIT_LN
#define HSSPI_SetSSDeactTime(n)         MODIFY_REG(HSSPI_CFG2, HSSPICFG2_SSDACT_MSK, ((n) << 0))

#define HSSPICFG2_FIFOEN_MSK            BIT4
#define HSSPI_EnableFIFO()              SET_REG_BIT(HSSPI_CFG2, HSSPICFG2_FIFOEN_MSK)
#define HSSPI_DisableFIFO()             CLR_REG_BIT(HSSPI_CFG2, HSSPICFG2_FIFOEN_MSK)

#define HSSPI_SetClockDivider(n)        WRITE_REG(HSSPI_PSCR, (n))

#define SPITOCR_ENTO_MSK                BIT7
#define SPI_EnableTimeout()             SET_REG_BIT(SPITOCR, SPITOCR_ENTO_MSK)
#define SPI_DisableTimeout()            CLR_REG_BIT(SPITOCR, SPITOCR_ENTO_MSK)

#define SPITOCR_SCALE_MSK               BIT5
#define SPI_TimeoutScale_SYSCLK()       SET_REG_BIT(SPITOCR, SPITOCR_SCALE_MSK)
#define SPI_TimeoutScale_1us()          CLR_REG_BIT(SPITOCR, SPITOCR_SCALE_MSK)

#define SPI_SetTimeoutInterval(n)       SPITOTL = BYTE0(n); \
                                        SPITOTH = BYTE1(n); \
                                        SPITOTE = BYTE2(n)

/////////////////////////////////////////////////

#define QSPICR1_EN_MSK                  BIT0
#define QSPI_Enable()                   SET_REG_BIT(QSPI_CR1, QSPICR1_EN_MSK)
#define QSPI_Disable()                  CLR_REG_BIT(QSPI_CR1, QSPICR1_EN_MSK)

#define QSPICR1_ABORT_MSK               BIT1
#define QSPI_Abort()                    SET_REG_BIT(QSPI_CR1, QSPICR1_ABORT_MSK)

#define QSPICR1_SSHIFT_MSK              BIT4
#define QSPI_ReadNoDelay()              CLR_REG_BIT(QSPI_CR1, QSPICR1_SSHIFT_MSK)
#define QSPI_ReadDelayHalfCycle()       SET_REG_BIT(QSPI_CR1, QSPICR1_SSHIFT_MSK)

#define QSPICR2_FTHRES_MSK              (BIT4 | BIT_LN)
#define QSPI_SetFIFOLevel(n)            MODIFY_REG(QSPI_CR2, QSPICR2_FTHRES_MSK, ((n) << 0))

#define QSPICR3_PMM_MSK                 BIT7
#define QSPI_PollingMatchAND()          CLR_REG_BIT(QSPI_CR3, QSPICR3_PMM_MSK)
#define QSPI_PollingMatchOR()           SET_REG_BIT(QSPI_CR3, QSPICR3_PMM_MSK)

#define QSPICR3_APMS_MSK                BIT6
#define QSPI_PollingManualStop()        CLR_REG_BIT(QSPI_CR3, QSPICR3_APMS_MSK)
#define QSPI_PollingAutoStop()          SET_REG_BIT(QSPI_CR3, QSPICR3_APMS_MSK)

#define QSPICR3_TOIE_MSK                BIT4
#define QSPICR3_SMIE_MSK                BIT3
#define QSPICR3_FTIE_MSK                BIT2
#define QSPICR3_TCIE_MSK                BIT1
#define QSPICR3_TEIE_MSK                BIT0
#define QSPI_EnableTimeoutInt()         SET_REG_BIT(QSPI_CR3, QSPICR3_TOIE_MSK)
#define QSPI_EnableMatchInt()           SET_REG_BIT(QSPI_CR3, QSPICR3_SMIE_MSK)
#define QSPI_EnableFIFOInt()            SET_REG_BIT(QSPI_CR3, QSPICR3_FTIE_MSK)
#define QSPI_EnableTransferInt()        SET_REG_BIT(QSPI_CR3, QSPICR3_TCIE_MSK)
#define QSPI_EnableErrorInt()           SET_REG_BIT(QSPI_CR3, QSPICR3_TEIE_MSK)

#define QSPI_SetClockDivider(n)         (QSPI_CR4 = (n))

#define QSPIDCR1_CSHT_MSK               (BIT6 | BIT5 | BIT4)
#define QSPI_SetCSHold(n)               MODIFY_REG(QSPI_DCR1, QSPIDCR1_CSHT_MSK, ((n) << 4))

#define QSPIDCR1_CKMODE_MSK             (BIT0)
#define QSPI_SetSCKNormalHigh()         SET_REG_BIT(QSPI_DCR1, QSPIDCR1_CKMODE_MSK)
#define QSPI_SetSCKNormalLow()          CLR_REG_BIT(QSPI_DCR1, QSPIDCR1_CKMODE_MSK)

#define QSPIDCR2_FSIZE_MSK              (BIT4 | BIT_LN)
#define QSPI_SetFlashSize(n)            MODIFY_REG(QSPI_DCR2, QSPIDCR2_FSIZE_MSK, ((n) << 0))

#define QSPISR1_BUSY_MSK                BIT5
#define QSPI_CheckBusy()                READ_REG_BIT(QSPI_SR1, QSPISR1_BUSY_MSK)

#define QSPISR2_FLEVEL_MSK              (BIT5 | BIT4 | BIT_LN)
#define QSPI_CheckFIFOLevel()           READ_REG_BIT(QSPI_SR2, QSPISR2_FLEVEL_MSK)

/*
#define QSPI_SetDataLength(n)           QSPI_DLR4 = BYTE3(n);       \
                                        QSPI_DLR3 = BYTE2(n);       \
                                        QSPI_DLR2 = BYTE1(n);       \
                                        QSPI_DLR1 = BYTE0(n)
*/

#define QSPI_SetDataLength(n)           QSPI_DLR2 = BYTE1(n);       \
                                        QSPI_DLR1 = BYTE0(n)

#define QSPI_SetAddress(n)              QSPI_AR4 = BYTE3(n);        \
                                        QSPI_AR3 = BYTE2(n);        \
                                        QSPI_AR2 = BYTE1(n);        \
                                        QSPI_AR1 = BYTE0(n)

/*
#define QSPI_SetAlternate(n)            QSPI_ABR4 = BYTE3(n);       \           
                                        QSPI_ABR3 = BYTE2(n);       \           
                                        QSPI_ABR2 = BYTE1(n);       \          
                                        QSPI_ABR1 = BYTE0(n)               
*/

#define QSPI_SetAlternate(n)            QSPI_ABR = BYTE0(n)               

#define QSPI_SetInstruction(n)          (QSPI_CCR1 = (n))

#define QSPICCR2_ADSIZE_MSK             (BIT5 | BIT4)
#define QSPI_SetAddressSize(n)          MODIFY_REG(QSPI_CCR2, QSPICCR2_ADSIZE_MSK, ((n) << 4))
#define QSPI_SetAddressSize_8B()        QSPI_SetAddressSize(0)
#define QSPI_SetAddressSize_16B()       QSPI_SetAddressSize(1)
#define QSPI_SetAddressSize_24B()       QSPI_SetAddressSize(2)
#define QSPI_SetAddressSize_32B()       QSPI_SetAddressSize(3)

#define QSPICCR3_ABSIZE_MSK             (BIT1 | BIT0)
#define QSPI_SetAlternateSize(n)        MODIFY_REG(QSPI_CCR3, QSPICCR3_ABSIZE_MSK, ((n) << 0))
#define QSPI_SetAlternateSize_8B()      QSPI_SetAlternateSize(0)
#define QSPI_SetAlternateSize_16B()     QSPI_SetAlternateSize(1)
#define QSPI_SetAlternateSize_24B()     QSPI_SetAlternateSize(2)
#define QSPI_SetAlternateSize_32B()     QSPI_SetAlternateSize(3)

#define QSPICCR3_DCYC_MSK              (BIT6 | BIT5 | BIT4 | BIT3 | BIT2)
#define QSPI_SetDummyCycles(n)          MODIFY_REG(QSPI_CCR3, QSPICCR3_DCYC_MSK, ((n) << 2))

#define QSPICCR2_IMODE_MSK              (BIT1 | BIT0)
#define QSPI_SetInstructionMode(n)      MODIFY_REG(QSPI_CCR2, QSPICCR2_IMODE_MSK, ((n) << 0))
#define QSPI_NoInstruction()            QSPI_SetInstructionMode(0)
#define QSPI_InstructionSingMode()      QSPI_SetInstructionMode(1)
#define QSPI_InstructionDualMode()      QSPI_SetInstructionMode(2)
#define QSPI_InstructionQuadMode()      QSPI_SetInstructionMode(3)

#define QSPICCR2_ADMODE_MSK             (BIT3 | BIT2)
#define QSPI_SetAddressMode(n)          MODIFY_REG(QSPI_CCR2, QSPICCR2_ADMODE_MSK, ((n) << 2))
#define QSPI_NoAddress()                QSPI_SetAddressMode(0)
#define QSPI_AddressSingMode()          QSPI_SetAddressMode(1)
#define QSPI_AddressDualMode()          QSPI_SetAddressMode(2)
#define QSPI_AddressQuadMode()          QSPI_SetAddressMode(3)

#define QSPICCR2_ABMODE_MSK             (BIT7 | BIT6)
#define QSPI_SetAlternateMode(n)        MODIFY_REG(QSPI_CCR2, QSPICCR2_ABMODE_MSK, ((n) << 6))
#define QSPI_NoAlternate()              QSPI_SetAlternateMode(0)
#define QSPI_AlternateSingMode()        QSPI_SetAlternateMode(1)
#define QSPI_AlternateDualMode()        QSPI_SetAlternateMode(2)
#define QSPI_AlternateQuadMode()        QSPI_SetAlternateMode(3)

#define QSPICCR4_DMODE_MSK              (BIT1 | BIT0)
#define QSPI_SetDataMode(n)             MODIFY_REG(QSPI_CCR4, QSPICCR4_DMODE_MSK, ((n) << 0))
#define QSPI_NoData()                   QSPI_SetDataMode(0)
#define QSPI_DataSingMode()             QSPI_SetDataMode(1)
#define QSPI_DataDualMode()             QSPI_SetDataMode(2)
#define QSPI_DataQuadMode()             QSPI_SetDataMode(3)

#define QSPICCR4_FMODE_MSK              (BIT3 | BIT2)
#define QSPI_SetFunctionMode(n)         MODIFY_REG(QSPI_CCR4, QSPICCR4_FMODE_MSK, ((n) << 2))
#define QSPI_SetWriteMode()             QSPI_SetFunctionMode(0)
#define QSPI_SetReadMode()              QSPI_SetFunctionMode(1)
#define QSPI_SetPollingMode()           QSPI_SetFunctionMode(2)

#define QSPICCR4_SIOO_MSK               BIT4
#define QSPI_InstructionOnce()          SET_REG_BIT(QSPI_CCR4, QSPICCR4_SIOO_MSK)
#define QSPI_InstructionAlways()        CLR_REG_BIT(QSPI_CCR4, QSPICCR4_SIOO_MSK)

#define QSPI_ReadData()                 (ACC = QSPI_DR)
#define QSPI_WriteData(d)               (QSPI_DR = (d))

#define QSPI_SetPollingMask(n)          /* QSPI_PSMKR4 = BYTE3(n); */   \
                                        /* QSPI_PSMKR3 = BYTE2(n); */   \
                                        /* QSPI_PSMKR2 = BYTE1(n);  */  \
                                        QSPI_PSMKR1 = BYTE0(n)

#define QSPI_SetPollingMatch(n)         /* QSPI_PSMAR4 = BYTE3(n); */   \
                                        /* QSPI_PSMAR3 = BYTE2(n); */   \
                                        /* QSPI_PSMAR2 = BYTE1(n);  */  \
                                        QSPI_PSMAR1 = BYTE0(n)

#define QSPI_SetPollingInterval(n)      QSPI_PIR2 = BYTE1(n);           \
                                        QSPI_PIR1 = BYTE0(n)

/////////////////////////////////////////////////

#define I2CCFG_ENI2C_MSK                BIT7
#define I2C_Enable()                    SET_REG_BIT(I2CCFG, I2CCFG_ENI2C_MSK)
#define I2C_Disable()                   CLR_REG_BIT(I2CCFG, I2CCFG_ENI2C_MSK)

#define I2CCFG_MSSL_MSK                 BIT6
#define I2C_MasterMode()                SET_REG_BIT(I2CCFG, I2CCFG_MSSL_MSK)
#define I2C_SlaveMode()                 CLR_REG_BIT(I2CCFG, I2CCFG_MSSL_MSK)

#define I2CCFG_SPEED_MSK                (BIT5 | BIT4 |BIT_LN)
#define I2C_SetClockDivider(n)          MODIFY_REG(I2CCFG, I2CCFG_SPEED_MSK, ((n) << 0)); \
                                        I2CPSCR = ((n) >> 6)

#define I2CMSCR_MSCMD_MSK               (BIT_LN)
#define I2C_SetMasterCommand(n)         MODIFY_REG(I2CMSCR, I2CMSCR_MSCMD_MSK, ((n) << 0))

#define I2CCMD_IDLE                     0
#define I2CCMD_START                    1
#define I2CCMD_SENDDATA                 2
#define I2CCMD_RECVACK                  3
#define I2CCMD_RECVDATA                 4
#define I2CCMD_SENDACK                  5
#define I2CCMD_STOP                     6
#define I2CCMD_START_SENDDATA_RECVACK   9
#define I2CCMD_SENDDATA_RECVACK         10
#define I2CCMD_RECVDATA_SENDACK         11
#define I2CCMD_RECVDATA_SENDNAK         12
#define I2C_Idle()                      I2C_SetMasterCommand(I2CCMD_IDLE)
#define I2C_Start()                     I2C_SetMasterCommand(I2CCMD_START)
#define I2C_SendData()                  I2C_SetMasterCommand(I2CCMD_SENDDATA)
#define I2C_RecvACK()                   I2C_SetMasterCommand(I2CCMD_RECVACK)
#define I2C_RecvData()                  I2C_SetMasterCommand(I2CCMD_RECVDATA)
#define I2C_SendACK()                   I2C_SetMasterCommand(I2CCMD_SENDACK)
#define I2C_Stop()                      I2C_SetMasterCommand(I2CCMD_STOP)
#define I2C_StartSendDataRecvACK()      I2C_SetMasterCommand(I2CCMD_START_SENDDATA_RECVACK)
#define I2C_SendDataRecvACK()           I2C_SetMasterCommand(I2CCMD_SENDDATA_RECVACK)
#define I2C_RecvDataSendACK()           I2C_SetMasterCommand(I2CCMD_RECVDATA_SENDACK)
#define I2C_RecvDataSendNAK()           I2C_SetMasterCommand(I2CCMD_RECVDATA_SENDNAK)

#define I2CMSST_BUSY_MSK                BIT7
#define I2C_CheckMasterBusy()           READ_REG_BIT(I2CMSST, I2CMSST_BUSY_MSK)

#define I2CMSST_MSIF_MSK                BIT6
#define I2C_CheckMasterFlag()           READ_REG_BIT(I2CMSST, I2CMSST_MSIF_MSK)
#define I2C_ClearMasterFlag()           CLR_REG_BIT(I2CMSST, I2CMSST_MSIF_MSK)

#define I2C_WriteData(d)                (I2CTXD = (d))
#define I2C_ReadData()                  (I2CRXD)

#define I2CMSST_ACKI_MSK                BIT1
#define I2CMSST_ACKO_MSK                BIT0
#define I2C_MasterSetACK()              CLR_REG_BIT(I2CMSST, I2CMSST_ACKO_MSK)
#define I2C_MasterSetNAK()              SET_REG_BIT(I2CMSST, I2CMSST_ACKO_MSK)
#define I2C_MasterReadACK()             READ_REG_BIT(I2CMSST, I2CMSST_ACKI_MSK)

#define I2CSLCR_SLRST_MSK               BIT0
#define I2C_SlaveReset()                SET_REG_BIT(I2CSLCR, I2CSLCR_SLRST_MSK)

#define I2CSLST_BUSY_MSK                BIT7
#define I2C_CheckSlaveBusy()            READ_REG_BIT(I2CSLST, I2CSLST_BUSY_MSK)

#define I2CSLST_ACKI_MSK                BIT1
#define I2CSLST_ACKO_MSK                BIT0
#define I2C_SlaveSetACK()               CLR_REG_BIT(I2CSLST, I2CSLST_ACKO_MSK)
#define I2C_SlaveSetNAK()               SET_REG_BIT(I2CSLST, I2CSLST_ACKO_MSK)
#define I2C_SlaveReadACK()              READ_REG_BIT(I2CSLST, I2CSLST_ACKI_MSK)

#define I2CSLADR_SLADR_MSK              (BIT_HN | BIT3 | BIT2 | BIT1)
#define I2C_SetSlaveAddress(n)          I2CSLADR = ((n) << 1)

#define I2CSLADR_MA_MSK                 (BIT0)
#define I2C_SetSlaveBroadcast()         SET_REG_BIT(I2CSLADR, I2CSLADR_MA_MSK)

#define I2CTOCR_ENTO_MSK                BIT7
#define I2C_EnableTimeout()             SET_REG_BIT(I2CTOCR, I2CTOCR_ENTO_MSK)
#define I2C_DisableTimeout()            CLR_REG_BIT(I2CTOCR, I2CTOCR_ENTO_MSK)

#define I2CTOCR_SCALE_MSK               BIT5
#define I2C_TimeoutScale_SYSCLK()       SET_REG_BIT(I2CTOCR, I2CTOCR_SCALE_MSK)
#define I2C_TimeoutScale_1us()          CLR_REG_BIT(I2CTOCR, I2CTOCR_SCALE_MSK)

#define I2C_SetTimeoutInterval(n)       I2CTOTL = BYTE0(n); \
                                        I2CTOTH = BYTE1(n); \
                                        I2CTOTE = BYTE2(n)

/////////////////////////////////////////////////

#define RTCCR_RUNRTC_MSK                BIT0
#define RTC_Run()                       SET_REG_BIT(RTCCR, RTCCR_RUNRTC_MSK)
#define RTC_Stop()                      CLR_REG_BIT(RTCCR, RTCCR_RUNRTC_MSK)

#define RTCCFG_RTCCKS_MSK               BIT1
#define RTC_CLK_X32K()                  CLR_REG_BIT(RTCCFG, RTCCFG_RTCCKS_MSK)
#define RTC_CLK_LIRC()                  SET_REG_BIT(RTCCFG, RTCCFG_RTCCKS_MSK)

#define RTCCFG_SETRTC_MSK               BIT0
#define RTC_SyncInitial()               SET_REG_BIT(RTCCFG, RTCCFG_SETRTC_MSK)
#define RTC_IsSyncing()                 READ_REG_BIT(RTCCFG, RTCCFG_SETRTC_MSK)

#define RTC_SetAlarmHour(n)             (ALAHOUR = (n))
#define RTC_SetAlarmMinute(n)           (ALAMIN = (n))
#define RTC_SetAlarmSecond(n)           (ALASEC = (n))
#define RTC_SetAlarmSSecond(n)          (ALASSEC = (n))

#define RTC_SetYear(n)                  (INIYEAR = (n))
#define RTC_SetMonth(n)                 (INIMONTH = (n))
#define RTC_SetDay(n)                   (INIDAY = (n))
#define RTC_SetHour(n)                  (INIHOUR = (n))
#define RTC_SetMinute(n)                (INIMIN = (n))
#define RTC_SetSecond(n)                (INISEC = (n))
#define RTC_SetSSecond(n)               (INISSEC = (n))

#define RTC_ReadYear()                  (RTCYEAR)
#define RTC_ReadMonth()                 (RTCMONTH)
#define RTC_ReadDay()                   (RTCDAY)
#define RTC_ReadHour()                  (RTCHOUR)
#define RTC_ReadMinute()                (RTCMIN)
#define RTC_ReadSecond()                (RTCSEC)
#define RTC_ReadSSecond()               (RTCSSEC)

/////////////////////////////////////////////////

#define LCMIFCR_EN_MSK                  BIT7
#define LCM_Enable()                    SET_REG_BIT(LCMIFCR, LCMIFCR_EN_MSK)
#define LCM_Disable()                   CLR_REG_BIT(LCMIFCR, LCMIFCR_EN_MSK)

#define LCMIFCFG_BW_MSK                 BIT1
#define LCM_SetBitWidth_8B()            CLR_REG_BIT(LCMIFCFG, LCMIFCFG_BW_MSK)
#define LCM_SetBitWidth_16B()           SET_REG_BIT(LCMIFCFG, LCMIFCFG_BW_MSK)

#define LCMIFCFG_MODE_MSK               BIT0
#define LCM_SetMode_i8080()             CLR_REG_BIT(LCMIFCFG, LCMIFCFG_MODE_MSK)
#define LCM_SetMode_M6800()             SET_REG_BIT(LCMIFCFG, LCMIFCFG_MODE_MSK)

#define LCMIFCFG2_SETUPT_MSK            (BIT4 | BIT3 | BIT2)
#define LCMIFCFG2_HOLDT_MSK             (BIT1 | BIT0)
#define LCM_SetSetupTime(n)             MODIFY_REG(LCMIFCFG2, LCMIFCFG2_SETUPT_MSK, ((n) << 2))
#define LCM_SetHoldTime(n)              MODIFY_REG(LCMIFCFG2, LCMIFCFG2_HOLDT_MSK, ((n) << 0))

#define LCMIFCR_ENDIAN_MSK              BIT4
#define LCM_SetDataBigEndian()          CLR_REG_BIT(LCMIFCR, LCMIFCR_ENDIAN_MSK)
#define LCM_SetDataLittleEndian()       SET_REG_BIT(LCMIFCR, LCMIFCR_ENDIAN_MSK)

#define LCMIFCR_CMD_MSK                 (BIT2 | BIT1 | BIT0)
#define LCM_SetCommand(n)               MODIFY_REG(LCMIFCR, LCMIFCR_CMD_MSK, ((n) << 0))

#define LCMCMD_IDLE                     0
#define LCMCMD_SENDCMD                  4
#define LCMCMD_SENDDATA                 5
#define LCMCMD_READSTAT                 6
#define LCMCMD_READDATA                 7
#define LCM_Idle()                      LCM_SetCommand(LCMCMD_IDLE)
#define LCM_TrigSendCommand()           LCM_SetCommand(LCMCMD_SENDCMD)
#define LCM_TrigSendData()              LCM_SetCommand(LCMCMD_SENDDATA)
#define LCM_TrigReadStatus()            LCM_SetCommand(LCMCMD_READSTAT)
#define LCM_TrigReadData()              LCM_SetCommand(LCMCMD_READDATA)

#define LCM_WriteData_8B(d)             WRITE_REG(LCMIFDATL, (d))
#define LCM_ReadData_8B()               (LCMIFDATL)

#define LCM_WriteData_16B(d)            (LCMIFDATH = HIBYTE(d), LCMIFDATL = LOBYTE(d))
#define LCM_ReadData_16B()              MAKEWORD(LCMIFDATL, LCMIFDATH)

#define LCM_SetClockDivider(n)          WRITE_REG(LCMIFPSCR, (n))

/////////////////////////////////////////////////

#define I2SCR_FRF_MSK                   BIT4
#define I2S_SetMOTOROLAFrame()          CLR_REG_BIT(I2SCR, I2SCR_FRF_MSK)
#define I2S_SetTIFrame()                SET_REG_BIT(I2SCR, I2SCR_FRF_MSK)

#define I2SCR_TXDMAEN_MSK               BIT1
#define I2SCR_RXDMAEN_MSK               BIT0
#define I2S_EnableTxDMA()               SET_REG_BIT(I2SCR, I2SCR_TXDMAEN_MSK)
#define I2S_DisableTxDMA()              CLR_REG_BIT(I2SCR, I2SCR_TXDMAEN_MSK)
#define I2S_EnableRxDMA()               SET_REG_BIT(I2SCR, I2SCR_RXDMAEN_MSK)
#define I2S_DisableRxDMA()              CLR_REG_BIT(I2SCR, I2SCR_RXDMAEN_MSK)

#define I2SSR_FRE_MSK                   BIT6
#define I2SSR_BUY_MSK                   BIT5
#define I2SSR_OVR_MSK                   BIT4
#define I2SSR_UDR_MSK                   BIT3
#define I2SSR_CHSID_MSK                 BIT2
#define I2S_CheckFREFlag()              READ_REG_BIT(I2SSR, I2SSR_FRE_MSK)
#define I2S_CheckBUYFlag()              READ_REG_BIT(I2SSR, I2SSR_BUY_MSK)
#define I2S_CheckOVRFlag()              READ_REG_BIT(I2SSR, I2SSR_OVR_MSK)
#define I2S_CheckUDRFlag()              READ_REG_BIT(I2SSR, I2SSR_UDR_MSK)
#define I2S_CheckCHSIDFlag()            READ_REG_BIT(I2SSR, I2SSR_CHSID_MSK)

#define I2S_WriteData_16B(d)            (I2SDRH = HIBYTE(d), I2SDRL = LOBYTE(d))
#define I2S_ReadData_16B()              MAKEWORD(I2SDRL, I2SDRH)

#define I2SPRH_MCKOE_MSK                BIT1
#define I2S_EnableMCKOutput()           SET_REG_BIT(I2SPRH, I2SPRH_MCKOE_MSK)
#define I2S_DisableMCKOutput()          CLR_REG_BIT(I2SPRH, I2SPRH_MCKOE_MSK)

#define I2SPRH_ODD_MSK                  BIT0
#define I2S_SetClockDivider(n)          MODIFY_REG(I2SPRH, I2SPRH_ODD_MSK, ((n) << 0)); \
                                        I2SPRL = ((n) >> 1)

#define I2SCFGH_I2SE_MSK                BIT2
#define I2S_Enable()                    SET_REG_BIT(I2SCFGH, I2SCFGH_I2SE_MSK)
#define I2S_Disable()                   CLR_REG_BIT(I2SCFGH, I2SCFGH_I2SE_MSK)

#define I2SCFGH_MODE_MSK                (BIT1 | BIT0)
#define I2S_SetMode(n)                  MODIFY_REG(I2SCFGH, I2SCFGH_MODE_MSK, ((n) << 0))
#define I2S_SlaveSendMode()             I2S_SetMode(0)
#define I2S_SlaveReceiveMode()          I2S_SetMode(1)
#define I2S_MasterSendMode()            I2S_SetMode(2)
#define I2S_MasterReceiveMode()         I2S_SetMode(3)

#define I2SCFGL_PCMSYNC_MSK             BIT7
#define I2S_ShortFrameSync()            CLR_REG_BIT(I2SCFGL, I2SCFGL_PCMSYNC_MSK)
#define I2S_LongFrameSync()             SET_REG_BIT(I2SCFGL, I2SCFGL_PCMSYNC_MSK)

#define I2SCFGL_STD_MSK                 (BIT5 | BIT4)
#define I2S_SetStandard(n)              MODIFY_REG(I2SCFGL, I2SCFGL_STD_MSK, ((n) << 4))
#define I2S_PhilipsStandard()           I2S_SetStandard(0)
#define I2S_LeftAlignStandard()         I2S_SetStandard(1)
#define I2S_RightAlignStandard()        I2S_SetStandard(2)
#define I2S_PCMStandard()               I2S_SetStandard(3)

#define I2SCFGL_CKPOL_MSK               BIT3
#define I2S_ClkNormalLow()              CLR_REG_BIT(I2SCFGL, I2SCFGL_CKPOL_MSK)
#define I2S_ClkNormalHigh()             SET_REG_BIT(I2SCFGL, I2SCFGL_CKPOL_MSK)

#define I2SCFGL_DATLEN_MSK              (BIT2 | BIT1)
#define I2S_SetDataLength(n)            MODIFY_REG(I2SCFGL, I2SCFGL_DATLEN_MSK, ((n) << 1))
#define I2S_SetData16Bit()              I2S_SetDataLength(0)
#define I2S_SetData24Bit()              I2S_SetDataLength(1)
#define I2S_SetData32Bit()              I2S_SetDataLength(2)

#define I2SCFGL_CHLEN_MSK               BIT0
#define I2S_SetChannel16Bit()           CLR_REG_BIT(I2SCFGL, I2SCFGL_CHLEN_MSK)
#define I2S_SetChannel32Bit()           SET_REG_BIT(I2SCFGL, I2SCFGL_CHLEN_MSK)

#define I2S_SetMCKDivider(n)            WRITE_REG(I2SMCKDIV, (n))

/////////////////////////////////////////////////

#define TFPU_CLK_SYSCLK()               WRITE_REG(DMAIR, (0x3e))
#define TFPU_CLK_HSIOCK()               WRITE_REG(DMAIR, (0x3f))

/////////////////////////////////////////////////

#define DMAUR1TCFG_UR1TPTY_MSK          (BIT1 | BIT0)
#define DMAUR2TCFG_UR2TPTY_MSK          (BIT1 | BIT0)
#define DMAUR3TCFG_UR3TPTY_MSK          (BIT1 | BIT0)
#define DMAUR4TCFG_UR4TPTY_MSK          (BIT1 | BIT0)
#define DMA_UART1_SetTxBusPriority(n)   MODIFY_REG(DMA_UR1T_CFG,  DMAUR1TCFG_UR1TPTY_MSK, ((n) << 0))
#define DMA_UART2_SetTxBusPriority(n)   MODIFY_REG(DMA_UR2T_CFG,  DMAUR2TCFG_UR2TPTY_MSK, ((n) << 0))
#define DMA_UART3_SetTxBusPriority(n)   MODIFY_REG(DMA_UR3T_CFG,  DMAUR3TCFG_UR3TPTY_MSK, ((n) << 0))
#define DMA_UART4_SetTxBusPriority(n)   MODIFY_REG(DMA_UR4T_CFG,  DMAUR4TCFG_UR4TPTY_MSK, ((n) << 0))

#define DMAUR1RCFG_UR1RPTY_MSK          (BIT1 | BIT0)
#define DMAUR2RCFG_UR2RPTY_MSK          (BIT1 | BIT0)
#define DMAUR3RCFG_UR3RPTY_MSK          (BIT1 | BIT0)
#define DMAUR4RCFG_UR4RPTY_MSK          (BIT1 | BIT0)
#define DMA_UART1_SetRxBusPriority(n)   MODIFY_REG(DMA_UR1R_CFG,  DMAUR1RCFG_UR1RPTY_MSK, ((n) << 0))
#define DMA_UART2_SetRxBusPriority(n)   MODIFY_REG(DMA_UR2R_CFG,  DMAUR2RCFG_UR2RPTY_MSK, ((n) << 0))
#define DMA_UART3_SetRxBusPriority(n)   MODIFY_REG(DMA_UR3R_CFG,  DMAUR3RCFG_UR3RPTY_MSK, ((n) << 0))
#define DMA_UART4_SetRxBusPriority(n)   MODIFY_REG(DMA_UR4R_CFG,  DMAUR4RCFG_UR4RPTY_MSK, ((n) << 0))

#define DMAUR1TCR_ENUR1T_MSK            BIT7
#define DMAUR2TCR_ENUR2T_MSK            BIT7
#define DMAUR3TCR_ENUR3T_MSK            BIT7
#define DMAUR4TCR_ENUR4T_MSK            BIT7
#define DMA_UART1_EnableTx()            SET_REG_BIT(DMA_UR1T_CR, DMAUR1TCR_ENUR1T_MSK)
#define DMA_UART2_EnableTx()            SET_REG_BIT(DMA_UR2T_CR, DMAUR2TCR_ENUR2T_MSK)
#define DMA_UART3_EnableTx()            SET_REG_BIT(DMA_UR3T_CR, DMAUR3TCR_ENUR3T_MSK)
#define DMA_UART4_EnableTx()            SET_REG_BIT(DMA_UR4T_CR, DMAUR4TCR_ENUR4T_MSK)
#define DMA_UART1_DisableTx()           CLR_REG_BIT(DMA_UR1T_CR, DMAUR1TCR_ENUR1T_MSK)
#define DMA_UART2_DisableTx()           CLR_REG_BIT(DMA_UR2T_CR, DMAUR2TCR_ENUR2T_MSK)
#define DMA_UART3_DisableTx()           CLR_REG_BIT(DMA_UR3T_CR, DMAUR3TCR_ENUR3T_MSK)
#define DMA_UART4_DisableTx()           CLR_REG_BIT(DMA_UR4T_CR, DMAUR4TCR_ENUR4T_MSK)

#define DMAUR1RCR_ENUR1R_MSK            BIT7
#define DMAUR2RCR_ENUR2R_MSK            BIT7
#define DMAUR3RCR_ENUR3R_MSK            BIT7
#define DMAUR4RCR_ENUR4R_MSK            BIT7
#define DMA_UART1_EnableRx()            SET_REG_BIT(DMA_UR1R_CR, DMAUR1RCR_ENUR1R_MSK)
#define DMA_UART2_EnableRx()            SET_REG_BIT(DMA_UR2R_CR, DMAUR2RCR_ENUR2R_MSK)
#define DMA_UART3_EnableRx()            SET_REG_BIT(DMA_UR3R_CR, DMAUR3RCR_ENUR3R_MSK)
#define DMA_UART4_EnableRx()            SET_REG_BIT(DMA_UR4R_CR, DMAUR4RCR_ENUR4R_MSK)
#define DMA_UART1_DisableRx()           CLR_REG_BIT(DMA_UR1R_CR, DMAUR1RCR_ENUR1R_MSK)
#define DMA_UART2_DisableRx()           CLR_REG_BIT(DMA_UR2R_CR, DMAUR2RCR_ENUR2R_MSK)
#define DMA_UART3_DisableRx()           CLR_REG_BIT(DMA_UR3R_CR, DMAUR3RCR_ENUR3R_MSK)
#define DMA_UART4_DisableRx()           CLR_REG_BIT(DMA_UR4R_CR, DMAUR4RCR_ENUR4R_MSK)

#define DMAUR1TCR_TRIG_MSK              BIT6
#define DMAUR2TCR_TRIG_MSK              BIT6
#define DMAUR3TCR_TRIG_MSK              BIT6
#define DMAUR4TCR_TRIG_MSK              BIT6
#define DMA_UART1_TriggerTx()           SET_REG_BIT(DMA_UR1T_CR, DMAUR1TCR_TRIG_MSK)
#define DMA_UART2_TriggerTx()           SET_REG_BIT(DMA_UR2T_CR, DMAUR2TCR_TRIG_MSK)
#define DMA_UART3_TriggerTx()           SET_REG_BIT(DMA_UR3T_CR, DMAUR3TCR_TRIG_MSK)
#define DMA_UART4_TriggerTx()           SET_REG_BIT(DMA_UR4T_CR, DMAUR4TCR_TRIG_MSK)

#define DMAUR1RCR_TRIG_MSK              BIT5
#define DMAUR2RCR_TRIG_MSK              BIT5
#define DMAUR3RCR_TRIG_MSK              BIT5
#define DMAUR4RCR_TRIG_MSK              BIT5
#define DMA_UART1_TriggerRx()           SET_REG_BIT(DMA_UR1R_CR, DMAUR1RCR_TRIG_MSK)
#define DMA_UART2_TriggerRx()           SET_REG_BIT(DMA_UR2R_CR, DMAUR2RCR_TRIG_MSK)
#define DMA_UART3_TriggerRx()           SET_REG_BIT(DMA_UR3R_CR, DMAUR3RCR_TRIG_MSK)
#define DMA_UART4_TriggerRx()           SET_REG_BIT(DMA_UR4R_CR, DMAUR4RCR_TRIG_MSK)

#define DMAUR1RCR_CLRFIFO_MSK           BIT0
#define DMAUR2RCR_CLRFIFO_MSK           BIT0
#define DMAUR3RCR_CLRFIFO_MSK           BIT0
#define DMAUR4RCR_CLRFIFO_MSK           BIT0
#define DMA_UART1_ClearFIFO()           SET_REG_BIT(DMA_UR1R_CR, DMAUR1RCR_CLRFIFO_MSK)
#define DMA_UART2_ClearFIFO()           SET_REG_BIT(DMA_UR2R_CR, DMAUR2RCR_CLRFIFO_MSK)
#define DMA_UART3_ClearFIFO()           SET_REG_BIT(DMA_UR3R_CR, DMAUR3RCR_CLRFIFO_MSK)
#define DMA_UART4_ClearFIFO()           SET_REG_BIT(DMA_UR4R_CR, DMAUR4RCR_CLRFIFO_MSK)

#define DMAUR1TSTA_TXOVW_MSK            BIT2
#define DMAUR2TSTA_TXOVW_MSK            BIT2
#define DMAUR3TSTA_TXOVW_MSK            BIT2
#define DMAUR4TSTA_TXOVW_MSK            BIT2
#define DMA_UART1_CheckOverWriteFlag()  READ_REG_BIT(DMA_UR1T_STA,  DMAUR1TSTA_TXOVW_MSK)
#define DMA_UART2_CheckOverWriteFlag()  READ_REG_BIT(DMA_UR2T_STA,  DMAUR2TSTA_TXOVW_MSK)
#define DMA_UART3_CheckOverWriteFlag()  READ_REG_BIT(DMA_UR3T_STA,  DMAUR3TSTA_TXOVW_MSK)
#define DMA_UART4_CheckOverWriteFlag()  READ_REG_BIT(DMA_UR4T_STA,  DMAUR4TSTA_TXOVW_MSK)
#define DMA_UART1_ClearOverWriteFlag()  CLR_REG_BIT(DMA_UR1T_STA,   DMAUR1TSTA_TXOVW_MSK)
#define DMA_UART2_ClearOverWriteFlag()  CLR_REG_BIT(DMA_UR2T_STA,   DMAUR2TSTA_TXOVW_MSK)
#define DMA_UART3_ClearOverWriteFlag()  CLR_REG_BIT(DMA_UR3T_STA,   DMAUR3TSTA_TXOVW_MSK)
#define DMA_UART4_ClearOverWriteFlag()  CLR_REG_BIT(DMA_UR4T_STA,   DMAUR4TSTA_TXOVW_MSK)

#define DMAUR1RSTA_RXLOSS_MSK           BIT1
#define DMAUR2RSTA_RXLOSS_MSK           BIT1
#define DMAUR3RSTA_RXLOSS_MSK           BIT1
#define DMAUR4RSTA_RXLOSS_MSK           BIT1
#define DMA_UART1_CheckRxLossFlag()     READ_REG_BIT(DMA_UR1R_STA,  DMAUR1RSTA_RXLOSS_MSK)
#define DMA_UART2_CheckRxLossFlag()     READ_REG_BIT(DMA_UR2R_STA,  DMAUR2RSTA_RXLOSS_MSK)
#define DMA_UART3_CheckRxLossFlag()     READ_REG_BIT(DMA_UR3R_STA,  DMAUR3RSTA_RXLOSS_MSK)
#define DMA_UART4_CheckRxLossFlag()     READ_REG_BIT(DMA_UR4R_STA,  DMAUR4RSTA_RXLOSS_MSK)
#define DMA_UART1_ClearRxLossFlag()     CLR_REG_BIT(DMA_UR1R_STA,   DMAUR1RSTA_RXLOSS_MSK)
#define DMA_UART2_ClearRxLossFlag()     CLR_REG_BIT(DMA_UR2R_STA,   DMAUR2RSTA_RXLOSS_MSK)
#define DMA_UART3_ClearRxLossFlag()     CLR_REG_BIT(DMA_UR3R_STA,   DMAUR3RSTA_RXLOSS_MSK)
#define DMA_UART4_ClearRxLossFlag()     CLR_REG_BIT(DMA_UR4R_STA,   DMAUR4RSTA_RXLOSS_MSK)

#define DMA_UART1_SetTxAmount(d)        (DMA_UR1T_AMTH = HIBYTE(d), DMA_UR1T_AMT = LOBYTE(d))
#define DMA_UART2_SetTxAmount(d)        (DMA_UR2T_AMTH = HIBYTE(d), DMA_UR2T_AMT = LOBYTE(d))
#define DMA_UART3_SetTxAmount(d)        (DMA_UR3T_AMTH = HIBYTE(d), DMA_UR3T_AMT = LOBYTE(d))
#define DMA_UART4_SetTxAmount(d)        (DMA_UR4T_AMTH = HIBYTE(d), DMA_UR4T_AMT = LOBYTE(d))

#define DMA_UART1_SetRxAmount(d)        (DMA_UR1R_AMTH = HIBYTE(d), DMA_UR1R_AMT = LOBYTE(d))
#define DMA_UART2_SetRxAmount(d)        (DMA_UR2R_AMTH = HIBYTE(d), DMA_UR2R_AMT = LOBYTE(d))
#define DMA_UART3_SetRxAmount(d)        (DMA_UR3R_AMTH = HIBYTE(d), DMA_UR3R_AMT = LOBYTE(d))
#define DMA_UART4_SetRxAmount(d)        (DMA_UR4R_AMTH = HIBYTE(d), DMA_UR4R_AMT = LOBYTE(d))

#define DMA_UART1_ReadTxDone()          MAKEWORD(DMA_UR1T_DONE, DMA_UR1T_DONEH)
#define DMA_UART2_ReadTxDone()          MAKEWORD(DMA_UR2T_DONE, DMA_UR2T_DONEH)
#define DMA_UART3_ReadTxDone()          MAKEWORD(DMA_UR3T_DONE, DMA_UR3T_DONEH)
#define DMA_UART4_ReadTxDone()          MAKEWORD(DMA_UR4T_DONE, DMA_UR4T_DONEH)
                               
#define DMA_UART1_ReadRxDone()          MAKEWORD(DMA_UR1R_DONE, DMA_UR1R_DONEH)
#define DMA_UART2_ReadRxDone()          MAKEWORD(DMA_UR2R_DONE, DMA_UR2R_DONEH)
#define DMA_UART3_ReadRxDone()          MAKEWORD(DMA_UR3R_DONE, DMA_UR3R_DONEH)
#define DMA_UART4_ReadRxDone()          MAKEWORD(DMA_UR4R_DONE, DMA_UR4R_DONEH)

#define DMA_UART1_SetTxAddress(d)       (DMA_UR1T_TXAH = HIBYTE(d), DMA_UR1T_TXAL = LOBYTE(d))
#define DMA_UART2_SetTxAddress(d)       (DMA_UR2T_TXAH = HIBYTE(d), DMA_UR2T_TXAL = LOBYTE(d))
#define DMA_UART3_SetTxAddress(d)       (DMA_UR3T_TXAH = HIBYTE(d), DMA_UR3T_TXAL = LOBYTE(d))
#define DMA_UART4_SetTxAddress(d)       (DMA_UR4T_TXAH = HIBYTE(d), DMA_UR4T_TXAL = LOBYTE(d))

#define DMA_UART1_SetRxAddress(d)       (DMA_UR1R_RXAH = HIBYTE(d), DMA_UR1R_RXAL = LOBYTE(d))
#define DMA_UART2_SetRxAddress(d)       (DMA_UR2R_RXAH = HIBYTE(d), DMA_UR2R_RXAL = LOBYTE(d))
#define DMA_UART3_SetRxAddress(d)       (DMA_UR3R_RXAH = HIBYTE(d), DMA_UR3R_RXAL = LOBYTE(d))
#define DMA_UART4_SetRxAddress(d)       (DMA_UR4R_RXAH = HIBYTE(d), DMA_UR4R_RXAL = LOBYTE(d))

#define DMA_UART1_SetInterval(d)        (DMA_UR1_ITVH = HIBYTE(d), DMA_UR1_ITVL = LOBYTE(d))
#define DMA_UART2_SetInterval(d)        (DMA_UR2_ITVH = HIBYTE(d), DMA_UR2_ITVL = LOBYTE(d))
#define DMA_UART3_SetInterval(d)        (DMA_UR3_ITVH = HIBYTE(d), DMA_UR3_ITVL = LOBYTE(d))
#define DMA_UART4_SetInterval(d)        (DMA_UR4_ITVH = HIBYTE(d), DMA_UR4_ITVL = LOBYTE(d))

/////////////////////////////////////////////////

#define DMAADCCFG_ADCPTY_MSK            (BIT1 | BIT0)
#define DMA_ADC_SetBusPriority(n)       MODIFY_REG(DMA_ADC_CFG,  DMAADCCFG_ADCPTY_MSK, ((n) << 0))

#define DMAADCCR_ENADC_MSK              BIT7
#define DMA_ADC_Enable()                SET_REG_BIT(DMA_ADC_CR, DMAADCCR_ENADC_MSK)
#define DMA_ADC_Disable()               CLR_REG_BIT(DMA_ADC_CR, DMAADCCR_ENADC_MSK)

#define DMAADCCR_TRIG_MSK               BIT6
#define DMA_ADC_Trigger()               SET_REG_BIT(DMA_ADC_CR, DMAADCCR_TRIG_MSK)

#define DMA_ADC_SetAmount(d)            (DMA_ADC_AMTH = HIBYTE(d), DMA_ADC_AMT = LOBYTE(d))
#define DMA_ADC_ReadDone()              MAKEWORD(DMA_ADC_DONE, DMA_ADC_DONEH)
#define DMA_ADC_SetAddress(d)           (DMA_ADC_RXAH = HIBYTE(d), DMA_ADC_RXAL = LOBYTE(d))
#define DMA_ADC_SetInterval(d)          (DMA_ADC_ITVH = HIBYTE(d), DMA_ADC_ITVL = LOBYTE(d))
#define DMA_ADC_SetChannels(d)          (DMA_ADC_CHSW1 = HIBYTE(d), DMA_ADC_CHSW0 = LOBYTE(d))

#define DMAADCCFG2_CVTIMES_MSK          (BIT_LN)
#define DAM_ADC_SetRepeatTimes(n)       MODIFY_REG(DMA_ADC_CFG2,  DMAADCCFG2_CVTIMES_MSK, ((n) << 0))
#define DMA_ADC_DisableRepeatConv()     DAM_ADC_SetRepeatTimes(0)
#define DMA_ADC_SetRepeat2Times()       DAM_ADC_SetRepeatTimes(8)
#define DMA_ADC_SetRepeat4Times()       DAM_ADC_SetRepeatTimes(9)
#define DMA_ADC_SetRepeat8Times()       DAM_ADC_SetRepeatTimes(10)
#define DMA_ADC_SetRepeat16Times()      DAM_ADC_SetRepeatTimes(11)
#define DMA_ADC_SetRepeat32Times()      DAM_ADC_SetRepeatTimes(12)
#define DMA_ADC_SetRepeat64Times()      DAM_ADC_SetRepeatTimes(13)
#define DMA_ADC_SetRepeat128Times()     DAM_ADC_SetRepeatTimes(14)
#define DMA_ADC_SetRepeat256Times()     DAM_ADC_SetRepeatTimes(15)

/////////////////////////////////////////////////

#define DMASPICFG_ACTTX_MSK             BIT6
#define DMASPICFG_ACTRX_MSK             BIT5
#define DMA_SPI_EnableTx()              SET_REG_BIT(DMA_SPI_CFG, DMASPICFG_ACTTX_MSK)
#define DMA_SPI_DisableTx()             CLR_REG_BIT(DMA_SPI_CFG, DMASPICFG_ACTTX_MSK)
#define DMA_SPI_EnableRx()              SET_REG_BIT(DMA_SPI_CFG, DMASPICFG_ACTRX_MSK)
#define DMA_SPI_DisableRx()             CLR_REG_BIT(DMA_SPI_CFG, DMASPICFG_ACTRX_MSK)

#define DMASPICFG_SPIPTY_MSK            (BIT1 | BIT0)
#define DMA_SPI_SetBusPriority(n)       MODIFY_REG(DMA_SPI_CFG,  DMASPICFG_SPIPTY_MSK, ((n) << 0))

#define DMASPICFG2_WRPSS_MSK            BIT2
#define DMA_SPI_AutoSS()                SET_REG_BIT(DMA_SPI_CFG2, DMASPICFG2_WRPSS_MSK)
#define DMA_SPI_ManualSS()              CLR_REG_BIT(DMA_SPI_CFG2, DMASPICFG2_WRPSS_MSK)

#define DMASPICFG2_SSS_MSK              (BIT1 | BIT0)
#define DMA_SPI_SetAutoSSPort(n)        MODIFY_REG(DMA_SPI_CFG2,  DMASPICFG2_SSS_MSK, ((n) << 0))
#define DMA_SPI_SetAutoSSP14()          DMA_SPI_SetAutoSSPort(0)
#define DMA_SPI_SetAutoSSP24()          DMA_SPI_SetAutoSSPort(1)
#define DMA_SPI_SetAutoSSP40()          DMA_SPI_SetAutoSSPort(2)
#define DMA_SPI_SetAutoSSP35()          DMA_SPI_SetAutoSSPort(3)

#define DMASPICR_ENSPI_MSK              BIT7
#define DMA_SPI_Enable()                SET_REG_BIT(DMA_SPI_CR, DMASPICR_ENSPI_MSK)
#define DMA_SPI_Disable()               CLR_REG_BIT(DMA_SPI_CR, DMASPICR_ENSPI_MSK)

#define DMASPICR_TRIGM_MSK              BIT6
#define DMASPICR_TRIGS_MSK              BIT5
#define DMA_SPI_MasterTrigger()         SET_REG_BIT(DMA_SPI_CR, DMASPICR_TRIGM_MSK)
#define DMA_SPI_SlaveTrigger()          SET_REG_BIT(DMA_SPI_CR, DMASPICR_TRIGS_MSK)

#define DMASPICR_CLRFIFO_MSK            BIT0
#define DMA_SPI_ClearFIFO()             SET_REG_BIT(DMA_SPI_CR, DMASPICR_CLRFIFO_MSK)

#define DMASPISTA_TXOVW_MSK             BIT2
#define DMASPISTA_RXLOSS_MSK            BIT1
#define DMA_SPI_CheckOverWriteFlag()    READ_REG_BIT(DMA_SPI_STA,  DMASPISTA_TXOVW_MSK)
#define DMA_SPI_ClearOverWriteFlag()    CLR_REG_BIT(DMA_SPI_STA,   DMASPISTA_TXOVW_MSK)
#define DMA_SPI_CheckRxLossFlag()       READ_REG_BIT(DMA_SPI_STA,  DMASPISTA_RXLOSS_MSK)
#define DMA_SPI_ClearRxLossFlag()       CLR_REG_BIT(DMA_SPI_STA,   DMASPISTA_RXLOSS_MSK)

#define DMA_SPI_SetAmount(d)            (DMA_SPI_AMTH = HIBYTE(d), DMA_SPI_AMT = LOBYTE(d))
#define DMA_SPI_ReadDone()              MAKEWORD(DMA_SPI_DONE, DMA_SPI_DONEH)
#define DMA_SPI_SetTxAddress(d)         (DMA_SPI_TXAH = HIBYTE(d), DMA_SPI_TXAL = LOBYTE(d))
#define DMA_SPI_SetRxAddress(d)         (DMA_SPI_RXAH = HIBYTE(d), DMA_SPI_RXAL = LOBYTE(d))
#define DMA_SPI_SetInterval(d)          (DMA_SPI_ITVH = HIBYTE(d), DMA_SPI_ITVL = LOBYTE(d))

/////////////////////////////////////////////////

#define DMALCMCFG_LCMPTY_MSK            (BIT1 | BIT0)
#define DMA_LCM_SetBusPriority(n)       MODIFY_REG(DMA_LCM_CFG,  DMALCMCFG_LCMPTY_MSK, ((n) << 0))

#define DMALCMCR_ENLCM_MSK              BIT7
#define DMA_LCM_Enable()                SET_REG_BIT(DMA_LCM_CR, DMALCMCR_ENLCM_MSK)
#define DMA_LCM_Disable()               CLR_REG_BIT(DMA_LCM_CR, DMALCMCR_ENLCM_MSK)

#define DMALCMCR_TRIGWC_MSK             BIT6
#define DMALCMCR_TRIGWD_MSK             BIT5
#define DMALCMCR_TRIGRC_MSK             BIT4
#define DMALCMCR_TRIGRD_MSK             BIT3
#define DMA_LCM_TriggerWriteCommand()   SET_REG_BIT(DMA_LCM_CR, DMALCMCR_TRIGWC_MSK)
#define DMA_LCM_TriggerWriteData()      SET_REG_BIT(DMA_LCM_CR, DMALCMCR_TRIGWD_MSK)
#define DMA_LCM_TriggerReadStatus()     SET_REG_BIT(DMA_LCM_CR, DMALCMCR_TRIGRC_MSK)
#define DMA_LCM_TriggerReadData()       SET_REG_BIT(DMA_LCM_CR, DMALCMCR_TRIGRD_MSK)

#define DMALCMSTA_TXOVW_MSK             BIT1
#define DMA_LCM_CheckOverWriteFlag()    READ_REG_BIT(DMA_LCM_STA,  DMALCMSTA_TXOVW_MSK)
#define DMA_LCM_ClearOverWriteFlag()    CLR_REG_BIT(DMA_LCM_STA,   DMALCMSTA_TXOVW_MSK)

#define DMA_LCM_SetAmount(d)            (DMA_LCM_AMTH = HIBYTE(d), DMA_LCM_AMT = LOBYTE(d))
#define DMA_LCM_ReadDone()              MAKEWORD(DMA_LCM_DONE, DMA_LCM_DONEH)
#define DMA_LCM_SetTxAddress(d)         (DMA_LCM_TXAH = HIBYTE(d), DMA_LCM_TXAL = LOBYTE(d))
#define DMA_LCM_SetRxAddress(d)         (DMA_LCM_RXAH = HIBYTE(d), DMA_LCM_RXAL = LOBYTE(d))
#define DMA_LCM_SetInterval(d)          (DMA_LCM_ITVH = HIBYTE(d), DMA_LCM_ITVL = LOBYTE(d))

/////////////////////////////////////////////////

#define DMAI2CTCFG_I2CTPTY_MSK          (BIT1 | BIT0)
#define DMAI2CRCFG_I2CRPTY_MSK          (BIT1 | BIT0)
#define DMA_I2C_SetTxBusPriority(n)     MODIFY_REG(DMA_I2CT_CFG,  DMAI2CTCFG_I2CTPTY_MSK, ((n) << 0))
#define DMA_I2C_SetRxBusPriority(n)     MODIFY_REG(DMA_I2CR_CFG,  DMAI2CRCFG_I2CRPTY_MSK, ((n) << 0))

#define DMAI2CTCR_ENI2CT_MSK            BIT7
#define DMAI2CRCR_ENI2CR_MSK            BIT7
#define DMA_I2C_EnableTx()              SET_REG_BIT(DMA_I2CT_CR, DMAI2CTCR_ENI2CT_MSK)
#define DMA_I2C_DisableTx()             CLR_REG_BIT(DMA_I2CT_CR, DMAI2CTCR_ENI2CT_MSK)
#define DMA_I2C_EnableRx()              SET_REG_BIT(DMA_I2CR_CR, DMAI2CRCR_ENI2CR_MSK)
#define DMA_I2C_DisableRx()             CLR_REG_BIT(DMA_I2CR_CR, DMAI2CRCR_ENI2CR_MSK)

#define DMAI2CTCR_TRIG_MSK              BIT6
#define DMAI2CRCR_TRIG_MSK              BIT6
#define DMA_I2C_TriggerTx()             SET_REG_BIT(DMA_I2CT_CR, DMAI2CTCR_TRIG_MSK)
#define DMA_I2C_TriggerRx()             SET_REG_BIT(DMA_I2CR_CR, DMAI2CRCR_TRIG_MSK)

#define DMAI2CRCR_CLRFIFO_MSK           BIT0
#define DMA_I2C_ClearFIFO()             SET_REG_BIT(DMA_I2CR_CR, DMAI2CRCR_CLRFIFO_MSK)

#define DMAI2CTSTA_TXOVW_MSK            BIT2
#define DMA_I2C_CheckOverWriteFlag()    READ_REG_BIT(DMA_I2CT_STA,  DMAI2CTSTA_TXOVW_MSK)
#define DMA_I2C_ClearOverWriteFlag()    CLR_REG_BIT(DMA_I2CT_STA,   DMAI2CTSTA_TXOVW_MSK)

#define DMAI2CRSTA_RXLOSS_MSK           BIT1
#define DMA_I2C_CheckRxLossFlag()       READ_REG_BIT(DMA_I2CR_STA,  DMAI2CRSTA_RXLOSS_MSK)
#define DMA_I2C_ClearRxLossFlag()       CLR_REG_BIT(DMA_I2CR_STA,   DMAI2CRSTA_RXLOSS_MSK)

#define DMA_I2C_SetTxAmount(d)          (DMA_I2CT_AMTH = HIBYTE(d), DMA_I2CT_AMT = LOBYTE(d))
#define DMA_I2C_SetRxAmount(d)          (DMA_I2CR_AMTH = HIBYTE(d), DMA_I2CR_AMT = LOBYTE(d))
#define DMA_I2C_ReadTxDone()            MAKEWORD(DMA_I2CT_DONE, DMA_I2CT_DONEH)
#define DMA_I2C_ReadRxDone()            MAKEWORD(DMA_I2CR_DONE, DMA_I2CR_DONEH)
#define DMA_I2C_SetTxAddress(d)         (DMA_I2CT_TXAH = HIBYTE(d), DMA_I2CT_TXAL = LOBYTE(d))
#define DMA_I2C_SetRxAddress(d)         (DMA_I2CR_RXAH = HIBYTE(d), DMA_I2CR_RXAL = LOBYTE(d))
#define DMA_I2C_SetInterval(d)          (DMA_I2C_ITVH = HIBYTE(d), DMA_I2C_ITVL = LOBYTE(d))

#define DMAI2CCR_RDSEL_MSK              BIT7
#define DMA_I2C_ReadAmount()            CLR_REG_BIT(DMA_I2C_CR, DMAI2CCR_RDSEL_MSK)
#define DMA_I2C_ReadDone()              SET_REG_BIT(DMA_I2C_CR, DMAI2CCR_RDSEL_MSK)

#define DMAI2CCR_ACKERR_MSK             BIT2
#define DMA_I2C_CheckACK()              READ_REG_BIT(DMA_I2C_CR,  DMAI2CCR_ACKERR_MSK)

#define DMAI2CCR_ERRIE_MSK              BIT1
#define DMA_I2C_EnableACKErrorInt()     SET_REG_BIT(DMA_I2C_CR, DMAI2CCR_ERRIE_MSK)
#define DMA_I2C_DisableACKErrorInt()    CLR_REG_BIT(DMA_I2C_CR, DMAI2CCR_ERRIE_MSK)

#define DMAI2CCR_DMAEN_MSK              BIT0
#define DMA_I2C_EnableDMA()             SET_REG_BIT(DMA_I2C_CR, DMAI2CCR_DMAEN_MSK)
#define DMA_I2C_DisableDMA()            CLR_REG_BIT(DMA_I2C_CR, DMAI2CCR_DMAEN_MSK)

#define DMA_I2C_SetDMAAmount(d)         (DMA_I2C_ST2 = HIBYTE(d), DMA_I2C_ST1 = LOBYTE(d))
#define DMA_I2C_ReadDMAAmount()         MAKEWORD(DMA_I2C_ST1, DMA_I2C_ST2)
#define DMA_I2C_ReadDMADone()           MAKEWORD(DMA_I2C_ST1, DMA_I2C_ST2)

/////////////////////////////////////////////////

#define DMAI2STCFG_I2STPTY_MSK          (BIT1 | BIT0)
#define DMAI2SRCFG_I2SRPTY_MSK          (BIT1 | BIT0)
#define DMA_I2S_SetTxBusPriority(n)     MODIFY_REG(DMA_I2ST_CFG,  DMAI2STCFG_I2STPTY_MSK, ((n) << 0))
#define DMA_I2S_SetRxBusPriority(n)     MODIFY_REG(DMA_I2SR_CFG,  DMAI2SRCFG_I2SRPTY_MSK, ((n) << 0))

#define DMAI2STCR_ENI2ST_MSK            BIT7
#define DMAI2SRCR_ENI2SR_MSK            BIT7
#define DMA_I2S_EnableTx()              SET_REG_BIT(DMA_I2ST_CR, DMAI2STCR_ENI2ST_MSK)
#define DMA_I2S_DisableTx()             CLR_REG_BIT(DMA_I2ST_CR, DMAI2STCR_ENI2ST_MSK)
#define DMA_I2S_EnableRx()              SET_REG_BIT(DMA_I2SR_CR, DMAI2SRCR_ENI2SR_MSK)
#define DMA_I2S_DisableRx()             CLR_REG_BIT(DMA_I2SR_CR, DMAI2SRCR_ENI2SR_MSK)

#define DMAI2STCR_TRIG_MSK              BIT6
#define DMAI2SRCR_TRIG_MSK              BIT6
#define DMA_I2S_TriggerTx()             SET_REG_BIT(DMA_I2ST_CR, DMAI2STCR_TRIG_MSK)
#define DMA_I2S_TriggerRx()             SET_REG_BIT(DMA_I2SR_CR, DMAI2SRCR_TRIG_MSK)

#define DMAI2SRCR_CLRFIFO_MSK           BIT0
#define DMA_I2S_ClearFIFO()             SET_REG_BIT(DMA_I2SR_CR, DMAI2SRCR_CLRFIFO_MSK)

#define DMAI2STSTA_TXOVW_MSK            BIT2
#define DMA_I2S_CheckOverWriteFlag()    READ_REG_BIT(DMA_I2ST_STA,  DMAI2STSTA_TXOVW_MSK)
#define DMA_I2S_ClearOverWriteFlag()    CLR_REG_BIT(DMA_I2ST_STA,   DMAI2STSTA_TXOVW_MSK)

#define DMAI2SRSTA_RXLOSS_MSK           BIT1
#define DMA_I2S_CheckRxLossFlag()       READ_REG_BIT(DMA_I2SR_STA,  DMAI2SRSTA_RXLOSS_MSK)
#define DMA_I2S_ClearRxLossFlag()       CLR_REG_BIT(DMA_I2SR_STA,   DMAI2SRSTA_RXLOSS_MSK)

#define DMA_I2S_SetTxAmount(d)          (DMA_I2ST_AMTH = HIBYTE(d), DMA_I2ST_AMT = LOBYTE(d))
#define DMA_I2S_SetRxAmount(d)          (DMA_I2SR_AMTH = HIBYTE(d), DMA_I2SR_AMT = LOBYTE(d))
#define DMA_I2S_ReadTxDone()            MAKEWORD(DMA_I2ST_DONE, DMA_I2ST_DONEH)
#define DMA_I2S_ReadRxDone()            MAKEWORD(DMA_I2SR_DONE, DMA_I2SR_DONEH)
#define DMA_I2S_SetTxAddress(d)         (DMA_I2ST_TXAH = HIBYTE(d), DMA_I2ST_TXAL = LOBYTE(d))
#define DMA_I2S_SetRxAddress(d)         (DMA_I2SR_RXAH = HIBYTE(d), DMA_I2SR_RXAL = LOBYTE(d))
#define DMA_I2S_SetInterval(d)          (DMA_I2S_ITVH = HIBYTE(d), DMA_I2S_ITVL = LOBYTE(d))

/////////////////////////////////////////////////

#define DMAQSPICFG_ACTWR_MSK            BIT6
#define DMAQSPICFG_ACTRD_MSK            BIT5
#define DMA_QSPI_EnableWrite()          SET_REG_BIT(DMA_QSPI_CFG, DMAQSPICFG_ACTWR_MSK)
#define DMA_QSPI_DisableWrite()         CLR_REG_BIT(DMA_QSPI_CFG, DMAQSPICFG_ACTWR_MSK)
#define DMA_QSPI_EnableRead()           SET_REG_BIT(DMA_QSPI_CFG, DMAQSPICFG_ACTRD_MSK)
#define DMA_QSPI_DisableRead()          CLR_REG_BIT(DMA_QSPI_CFG, DMAQSPICFG_ACTRD_MSK)

#define DMAQSPICFG_QSPIPTY_MSK          (BIT1 | BIT0)
#define DMA_QSPI_SetBusPriority(n)      MODIFY_REG(DMA_QSPI_CFG,  DMAQSPICFG_QSPIPTY_MSK, ((n) << 0))

#define DMAQSPICR_ENQSPI_MSK            BIT7
#define DMA_QSPI_Enable()               SET_REG_BIT(DMA_QSPI_CR, DMAQSPICR_ENQSPI_MSK)
#define DMA_QSPI_Disable()              CLR_REG_BIT(DMA_QSPI_CR, DMAQSPICR_ENQSPI_MSK)

#define DMAQSPICR_TRIGW_MSK             BIT6
#define DMAQSPICR_TRIGR_MSK             BIT5
#define DMA_QSPI_TriggerWrite()         SET_REG_BIT(DMA_QSPI_CR, DMAQSPICR_TRIGW_MSK)
#define DMA_QSPI_TriggerRead()          SET_REG_BIT(DMA_QSPI_CR, DMAQSPICR_TRIGR_MSK)

#define DMAQSPICR_CLRRFIFO_MSK          BIT0
#define DMAQSPICR_CLRWFIFO_MSK          BIT1
#define DMA_QSPI_ClearReadFIFO()        SET_REG_BIT(DMA_QSPI_CR, DMAQSPICR_CLRRFIFO_MSK)
#define DMA_QSPI_ClearWriteFIFO()       SET_REG_BIT(DMA_QSPI_CR, DMAQSPICR_CLRWFIFO_MSK)

#define DMA_QSPI_SetAmount(d)           (DMA_QSPI_AMTH = HIBYTE(d), DMA_QSPI_AMT = LOBYTE(d))
#define DMA_QSPI_ReadDone()             MAKEWORD(DMA_QSPI_DONE, DMA_QSPI_DONEH)
#define DMA_QSPI_SetTxAddress(d)        (DMA_QSPI_TXAH = HIBYTE(d), DMA_QSPI_TXAL = LOBYTE(d))
#define DMA_QSPI_SetRxAddress(d)        (DMA_QSPI_RXAH = HIBYTE(d), DMA_QSPI_RXAL = LOBYTE(d))
#define DMA_QSPI_SetInterval(d)         (DMA_QSPI_ITVH = HIBYTE(d), DMA_QSPI_ITVL = LOBYTE(d))

/////////////////////////////////////////////////

#define DMAPWMATCFG_PWMATPTY_MSK        (BIT1 | BIT0)
#define DMAPWMARCFG_PWMARPTY_MSK        (BIT1 | BIT0)
#define DMA_PWMA_SetTxBusPriority(n)    MODIFY_REG(DMA_PWMAT_CFG,  DMAPWMATCFG_PWMATPTY_MSK, ((n) << 0))
#define DMA_PWMA_SetRxBusPriority(n)    MODIFY_REG(DMA_PWMAR_CFG,  DMAPWMARCFG_PWMARPTY_MSK, ((n) << 0))

#define DMAPWMATCR_ENPWMAT_MSK          BIT7
#define DMAPWMARCR_ENPWMAR_MSK          BIT7
#define DMA_PWMA_EnableTx()             SET_REG_BIT(DMA_PWMAT_CR, DMAPWMATCR_ENPWMAT_MSK)
#define DMA_PWMA_DisableTx()            CLR_REG_BIT(DMA_PWMAT_CR, DMAPWMATCR_ENPWMAT_MSK)
#define DMA_PWMA_EnableRx()             SET_REG_BIT(DMA_PWMAR_CR, DMAPWMARCR_ENPWMAR_MSK)
#define DMA_PWMA_DisableRx()            CLR_REG_BIT(DMA_PWMAR_CR, DMAPWMARCR_ENPWMAR_MSK)

#define DMAPWMATCR_TRIG_MSK             BIT6
#define DMAPWMARCR_TRIG_MSK             BIT6
#define DMA_PWMA_TriggerTx()            SET_REG_BIT(DMA_PWMAT_CR, DMAPWMATCR_TRIG_MSK)
#define DMA_PWMA_TriggerRx()            SET_REG_BIT(DMA_PWMAR_CR, DMAPWMARCR_TRIG_MSK)

#define DMAPWMARCR_CLRFIFO_MSK          BIT0
#define DMA_PWMA_ClearFIFO()            SET_REG_BIT(DMA_PWMAR_CR, DMAPWMARCR_CLRFIFO_MSK)

#define DMAPWMATSTA_TXOVW_MSK           BIT2
#define DMA_PWMA_CheckOverWriteFlag()   READ_REG_BIT(DMA_PWMAT_STA,  DMAPWMATSTA_TXOVW_MSK)
#define DMA_PWMA_ClearOverWriteFlag()   CLR_REG_BIT(DMA_PWMAT_STA,   DMAPWMATSTA_TXOVW_MSK)

#define DMAPWMARSTA_RXLOSS_MSK          BIT1
#define DMA_PWMA_CheckRxLossFlag()      READ_REG_BIT(DMA_PWMAR_STA,  DMAPWMARSTA_RXLOSS_MSK)
#define DMA_PWMA_ClearRxLossFlag()      CLR_REG_BIT(DMA_PWMAR_STA,   DMAPWMARSTA_RXLOSS_MSK)

#define DMA_PWMA_SetTxAmount(d)         (DMA_PWMAT_AMTH = HIBYTE(d), DMA_PWMAT_AMT = LOBYTE(d))
#define DMA_PWMA_SetRxAmount(d)         (DMA_PWMAR_AMTH = HIBYTE(d), DMA_PWMAR_AMT = LOBYTE(d))
#define DMA_PWMA_ReadTxDone()           MAKEWORD(DMA_PWMAT_DONE, DMA_PWMAT_DONEH)
#define DMA_PWMA_ReadRxDone()           MAKEWORD(DMA_PWMAR_DONE, DMA_PWMAR_DONEH)
#define DMA_PWMA_SetTxAddress(d)        (DMA_PWMAT_TXAH = HIBYTE(d), DMA_PWMAT_TXAL = LOBYTE(d))
#define DMA_PWMA_SetRxAddress(d)        (DMA_PWMAR_RXAH = HIBYTE(d), DMA_PWMAR_RXAL = LOBYTE(d))
#define DMA_PWMA_SetInterval(d)         (DMA_PWMA_ITVH = HIBYTE(d), DMA_PWMA_ITVL = LOBYTE(d))

/////////////////////////////////////////////////

#define RSTCFG_P47RST_MSK               BIT4
#define RST_EnableResetPin()            SET_REG_BIT(RSTCFG, RSTCFG_P47RST_MSK)
#define RST_ResetPinAsGPIO()            CLR_REG_BIT(RSTCFG, RSTCFG_P47RST_MSK)

#define RSTCFG_ENLVR_MSK                BIT7
#define LVR_EnableLVR()                 SET_REG_BIT(RSTCFG, RSTCFG_ENLVR_MSK)
#define LVR_DisableLVR()                CLR_REG_BIT(RSTCFG, RSTCFG_ENLVR_MSK)

#define RSTCFG_LVDS_MSK                 (BIT1 | BIT0)
#define LVR_SetLVDS(n)                  MODIFY_REG(RSTCFG,  RSTCFG_LVDS_MSK, ((n) << 0))
#define LVR_SetLVDLevelV2p0()           LVR_SetLVDS(0)
#define LVR_SetLVDLevelV2p4()           LVR_SetLVDS(1)
#define LVR_SetLVDLevelV2p7()           LVR_SetLVDS(2)
#define LVR_SetLVDLevelV3p0()           LVR_SetLVDS(3)

/////////////////////////////////////////////////

#define PWMAENO_ENO1P_MSK               BIT0
#define PWMAENO_ENO1N_MSK               BIT1
#define PWMAENO_ENO2P_MSK               BIT2
#define PWMAENO_ENO2N_MSK               BIT3
#define PWMAENO_ENO3P_MSK               BIT4
#define PWMAENO_ENO3N_MSK               BIT5
#define PWMAENO_ENO4P_MSK               BIT6
#define PWMAENO_ENO4N_MSK               BIT7
#define PWMA_EnablePWM1POutput()        SET_REG_BIT(PWMA_ENO, PWMAENO_ENO1P_MSK)
#define PWMA_DisablePWM1POutput()       CLR_REG_BIT(PWMA_ENO, PWMAENO_ENO1P_MSK)
#define PWMA_EnablePWM1NOutput()        SET_REG_BIT(PWMA_ENO, PWMAENO_ENO1N_MSK)
#define PWMA_DisablePWM1NOutput()       CLR_REG_BIT(PWMA_ENO, PWMAENO_ENO1N_MSK)
#define PWMA_EnablePWM2POutput()        SET_REG_BIT(PWMA_ENO, PWMAENO_ENO2P_MSK)
#define PWMA_DisablePWM2POutput()       CLR_REG_BIT(PWMA_ENO, PWMAENO_ENO2P_MSK)
#define PWMA_EnablePWM2NOutput()        SET_REG_BIT(PWMA_ENO, PWMAENO_ENO2N_MSK)
#define PWMA_DisablePWM2NOutput()       CLR_REG_BIT(PWMA_ENO, PWMAENO_ENO2N_MSK)
#define PWMA_EnablePWM3POutput()        SET_REG_BIT(PWMA_ENO, PWMAENO_ENO3P_MSK)
#define PWMA_DisablePWM3POutput()       CLR_REG_BIT(PWMA_ENO, PWMAENO_ENO3P_MSK)
#define PWMA_EnablePWM3NOutput()        SET_REG_BIT(PWMA_ENO, PWMAENO_ENO3N_MSK)
#define PWMA_DisablePWM3NOutput()       CLR_REG_BIT(PWMA_ENO, PWMAENO_ENO3N_MSK)
#define PWMA_EnablePWM4POutput()        SET_REG_BIT(PWMA_ENO, PWMAENO_ENO4P_MSK)
#define PWMA_DisablePWM4POutput()       CLR_REG_BIT(PWMA_ENO, PWMAENO_ENO4P_MSK)
#define PWMA_EnablePWM4NOutput()        SET_REG_BIT(PWMA_ENO, PWMAENO_ENO4N_MSK)
#define PWMA_DisablePWM4NOutput()       CLR_REG_BIT(PWMA_ENO, PWMAENO_ENO4N_MSK)

#define HSPWMA_EnablePWM1POutput()      SET_HSPWMA_BIT(PWMA_ENO, PWMAENO_ENO1P_MSK)
#define HSPWMA_DisablePWM1POutput()     CLR_HSPWMA_BIT(PWMA_ENO, PWMAENO_ENO1P_MSK)
#define HSPWMA_EnablePWM1NOutput()      SET_HSPWMA_BIT(PWMA_ENO, PWMAENO_ENO1N_MSK)
#define HSPWMA_DisablePWM1NOutput()     CLR_HSPWMA_BIT(PWMA_ENO, PWMAENO_ENO1N_MSK)
#define HSPWMA_EnablePWM2POutput()      SET_HSPWMA_BIT(PWMA_ENO, PWMAENO_ENO2P_MSK)
#define HSPWMA_DisablePWM2POutput()     CLR_HSPWMA_BIT(PWMA_ENO, PWMAENO_ENO2P_MSK)
#define HSPWMA_EnablePWM2NOutput()      SET_HSPWMA_BIT(PWMA_ENO, PWMAENO_ENO2N_MSK)
#define HSPWMA_DisablePWM2NOutput()     CLR_HSPWMA_BIT(PWMA_ENO, PWMAENO_ENO2N_MSK)
#define HSPWMA_EnablePWM3POutput()      SET_HSPWMA_BIT(PWMA_ENO, PWMAENO_ENO3P_MSK)
#define HSPWMA_DisablePWM3POutput()     CLR_HSPWMA_BIT(PWMA_ENO, PWMAENO_ENO3P_MSK)
#define HSPWMA_EnablePWM3NOutput()      SET_HSPWMA_BIT(PWMA_ENO, PWMAENO_ENO3N_MSK)
#define HSPWMA_DisablePWM3NOutput()     CLR_HSPWMA_BIT(PWMA_ENO, PWMAENO_ENO3N_MSK)
#define HSPWMA_EnablePWM4POutput()      SET_HSPWMA_BIT(PWMA_ENO, PWMAENO_ENO4P_MSK)
#define HSPWMA_DisablePWM4POutput()     CLR_HSPWMA_BIT(PWMA_ENO, PWMAENO_ENO4P_MSK)
#define HSPWMA_EnablePWM4NOutput()      SET_HSPWMA_BIT(PWMA_ENO, PWMAENO_ENO4N_MSK)
#define HSPWMA_DisablePWM4NOutput()     CLR_HSPWMA_BIT(PWMA_ENO, PWMAENO_ENO4N_MSK)

#define PWMAIOAUX_AUX1P_MSK             BIT0
#define PWMAIOAUX_AUX1N_MSK             BIT1
#define PWMAIOAUX_AUX2P_MSK             BIT2
#define PWMAIOAUX_AUX2N_MSK             BIT3
#define PWMAIOAUX_AUX3P_MSK             BIT4
#define PWMAIOAUX_AUX3N_MSK             BIT5
#define PWMAIOAUX_AUX4P_MSK             BIT6
#define PWMAIOAUX_AUX4N_MSK             BIT7
#define PWMA_EnoCtrlPWM1P()             CLR_REG_BIT(PWMA_IOAUX, PWMAIOAUX_AUX1P_MSK)
#define PWMA_EnoBrkCtrlPWM1P()          SET_REG_BIT(PWMA_IOAUX, PWMAIOAUX_AUX1P_MSK)
#define PWMA_EnoCtrlPWM1N()             CLR_REG_BIT(PWMA_IOAUX, PWMAIOAUX_AUX1N_MSK)
#define PWMA_EnoBrkCtrlPWM1N()          SET_REG_BIT(PWMA_IOAUX, PWMAIOAUX_AUX1N_MSK)
#define PWMA_EnoCtrlPWM2P()             CLR_REG_BIT(PWMA_IOAUX, PWMAIOAUX_AUX2P_MSK)
#define PWMA_EnoBrkCtrlPWM2P()          SET_REG_BIT(PWMA_IOAUX, PWMAIOAUX_AUX2P_MSK)
#define PWMA_EnoCtrlPWM2N()             CLR_REG_BIT(PWMA_IOAUX, PWMAIOAUX_AUX2N_MSK)
#define PWMA_EnoBrkCtrlPWM2N()          SET_REG_BIT(PWMA_IOAUX, PWMAIOAUX_AUX2N_MSK)
#define PWMA_EnoCtrlPWM3P()             CLR_REG_BIT(PWMA_IOAUX, PWMAIOAUX_AUX3P_MSK)
#define PWMA_EnoBrkCtrlPWM3P()          SET_REG_BIT(PWMA_IOAUX, PWMAIOAUX_AUX3P_MSK)
#define PWMA_EnoCtrlPWM3N()             CLR_REG_BIT(PWMA_IOAUX, PWMAIOAUX_AUX3N_MSK)
#define PWMA_EnoBrkCtrlPWM3N()          SET_REG_BIT(PWMA_IOAUX, PWMAIOAUX_AUX3N_MSK)
#define PWMA_EnoCtrlPWM4P()             CLR_REG_BIT(PWMA_IOAUX, PWMAIOAUX_AUX4P_MSK)
#define PWMA_EnoBrkCtrlPWM4P()          SET_REG_BIT(PWMA_IOAUX, PWMAIOAUX_AUX4P_MSK)
#define PWMA_EnoCtrlPWM4N()             CLR_REG_BIT(PWMA_IOAUX, PWMAIOAUX_AUX4N_MSK)
#define PWMA_EnoBrkCtrlPWM4N()          SET_REG_BIT(PWMA_IOAUX, PWMAIOAUX_AUX4N_MSK)

#define HSPWMA_EnoCtrlPWM1P()           CLR_HSPWMA_BIT(PWMA_IOAUX, PWMAIOAUX_AUX1P_MSK)
#define HSPWMA_EnoBrkCtrlPWM1P()        SET_HSPWMA_BIT(PWMA_IOAUX, PWMAIOAUX_AUX1P_MSK)
#define HSPWMA_EnoCtrlPWM1N()           CLR_HSPWMA_BIT(PWMA_IOAUX, PWMAIOAUX_AUX1N_MSK)
#define HSPWMA_EnoBrkCtrlPWM1N()        SET_HSPWMA_BIT(PWMA_IOAUX, PWMAIOAUX_AUX1N_MSK)
#define HSPWMA_EnoCtrlPWM2P()           CLR_HSPWMA_BIT(PWMA_IOAUX, PWMAIOAUX_AUX2P_MSK)
#define HSPWMA_EnoBrkCtrlPWM2P()        SET_HSPWMA_BIT(PWMA_IOAUX, PWMAIOAUX_AUX2P_MSK)
#define HSPWMA_EnoCtrlPWM2N()           CLR_HSPWMA_BIT(PWMA_IOAUX, PWMAIOAUX_AUX2N_MSK)
#define HSPWMA_EnoBrkCtrlPWM2N()        SET_HSPWMA_BIT(PWMA_IOAUX, PWMAIOAUX_AUX2N_MSK)
#define HSPWMA_EnoCtrlPWM3P()           CLR_HSPWMA_BIT(PWMA_IOAUX, PWMAIOAUX_AUX3P_MSK)
#define HSPWMA_EnoBrkCtrlPWM3P()        SET_HSPWMA_BIT(PWMA_IOAUX, PWMAIOAUX_AUX3P_MSK)
#define HSPWMA_EnoCtrlPWM3N()           CLR_HSPWMA_BIT(PWMA_IOAUX, PWMAIOAUX_AUX3N_MSK)
#define HSPWMA_EnoBrkCtrlPWM3N()        SET_HSPWMA_BIT(PWMA_IOAUX, PWMAIOAUX_AUX3N_MSK)
#define HSPWMA_EnoCtrlPWM4P()           CLR_HSPWMA_BIT(PWMA_IOAUX, PWMAIOAUX_AUX4P_MSK)
#define HSPWMA_EnoBrkCtrlPWM4P()        SET_HSPWMA_BIT(PWMA_IOAUX, PWMAIOAUX_AUX4P_MSK)
#define HSPWMA_EnoCtrlPWM4N()           CLR_HSPWMA_BIT(PWMA_IOAUX, PWMAIOAUX_AUX4N_MSK)
#define HSPWMA_EnoBrkCtrlPWM4N()        SET_HSPWMA_BIT(PWMA_IOAUX, PWMAIOAUX_AUX4N_MSK)

#define PWMACR1_ARPE_MSK                BIT7
#define PWMA_UnbufferARR()              CLR_REG_BIT(PWMA_CR1, PWMACR1_ARPE_MSK)
#define PWMA_BufferARR()                SET_REG_BIT(PWMA_CR1, PWMACR1_ARPE_MSK)

#define HSPWMA_UnbufferARR()            CLR_HSPWMA_BIT(PWMA_CR1, PWMACR1_ARPE_MSK)
#define HSPWMA_BufferARR()              SET_HSPWMA_BIT(PWMA_CR1, PWMACR1_ARPE_MSK)

#define PWMACR1_CMS_MSK                 (BIT6 | BIT5)
#define PWMA_SetAlignMode(n)            MODIFY_REG(PWMA_CR1,  PWMACR1_CMS_MSK, ((n) << 5))
#define PWMA_EdgeAlignMode()            PWMA_SetAlignMode(0)
#define PWMA_CenterAlignMode1()         PWMA_SetAlignMode(1)
#define PWMA_CenterAlignMode2()         PWMA_SetAlignMode(2)
#define PWMA_CenterAlignMode3()         PWMA_SetAlignMode(3)

#define HSPWMA_SetAlignMode(n)          MODIFY_HSPWMA(PWMA_CR1,  PWMACR1_CMS_MSK, ((n) << 5))
#define HSPWMA_EdgeAlignMode()          HSPWMA_SetAlignMode(0)
#define HSPWMA_CenterAlignMode1()       HSPWMA_SetAlignMode(1)
#define HSPWMA_CenterAlignMode2()       HSPWMA_SetAlignMode(2)
#define HSPWMA_CenterAlignMode3()       HSPWMA_SetAlignMode(3)

#define PWMACR1_DIR_MSK                 BIT4
#define PWMA_UpCounter()                CLR_REG_BIT(PWMA_CR1, PWMACR1_DIR_MSK)
#define PWMA_DownCounter()              SET_REG_BIT(PWMA_CR1, PWMACR1_DIR_MSK)

#define HSPWMA_UpCounter()              CLR_HSPWMA_BIT(PWMA_CR1, PWMACR1_DIR_MSK)
#define HSPWMA_DownCounter()            SET_HSPWMA_BIT(PWMA_CR1, PWMACR1_DIR_MSK)

#define PWMACR1_OPM_MSK                 BIT3
#define PWMA_ContinueMode()             CLR_REG_BIT(PWMA_CR1, PWMACR1_OPM_MSK)
#define PWMA_OnePulseMode()             SET_REG_BIT(PWMA_CR1, PWMACR1_OPM_MSK)

#define HSPWMA_ContinueMode()           CLR_HSPWMA_BIT(PWMA_CR1, PWMACR1_OPM_MSK)
#define HSPWMA_OnePulseMode()           SET_HSPWMA_BIT(PWMA_CR1, PWMACR1_OPM_MSK)

#define PWMACR1_URS_MSK                 BIT2
#define PWMA_UpdateRequestMode1()       SET_REG_BIT(PWMA_CR1, PWMACR1_URS_MSK)
#define PWMA_UpdateRequestMode2()       CLR_REG_BIT(PWMA_CR1, PWMACR1_URS_MSK)

#define HSPWMA_UpdateRequestMode1()     SET_HSPWMA_BIT(PWMA_CR1, PWMACR1_URS_MSK)
#define HSPWMA_UpdateRequestMode2()     CLR_HSPWMA_BIT(PWMA_CR1, PWMACR1_URS_MSK)

#define PWMACR1_UDIS_MSK                BIT1
#define PWMA_EnableUpdate()             CLR_REG_BIT(PWMA_CR1, PWMACR1_UDIS_MSK)
#define PWMA_DisableUpdate()            SET_REG_BIT(PWMA_CR1, PWMACR1_UDIS_MSK)

#define HSPWMA_EnableUpdate()           CLR_HSPWMA_BIT(PWMA_CR1, PWMACR1_UDIS_MSK)
#define HSPWMA_DisableUpdate()          SET_HSPWMA_BIT(PWMA_CR1, PWMACR1_UDIS_MSK)

#define PWMACR1_CEN_MSK                 BIT0
#define PWMA_Run()                      SET_REG_BIT(PWMA_CR1, PWMACR1_CEN_MSK)
#define PWMA_Stop()                     CLR_REG_BIT(PWMA_CR1, PWMACR1_CEN_MSK)

#define HSPWMA_Run()                    SET_HSPWMA_BIT(PWMA_CR1, PWMACR1_CEN_MSK)
#define HSPWMA_Stop()                   CLR_HSPWMA_BIT(PWMA_CR1, PWMACR1_CEN_MSK)

#define PWMACR2_TIS_MSK                 BIT7
#define PWMA_PWM1PToTI()                CLR_REG_BIT(PWMA_CR2, PWMACR2_TIS_MSK)
#define PWMA_Xor1P2P3PToTI()            SET_REG_BIT(PWMA_CR2, PWMACR2_TIS_MSK)

#define HSPWMA_PWM1PToTI()              CLR_HSPWMA_BIT(PWMA_CR2, PWMACR2_TIS_MSK)
#define HSPWMA_Xor1P2P3PToTI()          SET_HSPWMA_BIT(PWMA_CR2, PWMACR2_TIS_MSK)

#define PWMACR2_MMS_MSK                 (BIT6 | BIT5 | BIT4)
#define PWMA_SelectTRGO(n)              MODIFY_REG(PWMA_CR2,  PWMACR2_MMS_MSK, ((n) << 4))
#define PWMA_ResetAsTRGO()              PWMA_SelectTRGO(0)
#define PWMA_EnableAsTRGO()             PWMA_SelectTRGO(1)
#define PWMA_UpdateAsTRGO()             PWMA_SelectTRGO(2)
#define PWMA_MatchAsTRGO()              PWMA_SelectTRGO(3)
#define PWMA_OC1REFAsTRGO()             PWMA_SelectTRGO(4)
#define PWMA_OC2REFAsTRGO()             PWMA_SelectTRGO(5)
#define PWMA_OC3REFAsTRGO()             PWMA_SelectTRGO(6)
#define PWMA_OC4REFAsTRGO()             PWMA_SelectTRGO(7)

#define HSPWMA_SelectTRGO(n)            MODIFY_HSPWMA(PWMA_CR2,  PWMACR2_MMS_MSK, ((n) << 4))
#define HSPWMA_ResetAsTRGO()            HSPWMA_SelectTRGO(0)
#define HSPWMA_EnableAsTRGO()           HSPWMA_SelectTRGO(1)
#define HSPWMA_UpdateAsTRGO()           HSPWMA_SelectTRGO(2)
#define HSPWMA_MatchAsTRGO()            HSPWMA_SelectTRGO(3)
#define HSPWMA_OC1REFAsTRGO()           HSPWMA_SelectTRGO(4)
#define HSPWMA_OC2REFAsTRGO()           HSPWMA_SelectTRGO(5)
#define HSPWMA_OC3REFAsTRGO()           HSPWMA_SelectTRGO(6)
#define HSPWMA_OC4REFAsTRGO()           HSPWMA_SelectTRGO(7)

#define PWMACR2_COMS_MSK                BIT2
#define PWMA_CCOCUpdateMode0()          CLR_REG_BIT(PWMA_CR2, PWMACR2_COMS_MSK)
#define PWMA_CCOCUpdateMode1()          SET_REG_BIT(PWMA_CR2, PWMACR2_COMS_MSK)

#define HSPWMA_CCOCUpdateMode0()        CLR_HSPWMA_BIT(PWMA_CR2, PWMACR2_COMS_MSK)
#define HSPWMA_CCOCUpdateMode1()        SET_HSPWMA_BIT(PWMA_CR2, PWMACR2_COMS_MSK)

#define PWMACR2_CCPC_MSK                BIT0
#define PWMA_UnbufferCCOC()             CLR_REG_BIT(PWMA_CR2, PWMACR2_CCPC_MSK)
#define PWMA_BufferCCOC()               SET_REG_BIT(PWMA_CR2, PWMACR2_CCPC_MSK)

#define HSPWMA_UnbufferCCOC()           CLR_HSPWMA_BIT(PWMA_CR2, PWMACR2_CCPC_MSK)
#define HSPWMA_BufferCCOC()             SET_HSPWMA_BIT(PWMA_CR2, PWMACR2_CCPC_MSK)

#define PWMASMCR_MSM_MSK                BIT7
#define PWMA_StandaloneMode()           CLR_REG_BIT(PWMA_SMCR, PWMASMCR_MSM_MSK)
#define PWMA_MasterSlaveMode()          SET_REG_BIT(PWMA_SMCR, PWMASMCR_MSM_MSK)

#define HSPWMA_StandaloneMode()         CLR_HSPWMA_BIT(PWMA_SMCR, PWMASMCR_MSM_MSK)
#define HSPWMA_MasterSlaveMode()        SET_HSPWMA_BIT(PWMA_SMCR, PWMASMCR_MSM_MSK)

#define PWMASMCR_TS_MSK                 (BIT6 | BIT5 | BIT4)
#define PWMA_SelectTRGI(n)              MODIFY_REG(PWMA_SMCR,  PWMASMCR_TS_MSK, ((n) << 4))
#define PWMA_DisableTRGI()              PWMA_SelectTRGI(0)
#define PWMA_ITR2AsTRGI()               PWMA_SelectTRGI(2)
#define PWMA_TI1FEDAsTRGI()             PWMA_SelectTRGI(4)
#define PWMA_TI1FP1AsTRGI()             PWMA_SelectTRGI(5)
#define PWMA_TI2FP2AsTRGI()             PWMA_SelectTRGI(6)
#define PWMA_ETRFAsTRGI()               PWMA_SelectTRGI(7)

#define HSPWMA_SelectTRGI(n)            MODIFY_HSPWMA(PWMA_SMCR,  PWMASMCR_TS_MSK, ((n) << 4))
#define HSPWMA_DisableTRGI()            HSPWMA_SelectTRGI(0)
#define HSPWMA_ITR2AsTRGI()             HSPWMA_SelectTRGI(2)
#define HSPWMA_TI1FEDAsTRGI()           HSPWMA_SelectTRGI(4)
#define HSPWMA_TI1FP1AsTRGI()           HSPWMA_SelectTRGI(5)
#define HSPWMA_TI2FP2AsTRGI()           HSPWMA_SelectTRGI(6)
#define HSPWMA_ETRFAsTRGI()             HSPWMA_SelectTRGI(7)

#define PWMASMCR_SMS_MSK                (BIT2 | BIT1 | BIT0)
#define PWMA_SetClockMode(n)            MODIFY_REG(PWMA_SMCR,  PWMASMCR_SMS_MSK, ((n) << 0))
#define PWMA_InternalClockMode()        PWMA_SetClockMode(0)
#define PWMA_EncoderMode1()             PWMA_SetClockMode(1)
#define PWMA_EncoderMode2()             PWMA_SetClockMode(2)
#define PWMA_EncoderMode3()             PWMA_SetClockMode(3)
#define PWMA_TriggerResetMode()         PWMA_SetClockMode(4)
#define PWMA_TriggerGateMode()          PWMA_SetClockMode(5)
#define PWMA_TriggerStartMode()         PWMA_SetClockMode(6)
#define PWMA_ExternalClockMode()        PWMA_SetClockMode(7)

#define HSPWMA_SetClockMode(n)          MODIFY_HSPWMA(PWMA_SMCR,  PWMASMCR_SMS_MSK, ((n) << 0))
#define HSPWMA_InternalClockMode()      HSPWMA_SetClockMode(0)
#define HSPWMA_EncoderMode1()           HSPWMA_SetClockMode(1)
#define HSPWMA_EncoderMode2()           HSPWMA_SetClockMode(2)
#define HSPWMA_EncoderMode3()           HSPWMA_SetClockMode(3)
#define HSPWMA_TriggerResetMode()       HSPWMA_SetClockMode(4)
#define HSPWMA_TriggerGateMode()        HSPWMA_SetClockMode(5)
#define HSPWMA_TriggerStartMode()       HSPWMA_SetClockMode(6)
#define HSPWMA_ExternalClockMode()      HSPWMA_SetClockMode(7)

#define PWMAETR_ETP_MSK                 BIT7
#define PWMA_ETRNonInverted()           CLR_REG_BIT(PWMA_ETR, PWMAETR_ETP_MSK)
#define PWMA_ETRInverted()              SET_REG_BIT(PWMA_ETR, PWMAETR_ETP_MSK)

#define HSPWMA_ETRNonInverted()         CLR_HSPWMA_BIT(PWMA_ETR, PWMAETR_ETP_MSK)
#define HSPWMA_ETRInverted()            SET_HSPWMA_BIT(PWMA_ETR, PWMAETR_ETP_MSK)

#define PWMAETR_ECE_MSK                 BIT6
#define PWMA_DisableExternalClock2()    CLR_REG_BIT(PWMA_ETR, PWMAETR_ECE_MSK)
#define PWMA_EnableExternalClock2()     SET_REG_BIT(PWMA_ETR, PWMAETR_ECE_MSK)

#define HSPWMA_DisableExternalClock2()  CLR_HSPWMA_BIT(PWMA_ETR, PWMAETR_ECE_MSK)
#define HSPWMA_EnableExternalClock2()   SET_HSPWMA_BIT(PWMA_ETR, PWMAETR_ECE_MSK)

#define PWMAETR_ETPS_MSK                (BIT5 | BIT4)
#define PWMA_SetETRDivider(n)           MODIFY_REG(PWMA_ETR,  PWMAETR_ETPS_MSK, ((n) << 4))
#define PWMA_ETRDivider1()              PWMA_SetETRDivider(0)
#define PWMA_ETRDivider2()              PWMA_SetETRDivider(1)
#define PWMA_ETRDivider4()              PWMA_SetETRDivider(2)
#define PWMA_ETRDivider8()              PWMA_SetETRDivider(3)

#define HSPWMA_SetETRDivider(n)         MODIFY_HSPWMA(PWMA_ETR,  PWMAETR_ETPS_MSK, ((n) << 4))
#define HSPWMA_ETRDivider1()            HSPWMA_SetETRDivider(0)
#define HSPWMA_ETRDivider2()            HSPWMA_SetETRDivider(1)
#define HSPWMA_ETRDivider4()            HSPWMA_SetETRDivider(2)
#define HSPWMA_ETRDivider8()            HSPWMA_SetETRDivider(3)

#define PWMAETR_ETF_MSK                 (BIT_LN)
#define PWMA_SetETRFilter(n)            MODIFY_REG(PWMA_ETR,  PWMAETR_ETF_MSK, ((n) << 0))
#define PWMA_ETRFilter1()               PWMA_SetETRFilter(0)
#define PWMA_ETRFilter2()               PWMA_SetETRFilter(1)
#define PWMA_ETRFilter4()               PWMA_SetETRFilter(2)
#define PWMA_ETRFilter8()               PWMA_SetETRFilter(3)
#define PWMA_ETRFilter12()              PWMA_SetETRFilter(4)
#define PWMA_ETRFilter16()              PWMA_SetETRFilter(5)
#define PWMA_ETRFilter24()              PWMA_SetETRFilter(6)
#define PWMA_ETRFilter32()              PWMA_SetETRFilter(7)
#define PWMA_ETRFilter48()              PWMA_SetETRFilter(8)
#define PWMA_ETRFilter64()              PWMA_SetETRFilter(9)
#define PWMA_ETRFilter80()              PWMA_SetETRFilter(10)
#define PWMA_ETRFilter96()              PWMA_SetETRFilter(11)
#define PWMA_ETRFilter128()             PWMA_SetETRFilter(12)
#define PWMA_ETRFilter160()             PWMA_SetETRFilter(13)
#define PWMA_ETRFilter192()             PWMA_SetETRFilter(14)
#define PWMA_ETRFilter256()             PWMA_SetETRFilter(15)

#define HSPWMA_SetETRFilter(n)          MODIFY_HSPWMA(PWMA_ETR,  PWMAETR_ETF_MSK, ((n) << 0))
#define HSPWMA_ETRFilter1()             HSPWMA_SetETRFilter(0)
#define HSPWMA_ETRFilter2()             HSPWMA_SetETRFilter(1)
#define HSPWMA_ETRFilter4()             HSPWMA_SetETRFilter(2)
#define HSPWMA_ETRFilter8()             HSPWMA_SetETRFilter(3)
#define HSPWMA_ETRFilter12()            HSPWMA_SetETRFilter(4)
#define HSPWMA_ETRFilter16()            HSPWMA_SetETRFilter(5)
#define HSPWMA_ETRFilter24()            HSPWMA_SetETRFilter(6)
#define HSPWMA_ETRFilter32()            HSPWMA_SetETRFilter(7)
#define HSPWMA_ETRFilter48()            HSPWMA_SetETRFilter(8)
#define HSPWMA_ETRFilter64()            HSPWMA_SetETRFilter(9)
#define HSPWMA_ETRFilter80()            HSPWMA_SetETRFilter(10)
#define HSPWMA_ETRFilter96()            HSPWMA_SetETRFilter(11)
#define HSPWMA_ETRFilter128()           HSPWMA_SetETRFilter(12)
#define HSPWMA_ETRFilter160()           HSPWMA_SetETRFilter(13)
#define HSPWMA_ETRFilter192()           HSPWMA_SetETRFilter(14)
#define HSPWMA_ETRFilter256()           HSPWMA_SetETRFilter(15)

#define PWMAEGR_BG_MSK                  BIT7
#define PWMAEGR_TG_MSK                  BIT6
#define PWMAEGR_COMG_MSK                BIT5
#define PWMAEGR_CC4G_MSK                BIT4
#define PWMAEGR_CC3G_MSK                BIT3
#define PWMAEGR_CC2G_MSK                BIT2
#define PWMAEGR_CC1G_MSK                BIT1
#define PWMAEGR_UG_MSK                  BIT0
#define PWMA_GenerateBreakEvent()       SET_REG_BIT(PWMA_EGR, PWMAEGR_BG_MSK)
#define PWMA_GenerateTriggerEvent()     SET_REG_BIT(PWMA_EGR, PWMAEGR_TG_MSK)
#define PWMA_GenerateCOMEvent()         SET_REG_BIT(PWMA_EGR, PWMAEGR_COMG_MSK)
#define PWMA_GenerateCC4Event()         SET_REG_BIT(PWMA_EGR, PWMAEGR_CC4G_MSK)
#define PWMA_GenerateCC3Event()         SET_REG_BIT(PWMA_EGR, PWMAEGR_CC3G_MSK)
#define PWMA_GenerateCC2Event()         SET_REG_BIT(PWMA_EGR, PWMAEGR_CC2G_MSK)
#define PWMA_GenerateCC1Event()         SET_REG_BIT(PWMA_EGR, PWMAEGR_CC1G_MSK)
#define PWMA_GenerateUpdateEvent()      SET_REG_BIT(PWMA_EGR, PWMAEGR_UG_MSK)

#define HSPWMA_GenerateBreakEvent()     SET_HSPWMA_BIT(PWMA_EGR, PWMAEGR_BG_MSK)
#define HSPWMA_GenerateTriggerEvent()   SET_HSPWMA_BIT(PWMA_EGR, PWMAEGR_TG_MSK)
#define HSPWMA_GenerateCOMEvent()       SET_HSPWMA_BIT(PWMA_EGR, PWMAEGR_COMG_MSK)
#define HSPWMA_GenerateCC4Event()       SET_HSPWMA_BIT(PWMA_EGR, PWMAEGR_CC4G_MSK)
#define HSPWMA_GenerateCC3Event()       SET_HSPWMA_BIT(PWMA_EGR, PWMAEGR_CC3G_MSK)
#define HSPWMA_GenerateCC2Event()       SET_HSPWMA_BIT(PWMA_EGR, PWMAEGR_CC2G_MSK)
#define HSPWMA_GenerateCC1Event()       SET_HSPWMA_BIT(PWMA_EGR, PWMAEGR_CC1G_MSK)
#define HSPWMA_GenerateUpdateEvent()    SET_HSPWMA_BIT(PWMA_EGR, PWMAEGR_UG_MSK)

#define PWMACCMR1_OC1CE_MSK             BIT7
#define PWMA_EnableETRFClearOC1REF()    SET_REG_BIT(PWMA_CCMR1, PWMACCMR1_OC1CE_MSK)
#define PWMA_DisableETRFClearOC1REF()   CLR_REG_BIT(PWMA_CCMR1, PWMACCMR1_OC1CE_MSK)

#define HSPWMA_EnableETRFClearOC1REF()  SET_HSPWMA_BIT(PWMA_CCMR1, PWMACCMR1_OC1CE_MSK)
#define HSPWMA_DisableETRFClearOC1REF() CLR_HSPWMA_BIT(PWMA_CCMR1, PWMACCMR1_OC1CE_MSK)

#define PWMACCMR1_OC1M_MSK              (BIT6 | BIT5 | BIT4)
#define PWMA_SetOC1OutputMode(n)        MODIFY_REG(PWMA_CCMR1,  PWMACCMR1_OC1M_MSK, ((n) << 4))
#define PWMA_OC1REFFrozen()             PWMA_SetOC1OutputMode(0)
#define PWMA_OC1REFHighIfMatch()        PWMA_SetOC1OutputMode(1)
#define PWMA_OC1REFLowIfMatch()         PWMA_SetOC1OutputMode(2)
#define PWMA_OC1REFToggleIfMatch()      PWMA_SetOC1OutputMode(3)
#define PWMA_OC1REFForceLow()           PWMA_SetOC1OutputMode(4)
#define PWMA_OC1REFForceHigh()          PWMA_SetOC1OutputMode(5)
#define PWMA_OC1REFPWMMode1()           PWMA_SetOC1OutputMode(6)
#define PWMA_OC1REFPWMMode2()           PWMA_SetOC1OutputMode(7)

#define HSPWMA_SetOC1OutputMode(n)      MODIFY_HSPWMA(PWMA_CCMR1,  PWMACCMR1_OC1M_MSK, ((n) << 4))
#define HSPWMA_OC1REFFrozen()           HSPWMA_SetOC1OutputMode(0)
#define HSPWMA_OC1REFHighIfMatch()      HSPWMA_SetOC1OutputMode(1)
#define HSPWMA_OC1REFLowIfMatch()       HSPWMA_SetOC1OutputMode(2)
#define HSPWMA_OC1REFToggleIfMatch()    HSPWMA_SetOC1OutputMode(3)
#define HSPWMA_OC1REFForceLow()         HSPWMA_SetOC1OutputMode(4)
#define HSPWMA_OC1REFForceHigh()        HSPWMA_SetOC1OutputMode(5)
#define HSPWMA_OC1REFPWMMode1()         HSPWMA_SetOC1OutputMode(6)
#define HSPWMA_OC1REFPWMMode2()         HSPWMA_SetOC1OutputMode(7)

#define PWMACCMR1_OC1PE_MSK             BIT3
#define PWMA_UnbufferCCR1()             CLR_REG_BIT(PWMA_CCMR1, PWMACCMR1_OC1PE_MSK)
#define PWMA_BufferCCR1()               SET_REG_BIT(PWMA_CCMR1, PWMACCMR1_OC1PE_MSK)

#define HSPWMA_UnbufferCCR1()           CLR_HSPWMA_BIT(PWMA_CCMR1, PWMACCMR1_OC1PE_MSK)
#define HSPWMA_BufferCCR1()             SET_HSPWMA_BIT(PWMA_CCMR1, PWMACCMR1_OC1PE_MSK)

#define PWMACCMR1_OC1FE_MSK             BIT2
#define PWMA_DisableOC1FastMode()       CLR_REG_BIT(PWMA_CCMR1, PWMACCMR1_OC1FE_MSK)
#define PWMA_EnableOC1FastMode()        SET_REG_BIT(PWMA_CCMR1, PWMACCMR1_OC1FE_MSK)

#define HSPWMA_DisableOC1FastMode()     CLR_HSPWMA_BIT(PWMA_CCMR1, PWMACCMR1_OC1FE_MSK)
#define HSPWMA_EnableOC1FastMode()      SET_HSPWMA_BIT(PWMA_CCMR1, PWMACCMR1_OC1FE_MSK)

#define PWMACCMR1_CC1S_MSK              (BIT1 | BIT0)
#define PWMA_SetCC1Mode(n)              MODIFY_REG(PWMA_CCMR1,  PWMACCMR1_CC1S_MSK, ((n) << 0))
#define PWMA_CC1Output()                PWMA_SetCC1Mode(0)
#define PWMA_CC1CaptureTI1FP1()         PWMA_SetCC1Mode(1)
#define PWMA_CC1CaptureTI2FP1()         PWMA_SetCC1Mode(2)
#define PWMA_CC1CaptureTRC()            PWMA_SetCC1Mode(3)

#define HSPWMA_SetCC1Mode(n)            MODIFY_HSPWMA(PWMA_CCMR1,  PWMACCMR1_CC1S_MSK, ((n) << 0))
#define HSPWMA_CC1Output()              HSPWMA_SetCC1Mode(0)
#define HSPWMA_CC1CaptureTI1FP1()       HSPWMA_SetCC1Mode(1)
#define HSPWMA_CC1CaptureTI2FP1()       HSPWMA_SetCC1Mode(2)
#define HSPWMA_CC1CaptureTRC()          HSPWMA_SetCC1Mode(3)

#define PWMACCMR1_IC1F_MSK              (BIT_HN)
#define PWMA_SetIC1Filter(n)            MODIFY_REG(PWMA_CCMR1,  PWMACCMR1_IC1F_MSK, ((n) << 4))
#define PWMA_IC1Filter1()               PWMA_SetIC1Filter(0)
#define PWMA_IC1Filter2()               PWMA_SetIC1Filter(1)
#define PWMA_IC1Filter4()               PWMA_SetIC1Filter(2)
#define PWMA_IC1Filter8()               PWMA_SetIC1Filter(3)
#define PWMA_IC1Filter12()              PWMA_SetIC1Filter(4)
#define PWMA_IC1Filter16()              PWMA_SetIC1Filter(5)
#define PWMA_IC1Filter24()              PWMA_SetIC1Filter(6)
#define PWMA_IC1Filter32()              PWMA_SetIC1Filter(7)
#define PWMA_IC1Filter48()              PWMA_SetIC1Filter(8)
#define PWMA_IC1Filter64()              PWMA_SetIC1Filter(9)
#define PWMA_IC1Filter80()              PWMA_SetIC1Filter(10)
#define PWMA_IC1Filter96()              PWMA_SetIC1Filter(11)
#define PWMA_IC1Filter128()             PWMA_SetIC1Filter(12)
#define PWMA_IC1Filter160()             PWMA_SetIC1Filter(13)
#define PWMA_IC1Filter192()             PWMA_SetIC1Filter(14)
#define PWMA_IC1Filter256()             PWMA_SetIC1Filter(15)

#define HSPWMA_SetIC1Filter(n)          MODIFY_HSPWMA(PWMA_CCMR1,  PWMACCMR1_IC1F_MSK, ((n) << 4))
#define HSPWMA_IC1Filter1()             HSPWMA_SetIC1Filter(0)
#define HSPWMA_IC1Filter2()             HSPWMA_SetIC1Filter(1)
#define HSPWMA_IC1Filter4()             HSPWMA_SetIC1Filter(2)
#define HSPWMA_IC1Filter8()             HSPWMA_SetIC1Filter(3)
#define HSPWMA_IC1Filter12()            HSPWMA_SetIC1Filter(4)
#define HSPWMA_IC1Filter16()            HSPWMA_SetIC1Filter(5)
#define HSPWMA_IC1Filter24()            HSPWMA_SetIC1Filter(6)
#define HSPWMA_IC1Filter32()            HSPWMA_SetIC1Filter(7)
#define HSPWMA_IC1Filter48()            HSPWMA_SetIC1Filter(8)
#define HSPWMA_IC1Filter64()            HSPWMA_SetIC1Filter(9)
#define HSPWMA_IC1Filter80()            HSPWMA_SetIC1Filter(10)
#define HSPWMA_IC1Filter96()            HSPWMA_SetIC1Filter(11)
#define HSPWMA_IC1Filter128()           HSPWMA_SetIC1Filter(12)
#define HSPWMA_IC1Filter160()           HSPWMA_SetIC1Filter(13)
#define HSPWMA_IC1Filter192()           HSPWMA_SetIC1Filter(14)
#define HSPWMA_IC1Filter256()           HSPWMA_SetIC1Filter(15)

#define PWMACCMR1_IC1PS_MSK             (BIT3 | BIT2)
#define PWMA_SetIC1Divider(n)           MODIFY_REG(PWMA_CCMR1,  PWMACCMR1_IC1PS_MSK, ((n) << 2))
#define PWMA_IC1Divider1()              PWMA_SetIC1Divider(0)
#define PWMA_IC1Divider2()              PWMA_SetIC1Divider(1)
#define PWMA_IC1Divider4()              PWMA_SetIC1Divider(2)
#define PWMA_IC1Divider8()              PWMA_SetIC1Divider(3)

#define HSPWMA_SetIC1Divider(n)         MODIFY_HSPWMA(PWMA_CCMR1,  PWMACCMR1_IC1PS_MSK, ((n) << 2))
#define HSPWMA_IC1Divider1()            HSPWMA_SetIC1Divider(0)
#define HSPWMA_IC1Divider2()            HSPWMA_SetIC1Divider(1)
#define HSPWMA_IC1Divider4()            HSPWMA_SetIC1Divider(2)
#define HSPWMA_IC1Divider8()            HSPWMA_SetIC1Divider(3)

#define PWMACCMR2_OC2CE_MSK             BIT7
#define PWMA_EnableETRFClearOC2REF()    SET_REG_BIT(PWMA_CCMR2, PWMACCMR2_OC2CE_MSK)
#define PWMA_DisableETRFClearOC2REF()   CLR_REG_BIT(PWMA_CCMR2, PWMACCMR2_OC2CE_MSK)

#define HSPWMA_EnableETRFClearOC2REF()  SET_HSPWMA_BIT(PWMA_CCMR2, PWMACCMR2_OC2CE_MSK)
#define HSPWMA_DisableETRFClearOC2REF() CLR_HSPWMA_BIT(PWMA_CCMR2, PWMACCMR2_OC2CE_MSK)

#define PWMACCMR2_OC2M_MSK              (BIT6 | BIT5 | BIT4)
#define PWMA_SetOC2OutputMode(n)        MODIFY_REG(PWMA_CCMR2,  PWMACCMR2_OC2M_MSK, ((n) << 4))
#define PWMA_OC2REFFrozen()             PWMA_SetOC2OutputMode(0)
#define PWMA_OC2REFHighIfMatch()        PWMA_SetOC2OutputMode(1)
#define PWMA_OC2REFLowIfMatch()         PWMA_SetOC2OutputMode(2)
#define PWMA_OC2REFToggleIfMatch()      PWMA_SetOC2OutputMode(3)
#define PWMA_OC2REFForceLow()           PWMA_SetOC2OutputMode(4)
#define PWMA_OC2REFForceHigh()          PWMA_SetOC2OutputMode(5)
#define PWMA_OC2REFPWMMode1()           PWMA_SetOC2OutputMode(6)
#define PWMA_OC2REFPWMMode2()           PWMA_SetOC2OutputMode(7)

#define HSPWMA_SetOC2OutputMode(n)      MODIFY_HSPWMA(PWMA_CCMR2,  PWMACCMR2_OC2M_MSK, ((n) << 4))
#define HSPWMA_OC2REFFrozen()           HSPWMA_SetOC2OutputMode(0)
#define HSPWMA_OC2REFHighIfMatch()      HSPWMA_SetOC2OutputMode(1)
#define HSPWMA_OC2REFLowIfMatch()       HSPWMA_SetOC2OutputMode(2)
#define HSPWMA_OC2REFToggleIfMatch()    HSPWMA_SetOC2OutputMode(3)
#define HSPWMA_OC2REFForceLow()         HSPWMA_SetOC2OutputMode(4)
#define HSPWMA_OC2REFForceHigh()        HSPWMA_SetOC2OutputMode(5)
#define HSPWMA_OC2REFPWMMode1()         HSPWMA_SetOC2OutputMode(6)
#define HSPWMA_OC2REFPWMMode2()         HSPWMA_SetOC2OutputMode(7)

#define PWMACCMR2_OC2PE_MSK             BIT3
#define PWMA_UnbufferCCR2()             CLR_REG_BIT(PWMA_CCMR2, PWMACCMR2_OC2PE_MSK)
#define PWMA_BufferCCR2()               SET_REG_BIT(PWMA_CCMR2, PWMACCMR2_OC2PE_MSK)

#define HSPWMA_UnbufferCCR2()           CLR_HSPWMA_BIT(PWMA_CCMR2, PWMACCMR2_OC2PE_MSK)
#define HSPWMA_BufferCCR2()             SET_HSPWMA_BIT(PWMA_CCMR2, PWMACCMR2_OC2PE_MSK)

#define PWMACCMR2_OC2FE_MSK             BIT2
#define PWMA_DisableOC2FastMode()       CLR_REG_BIT(PWMA_CCMR2, PWMACCMR2_OC2FE_MSK)
#define PWMA_EnableOC2FastMode()        SET_REG_BIT(PWMA_CCMR2, PWMACCMR2_OC2FE_MSK)

#define HSPWMA_DisableOC2FastMode()     CLR_HSPWMA_BIT(PWMA_CCMR2, PWMACCMR2_OC2FE_MSK)
#define HSPWMA_EnableOC2FastMode()      SET_HSPWMA_BIT(PWMA_CCMR2, PWMACCMR2_OC2FE_MSK)

#define PWMACCMR2_CC2S_MSK              (BIT1 | BIT0)
#define PWMA_SetCC2Mode(n)              MODIFY_REG(PWMA_CCMR2,  PWMACCMR2_CC2S_MSK, ((n) << 0))
#define PWMA_CC2Output()                PWMA_SetCC2Mode(0)
#define PWMA_CC2CaptureTI2FP2()         PWMA_SetCC2Mode(1)
#define PWMA_CC2CaptureTI1FP2()         PWMA_SetCC2Mode(2)
#define PWMA_CC2CaptureTRC()            PWMA_SetCC2Mode(3)

#define HSPWMA_SetCC2Mode(n)            MODIFY_HSPWMA(PWMA_CCMR2,  PWMACCMR2_CC2S_MSK, ((n) << 0))
#define HSPWMA_CC2Output()              HSPWMA_SetCC2Mode(0)
#define HSPWMA_CC2CaptureTI2FP2()       HSPWMA_SetCC2Mode(1)
#define HSPWMA_CC2CaptureTI1FP2()       HSPWMA_SetCC2Mode(2)
#define HSPWMA_CC2CaptureTRC()          HSPWMA_SetCC2Mode(3)

#define PWMACCMR2_IC2F_MSK              (BIT_HN)
#define PWMA_SetIC2Filter(n)            MODIFY_REG(PWMA_CCMR2,  PWMACCMR2_IC2F_MSK, ((n) << 4))
#define PWMA_IC2Filter1()               PWMA_SetIC2Filter(0)
#define PWMA_IC2Filter2()               PWMA_SetIC2Filter(1)
#define PWMA_IC2Filter4()               PWMA_SetIC2Filter(2)
#define PWMA_IC2Filter8()               PWMA_SetIC2Filter(3)
#define PWMA_IC2Filter12()              PWMA_SetIC2Filter(4)
#define PWMA_IC2Filter16()              PWMA_SetIC2Filter(5)
#define PWMA_IC2Filter24()              PWMA_SetIC2Filter(6)
#define PWMA_IC2Filter32()              PWMA_SetIC2Filter(7)
#define PWMA_IC2Filter48()              PWMA_SetIC2Filter(8)
#define PWMA_IC2Filter64()              PWMA_SetIC2Filter(9)
#define PWMA_IC2Filter80()              PWMA_SetIC2Filter(10)
#define PWMA_IC2Filter96()              PWMA_SetIC2Filter(11)
#define PWMA_IC2Filter128()             PWMA_SetIC2Filter(12)
#define PWMA_IC2Filter160()             PWMA_SetIC2Filter(13)
#define PWMA_IC2Filter192()             PWMA_SetIC2Filter(14)
#define PWMA_IC2Filter256()             PWMA_SetIC2Filter(15)

#define HSPWMA_SetIC2Filter(n)          MODIFY_HSPWMA(PWMA_CCMR2,  PWMACCMR2_IC2F_MSK, ((n) << 4))
#define HSPWMA_IC2Filter1()             HSPWMA_SetIC2Filter(0)
#define HSPWMA_IC2Filter2()             HSPWMA_SetIC2Filter(1)
#define HSPWMA_IC2Filter4()             HSPWMA_SetIC2Filter(2)
#define HSPWMA_IC2Filter8()             HSPWMA_SetIC2Filter(3)
#define HSPWMA_IC2Filter12()            HSPWMA_SetIC2Filter(4)
#define HSPWMA_IC2Filter16()            HSPWMA_SetIC2Filter(5)
#define HSPWMA_IC2Filter24()            HSPWMA_SetIC2Filter(6)
#define HSPWMA_IC2Filter32()            HSPWMA_SetIC2Filter(7)
#define HSPWMA_IC2Filter48()            HSPWMA_SetIC2Filter(8)
#define HSPWMA_IC2Filter64()            HSPWMA_SetIC2Filter(9)
#define HSPWMA_IC2Filter80()            HSPWMA_SetIC2Filter(10)
#define HSPWMA_IC2Filter96()            HSPWMA_SetIC2Filter(11)
#define HSPWMA_IC2Filter128()           HSPWMA_SetIC2Filter(12)
#define HSPWMA_IC2Filter160()           HSPWMA_SetIC2Filter(13)
#define HSPWMA_IC2Filter192()           HSPWMA_SetIC2Filter(14)
#define HSPWMA_IC2Filter256()           HSPWMA_SetIC2Filter(15)

#define PWMACCMR2_IC2PS_MSK             (BIT3 | BIT2)
#define PWMA_SetIC2Divider(n)           MODIFY_REG(PWMA_CCMR2,  PWMACCMR2_IC2PS_MSK, ((n) << 2))
#define PWMA_IC2Divider1()              PWMA_SetIC2Divider(0)
#define PWMA_IC2Divider2()              PWMA_SetIC2Divider(1)
#define PWMA_IC2Divider4()              PWMA_SetIC2Divider(2)
#define PWMA_IC2Divider8()              PWMA_SetIC2Divider(3)

#define HSPWMA_SetIC2Divider(n)         MODIFY_HSPWMA(PWMA_CCMR2,  PWMACCMR2_IC2PS_MSK, ((n) << 2))
#define HSPWMA_IC2Divider1()            HSPWMA_SetIC2Divider(0)
#define HSPWMA_IC2Divider2()            HSPWMA_SetIC2Divider(1)
#define HSPWMA_IC2Divider4()            HSPWMA_SetIC2Divider(2)
#define HSPWMA_IC2Divider8()            HSPWMA_SetIC2Divider(3)

#define PWMACCMR3_OC3CE_MSK             BIT7
#define PWMA_EnableETRFClearOC3REF()    SET_REG_BIT(PWMA_CCMR3, PWMACCMR3_OC3CE_MSK)
#define PWMA_DisableETRFClearOC3REF()   CLR_REG_BIT(PWMA_CCMR3, PWMACCMR3_OC3CE_MSK)

#define HSPWMA_EnableETRFClearOC3REF()  SET_HSPWMA_BIT(PWMA_CCMR3, PWMACCMR3_OC3CE_MSK)
#define HSPWMA_DisableETRFClearOC3REF() CLR_HSPWMA_BIT(PWMA_CCMR3, PWMACCMR3_OC3CE_MSK)

#define PWMACCMR3_OC3M_MSK              (BIT6 | BIT5 | BIT4)
#define PWMA_SetOC3OutputMode(n)        MODIFY_REG(PWMA_CCMR3,  PWMACCMR3_OC3M_MSK, ((n) << 4))
#define PWMA_OC3REFFrozen()             PWMA_SetOC3OutputMode(0)
#define PWMA_OC3REFHighIfMatch()        PWMA_SetOC3OutputMode(1)
#define PWMA_OC3REFLowIfMatch()         PWMA_SetOC3OutputMode(2)
#define PWMA_OC3REFToggleIfMatch()      PWMA_SetOC3OutputMode(3)
#define PWMA_OC3REFForceLow()           PWMA_SetOC3OutputMode(4)
#define PWMA_OC3REFForceHigh()          PWMA_SetOC3OutputMode(5)
#define PWMA_OC3REFPWMMode1()           PWMA_SetOC3OutputMode(6)
#define PWMA_OC3REFPWMMode2()           PWMA_SetOC3OutputMode(7)

#define HSPWMA_SetOC3OutputMode(n)      MODIFY_HSPWMA(PWMA_CCMR3,  PWMACCMR3_OC3M_MSK, ((n) << 4))
#define HSPWMA_OC3REFFrozen()           HSPWMA_SetOC3OutputMode(0)
#define HSPWMA_OC3REFHighIfMatch()      HSPWMA_SetOC3OutputMode(1)
#define HSPWMA_OC3REFLowIfMatch()       HSPWMA_SetOC3OutputMode(2)
#define HSPWMA_OC3REFToggleIfMatch()    HSPWMA_SetOC3OutputMode(3)
#define HSPWMA_OC3REFForceLow()         HSPWMA_SetOC3OutputMode(4)
#define HSPWMA_OC3REFForceHigh()        HSPWMA_SetOC3OutputMode(5)
#define HSPWMA_OC3REFPWMMode1()         HSPWMA_SetOC3OutputMode(6)
#define HSPWMA_OC3REFPWMMode2()         HSPWMA_SetOC3OutputMode(7)

#define PWMACCMR3_OC3PE_MSK             BIT3
#define PWMA_UnbufferCCR3()             CLR_REG_BIT(PWMA_CCMR3, PWMACCMR3_OC3PE_MSK)
#define PWMA_BufferCCR3()               SET_REG_BIT(PWMA_CCMR3, PWMACCMR3_OC3PE_MSK)

#define HSPWMA_UnbufferCCR3()           CLR_HSPWMA_BIT(PWMA_CCMR3, PWMACCMR3_OC3PE_MSK)
#define HSPWMA_BufferCCR3()             SET_HSPWMA_BIT(PWMA_CCMR3, PWMACCMR3_OC3PE_MSK)

#define PWMACCMR3_OC3FE_MSK             BIT2
#define PWMA_DisableOC3FastMode()       CLR_REG_BIT(PWMA_CCMR3, PWMACCMR3_OC3FE_MSK)
#define PWMA_EnableOC3FastMode()        SET_REG_BIT(PWMA_CCMR3, PWMACCMR3_OC3FE_MSK)

#define HSPWMA_DisableOC3FastMode()     CLR_HSPWMA_BIT(PWMA_CCMR3, PWMACCMR3_OC3FE_MSK)
#define HSPWMA_EnableOC3FastMode()      SET_HSPWMA_BIT(PWMA_CCMR3, PWMACCMR3_OC3FE_MSK)

#define PWMACCMR3_CC3S_MSK              (BIT1 | BIT0)
#define PWMA_SetCC3Mode(n)              MODIFY_REG(PWMA_CCMR3,  PWMACCMR3_CC3S_MSK, ((n) << 0))
#define PWMA_CC3Output()                PWMA_SetCC3Mode(0)
#define PWMA_CC3CaptureTI3FP3()         PWMA_SetCC3Mode(1)
#define PWMA_CC3CaptureTI4FP3()         PWMA_SetCC3Mode(2)
#define PWMA_CC3CaptureTRC()            PWMA_SetCC3Mode(3)

#define HSPWMA_SetCC3Mode(n)            MODIFY_HSPWMA(PWMA_CCMR3,  PWMACCMR3_CC3S_MSK, ((n) << 0))
#define HSPWMA_CC3Output()              HSPWMA_SetCC3Mode(0)
#define HSPWMA_CC3CaptureTI3FP3()       HSPWMA_SetCC3Mode(1)
#define HSPWMA_CC3CaptureTI4FP3()       HSPWMA_SetCC3Mode(2)
#define HSPWMA_CC3CaptureTRC()          HSPWMA_SetCC3Mode(3)

#define PWMACCMR3_IC3F_MSK              (BIT_HN)
#define PWMA_SetIC3Filter(n)            MODIFY_REG(PWMA_CCMR3,  PWMACCMR3_IC3F_MSK, ((n) << 4))
#define PWMA_IC3Filter1()               PWMA_SetIC3Filter(0)
#define PWMA_IC3Filter2()               PWMA_SetIC3Filter(1)
#define PWMA_IC3Filter4()               PWMA_SetIC3Filter(2)
#define PWMA_IC3Filter8()               PWMA_SetIC3Filter(3)
#define PWMA_IC3Filter12()              PWMA_SetIC3Filter(4)
#define PWMA_IC3Filter16()              PWMA_SetIC3Filter(5)
#define PWMA_IC3Filter24()              PWMA_SetIC3Filter(6)
#define PWMA_IC3Filter32()              PWMA_SetIC3Filter(7)
#define PWMA_IC3Filter48()              PWMA_SetIC3Filter(8)
#define PWMA_IC3Filter64()              PWMA_SetIC3Filter(9)
#define PWMA_IC3Filter80()              PWMA_SetIC3Filter(10)
#define PWMA_IC3Filter96()              PWMA_SetIC3Filter(11)
#define PWMA_IC3Filter128()             PWMA_SetIC3Filter(12)
#define PWMA_IC3Filter160()             PWMA_SetIC3Filter(13)
#define PWMA_IC3Filter192()             PWMA_SetIC3Filter(14)
#define PWMA_IC3Filter256()             PWMA_SetIC3Filter(15)

#define HSPWMA_SetIC3Filter(n)          MODIFY_HSPWMA(PWMA_CCMR3,  PWMACCMR3_IC3F_MSK, ((n) << 4))
#define HSPWMA_IC3Filter1()             HSPWMA_SetIC3Filter(0)
#define HSPWMA_IC3Filter2()             HSPWMA_SetIC3Filter(1)
#define HSPWMA_IC3Filter4()             HSPWMA_SetIC3Filter(2)
#define HSPWMA_IC3Filter8()             HSPWMA_SetIC3Filter(3)
#define HSPWMA_IC3Filter12()            HSPWMA_SetIC3Filter(4)
#define HSPWMA_IC3Filter16()            HSPWMA_SetIC3Filter(5)
#define HSPWMA_IC3Filter24()            HSPWMA_SetIC3Filter(6)
#define HSPWMA_IC3Filter32()            HSPWMA_SetIC3Filter(7)
#define HSPWMA_IC3Filter48()            HSPWMA_SetIC3Filter(8)
#define HSPWMA_IC3Filter64()            HSPWMA_SetIC3Filter(9)
#define HSPWMA_IC3Filter80()            HSPWMA_SetIC3Filter(10)
#define HSPWMA_IC3Filter96()            HSPWMA_SetIC3Filter(11)
#define HSPWMA_IC3Filter128()           HSPWMA_SetIC3Filter(12)
#define HSPWMA_IC3Filter160()           HSPWMA_SetIC3Filter(13)
#define HSPWMA_IC3Filter192()           HSPWMA_SetIC3Filter(14)
#define HSPWMA_IC3Filter256()           HSPWMA_SetIC3Filter(15)

#define PWMACCMR3_IC3PS_MSK             (BIT3 | BIT2)
#define PWMA_SetIC3Divider(n)           MODIFY_REG(PWMA_CCMR3,  PWMACCMR3_IC3PS_MSK, ((n) << 2))
#define PWMA_IC3Divider1()              PWMA_SetIC3Divider(0)
#define PWMA_IC3Divider2()              PWMA_SetIC3Divider(1)
#define PWMA_IC3Divider4()              PWMA_SetIC3Divider(2)
#define PWMA_IC3Divider8()              PWMA_SetIC3Divider(3)

#define HSPWMA_SetIC3Divider(n)         MODIFY_HSPWMA(PWMA_CCMR3,  PWMACCMR3_IC3PS_MSK, ((n) << 2))
#define HSPWMA_IC3Divider1()            HSPWMA_SetIC3Divider(0)
#define HSPWMA_IC3Divider2()            HSPWMA_SetIC3Divider(1)
#define HSPWMA_IC3Divider4()            HSPWMA_SetIC3Divider(2)
#define HSPWMA_IC3Divider8()            HSPWMA_SetIC3Divider(3)

#define PWMACCMR4_OC4CE_MSK             BIT7
#define PWMA_EnableETRFClearOC4REF()    SET_REG_BIT(PWMA_CCMR4, PWMACCMR4_OC4CE_MSK)
#define PWMA_DisableETRFClearOC4REF()   CLR_REG_BIT(PWMA_CCMR4, PWMACCMR4_OC4CE_MSK)

#define HSPWMA_EnableETRFClearOC4REF()  SET_HSPWMA_BIT(PWMA_CCMR4, PWMACCMR4_OC4CE_MSK)
#define HSPWMA_DisableETRFClearOC4REF() CLR_HSPWMA_BIT(PWMA_CCMR4, PWMACCMR4_OC4CE_MSK)

#define PWMACCMR4_OC4M_MSK              (BIT6 | BIT5 | BIT4)
#define PWMA_SetOC4OutputMode(n)        MODIFY_REG(PWMA_CCMR4,  PWMACCMR4_OC4M_MSK, ((n) << 4))
#define PWMA_OC4REFFrozen()             PWMA_SetOC4OutputMode(0)
#define PWMA_OC4REFHighIfMatch()        PWMA_SetOC4OutputMode(1)
#define PWMA_OC4REFLowIfMatch()         PWMA_SetOC4OutputMode(2)
#define PWMA_OC4REFToggleIfMatch()      PWMA_SetOC4OutputMode(3)
#define PWMA_OC4REFForceLow()           PWMA_SetOC4OutputMode(4)
#define PWMA_OC4REFForceHigh()          PWMA_SetOC4OutputMode(5)
#define PWMA_OC4REFPWMMode1()           PWMA_SetOC4OutputMode(6)
#define PWMA_OC4REFPWMMode2()           PWMA_SetOC4OutputMode(7)

#define HSPWMA_SetOC4OutputMode(n)      MODIFY_HSPWMA(PWMA_CCMR4,  PWMACCMR4_OC4M_MSK, ((n) << 4))
#define HSPWMA_OC4REFFrozen()           HSPWMA_SetOC4OutputMode(0)
#define HSPWMA_OC4REFHighIfMatch()      HSPWMA_SetOC4OutputMode(1)
#define HSPWMA_OC4REFLowIfMatch()       HSPWMA_SetOC4OutputMode(2)
#define HSPWMA_OC4REFToggleIfMatch()    HSPWMA_SetOC4OutputMode(3)
#define HSPWMA_OC4REFForceLow()         HSPWMA_SetOC4OutputMode(4)
#define HSPWMA_OC4REFForceHigh()        HSPWMA_SetOC4OutputMode(5)
#define HSPWMA_OC4REFPWMMode1()         HSPWMA_SetOC4OutputMode(6)
#define HSPWMA_OC4REFPWMMode2()         HSPWMA_SetOC4OutputMode(7)

#define PWMACCMR4_OC4PE_MSK             BIT3
#define PWMA_UnbufferCCR4()             CLR_REG_BIT(PWMA_CCMR4, PWMACCMR4_OC4PE_MSK)
#define PWMA_BufferCCR4()               SET_REG_BIT(PWMA_CCMR4, PWMACCMR4_OC4PE_MSK)

#define HSPWMA_UnbufferCCR4()           CLR_HSPWMA_BIT(PWMA_CCMR4, PWMACCMR4_OC4PE_MSK)
#define HSPWMA_BufferCCR4()             SET_HSPWMA_BIT(PWMA_CCMR4, PWMACCMR4_OC4PE_MSK)

#define PWMACCMR4_OC4FE_MSK             BIT2
#define PWMA_DisableOC4FastMode()       CLR_REG_BIT(PWMA_CCMR4, PWMACCMR4_OC4FE_MSK)
#define PWMA_EnableOC4FastMode()        SET_REG_BIT(PWMA_CCMR4, PWMACCMR4_OC4FE_MSK)

#define HSPWMA_DisableOC4FastMode()     CLR_HSPWMA_BIT(PWMA_CCMR4, PWMACCMR4_OC4FE_MSK)
#define HSPWMA_EnableOC4FastMode()      SET_HSPWMA_BIT(PWMA_CCMR4, PWMACCMR4_OC4FE_MSK)

#define PWMACCMR4_CC4S_MSK              (BIT1 | BIT0)
#define PWMA_SetCC4Mode(n)              MODIFY_REG(PWMA_CCMR4,  PWMACCMR4_CC4S_MSK, ((n) << 0))
#define PWMA_CC4Output()                PWMA_SetCC4Mode(0)
#define PWMA_CC4CaptureTI4FP4()         PWMA_SetCC4Mode(1)
#define PWMA_CC4CaptureTI3FP4()         PWMA_SetCC4Mode(2)
#define PWMA_CC4CaptureTRC()            PWMA_SetCC4Mode(3)

#define HSPWMA_SetCC4Mode(n)            MODIFY_HSPWMA(PWMA_CCMR4,  PWMACCMR4_CC4S_MSK, ((n) << 0))
#define HSPWMA_CC4Output()              HSPWMA_SetCC4Mode(0)
#define HSPWMA_CC4CaptureTI4FP4()       HSPWMA_SetCC4Mode(1)
#define HSPWMA_CC4CaptureTI3FP4()       HSPWMA_SetCC4Mode(2)
#define HSPWMA_CC4CaptureTRC()          HSPWMA_SetCC4Mode(3)

#define PWMACCMR4_IC4F_MSK              (BIT_HN)
#define PWMA_SetIC4Filter(n)            MODIFY_REG(PWMA_CCMR4,  PWMACCMR4_IC4F_MSK, ((n) << 4))
#define PWMA_IC4Filter1()               PWMA_SetIC4Filter(0)
#define PWMA_IC4Filter2()               PWMA_SetIC4Filter(1)
#define PWMA_IC4Filter4()               PWMA_SetIC4Filter(2)
#define PWMA_IC4Filter8()               PWMA_SetIC4Filter(3)
#define PWMA_IC4Filter12()              PWMA_SetIC4Filter(4)
#define PWMA_IC4Filter16()              PWMA_SetIC4Filter(5)
#define PWMA_IC4Filter24()              PWMA_SetIC4Filter(6)
#define PWMA_IC4Filter32()              PWMA_SetIC4Filter(7)
#define PWMA_IC4Filter48()              PWMA_SetIC4Filter(8)
#define PWMA_IC4Filter64()              PWMA_SetIC4Filter(9)
#define PWMA_IC4Filter80()              PWMA_SetIC4Filter(10)
#define PWMA_IC4Filter96()              PWMA_SetIC4Filter(11)
#define PWMA_IC4Filter128()             PWMA_SetIC4Filter(12)
#define PWMA_IC4Filter160()             PWMA_SetIC4Filter(13)
#define PWMA_IC4Filter192()             PWMA_SetIC4Filter(14)
#define PWMA_IC4Filter256()             PWMA_SetIC4Filter(15)

#define HSPWMA_SetIC4Filter(n)          MODIFY_HSPWMA(PWMA_CCMR4,  PWMACCMR4_IC4F_MSK, ((n) << 4))
#define HSPWMA_IC4Filter1()             HSPWMA_SetIC4Filter(0)
#define HSPWMA_IC4Filter2()             HSPWMA_SetIC4Filter(1)
#define HSPWMA_IC4Filter4()             HSPWMA_SetIC4Filter(2)
#define HSPWMA_IC4Filter8()             HSPWMA_SetIC4Filter(3)
#define HSPWMA_IC4Filter12()            HSPWMA_SetIC4Filter(4)
#define HSPWMA_IC4Filter16()            HSPWMA_SetIC4Filter(5)
#define HSPWMA_IC4Filter24()            HSPWMA_SetIC4Filter(6)
#define HSPWMA_IC4Filter32()            HSPWMA_SetIC4Filter(7)
#define HSPWMA_IC4Filter48()            HSPWMA_SetIC4Filter(8)
#define HSPWMA_IC4Filter64()            HSPWMA_SetIC4Filter(9)
#define HSPWMA_IC4Filter80()            HSPWMA_SetIC4Filter(10)
#define HSPWMA_IC4Filter96()            HSPWMA_SetIC4Filter(11)
#define HSPWMA_IC4Filter128()           HSPWMA_SetIC4Filter(12)
#define HSPWMA_IC4Filter160()           HSPWMA_SetIC4Filter(13)
#define HSPWMA_IC4Filter192()           HSPWMA_SetIC4Filter(14)
#define HSPWMA_IC4Filter256()           HSPWMA_SetIC4Filter(15)

#define PWMACCMR4_IC4PS_MSK             (BIT3 | BIT2)
#define PWMA_SetIC4Divider(n)           MODIFY_REG(PWMA_CCMR4,  PWMACCMR4_IC4PS_MSK, ((n) << 2))
#define PWMA_IC4Divider1()              PWMA_SetIC4Divider(0)
#define PWMA_IC4Divider2()              PWMA_SetIC4Divider(1)
#define PWMA_IC4Divider4()              PWMA_SetIC4Divider(2)
#define PWMA_IC4Divider8()              PWMA_SetIC4Divider(3)

#define HSPWMA_SetIC4Divider(n)         MODIFY_HSPWMA(PWMA_CCMR4,  PWMACCMR4_IC4PS_MSK, ((n) << 2))
#define HSPWMA_IC4Divider1()            HSPWMA_SetIC4Divider(0)
#define HSPWMA_IC4Divider2()            HSPWMA_SetIC4Divider(1)
#define HSPWMA_IC4Divider4()            HSPWMA_SetIC4Divider(2)
#define HSPWMA_IC4Divider8()            HSPWMA_SetIC4Divider(3)

#define PWMACCER1_CC1E_MSK              BIT0
#define PWMA_CC1PEnable()               SET_REG_BIT(PWMA_CCER1, PWMACCER1_CC1E_MSK)
#define PWMA_CC1PDisable()              CLR_REG_BIT(PWMA_CCER1, PWMACCER1_CC1E_MSK)

#define HSPWMA_CC1PEnable()             SET_HSPWMA_BIT(PWMA_CCER1, PWMACCER1_CC1E_MSK)
#define HSPWMA_CC1PDisable()            CLR_HSPWMA_BIT(PWMA_CCER1, PWMACCER1_CC1E_MSK)

#define PWMACCER1_CC1P_MSK              BIT1
#define PWMA_CC1PNonInverted()          CLR_REG_BIT(PWMA_CCER1, PWMACCER1_CC1P_MSK)
#define PWMA_CC1PInverted()             SET_REG_BIT(PWMA_CCER1, PWMACCER1_CC1P_MSK)

#define HSPWMA_CC1PNonInverted()        CLR_HSPWMA_BIT(PWMA_CCER1, PWMACCER1_CC1P_MSK)
#define HSPWMA_CC1PInverted()           SET_HSPWMA_BIT(PWMA_CCER1, PWMACCER1_CC1P_MSK)

#define PWMACCER1_CC1NE_MSK             BIT2
#define PWMA_CC1NEnable()               SET_REG_BIT(PWMA_CCER1, PWMACCER1_CC1NE_MSK)
#define PWMA_CC1NDisable()              CLR_REG_BIT(PWMA_CCER1, PWMACCER1_CC1NE_MSK)

#define HSPWMA_CC1NEnable()             SET_HSPWMA_BIT(PWMA_CCER1, PWMACCER1_CC1NE_MSK)
#define HSPWMA_CC1NDisable()            CLR_HSPWMA_BIT(PWMA_CCER1, PWMACCER1_CC1NE_MSK)

#define PWMACCER1_CC1NP_MSK             BIT3
#define PWMA_CC1NNonInverted()          CLR_REG_BIT(PWMA_CCER1, PWMACCER1_CC1NP_MSK)
#define PWMA_CC1NInverted()             SET_REG_BIT(PWMA_CCER1, PWMACCER1_CC1NP_MSK)

#define HSPWMA_CC1NNonInverted()        CLR_HSPWMA_BIT(PWMA_CCER1, PWMACCER1_CC1NP_MSK)
#define HSPWMA_CC1NInverted()           SET_HSPWMA_BIT(PWMA_CCER1, PWMACCER1_CC1NP_MSK)

#define PWMACCER1_CC2E_MSK              BIT4
#define PWMA_CC2PEnable()               SET_REG_BIT(PWMA_CCER1, PWMACCER1_CC2E_MSK)
#define PWMA_CC2PDisable()              CLR_REG_BIT(PWMA_CCER1, PWMACCER1_CC2E_MSK)

#define HSPWMA_CC2PEnable()             SET_HSPWMA_BIT(PWMA_CCER1, PWMACCER1_CC2E_MSK)
#define HSPWMA_CC2PDisable()            CLR_HSPWMA_BIT(PWMA_CCER1, PWMACCER1_CC2E_MSK)

#define PWMACCER1_CC2P_MSK              BIT5
#define PWMA_CC2PNonInverted()          CLR_REG_BIT(PWMA_CCER1, PWMACCER1_CC2P_MSK)
#define PWMA_CC2PInverted()             SET_REG_BIT(PWMA_CCER1, PWMACCER1_CC2P_MSK)

#define HSPWMA_CC2PNonInverted()        CLR_HSPWMA_BIT(PWMA_CCER1, PWMACCER1_CC2P_MSK)
#define HSPWMA_CC2PInverted()           SET_HSPWMA_BIT(PWMA_CCER1, PWMACCER1_CC2P_MSK)

#define PWMACCER1_CC2NE_MSK             BIT6
#define PWMA_CC2NEnable()               SET_REG_BIT(PWMA_CCER1, PWMACCER1_CC2NE_MSK)
#define PWMA_CC2NDisable()              CLR_REG_BIT(PWMA_CCER1, PWMACCER1_CC2NE_MSK)

#define HSPWMA_CC2NEnable()             SET_HSPWMA_BIT(PWMA_CCER1, PWMACCER1_CC2NE_MSK)
#define HSPWMA_CC2NDisable()            CLR_HSPWMA_BIT(PWMA_CCER1, PWMACCER1_CC2NE_MSK)

#define PWMACCER1_CC2NP_MSK             BIT7
#define PWMA_CC2NNonInverted()          CLR_REG_BIT(PWMA_CCER1, PWMACCER1_CC2NP_MSK)
#define PWMA_CC2NInverted()             SET_REG_BIT(PWMA_CCER1, PWMACCER1_CC2NP_MSK)

#define HSPWMA_CC2NNonInverted()        CLR_HSPWMA_BIT(PWMA_CCER1, PWMACCER1_CC2NP_MSK)
#define HSPWMA_CC2NInverted()           SET_HSPWMA_BIT(PWMA_CCER1, PWMACCER1_CC2NP_MSK)

#define PWMACCER2_CC3E_MSK              BIT0
#define PWMA_CC3PEnable()               SET_REG_BIT(PWMA_CCER2, PWMACCER2_CC3E_MSK)
#define PWMA_CC3PDisable()              CLR_REG_BIT(PWMA_CCER2, PWMACCER2_CC3E_MSK)

#define HSPWMA_CC3PEnable()             SET_HSPWMA_BIT(PWMA_CCER2, PWMACCER2_CC3E_MSK)
#define HSPWMA_CC3PDisable()            CLR_HSPWMA_BIT(PWMA_CCER2, PWMACCER2_CC3E_MSK)

#define PWMACCER2_CC3P_MSK              BIT1
#define PWMA_CC3PNonInverted()          CLR_REG_BIT(PWMA_CCER2, PWMACCER2_CC3P_MSK)
#define PWMA_CC3PInverted()             SET_REG_BIT(PWMA_CCER2, PWMACCER2_CC3P_MSK)

#define HSPWMA_CC3PNonInverted()        CLR_HSPWMA_BIT(PWMA_CCER2, PWMACCER2_CC3P_MSK)
#define HSPWMA_CC3PInverted()           SET_HSPWMA_BIT(PWMA_CCER2, PWMACCER2_CC3P_MSK)

#define PWMACCER2_CC3NE_MSK             BIT2
#define PWMA_CC3NEnable()               SET_REG_BIT(PWMA_CCER2, PWMACCER2_CC3NE_MSK)
#define PWMA_CC3NDisable()              CLR_REG_BIT(PWMA_CCER2, PWMACCER2_CC3NE_MSK)

#define HSPWMA_CC3NEnable()             SET_HSPWMA_BIT(PWMA_CCER2, PWMACCER2_CC3NE_MSK)
#define HSPWMA_CC3NDisable()            CLR_HSPWMA_BIT(PWMA_CCER2, PWMACCER2_CC3NE_MSK)

#define PWMACCER2_CC3NP_MSK             BIT3
#define PWMA_CC3NNonInverted()          CLR_REG_BIT(PWMA_CCER2, PWMACCER2_CC3NP_MSK)
#define PWMA_CC3NInverted()             SET_REG_BIT(PWMA_CCER2, PWMACCER2_CC3NP_MSK)

#define HSPWMA_CC3NNonInverted()        CLR_HSPWMA_BIT(PWMA_CCER2, PWMACCER2_CC3NP_MSK)
#define HSPWMA_CC3NInverted()           SET_HSPWMA_BIT(PWMA_CCER2, PWMACCER2_CC3NP_MSK)

#define PWMACCER2_CC4E_MSK              BIT4
#define PWMA_CC4PEnable()               SET_REG_BIT(PWMA_CCER2, PWMACCER2_CC4E_MSK)
#define PWMA_CC4PDisable()              CLR_REG_BIT(PWMA_CCER2, PWMACCER2_CC4E_MSK)

#define HSPWMA_CC4PEnable()             SET_HSPWMA_BIT(PWMA_CCER2, PWMACCER2_CC4E_MSK)
#define HSPWMA_CC4PDisable()            CLR_HSPWMA_BIT(PWMA_CCER2, PWMACCER2_CC4E_MSK)

#define PWMACCER2_CC4P_MSK              BIT5
#define PWMA_CC4PNonInverted()          CLR_REG_BIT(PWMA_CCER2, PWMACCER2_CC4P_MSK)
#define PWMA_CC4PInverted()             SET_REG_BIT(PWMA_CCER2, PWMACCER2_CC4P_MSK)

#define HSPWMA_CC4PNonInverted()        CLR_HSPWMA_BIT(PWMA_CCER2, PWMACCER2_CC4P_MSK)
#define HSPWMA_CC4PInverted()           SET_HSPWMA_BIT(PWMA_CCER2, PWMACCER2_CC4P_MSK)

#define PWMACCER2_CC4NE_MSK             BIT6
#define PWMA_CC4NEnable()               SET_REG_BIT(PWMA_CCER2, PWMACCER2_CC4NE_MSK)
#define PWMA_CC4NDisable()              CLR_REG_BIT(PWMA_CCER2, PWMACCER2_CC4NE_MSK)

#define HSPWMA_CC4NEnable()             SET_HSPWMA_BIT(PWMA_CCER2, PWMACCER2_CC4NE_MSK)
#define HSPWMA_CC4NDisable()            CLR_HSPWMA_BIT(PWMA_CCER2, PWMACCER2_CC4NE_MSK)

#define PWMACCER2_CC4NP_MSK             BIT7
#define PWMA_CC4NNonInverted()          CLR_REG_BIT(PWMA_CCER2, PWMACCER2_CC4NP_MSK)
#define PWMA_CC4NInverted()             SET_REG_BIT(PWMA_CCER2, PWMACCER2_CC4NP_MSK)

#define HSPWMA_CC4NNonInverted()        CLR_HSPWMA_BIT(PWMA_CCER2, PWMACCER2_CC4NP_MSK)
#define HSPWMA_CC4NInverted()           SET_HSPWMA_BIT(PWMA_CCER2, PWMACCER2_CC4NP_MSK)

#define PWMABRK_MOE_MSK                 BIT7
#define PWMA_EnableMainOutput()         SET_REG_BIT(PWMA_BKR, PWMABRK_MOE_MSK)
#define PWMA_DisableMainOutput()        CLR_REG_BIT(PWMA_BKR, PWMABRK_MOE_MSK)

#define HSPWMA_EnableMainOutput()       SET_HSPWMA_BIT(PWMA_BKR, PWMABRK_MOE_MSK)
#define HSPWMA_DisableMainOutput()      CLR_HSPWMA_BIT(PWMA_BKR, PWMABRK_MOE_MSK)

#define PWMABRK_AOE_MSK                 BIT6
#define PWMA_EnableAutoOutput()         SET_REG_BIT(PWMA_BKR, PWMABRK_AOE_MSK)
#define PWMA_DisableAutoOutput()        CLR_REG_BIT(PWMA_BKR, PWMABRK_AOE_MSK)

#define HSPWMA_EnableAutoOutput()       SET_HSPWMA_BIT(PWMA_BKR, PWMABRK_AOE_MSK)
#define HSPWMA_DisableAutoOutput()      CLR_HSPWMA_BIT(PWMA_BKR, PWMABRK_AOE_MSK)

#define PWMABRK_BKP_MSK                 BIT5
#define PWMA_BRKNonInverted()           SET_REG_BIT(PWMA_BKR, PWMABRK_BKP_MSK)
#define PWMA_BRKInverted()              CLR_REG_BIT(PWMA_BKR, PWMABRK_BKP_MSK)

#define HSPWMA_BRKNonInverted()         SET_HSPWMA_BIT(PWMA_BKR, PWMABRK_BKP_MSK)
#define HSPWMA_BRKInverted()            CLR_HSPWMA_BIT(PWMA_BKR, PWMABRK_BKP_MSK)

#define PWMABRK_BKE_MSK                 BIT4
#define PWMA_BRKEnable()                SET_REG_BIT(PWMA_BKR, PWMABRK_BKE_MSK)
#define PWMA_BRKDisable()               CLR_REG_BIT(PWMA_BKR, PWMABRK_BKE_MSK)

#define HSPWMA_BRKEnable()              SET_HSPWMA_BIT(PWMA_BKR, PWMABRK_BKE_MSK)
#define HSPWMA_BRKDisable()             CLR_HSPWMA_BIT(PWMA_BKR, PWMABRK_BKE_MSK)

#define PWMABRK_OSSR_MSK                BIT3
#define PWMABRK_OSSI_MSK                BIT2
#define PWMA_RunInactiveNoOutput()      CLR_REG_BIT(PWMA_BKR, PWMABRK_OSSR_MSK)
#define PWMA_RunInactiveOutput()        SET_REG_BIT(PWMA_BKR, PWMABRK_OSSR_MSK)
#define PWMA_IdleInactiveNoOutput()     CLR_REG_BIT(PWMA_BKR, PWMABRK_OSSI_MSK)
#define PWMA_IdleInactiveOutput()       SET_REG_BIT(PWMA_BKR, PWMABRK_OSSI_MSK)

#define HSPWMA_RunInactiveNoOutput()    CLR_HSPWMA_BIT(PWMA_BKR, PWMABRK_OSSR_MSK)
#define HSPWMA_RunInactiveOutput()      SET_HSPWMA_BIT(PWMA_BKR, PWMABRK_OSSR_MSK)
#define HSPWMA_IdleInactiveNoOutput()   CLR_HSPWMA_BIT(PWMA_BKR, PWMABRK_OSSI_MSK)
#define HSPWMA_IdleInactiveOutput()     SET_HSPWMA_BIT(PWMA_BKR, PWMABRK_OSSI_MSK)

#define PWMABRK_LOCK_MSK                (BIT1 | BIT0)
#define PWMA_SetLockLevel(n)            MODIFY_REG(PWMA_BKR,  PWMABRK_LOCK_MSK, ((n) << 0))
#define PWMA_SetUnLock()                PWMA_SetLockLevel(0)

#define HSPWMA_SetLockLevel(n)          MODIFY_HSPWMA(PWMA_BKR,  PWMABRK_LOCK_MSK, ((n) << 0))
#define HSPWMA_SetUnLock()              HSPWMA_SetLockLevel(0)

#define PWMAOISR_OIS1_MSK               BIT0
#define PWMAOISR_OIS1N_MSK              BIT1
#define PWMAOISR_OIS2_MSK               BIT2
#define PWMAOISR_OIS2N_MSK              BIT3
#define PWMAOISR_OIS3_MSK               BIT4
#define PWMAOISR_OIS3N_MSK              BIT5
#define PWMAOISR_OIS4_MSK               BIT6
#define PWMAOISR_OIS4N_MSK              BIT7
#define PWMA_OC1PIdleLevelLow()         CLR_REG_BIT(PWMA_OISR, PWMAOISR_OIS1_MSK)
#define PWMA_OC1PIdleLevelHigh()        SET_REG_BIT(PWMA_OISR, PWMAOISR_OIS1_MSK)
#define PWMA_OC1NIdleLevelLow()         CLR_REG_BIT(PWMA_OISR, PWMAOISR_OIS1N_MSK)
#define PWMA_OC1NIdleLevelHigh()        SET_REG_BIT(PWMA_OISR, PWMAOISR_OIS1N_MSK)
#define PWMA_OC2PIdleLevelLow()         CLR_REG_BIT(PWMA_OISR, PWMAOISR_OIS2_MSK)
#define PWMA_OC2PIdleLevelHigh()        SET_REG_BIT(PWMA_OISR, PWMAOISR_OIS2_MSK)
#define PWMA_OC2NIdleLevelLow()         CLR_REG_BIT(PWMA_OISR, PWMAOISR_OIS2N_MSK)
#define PWMA_OC2NIdleLevelHigh()        SET_REG_BIT(PWMA_OISR, PWMAOISR_OIS2N_MSK)
#define PWMA_OC3PIdleLevelLow()         CLR_REG_BIT(PWMA_OISR, PWMAOISR_OIS3_MSK)
#define PWMA_OC3PIdleLevelHigh()        SET_REG_BIT(PWMA_OISR, PWMAOISR_OIS3_MSK)
#define PWMA_OC3NIdleLevelLow()         CLR_REG_BIT(PWMA_OISR, PWMAOISR_OIS3N_MSK)
#define PWMA_OC3NIdleLevelHigh()        SET_REG_BIT(PWMA_OISR, PWMAOISR_OIS3N_MSK)
#define PWMA_OC4PIdleLevelLow()         CLR_REG_BIT(PWMA_OISR, PWMAOISR_OIS4_MSK)
#define PWMA_OC4PIdleLevelHigh()        SET_REG_BIT(PWMA_OISR, PWMAOISR_OIS4_MSK)
#define PWMA_OC4NIdleLevelLow()         CLR_REG_BIT(PWMA_OISR, PWMAOISR_OIS4N_MSK)
#define PWMA_OC4NIdleLevelHigh()        SET_REG_BIT(PWMA_OISR, PWMAOISR_OIS4N_MSK)

#define HSPWMA_OC1PIdleLevelLow()       CLR_HSPWMA_BIT(PWMA_OISR, PWMAOISR_OIS1_MSK)
#define HSPWMA_OC1PIdleLevelHigh()      SET_HSPWMA_BIT(PWMA_OISR, PWMAOISR_OIS1_MSK)
#define HSPWMA_OC1NIdleLevelLow()       CLR_HSPWMA_BIT(PWMA_OISR, PWMAOISR_OIS1N_MSK)
#define HSPWMA_OC1NIdleLevelHigh()      SET_HSPWMA_BIT(PWMA_OISR, PWMAOISR_OIS1N_MSK)
#define HSPWMA_OC2PIdleLevelLow()       CLR_HSPWMA_BIT(PWMA_OISR, PWMAOISR_OIS2_MSK)
#define HSPWMA_OC2PIdleLevelHigh()      SET_HSPWMA_BIT(PWMA_OISR, PWMAOISR_OIS2_MSK)
#define HSPWMA_OC2NIdleLevelLow()       CLR_HSPWMA_BIT(PWMA_OISR, PWMAOISR_OIS2N_MSK)
#define HSPWMA_OC2NIdleLevelHigh()      SET_HSPWMA_BIT(PWMA_OISR, PWMAOISR_OIS2N_MSK)
#define HSPWMA_OC3PIdleLevelLow()       CLR_HSPWMA_BIT(PWMA_OISR, PWMAOISR_OIS3_MSK)
#define HSPWMA_OC3PIdleLevelHigh()      SET_HSPWMA_BIT(PWMA_OISR, PWMAOISR_OIS3_MSK)
#define HSPWMA_OC3NIdleLevelLow()       CLR_HSPWMA_BIT(PWMA_OISR, PWMAOISR_OIS3N_MSK)
#define HSPWMA_OC3NIdleLevelHigh()      SET_HSPWMA_BIT(PWMA_OISR, PWMAOISR_OIS3N_MSK)
#define HSPWMA_OC4PIdleLevelLow()       CLR_HSPWMA_BIT(PWMA_OISR, PWMAOISR_OIS4_MSK)
#define HSPWMA_OC4PIdleLevelHigh()      SET_HSPWMA_BIT(PWMA_OISR, PWMAOISR_OIS4_MSK)
#define HSPWMA_OC4NIdleLevelLow()       CLR_HSPWMA_BIT(PWMA_OISR, PWMAOISR_OIS4N_MSK)
#define HSPWMA_OC4NIdleLevelHigh()      SET_HSPWMA_BIT(PWMA_OISR, PWMAOISR_OIS4N_MSK)

#define PWMA_SetCounter(n)              (PWMA_CNTRH = HIBYTE(n), PWMA_CNTRL = LOBYTE(n))
#define PWMA_SetClockDivider(n)         (PWMA_PSCRH = HIBYTE(n), PWMA_PSCRL = LOBYTE(n))
#define PWMA_SetReload16(n)             (PWMA_ARRH = HIBYTE(n), PWMA_ARRL = LOBYTE(n))
#define PWMA_SetRepeatCounter(n)        (PWMA_RCRH = HIBYTE(n), PWMA_RCR = LOBYTE(n))

#define HSPWMA_SetCounter(n)            (WRITE_HSPWMA(PWMA_CNTRH, HIBYTE(n)), WRITE_HSPWMA(PWMA_CNTRL, LOBYTE(n)))
#define HSPWMA_SetClockDivider(n)       (WRITE_HSPWMA(PWMA_PSCRH, HIBYTE(n)), WRITE_HSPWMA(PWMA_PSCRL, LOBYTE(n)))
#define HSPWMA_SetReload16(n)           (WRITE_HSPWMA(PWMA_ARRH, HIBYTE(n)), WRITE_HSPWMA(PWMA_ARRL, LOBYTE(n)))
#define HSPWMA_SetRepeatCounter(n)      (WRITE_HSPWMA(PWMA_RCRH, HIBYTE(n)), WRITE_HSPWMA(PWMA_RCR, LOBYTE(n)))

#define PWMA_SetCCR1Value(n)            (PWMA_CCR1H = HIBYTE(n), PWMA_CCR1L = LOBYTE(n))
#define PWMA_SetCCR2Value(n)            (PWMA_CCR2H = HIBYTE(n), PWMA_CCR2L = LOBYTE(n))
#define PWMA_SetCCR3Value(n)            (PWMA_CCR3H = HIBYTE(n), PWMA_CCR3L = LOBYTE(n))
#define PWMA_SetCCR4Value(n)            (PWMA_CCR4H = HIBYTE(n), PWMA_CCR4L = LOBYTE(n))

#define HSPWMA_SetCCR1Value(n)          (WRITE_HSPWMA(PWMA_CCR1H, HIBYTE(n)), WRITE_HSPWMA(PWMA_CCR1L, LOBYTE(n)))
#define HSPWMA_SetCCR2Value(n)          (WRITE_HSPWMA(PWMA_CCR2H, HIBYTE(n)), WRITE_HSPWMA(PWMA_CCR2L, LOBYTE(n)))
#define HSPWMA_SetCCR3Value(n)          (WRITE_HSPWMA(PWMA_CCR3H, HIBYTE(n)), WRITE_HSPWMA(PWMA_CCR3L, LOBYTE(n)))
#define HSPWMA_SetCCR4Value(n)          (WRITE_HSPWMA(PWMA_CCR4H, HIBYTE(n)), WRITE_HSPWMA(PWMA_CCR4L, LOBYTE(n)))

#define PWMA_ReadReload16()             MAKEWORD(PWMA_ARRL, PWMA_ARRH)

#define HSPWMA_ReadReload16()           MAKEWORD(READ_HSPWMA(PWMA_ARRL), READ_HSPWMA(PWMA_ARRH))

#define PWMA_ReadCCR1Value()            MAKEWORD(PWMA_CCR1L, PWMA_CCR1H)
#define PWMA_ReadCCR2Value()            MAKEWORD(PWMA_CCR2L, PWMA_CCR2H)
#define PWMA_ReadCCR3Value()            MAKEWORD(PWMA_CCR3L, PWMA_CCR3H)
#define PWMA_ReadCCR4Value()            MAKEWORD(PWMA_CCR4L, PWMA_CCR4H)

#define HSPWMA_ReadCCR1Value()          MAKEWORD(READ_HSPWMA(PWMA_CCR1L), READ_HSPWMA(PWMA_CCR1H))
#define HSPWMA_ReadCCR2Value()          MAKEWORD(READ_HSPWMA(PWMA_CCR2L), READ_HSPWMA(PWMA_CCR2H))
#define HSPWMA_ReadCCR3Value()          MAKEWORD(READ_HSPWMA(PWMA_CCR3L), READ_HSPWMA(PWMA_CCR3H))
#define HSPWMA_ReadCCR4Value()          MAKEWORD(READ_HSPWMA(PWMA_CCR4L), READ_HSPWMA(PWMA_CCR4H))

#define PWMA_SetDeadTime(n)             PWMA_DTR = LOBYTE(n)

#define HSPWMA_SetDeadTime(n)           WRITE_HSPWMA(PWMA_DTR, LOBYTE(n))

////////////////////////

#define PWMADER_TDE_MSK                 BIT6
#define PWMADER_COMDE_MSK               BIT5
#define PWMADER_CC4DE_MSK               BIT4
#define PWMADER_CC3DE_MSK               BIT3
#define PWMADER_CC2DE_MSK               BIT2
#define PWMADER_CC1DE_MSK               BIT1
#define PWMADER_UDE_MSK                 BIT0
#define PWMA_TriggerIntAsDMAEvent()     SET_REG_BIT(PWMA_DER, PWMADER_TDE_MSK)
#define PWMA_COMIntAsDMAEvent()         SET_REG_BIT(PWMA_DER, PWMADER_COMDE_MSK)
#define PWMA_CC4IntAsDMAEvent()         SET_REG_BIT(PWMA_DER, PWMADER_CC4DE_MSK)
#define PWMA_CC3IntAsDMAEvent()         SET_REG_BIT(PWMA_DER, PWMADER_CC3DE_MSK)
#define PWMA_CC2IntAsDMAEvent()         SET_REG_BIT(PWMA_DER, PWMADER_CC2DE_MSK)
#define PWMA_CC1IntAsDMAEvent()         SET_REG_BIT(PWMA_DER, PWMADER_CC1DE_MSK)
#define PWMA_UpdateIntAsDMAEvent()      SET_REG_BIT(PWMA_DER, PWMADER_UDE_MSK)

#define HSPWMA_TriggerIntAsDMAEvent()   SET_HSPWMA_BIT(PWMA_DER, PWMADER_TDE_MSK)
#define HSPWMA_COMIntAsDMAEvent()       SET_HSPWMA_BIT(PWMA_DER, PWMADER_COMDE_MSK)
#define HSPWMA_CC4IntAsDMAEvent()       SET_HSPWMA_BIT(PWMA_DER, PWMADER_CC4DE_MSK)
#define HSPWMA_CC3IntAsDMAEvent()       SET_HSPWMA_BIT(PWMA_DER, PWMADER_CC3DE_MSK)
#define HSPWMA_CC2IntAsDMAEvent()       SET_HSPWMA_BIT(PWMA_DER, PWMADER_CC2DE_MSK)
#define HSPWMA_CC1IntAsDMAEvent()       SET_HSPWMA_BIT(PWMA_DER, PWMADER_CC1DE_MSK)
#define HSPWMA_UpdateIntAsDMAEvent()    SET_HSPWMA_BIT(PWMA_DER, PWMADER_UDE_MSK)

#define PWMADBA_DBA_MSK                 (BIT4 | BIT_LN)
#define PWMA_SetDMABaseAddress(d)       MODIFY_REG(PWMA_DBA,  PWMADBA_DBA_MSK, ((d) << 0))

#define HSPWMA_SetDMABaseAddress(d)     MODIFY_HSPWMA(PWMA_DBA,  PWMADBA_DBA_MSK, ((d) << 0))

#define PWMADBL_DBL_MSK                 (BIT4 | BIT_LN)
#define PWMA_SetDMABurstCount(d)        MODIFY_REG(PWMA_DBL,  PWMADBL_DBL_MSK, ((d) << 0))

#define HSPWMA_SetDMABurstCount(d)      MODIFY_HSPWMA(PWMA_DBL,  PWMADBL_DBL_MSK, ((d) << 0))

#define PWMADMACR_SIZE_MSK              (BIT1 | BIT0)
#define PWMA_SetDMABurstBytes(n)        MODIFY_REG(PWMA_DMACR,  PWMADMACR_SIZE_MSK, ((n) << 0))
#define PWMA_SetDMABurst1Byte()         PWMA_SetDMABurstBytes(0)
#define PWMA_SetDMABurst2Bytes()        PWMA_SetDMABurstBytes(1)
#define PWMA_SetDMABurst3Bytes()        PWMA_SetDMABurstBytes(2)
#define PWMA_SetDMABurst4Bytes()        PWMA_SetDMABurstBytes(3)

#define HSPWMA_SetDMABurstBytes(n)      MODIFY_HSPWMA(PWMA_DMACR,  PWMADMACR_SIZE_MSK, ((n) << 0))
#define HSPWMA_SetDMABurst1Byte()       HSPWMA_SetDMABurstBytes(0)
#define HSPWMA_SetDMABurst2Bytes()      HSPWMA_SetDMABurstBytes(1)
#define HSPWMA_SetDMABurst3Bytes()      HSPWMA_SetDMABurstBytes(2)
#define HSPWMA_SetDMABurst4Bytes()      HSPWMA_SetDMABurstBytes(3)

#define PWMADMACR_DSKIP_MSK             BIT4
#define PWMA_DMASkipSpace()             SET_REG_BIT(PWMA_DMACR, PWMADMACR_DSKIP_MSK)
#define PWMA_DMANeverSkipSpace()        CLR_REG_BIT(PWMA_DMACR, PWMADMACR_DSKIP_MSK)

#define HSPWMA_DMASkipSpace()           SET_HSPWMA_BIT(PWMA_DMACR, PWMADMACR_DSKIP_MSK)
#define HSPWMA_DMANeverSkipSpace()      CLR_HSPWMA_BIT(PWMA_DMACR, PWMADMACR_DSKIP_MSK)

#define PWMADMACR_DDIR_MSK              BIT3
#define PWMA_SetDMAOutput()             CLR_REG_BIT(PWMA_DMACR, PWMADMACR_DDIR_MSK)
#define PWMA_SetDMAInput()              SET_REG_BIT(PWMA_DMACR, PWMADMACR_DDIR_MSK)

#define HSPWMA_SetDMAOutput()           CLR_HSPWMA_BIT(PWMA_DMACR, PWMADMACR_DDIR_MSK)
#define HSPWMA_SetDMAInput()            SET_HSPWMA_BIT(PWMA_DMACR, PWMADMACR_DDIR_MSK)

#define PWMADMACR_DMAEN_MSK             BIT2
#define PWMA_EnableDMA()                SET_REG_BIT(PWMA_DMACR, PWMADMACR_DMAEN_MSK)
#define PWMA_DisableDMA()               CLR_REG_BIT(PWMA_DMACR, PWMADMACR_DMAEN_MSK)

#define HSPWMA_EnableDMA()              SET_HSPWMA_BIT(PWMA_DMACR, PWMADMACR_DMAEN_MSK)
#define HSPWMA_DisableDMA()             CLR_HSPWMA_BIT(PWMA_DMACR, PWMADMACR_DMAEN_MSK)

////////////////////////

#define PWMBENO_ENO5P_MSK               BIT0
#define PWMBENO_ENO6P_MSK               BIT2
#define PWMBENO_ENO7P_MSK               BIT4
#define PWMBENO_ENO8P_MSK               BIT6
#define PWMB_EnablePWM5POutput()        SET_REG_BIT(PWMB_ENO, PWMBENO_ENO5P_MSK)
#define PWMB_DisablePWM5POutput()       CLR_REG_BIT(PWMB_ENO, PWMBENO_ENO5P_MSK)
#define PWMB_EnablePWM6POutput()        SET_REG_BIT(PWMB_ENO, PWMBENO_ENO6P_MSK)
#define PWMB_DisablePWM6POutput()       CLR_REG_BIT(PWMB_ENO, PWMBENO_ENO6P_MSK)
#define PWMB_EnablePWM7POutput()        SET_REG_BIT(PWMB_ENO, PWMBENO_ENO7P_MSK)
#define PWMB_DisablePWM7POutput()       CLR_REG_BIT(PWMB_ENO, PWMBENO_ENO7P_MSK)
#define PWMB_EnablePWM8POutput()        SET_REG_BIT(PWMB_ENO, PWMBENO_ENO8P_MSK)
#define PWMB_DisablePWM8POutput()       CLR_REG_BIT(PWMB_ENO, PWMBENO_ENO8P_MSK)

#define HSPWMB_EnablePWM5POutput()      SET_HSPWMB_BIT(PWMB_ENO, PWMBENO_ENO5P_MSK)
#define HSPWMB_DisablePWM5POutput()     CLR_HSPWMB_BIT(PWMB_ENO, PWMBENO_ENO5P_MSK)
#define HSPWMB_EnablePWM6POutput()      SET_HSPWMB_BIT(PWMB_ENO, PWMBENO_ENO6P_MSK)
#define HSPWMB_DisablePWM6POutput()     CLR_HSPWMB_BIT(PWMB_ENO, PWMBENO_ENO6P_MSK)
#define HSPWMB_EnablePWM7POutput()      SET_HSPWMB_BIT(PWMB_ENO, PWMBENO_ENO7P_MSK)
#define HSPWMB_DisablePWM7POutput()     CLR_HSPWMB_BIT(PWMB_ENO, PWMBENO_ENO7P_MSK)
#define HSPWMB_EnablePWM8POutput()      SET_HSPWMB_BIT(PWMB_ENO, PWMBENO_ENO8P_MSK)
#define HSPWMB_DisablePWM8POutput()     CLR_HSPWMB_BIT(PWMB_ENO, PWMBENO_ENO8P_MSK)

#define PWMBIOAUX_AUX5P_MSK             BIT0
#define PWMBIOAUX_AUX6P_MSK             BIT2
#define PWMBIOAUX_AUX7P_MSK             BIT4
#define PWMBIOAUX_AUX8P_MSK             BIT6
#define PWMB_EnoCtrlPWM5P()             CLR_REG_BIT(PWMB_IOAUX, PWMBIOAUX_AUX5P_MSK)
#define PWMB_EnoBrkCtrlPWM5P()          SET_REG_BIT(PWMB_IOAUX, PWMBIOAUX_AUX5P_MSK)
#define PWMB_EnoCtrlPWM6P()             CLR_REG_BIT(PWMB_IOAUX, PWMBIOAUX_AUX6P_MSK)
#define PWMB_EnoBrkCtrlPWM6P()          SET_REG_BIT(PWMB_IOAUX, PWMBIOAUX_AUX6P_MSK)
#define PWMB_EnoCtrlPWM7P()             CLR_REG_BIT(PWMB_IOAUX, PWMBIOAUX_AUX7P_MSK)
#define PWMB_EnoBrkCtrlPWM7P()          SET_REG_BIT(PWMB_IOAUX, PWMBIOAUX_AUX7P_MSK)
#define PWMB_EnoCtrlPWM8P()             CLR_REG_BIT(PWMB_IOAUX, PWMBIOAUX_AUX8P_MSK)
#define PWMB_EnoBrkCtrlPWM8P()          SET_REG_BIT(PWMB_IOAUX, PWMBIOAUX_AUX8P_MSK)

#define HSPWMB_EnoCtrlPWM5P()           CLR_HSPWMB_BIT(PWMB_IOAUX, PWMBIOAUX_AUX5P_MSK)
#define HSPWMB_EnoBrkCtrlPWM5P()        SET_HSPWMB_BIT(PWMB_IOAUX, PWMBIOAUX_AUX5P_MSK)
#define HSPWMB_EnoCtrlPWM6P()           CLR_HSPWMB_BIT(PWMB_IOAUX, PWMBIOAUX_AUX6P_MSK)
#define HSPWMB_EnoBrkCtrlPWM6P()        SET_HSPWMB_BIT(PWMB_IOAUX, PWMBIOAUX_AUX6P_MSK)
#define HSPWMB_EnoCtrlPWM7P()           CLR_HSPWMB_BIT(PWMB_IOAUX, PWMBIOAUX_AUX7P_MSK)
#define HSPWMB_EnoBrkCtrlPWM7P()        SET_HSPWMB_BIT(PWMB_IOAUX, PWMBIOAUX_AUX7P_MSK)
#define HSPWMB_EnoCtrlPWM8P()           CLR_HSPWMB_BIT(PWMB_IOAUX, PWMBIOAUX_AUX8P_MSK)
#define HSPWMB_EnoBrkCtrlPWM8P()        SET_HSPWMB_BIT(PWMB_IOAUX, PWMBIOAUX_AUX8P_MSK)

#define PWMBCR1_ARPE_MSK                BIT7
#define PWMB_UnbufferARR()              CLR_REG_BIT(PWMB_CR1, PWMBCR1_ARPE_MSK)
#define PWMB_BufferARR()                SET_REG_BIT(PWMB_CR1, PWMBCR1_ARPE_MSK)

#define HSPWMB_UnbufferARR()            CLR_HSPWMB_BIT(PWMB_CR1, PWMBCR1_ARPE_MSK)
#define HSPWMB_BufferARR()              SET_HSPWMB_BIT(PWMB_CR1, PWMBCR1_ARPE_MSK)

#define PWMBCR1_CMS_MSK                 (BIT6 | BIT5)
#define PWMB_SetAlignMode(n)            MODIFY_REG(PWMB_CR1,  PWMBCR1_CMS_MSK, ((n) << 5))
#define PWMB_EdgeAlignMode()            PWMB_SetAlignMode(0)
#define PWMB_CenterAlignMode1()         PWMB_SetAlignMode(1)
#define PWMB_CenterAlignMode2()         PWMB_SetAlignMode(2)
#define PWMB_CenterAlignMode3()         PWMB_SetAlignMode(3)

#define HSPWMB_SetAlignMode(n)          MODIFY_HSPWMB(PWMB_CR1,  PWMBCR1_CMS_MSK, ((n) << 5))
#define HSPWMB_EdgeAlignMode()          HSPWMB_SetAlignMode(0)
#define HSPWMB_CenterAlignMode1()       HSPWMB_SetAlignMode(1)
#define HSPWMB_CenterAlignMode2()       HSPWMB_SetAlignMode(2)
#define HSPWMB_CenterAlignMode3()       HSPWMB_SetAlignMode(3)

#define PWMBCR1_DIR_MSK                 BIT4
#define PWMB_UpCounter()                CLR_REG_BIT(PWMB_CR1, PWMBCR1_DIR_MSK)
#define PWMB_DownCounter()              SET_REG_BIT(PWMB_CR1, PWMBCR1_DIR_MSK)

#define HSPWMB_UpCounter()              CLR_HSPWMB_BIT(PWMB_CR1, PWMBCR1_DIR_MSK)
#define HSPWMB_DownCounter()            SET_HSPWMB_BIT(PWMB_CR1, PWMBCR1_DIR_MSK)

#define PWMBCR1_OPM_MSK                 BIT3
#define PWMB_ContinueMode()             CLR_REG_BIT(PWMB_CR1, PWMBCR1_OPM_MSK)
#define PWMB_OnePulseMode()             SET_REG_BIT(PWMB_CR1, PWMBCR1_OPM_MSK)

#define HSPWMB_ContinueMode()           CLR_HSPWMB_BIT(PWMB_CR1, PWMBCR1_OPM_MSK)
#define HSPWMB_OnePulseMode()           SET_HSPWMB_BIT(PWMB_CR1, PWMBCR1_OPM_MSK)

#define PWMBCR1_URS_MSK                 BIT2
#define PWMB_UpdateRequestMode1()       SET_REG_BIT(PWMB_CR1, PWMBCR1_URS_MSK)
#define PWMB_UpdateRequestMode2()       CLR_REG_BIT(PWMB_CR1, PWMBCR1_URS_MSK)

#define HSPWMB_UpdateRequestMode1()     SET_HSPWMB_BIT(PWMB_CR1, PWMBCR1_URS_MSK)
#define HSPWMB_UpdateRequestMode2()     CLR_HSPWMB_BIT(PWMB_CR1, PWMBCR1_URS_MSK)

#define PWMBCR1_UDIS_MSK                BIT1
#define PWMB_EnableUpdate()             CLR_REG_BIT(PWMB_CR1, PWMBCR1_UDIS_MSK)
#define PWMB_DisableUpdate()            SET_REG_BIT(PWMB_CR1, PWMBCR1_UDIS_MSK)

#define HSPWMB_EnableUpdate()           CLR_HSPWMB_BIT(PWMB_CR1, PWMBCR1_UDIS_MSK)
#define HSPWMB_DisableUpdate()          SET_HSPWMB_BIT(PWMB_CR1, PWMBCR1_UDIS_MSK)

#define PWMBCR1_CEN_MSK                 BIT0
#define PWMB_Run()                      SET_REG_BIT(PWMB_CR1, PWMBCR1_CEN_MSK)
#define PWMB_Stop()                     CLR_REG_BIT(PWMB_CR1, PWMBCR1_CEN_MSK)

#define HSPWMB_Run()                    SET_HSPWMB_BIT(PWMB_CR1, PWMBCR1_CEN_MSK)
#define HSPWMB_Stop()                   CLR_HSPWMB_BIT(PWMB_CR1, PWMBCR1_CEN_MSK)

#define PWMBCR2_TIS_MSK                 BIT7
#define PWMB_PWM5PToTI()                CLR_REG_BIT(PWMB_CR2, PWMBCR2_TIS_MSK)
#define PWMB_Xor5P6P7PToTI()            SET_REG_BIT(PWMB_CR2, PWMBCR2_TIS_MSK)

#define HSPWMB_PWM5PToTI()              CLR_HSPWMB_BIT(PWMB_CR2, PWMBCR2_TIS_MSK)
#define HSPWMB_Xor5P6P7PToTI()          SET_HSPWMB_BIT(PWMB_CR2, PWMBCR2_TIS_MSK)

#define PWMBCR2_MMS_MSK                 (BIT6 | BIT5 | BIT4)
#define PWMB_SelectTRGO(n)              MODIFY_REG(PWMB_CR2,  PWMBCR2_MMS_MSK, ((n) << 4))
#define PWMB_ResetAsTRGO()              PWMB_SelectTRGO(0)
#define PWMB_EnableAsTRGO()             PWMB_SelectTRGO(1)
#define PWMB_UpdateAsTRGO()             PWMB_SelectTRGO(2)
#define PWMB_MatchAsTRGO()              PWMB_SelectTRGO(3)
#define PWMB_OC5REFAsTRGO()             PWMB_SelectTRGO(4)
#define PWMB_OC6REFAsTRGO()             PWMB_SelectTRGO(5)
#define PWMB_OC7REFAsTRGO()             PWMB_SelectTRGO(6)
#define PWMB_OC8REFAsTRGO()             PWMB_SelectTRGO(7)

#define HSPWMB_SelectTRGO(n)            MODIFY_HSPWMB(PWMB_CR2,  PWMBCR2_MMS_MSK, ((n) << 4))
#define HSPWMB_ResetAsTRGO()            HSPWMB_SelectTRGO(0)
#define HSPWMB_EnableAsTRGO()           HSPWMB_SelectTRGO(1)
#define HSPWMB_UpdateAsTRGO()           HSPWMB_SelectTRGO(2)
#define HSPWMB_MatchAsTRGO()            HSPWMB_SelectTRGO(3)
#define HSPWMB_OC5REFAsTRGO()           HSPWMB_SelectTRGO(4)
#define HSPWMB_OC6REFAsTRGO()           HSPWMB_SelectTRGO(5)
#define HSPWMB_OC7REFAsTRGO()           HSPWMB_SelectTRGO(6)
#define HSPWMB_OC8REFAsTRGO()           HSPWMB_SelectTRGO(7)

#define PWMBCR2_COMS_MSK                BIT2
#define PWMB_CCOCUpdateMode0()          CLR_REG_BIT(PWMB_CR2, PWMBCR2_COMS_MSK)
#define PWMB_CCOCUpdateMode1()          SET_REG_BIT(PWMB_CR2, PWMBCR2_COMS_MSK)

#define HSPWMB_CCOCUpdateMode0()        CLR_HSPWMB_BIT(PWMB_CR2, PWMBCR2_COMS_MSK)
#define HSPWMB_CCOCUpdateMode1()        SET_HSPWMB_BIT(PWMB_CR2, PWMBCR2_COMS_MSK)

#define PWMBCR2_CCPC_MSK                BIT0
#define PWMB_UnbufferCCOC()             CLR_REG_BIT(PWMB_CR2, PWMBCR2_CCPC_MSK)
#define PWMB_BufferCCOC()               SET_REG_BIT(PWMB_CR2, PWMBCR2_CCPC_MSK)

#define HSPWMB_UnbufferCCOC()           CLR_HSPWMB_BIT(PWMB_CR2, PWMBCR2_CCPC_MSK)
#define HSPWMB_BufferCCOC()             SET_HSPWMB_BIT(PWMB_CR2, PWMBCR2_CCPC_MSK)

#define PWMBSMCR_MSM_MSK                BIT7
#define PWMB_StandaloneMode()           CLR_REG_BIT(PWMB_SMCR, PWMBSMCR_MSM_MSK)
#define PWMB_MasterSlaveMode()          SET_REG_BIT(PWMB_SMCR, PWMBSMCR_MSM_MSK)

#define HSPWMB_StandaloneMode()         CLR_HSPWMB_BIT(PWMB_SMCR, PWMBSMCR_MSM_MSK)
#define HSPWMB_MasterSlaveMode()        SET_HSPWMB_BIT(PWMB_SMCR, PWMBSMCR_MSM_MSK)

#define PWMBSMCR_TS_MSK                 (BIT6 | BIT5 | BIT4)
#define PWMB_SelectTRGI(n)              MODIFY_REG(PWMB_SMCR,  PWMBSMCR_TS_MSK, ((n) << 4))
#define PWMB_DisableTRGI()              PWMB_SelectTRGI(0)
#define PWMB_TI5FEDAsTRGI()             PWMB_SelectTRGI(4)
#define PWMB_TI5FP5AsTRGI()             PWMB_SelectTRGI(5)
#define PWMB_TI6FP6AsTRGI()             PWMB_SelectTRGI(6)
#define PWMB_ETRFAsTRGI()               PWMB_SelectTRGI(7)

#define HSPWMB_SelectTRGI(n)            MODIFY_HSPWMB(PWMB_SMCR,  PWMBSMCR_TS_MSK, ((n) << 4))
#define HSPWMB_DisableTRGI()            HSPWMB_SelectTRGI(0)
#define HSPWMB_TI5FEDAsTRGI()           HSPWMB_SelectTRGI(4)
#define HSPWMB_TI5FP5AsTRGI()           HSPWMB_SelectTRGI(5)
#define HSPWMB_TI6FP6AsTRGI()           HSPWMB_SelectTRGI(6)
#define HSPWMB_ETRFAsTRGI()             HSPWMB_SelectTRGI(7)

#define PWMBSMCR_SMS_MSK                (BIT2 | BIT1 | BIT0)
#define PWMB_SetClockMode(n)            MODIFY_REG(PWMB_SMCR,  PWMBSMCR_SMS_MSK, ((n) << 0))
#define PWMB_InternalClockMode()        PWMB_SetClockMode(0)
#define PWMB_EncoderMode1()             PWMB_SetClockMode(1)
#define PWMB_EncoderMode2()             PWMB_SetClockMode(2)
#define PWMB_EncoderMode3()             PWMB_SetClockMode(3)
#define PWMB_TriggerResetMode()         PWMB_SetClockMode(4)
#define PWMB_TriggerGateMode()          PWMB_SetClockMode(5)
#define PWMB_TriggerStartMode()         PWMB_SetClockMode(6)
#define PWMB_ExternalClockMode()        PWMB_SetClockMode(7)

#define HSPWMB_SetClockMode(n)          MODIFY_HSPWMB(PWMB_SMCR,  PWMBSMCR_SMS_MSK, ((n) << 0))
#define HSPWMB_InternalClockMode()      HSPWMB_SetClockMode(0)
#define HSPWMB_EncoderMode1()           HSPWMB_SetClockMode(1)
#define HSPWMB_EncoderMode2()           HSPWMB_SetClockMode(2)
#define HSPWMB_EncoderMode3()           HSPWMB_SetClockMode(3)
#define HSPWMB_TriggerResetMode()       HSPWMB_SetClockMode(4)
#define HSPWMB_TriggerGateMode()        HSPWMB_SetClockMode(5)
#define HSPWMB_TriggerStartMode()       HSPWMB_SetClockMode(6)
#define HSPWMB_ExternalClockMode()      HSPWMB_SetClockMode(7)

#define PWMBETR_ETP_MSK                 BIT7
#define PWMB_ETRNonInverted()           CLR_REG_BIT(PWMB_ETR, PWMBETR_ETP_MSK)
#define PWMB_ETRInverted()              SET_REG_BIT(PWMB_ETR, PWMBETR_ETP_MSK)

#define HSPWMB_ETRNonInverted()         CLR_HSPWMB_BIT(PWMB_ETR, PWMBETR_ETP_MSK)
#define HSPWMB_ETRInverted()            SET_HSPWMB_BIT(PWMB_ETR, PWMBETR_ETP_MSK)

#define PWMBETR_ECE_MSK                 BIT6
#define PWMB_DisableExternalClock2()    CLR_REG_BIT(PWMB_ETR, PWMBETR_ECE_MSK)
#define PWMB_EnableExternalClock2()     SET_REG_BIT(PWMB_ETR, PWMBETR_ECE_MSK)

#define HSPWMB_DisableExternalClock2()  CLR_HSPWMB_BIT(PWMB_ETR, PWMBETR_ECE_MSK)
#define HSPWMB_EnableExternalClock2()   SET_HSPWMB_BIT(PWMB_ETR, PWMBETR_ECE_MSK)

#define PWMBETR_ETPS_MSK                (BIT5 | BIT4)
#define PWMB_SetETRDivider(n)           MODIFY_REG(PWMB_ETR,  PWMBETR_ETPS_MSK, ((n) << 4))
#define PWMB_ETRDivider1()              PWMB_SetETRDivider(0)
#define PWMB_ETRDivider2()              PWMB_SetETRDivider(1)
#define PWMB_ETRDivider4()              PWMB_SetETRDivider(2)
#define PWMB_ETRDivider8()              PWMB_SetETRDivider(3)

#define HSPWMB_SetETRDivider(n)         MODIFY_HSPWMB(PWMB_ETR,  PWMBETR_ETPS_MSK, ((n) << 4))
#define HSPWMB_ETRDivider1()            HSPWMB_SetETRDivider(0)
#define HSPWMB_ETRDivider2()            HSPWMB_SetETRDivider(1)
#define HSPWMB_ETRDivider4()            HSPWMB_SetETRDivider(2)
#define HSPWMB_ETRDivider8()            HSPWMB_SetETRDivider(3)

#define PWMBETR_ETF_MSK                 (BIT_LN)
#define PWMB_SetETRFilter(n)            MODIFY_REG(PWMB_ETR,  PWMBETR_ETF_MSK, ((n) << 0))
#define PWMB_ETRFilter1()               PWMB_SetETRFilter(0)
#define PWMB_ETRFilter2()               PWMB_SetETRFilter(1)
#define PWMB_ETRFilter4()               PWMB_SetETRFilter(2)
#define PWMB_ETRFilter8()               PWMB_SetETRFilter(3)
#define PWMB_ETRFilter12()              PWMB_SetETRFilter(4)
#define PWMB_ETRFilter16()              PWMB_SetETRFilter(5)
#define PWMB_ETRFilter24()              PWMB_SetETRFilter(6)
#define PWMB_ETRFilter32()              PWMB_SetETRFilter(7)
#define PWMB_ETRFilter48()              PWMB_SetETRFilter(8)
#define PWMB_ETRFilter64()              PWMB_SetETRFilter(9)
#define PWMB_ETRFilter80()              PWMB_SetETRFilter(10)
#define PWMB_ETRFilter96()              PWMB_SetETRFilter(11)
#define PWMB_ETRFilter128()             PWMB_SetETRFilter(12)
#define PWMB_ETRFilter160()             PWMB_SetETRFilter(13)
#define PWMB_ETRFilter192()             PWMB_SetETRFilter(14)
#define PWMB_ETRFilter256()             PWMB_SetETRFilter(15)

#define HSPWMB_SetETRFilter(n)          MODIFY_HSPWMB(PWMB_ETR,  PWMBETR_ETF_MSK, ((n) << 0))
#define HSPWMB_ETRFilter1()             HSPWMB_SetETRFilter(0)
#define HSPWMB_ETRFilter2()             HSPWMB_SetETRFilter(1)
#define HSPWMB_ETRFilter4()             HSPWMB_SetETRFilter(2)
#define HSPWMB_ETRFilter8()             HSPWMB_SetETRFilter(3)
#define HSPWMB_ETRFilter12()            HSPWMB_SetETRFilter(4)
#define HSPWMB_ETRFilter16()            HSPWMB_SetETRFilter(5)
#define HSPWMB_ETRFilter24()            HSPWMB_SetETRFilter(6)
#define HSPWMB_ETRFilter32()            HSPWMB_SetETRFilter(7)
#define HSPWMB_ETRFilter48()            HSPWMB_SetETRFilter(8)
#define HSPWMB_ETRFilter64()            HSPWMB_SetETRFilter(9)
#define HSPWMB_ETRFilter80()            HSPWMB_SetETRFilter(10)
#define HSPWMB_ETRFilter96()            HSPWMB_SetETRFilter(11)
#define HSPWMB_ETRFilter128()           HSPWMB_SetETRFilter(12)
#define HSPWMB_ETRFilter160()           HSPWMB_SetETRFilter(13)
#define HSPWMB_ETRFilter192()           HSPWMB_SetETRFilter(14)
#define HSPWMB_ETRFilter256()           HSPWMB_SetETRFilter(15)

#define PWMBEGR_BG_MSK                  BIT7
#define PWMBEGR_TG_MSK                  BIT6
#define PWMBEGR_COMG_MSK                BIT5
#define PWMBEGR_CC8G_MSK                BIT4
#define PWMBEGR_CC7G_MSK                BIT3
#define PWMBEGR_CC6G_MSK                BIT2
#define PWMBEGR_CC5G_MSK                BIT1
#define PWMBEGR_UG_MSK                  BIT0
#define PWMB_GenerateBreakEvent()       SET_REG_BIT(PWMB_EGR, PWMBEGR_BG_MSK)
#define PWMB_GenerateTriggerEvent()     SET_REG_BIT(PWMB_EGR, PWMBEGR_TG_MSK)
#define PWMB_GenerateCOMEvent()         SET_REG_BIT(PWMB_EGR, PWMBEGR_COMG_MSK)
#define PWMB_GenerateCC8Event()         SET_REG_BIT(PWMB_EGR, PWMBEGR_CC8G_MSK)
#define PWMB_GenerateCC7Event()         SET_REG_BIT(PWMB_EGR, PWMBEGR_CC7G_MSK)
#define PWMB_GenerateCC6Event()         SET_REG_BIT(PWMB_EGR, PWMBEGR_CC6G_MSK)
#define PWMB_GenerateCC5Event()         SET_REG_BIT(PWMB_EGR, PWMBEGR_CC5G_MSK)
#define PWMB_GenerateUpdateEvent()      SET_REG_BIT(PWMB_EGR, PWMBEGR_UG_MSK)

#define HSPWMB_GenerateBreakEvent()     SET_HSPWMB_BIT(PWMB_EGR, PWMBEGR_BG_MSK)
#define HSPWMB_GenerateTriggerEvent()   SET_HSPWMB_BIT(PWMB_EGR, PWMBEGR_TG_MSK)
#define HSPWMB_GenerateCOMEvent()       SET_HSPWMB_BIT(PWMB_EGR, PWMBEGR_COMG_MSK)
#define HSPWMB_GenerateCC8Event()       SET_HSPWMB_BIT(PWMB_EGR, PWMBEGR_CC8G_MSK)
#define HSPWMB_GenerateCC7Event()       SET_HSPWMB_BIT(PWMB_EGR, PWMBEGR_CC7G_MSK)
#define HSPWMB_GenerateCC6Event()       SET_HSPWMB_BIT(PWMB_EGR, PWMBEGR_CC6G_MSK)
#define HSPWMB_GenerateCC5Event()       SET_HSPWMB_BIT(PWMB_EGR, PWMBEGR_CC5G_MSK)
#define HSPWMB_GenerateUpdateEvent()    SET_HSPWMB_BIT(PWMB_EGR, PWMBEGR_UG_MSK)

#define PWMBCCMR1_OC5CE_MSK             BIT7
#define PWMB_EnableETRFClearOC5REF()    SET_REG_BIT(PWMB_CCMR1, PWMBCCMR1_OC5CE_MSK)
#define PWMB_DisableETRFClearOC5REF()   CLR_REG_BIT(PWMB_CCMR1, PWMBCCMR1_OC5CE_MSK)

#define HSPWMB_EnableETRFClearOC5REF()  SET_HSPWMB_BIT(PWMB_CCMR1, PWMBCCMR1_OC5CE_MSK)
#define HSPWMB_DisableETRFClearOC5REF() CLR_HSPWMB_BIT(PWMB_CCMR1, PWMBCCMR1_OC5CE_MSK)

#define PWMBCCMR1_OC5M_MSK              (BIT6 | BIT5 | BIT4)
#define PWMB_SetOC5OutputMode(n)        MODIFY_REG(PWMB_CCMR1,  PWMBCCMR1_OC5M_MSK, ((n) << 4))
#define PWMB_OC5REFFrozen()             PWMB_SetOC5OutputMode(0)
#define PWMB_OC5REFHighIfMatch()        PWMB_SetOC5OutputMode(1)
#define PWMB_OC5REFLowIfMatch()         PWMB_SetOC5OutputMode(2)
#define PWMB_OC5REFToggleIfMatch()      PWMB_SetOC5OutputMode(3)
#define PWMB_OC5REFForceLow()           PWMB_SetOC5OutputMode(4)
#define PWMB_OC5REFForceHigh()          PWMB_SetOC5OutputMode(5)
#define PWMB_OC5REFPWMMode1()           PWMB_SetOC5OutputMode(6)
#define PWMB_OC5REFPWMMode2()           PWMB_SetOC5OutputMode(7)

#define HSPWMB_SetOC5OutputMode(n)      MODIFY_HSPWMB(PWMB_CCMR1,  PWMBCCMR1_OC5M_MSK, ((n) << 4))
#define HSPWMB_OC5REFFrozen()           HSPWMB_SetOC5OutputMode(0)
#define HSPWMB_OC5REFHighIfMatch()      HSPWMB_SetOC5OutputMode(1)
#define HSPWMB_OC5REFLowIfMatch()       HSPWMB_SetOC5OutputMode(2)
#define HSPWMB_OC5REFToggleIfMatch()    HSPWMB_SetOC5OutputMode(3)
#define HSPWMB_OC5REFForceLow()         HSPWMB_SetOC5OutputMode(4)
#define HSPWMB_OC5REFForceHigh()        HSPWMB_SetOC5OutputMode(5)
#define HSPWMB_OC5REFPWMMode1()         HSPWMB_SetOC5OutputMode(6)
#define HSPWMB_OC5REFPWMMode2()         HSPWMB_SetOC5OutputMode(7)

#define PWMBCCMR1_OC5PE_MSK             BIT3
#define PWMB_UnbufferCCR5()             CLR_REG_BIT(PWMB_CCMR1, PWMBCCMR1_OC5PE_MSK)
#define PWMB_BufferCCR5()               SET_REG_BIT(PWMB_CCMR1, PWMBCCMR1_OC5PE_MSK)

#define HSPWMB_UnbufferCCR5()           CLR_HSPWMB_BIT(PWMB_CCMR1, PWMBCCMR1_OC5PE_MSK)
#define HSPWMB_BufferCCR5()             SET_HSPWMB_BIT(PWMB_CCMR1, PWMBCCMR1_OC5PE_MSK)

#define PWMBCCMR1_OC5FE_MSK             BIT2
#define PWMB_DisableOC5FastMode()       CLR_REG_BIT(PWMB_CCMR1, PWMBCCMR1_OC5FE_MSK)
#define PWMB_EnableOC5FastMode()        SET_REG_BIT(PWMB_CCMR1, PWMBCCMR1_OC5FE_MSK)

#define HSPWMB_DisableOC5FastMode()     CLR_HSPWMB_BIT(PWMB_CCMR1, PWMBCCMR1_OC5FE_MSK)
#define HSPWMB_EnableOC5FastMode()      SET_HSPWMB_BIT(PWMB_CCMR1, PWMBCCMR1_OC5FE_MSK)

#define PWMBCCMR1_CC5S_MSK              (BIT1 | BIT0)
#define PWMB_SetCC5Mode(n)              MODIFY_REG(PWMB_CCMR1,  PWMBCCMR1_CC5S_MSK, ((n) << 0))
#define PWMB_CC5Output()                PWMB_SetCC5Mode(0)
#define PWMB_CC5CaptureTI5FP5()         PWMB_SetCC5Mode(1)
#define PWMB_CC5CaptureTI6FP5()         PWMB_SetCC5Mode(2)
#define PWMB_CC5CaptureTRC()            PWMB_SetCC5Mode(3)

#define HSPWMB_SetCC5Mode(n)            MODIFY_HSPWMB(PWMB_CCMR1,  PWMBCCMR1_CC5S_MSK, ((n) << 0))
#define HSPWMB_CC5Output()              HSPWMB_SetCC5Mode(0)
#define HSPWMB_CC5CaptureTI5FP5()       HSPWMB_SetCC5Mode(1)
#define HSPWMB_CC5CaptureTI6FP5()       HSPWMB_SetCC5Mode(2)
#define HSPWMB_CC5CaptureTRC()          HSPWMB_SetCC5Mode(3)

#define PWMBCCMR1_IC5F_MSK              (BIT_HN)
#define PWMB_SetIC5Filter(n)            MODIFY_REG(PWMB_CCMR1,  PWMBCCMR1_IC5F_MSK, ((n) << 4))
#define PWMB_IC5Filter1()               PWMB_SetIC5Filter(0)
#define PWMB_IC5Filter2()               PWMB_SetIC5Filter(1)
#define PWMB_IC5Filter4()               PWMB_SetIC5Filter(2)
#define PWMB_IC5Filter8()               PWMB_SetIC5Filter(3)
#define PWMB_IC5Filter12()              PWMB_SetIC5Filter(4)
#define PWMB_IC5Filter16()              PWMB_SetIC5Filter(5)
#define PWMB_IC5Filter24()              PWMB_SetIC5Filter(6)
#define PWMB_IC5Filter32()              PWMB_SetIC5Filter(7)
#define PWMB_IC5Filter48()              PWMB_SetIC5Filter(8)
#define PWMB_IC5Filter64()              PWMB_SetIC5Filter(9)
#define PWMB_IC5Filter80()              PWMB_SetIC5Filter(10)
#define PWMB_IC5Filter96()              PWMB_SetIC5Filter(11)
#define PWMB_IC5Filter128()             PWMB_SetIC5Filter(12)
#define PWMB_IC5Filter160()             PWMB_SetIC5Filter(13)
#define PWMB_IC5Filter192()             PWMB_SetIC5Filter(14)
#define PWMB_IC5Filter256()             PWMB_SetIC5Filter(15)

#define HSPWMB_SetIC5Filter(n)          MODIFY_HSPWMB(PWMB_CCMR1,  PWMBCCMR1_IC5F_MSK, ((n) << 4))
#define HSPWMB_IC5Filter1()             HSPWMB_SetIC5Filter(0)
#define HSPWMB_IC5Filter2()             HSPWMB_SetIC5Filter(1)
#define HSPWMB_IC5Filter4()             HSPWMB_SetIC5Filter(2)
#define HSPWMB_IC5Filter8()             HSPWMB_SetIC5Filter(3)
#define HSPWMB_IC5Filter12()            HSPWMB_SetIC5Filter(4)
#define HSPWMB_IC5Filter16()            HSPWMB_SetIC5Filter(5)
#define HSPWMB_IC5Filter24()            HSPWMB_SetIC5Filter(6)
#define HSPWMB_IC5Filter32()            HSPWMB_SetIC5Filter(7)
#define HSPWMB_IC5Filter48()            HSPWMB_SetIC5Filter(8)
#define HSPWMB_IC5Filter64()            HSPWMB_SetIC5Filter(9)
#define HSPWMB_IC5Filter80()            HSPWMB_SetIC5Filter(10)
#define HSPWMB_IC5Filter96()            HSPWMB_SetIC5Filter(11)
#define HSPWMB_IC5Filter128()           HSPWMB_SetIC5Filter(12)
#define HSPWMB_IC5Filter160()           HSPWMB_SetIC5Filter(13)
#define HSPWMB_IC5Filter192()           HSPWMB_SetIC5Filter(14)
#define HSPWMB_IC5Filter256()           HSPWMB_SetIC5Filter(15)

#define PWMBCCMR1_IC5PS_MSK             (BIT3 | BIT2)
#define PWMB_SetIC5Divider(n)           MODIFY_REG(PWMB_CCMR1,  PWMBCCMR1_IC5PS_MSK, ((n) << 2))
#define PWMB_IC5Divider1()              PWMB_SetIC5Divider(0)
#define PWMB_IC5Divider2()              PWMB_SetIC5Divider(1)
#define PWMB_IC5Divider4()              PWMB_SetIC5Divider(2)
#define PWMB_IC5Divider8()              PWMB_SetIC5Divider(3)

#define HSPWMB_SetIC5Divider(n)         MODIFY_HSPWMB(PWMB_CCMR1,  PWMBCCMR1_IC5PS_MSK, ((n) << 2))
#define HSPWMB_IC5Divider1()            HSPWMB_SetIC5Divider(0)
#define HSPWMB_IC5Divider2()            HSPWMB_SetIC5Divider(1)
#define HSPWMB_IC5Divider4()            HSPWMB_SetIC5Divider(2)
#define HSPWMB_IC5Divider8()            HSPWMB_SetIC5Divider(3)

#define PWMBCCMR2_OC6CE_MSK             BIT7
#define PWMB_EnableETRFClearOC6REF()    SET_REG_BIT(PWMB_CCMR2, PWMBCCMR2_OC6CE_MSK)
#define PWMB_DisableETRFClearOC6REF()   CLR_REG_BIT(PWMB_CCMR2, PWMBCCMR2_OC6CE_MSK)

#define HSPWMB_EnableETRFClearOC6REF()  SET_HSPWMB_BIT(PWMB_CCMR2, PWMBCCMR2_OC6CE_MSK)
#define HSPWMB_DisableETRFClearOC6REF() CLR_HSPWMB_BIT(PWMB_CCMR2, PWMBCCMR2_OC6CE_MSK)

#define PWMBCCMR2_OC6M_MSK              (BIT6 | BIT5 | BIT4)
#define PWMB_SetOC6OutputMode(n)        MODIFY_REG(PWMB_CCMR2,  PWMBCCMR2_OC6M_MSK, ((n) << 4))
#define PWMB_OC6REFFrozen()             PWMB_SetOC6OutputMode(0)
#define PWMB_OC6REFHighIfMatch()        PWMB_SetOC6OutputMode(1)
#define PWMB_OC6REFLowIfMatch()         PWMB_SetOC6OutputMode(2)
#define PWMB_OC6REFToggleIfMatch()      PWMB_SetOC6OutputMode(3)
#define PWMB_OC6REFForceLow()           PWMB_SetOC6OutputMode(4)
#define PWMB_OC6REFForceHigh()          PWMB_SetOC6OutputMode(5)
#define PWMB_OC6REFPWMMode1()           PWMB_SetOC6OutputMode(6)
#define PWMB_OC6REFPWMMode2()           PWMB_SetOC6OutputMode(7)

#define HSPWMB_SetOC6OutputMode(n)      MODIFY_HSPWMB(PWMB_CCMR2,  PWMBCCMR2_OC6M_MSK, ((n) << 4))
#define HSPWMB_OC6REFFrozen()           HSPWMB_SetOC6OutputMode(0)
#define HSPWMB_OC6REFHighIfMatch()      HSPWMB_SetOC6OutputMode(1)
#define HSPWMB_OC6REFLowIfMatch()       HSPWMB_SetOC6OutputMode(2)
#define HSPWMB_OC6REFToggleIfMatch()    HSPWMB_SetOC6OutputMode(3)
#define HSPWMB_OC6REFForceLow()         HSPWMB_SetOC6OutputMode(4)
#define HSPWMB_OC6REFForceHigh()        HSPWMB_SetOC6OutputMode(5)
#define HSPWMB_OC6REFPWMMode1()         HSPWMB_SetOC6OutputMode(6)
#define HSPWMB_OC6REFPWMMode2()         HSPWMB_SetOC6OutputMode(7)

#define PWMBCCMR2_OC6PE_MSK             BIT3
#define PWMB_UnbufferCCR6()             CLR_REG_BIT(PWMB_CCMR2, PWMBCCMR2_OC6PE_MSK)
#define PWMB_BufferCCR6()               SET_REG_BIT(PWMB_CCMR2, PWMBCCMR2_OC6PE_MSK)

#define HSPWMB_UnbufferCCR6()           CLR_HSPWMB_BIT(PWMB_CCMR2, PWMBCCMR2_OC6PE_MSK)
#define HSPWMB_BufferCCR6()             SET_HSPWMB_BIT(PWMB_CCMR2, PWMBCCMR2_OC6PE_MSK)

#define PWMBCCMR2_OC6FE_MSK             BIT2
#define PWMB_DisableOC6FastMode()       CLR_REG_BIT(PWMB_CCMR2, PWMBCCMR2_OC6FE_MSK)
#define PWMB_EnableOC6FastMode()        SET_REG_BIT(PWMB_CCMR2, PWMBCCMR2_OC6FE_MSK)

#define HSPWMB_DisableOC6FastMode()     CLR_HSPWMB_BIT(PWMB_CCMR2, PWMBCCMR2_OC6FE_MSK)
#define HSPWMB_EnableOC6FastMode()      SET_HSPWMB_BIT(PWMB_CCMR2, PWMBCCMR2_OC6FE_MSK)

#define PWMBCCMR2_CC6S_MSK              (BIT1 | BIT0)
#define PWMB_SetCC6Mode(n)              MODIFY_REG(PWMB_CCMR2,  PWMBCCMR2_CC6S_MSK, ((n) << 0))
#define PWMB_CC6Output()                PWMB_SetCC6Mode(0)
#define PWMB_CC6CaptureTI6FP6()         PWMB_SetCC6Mode(1)
#define PWMB_CC6CaptureTI5FP6()         PWMB_SetCC6Mode(2)
#define PWMB_CC6CaptureTRC()            PWMB_SetCC6Mode(3)

#define HSPWMB_SetCC6Mode(n)            MODIFY_HSPWMB(PWMB_CCMR2,  PWMBCCMR2_CC6S_MSK, ((n) << 0))
#define HSPWMB_CC6Output()              HSPWMB_SetCC6Mode(0)
#define HSPWMB_CC6CaptureTI6FP6()       HSPWMB_SetCC6Mode(1)
#define HSPWMB_CC6CaptureTI5FP6()       HSPWMB_SetCC6Mode(2)
#define HSPWMB_CC6CaptureTRC()          HSPWMB_SetCC6Mode(3)

#define PWMBCCMR2_IC6F_MSK              (BIT_HN)
#define PWMB_SetIC6Filter(n)            MODIFY_REG(PWMB_CCMR2,  PWMBCCMR2_IC6F_MSK, ((n) << 4))
#define PWMB_IC6Filter1()               PWMB_SetIC6Filter(0)
#define PWMB_IC6Filter2()               PWMB_SetIC6Filter(1)
#define PWMB_IC6Filter4()               PWMB_SetIC6Filter(2)
#define PWMB_IC6Filter8()               PWMB_SetIC6Filter(3)
#define PWMB_IC6Filter12()              PWMB_SetIC6Filter(4)
#define PWMB_IC6Filter16()              PWMB_SetIC6Filter(5)
#define PWMB_IC6Filter24()              PWMB_SetIC6Filter(6)
#define PWMB_IC6Filter32()              PWMB_SetIC6Filter(7)
#define PWMB_IC6Filter48()              PWMB_SetIC6Filter(8)
#define PWMB_IC6Filter64()              PWMB_SetIC6Filter(9)
#define PWMB_IC6Filter80()              PWMB_SetIC6Filter(10)
#define PWMB_IC6Filter96()              PWMB_SetIC6Filter(11)
#define PWMB_IC6Filter128()             PWMB_SetIC6Filter(12)
#define PWMB_IC6Filter160()             PWMB_SetIC6Filter(13)
#define PWMB_IC6Filter192()             PWMB_SetIC6Filter(14)
#define PWMB_IC6Filter256()             PWMB_SetIC6Filter(15)

#define HSPWMB_SetIC6Filter(n)          MODIFY_HSPWMB(PWMB_CCMR2,  PWMBCCMR2_IC6F_MSK, ((n) << 4))
#define HSPWMB_IC6Filter1()             HSPWMB_SetIC6Filter(0)
#define HSPWMB_IC6Filter2()             HSPWMB_SetIC6Filter(1)
#define HSPWMB_IC6Filter4()             HSPWMB_SetIC6Filter(2)
#define HSPWMB_IC6Filter8()             HSPWMB_SetIC6Filter(3)
#define HSPWMB_IC6Filter12()            HSPWMB_SetIC6Filter(4)
#define HSPWMB_IC6Filter16()            HSPWMB_SetIC6Filter(5)
#define HSPWMB_IC6Filter24()            HSPWMB_SetIC6Filter(6)
#define HSPWMB_IC6Filter32()            HSPWMB_SetIC6Filter(7)
#define HSPWMB_IC6Filter48()            HSPWMB_SetIC6Filter(8)
#define HSPWMB_IC6Filter64()            HSPWMB_SetIC6Filter(9)
#define HSPWMB_IC6Filter80()            HSPWMB_SetIC6Filter(10)
#define HSPWMB_IC6Filter96()            HSPWMB_SetIC6Filter(11)
#define HSPWMB_IC6Filter128()           HSPWMB_SetIC6Filter(12)
#define HSPWMB_IC6Filter160()           HSPWMB_SetIC6Filter(13)
#define HSPWMB_IC6Filter192()           HSPWMB_SetIC6Filter(14)
#define HSPWMB_IC6Filter256()           HSPWMB_SetIC6Filter(15)

#define PWMBCCMR2_IC6PS_MSK             (BIT3 | BIT2)
#define PWMB_SetIC6Divider(n)           MODIFY_REG(PWMB_CCMR2,  PWMBCCMR2_IC6PS_MSK, ((n) << 2))
#define PWMB_IC6Divider1()              PWMB_SetIC6Divider(0)
#define PWMB_IC6Divider2()              PWMB_SetIC6Divider(1)
#define PWMB_IC6Divider4()              PWMB_SetIC6Divider(2)
#define PWMB_IC6Divider8()              PWMB_SetIC6Divider(3)

#define HSPWMB_SetIC6Divider(n)         MODIFY_HSPWMB(PWMB_CCMR2,  PWMBCCMR2_IC6PS_MSK, ((n) << 2))
#define HSPWMB_IC6Divider1()            HSPWMB_SetIC6Divider(0)
#define HSPWMB_IC6Divider2()            HSPWMB_SetIC6Divider(1)
#define HSPWMB_IC6Divider4()            HSPWMB_SetIC6Divider(2)
#define HSPWMB_IC6Divider8()            HSPWMB_SetIC6Divider(3)

#define PWMBCCMR3_OC7CE_MSK             BIT7
#define PWMB_EnableETRFClearOC7REF()    SET_REG_BIT(PWMB_CCMR3, PWMBCCMR3_OC7CE_MSK)
#define PWMB_DisableETRFClearOC7REF()   CLR_REG_BIT(PWMB_CCMR3, PWMBCCMR3_OC7CE_MSK)

#define HSPWMB_EnableETRFClearOC7REF()  SET_HSPWMB_BIT(PWMB_CCMR3, PWMBCCMR3_OC7CE_MSK)
#define HSPWMB_DisableETRFClearOC7REF() CLR_HSPWMB_BIT(PWMB_CCMR3, PWMBCCMR3_OC7CE_MSK)

#define PWMBCCMR3_OC7M_MSK              (BIT6 | BIT5 | BIT4)
#define PWMB_SetOC7OutputMode(n)        MODIFY_REG(PWMB_CCMR3,  PWMBCCMR3_OC7M_MSK, ((n) << 4))
#define PWMB_OC7REFFrozen()             PWMB_SetOC7OutputMode(0)
#define PWMB_OC7REFHighIfMatch()        PWMB_SetOC7OutputMode(1)
#define PWMB_OC7REFLowIfMatch()         PWMB_SetOC7OutputMode(2)
#define PWMB_OC7REFToggleIfMatch()      PWMB_SetOC7OutputMode(3)
#define PWMB_OC7REFForceLow()           PWMB_SetOC7OutputMode(4)
#define PWMB_OC7REFForceHigh()          PWMB_SetOC7OutputMode(5)
#define PWMB_OC7REFPWMMode1()           PWMB_SetOC7OutputMode(6)
#define PWMB_OC7REFPWMMode2()           PWMB_SetOC7OutputMode(7)

#define HSPWMB_SetOC7OutputMode(n)      MODIFY_HSPWMB(PWMB_CCMR3,  PWMBCCMR3_OC7M_MSK, ((n) << 4))
#define HSPWMB_OC7REFFrozen()           HSPWMB_SetOC7OutputMode(0)
#define HSPWMB_OC7REFHighIfMatch()      HSPWMB_SetOC7OutputMode(1)
#define HSPWMB_OC7REFLowIfMatch()       HSPWMB_SetOC7OutputMode(2)
#define HSPWMB_OC7REFToggleIfMatch()    HSPWMB_SetOC7OutputMode(3)
#define HSPWMB_OC7REFForceLow()         HSPWMB_SetOC7OutputMode(4)
#define HSPWMB_OC7REFForceHigh()        HSPWMB_SetOC7OutputMode(5)
#define HSPWMB_OC7REFPWMMode1()         HSPWMB_SetOC7OutputMode(6)
#define HSPWMB_OC7REFPWMMode2()         HSPWMB_SetOC7OutputMode(7)

#define PWMBCCMR3_OC7PE_MSK             BIT3
#define PWMB_UnbufferCCR7()             CLR_REG_BIT(PWMB_CCMR3, PWMBCCMR3_OC7PE_MSK)
#define PWMB_BufferCCR7()               SET_REG_BIT(PWMB_CCMR3, PWMBCCMR3_OC7PE_MSK)

#define HSPWMB_UnbufferCCR7()           CLR_HSPWMB_BIT(PWMB_CCMR3, PWMBCCMR3_OC7PE_MSK)
#define HSPWMB_BufferCCR7()             SET_HSPWMB_BIT(PWMB_CCMR3, PWMBCCMR3_OC7PE_MSK)

#define PWMBCCMR3_OC7FE_MSK             BIT2
#define PWMB_DisableOC7FastMode()       CLR_REG_BIT(PWMB_CCMR3, PWMBCCMR3_OC7FE_MSK)
#define PWMB_EnableOC7FastMode()        SET_REG_BIT(PWMB_CCMR3, PWMBCCMR3_OC7FE_MSK)

#define HSPWMB_DisableOC7FastMode()     CLR_HSPWMB_BIT(PWMB_CCMR3, PWMBCCMR3_OC7FE_MSK)
#define HSPWMB_EnableOC7FastMode()      SET_HSPWMB_BIT(PWMB_CCMR3, PWMBCCMR3_OC7FE_MSK)

#define PWMBCCMR3_CC7S_MSK              (BIT1 | BIT0)
#define PWMB_SetCC7Mode(n)              MODIFY_REG(PWMB_CCMR3,  PWMBCCMR3_CC7S_MSK, ((n) << 0))
#define PWMB_CC7Output()                PWMB_SetCC7Mode(0)
#define PWMB_CC7CaptureTI7FP7()         PWMB_SetCC7Mode(1)
#define PWMB_CC7CaptureTI8FP7()         PWMB_SetCC7Mode(2)
#define PWMB_CC7CaptureTRC()            PWMB_SetCC7Mode(3)

#define HSPWMB_SetCC7Mode(n)            MODIFY_HSPWMB(PWMB_CCMR3,  PWMBCCMR3_CC7S_MSK, ((n) << 0))
#define HSPWMB_CC7Output()              HSPWMB_SetCC7Mode(0)
#define HSPWMB_CC7CaptureTI7FP7()       HSPWMB_SetCC7Mode(1)
#define HSPWMB_CC7CaptureTI8FP7()       HSPWMB_SetCC7Mode(2)
#define HSPWMB_CC7CaptureTRC()          HSPWMB_SetCC7Mode(3)

#define PWMBCCMR3_IC7F_MSK              (BIT_HN)
#define PWMB_SetIC7Filter(n)            MODIFY_REG(PWMB_CCMR3,  PWMBCCMR3_IC7F_MSK, ((n) << 4))
#define PWMB_IC7Filter1()               PWMB_SetIC7Filter(0)
#define PWMB_IC7Filter2()               PWMB_SetIC7Filter(1)
#define PWMB_IC7Filter4()               PWMB_SetIC7Filter(2)
#define PWMB_IC7Filter8()               PWMB_SetIC7Filter(3)
#define PWMB_IC7Filter12()              PWMB_SetIC7Filter(4)
#define PWMB_IC7Filter16()              PWMB_SetIC7Filter(5)
#define PWMB_IC7Filter24()              PWMB_SetIC7Filter(6)
#define PWMB_IC7Filter32()              PWMB_SetIC7Filter(7)
#define PWMB_IC7Filter48()              PWMB_SetIC7Filter(8)
#define PWMB_IC7Filter64()              PWMB_SetIC7Filter(9)
#define PWMB_IC7Filter80()              PWMB_SetIC7Filter(10)
#define PWMB_IC7Filter96()              PWMB_SetIC7Filter(11)
#define PWMB_IC7Filter128()             PWMB_SetIC7Filter(12)
#define PWMB_IC7Filter160()             PWMB_SetIC7Filter(13)
#define PWMB_IC7Filter192()             PWMB_SetIC7Filter(14)
#define PWMB_IC7Filter256()             PWMB_SetIC7Filter(15)

#define HSPWMB_SetIC7Filter(n)          MODIFY_HSPWMB(PWMB_CCMR3,  PWMBCCMR3_IC7F_MSK, ((n) << 4))
#define HSPWMB_IC7Filter1()             HSPWMB_SetIC7Filter(0)
#define HSPWMB_IC7Filter2()             HSPWMB_SetIC7Filter(1)
#define HSPWMB_IC7Filter4()             HSPWMB_SetIC7Filter(2)
#define HSPWMB_IC7Filter8()             HSPWMB_SetIC7Filter(3)
#define HSPWMB_IC7Filter12()            HSPWMB_SetIC7Filter(4)
#define HSPWMB_IC7Filter16()            HSPWMB_SetIC7Filter(5)
#define HSPWMB_IC7Filter24()            HSPWMB_SetIC7Filter(6)
#define HSPWMB_IC7Filter32()            HSPWMB_SetIC7Filter(7)
#define HSPWMB_IC7Filter48()            HSPWMB_SetIC7Filter(8)
#define HSPWMB_IC7Filter64()            HSPWMB_SetIC7Filter(9)
#define HSPWMB_IC7Filter80()            HSPWMB_SetIC7Filter(10)
#define HSPWMB_IC7Filter96()            HSPWMB_SetIC7Filter(11)
#define HSPWMB_IC7Filter128()           HSPWMB_SetIC7Filter(12)
#define HSPWMB_IC7Filter160()           HSPWMB_SetIC7Filter(13)
#define HSPWMB_IC7Filter192()           HSPWMB_SetIC7Filter(14)
#define HSPWMB_IC7Filter256()           HSPWMB_SetIC7Filter(15)

#define PWMBCCMR3_IC7PS_MSK             (BIT3 | BIT2)
#define PWMB_SetIC7Divider(n)           MODIFY_REG(PWMB_CCMR3,  PWMBCCMR3_IC7PS_MSK, ((n) << 2))
#define PWMB_IC7Divider1()              PWMB_SetIC7Divider(0)
#define PWMB_IC7Divider2()              PWMB_SetIC7Divider(1)
#define PWMB_IC7Divider4()              PWMB_SetIC7Divider(2)
#define PWMB_IC7Divider8()              PWMB_SetIC7Divider(3)

#define HSPWMB_SetIC7Divider(n)         MODIFY_HSPWMB(PWMB_CCMR3,  PWMBCCMR3_IC7PS_MSK, ((n) << 2))
#define HSPWMB_IC7Divider1()            HSPWMB_SetIC7Divider(0)
#define HSPWMB_IC7Divider2()            HSPWMB_SetIC7Divider(1)
#define HSPWMB_IC7Divider4()            HSPWMB_SetIC7Divider(2)
#define HSPWMB_IC7Divider8()            HSPWMB_SetIC7Divider(3)

#define PWMBCCMR4_OC8CE_MSK             BIT7
#define PWMB_EnableETRFClearOC8REF()    SET_REG_BIT(PWMB_CCMR4, PWMBCCMR4_OC8CE_MSK)
#define PWMB_DisableETRFClearOC8REF()   CLR_REG_BIT(PWMB_CCMR4, PWMBCCMR4_OC8CE_MSK)

#define HSPWMB_EnableETRFClearOC8REF()  SET_HSPWMB_BIT(PWMB_CCMR4, PWMBCCMR4_OC8CE_MSK)
#define HSPWMB_DisableETRFClearOC8REF() CLR_HSPWMB_BIT(PWMB_CCMR4, PWMBCCMR4_OC8CE_MSK)

#define PWMBCCMR4_OC8M_MSK              (BIT6 | BIT5 | BIT4)
#define PWMB_SetOC8OutputMode(n)        MODIFY_REG(PWMB_CCMR4,  PWMBCCMR4_OC8M_MSK, ((n) << 4))
#define PWMB_OC8REFFrozen()             PWMB_SetOC8OutputMode(0)
#define PWMB_OC8REFHighIfMatch()        PWMB_SetOC8OutputMode(1)
#define PWMB_OC8REFLowIfMatch()         PWMB_SetOC8OutputMode(2)
#define PWMB_OC8REFToggleIfMatch()      PWMB_SetOC8OutputMode(3)
#define PWMB_OC8REFForceLow()           PWMB_SetOC8OutputMode(4)
#define PWMB_OC8REFForceHigh()          PWMB_SetOC8OutputMode(5)
#define PWMB_OC8REFPWMMode1()           PWMB_SetOC8OutputMode(6)
#define PWMB_OC8REFPWMMode2()           PWMB_SetOC8OutputMode(7)

#define HSPWMB_SetOC8OutputMode(n)      MODIFY_HSPWMB(PWMB_CCMR4,  PWMBCCMR4_OC8M_MSK, ((n) << 4))
#define HSPWMB_OC8REFFrozen()           HSPWMB_SetOC8OutputMode(0)
#define HSPWMB_OC8REFHighIfMatch()      HSPWMB_SetOC8OutputMode(1)
#define HSPWMB_OC8REFLowIfMatch()       HSPWMB_SetOC8OutputMode(2)
#define HSPWMB_OC8REFToggleIfMatch()    HSPWMB_SetOC8OutputMode(3)
#define HSPWMB_OC8REFForceLow()         HSPWMB_SetOC8OutputMode(4)
#define HSPWMB_OC8REFForceHigh()        HSPWMB_SetOC8OutputMode(5)
#define HSPWMB_OC8REFPWMMode1()         HSPWMB_SetOC8OutputMode(6)
#define HSPWMB_OC8REFPWMMode2()         HSPWMB_SetOC8OutputMode(7)

#define PWMBCCMR4_OC8PE_MSK             BIT3
#define PWMB_UnbufferCCR8()             CLR_REG_BIT(PWMB_CCMR4, PWMBCCMR4_OC8PE_MSK)
#define PWMB_BufferCCR8()               SET_REG_BIT(PWMB_CCMR4, PWMBCCMR4_OC8PE_MSK)

#define HSPWMB_UnbufferCCR8()           CLR_HSPWMB_BIT(PWMB_CCMR4, PWMBCCMR4_OC8PE_MSK)
#define HSPWMB_BufferCCR8()             SET_HSPWMB_BIT(PWMB_CCMR4, PWMBCCMR4_OC8PE_MSK)

#define PWMBCCMR4_OC8FE_MSK             BIT2
#define PWMB_DisableOC8FastMode()       CLR_REG_BIT(PWMB_CCMR4, PWMBCCMR4_OC8FE_MSK)
#define PWMB_EnableOC8FastMode()        SET_REG_BIT(PWMB_CCMR4, PWMBCCMR4_OC8FE_MSK)

#define HSPWMB_DisableOC8FastMode()     CLR_HSPWMB_BIT(PWMB_CCMR4, PWMBCCMR4_OC8FE_MSK)
#define HSPWMB_EnableOC8FastMode()      SET_HSPWMB_BIT(PWMB_CCMR4, PWMBCCMR4_OC8FE_MSK)

#define PWMBCCMR4_CC8S_MSK              (BIT1 | BIT0)
#define PWMB_SetCC8Mode(n)              MODIFY_REG(PWMB_CCMR4,  PWMBCCMR4_CC8S_MSK, ((n) << 0))
#define PWMB_CC8Output()                PWMB_SetCC8Mode(0)
#define PWMB_CC8CaptureTI8FP8()         PWMB_SetCC8Mode(1)
#define PWMB_CC8CaptureTI7FP8()         PWMB_SetCC8Mode(2)
#define PWMB_CC8CaptureTRC()            PWMB_SetCC8Mode(3)

#define HSPWMB_SetCC8Mode(n)            MODIFY_HSPWMB(PWMB_CCMR4,  PWMBCCMR4_CC8S_MSK, ((n) << 0))
#define HSPWMB_CC8Output()              HSPWMB_SetCC8Mode(0)
#define HSPWMB_CC8CaptureTI8FP8()       HSPWMB_SetCC8Mode(1)
#define HSPWMB_CC8CaptureTI7FP8()       HSPWMB_SetCC8Mode(2)
#define HSPWMB_CC8CaptureTRC()          HSPWMB_SetCC8Mode(3)

#define PWMBCCMR4_IC8F_MSK              (BIT_HN)
#define PWMB_SetIC8Filter(n)            MODIFY_REG(PWMB_CCMR4,  PWMBCCMR4_IC8F_MSK, ((n) << 4))
#define PWMB_IC8Filter1()               PWMB_SetIC8Filter(0)
#define PWMB_IC8Filter2()               PWMB_SetIC8Filter(1)
#define PWMB_IC8Filter4()               PWMB_SetIC8Filter(2)
#define PWMB_IC8Filter8()               PWMB_SetIC8Filter(3)
#define PWMB_IC8Filter12()              PWMB_SetIC8Filter(4)
#define PWMB_IC8Filter16()              PWMB_SetIC8Filter(5)
#define PWMB_IC8Filter24()              PWMB_SetIC8Filter(6)
#define PWMB_IC8Filter32()              PWMB_SetIC8Filter(7)
#define PWMB_IC8Filter48()              PWMB_SetIC8Filter(8)
#define PWMB_IC8Filter64()              PWMB_SetIC8Filter(9)
#define PWMB_IC8Filter80()              PWMB_SetIC8Filter(10)
#define PWMB_IC8Filter96()              PWMB_SetIC8Filter(11)
#define PWMB_IC8Filter128()             PWMB_SetIC8Filter(12)
#define PWMB_IC8Filter160()             PWMB_SetIC8Filter(13)
#define PWMB_IC8Filter192()             PWMB_SetIC8Filter(14)
#define PWMB_IC8Filter256()             PWMB_SetIC8Filter(15)

#define HSPWMB_SetIC8Filter(n)          MODIFY_HSPWMB(PWMB_CCMR4,  PWMBCCMR4_IC8F_MSK, ((n) << 4))
#define HSPWMB_IC8Filter1()             HSPWMB_SetIC8Filter(0)
#define HSPWMB_IC8Filter2()             HSPWMB_SetIC8Filter(1)
#define HSPWMB_IC8Filter4()             HSPWMB_SetIC8Filter(2)
#define HSPWMB_IC8Filter8()             HSPWMB_SetIC8Filter(3)
#define HSPWMB_IC8Filter12()            HSPWMB_SetIC8Filter(4)
#define HSPWMB_IC8Filter16()            HSPWMB_SetIC8Filter(5)
#define HSPWMB_IC8Filter24()            HSPWMB_SetIC8Filter(6)
#define HSPWMB_IC8Filter32()            HSPWMB_SetIC8Filter(7)
#define HSPWMB_IC8Filter48()            HSPWMB_SetIC8Filter(8)
#define HSPWMB_IC8Filter64()            HSPWMB_SetIC8Filter(9)
#define HSPWMB_IC8Filter80()            HSPWMB_SetIC8Filter(10)
#define HSPWMB_IC8Filter96()            HSPWMB_SetIC8Filter(11)
#define HSPWMB_IC8Filter128()           HSPWMB_SetIC8Filter(12)
#define HSPWMB_IC8Filter160()           HSPWMB_SetIC8Filter(13)
#define HSPWMB_IC8Filter192()           HSPWMB_SetIC8Filter(14)
#define HSPWMB_IC8Filter256()           HSPWMB_SetIC8Filter(15)

#define PWMBCCMR4_IC8PS_MSK             (BIT3 | BIT2)
#define PWMB_SetIC8Divider(n)           MODIFY_REG(PWMB_CCMR4,  PWMBCCMR4_IC8PS_MSK, ((n) << 2))
#define PWMB_IC8Divider1()              PWMB_SetIC8Divider(0)
#define PWMB_IC8Divider2()              PWMB_SetIC8Divider(1)
#define PWMB_IC8Divider4()              PWMB_SetIC8Divider(2)
#define PWMB_IC8Divider8()              PWMB_SetIC8Divider(3)

#define HSPWMB_SetIC8Divider(n)         MODIFY_HSPWMB(PWMB_CCMR4,  PWMBCCMR4_IC8PS_MSK, ((n) << 2))
#define HSPWMB_IC8Divider1()            HSPWMB_SetIC8Divider(0)
#define HSPWMB_IC8Divider2()            HSPWMB_SetIC8Divider(1)
#define HSPWMB_IC8Divider4()            HSPWMB_SetIC8Divider(2)
#define HSPWMB_IC8Divider8()            HSPWMB_SetIC8Divider(3)

#define PWMBCCER1_CC5E_MSK              BIT0
#define PWMB_CC5PEnable()               SET_REG_BIT(PWMB_CCER1, PWMBCCER1_CC5E_MSK)
#define PWMB_CC5PDisable()              CLR_REG_BIT(PWMB_CCER1, PWMBCCER1_CC5E_MSK)

#define HSPWMB_CC5PEnable()             SET_HSPWMB_BIT(PWMB_CCER1, PWMBCCER1_CC5E_MSK)
#define HSPWMB_CC5PDisable()            CLR_HSPWMB_BIT(PWMB_CCER1, PWMBCCER1_CC5E_MSK)

#define PWMBCCER1_CC5P_MSK              BIT1
#define PWMB_CC5PNonInverted()          CLR_REG_BIT(PWMB_CCER1, PWMBCCER1_CC5P_MSK)
#define PWMB_CC5PInverted()             SET_REG_BIT(PWMB_CCER1, PWMBCCER1_CC5P_MSK)

#define HSPWMB_CC5PNonInverted()        CLR_HSPWMB_BIT(PWMB_CCER1, PWMBCCER1_CC5P_MSK)
#define HSPWMB_CC5PInverted()           SET_HSPWMB_BIT(PWMB_CCER1, PWMBCCER1_CC5P_MSK)

#define PWMBCCER1_CC6E_MSK              BIT4
#define PWMB_CC6PEnable()               SET_REG_BIT(PWMB_CCER1, PWMBCCER1_CC6E_MSK)
#define PWMB_CC6PDisable()              CLR_REG_BIT(PWMB_CCER1, PWMBCCER1_CC6E_MSK)

#define HSPWMB_CC6PEnable()             SET_HSPWMB_BIT(PWMB_CCER1, PWMBCCER1_CC6E_MSK)
#define HSPWMB_CC6PDisable()            CLR_HSPWMB_BIT(PWMB_CCER1, PWMBCCER1_CC6E_MSK)

#define PWMBCCER1_CC6P_MSK              BIT5
#define PWMB_CC6PNonInverted()          CLR_REG_BIT(PWMB_CCER1, PWMBCCER1_CC6P_MSK)
#define PWMB_CC6PInverted()             SET_REG_BIT(PWMB_CCER1, PWMBCCER1_CC6P_MSK)

#define HSPWMB_CC6PNonInverted()        CLR_HSPWMB_BIT(PWMB_CCER1, PWMBCCER1_CC6P_MSK)
#define HSPWMB_CC6PInverted()           SET_HSPWMB_BIT(PWMB_CCER1, PWMBCCER1_CC6P_MSK)

#define PWMBCCER2_CC7E_MSK              BIT0
#define PWMB_CC7PEnable()               SET_REG_BIT(PWMB_CCER2, PWMBCCER2_CC7E_MSK)
#define PWMB_CC7PDisable()              CLR_REG_BIT(PWMB_CCER2, PWMBCCER2_CC7E_MSK)

#define HSPWMB_CC7PEnable()             SET_HSPWMB_BIT(PWMB_CCER2, PWMBCCER2_CC7E_MSK)
#define HSPWMB_CC7PDisable()            CLR_HSPWMB_BIT(PWMB_CCER2, PWMBCCER2_CC7E_MSK)

#define PWMBCCER2_CC7P_MSK              BIT1
#define PWMB_CC7PNonInverted()          CLR_REG_BIT(PWMB_CCER2, PWMBCCER2_CC7P_MSK)
#define PWMB_CC7PInverted()             SET_REG_BIT(PWMB_CCER2, PWMBCCER2_CC7P_MSK)

#define HSPWMB_CC7PNonInverted()        CLR_HSPWMB_BIT(PWMB_CCER2, PWMBCCER2_CC7P_MSK)
#define HSPWMB_CC7PInverted()           SET_HSPWMB_BIT(PWMB_CCER2, PWMBCCER2_CC7P_MSK)

#define PWMBCCER2_CC8E_MSK              BIT4
#define PWMB_CC8PEnable()               SET_REG_BIT(PWMB_CCER2, PWMBCCER2_CC8E_MSK)
#define PWMB_CC8PDisable()              CLR_REG_BIT(PWMB_CCER2, PWMBCCER2_CC8E_MSK)

#define HSPWMB_CC8PEnable()             SET_HSPWMB_BIT(PWMB_CCER2, PWMBCCER2_CC8E_MSK)
#define HSPWMB_CC8PDisable()            CLR_HSPWMB_BIT(PWMB_CCER2, PWMBCCER2_CC8E_MSK)

#define PWMBCCER2_CC8P_MSK              BIT5
#define PWMB_CC8PNonInverted()          CLR_REG_BIT(PWMB_CCER2, PWMBCCER2_CC8P_MSK)
#define PWMB_CC8PInverted()             SET_REG_BIT(PWMB_CCER2, PWMBCCER2_CC8P_MSK)

#define HSPWMB_CC8PNonInverted()        CLR_HSPWMB_BIT(PWMB_CCER2, PWMBCCER2_CC8P_MSK)
#define HSPWMB_CC8PInverted()           SET_HSPWMB_BIT(PWMB_CCER2, PWMBCCER2_CC8P_MSK)

#define PWMBBRK_MOE_MSK                 BIT7
#define PWMB_EnableMainOutput()         SET_REG_BIT(PWMB_BKR, PWMBBRK_MOE_MSK)
#define PWMB_DisableMainOutput()        CLR_REG_BIT(PWMB_BKR, PWMBBRK_MOE_MSK)

#define HSPWMB_EnableMainOutput()       SET_HSPWMB_BIT(PWMB_BKR, PWMBBRK_MOE_MSK)
#define HSPWMB_DisableMainOutput()      CLR_HSPWMB_BIT(PWMB_BKR, PWMBBRK_MOE_MSK)

#define PWMBBRK_AOE_MSK                 BIT6
#define PWMB_EnableAutoOutput()         SET_REG_BIT(PWMB_BKR, PWMBBRK_AOE_MSK)
#define PWMB_DisableAutoOutput()        CLR_REG_BIT(PWMB_BKR, PWMBBRK_AOE_MSK)

#define HSPWMB_EnableAutoOutput()       SET_HSPWMB_BIT(PWMB_BKR, PWMBBRK_AOE_MSK)
#define HSPWMB_DisableAutoOutput()      CLR_HSPWMB_BIT(PWMB_BKR, PWMBBRK_AOE_MSK)

#define PWMBBRK_BKP_MSK                 BIT5
#define PWMB_BRKNonInverted()           SET_REG_BIT(PWMB_BKR, PWMBBRK_BKP_MSK)
#define PWMB_BRKInverted()              CLR_REG_BIT(PWMB_BKR, PWMBBRK_BKP_MSK)

#define HSPWMB_BRKNonInverted()         SET_HSPWMB_BIT(PWMB_BKR, PWMBBRK_BKP_MSK)
#define HSPWMB_BRKInverted()            CLR_HSPWMB_BIT(PWMB_BKR, PWMBBRK_BKP_MSK)

#define PWMBBRK_BKE_MSK                 BIT4
#define PWMB_BRKEnable()                SET_REG_BIT(PWMB_BKR, PWMBBRK_BKE_MSK)
#define PWMB_BRKDisable()               CLR_REG_BIT(PWMB_BKR, PWMBBRK_BKE_MSK)

#define HSPWMB_BRKEnable()              SET_HSPWMB_BIT(PWMB_BKR, PWMBBRK_BKE_MSK)
#define HSPWMB_BRKDisable()             CLR_HSPWMB_BIT(PWMB_BKR, PWMBBRK_BKE_MSK)

#define PWMBBRK_OSSR_MSK                BIT3
#define PWMBBRK_OSSI_MSK                BIT2
#define PWMB_RunInactiveNoOutput()      CLR_REG_BIT(PWMB_BKR, PWMBBRK_OSSR_MSK)
#define PWMB_RunInactiveOutput()        SET_REG_BIT(PWMB_BKR, PWMBBRK_OSSR_MSK)
#define PWMB_IdleInactiveNoOutput()     CLR_REG_BIT(PWMB_BKR, PWMBBRK_OSSI_MSK)
#define PWMB_IdleInactiveOutput()       SET_REG_BIT(PWMB_BKR, PWMBBRK_OSSI_MSK)

#define HSPWMB_RunInactiveNoOutput()    CLR_HSPWMB_BIT(PWMB_BKR, PWMBBRK_OSSR_MSK)
#define HSPWMB_RunInactiveOutput()      SET_HSPWMB_BIT(PWMB_BKR, PWMBBRK_OSSR_MSK)
#define HSPWMB_IdleInactiveNoOutput()   CLR_HSPWMB_BIT(PWMB_BKR, PWMBBRK_OSSI_MSK)
#define HSPWMB_IdleInactiveOutput()     SET_HSPWMB_BIT(PWMB_BKR, PWMBBRK_OSSI_MSK)

#define PWMBBRK_LOCK_MSK                (BIT1 | BIT0)
#define PWMB_SetLockLevel(n)            MODIFY_REG(PWMB_BKR,  PWMBBRK_LOCK_MSK, ((n) << 0))
#define PWMB_SetUnLock()                PWMB_SetLockLevel(0)

#define HSPWMB_SetLockLevel(n)          MODIFY_HSPWMB(PWMB_BKR,  PWMBBRK_LOCK_MSK, ((n) << 0))
#define HSPWMB_SetUnLock()              HSPWMB_SetLockLevel(0)

#define PWMBOISR_OIS5_MSK               BIT0
#define PWMBOISR_OIS6_MSK               BIT2
#define PWMBOISR_OIS7_MSK               BIT4
#define PWMBOISR_OIS8_MSK               BIT6
#define PWMB_OC5PIdleLevelLow()         CLR_REG_BIT(PWMB_OISR, PWMBOISR_OIS5_MSK)
#define PWMB_OC5PIdleLevelHigh()        SET_REG_BIT(PWMB_OISR, PWMBOISR_OIS5_MSK)
#define PWMB_OC6PIdleLevelLow()         CLR_REG_BIT(PWMB_OISR, PWMBOISR_OIS6_MSK)
#define PWMB_OC6PIdleLevelHigh()        SET_REG_BIT(PWMB_OISR, PWMBOISR_OIS6_MSK)
#define PWMB_OC7PIdleLevelLow()         CLR_REG_BIT(PWMB_OISR, PWMBOISR_OIS7_MSK)
#define PWMB_OC7PIdleLevelHigh()        SET_REG_BIT(PWMB_OISR, PWMBOISR_OIS7_MSK)
#define PWMB_OC8PIdleLevelLow()         CLR_REG_BIT(PWMB_OISR, PWMBOISR_OIS8_MSK)
#define PWMB_OC8PIdleLevelHigh()        SET_REG_BIT(PWMB_OISR, PWMBOISR_OIS8_MSK)

#define HSPWMB_OC5PIdleLevelLow()       CLR_HSPWMB_BIT(PWMB_OISR, PWMBOISR_OIS5_MSK)
#define HSPWMB_OC5PIdleLevelHigh()      SET_HSPWMB_BIT(PWMB_OISR, PWMBOISR_OIS5_MSK)
#define HSPWMB_OC6PIdleLevelLow()       CLR_HSPWMB_BIT(PWMB_OISR, PWMBOISR_OIS6_MSK)
#define HSPWMB_OC6PIdleLevelHigh()      SET_HSPWMB_BIT(PWMB_OISR, PWMBOISR_OIS6_MSK)
#define HSPWMB_OC7PIdleLevelLow()       CLR_HSPWMB_BIT(PWMB_OISR, PWMBOISR_OIS7_MSK)
#define HSPWMB_OC7PIdleLevelHigh()      SET_HSPWMB_BIT(PWMB_OISR, PWMBOISR_OIS7_MSK)
#define HSPWMB_OC8PIdleLevelLow()       CLR_HSPWMB_BIT(PWMB_OISR, PWMBOISR_OIS8_MSK)
#define HSPWMB_OC8PIdleLevelHigh()      SET_HSPWMB_BIT(PWMB_OISR, PWMBOISR_OIS8_MSK)

#define PWMB_SetCounter(n)              (PWMB_CNTRH = HIBYTE(n), PWMB_CNTRL = LOBYTE(n))
#define PWMB_SetClockDivider(n)         (PWMB_PSCRH = HIBYTE(n), PWMB_PSCRL = LOBYTE(n))
#define PWMB_SetReload16(n)             (PWMB_ARRH = HIBYTE(n), PWMB_ARRL = LOBYTE(n))
#define PWMB_SetRepeatCounter(n)        (PWMB_RCRH = HIBYTE(n), PWMB_RCR = LOBYTE(n))

#define HSPWMB_SetCounter(n)            (WRITE_HSPWMB(PWMB_CNTRH, HIBYTE(n)), WRITE_HSPWMB(PWMB_CNTRL, LOBYTE(n)))
#define HSPWMB_SetClockDivider(n)       (WRITE_HSPWMB(PWMB_PSCRH, HIBYTE(n)), WRITE_HSPWMB(PWMB_PSCRL, LOBYTE(n)))
#define HSPWMB_SetReload16(n)           (WRITE_HSPWMB(PWMB_ARRH, HIBYTE(n)), WRITE_HSPWMB(PWMB_ARRL, LOBYTE(n)))
#define HSPWMB_SetRepeatCounter(n)      (WRITE_HSPWMB(PWMB_RCRH, HIBYTE(n)), WRITE_HSPWMB(PWMB_RCR, LOBYTE(n)))

#define PWMB_SetCCR5Value(n)            (PWMB_CCR5H = HIBYTE(n), PWMB_CCR5L = LOBYTE(n))
#define PWMB_SetCCR6Value(n)            (PWMB_CCR6H = HIBYTE(n), PWMB_CCR6L = LOBYTE(n))
#define PWMB_SetCCR7Value(n)            (PWMB_CCR7H = HIBYTE(n), PWMB_CCR7L = LOBYTE(n))
#define PWMB_SetCCR8Value(n)            (PWMB_CCR8H = HIBYTE(n), PWMB_CCR8L = LOBYTE(n))

#define HSPWMB_SetCCR5Value(n)          (WRITE_HSPWMB(PWMB_CCR5H, HIBYTE(n)), WRITE_HSPWMB(PWMB_CCR5L, LOBYTE(n)))
#define HSPWMB_SetCCR6Value(n)          (WRITE_HSPWMB(PWMB_CCR6H, HIBYTE(n)), WRITE_HSPWMB(PWMB_CCR6L, LOBYTE(n)))
#define HSPWMB_SetCCR7Value(n)          (WRITE_HSPWMB(PWMB_CCR7H, HIBYTE(n)), WRITE_HSPWMB(PWMB_CCR7L, LOBYTE(n)))
#define HSPWMB_SetCCR8Value(n)          (WRITE_HSPWMB(PWMB_CCR8H, HIBYTE(n)), WRITE_HSPWMB(PWMB_CCR8L, LOBYTE(n)))

#define PWMB_ReadReload16()             MAKEWORD(PWMB_ARRL, PWMB_ARRH)

#define HSPWMB_ReadReload16()           MAKEWORD(READ_HSPWMB(PWMB_ARRL), READ_HSPWMB(PWMB_ARRH))

#define PWMB_ReadCCR5Value()            MAKEWORD(PWMB_CCR5L, PWMB_CCR5H)
#define PWMB_ReadCCR6Value()            MAKEWORD(PWMB_CCR6L, PWMB_CCR6H)
#define PWMB_ReadCCR7Value()            MAKEWORD(PWMB_CCR7L, PWMB_CCR7H)
#define PWMB_ReadCCR8Value()            MAKEWORD(PWMB_CCR8L, PWMB_CCR8H)

#define HSPWMB_ReadCCR5Value()          MAKEWORD(READ_HSPWMB(PWMB_CCR5L), READ_HSPWMB(PWMB_CCR5H))
#define HSPWMB_ReadCCR6Value()          MAKEWORD(READ_HSPWMB(PWMB_CCR6L), READ_HSPWMB(PWMB_CCR6H))
#define HSPWMB_ReadCCR7Value()          MAKEWORD(READ_HSPWMB(PWMB_CCR7L), READ_HSPWMB(PWMB_CCR7H))
#define HSPWMB_ReadCCR8Value()          MAKEWORD(READ_HSPWMB(PWMB_CCR8L), READ_HSPWMB(PWMB_CCR8H))

#define PWMB_SetDeadTime(n)             PWMB_DTR = LOBYTE(n)

#define HSPWMB_SetDeadTime(n)           WRITE_HSPWMB(PWMB_DTR, LOBYTE(n))

/////////////////////////////////////////////////

#define READ_HSPWMA(r)                  HSPWMA_ReadReg(BYTE0(&r) & 0x7f)
#define WRITE_HSPWMA(r, d)              HSPWMA_WriteReg((BYTE0(&r) & 0x7f), BYTE0(d))
#define MODIFY_HSPWMA(r, clr, set)      WRITE_HSPWMA((r), ((READ_HSPWMA(r) & ~(clr)) | ((set) & (clr))))
#define READ_HSPWMA_BIT(r, b)           (READ_HSPWMA(r) & (b))
#define CLR_HSPWMA_BIT(r, b)            WRITE_HSPWMA((r), (READ_HSPWMA(r) & ~(b)))
#define SET_HSPWMA_BIT(r, b)            WRITE_HSPWMA((r), (READ_HSPWMA(r) | (b)))

#define READ_HSPWMB(r)                  HSPWMB_ReadReg(BYTE0(&r) & 0x7f)
#define WRITE_HSPWMB(r, d)              HSPWMB_WriteReg((BYTE0(&r) & 0x7f), BYTE0(d))
#define MODIFY_HSPWMB(r, clr, set)      WRITE_HSPWMB((r), ((READ_HSPWMB(r) & ~(clr)) | ((set) & (clr))))
#define READ_HSPWMB_BIT(r, b)           (READ_HSPWMB(r) & (b))
#define CLR_HSPWMB_BIT(r, b)            WRITE_HSPWMB((r), (READ_HSPWMB(r) & ~(b)))
#define SET_HSPWMB_BIT(r, b)            WRITE_HSPWMB((r), (READ_HSPWMB(r) | (b)))

////////////////////////

#define HSPWMACFG_ASYNCEN_MSK           (BIT1 | BIT0)
#define HSPWMA_Enable()                 MODIFY_REG(HSPWMA_CFG, HSPWMACFG_ASYNCEN_MSK, ((3) << 0))
#define HSPWMA_Disable()                MODIFY_REG(HSPWMA_CFG, HSPWMACFG_ASYNCEN_MSK, ((0) << 0))

#define HSPWMACFG_INTEN_MSK             BIT2
#define HSPWMA_EnableInt()              SET_REG_BIT(HSPWMA_CFG, HSPWMACFG_INTEN_MSK)
#define HSPWMA_DisableInt()             CLR_REG_BIT(HSPWMA_CFG, HSPWMACFG_INTEN_MSK)

#define HSPWMACFG_EXTN_MSK              BIT3
#define HSPWMA_SelectBankFE80H()        CLR_REG_BIT(HSPWMA_CFG, HSPWMACFG_EXTN_MSK)
#define HSPWMA_SelectBankF900H()        SET_REG_BIT(HSPWMA_CFG, HSPWMACFG_EXTN_MSK)

#define HSPWMAADR_BUSY_MSK              BIT7
#define HSPWMA_CheckBusy()              READ_REG_BIT(HSPWMA_ADR, HSPWMAADR_BUSY_MSK)

#define HSPWMAADR_READ_MSK              BIT7
#define HSPWMA_AsyncRead(r, d)          HSPWMA_ADR = ((r) | HSPWMAADR_READ_MSK);    \
                                        while (HSPWMA_CheckBusy());                 \
                                        (d) = HSPWMA_DAT

#define HSPWMA_AsyncWrite(r, d)         HSPWMA_DAT = (d);                           \
                                        HSPWMA_ADR = ((r) & ~HSPWMAADR_READ_MSK)

////////////////////////

#define HSPWMBCFG_ASYNCEN_MSK           (BIT1 | BIT0)
#define HSPWMB_Enable()                 MODIFY_REG(HSPWMB_CFG, HSPWMBCFG_ASYNCEN_MSK, ((3) << 0))
#define HSPWMB_Disable()                MODIFY_REG(HSPWMB_CFG, HSPWMBCFG_ASYNCEN_MSK, ((0) << 0))

#define HSPWMBCFG_INTEN_MSK             BIT2
#define HSPWMB_EnableInt()              SET_REG_BIT(HSPWMB_CFG, HSPWMBCFG_INTEN_MSK)
#define HSPWMB_DisableInt()             CLR_REG_BIT(HSPWMB_CFG, HSPWMBCFG_INTEN_MSK)

#define HSPWMBCFG_EXTN_MSK              BIT3
#define HSPWMB_SelectBankFE80H()        CLR_REG_BIT(HSPWMB_CFG, HSPWMBCFG_EXTN_MSK)
#define HSPWMB_SelectBankF900H()        SET_REG_BIT(HSPWMB_CFG, HSPWMBCFG_EXTN_MSK)

#define HSPWMBADR_BUSY_MSK              BIT7
#define HSPWMB_CheckBusy()              READ_REG_BIT(HSPWMB_ADR, HSPWMBADR_BUSY_MSK)

#define HSPWMBADR_READ_MSK              BIT7
#define HSPWMB_AsyncRead(r, d)          HSPWMB_ADR = ((r) | HSPWMBADR_READ_MSK);    \
                                        while (HSPWMB_CheckBusy());                 \
                                        (d) = HSPWMB_DAT

#define HSPWMB_AsyncWrite(r, d)         HSPWMB_DAT = (d);                           \
                                        HSPWMB_ADR = ((r) & ~HSPWMBADR_READ_MSK)

/////////////////////////////////////////////////

#define P2PPERI_NONE                    0
#define P2PPERI_UART1                   1
#define P2PPERI_UART2                   2
#define P2PPERI_UART3                   3
#define P2PPERI_UART4                   4
#define P2PPERI_SPI                     5
#define P2PPERI_I2C                     6
#define P2PPERI_LCM                     7
#define P2PPERI_QSPI                    8
#define P2PPERI_ADC                     9
#define P2PPERI_I2S                     10

#define P2PCRn_SRC_MSK                  (BIT_HN)
#define P2PCRn_DEST_MSK                 (BIT_LN)
#define P2PDMA_SetSource(p, n)          MODIFY_REG(DMA_P2P_CR##p, P2PCRn_SRC_MSK, ((n) << 4))
#define P2PDMA_SetDestination(p, n)     MODIFY_REG(DMA_P2P_CR##p, P2PCRn_DEST_MSK, ((n) << 0))

#define DMA_P2PA_SetUART1Receive()      P2PDMA_SetSource(1, P2PPERI_UART1)
#define DMA_P2PA_SetUART2Receive()      P2PDMA_SetSource(1, P2PPERI_UART2)
#define DMA_P2PA_SetUART3Receive()      P2PDMA_SetSource(1, P2PPERI_UART3)
#define DMA_P2PA_SetUART4Receive()      P2PDMA_SetSource(1, P2PPERI_UART4)
#define DMA_P2PA_SetSPIReceive()        P2PDMA_SetSource(1, P2PPERI_SPI)
#define DMA_P2PA_SetI2CReceive()        P2PDMA_SetSource(1, P2PPERI_I2C)
#define DMA_P2PA_SetLCMReceive()        P2PDMA_SetSource(1, P2PPERI_LCM)
#define DMA_P2PA_SetQSPIReceive()       P2PDMA_SetSource(1, P2PPERI_QSPI)
#define DMA_P2PA_SetADCReceive()        P2PDMA_SetSource(1, P2PPERI_ADC)
#define DMA_P2PA_SetI2SReceive()        P2PDMA_SetSource(1, P2PPERI_I2S)

#define DMA_P2PA_SetUART1Send()         P2PDMA_SetDestination(1, P2PPERI_UART1)
#define DMA_P2PA_SetUART2Send()         P2PDMA_SetDestination(1, P2PPERI_UART2)
#define DMA_P2PA_SetUART3Send()         P2PDMA_SetDestination(1, P2PPERI_UART3)
#define DMA_P2PA_SetUART4Send()         P2PDMA_SetDestination(1, P2PPERI_UART4)
#define DMA_P2PA_SetSPISend()           P2PDMA_SetDestination(1, P2PPERI_SPI)
#define DMA_P2PA_SetI2CSend()           P2PDMA_SetDestination(1, P2PPERI_I2C)
#define DMA_P2PA_SetLCMSend()           P2PDMA_SetDestination(1, P2PPERI_LCM)
#define DMA_P2PA_SetQSPISend()          P2PDMA_SetDestination(1, P2PPERI_QSPI)
//#define DMA_P2PA_SetADCSend()         P2PDMA_SetDestination(1, P2PPERI_ADC)
#define DMA_P2PA_SetI2SSend()           P2PDMA_SetDestination(1, P2PPERI_I2S)

#define DMA_P2PB_SetUART1Receive()      P2PDMA_SetSource(2, P2PPERI_UART1)
#define DMA_P2PB_SetUART2Receive()      P2PDMA_SetSource(2, P2PPERI_UART2)
#define DMA_P2PB_SetUART3Receive()      P2PDMA_SetSource(2, P2PPERI_UART3)
#define DMA_P2PB_SetUART4Receive()      P2PDMA_SetSource(2, P2PPERI_UART4)
#define DMA_P2PB_SetSPIReceive()        P2PDMA_SetSource(2, P2PPERI_SPI)
#define DMA_P2PB_SetI2CReceive()        P2PDMA_SetSource(2, P2PPERI_I2C)
#define DMA_P2PB_SetLCMReceive()        P2PDMA_SetSource(2, P2PPERI_LCM)
#define DMA_P2PB_SetQSPIReceive()       P2PDMA_SetSource(2, P2PPERI_QSPI)
#define DMA_P2PB_SetADCReceive()        P2PDMA_SetSource(2, P2PPERI_ADC)
#define DMA_P2PB_SetI2SReceive()        P2PDMA_SetSource(2, P2PPERI_I2S)

#define DMA_P2PB_SetUART1Send()         P2PDMA_SetDestination(2, P2PPERI_UART1)
#define DMA_P2PB_SetUART2Send()         P2PDMA_SetDestination(2, P2PPERI_UART2)
#define DMA_P2PB_SetUART3Send()         P2PDMA_SetDestination(2, P2PPERI_UART3)
#define DMA_P2PB_SetUART4Send()         P2PDMA_SetDestination(2, P2PPERI_UART4)
#define DMA_P2PB_SetSPISend()           P2PDMA_SetDestination(2, P2PPERI_SPI)
#define DMA_P2PB_SetI2CSend()           P2PDMA_SetDestination(2, P2PPERI_I2C)
#define DMA_P2PB_SetLCMSend()           P2PDMA_SetDestination(2, P2PPERI_LCM)
#define DMA_P2PB_SetQSPISend()          P2PDMA_SetDestination(2, P2PPERI_QSPI)
//#define DMA_P2PB_SetADCSend()         P2PDMA_SetDestination(2, P2PPERI_ADC)
#define DMA_P2PB_SetI2SSend()           P2PDMA_SetDestination(2, P2PPERI_I2S)

#define DMA_P2PA_Disable()              WRITE_REG(DMA_P2P_CR1, 0)
#define DMA_P2PB_Disable()              WRITE_REG(DMA_P2P_CR2, 0)

/////////////////////////////////////////////////

#endif



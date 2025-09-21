/*
    NeaPolis Innovation Summer Campus Examples
    [ADC04] Using ADC Peripherals - Example 04
    Copyright (C) 2020-2025 Salvatore Dello Iacono
    Licensed under Apache License 2.0
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

// ======================= DEFINIZIONI =======================
#define MSG_ADC_OK    0x1337
#define MSG_ADC_KO    0x7331

#define BTN_LINE      PAL_LINE(GPIOA, 4U)
#define DEADZONE_SX   30
#define DEADZONE_DX   70

// PWM VIBRAZIONE
#define PWM_TIMER_FREQUENCY 100000
#define PWM_PERIOD          1000
static volatile uint16_t vibrazione = 0;

// Serial
BaseSequentialStream *HC05Master = (BaseSequentialStream*)&SD1;
BaseSequentialStream *chp          = (BaseSequentialStream*)&SD2;

// ADC
#define ADC_GRP_NUM_CHANNELS  2
#define ADC_GRP_BUF_DEPTH     20
static adcsample_t samples[ADC_GRP_NUM_CHANNELS * ADC_GRP_BUF_DEPTH];
static float converted[ADC_GRP_NUM_CHANNELS] = {0.0f};
volatile bool acceso = true;
static thread_reference_t trp = NULL;

// ======================= PWM =======================
static void pwmPeriodCb(PWMDriver *pwmp) { (void)pwmp; palSetLine(LINE_LED_GREEN); }
static void pwmChannelCb(PWMDriver *pwmp) { (void)pwmp; palClearLine(LINE_LED_GREEN); }

static PWMConfig pwmcfg = {
    PWM_TIMER_FREQUENCY,
    PWM_PERIOD,
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED,    NULL},
        {PWM_OUTPUT_DISABLED,    NULL},
        {PWM_OUTPUT_DISABLED,    NULL}
    },
    0, 0, 0
};

static THD_WORKING_AREA(waPWMThread, 128);
static THD_FUNCTION(PWMThread, arg) {
    (void)arg;
    chRegSetThreadName("PWMThread");

    // Configura pin
    palSetPadMode(GPIOB, 4, PAL_MODE_ALTERNATE(2));   // D4 PWM
    palSetPadMode(GPIOA, 8, PAL_MODE_OUTPUT_PUSHPULL); // D7 LED

    pwmStart(&PWMD3, &pwmcfg);

    while (true) {
        pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, vibrazione));
        chThdSleepMilliseconds(500);
    }

    pwmDisableChannel(&PWMD3, 0);
    pwmStop(&PWMD3);
}

// ======================= SERIAL THREAD =======================
#define WA_SERIAL_SIZE 256
THD_WORKING_AREA(waSerial, WA_SERIAL_SIZE);
THD_FUNCTION(thdSerial, arg) {
    (void)arg;
    chRegSetThreadName("SerialThread");

    while (true) {
        char buf;
        size_t n = sdReadTimeout((SerialDriver*)HC05Master, (uint8_t*)&buf, 1, TIME_MS2I(500));
        if (n > 0) {
            chprintf(chp, "Ricevuto: %c\r\n", buf);
            switch (buf) {
                case 'A':
                    palSetPad(GPIOA, 8);
                    chThdSleepMilliseconds(300);
                    break;
                case 'B':
                    palClearPad(GPIOA, 8);
                    chThdSleepMilliseconds(100);
                    break;
                default:
                    palClearPad(GPIOA, 8);
                    chThdSleepMilliseconds(100);
                    break;
            }
        }
        chThdSleepMilliseconds(500);
    }
}

// ======================= GPT4 (trigger ADC) =======================
const GPTConfig gpt4cfg = {
    .frequency = 1000000U,
    .callback  = NULL,
    .cr2       = TIM_CR2_MMS_1,
    .dier      = 0U
};

// ======================= ADC =======================
static void adccallback(ADCDriver *adcp) {
    (void)adcp;
    if (adcIsBufferComplete(adcp)) {
        for (int i = 0; i < ADC_GRP_NUM_CHANNELS; i++) converted[i] = 0.0f;
        for (int i = 0; i < ADC_GRP_NUM_CHANNELS * ADC_GRP_BUF_DEPTH; i++)
            converted[i % ADC_GRP_NUM_CHANNELS] += (float)samples[i] * 100 / 4096;
        for (int i = 0; i < ADC_GRP_NUM_CHANNELS; i++)
            converted[i] /= ADC_GRP_BUF_DEPTH;

        chSysLockFromISR();
        chThdResumeI(&trp, (msg_t)MSG_ADC_OK);
        chSysUnlockFromISR();
    }
}

static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {
    (void)adcp;
    (void)err;
    chSysLockFromISR();
    chThdResumeI(&trp, (msg_t)MSG_ADC_KO);
    chSysUnlockFromISR();
}

const ADCConversionGroup adcgrpcfg = {
    .circular     = true,
    .num_channels = ADC_GRP_NUM_CHANNELS,
    .end_cb       = adccallback,
    .error_cb     = adcerrorcallback,
    .cfgr         = ADC_CFGR_EXTEN_RISING | ADC_CFGR_EXTSEL_SRC(12),
    .cfgr2        = 0U,
    .tr1          = ADC_TR_DISABLED,
    .tr2          = ADC_TR_DISABLED,
    .tr3          = ADC_TR_DISABLED,
    .awd2cr       = 0U,
    .awd3cr       = 0U,
    .smpr         = {ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_247P5) | ADC_SMPR1_SMP_AN2(ADC_SMPR_SMP_247P5), 0U},
    .sqr          = {ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN2), 0U, 0U, 0U}
};

static THD_WORKING_AREA(waAdc, 256);
static THD_FUNCTION(thdAdc, arg) {
    (void)arg;

    palSetPadMode(GPIOA, 0U, PAL_MODE_INPUT_ANALOG);
    palSetPadMode(GPIOA, 1U, PAL_MODE_INPUT_ANALOG);

    gptStart(&GPTD4, &gpt4cfg);
    adcStart(&ADCD1, NULL);
    adcStartConversion(&ADCD1, &adcgrpcfg, samples, ADC_GRP_BUF_DEPTH);
    gptStartContinuous(&GPTD4, 10000);

    while (true) {
        chSysLock();
        msg_t msg = chThdSuspendS(&trp);
        chSysUnlock();

        float chX = converted[0];
        float chY = converted[1];

        if ((uint32_t)msg == MSG_ADC_OK && acceso) {
            if (chX > DEADZONE_DX && chY > DEADZONE_SX && chY < DEADZONE_DX) {
                chprintf(chp, "DX\r\n");
                chprintf(HC05Master, "D\r\n");
                chThdSleepMilliseconds(1000);
            } else if (chX < DEADZONE_SX && chY > DEADZONE_SX && chY < DEADZONE_DX) {
                chprintf(chp, "SX\r\n");
                chprintf(HC05Master, "S\r\n");
                chThdSleepMilliseconds(1000);
            } else if (chY < DEADZONE_SX && chX > DEADZONE_SX && chX < DEADZONE_DX) {
                chprintf(chp, "AVANTI\r\n");
                chprintf(HC05Master, "C\r\n");
                chThdSleepMilliseconds(1000);
            }
        }
    }

    adcStop(&ADCD1);
}

// ======================= MAIN =======================
int main(void) {
    halInit();
    chSysInit();

    // Configurazione seriali
    palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOC, 4, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOC, 5, PAL_MODE_ALTERNATE(7));

    palSetLineMode(BTN_LINE, PAL_MODE_INPUT_PULLUP);

    sdStart(&SD1, NULL);
    sdStart(&SD2, NULL);

    // Attesa pressione bottone
    while (palReadPad(GPIOA, 4) == PAL_HIGH) {
        chprintf(chp, "Button not pressed\r\n");
        chThdSleepMilliseconds(100);
    }
    while (palReadPad(GPIOA, 4) == PAL_LOW) { }

    // Creazione thread
    chThdCreateStatic(waPWMThread, sizeof(waPWMThread), NORMALPRIO, PWMThread, NULL);
    chThdCreateStatic(waAdc, sizeof(waAdc), NORMALPRIO + 5, thdAdc, NULL);
    chThdCreateStatic(waSerial, sizeof(waSerial), NORMALPRIO - 1, thdSerial, NULL);

    while (true) {
        palToggleLine(LINE_LED_GREEN);

        if (palReadPad(GPIOA, 4) == PAL_LOW) {
            acceso = !acceso;
            if (!acceso) {
                chprintf(HC05Master, "P\r\n");
                chprintf(chp, "SOSPESO\r\n");
            }
            while (palReadPad(GPIOA, 4) == PAL_LOW)
                chThdSleepMilliseconds(300);
        }

        chThdSleepMilliseconds(50);
    }

    sdStop(&SD1);
    sdStop(&SD2);
}

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

// ======================= SERIALI =======================
BaseSequentialStream *chp       = (BaseSequentialStream*)&SD2;  // Debug terminale
BaseSequentialStream *HC05Slave = (BaseSequentialStream*)&SD3;  // Bluetooth

// =================== DISTANZA (ULTRASUONI) ===================
#define TRIG_PORT   GPIOA
#define TRIG_PIN    10U
#define ECHO_PORT   GPIOB
#define ECHO_PIN    6U

#define FREQ_INVIO  1000   // ms tra invii

static volatile uint32_t echo_width_us = 0;
static volatile bool echo_new = false;

static void icu_width_cb(ICUDriver *icup) {
  (void)icup;
  echo_width_us = icuGetWidthX(icup);
  echo_new = true;
}

static const ICUConfig icucfg = {
  .mode         = ICU_INPUT_ACTIVE_HIGH,
  .frequency    = 1000000U,  // 1 tick = 1 µs
  .width_cb     = icu_width_cb,
  .period_cb    = NULL,
  .overflow_cb  = NULL,
  .channel      = ICU_CHANNEL_1,
  .dier         = 0
};

static THD_WORKING_AREA(waUltrasuoni, 1024);
static THD_FUNCTION(UltrasuoniThread, arg) {
  (void)arg;
  chRegSetThreadName("Ultrasuoni");

  // Start ICU
  icuStart(&ICUD4, &icucfg);
  palSetPadMode(ECHO_PORT, ECHO_PIN, PAL_MODE_ALTERNATE(2));
  icuStartCapture(&ICUD4);
  icuEnableNotifications(&ICUD4);

  while (true) {
    // Trigger 10 µs
    palClearPad(TRIG_PORT, TRIG_PIN);
    chThdSleepMicroseconds(2);
    palSetPad(TRIG_PORT, TRIG_PIN);
    chThdSleepMicroseconds(10);
    palClearPad(TRIG_PORT, TRIG_PIN);

    // Attesa max 60 ms
    systime_t t0 = chVTGetSystemTime();
    echo_new = false;
    while (!echo_new && TIME_I2MS(chVTTimeElapsedSinceX(t0)) < 60) {
      chThdSleepMilliseconds(1);
    }

    if (echo_new) {
      float dist_cm = (echo_width_us * 0.0343f) * 0.5f;
      if (dist_cm < 400) {
        chprintf(chp, "Distanza = %.2f cm\r\n", dist_cm);

        // Logica vibrazione via Bluetooth
        if (dist_cm <= 150) {
          sdWrite((SerialDriver*)HC05Slave, (uint8_t*)"A", 1);
          chprintf(chp, "Avvertito ostacolo!\r\n");
        } else {
          sdWrite((SerialDriver*)HC05Slave, (uint8_t*)"D", 1);
          chprintf(chp, "Libero\r\n");
        }
      }
    }

    chThdSleepMilliseconds(FREQ_INVIO);
  }
}

// =================== SERVO (PWM + BLUETOOTH) ===================
#define SERVO_STEP  50
#define SERVO_DX    250
#define SERVO_C     700
#define SERVO_SX    1150
#define SERVO_MAX   1200

#define PWM_TIMER_FREQUENCY  100000
#define PWM_PERIOD           (PWM_TIMER_FREQUENCY * 20 / 1000)

static void pwmWidthCb(PWMDriver *pwmp) {
  (void)pwmp;
}

static PWMConfig pwmcfg = {
  .frequency = PWM_TIMER_FREQUENCY,
  .period    = PWM_PERIOD,
  .callback  = pwmWidthCb,
  .channels  = {
    {PWM_OUTPUT_ACTIVE_HIGH, NULL},
    {PWM_OUTPUT_DISABLED,    NULL},
    {PWM_OUTPUT_DISABLED,    NULL},
    {PWM_OUTPUT_DISABLED,    NULL}
  }
};

void servoDestra(void) {
  for (int i = SERVO_C; i >= SERVO_DX; i -= SERVO_STEP) {
    pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, i));
    chThdSleepMilliseconds(20);
    pwmEnableChannel(&PWMD3, 0, 0);
    chThdSleepMilliseconds(100);
  }
  chThdSleepMilliseconds(2000);
  pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, SERVO_C));
  chThdSleepMilliseconds(20);
  pwmEnableChannel(&PWMD3, 0, 0);
}

void servoSinistra(void) {
  for (int i = SERVO_C; i <= SERVO_SX; i += SERVO_STEP) {
    pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, i));
    chThdSleepMilliseconds(20);
    pwmEnableChannel(&PWMD3, 0, 0);
    chThdSleepMilliseconds(100);
  }
  chThdSleepMilliseconds(2000);
  pwmEnableChannel(&PWMD3, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD3, SERVO_C));
  chThdSleepMilliseconds(20);
  pwmEnableChannel(&PWMD3, 0, 0);
}

// ======================= THREAD BLUETOOTH =======================
#define WA_SERIAL_SIZE 256
static THD_WORKING_AREA(waSerial, WA_SERIAL_SIZE);
static THD_FUNCTION(SerialThread, arg) {
  (void)arg;
  chRegSetThreadName("Bluetooth");

  while (true) {
    char buf[5] = {0};
    size_t n = sdReadTimeout((SerialDriver*)HC05Slave, (uint8_t*)buf, 1, TIME_MS2I(500));
    if (n > 0) {
      switch (buf[0]) {
        case 'C':
          chprintf(chp, "Centro\r\n");
          break;
        case 'D':
          chprintf(chp, "Destra\r\n");
          servoDestra();
          break;
        case 'S':
          chprintf(chp, "Sinistra\r\n");
          servoSinistra();
          break;
        case 'P':
          chprintf(chp, "Sospendi\r\n");
          break;
      }
    }
  }
}

// ======================= MAIN =======================
int main(void) {
  halInit();
  chSysInit();

  // Seriali
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7)); // USART2_TX - D1
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7)); // USART2_RX - D0
  sdStart(&SD2, NULL);
  sdStart(&SD3, NULL);

  // Ultrasuoni
  palSetPadMode(TRIG_PORT, TRIG_PIN, PAL_MODE_OUTPUT_PUSHPULL);

  // Servo
  palSetPadMode(GPIOB, 4, PAL_MODE_ALTERNATE(2)); // PWM - D5
  palSetPadMode(GPIOB, 8, PAL_MODE_ALTERNATE(7)); // USART3_TX - D15
  palSetPadMode(GPIOB, 9, PAL_MODE_ALTERNATE(7)); // USART3_RX - D14
  pwmStart(&PWMD3, &pwmcfg);

  // Threads
  chThdCreateStatic(waUltrasuoni, sizeof(waUltrasuoni), NORMALPRIO, UltrasuoniThread, NULL);
  chThdCreateStatic(waSerial, sizeof(waSerial), NORMALPRIO - 1, SerialThread, NULL);

  // Blinking LED
  while (true) {
    palToggleLine(LINE_LED_GREEN);
    chThdSleepMilliseconds(500);
  }
}

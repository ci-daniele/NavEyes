# 🚦 NAVEYES

Questo progetto dimostra l'uso di **ChibiOS** su microcontrollore STM32G474RE per:
- Misurare distanze tramite un **sensore a ultrasuoni** (HC-SR04 o simile).
- Controllare un **servo motore** via **PWM**.
- Comunicare tramite **Bluetooth HC-05**.
- Utilizzare **thread multipli** per gestire logica concorrente (ultrasuoni, servo, comunicazioni seriali).

---

## ⚙️ Funzionalità principali
- **Ultrasuoni**: misurazione distanza in cm, con soglia di sicurezza a 150 cm.
- **Bluetooth**:
  - Invio di comandi di stato (`A` = ostacolo vicino, `D` = libero).
  - Ricezione di comandi remoti (`C` centro, `D` destra, `S` sinistra, `P` pausa).
- **Servo motore**:
  - Movimento controllato a destra e sinistra con ritorno automatico al centro.
- **PWM**:
  - Generazione segnale per il servo.
  - Thread dedicato con gestione LED di stato.
- **LED di bordo**: lampeggio periodico per indicare che il sistema è attivo.

---

## 🧩 Struttura del progetto
- `main.c` → entry point, inizializza HAL, thread e periferiche.
- **Thread**:
  - `UltrasuoniThread`: gestione sensore HC-SR04.
  - `SerialThread`: gestione Bluetooth (ricezione comandi).
- **Funzioni helper**:
  - `servoDestra()` / `servoSinistra()` per il controllo del servo.

---

## 📡 Hardware richiesto
- **STM32G474RE**.
- **HC-SR04** (sensore ultrasuoni).
- **HC-05** (modulo Bluetooth).
- **Servo motore** compatibile con PWM.
- LED onboard (per debug e stato).

---

## 🔌 Pinout
| Periferica       | Pin STM32 | Note            |
|------------------|-----------|-----------------|
| HC-SR04 Trigger  | `PA10`    | Output          |
| HC-SR04 Echo     | `PB6`     | Input (ICU)     |
| Servo PWM        | `PB4`     | PWM (TIM3_CH1)  |
| Bluetooth TX     | `PB8`     | USART3_TX       |
| Bluetooth RX     | `PB9`     | USART3_RX       |
| Debug UART TX    | `PA2`     | USART2_TX       |
| Debug UART RX    | `PA3`     | USART2_RX       |
| LED Verde        | `LINE_LED_GREEN` | onboard |

# 🚦 NAVEYES

Questo progetto dimostra l'uso di **ChibiOS** su microcontrollore STM32G474RE per:
- Misurare distanze tramite un **sensore a ultrasuoni** (HC-SR04 o simile).
- Controllare un **servo motore** via **PWM**.
- Comunicare tramite **Bluetooth HC-05**.
- Utilizzare **thread multipli** per gestire logica concorrente (ultrasuoni, servo, comunicazioni seriali).

---

## ⚙️ Funzionalità principali
- **ADC**:
  - Acquisizione continua su due canali (A0, A1).
  - Media dei valori su buffer circolare.
  - Invio comandi via Bluetooth in base a DEADZONE.
- **Ultrasuoni**: misurazione distanza in cm, con soglia di sicurezza a 150 cm.
- **Bluetooth**:
  - Invio di comandi di stato (`A` = ostacolo vicino, `D` = libero).
  - Ricezione di comandi remoti (`C` centro, `D` destra, `S` sinistra, `P` pausa).
- **Servo motore**:
  - Movimento controllato a destra e sinistra con ritorno automatico al centro.
- **PWM Vibrazione**:
  - Controllo duty cycle tramite variabile `vibrazione`.
  - LED verde per indicazione attività.
- **Button Input**:
  - Attiva/disattiva acquisizione e trasmissione.
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

## 📡 Pinout
| Funzione           | Pin STM32 | Note            |
|-------------------|-----------|----------------|
| ADC1_CH1           | PA0       | A0 analog       |
| ADC1_CH2           | PA1       | A1 analog       |
| PWM Vibrazione     | PB4       | TIM3_CH1        |
| LED Verde          | PA8       | Output          |
| Bottone            | PA4       | Pull-up         |
| Serial HC-05 TX/RX | SD1       | USART1          |
| Serial Debug       | SD2       | USART2          |

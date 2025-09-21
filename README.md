# 🚦 NAVEYES

This project demonstrates the use of **ChibiOS** on an **STM32G474RE microcontroller** to:

- Measure distances using an ultrasonic sensor (HC-SR04 or similar).  
- Control a servo motor via PWM.  
- Communicate via HC-05 Bluetooth module.  
- Use multiple threads to handle concurrent logic (ultrasonics, servo, serial communications).

---

## ⚙️ Main Features

### ADC
- Continuous acquisition on two channels (A0, A1).  
- Averaging values over a circular buffer.  
- Sending commands via Bluetooth based on a DEADZONE.

### Ultrasonic Sensor
- Measures distance in cm, with a safety threshold at 150 cm.

### Bluetooth
- Sends status commands:  
  - `A` = obstacle near  
  - `D` = free  
- Receives remote commands:  
  - `C` = center  
  - `D` = right  
  - `S` = left  
  - `P` = pause

### Servo Motor
- Controlled movement to the right and left with automatic return to center.

### PWM Vibration
- Duty cycle controlled via a vibration variable.  
- Green LED indicates activity.

### Button Input
- Enable/disable acquisition and transmission.  
- Onboard LED: periodic blinking to indicate that the system is active.

---

## 🧩 Project Structure
- `main.c` → Entry point; initializes HAL, threads, and peripherals.
- **Threads**:  
  - `UltrasuoniThread`: manages the HC-SR04 sensor.  
  - `SerialThread`: manages Bluetooth communication.
- **Helper Functions**:  
  - `servoRight()` / `servoLeft()` for servo control.

---

## 📡 Required Hardware
- STM32G474RE  
- HC-SR04 ultrasonic sensor  
- HC-05 Bluetooth module  
- PWM-compatible servo motor  
- Onboard LED (for debugging and status)

---

## 📡 Pinout

| Function           | STM32 Pin | Notes        |
|-------------------|-----------|-------------|
| ADC1_CH1           | PA0       | A0 analog   |
| ADC1_CH2           | PA1       | A1 analog   |
| PWM Vibration      | PB4       | TIM3_CH1    |
| Green LED          | PA8       | Output      |
| Button             | PA4       | Pull-up     |
| Serial HC-05 TX/RX | SD1       | USART1      |
| Serial Debug       | SD2       | USART2      |



IT:
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

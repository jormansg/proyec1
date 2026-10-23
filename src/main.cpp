#include <Arduino.h>
#include <xtensa/core-macros.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "hal_motor.h"
#include "fsm_control.h"

// ---- pines ----
constexpr uint8_t PIN_ESTOP = 33;

// ---- sincronización ISR <-> tarea ----
constexpr uint32_t DEBOUNCE_CYCLES = 4000000UL;   // 25 ms @ 160 MHz
constexpr uint32_t RUN_TIMEOUT_MS  = 10000;

static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
static volatile bool     btnFlag       = false;
static volatile uint32_t pressCount    = 0;
static volatile uint32_t isrLastCycles = 0;
static SemaphoreHandle_t btnSem = nullptr;

// ---- ISR ----
void IRAM_ATTR onButtonISR() {
    uint32_t now = XTHAL_GET_CCOUNT();
    if ((uint32_t)(now - isrLastCycles) < DEBOUNCE_CYCLES) return;
    isrLastCycles = now;

    portENTER_CRITICAL_ISR(&mux);
    btnFlag = true;
    pressCount++;
    portEXIT_CRITICAL_ISR(&mux);

    BaseType_t woken = pdFALSE;
    xSemaphoreGiveFromISR(btnSem, &woken);
    if (woken) portYIELD_FROM_ISR();
}

// ---- tarea FSM ----
void fsmTask(void *) {
    for (;;) {
        Event ev = Event::NONE;
        if (xSemaphoreTake(btnSem, pdMS_TO_TICKS(50)) == pdTRUE) {
            portENTER_CRITICAL(&mux);
            if (btnFlag) { btnFlag = false; ev = Event::BTN_PRESS; }
            portEXIT_CRITICAL(&mux);
        } else if (fsmTimedOut(RUN_TIMEOUT_MS)) {
            ev = Event::TIMEOUT;
        }
        fsmDispatch(ev);
    }
}

void setup() {
    Serial.begin(115200);
    motorInit();
    fsmInit();                                   // mutex
    btnSem = xSemaphoreCreateBinary();           // semáforo

    pinMode(PIN_ESTOP, INPUT_PULLUP);
    isrLastCycles = XTHAL_GET_CCOUNT() - DEBOUNCE_CYCLES;
    attachInterrupt(digitalPinToInterrupt(PIN_ESTOP), onButtonISR, FALLING);   // interrupción

    xTaskCreatePinnedToCore(fsmTask, "fsm", 4096, nullptr, 2, nullptr, 1);     // tarea
}

void loop() {
    static State last = State::COUNT;
    State s = fsmGetState();
    if (s != last) {
        last = s;
        uint32_t n;
        portENTER_CRITICAL(&mux); n = pressCount; portEXIT_CRITICAL(&mux);
        Serial.printf("Estado=%u pulsaciones=%lu\n", (unsigned)s, (unsigned long)n);
    }
}
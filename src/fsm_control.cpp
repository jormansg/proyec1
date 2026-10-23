#include "fsm_control.h"
#include "hal_motor.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

constexpr uint32_t DUTY_RUN = 200;  

static SemaphoreHandle_t fsmMutex = nullptr;
static State    curState  = State::IDLE;
static uint32_t enteredMs = 0;

// ---- acciones: reciben el estado actual y devuelven el siguiente ----
using Handler = State (*)(State);

static State fsmStay(State s)  { return s; }
static State fsmStart(State)   { motorPWM(DUTY_RUN); return State::RUNNING; }
static State fsmFault(State)   { motorBrake();       return State::FAULT;   }
static State fsmReset(State)   { motorStop();        return State::IDLE;    }

// ---- transiciones: filas = estado, columnas = evento ----
constexpr size_t NS = static_cast<size_t>(State::COUNT);
constexpr size_t NE = static_cast<size_t>(Event::COUNT);

//                                   NONE     BTN_PRESS  TIMEOUT
static constexpr Handler FSM[NS][NE] = {
    /* IDLE    */ { fsmStay,  fsmStart,  fsmStay  },
    /* RUNNING */ { fsmStay,  fsmFault,  fsmReset },
    /* FAULT   */ { fsmStay,  fsmReset,  fsmStay  },
};

void fsmInit() { fsmMutex = xSemaphoreCreateMutex(); }

void fsmDispatch(Event e) {
    xSemaphoreTake(fsmMutex, portMAX_DELAY);
    State next = FSM[static_cast<size_t>(curState)][static_cast<size_t>(e)](curState);
    if (next != curState) { curState = next; enteredMs = millis(); }
    xSemaphoreGive(fsmMutex);
}

State fsmGetState() {
    xSemaphoreTake(fsmMutex, portMAX_DELAY);
    State s = curState;
    xSemaphoreGive(fsmMutex);
    return s;
}

bool fsmTimedOut(uint32_t limitMs) {
    xSemaphoreTake(fsmMutex, portMAX_DELAY);
    bool t = (curState == State::RUNNING) && (millis() - enteredMs >= limitMs);
    xSemaphoreGive(fsmMutex);
    return t;
}
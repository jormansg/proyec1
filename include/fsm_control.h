#pragma once
#include <Arduino.h>

// ---- estados ----
enum class State : uint8_t { IDLE, RUNNING, FAULT, COUNT };

// ---- eventos ----
enum class Event : uint8_t { NONE, BTN_PRESS, TIMEOUT, COUNT };

void  fsmInit();                       // crea el mutex
void  fsmDispatch(Event e);            // despacha un evento por la matriz
State fsmGetState();                   // lectura segura
bool  fsmTimedOut(uint32_t limitMs);   // true si lleva más de limitMs en RUNNING
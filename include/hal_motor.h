#pragma once
#include <Arduino.h>

void motorInit();
void motorPWM(uint32_t cicloTrabajo);   // 0..255
void motorStop();                       // inercia (coast)
void motorBrake();                      // frenado dinámico

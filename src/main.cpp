#include <Arduino.h>
#include <xtensa/core-macros.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>



// Mapeo de pines

// sincronizar FreeeRTOS: ISR y Semaforo

// FSM definiciones: States and Events

// Control de interrupciones ISR


//motor 

// motor config PWM

// init motor

// parada emergencia motor


//FSM 

// fsm start funcion
// fsm fault  funcion
// fsm reset funcion
// matriz de transiciones

void setup() {
  // serial 115200
  motorInit();
  //mutex 
  //semaphore
  //interrupt
  //task

}

void loop() {
  // put your main code here, to run repeatedly:
}

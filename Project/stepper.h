#ifndef STEPPER_H
#define STEPPER_H

#define MICROSTEP_COEFFICIENT 16 // Microstepping division factor to control stepper
#define PAIRS_OF_PHASES 2        // Number of phases of stepper

Bool STEPPER_DIR;

void Stepper_NoneMicroMove( STEPPER_MODE, float, uint16_t, Bool);  // Not recommended because of delay
void Stepper_NoneMicroMove_Once( STEPPER_MODE, short int);  // Use to move only one step.
void Stepper_MicroMove(uchar, uint16_t, float, float);
void Stepper_DePower();

#endif

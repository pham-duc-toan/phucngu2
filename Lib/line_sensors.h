#ifndef LINE_SENSORS_H
#define LINE_SENSORS_H
#include "board.h"

void LineSensors_CalibInit(void);
void LineSensors_UpdateCalib(const uint16_t* snap);
int  LineSensors_CalibQuality(void);  // Ki?m tra ch?t l??ng hi?u chu?n
int  LineSensors_ComputeError(const uint16_t* snap);  // �m: l?ch tr�i, duong: l?ch ph?i

#endif

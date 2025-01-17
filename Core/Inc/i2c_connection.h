/*
 * i2c_connection.h
 *
 *  Created on: Jan 12, 2025
 *      Author: mateu
 */

#ifndef INC_I2C_CONNECTION_H_
#define INC_O2C_CONNECTION_H_

#include <stdint.h>

extern _Bool stan_bitmapy;
extern _Bool stan_inwersji;
extern _Bool stan_rozpoczecia_inwersji;
extern uint32_t odliczanie_animacji;
extern const uint16_t czas_animacji;
extern uint32_t odliczenie_inwersji;
extern const uint8_t czas_inwersji;
extern uint16_t licznik_migniec;

void inwersja3x();
void oled();

#endif /* INC_O2C_CONNECTION_H_ */

/*
 * i2c_connection.c
 *
 *  Created on: Jan 12, 2025
 *      Author: mateu
 */

#include "i2c_connection.h"
#include "GFX.h"
#include "bitmapy.h"

_Bool stan_bitmapy;
_Bool stan_inwersji;
_Bool stan_rozpoczecia_inwersji;
uint32_t odliczanie_animacji;
const uint16_t czas_animacji = 500;
uint32_t odliczenie_inwersji;
const uint8_t czas_inwersji = 100;
uint16_t licznik_migniec;

void inwersja3x() {
  if ( stan_rozpoczecia_inwersji) {
    uint32_t aktualnyCzas = HAL_GetTick();

    if (aktualnyCzas - odliczenie_inwersji >= czas_inwersji) {
    	odliczenie_inwersji = aktualnyCzas;
    	stan_inwersji = !stan_inwersji;

    	SSD1306_display_invert(stan_inwersji);

      if (!stan_inwersji) {
    	  licznik_migniec++;
      }

      if (licznik_migniec >= 2) {
    	stan_rozpoczecia_inwersji = false;
        SSD1306_display_invert(false);
      }
    }
  }
}

void oled(int war_odczytana, int war_aktualna, int war_pwm) {
	uint32_t aktualnyCzas = HAL_GetTick();

  //Animacja bitmap
	if (aktualnyCzas - odliczanie_animacji >= czas_animacji) {
	    odliczanie_animacji = aktualnyCzas;

    //display.clearDisplay();

    if (stan_bitmapy) {
    	GFX_draw_Bitmap(0, 0, wiatrak1, 128, 64, WHITE, BLACK);
    } else {
    	GFX_draw_Bitmap(0, 0, wiatrak2, 128, 64, WHITE, BLACK);
    }

    stan_bitmapy = !stan_bitmapy;


    char buffer_o[16];
    sprintf(buffer_o, "%d", war_odczytana);

    char buffer_a[16];
    sprintf(buffer_a, "%d", war_aktualna);

    char buffer_p[16];
    sprintf(buffer_p, "%d", war_pwm);

    //128 64

    GFX_draw_string(0, 56, (unsigned char *)"Zadana:", WHITE, BLACK, 1,1);
    GFX_draw_string(0, 36, (unsigned char *)buffer_o, WHITE, BLACK, 2, 2);
    GFX_draw_string(42, 36, (unsigned char *)"cm", WHITE, BLACK, 2, 2);

    GFX_draw_string(0, 20, (unsigned char *)"Aktualna:", WHITE, BLACK, 1,1);
    GFX_draw_string(0, 0, (unsigned char *)buffer_a, WHITE, BLACK, 2, 2);
    GFX_draw_string(42, 0, (unsigned char *)"cm", WHITE, BLACK, 2, 2);

    GFX_draw_string(76, 20, (unsigned char *)"PWM:", WHITE, BLACK, 1,1);
    GFX_draw_string(76, 0, (unsigned char *)buffer_p, WHITE, BLACK, 2, 2);
    GFX_draw_string(120, 0, (unsigned char *)"%", WHITE, BLACK, 2, 2);


    SSD1306_display_repaint();}
}

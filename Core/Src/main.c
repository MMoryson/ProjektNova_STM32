/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "eth.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/*
 * Jest to regulator PI wiec:
 * sygnal refenercyjny - 	czyjnik			- dystans
 * sygnal wejsciowy - 		enkoder			- pozycja
 * sygnal wyjsciowy - 		wypelnienie pwm - wypelnienie
 */

#include <stdio.h>
#include <string.h>
#include "GFX.h"
#include "bitmapy.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t znak, komunikat[20];
uint16_t dl_kom;

uint16_t wypelnienie = 0;
uint8_t pwm_send = 0;

char msg[64];

uint32_t counter;
int16_t count;
int16_t pozycja = 20;
int16_t pozycja_send=20;

char msg_dyst[64];
uint32_t pMillis;
uint32_t Value1 = 0;
uint32_t Value2 = 0;
uint16_t dystans  = 0;
uint16_t dystans_temp = 0;

uint32_t previousTime = 0;

typedef struct {
    float Kp;      // Wzmocnienie proporcjonalne
    float Ki;      // Wzmocnienie całkujące
    float calka; // Skumulowany błąd
    float zadana; // Wartość zadana
} Reg_PI;

Reg_PI nasz_PI = {
    .Kp = 1.0f,  // Ustawienia wzmocnienia
    .Ki = 0.1f,
    .calka = 0.0f,
    .zadana = 0.0f // Przykładowa wartość zadana
};

float dt = 0.01f;
float wynik;
float wynik_temp;
float uchyb;
int ograniczenie_calki = 20;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == P2_Pin)
	{
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
		pozycja = pozycja_send;

		//SSD1306_display_invert(true);
		//HAL_Delay(10);
		//SSD1306_display_invert(false);
	}
	if (GPIO_Pin == Echo_Pin)
	{
		pMillis = HAL_GetTick();
		while (!(HAL_GPIO_ReadPin (Echo_GPIO_Port, Echo_Pin)) && pMillis + 10 >  HAL_GetTick());
		Value1 = __HAL_TIM_GET_COUNTER (&htim4);

		pMillis = HAL_GetTick();
		while ((HAL_GPIO_ReadPin (Echo_GPIO_Port, Echo_Pin)) && pMillis + 50 > HAL_GetTick());
		Value2 = __HAL_TIM_GET_COUNTER (&htim4);
		dystans_temp = (Value2-Value1)* 0.034/2;
		if(abs(dystans_temp - dystans)>100)
		{
			dystans = saturacja(dystans, 10, 100);
		}
		else{
			dystans = saturacja(dystans_temp, 10, 100);
		}


		/*
		int tx_msg_len_tim = sprintf((char *)msg_dyst, "dystans: %d cm\r\n", dystans);
		HAL_UART_Transmit(&huart3, (uint8_t *)msg_dyst, tx_msg_len_tim, 100);
		*/

	 }

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART3)
	{

		int enkoder = strtol((char*)&znak, 0, 10);
		pozycja = saturacja(enkoder, 20, 100);

		HAL_UART_Transmit_IT(&huart3, komunikat, dl_kom);
		HAL_UART_Receive_IT(&huart3, &znak, 3);
	}
}

int wrap_encoder(int count, int min, int max, int shift, TIM_HandleTypeDef *htim)
{
    if (count < min * 4) {
        count += (max - min) * 4;
        __HAL_TIM_SET_COUNTER(htim, count);
        return min;
    }
    if (count > max * 4) {
        count -= (max - min) * 4;
        __HAL_TIM_SET_COUNTER(htim, count);
        return max;
    }
    return count / 4 + shift;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

	//static uint32_t last_send_time = 0;

	counter = __HAL_TIM_GET_COUNTER(&htim3);
	count = (int16_t)counter;

	pozycja_send = wrap_encoder(count, 0, 80, 20, &htim3);

	/*if (HAL_GetTick() - last_send_time >= 100)
	{
		last_send_time = HAL_GetTick();
		uint8_t tx_buffer[32];
		int tx_msg_len = sprintf((char*)tx_buffer, "Encoder counter: %d\n\r", pozycja);
		HAL_UART_Transmit(&huart3, tx_buffer, tx_msg_len, 100);
	}*/

}

void uTime(int time)
{
	HAL_GPIO_WritePin(Trigger_GPIO_Port, Trigger_Pin, GPIO_PIN_SET);
	__HAL_TIM_SET_COUNTER(&htim4, 0);
	while (__HAL_TIM_GET_COUNTER(&htim4) < time);
	HAL_GPIO_WritePin(Trigger_GPIO_Port, Trigger_Pin, GPIO_PIN_RESET);
}

int saturacja(int wartosc, int min, int max)
{

	if(wartosc > max)
	{
		wartosc = max;
	}
	if(wartosc < min)
	{
		wartosc = min;
	}
	return wartosc;
}


float obliczanie_PI(Reg_PI *regulator, float dystans, float dt, int ograniczenie) {
    // Obliczanie błędu
    uchyb = regulator->zadana - dystans;

    // Część całkująca
    regulator->calka += uchyb*dt;

    float P = regulator->Kp * uchyb;
    float I;

    if (100*abs(uchyb)/regulator->zadana > ograniczenie)
    {
    	regulator->calka= 0;
    }

    I = regulator->Ki * regulator->calka;

    return  P+I;
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ETH_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart3, &znak, 3);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  HAL_TIM_Encoder_Start_IT(&htim3, TIM_CHANNEL_ALL);

  SSD1306_init();
  GFX_draw_fill_rect(0, 0, 2*64, 2*32, BLACK);
  //GFX_draw_fill_rect(64, 32, 64, 32, WHITE);
  //GFX_draw_char(10, 10, 'a', WHITE, BLACK, 2, 2);
  //GFX_draw_string(0, 0, (unsigned char *)"Dupia", WHITE, BLACK, 2,2);
  //SSD1306_display_invert(true);
  //FX_draw_Bitmap(0, 0, wiatrak1, 128, 64, WHITE, BLACK);
  //GFX_draw_string(0, 0, (unsigned char *)"Dupia", WHITE, BLACK, 2,2);
  SSD1306_display_repaint();


  HAL_TIM_Base_Start_IT(&htim4);
  HAL_GPIO_WritePin(Trigger_GPIO_Port, Trigger_Pin, GPIO_PIN_RESET);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //CZUJNIK
	  uTime(10);
	  nasz_PI.zadana = pozycja;
	  wynik_temp = obliczanie_PI(&nasz_PI, (float)dystans, dt, ograniczenie_calki);

	  wynik = saturacja(wynik_temp,0, 999);
	  wypelnienie = (uint16_t)wynik;
	  pwm_send = (uint8_t)(100*wynik/999);
	  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, wypelnienie);




		 if (HAL_GetTick() - previousTime >= 500) // Sprawdź, czy minęło 100 ms
		    {
		        previousTime = HAL_GetTick(); // Zapisz aktualny czas

		        int tx_msg_len = sprintf((char *)msg_dyst, "Distance: %d cm, Pozycja: %d , PWM: %d , Encoder counter: %d \n\r", dystans, pozycja, wypelnienie, pozycja_send);
		        HAL_UART_Transmit(&huart3, (uint8_t *)msg_dyst, tx_msg_len, 100);
		    }

		  HAL_Delay(100);

		  /*
		  //PWM
		 	  wypelnienie = wypelnienie + 1;
		 	  if (wypelnienie > 998){wypelnienie = 0;}
		 	  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, wypelnienie);
		 	  pwm_send = wypelnienie;
		 	  */

		/*
		 		int tx_msg_len_tim = sprintf((char *)msg_dyst, "dystans: %d cm\r\n", dystans);
		HAL_UART_Transmit(&huart3, (uint8_t *)msg_dyst, tx_msg_len_tim, 100);
		 if (HAL_GetTick() - last_send_time >= 100)
		 	{
		 		last_send_time = HAL_GetTick();
		 		uint8_t tx_buffer[32];
		 		int tx_msg_len = sprintf((char*)tx_buffer, "Encoder counter: %d\n\r", pozycja);
		 		HAL_UART_Transmit(&huart3, tx_buffer, tx_msg_len, 100);
		 	}*/


	  /*
	  if(HAL_GPIO_ReadPin(P2_GPIO_Port, P2_Pin))
		 {
			 HAL_Delay(10);
			 HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		 }
	  */

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

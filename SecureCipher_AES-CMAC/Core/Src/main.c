/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/wolfssl/wolfcrypt/aes.h"
#include "../Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/wolfssl/wolfcrypt/cryptocb.h"
#include "../Middlewares/Third_Party/wolfSSL_wolfSSL_wolfSSL/wolfssl/wolfssl/wolfcrypt/cmac.h"
/* USER CODE END Includes */
/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */

//typedef struct {
//    Aes aes;
//    byte buffer[AES_BLOCK_SIZE]; /* partially stored block */
//    byte digest[AES_BLOCK_SIZE]; /* running digest */
//    byte k1[AES_BLOCK_SIZE];
//    byte k2[AES_BLOCK_SIZE];
//    word32 bufferSz;
//    word32 totalSz;
//}Cmac;

/* used to encrypt end decrypt message*/
#if 1  /* AES */

    #define AES_128_KEY_SIZE 16
	#define AES_128_IV_SIZE 16

	Aes enc;  // Workspace
	Aes dec;  // Workspace
	void* hint = NULL;
	int devId = INVALID_DEVID;

	int EncRet = 0;
	uint8_t AES_key[AES_128_KEY_SIZE] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
	uint8_t iv[AES_128_IV_SIZE]  = {0x0, 0x1, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};

	// Multiple of 16 bytes
	uint8_t UART_msg[AES_BLOCK_SIZE] = {0x0, 0x1, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
	uint8_t UART_msgCipher[AES_BLOCK_SIZE] = {0x00};
	uint8_t UART_msgPlain [AES_BLOCK_SIZE] = {0x00};

	int DecRet = 0;

#endif

/* CMAC (MAC) used to integrity and authentication */
#if 1  /* CMAC  */

    #define CMAC_128_KEY_SIZE 16
	#define CMAC_SIZE 8
	#define MSG_SIZE  8
	Cmac cmac[1];
	uint8_t key[CMAC_128_KEY_SIZE] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
	uint8_t msg[MSG_SIZE] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
	uint8_t mac[CMAC_SIZE];
	word32 macSize = 8;  // word32 = unsigned int
	int CmacRet = 0;
	int onSuccessFlag = 0;

#endif




/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

#if 1
     /* For Sender (Encrypt and Decrypt message using AES Algorithms */
     EncRet = wc_AesInit(&enc, hint, devId);
     if(EncRet == 0){
    	  EncRet |= wc_AesSetKey(&enc, AES_key, AES_BLOCK_SIZE, iv, AES_ENCRYPTION);
    	  if(EncRet == 0){
    		  EncRet |= wc_AesSetIV(&enc, iv);
    		  if(EncRet == 0){
    			  EncRet |= wc_AesCbcEncrypt(&enc, UART_msgCipher, UART_msg, sizeof(UART_msg));
    			  if(EncRet == 0){
    				  CmacRet = wc_InitCmac(cmac, key, CMAC_128_KEY_SIZE, NULL);
    				  CmacRet |= wc_AesCmacGenerate(mac, &macSize, UART_msgCipher, sizeof(UART_msgCipher), key, CMAC_128_KEY_SIZE);
    			  }
    		  }
    	  }

     }

     /* For Receiver (Verification) */
     if(CmacRet == 0){
    	 CmacRet |= wc_AesCmacVerify(mac, macSize, UART_msgCipher,  sizeof(UART_msgCipher), key, CMAC_128_KEY_SIZE);
    	 if(CmacRet = 0){
    		 EncRet = wc_AesInit(&dec, hint, devId);
    		 EncRet |= wc_AesSetKey(&dec, AES_key, AES_BLOCK_SIZE, iv, AES_DECRYPTION);
    		 EncRet |= wc_AesSetIV(&dec, iv);
    		 if(EncRet == 0){
    			 EncRet |= wc_AesCbcDecrypt(&dec, UART_msgPlain, UART_msgCipher, sizeof(UART_msgCipher));
    			 if(EncRet == 0){
    				 DecRet = 1;
    			 }
    		 }
    	 }
     }

#endif
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

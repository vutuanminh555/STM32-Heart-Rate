/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "MAX30102.h"
#include "printf.h"
#include "algorithm_by_RF.h"
#include "graphics.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define MAX_HEART_BEAT_TRACE 		140.0f
#define MY_SNPRINTF 		snprintf

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

/* USER CODE BEGIN PV */
static uint32_t aun_ir_buffer[BUFFER_SIZE];		// dữ liệu từ hồng ngoại
static uint32_t aun_red_buffer[BUFFER_SIZE];	// dữ liệu từ LED đỏ

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */
static void Max30102Setup(void); // Khai báo hàm sử dụng
static void Max30102Loop(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Max30102Setup()
{
  uint8_t uch_dummy;

  maxim_max30102_reset(); // reset cảm biến
  maxim_max30102_read_reg(REG_INTR_STATUS_1, &uch_dummy);  // đọc / xóa thanh ghi interrupt status
  maxim_max30102_init(); // Khởi tạo các giá trị ban đầu
}

// Lấy mẫu từ cảm biến MAX30102. Nhịp tim và SpO2 được tính toán mỗi ST giây
void Max30102Loop()
{
  char buf[20];
  float n_spo2;
  float ratio;
  float correl;
  int8_t ch_spo2_valid;  				// Chỉ số xác định tính toán SpO2 chính xác hay không
  int32_t n_heart_rate; 				// Giá trị nhịp tim
  int8_t  ch_hr_valid;  				// Chỉ số xác định tính toán nhịp tim chính xác hay không
  uint8_t i;
  static uint32_t un_min = 0x3FFFFUL;	// Giá trị khởi tạo của un_min và un_max
  static uint32_t un_max = 0UL;
  static uint32_t un_prev_data = 0UL;  	// Giá trị để tính độ sáng LED dùng cho nhịp tim
  static float f_heartbeatTrace = 0UL;
  float f_temp;

  static int16_t x = 1;
  static int16_t lastY = 35;

  // Lấy mẫu tín hiệu trong BUFFER_SIZE
  for (i = 0U; i < BUFFER_SIZE; i++)
  {
	while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_SET);	// Không đọc dữ liêu khi chân interrupt = 1
    maxim_max30102_read_fifo((aun_red_buffer + i), (aun_ir_buffer + i));  // Đọc dữ liệu IRLED và REDLED từ MAX30102 FIFO
    // Tính toán heartbeatTrace
    if (aun_red_buffer[i] > un_prev_data) // Nếu giá trị hiện tại lớn hơn giá trị trước đó
    {
      f_temp = aun_red_buffer[i] - un_prev_data;
      f_temp /= (un_max - un_min);
      f_temp *= MAX_HEART_BEAT_TRACE;
      f_heartbeatTrace -= f_temp;
      if (f_heartbeatTrace < -25.0f) // giá trị tối thiểu của heartbeatTrace = -25.0f
      {
        f_heartbeatTrace = -25.0f;
      }
    }
    else // nếu giá trị hiện tại nhỏ hơn giá trị trước đó
    {
	  f_temp = un_prev_data - aun_red_buffer[i];
	  f_temp /= (un_max - un_min);
	  f_temp *= MAX_HEART_BEAT_TRACE;
	  f_heartbeatTrace += f_temp;
	  if (f_heartbeatTrace > MAX_HEART_BEAT_TRACE + 25.0f) // giá trị tối đa của heartbeatTrace = MAX_HEART_BEAT_TRACE + 25.0f
	  {
		f_heartbeatTrace = MAX_HEART_BEAT_TRACE + 25.0f;
	  }
    }

    // hiển thị đồ thị lên màn hình
    GraphicsLine(x, 210 - lastY, x + 3, 210 - ((int16_t)f_heartbeatTrace + 30), CYAN);
    lastY = (int16_t)f_heartbeatTrace + 30; // tín hiệu đọc được
    x += 3; // hiển thị theo thời gian
    if (x == 238) // Nếu đến cuối bên phải màn hình thì reset màn hình và bắt đầu lại từ bên trái
    {
    	GraphicsFilledRectangle(1, 1, 238, 208, BLACK);
    	x = 1;
    }

    un_prev_data = aun_red_buffer[i]; // lưu dữ liệu hiện tại để dùng cho vòng lặp sau
  }

  un_min = 0x3FFFFUL; // reset lại giá trị để sử dụng làm tham chiếu
  un_max = 0UL;
  // cập nhật un_min và un_max cho chu kỳ tiép theo
  for (i = 0U; i < BUFFER_SIZE; i++)
  {
    if (un_min > aun_red_buffer[i])
    {
      un_min = aun_red_buffer[i];
    }
    if (un_max < aun_red_buffer[i])
    {
      un_max = aun_red_buffer[i];
    }
  }

  // Tính toán giá trị nhịp tim và SpO2 sau BUFFER_SIZE thời gian lấy mẫu sử dụng thuật toán của Robert
  rf_heart_rate_and_oxygen_saturation(aun_ir_buffer, BUFFER_SIZE, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid, &ratio, &correl);

  // Hiển thị giá trị số
  GraphicsFilledRectangle(10, 212, 229, 28, BLACK);
  if (ch_hr_valid && ch_spo2_valid) // nếu giá trị tính được là chính xác
  {
	  colour_t spo2 = GREEN;
	  colour_t heart_rate = GREEN;

	if(n_heart_rate > 100 && n_heart_rate <=120)
		heart_rate = YELLOW;
	if(n_heart_rate >120)
		heart_rate = RED;
	if(n_spo2 < 95 && n_spo2 >= 90)
		spo2 = YELLOW;
	if(n_spo2 < 90 )
		spo2 = RED;

    MY_SNPRINTF(buf, (size_t)20, "SpO2: %3.2f%%", n_spo2);
    GraphicsLargeString(10, 212, buf, spo2);
    snprintf(buf, (size_t)20, "Heart rate: %d b/m", n_heart_rate);
    GraphicsLargeString(10, 227, buf, heart_rate);
	}

  else
  {
	  GraphicsLargeString(10, 220, "Not Valid", LIGHT_ORANGE);
  }
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
  MX_I2C1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  Max30102Setup();
  SPI_1LINE_TX(&hspi1);
  GraphicsInit();
  GraphicsClear(BLACK);
  GraphicsRectangle(0, 0, 240, 210, WHITE);
  GraphicsLargeString(10, 220, "Sampling", BRICK_RED);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  Max30102Loop();
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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

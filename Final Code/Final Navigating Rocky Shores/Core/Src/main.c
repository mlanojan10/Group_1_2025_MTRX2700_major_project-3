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
#include "ptu_definitions.h"
#include "ptu_i2c.h"
#include "serial.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

/* Constants ------------------------------------------------------------------*/
// PWM boundaries for servo motors
#define SERVO_MIN_PWM             1000
#define SERVO_CENTER_PWM          1500
#define SERVO_MAX_PWM             2000
#define PWM_STEP_SIZE             1     // Smaller = slower PTU movement
#define PWM_PERIOD_US             20000 // 20ms period = 50Hz for hobby servo

// Joystick ADC range
#define ADC_MIN                   0
#define ADC_MAX                   4095

// Distance filtering
#define DISTANCE_FILTER_SIZE      5
#define NO_DETECTION_MM           300
#define NO_DETECTION_THRESHOLD    50
#define NO_DETECTION_COUNT_LIMIT  3

// LIDAR distance LED thresholds
#define OBSTACLE_THRESHOLD_MM     300

// Delay between control loop iterations
#define CONTROL_LOOP_DELAY_MS     20

/* Global Variables ----------------------------------------------------------*/

// PWM values for PTU
static uint16_t current_pan_pwm  = SERVO_CENTER_PWM;
static uint16_t current_tilt_pwm = SERVO_CENTER_PWM;

// Distance filtering buffer
uint32_t distance_buffer[DISTANCE_FILTER_SIZE] = {0};
uint8_t distance_index = 0;

// LIDAR reading
volatile uint32_t lidar_distance_mm = 0;

// Joystick readings
volatile uint32_t joy_x = 0;
volatile uint32_t joy_y = 0;

/* Function Declarations -----------------------------------------------------*/
int map(int val, int in_min, int in_max, int out_min, int out_max);
uint32_t get_filtered_distance(void);
void enable_clocks(void);
void initialise_board(void);

/* HAL Handles ---------------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
PCD_HandleTypeDef hpcd_USB_FS;

/* Utility Functions ---------------------------------------------------------*/

// Maps a value from one range to another
int map(int val, int in_min, int in_max, int out_min, int out_max) {
    return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Moving average distance filter
uint32_t get_filtered_distance(void) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < DISTANCE_FILTER_SIZE; i++) {
        sum += distance_buffer[i];
    }
    return sum / DISTANCE_FILTER_SIZE;
}

// Enable GPIO clocks
void enable_clocks() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOEEN;
}

// Configure Discovery board LEDs as output (PE8–PE15)
void initialise_board() {
    uint16_t *led_output_registers = ((uint16_t *)&(GPIOE->MODER)) + 1;
    *led_output_registers = 0x5555;
}



/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USB_PCD_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM1_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */


typedef union {
	uint8_t all_leds;
	struct {
		uint8_t led_pair_1 : 2;
		uint8_t led_pair_2 : 2;
		uint8_t led_set_of_4 : 4;
	} led_groups;
} LedRegister;



uint16_t last_capture = 0;
uint16_t diff = 0;

uint16_t rise_time = 0;
uint16_t last_period = 0;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    static uint32_t rising_edge_time = 0;
    static bool last_edge_was_rising = true;
    static uint8_t no_detection_counter = 0;

    if (htim->Instance == TIM1) // Only handle TIM1 input capture
    {
        if (last_edge_was_rising)
        {
            // Store rising edge timestamp
            rising_edge_time = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
            last_edge_was_rising = false;
        }
        else
        {
            uint32_t falling_edge_time = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
            uint32_t pulse_width;

            // Handle timer wrap-around
            if (falling_edge_time >= rising_edge_time) {
                pulse_width = falling_edge_time - rising_edge_time;
            } else {
                pulse_width = (htim->Instance->ARR - rising_edge_time + falling_edge_time + 1);
            }

            // Convert pulse width to mm
            lidar_distance_mm = pulse_width / 10;

            // Handle missed/no detections
            if (pulse_width < NO_DETECTION_THRESHOLD) {
                no_detection_counter++;
                if (no_detection_counter >= NO_DETECTION_COUNT_LIMIT) {
                    for (int i = 0; i < DISTANCE_FILTER_SIZE; i++) {
                        distance_buffer[i] = NO_DETECTION_MM;
                    }
                    distance_index = 0;
                }
            } else {
                no_detection_counter = 0;
                distance_buffer[distance_index++] = lidar_distance_mm;
                if (distance_index >= DISTANCE_FILTER_SIZE) distance_index = 0;
            }

            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
            last_edge_was_rising = true;
        }
    }
}

/* Main Entry Point ----------------------------------------------------------*/

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    // HAL peripheral initializations
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USB_PCD_Init();
    MX_TIM2_Init();
    MX_TIM1_Init();
    MX_ADC1_Init();
    MX_ADC2_Init();

    // Custom peripheral setups
    enable_clocks();
    initialise_board();
    initialise_ptu_i2c(&hi2c1);
    SerialInitialise(BAUD_115200, &USART1_PORT, 0x00);

    // Start PWM for PTU
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    TIM2->ARR = PWM_PERIOD_US;
    TIM2->CR1 |= TIM_CR1_ARPE;

    // Start input capture for LIDAR
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);

    // Initialize distance buffer to no detection value
    for (int i = 0; i < DISTANCE_FILTER_SIZE; i++) {
        distance_buffer[i] = NO_DETECTION_MM;
    }

    /* Main Loop -------------------------------------------------------------*/
    while (1)
    {
        // --- Joystick Input (X and Y) ---
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
        joy_x = HAL_ADC_GetValue(&hadc1);

        HAL_ADC_Start(&hadc2);
        HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);
        joy_y = HAL_ADC_GetValue(&hadc2);

        // --- Map ADC to PWM targets ---
        uint16_t target_pan_pwm  = map(joy_x, ADC_MIN, ADC_MAX, SERVO_MIN_PWM, SERVO_MAX_PWM);
        uint16_t target_tilt_pwm = map(joy_y, ADC_MIN, ADC_MAX, SERVO_MIN_PWM, SERVO_MAX_PWM);

        // --- Smooth Pan Movement ---
        if (current_pan_pwm < target_pan_pwm) {
            current_pan_pwm += PWM_STEP_SIZE;
            if (current_pan_pwm > target_pan_pwm) current_pan_pwm = target_pan_pwm;
        } else if (current_pan_pwm > target_pan_pwm) {
            current_pan_pwm -= PWM_STEP_SIZE;
            if (current_pan_pwm < target_pan_pwm) current_pan_pwm = target_pan_pwm;
        }

        // --- Smooth Tilt Movement ---
        if (current_tilt_pwm < target_tilt_pwm) {
            current_tilt_pwm += PWM_STEP_SIZE;
            if (current_tilt_pwm > target_tilt_pwm) current_tilt_pwm = target_tilt_pwm;
        } else if (current_tilt_pwm > target_tilt_pwm) {
            current_tilt_pwm -= PWM_STEP_SIZE;
            if (current_tilt_pwm < target_tilt_pwm) current_tilt_pwm = target_tilt_pwm;
        }

        // --- Apply PWM values to PTU ---
        TIM2->CCR1 = current_pan_pwm;
        TIM2->CCR2 = current_tilt_pwm;

        // --- Distance Filtering & LED Indicator ---
        uint32_t filtered_distance = get_filtered_distance();

        if (filtered_distance > 0 && filtered_distance < OBSTACLE_THRESHOLD_MM) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);   // Red ON
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET); // Green OFF
        } else {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);   // Green ON
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET); // Red OFF
        }

        HAL_Delay(CONTROL_LOOP_DELAY_MS);
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                 RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) Error_Handler();

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB | RCC_PERIPHCLK_I2C1 |
                                       RCC_PERIPHCLK_TIM1 | RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL;
  PeriphClkInit.Tim1ClockSelection = RCC_TIM1CLK_HCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) Error_Handler();
}


/**
  * @brief Initialize ADC1 (PC1 = ADC1_IN7)
  */
static void MX_ADC1_Init(void)
{
  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init = (ADC_InitTypeDef){
    .ClockPrescaler = ADC_CLOCK_ASYNC_DIV1,
    .Resolution = ADC_RESOLUTION_12B,
    .ScanConvMode = ADC_SCAN_DISABLE,
    .ContinuousConvMode = DISABLE,
    .DiscontinuousConvMode = DISABLE,
    .ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE,
    .ExternalTrigConv = ADC_SOFTWARE_START,
    .DataAlign = ADC_DATAALIGN_RIGHT,
    .NbrOfConversion = 1,
    .DMAContinuousRequests = DISABLE,
    .EOCSelection = ADC_EOC_SINGLE_CONV,
    .LowPowerAutoWait = DISABLE,
    .Overrun = ADC_OVR_DATA_OVERWRITTEN
  };
  if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();

  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK) Error_Handler();

  sConfig = (ADC_ChannelConfTypeDef){
    .Channel = ADC_CHANNEL_7,
    .Rank = ADC_REGULAR_RANK_1,
    .SingleDiff = ADC_SINGLE_ENDED,
    .SamplingTime = ADC_SAMPLETIME_1CYCLE_5,
    .OffsetNumber = ADC_OFFSET_NONE,
    .Offset = 0
  };
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) Error_Handler();
}

/**
  * @brief Initialize ADC2 (PA1 = ADC2_IN1)
  */
static void MX_ADC2_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc2.Instance = ADC2;
  hadc2.Init = hadc1.Init;
  if (HAL_ADC_Init(&hadc2) != HAL_OK) Error_Handler();

  sConfig = (ADC_ChannelConfTypeDef){
    .Channel = ADC_CHANNEL_1,
    .Rank = ADC_REGULAR_RANK_1,
    .SingleDiff = ADC_SINGLE_ENDED,
    .SamplingTime = ADC_SAMPLETIME_1CYCLE_5,
    .OffsetNumber = ADC_OFFSET_NONE,
    .Offset = 0
  };
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) Error_Handler();
}


/**
  * @brief Initialize I2C1
  */
static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init = (I2C_InitTypeDef){
    .Timing = 0x00201D2B,
    .OwnAddress1 = 0,
    .AddressingMode = I2C_ADDRESSINGMODE_7BIT,
    .DualAddressMode = I2C_DUALADDRESS_DISABLE,
    .OwnAddress2 = 0,
    .OwnAddress2Masks = I2C_OA2_NOMASK,
    .GeneralCallMode = I2C_GENERALCALL_DISABLE,
    .NoStretchMode = I2C_NOSTRETCH_DISABLE
  };
  if (HAL_I2C_Init(&hi2c1) != HAL_OK) Error_Handler();
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK) Error_Handler();
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) Error_Handler();
}

/**
  * @brief Initialize TIM1 (Input Capture)
  */
static void MX_TIM1_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  htim1.Instance = TIM1;
  htim1.Init = (TIM_Base_InitTypeDef){
    .Prescaler = 47,
    .CounterMode = TIM_COUNTERMODE_UP,
    .Period = 65535,
    .ClockDivision = TIM_CLOCKDIVISION_DIV1,
    .RepetitionCounter = 0,
    .AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE
  };
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK) Error_Handler();

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) Error_Handler();

  if (HAL_TIM_IC_Init(&htim1) != HAL_OK) Error_Handler();

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK) Error_Handler();

  sConfigIC = (TIM_IC_InitTypeDef){
    .ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE,
    .ICSelection = TIM_ICSELECTION_DIRECTTI,
    .ICPrescaler = TIM_ICPSC_DIV1,
    .ICFilter = 0
  };
  if (HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
}

/**
  * @brief Initialize TIM2 (PWM)
  */
static void MX_TIM2_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init = (TIM_Base_InitTypeDef){
    .Prescaler = 47,
    .CounterMode = TIM_COUNTERMODE_UP,
    .Period = 20000,
    .ClockDivision = TIM_CLOCKDIVISION_DIV1,
    .AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE
  };
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) Error_Handler();

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) Error_Handler();

  sConfigOC = (TIM_OC_InitTypeDef){
    .OCMode = TIM_OCMODE_PWM1,
    .Pulse = 2000,
    .OCPolarity = TIM_OCPOLARITY_HIGH,
    .OCFastMode = TIM_OCFAST_DISABLE
  };
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) Error_Handler();

  HAL_TIM_MspPostInit(&htim2);
}

/**
  * @brief Initialize USB
  */
static void MX_USB_PCD_Init(void)
{
  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init = (PCD_InitTypeDef){
    .dev_endpoints = 8,
    .speed = PCD_SPEED_FULL,
    .phy_itface = PCD_PHY_EMBEDDED,
    .low_power_enable = DISABLE,
    .battery_charging_enable = DISABLE
  };
  if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK) Error_Handler();
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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();  // Needed for PC0, PC1
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* Configure GPIO pin Output Level for GPIOE */
  HAL_GPIO_WritePin(GPIOE, CS_I2C_SPI_Pin|LD4_Pin|LD3_Pin|LD5_Pin
                          |LD7_Pin|LD9_Pin|LD10_Pin|LD8_Pin
                          |LD6_Pin, GPIO_PIN_RESET);

  /* Configure GPIO pin Output Level for PA9, PA10 */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9|GPIO_PIN_10, GPIO_PIN_RESET);

  /* Configure GPIO pin Output Level for PC0, PC1 (Red/Green LEDs) */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_1, GPIO_PIN_RESET);

  /* GPIOE: LEDs and CS pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin|LD4_Pin|LD3_Pin|LD5_Pin
                        |LD7_Pin|LD9_Pin|LD10_Pin|LD8_Pin
                        |LD6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* GPIOE: MEMS INT pins */
  GPIO_InitStruct.Pin = MEMS_INT3_Pin|MEMS_INT4_Pin|MEMS_INT1_Pin|MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* GPIOC: Red and Green LEDs */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* GPIOA: Additional output pins */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* SPI1 MISO: Alternate function */
  GPIO_InitStruct.Pin = SPI1_MISO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(SPI1_MISO_GPIO_Port, &GPIO_InitStruct);

  /* GPIOC: User Button */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);
}

/**
  * @brief  This function is executed in case of an error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* Disable all interrupts to prevent further execution */
  __disable_irq();

  /* Infinite loop to halt the system for debugging */
  while (1)
  {
    // Optional: Add LED blinking here to indicate an error visually
  }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* Optional: Print the file name and line number using a debug UART or semihosting */
  // Example: printf("Assertion failed: file %s, line %d\r\n", file, line);

  /* You may also implement logging or a breakpoint here */
}
#endif /* USE_FULL_ASSERT */


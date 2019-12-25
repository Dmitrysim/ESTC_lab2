#include <stm32f4xx.h>

void configure_button(void);
void configure_lde(void);
void MCO_out(void);
void Clock_initial(void);
void configure_timers(void);
void TIM2_IRQHandler(void);
void EXTI_init(void);
void EXTI0_IRQHandler(void);

const uint16_t LEDS = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
const uint16_t LED[] = {GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10};

/*const uint16_t LEDS = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
const uint16_t LED[] = { GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10};*/
uint16_t flag = 0;
static int8_t leds_counter;
static int8_t leds_direction;

void configure_lde(void) {

    GPIO_InitTypeDef leds_init_structure;
   /* Enable clocking for LEDS */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
   leds_init_structure.GPIO_Pin = LEDS;
   leds_init_structure.GPIO_Mode = GPIO_Mode_OUT;
   leds_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
   leds_init_structure.GPIO_OType = GPIO_OType_PP;
   GPIO_Init(GPIOA, &leds_init_structure);
   GPIO_SetBits(GPIOA, LEDS);

}

void configure_button(void) {

   GPIO_InitTypeDef buttons_init_structure;
   /* Enable clocking for Buttons */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
   buttons_init_structure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1;
   buttons_init_structure.GPIO_Mode  = GPIO_Mode_IN;
   buttons_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
   buttons_init_structure.GPIO_OType = GPIO_OType_PP;
   buttons_init_structure.GPIO_PuPd  = GPIO_PuPd_UP;
   GPIO_Init(GPIOA, &buttons_init_structure);

}

void configure_timers(void)
{
   /* Timer  */
   TIM_TimeBaseInitTypeDef timer_init_structure;
   /* Initialize peripheral clock */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
   /* Initialize timer */
   timer_init_structure.TIM_Prescaler     = 168;  /* Scale value to microseconds */
   timer_init_structure.TIM_CounterMode   = TIM_CounterMode_Up;
   timer_init_structure.TIM_Period        = 1000000;   /* Gives us a second interval */
   timer_init_structure.TIM_ClockDivision = TIM_CKD_DIV1; /* Tell timer to divide clocks */
   timer_init_structure.TIM_RepetitionCounter = 0;
   TIM_TimeBaseInit(TIM2, &timer_init_structure);

   /* NVIC TIM2 */
   TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

   /* Start timer */
   TIM_Cmd(TIM2, ENABLE);

   /* Configure interrupts for timer 2 */
    NVIC_InitTypeDef nvic_struct;
    nvic_struct.NVIC_IRQChannel = TIM2_IRQn;
    nvic_struct.NVIC_IRQChannelPreemptionPriority = 0;
    nvic_struct.NVIC_IRQChannelSubPriority = 1;
    nvic_struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_struct);

}

int main(void) {

    Clock_initial();
    //MCO_out();
    configure_timers();
    configure_button();
    configure_lde();
    EXTI_init();
    //_nvic_enable();

    leds_direction = 1;

    while(1) {

        //GPIO_Write(GPIOD, LED[flag]);

    }

}


void Clock_initial(void) {

    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET); //Waiting for HSE

    //RCC_HCLKConfig(RCC_SYSCLK_Div1);                    // AHB Prescaler
    //RCC_PCLK1Config(RCC_SYSCLK_Div4);                   // APB1 Prescaler
    //RCC_PCLK2Config(RCC_SYSCLK_Div2);                   // APB2 Prescaler

    RCC_PLLConfig(RCC_PLLSource_HSE, 8, 336, 2, 7);      // 168 MHz

    RCC_PLLCmd(ENABLE);

    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    RCC_HSICmd(DISABLE);                                //Disable HSI

}

void MCO_out(void) {

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_MCO);

    RCC_MCO1Config(RCC_MCO1Source_PLLCLK, RCC_MCO1Div_1);

}

void TIM2_IRQHandler(void) {

  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
   {
      TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
      //GPIOA->ODR ^= GPIO_ODR_ODR_8;
      GPIO_SetBits(GPIOA, GPIO_Pin_8 << leds_counter);
      leds_counter = (3 + (leds_counter - leds_direction) % 3) % 3;
      GPIO_ResetBits(GPIOA, GPIO_Pin_8 << leds_counter);
   }

}

void EXTI_init(void) {

    GPIO_InitTypeDef GPIO_initio;
    EXTI_InitTypeDef EXTI_initio;
    NVIC_InitTypeDef NVIC_initio;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  //Включаем тактирование контроллера конфигурации системы

    GPIO_initio.GPIO_Pin = GPIO_Pin_0;
    GPIO_initio.GPIO_Mode = GPIO_Mode_IN;
    GPIO_initio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_initio.GPIO_OType = GPIO_OType_PP;
    GPIO_initio.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &GPIO_initio);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

    EXTI_initio.EXTI_Line = EXTI_Line0;                            //Выбираем нулевую линию
    EXTI_initio.EXTI_Mode = EXTI_Mode_Interrupt;                   //Устанавливаем, что событие будет источником прерывания
                                                                   //Запрос на прерывание генерируется по фронту импульса, т.е. при
    EXTI_initio.EXTI_Trigger = EXTI_Trigger_Rising;                //переходе из 0 в 1
    EXTI_initio.EXTI_LineCmd = ENABLE;                             //Указываем новое состояние для линии прерывания
    EXTI_Init(&EXTI_initio);                                       //Инициализируем настройки

    NVIC_initio.NVIC_IRQChannel = EXTI0_IRQn;                      //Устанавливаем источник запроса на прерывание
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);                //присваиваем данному прерыванию первую группу
    NVIC_initio.NVIC_IRQChannelPreemptionPriority = 0;             //Устанавливаем приоритет для обработчика прерывания
    NVIC_initio.NVIC_IRQChannelSubPriority = 1;                    //Устанавливаем субприоритет
    NVIC_initio.NVIC_IRQChannelCmd = ENABLE;                       //Указываем, что выбранный канал будет источником вектора прерывания
    NVIC_Init(&NVIC_initio);                                       //Инициализируем настройки

}

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET){

        leds_direction = -leds_direction;
        EXTI_ClearITPendingBit(EXTI_Line0);  // сбрасываем флаг прерывания

    }
}



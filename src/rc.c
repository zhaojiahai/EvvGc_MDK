#include "rc.h"
#include "stm32f10x.h"
#include "utils.h"
#include "comio.h"
#include "definitions.h"
#include "pwm.h"

void Timer3_Config(void) // RC control timer config
{
    TIM_TimeBaseInitTypeDef     TIM_TimeBaseInitStructure;

    TIM_TimeBaseInitStructure.TIM_Prescaler = 71; // 1MHz
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 0xffff;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    TIM_Cmd(TIM3, ENABLE);
}

//@HackOS: 接收机输入配置
void RC_Config(void)
{
    GPIO_InitTypeDef    GPIO_InitStructure;

    __disable_irq();

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);

    // PA15 must be initialized after PA15/PB3 are made available with GPIO_Remap_SWJ_JTAGDisable
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15;				// PA15
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;  	// Set to Input
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   		// GPIO Speed
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //EXTI IN GPIO Config
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3 | GPIO_Pin_4;	// PB3-Pitch, PB4-Roll
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;  			// Set to Input Pull Down
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    	  	// GPIO Speed
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 					// PC2-Yaw
    GPIO_Init(GPIOC, &GPIO_InitStructure);

	//@HackOS: 设置外部中断源
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource3);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource4);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource2);

    EXTI_InitTypeDef EXTI_InitStructure;

	//@HackOS: 设置外部中断线
    EXTI_InitStructure.EXTI_Line    = EXTI_Line3 | EXTI_Line4 | EXTI_Line2;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;		//@HackOS: 上升下降均中断
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    EXTI_GenerateSWInterrupt(EXTI_Line3 | EXTI_Line4 | EXTI_Line2);
    EXTI_ClearITPendingBit(EXTI_Line3 | EXTI_Line4 | EXTI_Line2);

    NVIC_EnableIRQ(EXTI3_IRQn); // Enable interrupt
    NVIC_EnableIRQ(EXTI4_IRQn); // Enable interrupt
    NVIC_EnableIRQ(EXTI2_IRQn); // Enable interrupt

    Timer3_Config(); //RC control timer

    __enable_irq();
}

/*-----------------Read RC on AUX 3--------------------------*/
// Pitch
static int rc3 = 0;

int GetAUX3(void)
{
    return rc3;
}

//@HackOS: 接收机外部中断
void EXTI3_IRQHandler(void) //EXTernal interrupt routine PB3-Pitch
{
    static unsigned short rc3a = 0, rc3b = 0;

    //if (EXTI_GetITStatus(EXTI_Line3) != RESET)
    if (EXTI->PR & (1 << 3))
    {
        DEBUG_PutChar('3');

        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) == 1)
        {
            rc3a = TIM3->CNT;
        }

        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) == 0)
        {
            rc3b = TIM3->CNT;
        }

        unsigned short diff = rc3b - rc3a;

        if ((diff > 1000) && (diff < 2000))
        {
            rc3 = (int)diff - 1000;
        }
        else
        {
            //rc3 = 0;
        }

        // EXTI3 interrupt pending?
        // EXTI_ClearITPendingBit(EXTI_Line3); // clear pending interrupt
        EXTI->PR |= (1 << 3); // clear pending interrupt

    }
}

/*-----------------Read RC on AUX 2--------------------------*/
// Roll
static int rc2 = 0;

int GetAUX2(void)
{
    return rc2;
}

void EXTI2_IRQHandler(void) //EXTernal interrupt routine PC2-Pitch
{
    static unsigned short rc2a = 0, rc2b = 0;

    //if (EXTI_GetITStatus(EXTI_Line2) != RESET)
    if (EXTI->PR & (1 << 2))
    {
        DEBUG_PutChar('2');

        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2) == 1)
        {
            rc2a = TIM3->CNT;
        }

        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2) == 0)
        {
            rc2b = TIM3->CNT;
        }

        unsigned short diff = rc2b - rc2a;

        if ((diff > 1000) && (diff < 2000))
        {
            rc2 = (int)diff - 1000;
        }
        else
        {
            //rc2 = 0;
        }

        // EXTI2 interrupt pending?
        //EXTI_ClearITPendingBit(EXTI_Line2); // clear pending interrupt
        EXTI->PR |= (1 << 2); // clear pending interrupt

    }
}

/*-----------------Read RC on AUX 4--------------------------*/
// Yaw
static int rc4 = 0;

int GetAUX4(void)
{
    return rc4;
}

void EXTI4_IRQHandler(void) //EXTernal interrupt routine PB4-Yaw
{
    static unsigned short rc4a = 0, rc4b = 0;

    //if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    if (EXTI->PR & (1 << 4))
    {
        DEBUG_PutChar('4');

        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 1)
        {
            rc4a = TIM3->CNT;
        }

        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0)
        {
            rc4b = TIM3->CNT;
        }

        unsigned short diff = rc4b - rc4a;

        if ((diff > 1000) && (diff < 2000))
        {
            rc4 = (int)diff - 1000;
        }
        else
        {
            //rc4 = 0;
        }

        // EXTI3 interrupt pending?
        //EXTI_ClearITPendingBit(EXTI_Line4); // clear pending interrupt
        EXTI->PR |= (1 << 4);                         // clear pending interrupt
    }
}

/*
  Get value from RC input (0-1000) and converts it to step
  Value of step is is stored as a 1x3 array
*/
//@HackOS: 将遥控值(0-1000)转为步进值
void Get_RC_Step(float *Step, float *RCSmooth)
{

    int aux3 = GetAUX3(); //PITCH
    int aux2 = GetAUX2(); //ROLL
    int aux4 = GetAUX4(); //YAW

    // Pitch
    if (aux3 != 0) //check there is a rc input
    {
    	//@HackOS: 换算至-500--500，并滤波
        RCSmooth[PITCH] = ((RCSmooth[PITCH] * 199) + (aux3 - RC_CENTER_VAL)) / 200;

		//@HackOS: 大于死区
        if (RCSmooth[PITCH] > DEAD_ZONE || RCSmooth[PITCH] < -DEAD_ZONE)
        {
            Step[PITCH] = RCSmooth[PITCH] * RC2STEP;
        }
        else
        {
            Step[PITCH] = 0;
        }
    }
    else
    {
        Step[PITCH] = 0;
    }

    // Roll
    if (aux2 != 0) //check there is a rc input
    {
        RCSmooth[ROLL] = ((RCSmooth[ROLL] * 199) + (aux2 - RC_CENTER_VAL)) / 200;

        if (RCSmooth[ROLL] > DEAD_ZONE || RCSmooth[ROLL] < -DEAD_ZONE)
        {
            Step[ROLL] = RCSmooth[ROLL] * RC2STEP;
        }
        else
        {
            Step[ROLL] = 0;
        }
    }
    else
    {
        Step[ROLL] = 0;
    }

    // YAW
    if (aux4 != 0) //check there is a rc input
    {
        RCSmooth[YAW] = ((RCSmooth[YAW] * 199) + (aux4 - RC_CENTER_VAL)) / 200;

        if (RCSmooth[YAW] > DEAD_ZONE || RCSmooth[YAW] < -DEAD_ZONE)
        {
            Step[YAW] = RCSmooth[YAW] * RC2STEP;
        }
        else
        {
            Step[YAW] = 0;
        }
    }
    else
    {
        Step[YAW] = 0;
    }
}


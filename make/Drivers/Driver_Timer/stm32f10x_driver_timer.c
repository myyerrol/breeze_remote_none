#include <stdio.h>
#include "stm32f10x_driver_timer.h"
#include "stm32f10x_module_nrf24l01.h"

extern u8  TX_ADDRESS[TX_ADR_WIDTH];

u16 flag10Hzcnt,flag50Hzcnt,flag80Hzcnt,flag100Hzcnt;
u16 flag10Hz,flag50Hz,flag80Hz,flag100Hz;


void TIM4_IRQHandler(void) //1ms中断一次,用于遥控器主循环
{
    if( TIM_GetITStatus(TIM4 , TIM_IT_Update) != RESET )
    {
        if(++flag10Hzcnt == 100)//10Hz
        {
            flag10Hzcnt = 0;
            flag10Hz = 1;
        }
        if(++flag50Hzcnt == 20)//50Hz
        {
            flag50Hzcnt = 0;
            flag50Hz = 1;
        }
        if(++flag80Hzcnt == 12) //80Hz
        {
            flag80Hzcnt = 0;
            flag80Hz = 1;
        }
        if(++flag100Hzcnt == 10) //100Hz
        {
            flag100Hzcnt = 0;
            flag100Hz = 1;
        }
        TIM_ClearITPendingBit(TIM4 , TIM_FLAG_Update);   //清除中断标志
    }
}

extern int Throttle;
extern int Pitch;
extern int Roll;
extern int Yaw;

void TIM3_IRQHandler(void) //打印中断服务程序
{
    if( TIM_GetITStatus(TIM3 , TIM_IT_Update) != RESET )
    {

        printf("r: %d\r\n", Roll);
        printf("p: %d\r\n", Pitch);
        printf("y: %d\r\n", Yaw);
        printf("t: %d\r\n", Throttle);
        // printf("remote addr -->0x%x\r\n",TX_ADDRESS[4]);// tx addr
        printf("##########\n");

        TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update);   //清除中断标志
    }
}

//定时器4初始化：用来中断处理PID
void TIM4_Init(char clock,int Preiod)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  //打开时钟

    TIM_DeInit(TIM4);

    TIM_TimeBaseStructure.TIM_Period = Preiod;
    TIM_TimeBaseStructure.TIM_Prescaler = clock-1;//定时1ms
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);
    TIM_ClearFlag(TIM4,TIM_FLAG_Update);

    TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM4,ENABLE);
    // printf("Timer 4 init success...\r\n");
}

//定时器3初始化
void TIM3_Init(char clock,int Preiod)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  //打开时钟

    TIM_DeInit(TIM3);

    TIM_TimeBaseStructure.TIM_Period = Preiod;
    //预分频系数为36000-1，这样计数器时钟为72MHz/36000 = 2kHz
    TIM_TimeBaseStructure.TIM_Prescaler = 36000 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
    TIM_ClearFlag(TIM3,TIM_FLAG_Update);

    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM3,ENABLE);
    // printf("Timer 3 init success...\r\n");
}

void TimerNVIC_Configuration()
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* NVIC_PriorityGroup 2 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    //TIM3
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//定时器3主优先级为2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    //TIM4
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//定时器4主优先级为1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

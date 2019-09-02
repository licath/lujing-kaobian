#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "can.h"
#include "point2point.h"
#include "pid.h"
#include "lcd.h"
#include "led.h"
#include "timer.h"
#include "math.h"
#include "move_base.h"
#include "main.h"
#include "ps2.h"


#define PI (3.141593f)
static float laser_distance;
fp32 Px,Py;
float Pp;
fp32 v;
long i;
int main(void)
{
		u8 state[5]={0};
		u8 switch_state;
		u8 clear_flag=0;
		u8 flag=0;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    uart_init(115200);//���ڳ�ʼ��Ϊ115200
    delay_init();//��ʱ������ʼ��
    MyusartInit2(19200);//����2
    MyusartInit5(115200);
    LCD_Init();
    CAN1_Mode_Init(CAN_SJW_1tq, CAN_BS2_3tq, CAN_BS1_8tq, 3, CAN_Mode_Normal);  //CAN��ʼ��ģʽ,������1Mbps
    chassis_init(&chassis_move);//���̳�ʼ��
    delay_ms(50);
    TIM2_Init(3000,36000-1);//1.5�붨ʱ
    TIM3_Init(3000,36000-1);//1.5�붨ʱ
    TIM4_Init(2000,72-1);
    delay_ms(3000);
//		cycleCounterInit();
	  clear();
    while(1)
   {
        Px=GetPosX();
        Py=GetPosY();
        Pp=GetAngle();
        if(Px<0)
        {
            LCD_ShowString(60,50,200,16,16,"x=");
            LCD_ShowString(55,70,200,16,16,"-");
            LCD_ShowxNum(60,70,-Px,6,16,0X80);
        }
        else
        {
            LCD_ShowString(60,50,200,16,16,"x=");
            LCD_ShowString(55,70,200,16,16," ");

            LCD_ShowxNum(60,70,Px,6,16,0X80);
        }
        if(Py<0)
        {
            LCD_ShowString(60,90,200,16,16,"y=");
            LCD_ShowString(55,110,200,16,16,"-");
            LCD_ShowxNum(60,110,-Py,6,16,0X80);
        }
        else
        {
            LCD_ShowString(60,90,200,16,16,"y=");
            LCD_ShowString(55,110,200,16,16," ");
            LCD_ShowxNum(60,110,Py,6,16,0X80);
        }
        if(Pp<0)
        {
            LCD_ShowString(60,130,200,16,16,"Angle=");
            LCD_ShowString(55,150,200,16,16,"-");
            LCD_ShowxNum(60,150,-Pp,6,16,0X80);
        }
        else
        {
            LCD_ShowString(60,130,200,16,16,"Angle=");
            LCD_ShowString(55,150,200,16,16," ");
            LCD_ShowxNum(60,150,Pp,6,16,0X80);
        }
			 close_LargeToSmall(1,flag);
//			 i=millis();
       printf("%ld\n",i);
    }
	}

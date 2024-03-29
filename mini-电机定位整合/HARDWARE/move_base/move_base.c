#include "move_base.h"

ActPoint initpoint;     //点到点算法 初始点结构体初始化
float initangle = 0;
ActPoint targetpoint;		//目标点结构体初始化
float targetangle = 0;
ActLine2 presentline;		//初始直线结构体初始化
ActLine2 targetline;		//目标直线结构体初始化


PidTypeDef PID_distance;	//PID结构体初始化
PidTypeDef PID_angle;
PidTypeDef PID_DisArc;		//角度对距离增量PID

fp32 PID_param_dis[3];		//距离PID变量参数结构体初始化
fp32 PID_param_angle[3];//角度PID变量参数结构体初始化
fp32 PID_param_DisArc[3];//距离转角度PID变量参数结构体初始化

int32_t motor_v1=0;
int32_t motor_v2=0;


//Vel差值为正则为右转
//speedBase为基础前进速度
void motorCMD(int32_t motor1,int32_t motor2)//电机控制 非点到点电机控制函数
{
    if(motor1>9000.0f)
        motor1=9000;
    if(motor2>9000.0f)
        motor2=9000;
    if(motor1<-9000.0f)
        motor1=-9000;
    if(motor2<-9000.0f)
        motor2=-9000;
    give_motor1(motor1);
    give_motor2(-motor2);

		if(motor1>0)
		{
    LCD_ShowString(60,170,200,16,16,"V left=");
    LCD_ShowxNum(60,190,motor1,6,16,0X80);
		}
		else
		{
		
		LCD_ShowString(60,170,200,16,16,"V left=");
		LCD_ShowString(60,190,200,16,16,"-");
		LCD_ShowxNum(60,190,-motor1,6,16,0X80);	
		}
		if(motor2>0)
		{
    LCD_ShowString(60,210,200,16,16,"V right=");
    LCD_ShowxNum(60,230,motor2,6,16,0X80);
		}
		else{
		LCD_ShowString(60,210,200,16,16,"V right=");
		LCD_ShowString(60,210,200,16,16,"-");
    LCD_ShowxNum(60,230,-motor2,6,16,0X80);
		}
}


void motor_back_CMD(int32_t motor1,int32_t motor2)//电机控制 非点到点电机控制函数
{
    if(motor1>9000.0f)
        motor1=9000;
    if(motor2>9000.0f)
        motor2=9000;
    if(motor1<-9000.0f)
        motor1=-9000;
    if(motor2<-9000.0f)
        motor2=-9000;
    give_motor1(motor2);
    give_motor2(-motor1);

    LCD_ShowString(60,170,200,16,16,"V left=");
    LCD_ShowxNum(60,190,motor2,6,16,0X80);
    LCD_ShowString(60,210,200,16,16,"V right=");
    LCD_ShowxNum(60,230,motor1,6,16,0X80);
}


void ps2_move()
{
    u8 key=0;
    s16 speed;
    s16 swerve;
    delay_ms(100);
    key=PS2_DataKey();
    if(key!=0)                   //有按键按下
    {
//				printf(" %5d %5d %5d %5d\r\n",PS2_AnologData(PSS_LX),PS2_AnologData(PSS_LY),
//		                              PS2_AnologData(PSS_RX),PS2_AnologData(PSS_RY) );
//				printf(" %5d %5d\r\n",PS2_AnologData(PSS_LY),PS2_AnologData(PSS_RX));

        if(key == 11)
        {
            speed = PS2_AnologData(PSS_LY)-127;
            swerve = (PS2_AnologData(PSS_RX)-128)*12*((float)abs(speed)/128); //	speed取绝对值，	算数运算，得到转弯量。
            speed = -(PS2_AnologData(PSS_LY)-127)*20;	   //正：前进；  负：后退
//				delay_ms(10);
//				printf(" %5d \r\n",speed);
//				printf(" %5d \r\n",swerve);
            if(speed==0&&swerve==0)
            {
                give_motor1(0);
                give_motor2(0);
                PID_Move_Clear(&chassis_move);
            }
            else
            {
                if(speed > 0) //向前
                {
                    if(swerve < 0)//左转弯
                    {
                        //						speed1 = speed + swerve;
                        //						speed2 = speed;
                        give_motor1(speed + swerve);
                        give_motor2(~speed+1);
                        //						printf(" %5d %5d\r\n",wheel_speed[0],wheel_speed[1]);
                    }
                    else          //右转弯
                    {
                        //						speed1 = speed;
                        //						speed2 = speed - swerve;
                        give_motor1(speed);
                        give_motor2(~(speed - swerve)+1);
                        //						printf(" %5d %5d\r\n",wheel_speed[0],wheel_speed[1]);
                    }
                }
                else if(speed < 0)//向后
                {
                    if(swerve < 0)//左转弯
                    {
                        //						speed1 = speed - swerve;
                        //						speed2 = speed;
                        give_motor1(speed - swerve);
                        give_motor2(~(speed - swerve)+1);
                    }

                    else//右转弯
                    {
                        //						speed1 = speed;
                        //						speed2 = speed + swerve;
                        give_motor1(speed);
                        give_motor2(~(speed + swerve)+1);
                    }
                }
            }
        }
        else
        {
//				delay_ms(10);
            give_motor1(0);
            give_motor2(0);
            PID_Move_Clear(&chassis_move);
//				printf("  \r\n   %d  is  light  \r\n",Data[1]);//ID
//				printf("  \r\n   %d  is  pressed  \r\n",key);
//				printf("  \r\n   %d  is  pressed  \r\n",MASK[key]);
            if(key == 12)
            {
                PS2_Vibration(0x00,0xFF);  //发出震动后必须有延时  delay_ms(1000);
                delay_ms(500);
            }
            else
                PS2_Vibration(0x00,0x00);
        }
    }
    else
    {
        give_motor1(0);
        give_motor2(0);
        PID_Move_Clear(&chassis_move);
    }
}
void walk_point(int x,int y, int p)
{
    presentline.point.x=GetPosX();
    presentline.point.y=GetPosY();
    presentline.angle=GetAngle();
		targetline.point.x=x;
    targetline.point.y=y;
    targetline.angle=p;//设置目标点	x,y,p
	
	
	
	if(targetline.point.x<0)
        {
            LCD_ShowString(120,50,200,16,16,"x=");
            LCD_ShowString(115,70,200,16,16,"-");
            LCD_ShowxNum(120,70,-targetline.point.x,3,16,0X80);
        }
        else
        {
            LCD_ShowString(120,50,200,16,16,"x=");
            LCD_ShowString(115,70,200,16,16," ");

            LCD_ShowxNum(120,70,targetline.point.x,3,16,0X80);
        }
        if(y<0)
        {
            LCD_ShowString(120,90,200,16,16,"y=");
            LCD_ShowString(115,110,200,16,16,"-");
            LCD_ShowxNum(120,110,-targetline.point.y,3,16,0X80);
        }
        else
        {
            LCD_ShowString(120,90,200,16,16,"y=");
            LCD_ShowString(115,110,200,16,16," ");

            LCD_ShowxNum(120,110,targetline.point.y,3,16,0X80);
        }
        if(p<0)
        {
            LCD_ShowString(120,130,200,16,16,"Angle=");
            LCD_ShowString(115,150,200,16,16,"-");
            LCD_ShowxNum(120,150,-targetline.angle,3,16,0X80);
        }
        else
        {
            LCD_ShowString(120,130,200,16,16,"Angle=");
            LCD_ShowString(115,150,200,16,16," ");
            LCD_ShowxNum(120,150,targetline.angle,3,16,0X80);
        }
				
				
				
    MvByLine(presentline,targetline, 1000);
}


void set_point(u16 x,u16 y,u16 p)
{
    targetline.point.x=x;
    targetline.point.y=y;
    targetangle=p;
}


/**
* @brief 距离PID
* @note	PID类型：位置型PID
* @param valueSet：距离设定值
* @param valueNow：当前距离值
* @retval 电机前进基础速度
* @attention
*/
float DistancePid(float distance)
{
    fp32 valueOut;
    PID_param_dis[0]=55;
    PID_param_dis[1]=0;
    PID_param_dis[2]=0;

    PID_Init(&PID_distance,PID_POSITION,PID_param_dis,9000.0f,1000.0f);//PID初始化
    valueOut=PID_Calc(&PID_distance,distance,0);


    return valueOut;
}



/**
* @brief 距离对角度的增量PID
* @note	PID类型：位置型PID
* @param valueSet：距离设定值
* @param valueNow：当前距离值
* @retval 电机前进基础速度
* @attention
*/
float Distance_Arc_Pid(float distance)
{
    float valueOut;
    PID_Init(&PID_DisArc,PID_POSITION,PID_param_DisArc,90.0f,10.0f);//位置型PID初始化
    valueOut=PID_Calc(&PID_DisArc,0,distance);
	if(valueOut<0)
	{
    LCD_ShowString(130,130,200,16,16,"ARCPID=");
		    LCD_ShowString(125,150,200,16,16,"-");
    LCD_ShowxNum(130,150,-valueOut,6,16,0X80);
	}
	else
	{
	 LCD_ShowString(130,130,200,16,16,"ARCPID=");
    LCD_ShowxNum(130,150,valueOut,6,16,0X80);
	
	}
    return valueOut;
}



/**
* @brief 角度PID
* @note	PID类型：位置型PID
* @param valueSet：角度设定值
* @param valueNow：当前角度值
* @retval 电机速度差值
* @attention
*/
float AnglePid(float valueSet,float valueNow)
{
    float err=0;
    float valueOut=0;
    err=valueSet-valueNow;
    //角度突变处理
    if(err > 180)
    {
        err=err-360;
    }
    else if(err < -180)
    {
        err=360+err;
    }
    PID_Init(&PID_angle,PID_POSITION,PID_param_angle,9000.0f,20.0f);//PID初始化
    valueOut=PID_Calc(&PID_angle,0,err);	//PID计算转弯差值
    if(valueOut>0)
    {
        LCD_ShowString(130,170,200,16,16,"AnglePID=");
        LCD_ShowString(120,190,200,16,16," ");
        LCD_ShowxNum(130,190,valueOut,6,16,0X80);
    }
    else
    {
        LCD_ShowString(130,170,200,16,16,"AnglePID=");
        LCD_ShowString(120,190,200,16,16,"-");
        LCD_ShowxNum(130,190,-valueOut,6,16,0X80);
    }
    return valueOut;
}


/**
 * @brief  两个PID移动到目标位置
 * @note	代替point2point算法
 * @param  x		目标位置X坐标
 * @param  y		目标位置Y坐标
 * @param  angle目标角度
 * @retval None
 */

//小动作
void move_to_pos(float x,float y,float angle)
{
    float x_Now=GetPosX();
    float y_Now=GetPosX();
    float angle_Now=GetAngle();
    float forwardspeed,turnspeed;

    float aim_distance=sqrt(pow(x_Now-x,2)+pow(y_Now-y,2));  //当前点到目标点的直线距离

    forwardspeed = DistancePid(aim_distance);		//距离PID调整前进速度到达目标点			应调整为距离——角度PID
    turnspeed = AnglePid(angle,angle_Now);			//角度PID调整两轮差速到达目标方向

    motorCMD(forwardspeed-turnspeed,forwardspeed+turnspeed);
}


/**
 * @brief  PID 最小转弯
 * @note		无前进速度 仅为绕自身旋转中心旋转
 * @param  angle：给定角度,为正左转，为负右转
 * @param  gospeed：基础速度
 * @retval None
 */
//小动作 原地旋转
void minimum_Turn(float angle)
{
    float getAngle=0.0f;
    float speed=0.0f;
    PID_param_angle[0]=55;// 转速10000除以转弯最大角度180 = 55
    PID_param_angle[1]=0;
    PID_param_angle[2]=0;
    getAngle=GetAngle();
    speed = AnglePid(angle,getAngle);	//根据角度PID算出转向的差速
    motorCMD(speed,speed);
}



/**
 * @brief  PID 前进转弯
 * @note
 * @param  angle：给定角度,为正左转，为负右转
 * @param  gospeed：基础速度
 * @retval None
 */
//小动作 向前旋转
void forward_Turn(float angle,float gospeed)
{
    float getAngle=0.0f;
    float speed=0.0f;

	
    PID_param_angle[0]=10;
    PID_param_angle[1]=0;
    PID_param_angle[2]=0;

    getAngle=GetAngle();
    speed = AnglePid(angle,getAngle);	//根据角度PID算出转向的差速
//		if(gospeed<0)
//		motor_back_CMD(gospeed-speed,gospeed+speed);
//		else
    motorCMD(gospeed+speed,gospeed-speed);
}



/**
 * @brief  PID 后退转弯
 * @note
 * @param  angle：给定角度,为正左转，为负右转
 * @param  gospeed：基础速度
 * @retval None
 */
//小动作 倒行转弯
void back_Turn(float angle,float gospeed)
{
    float getAngle=0.0f;
    float speed=0.0f;
    PID_param_angle[0]=5;
    PID_param_angle[1]=0;
    PID_param_angle[2]=0;

    getAngle=GetAngle();
    speed = AnglePid(angle,getAngle);	//根据角度PID算出转向的差速
    motor_back_CMD(gospeed-speed,gospeed+speed);
}


/**
  * @brief  新底盘直线闭环
  * @note	Ax1+By1+C1=0 直线方程一般式
  * @note	可由A1，B1，C1设置直线方程后，沿目标直线方向行走
  * @note	前进速度可由距离PID调整  问题是两个PID不太好调  待调试后解决
  * @note angleAdd为正值

  * @param A1
  * @param B1
  * @param C1
  * @param dir:为0 往上或右走，为1 往下或往左走
  * @param setSpeed：速度
  * @retval return_value 0为 未到达目标直线  1为 已到达目标直线距离范围内

  * @note 大动作 在目标直线轨道上行驶 可通过坐标判断在直线的哪个位置
	* @note 用到了距离-角度PID 来接近目标轨道 角度PID来保证沿着直线轨道行驶
  * @note 用到了两种小动作 离目标直线距离远时 用前进转弯来接近
  * @note 接近到达目标直线时原地旋转调整到目标方向 在用前进转弯来保持行驶直线
  */
uint8_t straightLine(float A1,float B1,float C1,uint8_t dir,float setSpeed)
{
    fp32 setAngle=0;
    fp32 angleAdd=0;
    int	return_value=0;
    fp32 getAngleNow=GetAngle();
    fp32 getX=GetPosX();
    fp32 getY=GetPosY();
    float distance=((A1*getX)+(B1*getY)+C1)/sqrt((A1*A1)+(B1*B1));//当前点到目标直线距离

//		PID_param_dis[0]=0.5f;
//		PID_param_dis[0]=0;
//		PID_param_dis[0]=0;

    PID_param_DisArc[0]=0.8;
    PID_param_DisArc[1]=0;
    PID_param_DisArc[2]=0;

    angleAdd=Distance_Arc_Pid(distance);
	if(angleAdd<0)
        {   LCD_ShowString(130,90,200,16,16,"angleAdd =");
            LCD_ShowString(130,110,200,16,16,"-");
            LCD_ShowxNum(130,110,-angleAdd,6,16,0X80);
        }
        else {
            LCD_ShowString(130,90,200,16,16,"angleAdd =");
            LCD_ShowString(130,110,200,16,16," ");
            LCD_ShowxNum(130,110,angleAdd,6,16,0X80);
        }
    //离直线35以内时表示到达直线
//    if((distance < 150) && (distance > -150))	//到达直线位置时用最小半径旋转调整为目标角度
//    {
//        angleAdd=0;
//        return_value=1;
//			forward_Turn(setAngle,setSpeed);
//    }
//    else		//未到达目标直线距离范围内时，用前进转弯来接近目标直线
//    {
        if((B1 > -0.005f) && (B1 < 0.005f))
        {
            if(!dir)
            {
                setAngle=0;							//目标角度为水平直线 方向由dir确定 0为右 1为左
                forward_Turn(setAngle-angleAdd,setSpeed);
            }
            else
            {
                if(A1 > 0)
                {
                    setAngle=-180;			//目标角度为水平直线 方向由dir确定 0为上 1为下
                    forward_Turn(setAngle-angleAdd,setSpeed);//角度PID转向目标角度
                }
                else
                {

                    setAngle=180;
                    forward_Turn(setAngle+angleAdd,setSpeed);
                }
            }
        }
        else
        {
            if(!dir)
            {
                setAngle=(atan(-A1/B1)*180/PI)-90;
                forward_Turn(setAngle-angleAdd,setSpeed);
            }
            else
            {
                setAngle=(atan(-A1/B1)*180/PI)+90;
                forward_Turn(setAngle+angleAdd,setSpeed);
            }

        }
        return_value=0;
//    } 
						LCD_ShowString(130,50,200,16,16,"get_detination ?");
            LCD_ShowxNum(130,70,return_value,6,16,0X80);
		        return return_value;
}

/**
  * @brief  底盘圆环路径
  * @note		直线斜率为负	
	* @note		思路为 直线距离增大角度差值 到达圆环轨迹后 直线-角度PID不再计算 仅计算角度PID
	* @note		PID期望角度始终为当前行进位置指向圆心的方向
	* @note		直线——角度PID，角度——角度参数调整完成后，可添加直线——距离PID 调整前进速度 使得底盘更快进入目标轨道
  * @param x：圆心x坐标
  * @param y：圆心y坐标
  * @param R：半径
  * @param clock:为1 顺时针，为2 逆时针
  * @param v：速度
  * @retval None


	* @note 大动作 输入目标圆环圆心的 x,y,以及圆环半径R。在目标圆环轨道上行驶 可通过坐标判断在直线的哪个位置
	* @note 用到了距离-角度PID 来接近目标轨道 角度PID来保证沿着圆环轨道行驶
  * @note 用到了两种小动作 离目标直线距离远时 用前进转弯来接近
  * @note 接近到达目标直线时原地旋转调整到目标方向 在用前进转弯来保持行驶直线


  * @note PID调参时  角度PID 


  */
void close_LargeToSmall(u8 SideNumber,u8 state)
{
	
	static u8 flag=1;    
/*靠在出发去区的边上*/
	if(SideNumber==1)            //靠在出发边上；
	{
		if(state==0)               //state用来判断车子是否靠边
		{
		 if(flag==1)               //flag等于1，则行走大圆
			{
				closeRound(0,2200,1700,1,3000,0);				//大圈
				LCD_ShowString(60,250,200,16,16,"Number1 step1");
					if(GetPosX()>900&&GetPosX()<1000&&GetPosY()>0&&GetPosY()<2000)
				{
					flag=2;
				} 
			}
			if(flag==2)             //flag等于2，则判断i
			{
				static u8 i=0;
				if(i==0)              //i为0，则走小圆
				{
				LCD_ShowString(60,250,200,16,16,"Number1 step2");
					closeRound(0,2200, 800,1,3000,1);//小圈
				}
				if(i==1)              //i为1，寻找x=0的直线
				{
					LCD_ShowString(60,250,200,16,16,"Number1 step2_1");
					straightLine(1,0,0,0,2000);
					if((GetPosX()>-50&&GetPosX()<50)&&(GetPosY()>1600&&GetPosY()<1700)||(GetPosY()>2000&&GetPosY()<2100))
					{
						flag=3;           
					}
				}
				  if(GetPosX()>1000&&GetPosX()<1100&&GetPosY()>0&&GetPosY()<2000)
				{
					i=1;
				}
			}
			if(flag==3)             //flag为3，开始后退靠边，
			{
				LCD_ShowString(60,250,200,16,16,"Number1 step3");
				back_Turn(0,-2000);
			}
		}
		 if(state==1)           //当state为1，靠边成功
		 {
			 //填写当车子靠边后的程序
		 }
	}
/*靠在出发区的右边上*/
		if(SideNumber==2)            //靠在初发边的右边的边上；
	{
		if(state==0)               //state用来判断车子是否靠边
		{
		 if(flag==1)               //flag等于1，则行走大圆
			{
				closeRound(0,2200,1700,1,2000,0);				//大圈
				LCD_ShowString(60,250,200,16,16,"Number1 step1");
				if(GetPosX()>400&&GetPosX()<2400&&GetPosY()>3300&&GetPosY()<3400)
				{
					flag=2;
				} 
			}
			if(flag==2)             //flag等于2，则判断i
			{
				static u8 i=0;
				if(i==0)              //i为0，则走小圆
				{
				LCD_ShowString(60,250,200,16,16,"Number1 step2");
					closeRound(0,2200, 800,1,3000,1);//小圈
				}
				if(i==1)              //i为1，寻找x=0的直线
				{
					LCD_ShowString(60,250,200,16,16,"Number1 step2_1");
					straightLine(1,0,150,0,2000);
					if((GetPosX()>-50&&GetPosX()<50)&&(GetPosY()>1600&&GetPosY()<1700)||(GetPosY()>2000&&GetPosY()<2100))
					{
						flag=3;           
					}
				}
				  if(GetPosX()>1000&&GetPosX()<1100&&GetPosY()>0&&GetPosY()<2000)
				{
					i=1;
				}
			}
			if(flag==3)             //flag为3，开始后退靠边，
			{
				LCD_ShowString(60,250,200,16,16,"Number1 step3");
				back_Turn(0,-2000);
			}
		}
		 if(state==1)           //当state为1，靠边成功
		 {
			 //填写当车子靠边后的程序
		 }
	 }
/*靠在出发区的对面边上*/
			if(SideNumber==3)            //靠在出发去的对面的边上；
	{
		if(state==0)               //state用来判断车子是否靠边
		{
		 if(flag==1)               //flag等于1，则行走大圆
			{
				closeRound(0,2200,1700,1,2000,0);				//大圈
				LCD_ShowString(60,250,200,16,16,"Number1 step1");
				if(GetPosX()>-900&&GetPosX()<-1000&&GetPosY()>2800&&GetPosY()<4800)
				{
					flag=2;
				} 
			}
			if(flag==2)             //flag等于2，则判断i
			{
				static u8 i=0;
				if(i==0)              //i为0，则走小圆
				{
				LCD_ShowString(60,250,200,16,16,"Number1 step2");
					closeRound(0,2200, 800,1,3000,1);//小圈
				}
				if(i==1)              //i为1，寻找x=0的直线
				{
					LCD_ShowString(60,250,200,16,16,"Number1 step2_1");
					straightLine(1,0,150,0,2000);
					if((GetPosX()>-50&&GetPosX()<50)&&(GetPosY()>1600&&GetPosY()<1700)||(GetPosY()>2000&&GetPosY()<2100))
					{
						flag=3;           
					}
				}
				  if(GetPosX()>1000&&GetPosX()<1100&&GetPosY()>0&&GetPosY()<2000)
				{
					i=1;
				}
			}
			if(flag==3)             //flag为3，开始后退靠边，
			{
				LCD_ShowString(60,250,200,16,16,"Number1 step3");
				back_Turn(0,-2000);
			}
		}
		 if(state==1)           //当state为1，靠边成功
		 {
			 //填写当车子靠边后的程序
		 }
	 }
	/*靠在出发区的左边边上*/
		if(SideNumber==4)            //靠在出发去的左边的边上；
	{
		if(state==0)               //state用来判断车子是否靠边
		{
		 if(flag==1)               //flag等于1，则行走大圆
			{
				closeRound(0,2200,1700,1,2000,0);				//大圈
				LCD_ShowString(60,250,200,16,16,"Number1 step1");
				if(GetPosX()>-2400&&GetPosX()<-400&&GetPosY()>1400&&GetPosY()<1500)
				{
					flag=2;
				} 
			}
			if(flag==2)             //flag等于2，则判断i
			{
				static u8 i=0;
				if(i==0)              //i为0，则走小圆
				{
				LCD_ShowString(60,250,200,16,16,"Number1 step2");
					closeRound(0,2200, 800,1,3000,1);//小圈
				}
				if(i==1)              //i为1，寻找x=0的直线
				{
					LCD_ShowString(60,250,200,16,16,"Number1 step2_1");
					straightLine(1,0,150,0,2000);
					if((GetPosX()>-50&&GetPosX()<50)&&(GetPosY()>1600&&GetPosY()<1700)||(GetPosY()>2000&&GetPosY()<2100))
					{
						flag=3;           
					}
				}
				  if(GetPosX()>1000&&GetPosX()<1100&&GetPosY()>0&&GetPosY()<2000)
				{
					i=1;
				}
			}
			if(flag==3)             //flag为3，开始后退靠边，
			{
				LCD_ShowString(60,250,200,16,16,"Number1 step3");
				back_Turn(0,-2000);
			}
		}
		 if(state==1)           //当state为1，靠边成功
		 {
			 //填写当车子靠边后的程序
		 }
	 }
}

void closeRound(float x,float y,float R,float clock,float forwardspeed,u8 Roundsize)
{
    float target_Distance=0;
    float k=0;
    float setangle=0,Agl=0,frontspeed=0;
    target_Distance=sqrt(pow(GetPosX()-x,2)+pow(GetPosY()-y,2))-R;
    k=(GetPosX()-x)/(y-GetPosY());		//前进直线斜率
	if(Roundsize==0)//大圈
	{
    PID_param_angle[0]=40.0f;
    PID_param_angle[1]=0.0f;
    PID_param_angle[2]=0.0f;

    PID_param_DisArc[0]=0.1;
    PID_param_DisArc[1]=0;
    PID_param_DisArc[2]=0;
	}
		if(Roundsize==1)//中圈
{
    PID_param_angle[0]=45.0f;
    PID_param_angle[1]=0.0f;
    PID_param_angle[2]=0.0f;

    PID_param_DisArc[0]=0.05;
    PID_param_DisArc[1]=0;
    PID_param_DisArc[2]=0;
	}
if(Roundsize==2)//小圈
{
    PID_param_angle[0]=30.0f;
    PID_param_angle[1]=0.0f;
    PID_param_angle[2]=0.0f;

    PID_param_DisArc[0]=0.03;
    PID_param_DisArc[1]=0;
    PID_param_DisArc[2]=0;
	}

//	顺1逆2
    if(clock==1)
    {
        if(GetPosY()>y)
            Agl=-90+atan(k)*180/PI;
        else if(GetPosY()<y)
            Agl=90+atan(k)*180/PI;
        else if(GetPosY()==y&&GetPosX()>=x)
            Agl=180;
        else if(GetPosY()==y&&GetPosX()<x)
            Agl=0;
        setangle=Agl+Distance_Arc_Pid(target_Distance);//距离赋值给角度的增量 Agl为角度转换常量
        if(setangle<0)
        {   LCD_ShowString(130,50,200,16,16,"setAngle =");
            LCD_ShowString(130,70,200,16,16,"-");
            LCD_ShowxNum(130,70,-setangle,6,16,0X80);
        }
        else {
            LCD_ShowString(130,50,200,16,16,"setAngle =");
            LCD_ShowString(130,70,200,16,16," ");
            LCD_ShowxNum(130,70,setangle,6,16,0X80);
        }
        frontspeed=AnglePid(setangle,GetAngle());//角度PID计算两轮速度差值
    }
    else if(clock==2)
    {
        if(GetPosY()>y)
            Agl=90+atan(k)*180/PI;
        else if(GetPosY()<y)
            Agl=-90+atan(k)*180/PI;
        else if(GetPosY()==y&&GetPosX()>=x)
            Agl=0;
        else if(GetPosY()==y&&GetPosX()<x)
            Agl=180;
        setangle=Agl+Distance_Arc_Pid(target_Distance);//距离赋值给角度的增量
        frontspeed=AnglePid(setangle,GetAngle());//角度PID计算两轮速度差值
    }
    motorCMD(forwardspeed+frontspeed,forwardspeed-frontspeed);//电机控制
}
void VelCrl(unsigned char motorNum,int vel)//电机控制  点到点控制函数
{
    if(motorNum==1)
		{
        give_motor1(vel);
		if(vel>0)
		{
    LCD_ShowString(60,170,200,16,16,"V left=");
    LCD_ShowxNum(60,190,vel,6,16,0X80);
		}
		else
		{
		
		LCD_ShowString(60,170,200,16,16,"V left=");
   	LCD_ShowString(60,190,200,16,16,"-");
		LCD_ShowxNum(60,190,-vel,6,16,0X80);	
		}
		}
    if(motorNum==2)
		{
        give_motor2(vel);
		if(vel>0)
		{
    LCD_ShowString(60,210,200,16,16,"V right=");
    LCD_ShowxNum(60,230,vel,6,16,0X80);
		}
		else
		{
		
		LCD_ShowString(60,210,200,16,16,"V right=");
    		LCD_ShowString(60,210,200,16,16,"-");
			LCD_ShowxNum(60,230,-vel,6,16,0X80);	
		}
		}
}

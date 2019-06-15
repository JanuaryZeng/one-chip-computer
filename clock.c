#include<reg52.h>
typedef unsigned int u16;
typedef unsigned char u8;

//矩阵键盘输出 端口
#define GPIO_KEY P1

unsigned char value;
//用来存放读取到的键值
unsigned char KeyState;	//记录按键的状态，0没有，1有
unsigned char jishiState = 0;//用来判断是否在计时状态
unsigned char daojishi = 0;//用来判断是否在倒计时状态

sbit led=P2^0;
sbit lsa=P2^2;
sbit lsb=P2^3;
sbit lsc=P2^4;
sbit beep=P1^5;
sbit k1=P3^1;
sbit k2=P3^0;
sbit k3=P3^2;
sbit k4=P3^3;
sbit dian=P0^7;

//lsa lsb lsc 用来选择显示的数码管
 
u8 value,alarmexist=0;
u8 smgduan[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x67,0x00};//段选码，分别对应0到无
u16 cnt=0,i=0,j=0,flag=0,ji=0,zhizhen=0,time[8]={0,0,0,0,0,0,0,0};//只用数码管0，1 3，4 6，7
u16 alarm[8]={0,0,0,0,0,0,1,0}, chucun[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
u16*p;
u8 ssec,sec,min;
u16 jisuan[8] = {0,0,0,0,0,0,0,0};
u8 tth0, ttl0;
u8 coun;
 
void delay(u16 mmp);
void t0andt1init();
void addproct(int x);						    
void addproca(int x);
void reduceproct(int x);
void reduceproca(int x);
void keyscan();
void jia();
void jian();
void KeyDown();		 //检测按键函数
void Delay10ms();   //延时10ms
void clear();//把时间清零准备计时
void DigDisplay();
void datapros();
u16 outputMS();
void outputSGM(u16 num);
	 
void main()
{
  led=0;
  t0andt1init();
	P0=0x00;
	while(1)
	{   
	    
		if(time[0]==alarm[0]&&time[1]==alarm[1]&&time[3]==alarm[3]&&time[4]==alarm[4]&&time[6]==alarm[6]&&time[7]==alarm[7])//满足条件 蜂鸣器以一定的时间间隔响,只到K4按键并松开,此时数码管任然正常显示当前时间
		{   
			alarmexist=0;
			while(k4)
			{
				beep=~beep;
				delay(25);
				beep=~beep;
				delay(25);
			}
		}
		keyscan();
		
		KeyDown();
		if(KeyState == 1){
			switch(value){
				case 0:
					TR0 = 0;
					ssec = 0;
					sec = 0;
					min = 0;
					zhizhen = 0;
					datapros();
					lsa=0;lsb=0;lsc=0;P0=smgduan[0];
					break;
				case 1:
					jishiState = 1;
					TR0 = 1;
					while(1){
						datapros();
						DigDisplay();	
						KeyDown();
						if(value == 2 && zhizhen < 15 && KeyState == 1){
							TR0 = 0;
							chucun[zhizhen] = outputMS();
							TR0 = 1;
							zhizhen++;
							KeyState = 0;
						}
						if(value == 3&& KeyState == 1){
							jishiState = 0;
							if(TR0 == 0)
								TR0 = 1;
							break;
						}
						if(value == 0&& KeyState == 1){
							tth0 = TH0;
							ttl0 = TL0;
							TR0 = 0;
							KeyState = 0;
						}
						if(value == 1&& KeyState == 1){
							TH0 = tth0;
							TL0 = ttl0;
							if(TR1 == 1){
								TR1 = 0;
							}
							TR0 = 1;
							KeyState = 0;
						}
						if(value == 4 && zhizhen>0&& KeyState == 1){
							zhizhen--;
							tth0 = TH0;
							ttl0 = TL0;
							if(TR0 != 0)
							TR0=0;//不在显示当前时间
							outputSGM(chucun[zhizhen]);
							KeyState = 0;
						}
					}
					break;
				case 8:
					TR0 = 0;
					ssec = 3;
					sec = 0;
					min = 0;
					daojishi = 1;
					datapros();
					DigDisplay();	
					while(1){
						KeyState = 0;
						datapros();
						DigDisplay();	
						KeyDown();
						if(KeyState == 1)
						switch(value){
							case 9:
								if(min == 23)
									min = 0;
								else
									min++;
								break;
							case 10:
								if(sec == 23)
									sec = 0;
								else
									sec++;
								break;
							case 11:
								if(ssec == 23)
									ssec = 0;
								else
									ssec++;
								break;
							case 13:
								if(min == 0)
									min = 23;
								else
									min--;
								break;
							case 14:
								if(sec == 0)
									sec = 23;
								else
									sec--;
								break;
							case 15:
								if(ssec == 0)
									ssec = 23;
								else
									ssec--;
								break;
							case 12:
								TR0 = 1;
								while(TR0 == 1){
									KeyState = 0;
									KeyDown();
									if(KeyState == 1){
										TR0 = 0;
										daojishi = 0;
										break;
									}	
								}
								ssec = 0;
								sec = 0;
								min = 0;
								datapros();
								DigDisplay();	
								break;
							default:
								break;
						}
						if(daojishi == 0){
							KeyState = 0;
							break;
						}
					}
					while(TR0 == 1){
						DigDisplay();	
						KeyDown();
						if(KeyState == 1){
							KeyState = 0;
							break;
						}
					}
					TR0 = 1;
					break;
				default:
					break;
			}
			KeyState = 0;
		}
	}
}


void delay(u16 mmp)
{
	while(mmp--);
}
void t0andt1init()
{
	TMOD=0x11;
	TH0=0xfc;
	TL0=0x67;//赋定时器初始值，定时1ms
	TH1=0xfc;
	TL1=0x67;
	TR0=1;//T0定时器开始定时
	EA=1;
	ET0=1;
	ET1=1;	
}
void addproct(int x)	  //对非调整状态和调整状态的加1处理(仅用于对当前时间)
{
	if(x==0||x==3||x==6)
		if(time[x]==10)
		{
			time[x]=0;
			time[x+1]=time[x+1]+1;
			addproct(x+1);
		}
   
   	if(x==1||x==4)
   		if(time[x]==6)
		{
			time[x]=0;
			time[x+2]=time[x+2]+1;
			addproct(x+2);
		}
	if(time[6]==4&&time[7]==2)
		time[6]=time[7]=0;		
}
void addproca(int x)	  //对调整状态的加1处理(仅用于对闹钟时间)
{
	if(x==0||x==3||x==6)
		if(alarm[x]==10)
		{
			alarm[x]=0;
			alarm[x+1]=alarm[x+1]+1;
			addproca(x+1);
		}
   
   	if(x==1||x==4)
   		if(alarm[x]==6)
		{
			alarm[x]=0;
			alarm[x+2]=alarm[x+2]+1;
			addproca(x+2);
		}
	if(alarm[6]==4&&alarm[7]==2)
		alarm[6]=alarm[7]=0;		
}
void reduceproct(int x) // 对调整状态的减1处理(仅用于对当前时间)
{
	if(time[0]==0&&time[1]==0&&time[3]==0&&time[4]==0&&time[6]==0&&time[7]==0)
	{
		time[0]=time[3]=9;
		time[1]=time[4]=5;
		time[6]=3;
		time[7]=2;
		return ;
	}
	if(x==0||x==3||x==6)
	{
		if(time[x]>=1)
			time[x]=time[x]-1;
		else
		{   
			if(time[7]==0&&x==6)
			{
				time[6]=3;
				time[7]=2;
			}
			else
			{
				time[x]=9;
				reduceproct(x+1);	
			}
		}
		
	}
	else
	{
		if(time[x]>=1)
			time[x]=time[x]-1;
		else
		{
			time[x]=5;
			if(x!=7)
				reduceproct(x+2);
			else
				time[x]=1;
		}
	}
	
}
void reduceproca(int x) // 对调整状态的减1处理(仅用于对闹钟时间)
{
	if(alarm[0]==0&&alarm[1]==0&&alarm[3]==0&&alarm[4]==0&&alarm[6]==0&&alarm[7]==0)
	{
		alarm[0]=alarm[3]=9;
		alarm[1]=alarm[4]=5;
		alarm[6]=3;
		alarm[7]=2;
		return ;
	}
	if(x==0||x==3||x==6)
	{
		if(alarm[x]>=1)
			alarm[x]=alarm[x]-1;
		else
		{   
			if(alarm[7]==0&&x==6)
			{
				alarm[6]=3;
				alarm[7]=2;
			}
			else
			{
				alarm[x]=9;
				reduceproca(x+1);	
			}
		}
		
	}
	else
	{
		if(alarm[x]>=1)
			alarm[x]=alarm[x]-1;
		else
		{
			alarm[x]=5;
			if(x!=7)
				reduceproca(x+2);
			else
				alarm[x]=1;
		}
	}
	
}
void keyscan()
{   
	flag=0;
	value=0;
    if(k1==0)
	{   
	    TR0=0;//不在显示当前时间
			TR1=1;
	    delay(1000);
		if(k1==0)
		{   
			while(!k1) ;
			delay(1000);
			led=~led;
		    value++;
		}
		else
			return ;
		
		while(value!=7)
		{
			if(k1==0)
			{   
			    delay(1000);
				if(k1==0)
				{
					while(!k1) ;
					led=~led;
		            value++;
					if(value==4)
						flag=1;
				} 
				
			}	
			if(k2==0)
			{
				delay(1000);
				if(k2==0)
				{
					while(!k2) ;
				 	led=~led;
					jia();
				}
			}
			if(k3==0)
			{
				delay(1000);
				if(k3==0)
				{
					while(!k3) ;
				    led=~led;
					jian();
				}
			}	
		}
		TR0=1;
		TR1=0;
		if(((alarm[6]+alarm[7]*10)*60+alarm[3]+alarm[4]*10)>((time[6]+time[7]*10)*60+time[3]+time[4]*10))//判断闹钟时间是否大于当前时间,
			alarmexist=1;
		else
			alarmexist=0;
		
		
	}
}			
void jia()
{
	switch(value)
	{
		case 1:time[0]=time[0]+1;addproct(0);break;
		case 2:time[3]=time[3]+1;addproct(3);break;
		case 3:time[6]=time[6]+1;addproct(6);break;
		case 4:alarm[0]=alarm[0]+1;addproca(0);break;
		case 5:alarm[3]=alarm[3]+1;addproca(3);break;
		case 6:alarm[6]=alarm[6]+1;addproca(6);break;
		
 
	}
}
void jian()
{
	switch(value)
	{
		case 1:reduceproct(0);break;
		case 2:reduceproct(3);break;
		case 3:reduceproct(6);break;
		case 4:reduceproca(0);break;
		case 5:reduceproca(3);break;
		case 6:reduceproca(6);break;
		
 
	}
	
}
void Timer0() interrupt 1//非调整时，执行的中断服务程序
{
	if(jishiState == 0 && daojishi == 0){
		TH0=0xfc;
		TL0=0x67;//赋定时器初始值，定时1ms
		cnt++;
		if(1000==cnt)
		{   
			cnt=0;
			time[0]=time[0]+1;
			addproct(0);
		}
		P0=0X00;
		switch(i)
		{	
			case 0:lsa=0;lsb=0;lsc=0;P0=smgduan[time[0]];dian=(alarmexist==1)?1:0;i++;break;
			case 1:lsa=1;lsb=0;lsc=0;P0=smgduan[time[1]];i++;i++;break;
			case 3:lsa=1;lsb=1;lsc=0;P0=smgduan[time[3]];i++;break;
			case 4:lsa=0;lsb=0;lsc=1;P0=smgduan[time[4]];i++;i++;break;
			case 6:lsa=0;lsb=1;lsc=1;P0=smgduan[time[6]];i++;break;
			case 7:lsa=1;lsb=1;lsc=1;P0=smgduan[time[7]];i=0;break;
		}
	
	}
	else if(jishiState == 0 && daojishi == 1){
		TH0=0xfc;
		TL0=0x67;//赋定时器初始值，定时1ms
		cnt++;
		if(1000==cnt)
		{   
			cnt=0;
			if(ssec == 0 && sec == 0 && min == 0)
			{
				daojishi = 0;
				TR0 = 0;
			}else{
				
			if(ssec >= 1)
				ssec--;
			else if(sec >= 1){
				ssec = 59;
				sec--;
			}
			else if(min >= 1){
				ssec = 59;
				sec = 59;
				min--;
				}
			}
			datapros();
		}
		P0=0X00;
		switch(i)
		{	
			case 0:lsa=0;lsb=0;lsc=0;P0=jisuan[0];i++;break;
			case 1:lsa=1;lsb=0;lsc=0;P0=jisuan[1];i++;break;
			case 2:lsa=0;lsb=1;lsc=0;P0=0x40;i++;break;
			case 3:lsa=1;lsb=1;lsc=0;P0=jisuan[3];i++;break;
			case 4:lsa=0;lsb=0;lsc=1;P0=jisuan[4];i++;break;
			case 5:lsa=1;lsb=0;lsc=1;P0=0x40;i++;break;
			case 6:lsa=0;lsb=1;lsc=1;P0=jisuan[6];i++;break;
			case 7:lsa=1;lsb=1;lsc=1;P0=jisuan[7];i=0;break;
		}
	}
	else{
		TH0=0Xd8;	//给定时器赋初值，定时10ms
		TL0=0Xf0;
		ssec++;
		if(ssec>=100)  //1s
		{
			ssec=0;
			sec++;
			if(sec>=60)
			{
				sec=0;
				min++;
				if(min>=60)
				{
					min=0;
				}
			}	
		}	
	}
}
void Timer1() interrupt 3	//调整时，执行的中断服务程序  ，flag=0显示正在调整的time，为1显示正在调整的alarm
{
		TH1=0xfc;
		TL1=0x67;//赋定时器初始值，定时1ms
		if(flag==0 && daojishi == 0)
			p=time;
		else
			p=alarm;
		P0=0X00;
		switch(j)
		{	
			case 0:lsa=0;lsb=0;lsc=0;P0=smgduan[p[0]];j++;break;
			case 1:lsa=1;lsb=0;lsc=0;P0=smgduan[p[1]];j++;j++;break;
			case 3:lsa=1;lsb=1;lsc=0;P0=smgduan[p[3]];j++;break;
			case 4:lsa=0;lsb=0;lsc=1;P0=smgduan[p[4]];j++;j++;break;
			case 6:lsa=0;lsb=1;lsc=1;P0=smgduan[p[6]];j++;break;
			case 7:lsa=1;lsb=1;lsc=1;P0=smgduan[p[7]];j=0;break;
		}
}

void DigDisplay()
{
	u8 i;
	for(i=0;i<8;i++)
	{
		switch(i)	 //位选，选择点亮的数码管，
		{
			case(0):
				lsa=0;lsb=0;lsc=0; break;//显示第0位
			case(1):
				lsa=1;lsb=0;lsc=0; break;//显示第1位
			case(2):
				lsa=0;lsb=1;lsc=0; break;//显示第2位
			case(3):
				lsa=1;lsb=1;lsc=0; break;//显示第3位
			case(4):
				lsa=0;lsb=0;lsc=1; break;//显示第4位
			case(5):
				lsa=1;lsb=0;lsc=1; break;//显示第5位
			case(6):
				lsa=0;lsb=1;lsc=1; break;//显示第6位
			case(7):
				lsa=1;lsb=1;lsc=1; break;//显示第7位	
		}
		P0=jisuan[i];//发送段码
		delay(100); //间隔一段时间扫描	
		P0=0x00;//消隐
	}
}
void datapros()
{
	jisuan[0]=smgduan[ssec%10];
	jisuan[1]=smgduan[ssec/10];
	jisuan[2]=0x40;	
	jisuan[3]=smgduan[sec%10];
	jisuan[4]=smgduan[sec/10];
	jisuan[5]=0x40;
	jisuan[6]=smgduan[min%10];
	jisuan[7]=smgduan[min/10];
}
/*******************************************************************************
* 函 数 名         : KeyDown
* 函数功能		   : 检测有按键按下并读取键值
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void KeyDown(void)
{
	unsigned int a=0;
	GPIO_KEY=0x0f;
	if(GPIO_KEY!=0x0f)
  {
		Delay10ms();
		a++;	
		a=0;
		if(GPIO_KEY!=0x0f)
		{
			KeyState=1;//有按键按下
			//测试列
			GPIO_KEY=0X0F;
// 			Delay10ms();
			switch(GPIO_KEY)
			{
				case(0X07):	value=0;break;
				case(0X0b):	value=1;break;
				case(0X0d): value=2;break;
				case(0X0e):	value=3;break;
//				default:	value=17;	//检测出错回复17意思是把数码管全灭掉。
			}
			//测试行
			GPIO_KEY=0XF0;
			Delay10ms();
			switch(GPIO_KEY)
			{
				case(0X70):	value=value;break;
				case(0Xb0):	value=value+4;break;
				case(0Xd0): value=value+8;break;
				case(0Xe0):	value=value+12;break;
//				default:	value=17;
			}
			while((a<500)&&(GPIO_KEY!=0xf0))	 //按键松手检测
			{
				Delay10ms();
				a++;
			}
			a=0;
		}
	}
}
/*******************************************************************************
* 函 数 名         : Delay10ms
* 函数功能		   : 延时函数，延时10ms
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void Delay10ms(void)   //误差 0us
{
    unsigned char a,b,c;
    for(c=1;c>0;c--)
        for(b=38;b>0;b--)
            for(a=130;a>0;a--);
}
u16 outputMS(){
	u16 num = 0;
	num += ssec;
	num += sec*100;
	num += min*10000;
	return num;
}

void outputSGM(u16 num){
	min = num/10000;
	sec = (num - min*10000)/100;
	ssec = num - min*10000 - sec*100;
}
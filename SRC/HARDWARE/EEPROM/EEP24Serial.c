#define _EEP24SERIAL_GLOBALS_
#include "common.h"

//IO操作函数
#if (defined A12_901)||(defined A12_909)
#define iic_scl		PBout(15) //SCL ---PC3
#define iic_sda		PBout(14) //SDA ---PA0
#define READ_SDA   	PBin(14)  //输入SDA
#endif
#ifdef A12_906
#define iic_scl     PAout(1) //SCL
#define iic_sda     PAout(0) //SDA
#define READ_SDA    PAin(0)  //输入SDA
#endif

#define  EEP_sck_Set()   	( iic_scl = 1)      			//Pa0
#define  EEP_sck_Clr()   	( iic_scl = 0)
#define  EEP_sda_Set()   	( iic_sda = 1)      			//PC3
#define  EEP_sda_Clr()  	( iic_sda = 0)
#define  Is_EEP_sda()	 	( READ_SDA)

#define	delay_t1			50//5
#define	delay_t2			1500//250
#define	delay_t3			300//50

#define SomeNOP()   	iic_flash(delay_t1)
#define I2CNbusy()		(EEP_sda_Set(),EEP_sck_Set(),SomeNOP())
#define I2CStart()		(EEP_sda_Set(),EEP_sck_Set(),SomeNOP(),EEP_sda_Clr(),SomeNOP())
#define I2CStop()		(EEP_sda_Clr(),EEP_sck_Set(),SomeNOP(),EEP_sda_Set(),SomeNOP())

#define OP_READ 		0xa1  	// 器件地址以及读取操作
#define OP_WRITE 		0xa0  	// 器件地址以及写入操作

// -----------------------------------------------------------// -----------------------------------------------------------
void iic_INIT(void)
{
    #if (defined A12_901)||(defined A12_909)
	RCC->APB2ENR |= (RCC_APB2Periph_GPIOB) ;    			//使能PORTA,PORTC时钟

	GPIOB->CRH &= GPIO_Crh_P14;
	GPIOB->CRH |= GPIO_Mode_Out_PP_50MHz_P14;
    GPIOB->ODR |= GPIO_Pin_14; 													//输出高

	GPIOB->CRH &= GPIO_Crh_P15;
	GPIOB->CRH |= GPIO_Mode_Out_PP_50MHz_P15;
    GPIOB->ODR |= GPIO_Pin_15; 													//输出高
    #endif
    #ifdef A12_906
	RCC->APB2ENR |= (RCC_APB2Periph_GPIOA) ;    			//使能PORTA,PORTC时钟

	GPIOA->CRL &= GPIO_Crl_P0;
	GPIOA->CRL |= GPIO_Mode_Out_PP_50MHz_P0;
    GPIOA->ODR |= GPIO_Pin_0; 													//输出高

	GPIOA->CRL &= GPIO_Crl_P1;
	GPIOA->CRL |= GPIO_Mode_Out_PP_50MHz_P1;
    GPIOA->ODR |= GPIO_Pin_1; 													//输出高
    #endif
}

void EEP_da_in(void )
{
    #if (defined A12_901)||(defined A12_909)
	GPIOB->CRH &= GPIO_Crh_P14;
	GPIOB->CRH |= GPIO_Mode_IN_PU_PD_P14;
    GPIOB->ODR |= GPIO_Pin_14; 													//输入上拉
    #endif
    #ifdef A12_906
	GPIOA->CRL &= GPIO_Crl_P0;
	GPIOA->CRL |= GPIO_Mode_IN_PU_PD_P0;
    GPIOA->ODR |= GPIO_Pin_0; 													//输入上拉
    #endif
}

void EEP_da_out(void)
{
    #if (defined A12_901)||(defined A12_909)
	GPIOB->CRH &= GPIO_Crh_P14;
	GPIOB->CRH |= GPIO_Mode_Out_PP_50MHz_P14;
    GPIOB->ODR |= GPIO_Pin_14; 													//输出高
    #endif
    #ifdef A12_906
	GPIOA->CRL &= GPIO_Crl_P0;
	GPIOA->CRL |= GPIO_Mode_Out_PP_50MHz_P0;
    GPIOA->ODR |= GPIO_Pin_0; 													//输出高
    #endif
}


void iic_flash(unsigned short clock)
{
	while(clock--);
}

unsigned char I2CPageRead_Nbytes(unsigned short StartAddr, unsigned short length,unsigned char *pRbuf)
{
	unsigned char j,temp,eI2c_rCheckSum;
	unsigned short i;

	eI2c_rCheckSum = 0;

	EEP_da_out();
	I2CStart();
	EEP_sck_Clr();
	SomeNOP();
	for(i=0;i < (length + 4);i++)
	{
		#if EEPROM_MoreThan16K
			if(i == 0)	temp = OP_WRITE;							//send random read mode command
			else if(i == 1) temp = (unsigned char)(StartAddr>>8);		//send address Msb
			else if(i == 2) temp = (unsigned char)StartAddr;			//send address Lsb
		#else
			if(i == 0) continue;								// skip
			else if(i == 1)
			{
				temp = (unsigned char)(StartAddr>>8);			//send address Msb
				temp <<= 1;
				temp += OP_WRITE;							// send random read mode command
			}
			else if(i == 2) temp = (unsigned char)StartAddr;		//send address Lsb
		#endif
		else if(i == 3)										//send read command
		{
			I2CStart();
			EEP_sck_Clr();
			SomeNOP();
			#if EEPROM_MoreThan16K
				temp = OP_READ;
			#else
				temp = (unsigned char)(StartAddr>>8);			//send address Msb
				temp <<= 1;
				temp += OP_READ;						// send random read mode command
			#endif
		}
		else 	{EEP_da_in();temp = 0;}		//read data

		for(j=0;j<8;j++)
		{
			if(i >= 4)
			{
				EEP_sck_Set();
				SomeNOP();
				temp <<= 1;
				if(Is_EEP_sda()) temp++;	//clock low read data
				EEP_sck_Clr();
				SomeNOP();
			}
			else
			{
				if(temp & 0x80) EEP_sda_Set();
				else EEP_sda_Clr();			//clock low change data
				EEP_sck_Set();
				SomeNOP();
				EEP_sck_Clr();
				SomeNOP();
				temp <<= 1;
			}
		}

		if(i>=4)
		{
			EEP_da_out();
			if(i< (length + 3)) EEP_sda_Clr();		//ack
			else EEP_sda_Set();					//nack
			EEP_sck_Set();
			SomeNOP();
			EEP_sck_Clr();
			SomeNOP();
			*pRbuf ++ =  temp;
	//		TxOneData(temp );   // test
			eI2c_rCheckSum ^= temp;
		}
		else
		{
			//wait for ack
			EEP_da_in();
			EEP_sck_Set();
			SomeNOP();
			for(j=0;j<200;j++)
			{
				if(Is_EEP_sda())  iic_flash(delay_t3);
				else break;
			}
			EEP_sck_Clr();
			SomeNOP();
			EEP_da_out();
		}
	}
	I2CStop();
	I2CNbusy();
	for(j=0;j<20;j++)
	{
		iic_flash(delay_t2);
		iic_flash(delay_t2);
		iic_flash(delay_t2);
		iic_flash(delay_t2);
	}

	return(eI2c_rCheckSum);
}

unsigned char I2CPageWrite_Nbytes (unsigned short StartAddr, unsigned short length,unsigned char *pWbuf)
{
	unsigned char i,j,temp,Length_temp,eI2c_wCheckSum;
	unsigned short count,loop;

	eI2c_wCheckSum = 0;
	// 计算要写的次数
	Length_temp = length % Page_Size;
	loop = length  / Page_Size;
	if(Length_temp) loop++;
	for(count =0 ; count < loop; count++)
	{// 写的次数
		#if EEPROM_MoreThan16K
			// 换算连续写的长度
			Length_temp = StartAddr % Page_Size;
			if(Length_temp)
			{// 写入的起始地址不是页最大操作数的倍数
				Length_temp = Page_Size - Length_temp;	// 与页最大操作数的差值
				if(length <= Length_temp) Length_temp = length;
				else loop++;
			}
			else  if(length >= Page_Size) Length_temp = Page_Size;
			else  Length_temp = length;
		#else
			// 换算连续写的长度
			Length_temp = StartAddr % Page_Size;		// 小页换算
			temp = StartAddr % 256;					// 大页换算
			temp = 256 -temp;
			if(Length_temp > temp )
			{// 大页换算
				Length_temp = temp;	// 与页最大操作数的差值
				loop++;
			}
			else if(Length_temp)
			{// 写入的起始地址不是页最大操作数的倍数--- 小页换算
				Length_temp = Page_Size - Length_temp;	// 与页最大操作数的差值
				if(length <= Length_temp) Length_temp = length;
				else loop++;
			}
			else  if(length >= Page_Size) Length_temp = Page_Size;
			else  Length_temp = length;

		#endif

		// 开始写数据
		EEP_da_out();
		I2CStart();
		EEP_sck_Clr();
		SomeNOP();
		for(i=0;i<(Length_temp+3);i++)
		{
			#if EEPROM_MoreThan16K
				if(i == 0) 	temp = OP_WRITE;						//send random read mode command
				else if(i == 1) temp = (unsigned char)(StartAddr>>8);	//send address Msb
			#else
				if(i == 0) continue;								// skip
				else if(i == 1)
				{
					temp = (unsigned char)(StartAddr>>8);			//send address Msb
					temp <<= 1;
					temp += OP_WRITE;							// send random read mode command
				}
			#endif
			else if(i == 2) temp = (unsigned char)StartAddr;		//send address Lsb
			else
			{
				temp = *pWbuf ++;
		//		TxOneData(temp );		// test
				eI2c_wCheckSum ^= temp;
			}
			for(j=0;j<8;j++)
			{
				if(temp & 0x80) EEP_sda_Set();						//clock low change data
				else EEP_sda_Clr();
				EEP_sck_Set();
				SomeNOP();
				EEP_sck_Clr();
				SomeNOP();
				temp <<= 1;
			}
			//wait for ack
			EEP_da_in();
			EEP_sck_Set();
			SomeNOP();
			for(j=0;j<200;j++)
			{
				if(Is_EEP_sda()) iic_flash(delay_t3);
				else break;
			}
			EEP_sck_Clr();
			SomeNOP();
			EEP_da_out();
		}
		I2CStop();
		I2CNbusy();

		for(j=0;j<20;j++)
		{//换页要延时
			iic_flash(delay_t2);
			iic_flash(delay_t2);
			iic_flash(delay_t2);
			iic_flash(delay_t2);
		}

		// 计算新的地址
		StartAddr += Length_temp;
		// 扣除已经写的长度
		length -= Length_temp;

	}
	return(eI2c_wCheckSum);
}



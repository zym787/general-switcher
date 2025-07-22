#ifndef _STMFLASH_H_
#define _STMFLASH_H_
#ifdef _STMFLASH_GLOBALS_
#define PEXT 
#else
#define PEXT extern
#endif

//用户根据自己的需要设置
#define STM32_FLASH_SIZE 	64 	 		//所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_WREN 	1              	//使能FLASH写入(0，不使能;1，使能)

//FLASH起始地址
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH的起始地址
//FLASH解锁键值
#define FLASH_KEY1               0x45670123
#define FLASH_KEY2               0xCDEF89AB

PEXT void STMFLASH_Unlock(void);					  //FLASH解锁
PEXT void STMFLASH_Lock(void);					  //FLASH上锁
PEXT unsigned char STMFLASH_GetStatus(void);				  //获得状态
PEXT unsigned char STMFLASH_WaitDone(unsigned short time);				  //等待操作结束
PEXT unsigned char STMFLASH_ErasePage(unsigned int paddr);			  //擦除页
PEXT unsigned char STMFLASH_WriteHalfWord(unsigned int faddr, unsigned short dat);//写入半字
PEXT unsigned short STMFLASH_ReadHalfWord(unsigned int faddr);		  //读出半字  
PEXT void STMFLASH_WriteLenByte(unsigned int WriteAddr,unsigned int DataToWrite,unsigned short Len);	//指定地址开始写入指定长度的数据
PEXT unsigned int STMFLASH_ReadLenByte(unsigned int ReadAddr,unsigned short Len);						//指定地址开始读取指定长度数据
PEXT void STMFLASH_Write(unsigned int WriteAddr,unsigned short *pBuffer,unsigned short NumToWrite);		//从指定地址开始写入指定长度的数据
PEXT void STMFLASH_Read(unsigned int ReadAddr,unsigned short *pBuffer,unsigned short NumToRead);   		//从指定地址开始读出指定长度的数据

//测试写入
PEXT void Test_Write(unsigned int WriteAddr,unsigned short WriteData);	

#undef PEXT							   
#endif


















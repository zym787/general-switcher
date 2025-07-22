#ifndef _EEP24SERIAL_H_
#define _EEP24SERIAL_H_

#ifdef _EEP24SERIAL_GLOBALS_
#define PEXT
#else
#define PEXT extern
#endif

#define  EEPROM_MoreThan16K	0		// ≥¨π˝16K±»Ãÿ÷√1

#if EEPROM_MoreThan16K
	#define Page_Size 	128		//24lc512: The 512Kbit EEPROM is capable of 128-byte page writes
	#define TotalPage 	512		// 512k/8 =64Kbyte, 64*1024/128 = 512.

//	#define Page_Size 	32		// The 64K EEPROM is capable of 32-byte page writes
//	#define TotalPage 	256		// 64/8 =8k, 8*1024/32 = 256.
#else
//	#define Page_Size 	16		//24lc16: The 16 Kbit EEPROM is capable of 16-byte page writes
//	#define TotalPage 	128		// 16/8 =2k, 2*1024/16 = 128.

	#define Page_Size 	16		//24lc04: The 4Kbit EEPROM is capable of 16-byte page writes
	#define TotalPage 	32		// 4k/8 =512byte, 512/16 = 32
	
//	#define Page_Size 	8		//24lc02: The 2 Kbit EEPROM is capable of 8-byte page writes
//	#define TotalPage 	32		// 2k/8 =256byte, 256/8 = 32
#endif

PEXT void		    iic_INIT(void);
PEXT void   			iic_flash(unsigned short clock);
PEXT unsigned char 	I2CPageRead_Nbytes(unsigned short StartAddr, unsigned short length,unsigned char *pRbuf);
PEXT unsigned char 	I2CPageWrite_Nbytes (unsigned short StartAddr, unsigned short length,unsigned char *pWbuf);
PEXT unsigned char  	GetSystemParam(void);

#undef PEXT
#endif


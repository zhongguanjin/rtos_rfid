#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "stm32f10x.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"





typedef int8_t	    int8;
typedef int16_t		int16;
typedef int32_t	    int32;

typedef     uint8_t		uint8;
typedef     uint16_t		uint16;
typedef     uint32_t		uint32;


typedef char					i8;

typedef signed     short   		i16;

typedef signed     int    		i32;




#define  false     0
#define  true      1


//#pragma anon_unions

#define OK		(1)
#define ERR		(0)
#define GoOn    (2)

#define ON 1
#define OFF 0


typedef union {
	uint32		u;		// u=0x01020304
	int32		i;
	uint16		ush[2];
	int16		sh[2];
	uint8		uch[4];	// ch[0]=0x04 ch[1]=03
	int8		ch[4];
} UN32;

typedef union {
	uint16		ush;
	int16		sh;
	uint8		uch[2];
	int8		ch[2];
} UN16;

#define u32Type(dt0,dt1,dt2,dt3)		(((u32)dt3<<24)|((u32)dt2<<16)|((u32)dt1<<8)|(u32)dt0)
#define u16Type(dt0,dt1)		        (((u16)dt1<<8)|(u16)dt0)


#define val_and(REG,VAL)		((REG) &= (VAL))	// 单纯与操作
#define val_or(REG, VAL)		((REG) |= (VAL))	// 单纯或操作
#define val_xor(REG, VAL)		((REG) ^= (VAL))	// 单纯异或操作

#define val_setb(REG,VAL)		((REG) |= (VAL))	// 设置VAL中相应=1的位
#define val_clrb(REG,VAL)		((REG) &= ~(VAL))	// 清除VAL中相应=1的位

                        // DT:u8 u16 u32
#define bit_set(DT, REG, BN)	((REG) |= (((DT)1)<<(BN)))
#define bit_clr(DT, REG, BN)	((REG) &= ~(((DT)1)<<(BN)))
#define bit_xor(DT, REG, BN)	((REG) ^= (((DT)1)<<(BN)))
#define testbit(DT, REG, BN)	((REG) & (((DT)1)<<(BN)))

#endif


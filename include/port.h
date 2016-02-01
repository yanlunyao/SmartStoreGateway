#ifndef _PORT_H_
#define _PORT_H_

/*
  Data type
*/


typedef unsigned char           u1_t;			/* unsigned  8 bit type         */
typedef signed   char		        s1_t;			/*   signed  8 bit type         */
typedef const unsigned char     cu1_t;    /* const unsigned 8 bit type    */
typedef const signed char       cs1_t;    /* const signed 8 bit type      */
typedef unsigned short	        u2_t;			/* unsigned 16 bit type         */
typedef signed   short 	        s2_t;			/*   signed 16 bit type         */
typedef const unsigned short    cu2_t;    /* const unsigned 16 bit type   */
typedef const signed short      cs2_t;    /* const signed 16 bit type     */
typedef unsigned int		        u4_t;			/* unsigned 32 bit type         */
typedef signed   int		        s4_t;			/*   signed 32 bit type         */
typedef const unsigned int      cu4_t;    /* const unsigned 32 bit type   */
typedef const signed int        cs4_t;    /* const signed 32 bit type     */

//typedef u4_t                	size_t;

typedef u1_t            *pu1_t;
typedef s1_t            *ps1_t;
typedef u2_t            *pu2_t;
typedef s2_t            *ps2_t;
typedef u4_t            *pu4_t;
typedef s4_t            *ps4_t;
typedef pu1_t           str_t;
typedef u2_t            err_t;
typedef void            (*vfp_t)( void);
typedef u4_t            specialRegister_t;

#define enum8(name)		  name##_et; typedef u1_t	name##_t
#define enum16(name)		name##_et; typedef u2_t	name##_t

typedef struct
{
    u1_t b0:1;
    u1_t b1:1;
    u1_t b2:1;
    u1_t b3:1;
    u1_t b4:1;
    u1_t b5:1;
    u1_t b6:1;
    u1_t b7:1;
}struct_ByteBit8;

typedef struct
{
    u2_t b0 :1;
    u2_t b1 :1;
    u2_t b2 :1;
    u2_t b3 :1;
    u2_t b4 :1;
    u2_t b5 :1;
    u2_t b6 :1;
    u2_t b7 :1;
    u2_t b8 :1;
    u2_t b9 :1;
    u2_t b10:1;
    u2_t b11:1;
    u2_t b12:1;
    u2_t b13:1;
    u2_t b14:1;
    u2_t b15:1;
}struct_WordBit16;

typedef struct
{
    u1_t l:4;                          //0
    u1_t h:4;                          //1
}struct_halfbytex2;

typedef union
{
    u1_t 								int8u;
		struct_ByteBit8			ByteBit8 ;
    struct_halfbytex2 	halfbytex2;
}union_byte;

typedef struct
{
    u1_t hl:4;                          //0
    u1_t hh:4;                          //1

    u1_t ll:4;                          //0
    u1_t lh:4;                          //1
}struct_halfbytex4;

typedef union
{
    u2_t int16u;
    u1_t int8u[2];
		struct
    {
      u1_t Hbyte;
      u1_t Lbyte;                            //INT_16.h 是int_16的高字节
    } U_2byte;                        //int8[0]是高字节
		struct_WordBit16		WordBit16	;
    struct_halfbytex4 	halfbytex4;
}union_word;

typedef struct
{
    u1_t h1l:4;                          //1
    u1_t h1h:4;                          //0

    u1_t h0l:4;                          //1
    u1_t h0h:4;                          //0

    u1_t l0l:4;                          //1
    u1_t l0h:4;                          //0

    u1_t l1l:4;                          //1
    u1_t l1h:4;                          //0
}struct_halfbytex8;

typedef union
{
//    Int32s int32s;
    u4_t int32u;
//    FlPo32 flot32;
//    Int08s int8s[4];
    u1_t int8uArry[4];                        //int8[0]是最高字节
//    STRUCTINT16X2 INT16X2;
//    STRUCTINT8X4 INT8X4;
//    STRUCTBIT4X8 structbit4x8;
		struct_halfbytex8  halfbytex8	;
}union_doubleword;

typedef enum
{
  FALSE = 0,
  TRUE  = 1
}bool_t;

#define  ENABLE     true
#define  DISABLE    !(true)

#define  SET        true
#define  CLEAR      !(SET)

#define  BITSET     true
#define  BITCLEAR   !(BITSET)

//#define false 			FALSE
//#define true        TRUE
#define equal			  0				/* for strcmp() and memcmp() */

#ifndef NULL
  #define NULL 			0
#else
  #undef NULL
  #define NULL 			0
#endif

#define U1_MIN_VALUE 	0x00
#define U1_MAX_VALUE 	0xff

#define U2_MIN_VALUE 	0x0000
#define U2_MAX_VALUE 	0xffff

#define U4_MIN_VALUE 	0x00000000
#define U4_MAX_VALUE 	0xffffffff

#define S1_MIN_VALUE 	0xff
#define S1_MAX_VALUE 	0x7f

#define S2_MIN_VALUE 	0xffff
#define S2_MAX_VALUE 	0x7fff

#define S4_MIN_VALUE 	0xffffffff
#define S4_MAX_VALUE 	0x7fffffff

#define VPint    		*(volatile unsigned int *)

#define sfr(x) 			(VPint(x)) 			/* Access SFRs through this macro when coding in C 	*/



/*
  Macro functions
*/

/* Accept bit index such as 0, 1, 2, etc */
#define setBit(x,b)         (x) |=  (1U<<(b))
#define clearBit(x,b)		    (x) &= ~(1U<<(b))
#define flipBit(x,b)		    (x) ^=  (1U<<(b))
#define testBit(x,b)		    ((x) &  (1U<<(b)))
#define bitIsCleared(a,b)   (((a) & (1U << (b))) == 0)
#define bitIsSet(a,b)       (((a) & (1U << (b))) != 0)


#define setBitField(x,f) 	      (x |= f)
#define clearBitField(x,f) 	    (x &= (~(f)))
#define isBitFieldSet(x,f)      ((x & f) == f)
#define isBitFieldClear(x,f)    ((x & f) == 0)

/* Only for 32-bit values (word) in 32-bit system */
#define highHalfWord(x) 	  (((x) >> 16) & 0xffff)		  /* bits 16-31	*/
#define lowHalfWord(x) 	    ((x) & 0xffff)			        /* bits 0-15	*/

/* Only for 16-bit values (half word) in 32-bit system */
#define highByte(x)		(((x) >> 8) & 0xff)		        /* bits 8-15	  */
#define lowByte(x)		((x) & 0xff)			            /* bits 0-7	    */
#define HIGH(x)			  highByte(x)		                /* bits 8-15	  */
#define LOW(x)			  lowByte(x)		                /* bits 0-7	    */
#define highNibble(x)	(((x) >> 4) & 0xf)            /* bits 4-7	    */
#define lowNibble(x)	((x) & 0xf)			              /* bits 0-3	    */

/* Loop */
#define forever			    for(;;)				/* Forever loop				*/
#define halt()			    Forever{}			/* Do nothing forever 		*/


#define copyBit(x,b,val)        \
        {                       \
          if(val)               \
          {                     \
            SetBit(x,b);        \
		      }                     \
		      else                  \
          {                     \
            clearBit(x,b);      \
          }                     \
		    }

/*
  Operators
*/

#define and         &&
#define or          ||
#define bitAnd      &
#define bitOr       |



/*
  Odd/even test
*/

#define odd(x)      ((x) & 0x01)
#define even(x)     (!Odd(x))



#define reentrant 		/* */
#define xdata 			/* */



/*
  Adding to pointers of anything besides bytes does unexpected things.
  So here is a nicer way to do it.
*/
#define byteAdd(ptr,bytes)	(((u1_t *)(ptr))+(bytes))


/*
  Set a buffer to all 0.
  "buffer" must a structure or array, not a pointer.
*/
#define memErase(buffer)	memset( &buffer, 0, sizeof(buffer))


/*
  Increment a variable but wrap around before it gets to max.
*/
#define modInc(var,max) \
    var++;              \
    if( var >= max)     \
    {                   \
      var = 0;          \
    }


/*
  Decrement a variable but wrap around before it gets to max.
*/
#define modDec(var,max)     \
    if( var == 0)           \
    {                       \
      var = max - 1;        \
    }                       \
    else                    \
    {                       \
      var--;                \
    }

#define limitInc(var, max)  \
    if(var < max)           \
    {                       \
      var++;                \
    }

#define limitDec(var, mix)  \
    if(var > mix)           \
    {                       \
      var--;                \
    }

#define modIncInBound(var, min, max) \
		var++;              \
    if( var >= max)     \
    {                   \
      var = min;        \
    }

/*
  Rotate an 8 or 16 bit variable left (<<) or right (>>).
  Make sure that p is an lvalue (preferably a simple variable).
  (i.e.  x, s.x, *p, p->x, a[e], ...
   not   i++, f(), I/O, a, p, s, &x, exp, ...)
*/
#define rotateLeft8( val)		\
    if( val & 0x80)		      \
    {				                \
      val <<= 1;		        \
      val |=  1;		        \
    }			    	            \
    else			              \
    {				                \
      val <<= 1;		        \
    }

#define rotateRight8( val)	\
    if( val & 0x01)		      \
    {				                \
      val >>= 1;		        \
      val |=  0x80;		      \
    }				                \
    else			              \
    {				                \
      val >>= 1;		        \
    }

#define rotateLeft16( val)	\
    if( val & 0x8000)       \
    {				                \
      val <<= 1;		        \
      val |=  1;		        \
    }				                \
    else			              \
    {				                \
      val <<= 1;		        \
    }

#define rotateRight16( val) \
    if( val & 0x0001)	      \
    {				                \
      val >>= 1;		        \
      val |=  0x8000;	      \
    }				                \
    else			              \
    {				                \
      val >>= 1;		        \
    }

#define rotateLeft32( val)  \
    if( val & 0x80000000)	  \
    {				                \
      val <<= 1;		        \
      val |=  1;		        \
    }				                \
    else			              \
    {				                \
      val <<= 1;		        \
    }

#define rotateRight32( val) \
    if( val & 0x00000001)	  \
    {				                \
      val >>= 1;		        \
      val |=  0x80000000;	  \
    }				                \
    else			              \
    {				                \
      val >>= 1;		        \
    }

//#define _nop_() __asm__ __volatile__ ("nop"::)
#define _nop_() asm("nop")
//宏定义方法：
#define __NOP1__ asm("nop");
#define __NOP2__ __NOP1__ __NOP1__
#define __NOP4__ __NOP2__ __NOP2__
#define __NOP8__ __NOP4__ __NOP4__
#define __NOP16__ __NOP8__ __NOP8__
#define __NOP32__ __NOP16__ __NOP16__
#define __NOP64__ __NOP32__ __NOP32__
#define __NOP128__ __NOP64__ __NOP64__


#define __NOPX__(a)             \
    if ((a)&(0x01))    {__NOP1__}                    \
    if ((a)&(0x02))    {__NOP2__}                    \
    if ((a)&(0x04))    {__NOP4__}                    \
    if ((a)&(0x08))    {__NOP8__}                    \
    if ((a)&(0x10))    {__NOP16__}                    \
    if ((a)&(0x20))    {__NOP32__}                    \
    if ((a)&(0x40))    {__NOP64__}                    \
    if ((a)&(0x80))    {__NOP128__}

#define 	TIMEOUT             0x0
#define 	CONTINUE            0xFF

#endif


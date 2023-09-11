#include "stdio.h"
#include "stdint.h"
#include "utils.h"
#include "string.h"
#include <stdlib.h>

uint8_t ConvertHex2BCD(uint8_t bData );
uint8_t calcCS(uint8_t *pbytes, uint16_t plen);

const char HexMapL[16]={'0','1','2','3','4','5',
					    '6','7','8','9','A','B',
					    'C','D','E','F'};

void ToUpperCase(uint16_t * lowChar)
{
	if(* lowChar>'Z')
		* lowChar-=32;
}
/* reverse:  reverse string s in place */
void reverse(uint8_t * s)
{
    uint8_t c, i, j,len;
	len=strlen((char *)s);
	if(len==1){
		c=s[0];
		s[0]='0';
		s[1]=c;
		s[2] = '\0';
	}


	
    for (i = 0, j =len-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}
/* itoa:  convert 32 bit n to characters in s */
void i32toa(uint32_t n, uint8_t * s,int base)
{
    int i;
	
   // if ((sign = n) < 0)  /* record sign */
   //     n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % base + '0';   /* get next digit */
    } while ((n /= base) > 0);     /* delete it */
   // if (sign < 0)
    //    s[i++] = '-';
    s[i] = '\0';
//	if(n<10)s[1]=0x30;
    reverse(s);
}

uint16_t m_atoi(uint16_t *s,int base)
{
	uint16_t num;
	if(s[1]<'0' || s[1]>'9')
		num=s[0]-48;
	else
		num=(s[0]-48)*base+s[1]-48;
	return num;

}
/*
uint32_t xto32i(uint16_t *s, uint16_t sLen)
{
	uint32_t result = 0,i;
	BOOL firsttime = TRUE;
	ToUpperCase(s+1);
	if (*s == '0' &&( *(s + 1) ==  'X')) s += 2;
	while (sLen--)
	{
    	BOOL found = FALSE;
	    for (i = 0; i < 16; i++)
	    {
		  ToUpperCase(s);
	      if (*s == HexMapL[i])
	      {
	        if (!firsttime) result <<= 4;
	        result |= i;
	        found = TRUE;
	        break;
	      }
	    }
	    if (!found) break;
	    s++;
	    firsttime = FALSE;
	  }
	
	  return result;

}*/
/*
uint16_t xto16i(uint16_t *s, uint16_t sLen)
{
	uint32_t result = 0,i;
	BOOL firsttime = TRUE;
	ToUpperCase(s+1);
	if (*s == '0' && ( *(s + 1) ==  'X'))
		 s += 2;
	while (sLen--)
	{
    	BOOL found = FALSE;
	    for (i = 0; i < 16; i++)
	    {
		 ToUpperCase(s);
	      if (*s == HexMapL[i])
	      {
	        if (!firsttime) result <<= 4;
	        result |= i;
	        found = TRUE;
	        break;
	      }
	    }
	    if (!found) break;
	    s++;
	    firsttime = FALSE;
	  }
	
	  return result;

}
*/


/****old versions  ****/

/* reverse:  reverse string s in place
void reverse(char s[])
{
    int c, i, j;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

// itoa:  convert n to characters in s
void itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  // record sign
        n = -n;          // make n positive
    i = 0;
    do {       // generate digits in reverse order
        s[i++] = n % 10 + '0';   // get next digit
    } while ((n /= 10) > 0);     // delete it
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}
*/
//convert 32bit number to a 32 ascii character array, pointed bey '*ascStr'
/*
uint16_t BinToAscii(uint32_t binNum,uint16_t * ascStr)
{
	
	uint16_t abit;
	abit=31;
	while(binNum!=0){
		ascStr[abit] = (binNum&1)+0x30;
		//ascStr++;
		abit--;
		binNum=binNum>>1;
		}

	while(abit!=0){
		 ascStr[abit]='0';
	// ascStr++;
		abit--;
	}
	ascStr[abit]='0';
	return OK;
}


*/


/************************************************************************************/
/*                                                                                  */
/* - FUNCTION        : ConvertHex2BCD                                               */                                      			
/*                                                                                  */
/* - DESCRIPTION     : This routine converts one byte hex to bcd presentaion        */
/*                     EX. the min 35 =  0x23 will be presented by the number 0x35  */
/* - INPUT           : one byte of data.     		  			    */
/* - CHANGES         :          					            */								
/* - CREATION      								    */									
/*   Yossi Ferlandes 24/02/2010							    */
/*                                                                                  */
/************************************************************************************/

uint8_t ConvertHex2BCD(uint8_t bData )
{
  if (bData == 0) return 0;
  return ( (bData /100 )*0x100 +
           ((bData /10 ) % 10 )*0x10 +
            bData % 10);
}


uint8_t ConvertBCD2Hex(uint8_t bData)
{
   if (bData < 0x9a) // y.f. add restiriction to max 99 2/3/2021
   return ( (bData /0x100 )*100 +
           ((bData /0x10 ) % 0x10 )*10 +
            bData % 0x10);
   return bData;
}

uint8_t calcCS(uint8_t *pbytes, uint16_t plen)
{
  uint8_t result=0;

  while(plen--)
  {
    result += *pbytes++;
  }

  return result;


}


/************************************************************************************/
/*                                                                                  */
/* - FUNCTION        : ConvertBinToAscii                                            */                                      			
/*                                                                                  */
/* - DESCRIPTION     : This routine converts one 64 bit to ascii string             */
/*                     EX. the min 35 =  0x23 will be presented by the number 0x35  */
/* - INPUT           : one byte of data.     		  			    */
/* - CHANGES         :          					            */								
/* - CREATION      								    */									
/*   Yossi Ferlandes 07/08/2013							    */
/*                                                                                  */
/************************************************************************************/

void  ConvertBinToAscii(long long binNum,unsigned char * ascStr)
{
	
	uint8_t abit,bInd=0;
	long long diverder;
        diverder = 1000000000000000;
        if (binNum == 0) 
        {
          ascStr[0] = 0x30;
          ascStr[1] = 0;
          return;
        }
        for (bInd = 0 ; bInd < 16 ; bInd++)
        {
          if (binNum /diverder == 0)
          {
            ascStr[15-bInd] = 0;
            diverder=  diverder/10;  
          }
          else
          {
            for (abit = 0 ; abit < 16-bInd ; abit++)
            {
              ascStr[abit] = ((binNum /diverder) %10)+0x30;
              diverder=  diverder/10;  
            }
            break;
          }
        }    
   
}



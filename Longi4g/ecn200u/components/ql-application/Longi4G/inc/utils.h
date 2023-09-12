
/* reverse:  reverse string s in place */
void reverse(uint8_t * s);

 
/* itoa:  convert n to characters in s */
void itoa(uint8_t n, uint8_t * s,int base);
/* itoa:  convert 32 bit n to characters in s */
void i32toa(uint32_t n, uint8_t * s,int base);
 

/* m_atoi: convert ascii number to int*/
uint16_t m_atoi(uint16_t * s,int base);

//convert 32bit number to a 32 ascii character array, pointed bey '*ascStr' 
uint16_t BinToAscii(uint32_t binNum,uint16_t * ascStr);
//convert lower case to upper case
void ToUpperCase(uint16_t * lowChar);

uint16_t check_regs_OK_or_ERROR(uint16_t regAdd,uint16_t cmpVal);
//error codes handling
uint16_t handle_error(uint16_t err_num);

uint32_t xto32i(uint16_t *s, uint16_t sLen);
uint16_t xto16i(uint16_t *s, uint16_t sLen);

uint8_t ConvertBCD2Hex(uint8_t bData);
uint8_t ConvertHex2BCD(uint8_t bData);

void  ConvertBinToAscii(long long binNum,unsigned char * ascStr);
/* takes a byte out of a uint32 : var - uint32,  ByteNum - byte to take out (0 - 3) */
#define BREAK_UINT32( var, ByteNum ) \
          (uint8_t)((uint32_t)(((var) >>((ByteNum) * 8)) & 0x00FF))

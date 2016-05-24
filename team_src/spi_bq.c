

#include "all.h"

extern user_data_struct data_temp;
extern user_ops_struct ops_temp;



/**
* @brief  Local functions          .                     
*/
unsigned char rdata;     // Receive data buffer
unsigned char calculate_crc(unsigned char* buffer, 
                            unsigned char buffer_lenght,
                            unsigned char crc);

/**
* @brief  constants             .                     
*/
//read write masks for the 'PL536 devices
const unsigned char spi_regs_rw_mask[] = 
{
  0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,
  0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x3,0x3,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x3,0x3,0x3,0x3,0x3,0x0,0x0,0x0,0x0,0x0,0x3,0x3,0x2,0x3,0x0,0x3,
  0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0
};

//CRC lookup table
const unsigned char CrcTable[] = {
  0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
  0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
  0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
  0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
  0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
  0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
  0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
  0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
  0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
  0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
  0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
  0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
  0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
  0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
  0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
  0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
  0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
  0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
  0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
  0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
  0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
  0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
  0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
  0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
  0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
  0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
  0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
  0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
  0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
  0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
  0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
  0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

  
short spi_write_reg(unsigned char dev_addr, 
                    unsigned char reg_addr, 
                    unsigned char data)
{
	unsigned char crc = 0;
	unsigned char package[4];
	char ret = VALID;
	short i=0;
	if (spi_regs_rw_mask[reg_addr] & WRITE_ACCESS)
	{
		package[0] = (dev_addr<<1) | 0x01/*Write*/;
		package[1] = reg_addr;
		package[2] = data;
		package[3] = calculate_crc(package, 3,crc);

		//Write 1 byte into the selected register
		SLAVEENCLEAR();
		//   Task_sleep(1);
		for (i=0; i<4; i++)
		{
			if(Send_SPI(&package[i]) == INVALID) //send data
			{
				ret = INVALID;
				i = 5;
			}

		}
		SLAVEENSET();
		DELAY_US(200);
	}

	return ret;

}

/**
* @brief Function Name: spi_read_reg.                                                  
* @brief Description  :   Read elem_num bytes from SPI reg of the attached 
* device, read data is returned in pData, returns number of received bytes 
* (including CRC as last pData byte if required) .
* @param parameters   :  device address, register address, num of bytes, 
* CRC enable/disable, receiver buffer .                                                   
* @return Value       :  number of bytes received                                                   
*/     
short spi_read_reg(unsigned char dev_addr,
                   unsigned char reg_addr,
                   short elem_num,
                   unsigned char discard_crc,//*True, False*/
                   unsigned char* pData)
{
	unsigned char package[3];
	unsigned char crc;
	unsigned char readchar;
	char ret = VALID;
	short i=0;
	if (spi_regs_rw_mask[reg_addr] & READ_ACCESS)
	{
		crc = 0;
		package[0] = (dev_addr<<1)/*Read*/;
		package[1] = reg_addr;
		package[2] = elem_num;
		crc = calculate_crc(package, 3,crc);


		//Write 1 byte into the selected register
		SLAVEENCLEAR();

		for (i=0; i<3; i++)
		{
			if(Send_SPI(&package[i]) == INVALID) //send data
			{
				ret = INVALID;
				i = 4;
			}
		}

		if (i == 3)
		{
			for (i=0; i<elem_num+1/*+1 to count CRC*/; i++)
			{
				readchar = 0;
				if(Send_SPI(&readchar) == INVALID) //send data
				{
					ret = INVALID;
					i = elem_num+2;
				}

				if (i==elem_num)
				{
					/*Read CRC*/
					if (discard_crc)
						package[0] = readchar;           // Read and discard CRC
					else
						*pData++ = readchar;             // R15 = 00|MSB
				}
				else
				{
					/*Read data*/
					*pData++ = readchar;               // R15 = 00|MSB
				}
			}
		}

		SLAVEENSET();
		DELAY_US(200);

		if(discard_crc)
		{
			i = dev_addr-1;
			unsigned char* data = (unsigned char*)(pData - (elem_num));
			if(calculate_crc(data,elem_num,crc) == package[0])
			{
				if(dev_addr > 0 && dev_addr <= NUMBER_OF_BQ_DEVICES)
				{
					data_temp.bq_pack.bq_devs[i].crc_error_count = 0;
					ops_temp.UserFlags.bit.BQ_error &= ~(1 << i); //clear error flag
				}

			}
			else
			{
				if(dev_addr > 0 && dev_addr <= NUMBER_OF_BQ_DEVICES)
				{
					data_temp.bq_pack.bq_devs[i].crc_error_count++;
					ops_temp.UserFlags.bit.BQ_error |= 1 << i; //set error flag
				}
				ret = INVALID;
			}
		}


	}

	return ret;
}

/**
* @brief  Local functions        .                     
*/

/**
* @brief Function Name: calculate_crc        .                                          
* @brief Description  : calculates CRC coming/going to the 'PL536 device.
* @param parameters   : buffer, buffer size                                                    
* @return Value       : crc                                                    
*/     
unsigned char calculate_crc(unsigned char* buffer, 
                            unsigned char buffer_length,
                            unsigned char crc)
{
  //unsigned char crc = 0;
  unsigned short temp = 0;
  unsigned short i;
  for ( i = 0; i < buffer_length; i++)
  {
    temp = crc ^ buffer[i];
    crc = CrcTable[temp];
  }
  return crc;
}




/*EOF*/

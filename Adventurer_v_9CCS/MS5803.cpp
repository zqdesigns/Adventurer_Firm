//
//  MS5803.cpp
//
//  This library is for reading and writing to the MS5803 pressure/temperature sensor.
//
//  Created by Victor Konshin on 4/10/13.
//
//
//  Copyright (c) 2013, Victor Konshin, info@ayerware.com
//  for the DIY Dive Computer project www.diydivecomputer.com
//  All rights reserved.

//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//  * Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.
//  * Neither the name of Ayerware Publishing nor the
//  names of its contributors may be used to endorse or promote products
//  derived from this software without specific prior written permission.

//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
//  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "MS5803.h"
#include "msp430.h"

// Sensor constants:
#define SENSOR_CMD_RESET      0x1E
#define SENSOR_CMD_ADC_READ   0x00
#define SENSOR_CMD_ADC_CONV   0x40
#define SENSOR_CMD_ADC_D1     0x00
#define SENSOR_CMD_ADC_D2     0x10
#define SENSOR_CMD_ADC_256    0x00
#define SENSOR_CMD_ADC_512    0x02
#define SENSOR_CMD_ADC_1024   0x04
#define SENSOR_CMD_ADC_2048   0x06
#define SENSOR_CMD_ADC_4096   0x08

#define SENSOR_I2C_ADDRESS    0x77 // If the CSB Pin (pin 3) is high, then the address is 0x76, if low, then it's 0x77

static uint16_t          sensorCoefficients[8];           // calibration coefficients
static uint32_t          D1                       = 0;    // Stores uncompensated pressure value
static uint32_t          D2                       = 0;    // Stores uncompensated temperature value
static int32_t           deltaTemp                = 0;    // These three variable are used for the conversion.
static int64_t           sensorOffset             = 0;
static int64_t           sensitivity              = 0;

// Constructor when using i2c.
MS5803::MS5803() {
  ref_pres = 10133;
} 

uint8_t MS5803::initializeSensor() {
    
    // Start the appropriate interface.
    /*UCB0CTLW0 = UCSWRST;
    UCB0CTLW0 |= UCMODE_3 + UCMST + UCSYNC + UCSSEL_2 + UCTR;      // I2C master mode Use SMCLK, keep SW reset
    UCB0BRW = 0x0050;                       // < 100 kHz
    UCB0CTLW0 &= ~UCSWRST;*/
       
    // resetting the sensor on startup is important
    resetSensor(); 
	
	// Read sensor coefficients - these will be used to convert sensor data into pressure and temp data
    for (int i = 0; i < 8; i++ ){
        sensorCoefficients[ i ] = ms5803ReadCoefficient( i );  // read coefficients
        
    }
    
    uint8_t p_crc = sensorCoefficients[ 7 ];
    uint8_t n_crc = ms5803CRC4( sensorCoefficients ); // calculate the CRC
    
    // If the calculated CRC does not match the returned CRC, then there is a data integrity issue.
    // Check the connections for bad solder joints or "flakey" cables. 
    // If this issue persists, you may have a bad sensor.
    if ( p_crc != n_crc ) {
        return 0;
    }
    return 1;
}

void MS5803::readSensor() {

	// If power or speed are important, you can change the ADC resolution to a lower value.
	// Currently set to SENSOR_CMD_ADC_4096 - set to a lower defined value for lower resolution.
    D1 = ms5803CmdAdc( SENSOR_CMD_ADC_D1 + SENSOR_CMD_ADC_4096 );    // read uncompensated pressure
    D2 = ms5803CmdAdc( SENSOR_CMD_ADC_D2 + SENSOR_CMD_ADC_4096 );    // read uncompensated temperature
    
    // calculate 1st order pressure and temperature correction factors (MS5803 1st order algorithm). 
    deltaTemp = D2 - ((uint32_t)sensorCoefficients[5] << 8);
    sensorOffset = ((uint64_t)sensorCoefficients[2] << 17) + (int32_t)((deltaTemp * (uint64_t)sensorCoefficients[4]) >> 6);
    sensitivity = ((uint64_t)sensorCoefficients[1] << 16) + (int32_t)((deltaTemp * (uint64_t)sensorCoefficients[3]) >> 7);
    
    // calculate 2nd order pressure and temperature (MS5803 2st order algorithm)
    press = ((((uint64_t)(D1 * sensitivity) >> 21) - sensorOffset) >> 15)/10;
    temp = (2000 + (int32_t)((deltaTemp * (uint64_t)sensorCoefficients[6] ) >> 23));
    alt = ((ref_pres - press) * (160000 + (temp * 1600) / 273)) / (ref_pres + press);
    temp = temp/10;
    press = (press*3)/4;
}

void MS5803::adjustAltitude(int32_t adj_ref) {

	ref_pres = adj_ref;
	press = ((((uint64_t)(D1 * sensitivity) >> 21) - sensorOffset) >> 15)/10;
	temp = (2000 + ((deltaTemp * sensorCoefficients[6] ) >> 23));
	alt = ((ref_pres - press) * (160000 + (temp * 1600) / 273)) / (ref_pres + press);
}

// Sends a power on reset command to the sensor.
// Should be done at powerup and maybe on a periodic basis (needs to confirm with testing).
void MS5803::resetSensor() {

    while (UCB0CTLW0 & UCTXSTP);
    UCB0I2CSA = SENSOR_I2C_ADDRESS;                         
    UCB0CTLW0 |= UCTXSTT ;              // I2C start
    while(UCB0CTLW0 & UCTXSTT);        // wait till UCTXSTT bit is reset
    UCB0TXBUF = SENSOR_CMD_RESET; 
    while (~UCB0IFG&UCTXIFG0);         // Wait for TX buffer to empty
    UCB0CTLW0 |= UCTXSTP;
    __delay_cycles(80000);
}

// These sensors have coefficient values stored in ROM that are used to convert the raw temp/pressure data into degrees and mbars.
// This method reads the coefficient at the index value passed.  Valid values are 0-7. See datasheet for more info.
uint16_t MS5803::ms5803ReadCoefficient(uint8_t index) {
    uint16_t result = 0;   // result to return
    
    while (UCB0CTLW0 & UCTXSTP);
    UCB0I2CSA = SENSOR_I2C_ADDRESS;                         
    UCB0CTLW0 |= UCTXSTT ;              // I2C start
    while(UCB0CTLW0 & UCTXSTT);        // wait till UCTXSTT bit is reset
    UCB0TXBUF = 0xA0 + ( index * 2 ); 
    while (~UCB0IFG&UCTXIFG0);         // Wait for TX buffer to empty
    UCB0CTLW0 |= UCTXSTP;
    
    while (UCB0CTLW0 & UCTXSTP);
    UCB0CTLW0 &= ~UCTR;
    UCB0CTLW0 |= UCTXSTT ;              // I2C start
    while (~UCB0IFG&UCRXIFG0);
    result = UCB0RXBUF;
    while (~UCB0IFG&UCRXIFG0);
    result = UCB0RXBUF;
    while (~UCB0IFG&UCRXIFG0);
    result = result << 8;
    result += UCB0RXBUF;
    UCB0CTLW0 |= UCTXSTP;
    __delay_cycles(80000);
    UCB0CTLW0 |= UCTR;
    
    return( result );
}

// Coefficient at index 7 is a four bit CRC value for verifying the validity of the other coefficients.
// The value returned by this method should match the coefficient at index 7.
// If not there is something works with the sensor or the connection.
uint8_t MS5803::ms5803CRC4(uint16_t n_prom[]) {

    int16_t cnt;
    uint16_t n_rem;
    uint16_t crc_read;
    uint8_t  n_bit;
    
    n_rem = 0x00;
    crc_read = sensorCoefficients[7];
    sensorCoefficients[7] = ( 0xFF00 & ( sensorCoefficients[7] ) );
    
    for (cnt = 0; cnt < 16; cnt++)
    { // choose LSB or MSB
        if ( cnt%2 == 1 ) n_rem ^= (unsigned short) ( ( sensorCoefficients[cnt>>1] ) & 0x00FF );
        else n_rem ^= (unsigned short) ( sensorCoefficients[cnt>>1] >> 8 );
        for ( n_bit = 8; n_bit > 0; n_bit-- )
        {
            if ( n_rem & ( 0x8000 ) )
            {
                n_rem = ( n_rem << 1 ) ^ 0x3000;
            }
            else {
                n_rem = ( n_rem << 1 );
            }
        }
    }
    
    n_rem = ( 0x000F & ( n_rem >> 12 ) );// // final 4-bit reminder is CRC code
    sensorCoefficients[7] = crc_read; // restore the crc_read to its original place
    
    return ( n_rem ^ 0x00 ); // The calculated CRC should match what the device initally returned.
}

// Use this method to send commands to the sensor.  Pretty much just used to read the pressure and temp data.
uint32_t MS5803::ms5803CmdAdc(uint8_t cmd) {

    uint32_t result = 0;
        
    while (UCB0CTLW0 & UCTXSTP);
    UCB0I2CSA = SENSOR_I2C_ADDRESS;                         
    UCB0CTLW0 |= UCTXSTT ;              // I2C start
    while(UCB0CTLW0 & UCTXSTT);        // wait till UCTXSTT bit is reset
    UCB0TXBUF = SENSOR_CMD_ADC_CONV + cmd; 
    while (~UCB0IFG&UCTXIFG0);         // Wait for TX buffer to empty
    UCB0CTLW0 |= UCTXSTP;
      
    switch ( cmd & 0x0f )
    {
        case SENSOR_CMD_ADC_256 :
            __delay_cycles(8000);
            break;
        case SENSOR_CMD_ADC_512 :
            __delay_cycles(24000);
            break;
        case SENSOR_CMD_ADC_1024:
            __delay_cycles(32000);
            break;
        case SENSOR_CMD_ADC_2048:
            __delay_cycles(48000);
            break;
        case SENSOR_CMD_ADC_4096:
            __delay_cycles(80000);
            break;
    }
    
    __delay_cycles(24000);
    while (UCB0CTLW0 & UCTXSTP);
    UCB0I2CSA = SENSOR_I2C_ADDRESS;                         
    UCB0CTLW0 |= UCTXSTT ;              // I2C start
    while(UCB0CTLW0 & UCTXSTT);        // wait till UCTXSTT bit is reset
    UCB0TXBUF = SENSOR_CMD_ADC_READ; 
    while (~UCB0IFG&UCTXIFG0);         // Wait for TX buffer to empty
    UCB0CTLW0 |= UCTXSTP;
           
    // Always read back three bytes (24 bits)
    while (UCB0CTLW0 & UCTXSTP);
    UCB0CTLW0 &= ~UCTR;
    UCB0CTLW0 |= UCTXSTT ;              // I2C start
    while (~UCB0IFG&UCRXIFG0);
    result = UCB0RXBUF;
    while (~UCB0IFG&UCRXIFG0);
    result = UCB0RXBUF;
    while (~UCB0IFG&UCRXIFG0);
    result = result << 8;
    result += UCB0RXBUF;
    while (~UCB0IFG&UCRXIFG0);
    result = result << 8;
    result += UCB0RXBUF;
    UCB0CTLW0 |= UCTXSTP;
    __delay_cycles(80000);
    UCB0CTLW0 |= UCTR;
    
    return result;
}

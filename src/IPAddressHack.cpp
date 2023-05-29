/*
 *---------------------------------------------------------------------
 * 2023 Dave Nardella
 * This file il part of Ethernet_SP2, a library which allows the use of 
 * a WXXXX Wiznet chip/module on the second SPI port.
 *---------------------------------------------------------------------
 */
#include "IPAddressHack.h"

uint8_t bytes[4];
uint8_t * raw_address(IPAddress Address)
{
    for(int c = 0; c < 4; c++)
        bytes[c] = Address[c];
    return (uint8_t*)&bytes[0];
}


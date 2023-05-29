/*
 *---------------------------------------------------------------------
 * 2023 Dave Nardella
 * This file il part of Ethernet_SP2, a library which allows the use of 
 * a WXXXX Wiznet chip/module on the second SPI port.
 *---------------------------------------------------------------------
 */
#ifndef ipaddresshack_h
#define ipaddresshack_h
#include <Arduino.h>

uint8_t* raw_address(IPAddress Address);


#endif
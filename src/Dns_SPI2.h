/*
 *---------------------------------------------------------------------
 * 2023 Dave Nardella
 * This file il part of Ethernet_SP2, a library which allows the use of 
 * a WXXXX Wiznet chip/module on the second SPI port.
 * The library derived from original Ethernet library 2.0.2 
 * I leaved below all the original copyright statements.
 *---------------------------------------------------------------------
*/
// Arduino DNS client for WIZnet W5100-based Ethernet shield
// (c) Copyright 2009-2010 MCQN Ltd.
// Released under Apache License, version 2.0

#ifndef DNSClient_spi2_h
#define DNSClient_spi2_h

#include "Ethernet_SPI2.h"
#include "IPAddressHack.h"

class DNSClient_SPI2
{
public:
	void begin(const IPAddress& aDNSServer);

	/** Convert a numeric IP address string into a four-byte IP address.
	    @param aIPAddrString IP address to convert
	    @param aResult IPAddress structure to store the returned IP address
	    @result 1 if aIPAddrString was successfully converted to an IP address,
	            else error code
	*/
	int inet_aton(const char *aIPAddrString, IPAddress& aResult);

	/** Resolve the given hostname to an IP address.
	    @param aHostname Name to be resolved
	    @param aResult IPAddress structure to store the returned IP address
	    @result 1 if aIPAddrString was successfully converted to an IP address,
	            else error code
	*/
	int getHostByName(const char* aHostname, IPAddress& aResult, uint16_t timeout=5000);

protected:
	uint16_t BuildRequest(const char* aName);
	uint16_t ProcessResponse(uint16_t aTimeout, IPAddress& aAddress);

	IPAddress iDNSServer;
	uint16_t iRequestId;
	EthernetUDP_SPI2 iUdp;
};

#endif

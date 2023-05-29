/*
 *---------------------------------------------------------------------
 * 2023 Dave Nardella
 * This file il part of Ethernet_SP2, a library which allows the use of 
 * a WXXXX Wiznet chip/module on the second SPI port.
 * The library derived from original Ethernet library 2.0.2 
 * I leaved below all the original copyright statements.
 *---------------------------------------------------------------------
 *
 * Copyright 2018 Paul Stoffregen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <Arduino.h>
#include "Ethernet_SPI2.h"
#include "utility/w5100_SPI2.h"
#include "Dhcp_SPI2.h"

IPAddress EthernetClass_SPI2::_dnsServerAddress;
DhcpClass_SPI2* EthernetClass_SPI2::_dhcp = NULL;

int EthernetClass_SPI2::begin(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout)
{
	static DhcpClass_SPI2 s_dhcp;
	_dhcp = &s_dhcp;

	// Initialise the basic info
	if (W5100_SPI2.init() == 0) return 0;
	SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100_SPI2.setMACAddress(mac);
//	W5100_SPI2.setIPAddress(IPAddress(0,0,0,0).raw_address());
	W5100_SPI2.setIPAddress(raw_address(IPAddress(0,0,0,0)));
	SPI1.endTransaction();

	// Now try to get our config info from a DHCP server
	int ret = _dhcp->beginWithDHCP(mac, timeout, responseTimeout);
	if (ret == 1) {
		// We've successfully found a DHCP server and got our configuration
		// info, so set things accordingly
		SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);
/*
		W5100_SPI2.setIPAddress(_dhcp->getLocalIp().raw_address());
		W5100_SPI2.setGatewayIp(_dhcp->getGatewayIp().raw_address());
		W5100_SPI2.setSubnetMask(_dhcp->getSubnetMask().raw_address());
*/
		W5100_SPI2.setIPAddress(raw_address(_dhcp->getLocalIp()));
		W5100_SPI2.setGatewayIp(raw_address(_dhcp->getGatewayIp()));
		W5100_SPI2.setSubnetMask(raw_address(_dhcp->getSubnetMask()));

		SPI1.endTransaction();
		_dnsServerAddress = _dhcp->getDnsServerIp();
		socketPortRand(micros());
	}
	return ret;
}

void EthernetClass_SPI2::begin(uint8_t *mac, IPAddress ip)
{
	// Assume the DNS server will be the machine on the same network as the local IP
	// but with last octet being '1'
	IPAddress dns = ip;
	dns[3] = 1;
	begin(mac, ip, dns);
}

void EthernetClass_SPI2::begin(uint8_t *mac, IPAddress ip, IPAddress dns)
{
	// Assume the gateway will be the machine on the same network as the local IP
	// but with last octet being '1'
	IPAddress gateway = ip;
	gateway[3] = 1;
	begin(mac, ip, dns, gateway);
}

void EthernetClass_SPI2::begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway)
{
	IPAddress subnet(255, 255, 255, 0);
	begin(mac, ip, dns, gateway, subnet);
}

void EthernetClass_SPI2::begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet)
{
	if (W5100_SPI2.init() == 0) return;
	SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100_SPI2.setMACAddress(mac);
/*
	W5100_SPI2.setIPAddress(ip.raw_address());
	W5100_SPI2.setGatewayIp(gateway.raw_address());
	W5100_SPI2.setSubnetMask(subnet.raw_address());
*/	
	W5100_SPI2.setIPAddress(raw_address(ip));
	W5100_SPI2.setGatewayIp(raw_address(gateway));
	W5100_SPI2.setSubnetMask(raw_address(subnet));
	SPI1.endTransaction();
	_dnsServerAddress = dns;
}

void EthernetClass_SPI2::init(uint8_t sspin)
{
	W5100_SPI2.setSS(sspin);
}

EthernetSPI2LinkStatus EthernetClass_SPI2::linkStatus()
{
	switch (W5100_SPI2.getLinkStatus()) {
		case UNKNOWN:  return Unknown_SPI2;
		case LINK_ON:  return LinkON_SPI2;
		case LINK_OFF: return LinkOFF_SPI2;
		default:       return Unknown_SPI2;
	}
}

EthernetSPI2HardwareStatus EthernetClass_SPI2::hardwareStatus()
{
	switch (W5100_SPI2.getChip()) {
		case 51: return EthernetW5100_SPI2;
		case 52: return EthernetW5200_SPI2;
		case 55: return EthernetW5500_SPI2;
		default: return EthernetNoHardware_SPI2;
	}
}

int EthernetClass_SPI2::maintain()
{
	int rc = DHCP_CHECK_NONE;
	if (_dhcp != NULL) {
		// we have a pointer to dhcp, use it
		rc = _dhcp->checkLease();
		switch (rc) {
		case DHCP_CHECK_NONE:
			//nothing done
			break;
		case DHCP_CHECK_RENEW_OK:
		case DHCP_CHECK_REBIND_OK:
			//we might have got a new IP.
			SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);

/*
			W5100_SPI2.setIPAddress(_dhcp->getLocalIp().raw_address());
			W5100_SPI2.setGatewayIp(_dhcp->getGatewayIp().raw_address());
			W5100_SPI2.setSubnetMask(_dhcp->getSubnetMask().raw_address());
*/			
			W5100_SPI2.setIPAddress(raw_address(_dhcp->getLocalIp()));
			W5100_SPI2.setGatewayIp(raw_address(_dhcp->getGatewayIp()));
			W5100_SPI2.setSubnetMask(raw_address(_dhcp->getSubnetMask()));
			SPI1.endTransaction();
			_dnsServerAddress = _dhcp->getDnsServerIp();
			break;
		default:
			//this is actually an error, it will retry though
			break;
		}
	}
	return rc;
}


void EthernetClass_SPI2::MACAddress(uint8_t *mac_address)
{
	SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100_SPI2.getMACAddress(mac_address);
	SPI1.endTransaction();
}

IPAddress EthernetClass_SPI2::localIP()
{
	IPAddress ret;
	uint8_t local[4];
	SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);
//	W5100_SPI2.getIPAddress(ret.raw_address());
	W5100_SPI2.getIPAddress((uint8_t*)&local);
	SPI1.endTransaction();
	for (int c = 0; c < 4; c++)
		ret[c] = local[c];
	return ret;
}

IPAddress EthernetClass_SPI2::subnetMask()
{
	IPAddress ret;
	uint8_t local[4];
	SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);
//	W5100_SPI2.getSubnetMask(ret.raw_address());
	W5100_SPI2.getSubnetMask((uint8_t*)&local);
	SPI1.endTransaction();
	for (int c = 0; c < 4; c++)
		ret[c] = local[c];
	return ret;
}

IPAddress EthernetClass_SPI2::gatewayIP()
{
	IPAddress ret;
	uint8_t local[4];
	SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);
//	W5100_SPI2.getGatewayIp(ret.raw_address());
	W5100_SPI2.getGatewayIp((uint8_t*)&local);
	SPI1.endTransaction();
	for (int c = 0; c < 4; c++)
		ret[c] = local[c];
	return ret;
}

void EthernetClass_SPI2::setMACAddress(const uint8_t *mac_address)
{
	SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100_SPI2.setMACAddress(mac_address);
	SPI1.endTransaction();
}

void EthernetClass_SPI2::setLocalIP(const IPAddress local_ip)
{
	SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress ip = local_ip;
//	W5100_SPI2.setIPAddress(ip.raw_address());
	W5100_SPI2.setIPAddress(raw_address(ip));
	SPI1.endTransaction();
}

void EthernetClass_SPI2::setSubnetMask(const IPAddress subnet)
{
	SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress ip = subnet;
//	W5100_SPI2.setSubnetMask(ip.raw_address());
	W5100_SPI2.setSubnetMask(raw_address(ip));
	SPI1.endTransaction();
}

void EthernetClass_SPI2::setGatewayIP(const IPAddress gateway)
{
	SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress ip = gateway;
//	W5100_SPI2.setGatewayIp(ip.raw_address());
	W5100_SPI2.setGatewayIp(raw_address(ip));
	SPI1.endTransaction();
}

void EthernetClass_SPI2::setRetransmissionTimeout(uint16_t milliseconds)
{
	if (milliseconds > 6553) milliseconds = 6553;
	SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100_SPI2.setRetransmissionTime(milliseconds * 10);
	SPI1.endTransaction();
}

void EthernetClass_SPI2::setRetransmissionCount(uint8_t num)
{
	SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100_SPI2.setRetransmissionCount(num);
	SPI1.endTransaction();
}










EthernetClass_SPI2 Ethernet_SPI2;

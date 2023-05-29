/* 
 *---------------------------------------------------------------------
 * 2023 Dave Nardella
 * This file il part of Ethernet_SP2, a library which allows the use of 
 * a WXXXX Wiznet chip/module on the second SPI port.
 * The library derived from original Ethernet library 2.0.2 
 * I leaved below all the original copyright statements.
 *---------------------------------------------------------------------
 *
 *Copyright 2018 Paul Stoffregen
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
#include "Dns_SPI2.h"
#include "utility/w5100_SPI2.h"

int EthernetClient_SPI2::connect(const char * host, uint16_t port)
{
	DNSClient_SPI2 dns; // Look up the host first
	IPAddress remote_addr;

	if (_sockindex < MAX_SOCK_NUM) {
		if (Ethernet_SPI2.socketStatus(_sockindex) != SnSR::CLOSED) {
			Ethernet_SPI2.socketDisconnect(_sockindex); // TODO: should we call stop()?
		}
		_sockindex = MAX_SOCK_NUM;
	}
	dns.begin(Ethernet_SPI2.dnsServerIP());
	if (!dns.getHostByName(host, remote_addr)) return 0; // TODO: use _timeout
	return connect(remote_addr, port);
}

int EthernetClient_SPI2::connect(IPAddress ip, uint16_t port)
{
	if (_sockindex < MAX_SOCK_NUM) {
		if (Ethernet_SPI2.socketStatus(_sockindex) != SnSR::CLOSED) {
			Ethernet_SPI2.socketDisconnect(_sockindex); // TODO: should we call stop()?
		}
		_sockindex = MAX_SOCK_NUM;
	}
#if defined(ESP8266) || defined(ESP32)
	if (ip == IPAddress((uint32_t)0) || ip == IPAddress(0xFFFFFFFFul)) return 0;
#else
	if (ip == IPAddress(0ul) || ip == IPAddress(0xFFFFFFFFul)) return 0;
#endif
	_sockindex = Ethernet_SPI2.socketBegin(SnMR::TCP, 0);
	if (_sockindex >= MAX_SOCK_NUM) return 0;
	Ethernet_SPI2.socketConnect(_sockindex, rawIPAddress(ip), port);
	uint32_t start = millis();
	while (1) {
		uint8_t stat = Ethernet_SPI2.socketStatus(_sockindex);
		if (stat == SnSR::ESTABLISHED) return 1;
		if (stat == SnSR::CLOSE_WAIT) return 1;
		if (stat == SnSR::CLOSED) return 0;
		if (millis() - start > _timeout) break;
		delay(1);
	}
	Ethernet_SPI2.socketClose(_sockindex);
	_sockindex = MAX_SOCK_NUM;
	return 0;
}

int EthernetClient_SPI2::availableForWrite(void)
{
	if (_sockindex >= MAX_SOCK_NUM) return 0;
	return Ethernet_SPI2.socketSendAvailable(_sockindex);
}

size_t EthernetClient_SPI2::write(uint8_t b)
{
	return write(&b, 1);
}

size_t EthernetClient_SPI2::write(const uint8_t *buf, size_t size)
{
	if (_sockindex >= MAX_SOCK_NUM) return 0;
	if (Ethernet_SPI2.socketSend(_sockindex, buf, size)) return size;
	setWriteError();
	return 0;
}

int EthernetClient_SPI2::available()
{
	if (_sockindex >= MAX_SOCK_NUM) return 0;
	return Ethernet_SPI2.socketRecvAvailable(_sockindex);
	// TODO: do the WIZnet chips automatically retransmit TCP ACK
	// packets if they are lost by the network?  Someday this should
	// be checked by a man-in-the-middle test which discards certain
	// packets.  If ACKs aren't resent, we would need to check for
	// returning 0 here and after a timeout do another Sock_RECV
	// command to cause the WIZnet chip to resend the ACK packet.
}

int EthernetClient_SPI2::read(uint8_t *buf, size_t size)
{
	if (_sockindex >= MAX_SOCK_NUM) return 0;
	return Ethernet_SPI2.socketRecv(_sockindex, buf, size);
}

int EthernetClient_SPI2::peek()
{
	if (_sockindex >= MAX_SOCK_NUM) return -1;
	if (!available()) return -1;
	return Ethernet_SPI2.socketPeek(_sockindex);
}

int EthernetClient_SPI2::read()
{
	uint8_t b;
	if (Ethernet_SPI2.socketRecv(_sockindex, &b, 1) > 0) return b;
	return -1;
}

void EthernetClient_SPI2::flush()
{
	while (_sockindex < MAX_SOCK_NUM) {
		uint8_t stat = Ethernet_SPI2.socketStatus(_sockindex);
		if (stat != SnSR::ESTABLISHED && stat != SnSR::CLOSE_WAIT) return;
		if (Ethernet_SPI2.socketSendAvailable(_sockindex) >= W5100_SPI2.SSIZE) return;
	}
}

void EthernetClient_SPI2::stop()
{
	if (_sockindex >= MAX_SOCK_NUM) return;

	// attempt to close the connection gracefully (send a FIN to other side)
	Ethernet_SPI2.socketDisconnect(_sockindex);
	unsigned long start = millis();

	// wait up to a second for the connection to close
	do {
		if (Ethernet_SPI2.socketStatus(_sockindex) == SnSR::CLOSED) {
			_sockindex = MAX_SOCK_NUM;
			return; // exit the loop
		}
		delay(1);
	} while (millis() - start < _timeout);

	// if it hasn't closed, close it forcefully
	Ethernet_SPI2.socketClose(_sockindex);
	_sockindex = MAX_SOCK_NUM;
}

uint8_t EthernetClient_SPI2::connected()
{
	if (_sockindex >= MAX_SOCK_NUM) return 0;

	uint8_t s = Ethernet_SPI2.socketStatus(_sockindex);
	return !(s == SnSR::LISTEN || s == SnSR::CLOSED || s == SnSR::FIN_WAIT ||
		(s == SnSR::CLOSE_WAIT && !available()));
}

uint8_t EthernetClient_SPI2::status()
{
	if (_sockindex >= MAX_SOCK_NUM) return SnSR::CLOSED;
	return Ethernet_SPI2.socketStatus(_sockindex);
}

// the next function allows us to use the client returned by
// EthernetServer::available() as the condition in an if-statement.
bool EthernetClient_SPI2::operator==(const EthernetClient_SPI2& rhs)
{
	if (_sockindex != rhs._sockindex) return false;
	if (_sockindex >= MAX_SOCK_NUM) return false;
	if (rhs._sockindex >= MAX_SOCK_NUM) return false;
	return true;
}

// https://github.com/per1234/EthernetMod
// from: https://github.com/ntruchsess/Arduino-1/commit/937bce1a0bb2567f6d03b15df79525569377dabd
uint16_t EthernetClient_SPI2::localPort()
{
	if (_sockindex >= MAX_SOCK_NUM) return 0;
	uint16_t port;
	SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);
	port = W5100_SPI2.readSnPORT(_sockindex);
	SPI1.endTransaction();
	return port;
}

// https://github.com/per1234/EthernetMod
// returns the remote IP address: https://forum.arduino.cc/index.php?topic=82416.0
IPAddress EthernetClient_SPI2::remoteIP()
{
	if (_sockindex >= MAX_SOCK_NUM) return IPAddress((uint32_t)0);
	uint8_t remoteIParray[4];
	SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100_SPI2.readSnDIPR(_sockindex, remoteIParray);
	SPI1.endTransaction();
	return IPAddress(remoteIParray);
}

// https://github.com/per1234/EthernetMod
// from: https://github.com/ntruchsess/Arduino-1/commit/ca37de4ba4ecbdb941f14ac1fe7dd40f3008af75
uint16_t EthernetClient_SPI2::remotePort()
{
	if (_sockindex >= MAX_SOCK_NUM) return 0;
	uint16_t port;
	SPI1.beginTransaction(SPI_ETHERNET_SETTINGS);
	port = W5100_SPI2.readSnDPORT(_sockindex);
	SPI1.endTransaction();
	return port;
}

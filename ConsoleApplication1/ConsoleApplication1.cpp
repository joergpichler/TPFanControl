// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <sstream>
#include <Windows.h>
#include "TVicPort.h"

// Registers of the embedded controller
constexpr unsigned int EC_DATAPORT = 0x1600;	// EC data io-port 0x62
constexpr unsigned int EC_CTRLPORT = 0x1604;	// EC control io-port 0x66


// Embedded controller status register bits
constexpr unsigned int EC_STAT_OBF = 0x01; // Output buffer full 
constexpr unsigned int EC_STAT_IBF = 0x02; // Input buffer full 
constexpr unsigned int  EC_STAT_CMD = 0x08; // Last write was a command write (0=data) 


// Embedded controller commands
// (write to EC_CTRLPORT to initiate read/write operation)
constexpr unsigned char  EC_CTRLPORT_READ = 0x80;
constexpr unsigned char  EC_CTRLPORT_WRITE = 0x81;
constexpr unsigned char  EC_CTRLPORT_QUERY = 0x84;


int verbosity = 1;	// verbosity for the logbuf (0= nothing)
char lasterrorstring[256] = "";


//-------------------------------------------------------------------------
// read a byte from the embedded controller (EC) via port io 
//-------------------------------------------------------------------------
int ReadByteFromEC(int offset, char *pdata)
{
	char data = -1;
	char data2 = -1;
	int iOK = false;
	int iTimeout = 100;
	int iTimeoutBuf = 1000;
	int	iTime = 0;
	int iTick = 10;

	for (iTime = 0; iTime < iTimeoutBuf; iTime += iTick) {	// wait for full buffers to clear
		data = (char)ReadPort(EC_CTRLPORT) & 0xff;			// or timeout iTimeoutBuf = 1000
		if (!(data & (EC_STAT_IBF | EC_STAT_OBF))) break;
		::Sleep(iTick);
	}

	if (data & EC_STAT_OBF) data2 = (char)ReadPort(EC_DATAPORT); //clear OBF if full

	WritePort(EC_CTRLPORT, EC_CTRLPORT_READ);			// tell 'em we want to "READ"

	for (iTime = 0; iTime < iTimeout; iTime += iTick) {	// wait for IBF and OBF to clear
		data = (char)ReadPort(EC_CTRLPORT) & 0xff;
		if (!(data & (EC_STAT_IBF | EC_STAT_OBF))) {
			iOK = true;
			break;
		}
		::Sleep(iTick);
	} // try again after a moment

	if (!iOK) return 0;
	iOK = false;

	WritePort(EC_DATAPORT, offset);						// tell 'em where we want to read from

	if (!(data & EC_STAT_OBF)) {
		for (iTime = 0; iTime < iTimeout; iTime += iTick) { // wait for OBF 
			data = (char)ReadPort(EC_CTRLPORT) & 0xff;
			if ((data & EC_STAT_OBF)) {
				iOK = true;
				break;
			}
			::Sleep(iTick);
		}							// try again after a moment
		if (!iOK) return 0;
	}

	*pdata = ReadPort(EC_DATAPORT);

	if (verbosity > 0) {
		std::stringstream sstream;
		sstream << "readec: offset= " << std::hex << offset << ", data= " << *pdata << "\n";
		// todo output to console
		//std::cout << "readec: offset= " << std::hex << offset << ", data= " << *pdata << "\n";
		OutputDebugString(sstream.str().c_str());
	}

	return 1;
}


int main()
{
	bool ok = false;
	for (int i = 0; i < 180; i++)
	{
		if (OpenTVicPort())
		{
			ok = true;
			break;
		}
		::Sleep(1000);
	}
	if (ok)
	{
		/*TestHardAccess();
		SetHardAccess(true);
		TestHardAccess();*/
	}
	else {
		return -1;
	}

	char data;
	ReadByteFromEC(0x2f, &data);
	std::cout << std::hex << (int)data;

	CloseTVicPort();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

#include "NET_CanInterface.hpp"

#ifdef WIN32
	#include <windows.h>
#else
	#include "fcntl.h>
	#include <net/if.h>
	#include <stdio.h>
	#include <cstring>
	#include <sys/ioctl.h>
	#include <linux/can.h>
	#include <linux/can/raw.h>
	#include <linux/can/error.h>
#endif


NET_CanInterface::NET_CanInterface (char* port)
{
    open_port (port);
}

NET_CanInterface::~NET_CanInterface()
{
    close_port();
}

#ifdef WIN32
	int NET_CanInterface::open_port(const char *port)
	{
		soc =  CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		// Do some basic settings
		DCB serialParams = { 0 };
		serialParams.DCBlength = sizeof(serialParams);

		GetCommState(soc, &serialParams);
		serialParams.BaudRate = baudRate;
		serialParams.ByteSize = 8;
		//serialParams.StopBits = stopBits;
		// serialParams.Parity = parity;
		SetCommState(soc, &serialParams);

		// Set timeouts
		COMMTIMEOUTS timeout = { 0 };
		timeout.ReadIntervalTimeout = 50;
		timeout.ReadTotalTimeoutConstant = 50;
		timeout.ReadTotalTimeoutMultiplier = 50;
		timeout.WriteTotalTimeoutConstant = 50;
		timeout.WriteTotalTimeoutMultiplier = 10;

		SetCommTimeouts(soc, &timeout);
		return 1;
	}

	int NET_CanInterface::send_frame(struct can_frame *frame)
	{
		LPDWORD written = NULL;
		WriteFile(soc, frame, sizeof(struct can_frame), written, NULL);
		return (int)*written;
	}

	void NET_CanInterface::read_frame(struct can_frame * frame_rd, int *recvBytes)
	{
		ReadFile(soc, frame_rd, sizeof(struct can_frame), (LPDWORD)recvBytes, NULL);
	}

	int NET_CanInterface::close_port()
	{
		CloseHandle(soc);
		return 1;
	}

#elif __linux__
	int NET_CanInterface::open_port(const char *port)
	{
		struct ifreq ifr;
		struct sockaddr_can addr;

		// open socket
		soc = socket(PF_CAN, SOCK_RAW, CAN_RAW);

		//Set error mask for error messages as can frames
		can_err_mask_t err_mask = ( CAN_ERR_TX_TIMEOUT | CAN_ERR_BUSOFF );
		setsockopt(soc, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask));

		//Invalid socket
		if(soc < 0)
		{
			return (-1);
		}

		addr.can_family = AF_CAN;
		strcpy(ifr.ifr_name, port);
		if (ioctl(soc, SIOCGIFINDEX, &ifr) < 0)
		{
			return (-1);
		}
		addr.can_ifindex = ifr.ifr_ifindex;
		fcntl(soc, F_SETFL, O_NONBLOCK);

		//Check if can bind socket to interface
		if (bind(soc, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		{
			return (-1);
		}
		return 0;
	}
	int NET_CanInterface::send_frame(struct can_frame *frame)
	{
		int retval;
		retval = write(soc, frame, sizeof(struct can_frame));
		if (retval != sizeof(struct can_frame))
		{
			return (-1);
		}
		else
		{
			return (0);
		}
	}

	void NET_CanInterface::read_frame(struct can_frame * frame_rd, int *recvBytes)
	{
		//struct can_frame frame_rd;
		//int recvbytes = 0;
		recvBytes = 0;
		read_can_port = 1;
		while(read_can_port)
		{
			struct timeval timeout = {1, 0};
			fd_set readSet;
			FD_ZERO(&readSet);
			FD_SET(soc, &readSet);
			if (select((soc + 1), &readSet, NULL, NULL, &timeout) >= 0)
			{
				if (!read_can_port)
				{
					break;
				}
				if (FD_ISSET(soc, &readSet))
				{
					recvbytes = read(soc, frame_rd, sizeof(struct can_frame));
					if(recvbytes)
					{
						//printf("id = %X dlc = %X, data = %s\n", frame_rd->can_id, frame_rd->can_dlc, frame_rd->data);
						break;
					}
				}
			}
		}
	}

	int NET_CanInterface::close_port()
	{
		close(soc);
		return 0;
	}
#else EMBEDDED
	int NET_CanInterface::open_port(const char *port)
	{
		
	}
	int NET_CanInterface::send_frame(struct can_frame *frame)
	{
		if (io_message)
		{
			//Intel 82527
			#define CAN_BASE 0xA000
			#define MESSAGE7_OFFSET 0x50
			#define CONTROL0_OFFSET 0x00
			#define CONTROL1_OFFSET 0x01
			#define MESS_CONF 0x06

			RmtPnd = 1; //RemoteFramePendingFlag 1 if requested but not answered yet 0 if no waiting remote request 
			NewDat = 2; //NewDataFlag 1 if changed 0 if not changed

						//MessageConfigurationReg
						//length(4b)+direction(1b)+extended id(1b)+reserved(2b)
			MessConf = length << 4 | 0x08;

			/*
			// load io poll response into can chip object #9
			for (i = 0; i < length; i++)  					// load CAN data
			{
			pokeb(CAN_BASE, (0x57 + i), response[i]);
			}
			pokeb(CAN_BASE, 0x56, ((length << 4) | 0x08));	// load config register
			pokeb(CAN_BASE, 0x51, 0x66);      				// set transmit request
			*/
		}
	}

	void NET_CanInterface::read_frame(struct can_frame * frame_rd, int *recvBytes)
	{
		
	}

	int NET_CanInterface::close_port()
	{
		
	}
#endif
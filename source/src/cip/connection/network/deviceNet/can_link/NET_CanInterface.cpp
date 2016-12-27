#include "NET_CanInterface.h"

#ifdef WIN32
	#include <windows.h>
#else
	#include "fcntl.h>
	#include <net/if.h>
	#include <stdio.h>
	#include <cstring>
	#include <sys/ioctl.h>
	#include <linux/can/raw.h>
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

	int NET_CanInterface::send_port(struct can_frame *frame)
	{
		LPDWORD written = NULL;
		WriteFile(soc, frame, sizeof(struct can_frame), written, NULL);
		return (int)*written;
	}

	void NET_CanInterface::read_port(struct can_frame * frame_rd, int *recvBytes)
	{
		ReadFile(soc, frame_rd, sizeof(struct can_frame), (LPDWORD)recvBytes, NULL);
	}

	int NET_CanInterface::close_port()
	{
		CloseHandle(soc);
		return 1;
	}

#else
	int NET_CanInterface::open_port(const char *port)
	{
		struct ifreq ifr;
		struct sockaddr_can addr;
		// open socket
		soc = socket(PF_CAN, SOCK_RAW, CAN_RAW);
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
		if (bind(soc, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		{
			return (-1);
		}
		return 0;
	}
	int NET_CanInterface::send_port(struct can_frame *frame)
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

	void NET_CanInterface::read_port(struct can_frame * frame_rd, int *recvBytes)
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
#endif
//
// Created by Gabriel Ferreira (@gabrielcarvfer)
//
#ifndef NET_CANINTERFACE_H
#define NET_CANINTERFACE_H

#if __linux__
	#include <linux/can.h>
#else
	#ifdef __WIN32__
		#include <Windows.h>
	#endif
	#include <cstdint>

	//From linux/can.h
	#define CAN_MAX_DLEN 8

	/* special address description flags for the CAN_ID */
	 #define CAN_EFF_FLAG 0x80000000U /* EFF/SFF is set in the MSB */
	 #define CAN_RTR_FLAG 0x40000000U /* remote transmission request */
	 #define CAN_ERR_FLAG 0x20000000U /* error message frame */

	 /* valid bits in CAN ID for frame formats */
	 #define CAN_SFF_MASK 0x000007FFU /* standard frame format (SFF) */
	 #define CAN_EFF_MASK 0x1FFFFFFFU /* extended frame format (EFF) */
	 #define CAN_ERR_MASK 0x1FFFFFFFU /* omit EFF, RTR, ERR flags */
	struct can_frame {
		uint32_t   can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
		uint8_t    can_dlc; /* frame payload length in byte (0 .. CAN_MAX_DLEN) */
		uint8_t    __pad;   /* padding */
		uint8_t    __res0;  /* reserved / padding */
		uint8_t    __res1;  /* reserved / padding */
		uint8_t    data[CAN_MAX_DLEN];
	
	};
#endif
class NET_CanInterface
{
    public:
        NET_CanInterface(char * port);
        ~NET_CanInterface ();
        int close_port();
        void read_frame(struct can_frame * frame_rd, int *recvBytes);
        int send_frame(struct can_frame *frame);
        int open_port(const char *port);
	
		typedef enum {
			S0=10, S1=20, S2=50, S3=100, S4=125, 
			S5=250,	S6=500,	S7=800,	S8=1000
		}kCanBaudRate; // All CAN supported speeds in kbits/s

    private:

#ifdef __WIN32__
		HANDLE soc;
		kCanBaudRate baudRate;
#elif __linux__
        int soc;
		int read_can_port;
#elif EMBEDDED
#endif
};

#endif //NET_CANINTERFACE_H
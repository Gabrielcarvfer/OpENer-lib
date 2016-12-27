//
// @author Gabriel Ferreira (gabrielcarvfer)
//

#ifdef WIN32
	#include <Windows.h>
	#include <cstdint>
	#define CAN_MAX_DLEN 8
	struct can_frame {
		uint32_t can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
		uint8_t    can_dlc; /* frame payload length in byte (0 .. CAN_MAX_DLEN) */
		uint8_t    __pad;   /* padding */
		uint8_t    __res0;  /* reserved / padding */
		uint8_t    __res1;  /* reserved / padding */
		uint8_t    data[CAN_MAX_DLEN];
	
	};
#else
	#include <linux/can.h>
#endif
class NET_CanInterface
{
    public:
        NET_CanInterface(char * port);
        ~NET_CanInterface ();
        int close_port();
        void read_port(struct can_frame * frame_rd, int *recvBytes);
        int send_port(struct can_frame *frame);
        int open_port(const char *port);
#ifdef WIN32	
		typedef enum {
			S0=10, S1=20, S2=50, S3=100, S4=125, 
			S5=250,	S6=500,	S7=800,	S8=1000
		}kCanBaudRate;

		typedef enum {

		}kCan;
#endif
    private:
#ifdef WIN32
		HANDLE soc;
		kCanBaudRate baudRate;
#else
        int soc;
#endif
};
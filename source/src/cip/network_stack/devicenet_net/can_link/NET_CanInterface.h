#include <linux/can.h>

class NET_CanInterface
{
    public:
        NET_CanInterface(char * port);
        ~NET_CanInterface ();
        int close_port();
        void read_port(struct can_frame * frame_rd, int *recvBytes);
        int send_port(struct can_frame *frame);
        int open_port(const char *port);
    private:
        int soc;
};
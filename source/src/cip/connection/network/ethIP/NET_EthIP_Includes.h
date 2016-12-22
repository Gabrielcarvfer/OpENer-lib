//
// Created by gabriel on 20/12/2016.
//

#ifndef OPENERMAIN_NET_ETHIP_INCLUDES_H
#define OPENERMAIN_NET_ETHIP_INCLUDES_H

#ifdef WIN32
    #include <winsock.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
#else
    #include <sys/sock.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sys/select.h>
#endif


#endif //OPENERMAIN_NET_ETHIP_INCLUDES_H

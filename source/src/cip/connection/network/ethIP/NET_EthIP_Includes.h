//
// Created by gabriel on 20/12/2016.
//

#ifndef OPENERMAIN_NET_ETHIP_INCLUDES_H
#define OPENERMAIN_NET_ETHIP_INCLUDES_H

#ifdef WIN32
    #include <winsock2.h>
    #include <windows.h>
    #include <winsock.h>
    #include <ws2tcpip.h>
#elif __linux__
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sys/select.h>
    #include <bits/socket.h>
#endif


#endif //OPENERMAIN_NET_ETHIP_INCLUDES_H

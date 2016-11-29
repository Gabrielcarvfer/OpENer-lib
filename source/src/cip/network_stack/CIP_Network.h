//
// Created by gabriel on 11/11/2016.
//

#ifndef OPENER_CIP_NETWORK_H
#define OPENER_CIP_NETWORK_H

/**
 * @brief Networking layer
 */
class CIP_Network
{

    private:
        std::map<CipUdint, NET_ETHIPConnection>ETHIP_Connection_set;
        std::map<CipUdint, NET_DNETConnection> DNET_Connection_set;

};


#endif //OPENER_CIP_NETWORK_H

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <netdb.h>
#include "net.h"
#include "spdlog/spdlog.h"

Net::Net( std::string hostname, unsigned int port ){
    struct hostent *h;
    // create socket
    if( ( _sockFd = socket( AF_INET, SOCK_DGRAM, 0)) < 0){
        spdlog::error("Error on creating UDP socket");
    }
    memset(&_servaddr, 0, sizeof(_servaddr)); 
    if( (h = gethostbyname ( hostname.c_str())) == NULL ){
        spdlog::error("Unknown hostname [{}]", hostname);
    }      
  /* Socket erzeugen */
    // Filling server information 
    _servaddr.sin_family = AF_INET; 
    _servaddr.sin_port = htons( port ); 
    _servaddr.sin_addr.s_addr = INADDR_ANY; 
    memcpy ( (char *) &_servaddr.sin_addr.s_addr,
           h->h_addr_list[0], h->h_length);
    spdlog::debug("send UDP to {}:{} [{}]",hostname, port, inet_ntoa (*(struct in_addr *) h->h_addr_list[0]));
}

int Net::send(void* data, int len){
    ssize_t num;
    num = sendto( _sockFd, (const char *)data, len, 
            0 , (const struct sockaddr *) &_servaddr,  
            sizeof(_servaddr)); 
    if( num != len){
        spdlog::debug("gave {} frames to UDP, but only {} where send !", len, num);
    }
    return num;
}

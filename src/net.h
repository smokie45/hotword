#include <string>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h>

class Net {
    struct sockaddr_in _servaddr; 
    int _sockFd;
    
    public:
    Net( std::string hostname, unsigned int port);
    int send( void* data, int len );

};


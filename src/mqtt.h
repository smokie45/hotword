#include <string>
#include "mqtt/async_client.h"
// rhassoy expects audio on : "hermes/audioServer/<siteId>/audioFrame"


class Mqtt {

    mqtt::async_client* _client;
    mqtt::topic _topic;

    public:
    Mqtt( string server);
    int send( void* data, int len );

};

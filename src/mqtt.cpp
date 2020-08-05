#include "mqtt.h"

Mqtt::Mqtt( string server, string topic){
    _client = mqtt::async_client( server, "");
    _client->connect()->wait();
    _topic = mqtt::topic( _client, topic, QOS);
}
int Mqtt::send( void* data, int len ){
    _topic.publish( data, len);
    return 0;
}

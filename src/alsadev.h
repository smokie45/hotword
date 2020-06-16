
#include <stdbool.h>
#include <string>

bool alsadev_open_play( std::string adev );
int alsadev_play( void* buf, int num);
bool alsadev_close_play();

bool alsadev_open_capture( std::string adev );
int alsadev_capture(void *buf, int num );
bool alsadev_close_capture();

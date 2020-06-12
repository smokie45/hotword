#include "cmdargs.h"
#include "alsadev.h"
// #include <iostream>
using namespace std;

int main(int argv, char **argc) {
    CmdArgs* arg = new CmdArgs( argv, argc );
    // set debug level
    spdlog::set_level( arg->loglevel );
    spdlog::set_pattern("# [%L] %v");

    // open_snd_dev(NULL);
    alsadev_open_rec("hw:0");
    alsadev_close();
    return 0;
}

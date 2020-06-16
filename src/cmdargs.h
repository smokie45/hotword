#include "spdlog/spdlog.h"
#include <string>
class CmdArgs{

    public:
        spdlog::level::level_enum loglevel;
        std::string alsadev = "hw:0";

        CmdArgs(int argc, char** argv );
};

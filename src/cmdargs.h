#include "spdlog/spdlog.h"
#include <string>
class CmdArgs{

    public:
        spdlog::level::level_enum loglevel;

        CmdArgs(int argc, char** argv );
};

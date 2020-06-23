#include "spdlog/spdlog.h"
#include <string>
class CmdArgs{

    public:
        spdlog::level::level_enum loglevel;
        std::string micdev = "respeaker";
        std::string spkdev = "respeaker_speaker";

        CmdArgs(int argc, char** argv );
};

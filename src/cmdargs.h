#include "spdlog/spdlog.h"
#include <string>
class CmdArgs{

    public:
        spdlog::level::level_enum loglevel = spdlog::level::info;
        std::string micdev = "respeaker";
        std::string spkdev = "respeaker_speaker";
        std::string cfgfile = "/etc/hotword.cfg";
        CmdArgs(int argc, char** argv );
};

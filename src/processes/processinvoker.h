#pragma once

#include <optional>
#include <string>

#include <Components/ExtraClasses/Containers/StringList.h>

namespace Thread
{

using namespace ExtraClasses;

// Default limit of processes before they will be killed (setups by default in
// invoke function)
const int PROCESS_DEFAULT_TIMEOUT{-1};
const int PROCESS_DEFAULT_START_TIMEOUT{100};

class ProcessInvoker
{
public:
    static std::optional<int> invoke(const std::string& program, const StringList& arguments = {},
                                     const int timeout = PROCESS_DEFAULT_TIMEOUT);
    static std::optional<int> invoke(const std::string& program, const StringList& arguments,
                                     std::string& programOutput,
                                     const int timeout = PROCESS_DEFAULT_TIMEOUT);
    static std::optional<int> invoke(const std::string& program, const StringList& arguments,
                                     std::string& programOutput,
                                     std::string& programErrorOutput,
                                     const int timeout = PROCESS_DEFAULT_TIMEOUT);

    static std::string getUsername();
    static bool isSuperuser();
};

} // namespace Libraries

#include "processinvoker.h"

// Update on 18.01.2025: Rewrite to boost::asio
//#include "processclass.hpp"
#include "processclass_v2.h"

#include <unistd.h>

namespace Thread
{

std::optional<int> ProcessInvoker::invoke(const std::string& program,
                                       const StringList& arguments,
                                       const int timeout)
{
    std::string programOutput, errorOutput;
    auto res = invoke(program, arguments, programOutput, errorOutput, timeout);
    return res;
}

std::optional<int> ProcessInvoker::invoke(const std::string& program,
                                       const StringList& arguments,
                                       std::string& programOutput,
                                       const int timeout)
{
    std::string errorOutput;
    auto res = invoke(program, arguments, programOutput, errorOutput, timeout);
    return res;
}

std::optional<int> ProcessInvoker::invoke(const std::string &program, const StringList &arguments, std::string &programOutput, std::string &programErrorOutput, const int timeout)
{
    ProcessClass_v2 process;
    process.setProgram(program);
    process.setArgs(arguments);

    if (!process.exec() || !process.poll(timeout)) {
        return {};
    }

    programOutput = process.output();
    programErrorOutput = process.errorOutput();
    return process.exitCode();
}

std::string ProcessInvoker::getUsername()
{
    return getlogin();
}

bool ProcessInvoker::isSuperuser()
{
    return (getuid() == 0);
}

}

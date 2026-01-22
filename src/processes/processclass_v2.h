#pragma once

#include <boost/asio.hpp>
#include <boost/process.hpp>

#include <Components/ExtraClasses/StringList.h>

namespace Thread
{

using namespace ExtraClasses;

class ProcessClass_v2
{
public:
    ProcessClass_v2();
    ~ProcessClass_v2();

    void setProgram(const std::string& prog);

    void setArgs(const std::string& arg);
    void setArgs(const StringList& argList);

    bool exec();
    bool poll(int64_t timeoutMs = 100);

    int exitCode();

    // Not const only because poll is needed
    std::string output() const;
    std::string errorOutput() const;

private:
    struct Impl;
    std::shared_ptr<Impl> d;
    std::string m_prog;
    StringList m_args;

    std::string m_outBuffer;
    std::string m_outErrorBuffer;
};

}

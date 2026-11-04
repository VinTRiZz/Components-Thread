#pragma once

#include <memory>
#include <string>

#include <Components/ExtraClasses/Containers/StringList.h>

namespace Thread
{

using namespace ExtraClasses;

class Process
{
  public:
    Process();
    ~Process();

    void setProgram(const std::string& program);
    void setArguments(const StringList& argumentList);

    void start();
    bool isWorking() const;

    bool waitForStarted(const int timeout);
    bool waitForFinished(const int timeout);

    std::string readAllOutput() const;

    int pid() const;
    int exitCode() const;

  private:
    struct ProcessPrivate;
    std::unique_ptr<ProcessPrivate> d;
};

} // namespace Libraries

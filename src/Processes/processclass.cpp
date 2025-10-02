#include "processclass.h"

#include  <Components/ExtraClasses/StringList.h>

// PID getting
#include <fcntl.h>
#include <future>
#include <iostream>
#include <stdio_ext.h>
#include <thread>

namespace Thread
{

struct Process::ProcessPrivate {
    std::string m_program;
    StringList m_arguments;
    int pid{-1};
    int exitCode{-1};

    std::string m_programOutput;

    std::future<void> m_awaiterFut;
    FILE* m_processPipe;

    bool m_isStarted{false};

    void startProgram()
    {
        const std::string executionCommand =
            m_program + " " + m_arguments.join(" ");
        m_processPipe = popen(executionCommand.c_str(), "r");

        if (m_processPipe)
            m_isStarted = true;
        else
        {
            m_isStarted = false;
            std::cout << "\nStart error!\n";
            return;
        }

        char buffer[4096];

        while (!feof(m_processPipe))
        {
            if (fgets(buffer, 4096, m_processPipe) != NULL)
            {
                m_programOutput += buffer;
            }
        }
        exitCode    = pclose(m_processPipe);
        m_isStarted = false;

        if (WIFEXITED(exitCode))
        {
            pid      = -1;
            exitCode = WEXITSTATUS(exitCode);
            return;
        } else if (WIFSIGNALED(exitCode))
        {
            pid      = -1;
            exitCode = -1;
            // Here can be signal working
            return;
        }
    }
};

Process::Process() : d{new ProcessPrivate()} {}

Process::~Process()
{
    d->m_isStarted = !d->m_isStarted; // Switch for all awaiters
}

void Process::setProgram(const std::string& program)
{
    if (d->pid == -1) d->m_program = program;
}

void Process::setArguments(const StringList& argumentList)
{
    if (d->pid == -1) d->m_arguments = argumentList;
}

void Process::start()
{
    if (d->m_awaiterFut.valid()) d->m_awaiterFut.get();

    d->m_awaiterFut = std::async([this]() { return d->startProgram(); });
}

bool Process::isWorking() const
{
    return (d->pid != -1);
}

bool Process::waitForStarted(const int timeout)
{
    if (timeout == -1)
    {
        while (!d->m_isStarted)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        return (d->pid != -1);
    }

    for (int i = 0; i < timeout; i++)
    {
        if (d->m_isStarted) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return false;
}
bool Process::waitForFinished(const int timeout)
{
    if (timeout == -1)
    {
        while (d->m_isStarted)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return (d->pid == -1);
    }

    for (int i = 0; i < timeout; i++)
    {
        if (!d->m_isStarted) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return false;
}

std::string Process::readAllOutput() const
{
    return d->m_programOutput;
}

int Process::pid() const
{
    return d->pid;
}
int Process::exitCode() const
{
    return d->exitCode;
}

}

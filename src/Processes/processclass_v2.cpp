#include "processclass_v2.h"

#include <Components/Logger/Logger.h>

#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <boost/thread.hpp>

#include <thread>

namespace bp = boost::process;

namespace Thread
{

struct ProcessClass_v2::Impl
{
    boost::asio::io_context ioc;
    boost::thread_group thg;

    bp::child c;
    bp::ipstream buf;
    bp::ipstream ebuf;
};

ProcessClass_v2::ProcessClass_v2() :
    d {new Impl}
{

}

ProcessClass_v2::~ProcessClass_v2()
{

}

void ProcessClass_v2::setProgram(const std::string &prog)
{
    m_prog = prog;
}

void ProcessClass_v2::setArgs(const std::string &arg)
{
    m_args.stringVect.clear();
    m_args.stringVect.push_back(arg);
}

void ProcessClass_v2::setArgs(const StringList &argList)
{
    m_args = argList;
}

bool ProcessClass_v2::exec()
{
    if (m_prog.empty()) {
        LOG_ERROR("Error starting program: [Empty program]");
        return false;
    }

    m_outBuffer.clear();
    m_outErrorBuffer.clear();

    try {
        d->c = bp::child( bp::search_path(m_prog),
                     bp::args(m_args.join(" ")),
                     bp::std_in.close(),
                     bp::std_out > d->buf,
                     bp::std_err > d->ebuf,
                     bp::on_exit([this](int exCode, const std::error_code& ec){

            if (ec) {
                LOG_ERROR("Program execution error:", ec.message());
                return;
            }

            const int reserveCoeff = 4096;
            while (!d->buf.eof()) {

                if (! m_outBuffer.size() % reserveCoeff) {
                    m_outBuffer.reserve(reserveCoeff);
                }

                m_outBuffer.push_back(d->buf.get());
            }
            if (!m_outBuffer.empty()) {
                m_outBuffer.pop_back(); // Erase last symbol (eof)
                boost::algorithm::trim_right(m_outBuffer);
            }

            while (!d->ebuf.eof()) {

                if (! m_outErrorBuffer.size() % reserveCoeff) {
                    m_outErrorBuffer.reserve(reserveCoeff);
                }

                m_outErrorBuffer.push_back(d->ebuf.get());
            }
            if (!m_outErrorBuffer.empty()) {
                m_outErrorBuffer.pop_back(); // Erase last symbol
                boost::algorithm::trim_right(m_outErrorBuffer);
            }
        }),
        d->ioc);

        d->ioc.run();
        d->c.wait();

    } catch (boost::process::process_error& ex) {
        LOG_ERROR("Error starting program: [", m_prog, "] what: [", ex.what(), "]");
        return false;
    }

    return true;
}

bool ProcessClass_v2::poll(int64_t timeoutMs)
{
    if (!d->c.running()) {
        return true;
    }
    d->ioc.poll();

    bool res = true;
    if (timeoutMs >= 0) {
        res = d->c.wait_for(std::chrono::milliseconds(timeoutMs));
        if (!res) {
            d->c.terminate();
            LOG_WARNING("Process", m_prog, "terminated. Reason: [TIMEOUT]");
        }

    } else {
        d->c.wait();
    }
    return res;
}

int ProcessClass_v2::exitCode()
{
    return d->c.exit_code();
}

std::string ProcessClass_v2::output() const
{
    return m_outBuffer;
}

std::string ProcessClass_v2::errorOutput() const
{
    return m_outErrorBuffer;
}

}

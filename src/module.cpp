#include "module.hpp"
#include <thread>

namespace Thread
{

Module::Module(const ModuleConfiguration &config) :
    m_config {config}
{

}

PModule Module::createModule(const ModuleConfiguration &config)
{
    PModule result;
    result = PModule(new Module(config));
    result->m_pSelf = result;
    return result;
}

Module::~Module()
{

}

ModuleType Module::type() const
{
    return m_config.type;
}

std::string Module::name() const
{
    return m_config.name;
}

ModuleStatus Module::status() const
{
    return m_status.load();
}

void Module::setStatus(ModuleStatus s)
{
    m_status.store(s);
}

void ModuleConfiguration::addRequiredConnection(ModuleType _type)
{
    for (auto t : requiredConnections)
    {
        if (t == _type)
            return;
    }

    requiredConnections.push_back(_type);
}

PMessage Module::sendMessage(const PMessage &msg)
{
    auto receiverType = msg->receiver;
    for (auto con : m_connections)
    {
        if (con->type() == receiverType)
        {
            return con->process(msg);
        }
    }
    return PMessage();
}

PMessage Module::process(PMessage msg)
{
    if (!m_config.messageProcessingFunction)
        return {};

    return m_config.messageProcessingFunction(msg, m_pSelf.lock());
}

std::future<void> Module::init()
{
    if (!m_config.initFunction)
        return {};

    if (m_config.initAsync)
        return std::async([this](){setStatus(m_config.initFunction(m_pSelf.lock()));});

    setStatus(m_config.initFunction(m_pSelf.lock()));
    return {};
}

void Module::start()
{
    if (!m_config.mainCycleFunction)
        return;

    if (m_config.workAsync)
    {
        m_asyncWorker = std::async(m_config.mainCycleFunction, m_pSelf.lock());
    }
    else
    {
        m_threadWorker = std::shared_ptr<std::thread>(
            new std::thread(m_config.mainCycleFunction, m_pSelf.lock()),
            [](std::thread * pThread)
            {
                if (pThread->joinable())
                    pThread->join();
                delete pThread;
            }
        );
    }
}

void Module::poll()
{
    if (!m_config.mainCycleFunction)
        return;

    if (m_config.workAsync)
        m_asyncWorker.get();
    else
        m_threadWorker.reset();
}

void Module::stop()
{
    setStatus(ModuleStatus::MODULE_STATUS_STOPPING);

    if (m_config.stopCallbackFunction) m_config.stopCallbackFunction(m_pSelf.lock());
}

void Module::lock()
{
    m_workingThreadMx.lock();
}

void Module::unlock()
{
    m_workingThreadMx.unlock();
}


void Module::sleep_us(uint64_t time)
{
    std::this_thread::sleep_for(std::chrono::microseconds(time));
}

void Module::sleep_ms(uint64_t time)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

void Module::sleep_s(uint64_t time)
{
    std::this_thread::sleep_for(std::chrono::seconds(time));
}

}

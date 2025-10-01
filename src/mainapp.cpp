#include "mainapp.hpp"
#include "module.hpp"

#include <Components/Logger/Logger.h>

namespace Thread
{

MainApp::MainApp(int argc, char *argv[])
{
    if (argc > 0)
    {
        LOG_INFO("Application started");
        for (int i = 0; i < argc; i++)
            m_argsVect.push_back(argv[i]);
    }
}

MainApp::~MainApp()
{
    this->exit();
}

void MainApp::addModule(const ModuleConfiguration &m)
{
    m_moduleVect.push_back(Module::createModule(m));
}

std::size_t MainApp::argCount() const
{
    return m_argsVect.size();
}

std::vector<std::string> MainApp::args() const
{
    return m_argsVect;
}

std::string MainApp::argument(std::size_t argNo)
{
    if (m_argsVect.size() > argNo)
    {
        return m_argsVect[argNo];
    }
    return {};
}

bool MainApp::init()
{
    // For output only
    size_t initedModuleCount {0};
    size_t currentModuleNo {1};

    // Init modules
    std::vector<std::future<void>> asyncInitResults;
    for (auto module : m_moduleVect)
    {
        // If module must be inited async, then start it
        asyncInitResults.push_back(module->init());
    }

    for (auto& initProcess : asyncInitResults)
    {
        if (initProcess.valid())
        {
            initProcess.get();
            LOG_DEBUG("Init awaiting");
        }
    }

    // Check init results
    for (auto module : m_moduleVect)
    {
        if (module->status() == ModuleStatus::MODULE_STATUS_INITED)
        {
            LOG_OK("PModule inited: " + module->name() + " (" + std::to_string(currentModuleNo++) + " / " + std::to_string(m_moduleVect.size()) + ")");
        }
        else
        {
            LOG_ERROR("PModule: " + module->name() + " init error (" + std::to_string(currentModuleNo++) + " / " + std::to_string(m_moduleVect.size()) + ")");
        }
    }

    // Connect all to all. No big need to optimise (one time fast proceed)
    for (auto module : m_moduleVect)
    {
        for (auto pCon : m_moduleVect)
        {
            // Connect to needed modules by types
            for (auto con : module->m_config.requiredConnections)
            {
                if (pCon->type() == con)
                {
                    module->m_connections.push_back(pCon);
                    break;
                }
            }
        }
    }

    LOG_INFO("Initialisation complete");
    return (initedModuleCount == m_moduleVect.size());
}

int MainApp::exec()
{
    LOG_INFO("Starting modules");

    // Start modules
    for (auto module : m_moduleVect)
    {
        if (module->status() == ModuleStatus::MODULE_STATUS_INITED)
        {
            module->start();
            LOG_OK(std::string("PModule ") + module->name() + " started");
        }
        else
        {
            LOG_ERROR(std::string("PModule ") + module->name() + " not started");
        }
    }

    for (auto module : m_moduleVect)
    {
        module->poll();
    }
    return 0;
}

void MainApp::exit()
{
    for (auto module : m_moduleVect)
    {
        if (module->status() == ModuleStatus::MODULE_STATUS_RUNNING)
            module->stop();
    }

    LOG_INFO("App exit normal");
}

}

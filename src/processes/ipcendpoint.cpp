#include "ipcendpoint.hpp"

#include <Components/Logger/Logger.h>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_condition.hpp>

#include <thread>
#include <atomic>

namespace bip = boost::interprocess;

namespace IPC
{

struct Endpoint::Impl
{
    std::string address;
    std::atomic<bool> isConnected {false};
    std::atomic<bool> isWorking   {false};

    std::string lastError;

    std::unique_ptr<bip::shared_memory_object>  exchangeObject;
    std::unique_ptr<bip::mapped_region>         exchangeRegion;

    std::unique_ptr<bip::named_mutex>           exchangeWriteMutex;
    std::unique_ptr<bip::named_condition>       exchangeCv;

    std::thread exchangeThread;
    std::unordered_map<std::string, targetProcessor_t> processors;
};

Endpoint::Endpoint() :
    d {new Impl}
{

}

Endpoint::~Endpoint()
{
    if (d->exchangeThread.joinable()) {
        d->exchangeThread.join();
    }
}

void Endpoint::connect(const std::string &exchangeAddress, unsigned exchangeBufferSize)
{
    if (d->exchangeThread.joinable()) {
        d->exchangeThread.join();
    }

    d->address = exchangeAddress;

    d->exchangeThread = std::thread([this, exchangeBufferSize]() -> void {
        try {
            LOG_INFO("IPC::Endpoint Started");
            d->exchangeObject = std::make_unique<bip::shared_memory_object>(
                bip::open_or_create,
                d->address.c_str(),
                bip::read_write
            );
            d->exchangeObject->truncate(exchangeBufferSize);

            d->exchangeRegion = std::make_unique<bip::mapped_region>(
                *d->exchangeObject,
                bip::read_write
            );

            d->exchangeWriteMutex = std::make_unique<bip::named_mutex>(
                bip::open_or_create,
                (d->address + "_mx").c_str()
            );
            d->exchangeCv = std::make_unique<bip::named_condition>(
                bip::open_or_create,
                (d->address + "_cv").c_str()
            );

            d->isConnected = true;

            while (d->isWorking) {

            }

            LOG_INFO("IPC::Endpoint Exited normally");
        }
        catch (const bip::interprocess_exception& e) {
            d->lastError = "Send exception: ";
            d->lastError += e.what();
            LOG_ERROR("IPC::Endpoint Exception:", e.what());
            d->isConnected = false;
        }
    });
}

bool Endpoint::isConnected() const
{
    return d->isConnected;
}

void Endpoint::disconnect()
{
    if (!isConnected()) {
        return;
    }

    while (isConnected()) {
        d->isWorking = false;
        exchange_stopWait();
    }

    if (d->exchangeThread.joinable()) {
        d->exchangeThread.join();
    }

    d->exchangeObject->remove(d->address.c_str());
    d->exchangeWriteMutex->remove((d->address + "_mx").c_str());
    d->exchangeCv->remove((d->address + "_cv").c_str());

    d->exchangeObject.reset();
    d->exchangeRegion.reset();
    d->exchangeWriteMutex.reset();
    d->exchangeCv.reset();
    LOG_INFO("IPC::Endpoint Disconnected");
}

bool Endpoint::send(const std::string &messageStr)
{
    auto sendEnd = messageStr.size();
    std::size_t partSize {0};
    auto deltaSize = PACKET_PART_SIZE - 1;
    for (std::size_t startPos = 0; startPos < sendEnd; startPos += deltaSize) {
        partSize = sendEnd - startPos;
        partSize = (partSize > deltaSize ? deltaSize : partSize);

        if (!sendPart(std::string_view(messageStr.data() + startPos, partSize))) {
            return false;
        }
    }
    LOG_DEBUG("IPC::Endpoint Message sent:", messageStr);
    return true;
}

void Endpoint::addProcessor(const std::string &target, const targetProcessor_t &targetProcessor)
{
    d->processors[target] = targetProcessor;
}

std::string Endpoint::getLastError() const
{
    return d->lastError;
}

bool Endpoint::sendPart(const std::string_view &str)
{
    if (!d->isConnected) {
        d->lastError = "Not connected";
        return false;
    }

    try {
        bip::scoped_lock<bip::named_mutex> lock(*d->exchangeWriteMutex);
        auto* addr = static_cast<char*>(d->exchangeRegion->get_address());
        std::copy(str.begin(), str.end(), addr);
        *(addr + str.size()) = '\0';

        exchange_stopWait();
    }
    catch (const bip::interprocess_exception& e) {
        d->lastError = "Send exception: ";
        d->lastError += e.what();
        LOG_ERROR("Part send exception:", e.what());
        return false;
    }
    return true;
}

void Endpoint::exchange_wait()
{
    bip::scoped_lock<bip::named_mutex> lock(*d->exchangeWriteMutex);
    d->exchangeCv->wait(lock);
}

void Endpoint::exchange_stopWait()
{
    bip::scoped_lock<bip::named_mutex> lock(*d->exchangeWriteMutex);
    d->exchangeCv->notify_one();
}

}

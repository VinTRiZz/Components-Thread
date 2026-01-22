#pragma once

#include <string>
#include <memory>
#include <functional>

namespace IPC
{
using targetProcessor_t = std::function<void(const std::string&)>;

constexpr unsigned PACKET_PART_SIZE {1024};

class Endpoint
{
public:
    Endpoint();
    ~Endpoint();

    void connect(const std::string& exchangeAddress, unsigned exchangeBufferSize = PACKET_PART_SIZE * 8);
    bool isConnected() const;
    void disconnect();

    bool send(const std::string& messageStr);
    void addProcessor(const std::string& target,
                      const targetProcessor_t& targetProcessor);

    std::string getLastError() const;

private:
    bool sendPart(const std::string_view& str);

    void exchange_wait();
    void exchange_stopWait();

    struct Impl;
    std::shared_ptr<Impl> d;
};

}

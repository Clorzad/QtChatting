#pragma once
#include "RingBuffer.hpp"
#include "Singleton.hpp"
#include "protos/message.grpc.pb.h"
#include "protos/message.pb.h"
#include <condition_variable>
#include <cstddef>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <memory>

class RpcConnectionPool {
public:
    RpcConnectionPool(size_t size, std::string host, std::string port);
    std::unique_ptr<message::VarifyService::Stub> getConnection();
    void releaseConnection(std::unique_ptr<message::VarifyService::Stub> connection);

private:
    CircularQueueMPMC<message::VarifyService::Stub> connections_;
    std::condition_variable cond_;
    std::mutex mutex_;
    std::string target_;
    size_t max_size_;
    size_t current_size_;
};

class VarifyGrpcClient : public Singleton<VarifyGrpcClient> {
    friend class Singleton<VarifyGrpcClient>;

public:
    message::GetVarifyRsp getVarifyCode(std::string email, int module);
    
private:
    VarifyGrpcClient();
    std::unique_ptr<RpcConnectionPool> pool_;
};

#include "VarifyGrpcClient.h"
#include "ConfigMgr.h"
#include "RingBuffer.hpp"
#include "protos/message.grpc.pb.h"
#include <cstddef>
#include <cstdlib>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <memory>

message::GetVarifyRsp VarifyGrpcClient::getVarifyCode(std::string email)
{
    grpc::ClientContext context;
    message::GetVarifyRsp reply;
    message::GetVarifyReq request;
    request.set_email(email);
    auto stub = pool_->getConnection();
    grpc::Status status = stub->GetVarifyCode(&context, request, &reply);
    if (status.ok()) {
        pool_->releaseConnection(std::move(stub));
        return reply;
    } else {
        pool_->releaseConnection(std::move(stub));
        reply.set_error(ErrorCodes::ERR_RPC_FAILED);
        return reply;
    }
}

VarifyGrpcClient::VarifyGrpcClient()
{
    auto& config_mgr = ConfigMgr::getInstance();
    std::string host = config_mgr["VarifyServer"]["host"];
    std::string port = config_mgr["VarifyServer"]["port"];
    size_t pool_size = atoi(config_mgr["VarifyServer"]["pool_size"].c_str());
    pool_ = std::make_unique<RpcConnectionPool>(pool_size, host, port);
}

RpcConnectionPool::RpcConnectionPool(size_t size, std::string host, std::string port)
    : connections_(size)
    , target_(host + ":" + port)
    , max_size_(size - 1)
    , current_size_(0)
{    
}

std::unique_ptr<message::VarifyService::Stub> RpcConnectionPool::getConnection()
{
    
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this]() {
        return !connections_.isEmpty() || current_size_ < max_size_;
    });
    if (!connections_.isEmpty()) {
        std::unique_ptr<message::VarifyService::Stub> con;
        connections_.dequeue(con);
        return con;
    }
    ++current_size_;
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(target_, grpc::InsecureChannelCredentials());
    return message::VarifyService::NewStub(channel);

}

void RpcConnectionPool::releaseConnection(std::unique_ptr<message::VarifyService::Stub> connection)
{
    std::unique_lock<std::mutex> lock(mutex_);
    connections_.enqueue(std::move(connection));
    cond_.notify_one();
}

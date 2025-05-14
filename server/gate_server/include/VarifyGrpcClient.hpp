#pragma once
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <memory>
#include "protos/message.grpc.pb.h"
#include "common.h"
#include "Singleton.hpp"
#include "protos/message.pb.h"

class VarifyGrpcClient : public Singleton<VarifyGrpcClient> {
    friend class Singleton<VarifyGrpcClient>;

public:
    message::GetVarifyRsp getVarifyCode(std::string email)
    {
        grpc::ClientContext context;
        message::GetVarifyRsp reply;
        message::GetVarifyReq request;
        request.set_email(email);
        grpc::Status status = stub_->GetVarifyCode(&context, request, &reply);

        if (status.ok()) {
            return reply;
        } else {
            reply.set_error(ErrorCodes::PRC_FAILED);
            return reply;
        }
    }

private:
    VarifyGrpcClient()
    {
        std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials());
        stub_ = message::VarifyService::NewStub(channel);
    }
    std::unique_ptr<message::VarifyService::Stub> stub_;
};
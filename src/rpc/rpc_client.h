// Copyriht (c) 2015, Flash Authors. All Rights Reserved
// Author: yuanyi03@baidu.com

#ifndef _RPC_CLIENT_H_
#define _RPC_CLIENT_H_

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "sofa/pbrpc/pbrpc.h"
#include "common/thread/mutex.h"
#include "common/util/log.h"

namespace flash {

template<class RequestType, class ResponseType>
struct RpcContext {
    sofa::pbrpc::RpcController* rpc_controller;    
    const RequestType* request;
    ResponseType* response;
    boost::function<void(const RequestType*, ResponseType*, int32_t)> callback;
    RpcContext(sofa::pbrpc::RpcController* rpc_controller, 
            const RequestType* request,
            ResponseType* response,
            boost::function<void(const RequestType*, ResponseType*, int32_t)> callback) 
        : rpc_controller(rpc_controller),
          request(request),
          response(response),
          callback(callback),
          thread_pool(NULL) {
    }
};

class RpcClientBase {
public:
    static void SetRpcClientOptions(
                int32_t max_in_flow,
                int32_t max_out_flow,
                int32_t max_pending_buffer,
                int32_t thread_num) {
        if (max_in_flow != -1) {
            rpc_client_options_.max_throughput_in = max_in_flow; 
        } 

        if (max_out_flow != -1) {
            rpc_client_options_.max_throughput_out = max_out_flow; 
        }

        if (max_pending_buffer != -1) {
            rpc_client_options_.max_pending_buffer_size = max_pending_buffer; 
        }

        if (thread_num != -1) {
            rpc_client_options_.work_thread_num = thread_num; 
        }

        rpc_client_.ResetOptions(rpc_client_options_);
        WARNING_LOG("rpc client options max_throughput_in[%ld] "
                "max_throughput_out[%ld] max_pending_buffer_size"
                "[%ld] work_thread_num[%ld]",
                rpc_client_options_.max_throughput_in,
                rpc_client_options_.max_throughput_out,
                rpc_client_options_.max_pending_buffer_size,
                rpc_client_options_.work_thread_num);
    }

    RpcClientBase() :rpc_channel_(NULL) {}
    virtual ~RpcClientBase() {}

protected:
    virtual void SetRpcChannel(const std::string& server_addr) {
        common::MutexScoped lock(mutex_); 
        std::map<std::string, sofa::pbrpc::RpcChannel*>::iterator it 
            = rpc_channels_.find(server_addr);
        if (it != rpc_channels_.end()) {
            rpc_channel_ = it->second; 
        } else {
            rpc_channel_ = new sofa::pbrpc::RpcChannel(
                    &rpc_client_, server_addr, channel_options_); 
            rpc_channels_[server_addr] = rpc_channel_;
        }
    } 
protected:
    sofa::pbrpc::RpcChannel* rpc_channel_;
    static sofa::pbrpc::RpcClientOptions rpc_client_options_;
    static sofa::pbrpc::RpcChannelOptions channel_options_;
    static sofa::pbrpc::RpcClient rpc_client_;
    static std::map<std::string, sofa::pbrpc::RpcChannel* > rpc_channels_;
    static common::Mutex mutex_;
};


template<class ServiceType>
class RpcClient : public RpcClientBase {
pubilc:
    explicit RpcClient(const std::string& server_addr) 
            : service_stub_(NULL), current_server_addr_() {
        SetRpcChannel(server_addr);     
    }
    virtual ~RpcClient() {}

    std::string GetServerAddr() {
        return current_server_addr_;
    }

    template<class RequestType, class ResponseType>
    bool AsyncSendRecvMessage(void(ServiceType::*func)(
                sofa::pbrpc::RpcController* rpc_controller,
                const RequestType* request,
                ResponseType* response,
                google::protobuf::Closure* done),
            const RequestType* request,
            ResponseType* response,
            boost::function<void(const RequestType*, ResponseType*, int32_t)> callback,
            int32_t rpc_timeout = 1000) {
        if (request == NULL
                || response == NULL) {
            return false; 
        } 

        sofa::pbrpc::RpcController* rpc_controller =
            new sofa::pbrpc::RpcController();
        rpc_controller->SetTimeout(rpc_timeout);

        RpcContext* context = new RpcContext(request, response, callback);

        google::protobuf::Closure* done = 
            new sofa::pbrpc::NewClosure(
                    RpcClient::template RpcCallback<RequestType, ResponseType>, 
                    this, context);
        (service_stub_->*func)(rpc_controller, request, response, done);
        return true;
    }

    template<class RequestType, class ResponseType>
    bool SendRecvMessage(void(ServiceType::*func)(
                sofa::pbrpc::RpcController* rpc_controller,
                const RequestType* request,
                ResponseType* response,
                google::protobuf::Closure* done), 
            const RequestType* request,
            ResponseType* response, 
            int32_t rpc_timeout = 1000, 
            int32_t* err_code = NULL) {
        if (request == NULL
                || response == NULL) {
            return false;j 
        }
        
        sofa::pbrpc::RpcController* rpc_controller = new sofa::pbrpc::RpcController();
        rpc_controller->SetTimeout(rpc_timeout);
        
        (service_stub_->*func)(rpc_controller, request, response, NULL); 
        bool rs = rpc_controller->Failed();
        if (err_code != NULL) {
            *err_code = rpc_controller->ErrorCode();  
        }
        delete rpc_controller;
        return rs;
    }

    template<class RequestType, class ResponseType> 
    static void RpcCallback(RpcClient<ServiceType>* rpc_client,
            RpcContext* context) {
        if (rpc_client == NULL 
                || context == NULL) {
            return; 
        }

        sofa::pbrpc::RpcController* rpc_controller =
            context->rpc_controller;

        int32_t err_code = 0;
        if (rpc_controller->Failed()) {
            err_code = rpc_controller->ErrorCode(); 
        }

        delete context->rpc_controller;
        context->rpc_controller = NULL;
          
        context->callback(
                context->request, context->response, err_code);
        delete context;
        context = NULL;
    }
protected:
    virtual void SetRpcChannel(const std::string& server_addr) {
        if (current_server_addr_ == server_addr) {
            return; 
        } 

        RpcClientBase::SetRpcChannel(server_addr);
        current_server_addr_ = server_addr;
        service_stub_ = new ServiceType(rpc_channel_);
    }
    
    ServiceType* service_stub_;
    std::string current_server_addr_;
};

}   // ending namespace Flash

#endif


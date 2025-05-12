#include "LogicSystem.h"
#include "HttpConnection.h"
#include <json/reader.h>
#include <json/value.h>
#include <memory>
LogicSystem::LogicSystem()
{
    regGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
        beast::ostream(connection->response_.body())
            << "receive get_test req";});

    regPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection) {
        auto body_str = boost::beast::buffers_to_string(connection->request_.body().data());
        connection->response_.set(http::field::content_type, "text/json");
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
        if (!parse_success) {
            std::cerr << "Error: Failed to parse Json data" << std::endl;
            root["error"] = ErrorCodes::ERROR_JSON;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body()) << jsonstr;
            return true;
        }
        auto email = src_root["email"].asString();
        root["error"] = 0;
        root["email"] = src_root["email"];
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    });
}

bool LogicSystem::handleGet(std::string path, std::shared_ptr<HttpConnection> http_connection)
{
    if (get_handles.find(path) == get_handles.end()) {
        return false;
    }
    get_handles[path](http_connection);
    return true;
}

bool LogicSystem::handlePost(std::string path, std::shared_ptr<HttpConnection> http_connection)
{
    if (post_handles_.find(path) == post_handles_.end()) {
        return false;
    }
    post_handles_[path](http_connection);
    return true;
}

void LogicSystem::regGet(std::string url, HttpHandle http_handle)
{
    get_handles.insert(make_pair(url, http_handle));
}

void LogicSystem::regPost(std::string url, HttpHandle http_handle)
{
    post_handles_.insert(make_pair(url, http_handle));
}

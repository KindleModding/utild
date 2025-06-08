#pragma once
#include "lipc/IHandler.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>

namespace utild::lipc {
template <typename T> class StringHandler;

class LIPCString {
  public:
    LIPCString(char *value, void *data) : value(value), data(data) {
    }
    LIPCcode check(std::string value) {
        if (value.length() + 1 > *(size_t *)data) {
            *(int *)data = value.length() + 1;
            return LIPC_ERROR_BUFFER_TOO_SMALL;
        }
        return LIPC_OK;
    }

    LIPCcode set(const std::string value) {
        auto result = check(value);
        if (result != LIPC_OK)
            return result;
        strcpy(this->value, value.c_str());
        return LIPC_OK;
    }

    char *get() {
        return value;
    }

    std::string toString() {
        return std::string(value);
    }

  private:
    char *value;
    void *data;
};

template <typename T>
using StringCallback = LIPCcode(StringHandler<T> *_this, LIPC *lipc, LIPCString *value);

inline std::unordered_map<std::string, void *> g_string_handlers;
template <typename T> class StringHandler : public IHandler {
  public:
    StringHandler(const std::string &command) : command(command) {
        g_string_handlers[command] = this;
    }

    static LIPCcode propGetCallback(LIPC *lipc, const char *property, void *value, void *data) {

        auto _this = static_cast<StringHandler<T> *>(g_string_handlers[property]);
        if (_this->getter_cb) {
            return _this->getter_cb(_this, lipc, new LIPCString((char*)value, data));
        }
        return LIPC_ERROR_NO_SUCH_PROPERTY;
    }

    static LIPCcode propSetCallback(LIPC *lipc, const char *property, void *value, void *data) {

        auto _this = static_cast<StringHandler<T> *>(g_string_handlers[property]);
        if (_this->setter_cb) {
            return _this->setter_cb(_this, lipc, new LIPCString((char*)value, data));
        }
        return LIPC_ERROR_NO_SUCH_PROPERTY;
    }
    StringHandler *setGetter(std::function<StringCallback<T>> callback) {
        this->getter_cb = callback;
        return this;
    }
    StringHandler *setSetter(std::function<StringCallback<T>> callback) {
        this->setter_cb = callback;
        return this;
    }

    StringHandler *subscribe(LIPC *lipc) {
        LipcRegisterStringProperty(lipc, this->command.c_str(), this->propGetCallback, this->propSetCallback, nullptr);
        return this;
    };

    void setData(T data) {
        this->data = data;
    }
    T getData() {
        return this->data;
    }

  private:
    std::string command;
    T data;
    std::function<StringCallback<T>> getter_cb;
    std::function<StringCallback<T>> setter_cb;
};

} // namespace utild::lipc

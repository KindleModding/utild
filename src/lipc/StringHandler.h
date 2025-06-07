#pragma once
#include "lipc.h"
#include "lipc/IHandler.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <string>
namespace utild::lipc {
template <typename T> class StringHandler;

template <typename T>
using SimpleStringSetterCB = LIPCcode(StringHandler<T> *_this, LIPC *lipc, std::string value, void *data);

template <typename T>
using SimpleStringGetterCB = LIPCcode(StringHandler<T> *_this, LIPC *lipc, char *value, void *data);

template <typename T> struct Data {
    Data() : handler(nullptr), data(nullptr) {}
    void *data;
    StringHandler<T> *handler;
};
inline std::unordered_map<std::string, void*> g_string_handlers;
template <typename T> class StringHandler : public IHandler {
  public:
    StringHandler(const std::string &command) : command(command) {
        g_string_handlers[command] = this;
    }

    static LIPCcode propGetCallback(LIPC *lipc, const char *property, void *value, void *data) {

        auto _this = static_cast<StringHandler<T>*>(g_string_handlers[property]);
        if (_this->getter_cb) {
            return _this->getter_cb(_this, lipc, (char *)value, data);
        }
        return LIPC_ERROR_NO_SUCH_PROPERTY;
    }

    static LIPCcode propSetCallback(LIPC *lipc, const char *property, void *value, void *data) {

        auto _this = static_cast<StringHandler<T>*>(g_string_handlers[property]);
        if (_this->setter_cb) {
            return _this->setter_cb(_this, lipc, reinterpret_cast<char *>(value), data);
        }
        return LIPC_ERROR_NO_SUCH_PROPERTY;
    }
    StringHandler *setGetter(std::function<SimpleStringGetterCB<T>> callback) {
        this->getter_cb = callback;
        return this;
    }
    StringHandler *setSetter(std::function<SimpleStringSetterCB<T>> callback) {
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
    std::function<SimpleStringGetterCB<T>> getter_cb;
    std::function<SimpleStringSetterCB<T>> setter_cb;
};

} // namespace utild::lipc

#pragma once

#include "lipc/IHandler.h"
#include <functional>
#include <string>

namespace utild::lipc {
template <typename T> class IntHandler;

template <typename T> using IntCallback = LIPCcode(IntHandler<T> *_this, LIPC *lipc, int *value);

template <typename T> class IntHandler : public IHandler {
  public:
    IntHandler(const std::string &command) : command(command) {};
    static LIPCcode propGetCallback(LIPC *lipc, const char *property, void *value, void *data) {
        auto _this = static_cast<IntHandler<T> *>(data);
        if (_this->getter_cb) {
            return _this->getter_cb(_this, lipc, (int *)value);
        }
        return LIPC_ERROR_NO_SUCH_PROPERTY;
    }

    static LIPCcode propSetCallback(LIPC *lipc, const char *property, void *value, void *data) {

        auto _this = static_cast<IntHandler<T> *>(data);
        if (_this->setter_cb) {
            return _this->setter_cb(_this, lipc, (int *)&value);
        }
        return LIPC_ERROR_NO_SUCH_PROPERTY;
    }
    IntHandler *setGetter(std::function<IntCallback<T>> callback) {
        this->getter_cb = callback;
        return this;
    }
    IntHandler *setSetter(std::function<IntCallback<T>> callback) {
        this->setter_cb = callback;
        return this;
    }

    IntHandler *subscribe(LIPC *lipc) {
        LipcRegisterIntProperty(lipc, this->command.c_str(), this->propGetCallback, this->propSetCallback, this);
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
    std::function<IntCallback<T>> getter_cb;
    std::function<IntCallback<T>> setter_cb;
};
}; // namespace utild::lipc

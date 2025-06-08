#pragma once

#include "lipc.h"
#include "lipc/IHandler.h"
#include <functional>

namespace utild::lipc {

class LIPCHash {
  public:
    LIPCHash(LIPCha *ha, int index) : ha(ha), index(index) {
    }
    ~LIPCHash() {
    }

    LIPCcode set(std::string &key, std::string &value) {
        return LipcHasharrayPutString(this->ha, this->index, key.c_str(), value.c_str());
    }

    LIPCcode set(std::string &key, int value) {
        return LipcHasharrayPutInt(this->ha, this->index, key.c_str(), value);
    }

    LIPCcode set(std::string &key, const unsigned char *value, size_t size) {
        return LipcHasharrayPutBlob(this->ha, this->index, key.c_str(), value, size);
    }

    int getInt(std::string &key) {
        int value;
        LIPCcode res = LipcHasharrayGetInt(this->ha, this->index, key.c_str(), &value);
        if (res == LIPC_OK) {
            return value;
        }
        return -1;
    }

    std::string getString(std::string &key) {
        char *value;
        LIPCcode res = LipcHasharrayGetString(this->ha, this->index, key.c_str(), &value);
        if (res == LIPC_OK) {
            return value;
        }
        return "";
    }

    size_t getBlob(std::string &key, unsigned char *value[]) {
        size_t size;
        LIPCcode res = LipcHasharrayGetBlob(this->ha, this->index, key.c_str(), value, &size);
        if (res == LIPC_OK) {
            return size;
        }
        return 0;
    }

  private:
    LIPCha *ha;
    int index;
};

}

class LIPCHasharray {
  public:
    LIPCHasharray(LIPC *lipc) : lipc(lipc) {
        this->ha = LipcHasharrayNew(this->lipc);
    }

    LIPCHasharray(LIPC *lipc, LIPCha *ha) : lipc(lipc) {
        this->ha = ha;
    }

    ~LIPCHasharray() {
        LipcHasharrayFree(this->ha, 0);
    }

    void Destroy() {
        LipcHasharrayFree(this->ha, 1);
    }

    int size() {
        return LipcHasharrayGetHashCount(this->ha);
    }

    int addHash() {
        size_t size;
        LIPCcode res = LipcHasharrayAddHash(this->ha, &size);
        if (res == LIPC_OK) {
            return size;
        }
        return -1;
    }

    std::string toString() {
        size_t size = 0;
        LipcHasharrayToString(this->ha, nullptr, &size);
        char *result = new char[size + 1];
        LIPCcode res = LipcHasharrayToString(this->ha, result, &size);
        if (res == LIPC_OK) {
            return result;
        }
        return "";
    }

  private:
    LIPCha *ha;
    LIPC *lipc;
};

template <typename T> class HashHandler;

template <typename T> using HashCallback = LIPCcode(HashHandler<T> *_this, LIPC *lipc, LIPCha *value);

template <typename T> class HashHandler : public IHandler {
  public:
    HashHandler(const std::string &command) : command(command) {};

    static LIPCcode propAccessCallback(LIPC *lipc, const char *property, void *value, void *data) {

        auto _this = static_cast<HashHandler<T> *>(data);
        if (_this->setter_cb) {
            return _this->setter_cb(_this, lipc, (LIPCha *)value);
        }
        return LIPC_ERROR_NO_SUCH_PROPERTY;
    }

    HashHandler *setGetter(std::function<HashCallback<T>> callback) {
        this->getter_cb = callback;
        return this;
    }

    HashHandler *setSetter(std::function<HashCallback<T>> callback) {
        this->setter_cb = callback;
        return this;
    }

    HashHandler *subscribe(LIPC *lipc) {
        LipcRegisterHasharrayProperty(lipc, this->command.c_str(), this->propAccessCallback, this);
        return this;
    }

    void setData(T data) {
        this->data = data;
    }

    T getData() {
        return this->data;
    }

  private:
    std::string command;
    T data;
    std::function<HashCallback<T>> getter_cb;
    std::function<HashCallback<T>> setter_cb;
};
}
; // namespace utild::lipc

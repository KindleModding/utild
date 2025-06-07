#pragma once
#include "lipc.h"
#include <string>

class IHandler {
  public:
    static LIPCcode propGetCallback(LIPC *lipc, const char *property, void *value, void *data);
    static LIPCcode propSetCallback(LIPC *lipc, const char *property, void *value, void *data);
    std::string &getCommand() {
        return this->command;
    };
    void setCommand(std::string &command) {
        this->command = command;
    }

  private:
    std::string command;
};

#include "lipc.h"
#include "lipc/StringHandler.h"
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <memory>
#include <stdexcept>
#include <array>
#include <cstdio>

#define APP_PREFIX "com.kindlemodding.utild"
namespace utild {
static volatile sig_atomic_t keep_running = 1;
static LIPC *global_handle = NULL;

void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        printf("\nReceiving shutdown signal. Cleaning up...\n");
        keep_running = 0;
    }
}

static void skeleton_daemon() {
    // taken from https://stackoverflow.com/a/17955149
    pid_t pid;
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);
    chdir("/var/local/kmc/utils");

    int x;
    for (x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
        close(x);
    }
}

} // namespace utild

std::string exec(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

int main(int argc, char *argv[]) {
    bool run_as_daemon = true;

    int opt;
    static struct option long_options[] = {{"no-daemon", no_argument, 0, 'n'}, {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, "n", long_options, NULL)) != -1) {
        switch (opt) {
        case 'n':
            run_as_daemon = false;
            break;
        default:
            fprintf(stderr, "Usage: %s [--no-daemon|-n]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (run_as_daemon) {
        printf("Forking into the background.\n");
        utild::skeleton_daemon();
    } else {
        printf("Running in foreground mode.\n");

        // Setup signal handling for foreground mode
        if (signal(SIGINT, utild::handle_signal) == SIG_ERR) {
            perror("Unable to set SIGINT handler");
            exit(EXIT_FAILURE);
        }

        if (signal(SIGTERM, utild::handle_signal) == SIG_ERR) {
            perror("Unable to set SIGTERM handler");
            exit(EXIT_FAILURE);
        }
    }

    LIPCcode opened;
    LIPC *handle = LipcOpenEx(APP_PREFIX, &opened);
    if (opened != LIPC_OK) {
        printf("Failed to open LIPC\n");
        return 1;
    }

    utild::global_handle = handle;

    utild::lipc::StringHandler<std::nullptr_t> exit_handler("exit");
    exit_handler.setSetter(
        [](utild::lipc::StringHandler<std::nullptr_t> *_this, LIPC *_lipc, utild::lipc::LIPCString* _value) -> LIPCcode {
            utild::keep_running = 0;
            return LIPC_OK;
        }
    )->setGetter([](utild::lipc::StringHandler<std::nullptr_t> *_this, LIPC *_lipc, utild::lipc::LIPCString* value) -> LIPCcode {
        return value->set("Write into this property to exit utild");
    })->subscribe(handle);

    utild::lipc::StringHandler<std::string> cmd_handler("runCMD");

    cmd_handler
        .setSetter([](utild::lipc::StringHandler<std::string> *_this, LIPC *_lipc, utild::lipc::LIPCString* value) -> LIPCcode {
           _this->setData(exec(value->toString()));
           return LIPC_OK;
        })->setGetter([](utild::lipc::StringHandler<std::string> *_this, LIPC *_lipc, utild::lipc::LIPCString* value) -> LIPCcode {
            return value->set(_this->getData().empty() ? "No output yet." : _this->getData().c_str());
        })->subscribe(handle);

    while (utild::keep_running) {
        sleep(1);
    }

    // Cleanup
    LipcClose(handle);
    printf("utild shutting down.\n");

    return 0;
}

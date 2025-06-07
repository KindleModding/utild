# KindleModding utild
### This is a simple utility library that you can interact with over lipc.

# Features
## runCMD
- Run a command on the device
    - Example:
      - `lipc-set-prop com.kindlemodding.utild runCMD "whoami"`
      - `lipc-get-prop com.kindlemodding.utild runCMD` -> `root`

## info
- Get information about the utild build
    - Example:
      - `lipc-get-prop com.kindlemodding.utild info` -> `Build Info: Branch: master, Commit: a37f93e, Built On: 2025-06-07 14:12:47`

## exit
- Exit the utility library
    - Example:
      - `lipc-set-prop com.kindlemodding.utild exit 1`

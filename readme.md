# KindleModding utild
### This is a simple utility library that you can interact with over lipc.

# Features
## runCMD
- Run a command on the device
    - Example:
      - `lipc-set-prop com.kindlemodding.utild runCMD "whoami"`
      - `lipc-get-prop com.kindlemodding.utild runCMD` -> `root`

## exit
- Exit the utility library
    - Example:
      - `lipc-set-prop com.kindlemodding.utild exit 1`

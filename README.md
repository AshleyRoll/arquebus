# Arquebus

An experiment in low latency inter-process shared memory queues.

_Currently under development_ lots more work to go.

This currently only supports Linux but should work for other POSIX like operating systems.
This is a limitation only on the way we open and map shared memory and this can later be
extended to Windows etc.

## TODO

- CI/CD pipelines
- complete Single Producer Single Consumer Queue
- build performance measurement suite
- implement Single Producer Multiple Consumer Queue
- implement Multiple Producer Single Consumer Queue
- improve CMake to allow consumption as library in other projects
- implement fixed-size variants of the queues
- add support for Windows shared memory mapping

## Setup and Build

1. Clone the repo
2. Ensure you have the following installed:
   - **cmake-format** for formatting CMake files
     `yay -S cmake-format`
   - **pre-commit** for managing git pre-commit hooks
     `yay -S pre-commit`
   - **codespell** for checking code spelling issues
     `yay -S codespell`
3. Configure pre-commit hooks
   In the top level project folder run `pre-commit install`

Current development is done in CLion. Open the project and enable
one of the CMake Presets. Suggest `clang-release-dbg` for now.

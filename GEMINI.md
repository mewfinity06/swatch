# Swatch - A Naive File Watcher

## Project Overview

`Swatch` is a C-based file monitoring program designed to detect changes in specified files or directories. It leverages a custom "no-build" system implemented in `nob.h` and `nob.c` for its build process. The core functionality involves continuously checking the modification times of watched files and logging when changes occur.

## Building and Running

This project uses a custom build system (`nob.c`) to compile the `swatch` executable.

### Building the `nob` Build System

First, compile the `nob.c` file to create the build system executable:

```bash
cc -o nob nob.c
```

### Building and Running `Swatch`

Once the `nob` executable is available, you can use it to build and run the `swatch` program.

*   **Build:** To compile the `swatch` program:
    ```bash
    ./nob build
    ```
    This will create the `swatch` executable in the `./build/` directory.

*   **Run:** To run the `swatch` program:
    ```bash
    ./nob run

    ```

### Usage of `Swatch`

The `swatch` program supports the following command-line arguments:

*   `-x <command>`: Specifies a command to execute when a file change is detected. (Note: This feature is currently unimplemented in `src/main.c`).
*   `-w <file_or_directory>`: Specifies files or directories to watch for changes.

Example (conceptual, as `-x` is not yet implemented):

```bash
./nob run -w src/main.c -x "echo 'main.c changed!'"
```

## Development Conventions

This project adheres to the LLVM coding style, with an indent width of 4 spaces, as defined in the `.clang-format` file.

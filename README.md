# RemMux

RemMux is a concurrent client-server application written in C++ that enables
remote shell command execution over TCP. It provides an `ncurses`-based terminal
interface where users can create and navigate multiple independent windows,
similar to a lightweight terminal multiplexer.

> **Academic project:** RemMux was developed for the **Computer Networks**
> course at the Faculty of Computer Science, Alexandru Ioan Cuza University of
> Iași.

## Features

- Remote shell command execution through a TCP client-server architecture
- Concurrent handling of multiple clients using POSIX threads
- Separate UNIX process for each command using `fork()` and `execv()`
- Support for:
  - pipelines: `|`
  - logical AND: `&&`
  - logical OR: `||`
  - sequential execution: `;`
  - output redirection: `>`
  - input redirection: `<`
  - error redirection: `2>`
- Ten-second timeout for commands that do not terminate
- Length-prefixed protocol for transmitting commands and responses
- Multi-window terminal interface built with `ncurses`
- Up to six simultaneous client windows
- Independent history and scrolling for each window
- Automatic interface resizing when the terminal dimensions change
- Error handling for unavailable servers and interrupted connections

## Architecture

RemMux consists of two main components:

### Server

The server listens for TCP connections on port `2908`. Each accepted client is
assigned to a detached POSIX thread, allowing multiple clients to execute
commands without blocking one another.

Commands are parsed by the custom `Commandments` class. Each command is then
executed in a separate UNIX process. The server uses:

- `fork()` to create child processes
- `execv()` to execute programs
- `dup2()` to configure standard input, output, and error redirection
- `waitpid()` to monitor child processes without permanently blocking the server
- signals to terminate commands that exceed the execution timeout

### Client

The client provides an interactive terminal UI implemented with `ncurses`. Each
window maintains its own displayed content and scroll position. The interface
remains available even when the server cannot be reached, allowing the user to
navigate, resize, or create windows before attempting another connection.

## Communication Protocol

Messages use a simple length-prefixed protocol:

1. The client sends a four-byte integer containing the command length.
2. The client sends the command itself.
3. The server parses and executes the command.
4. The server sends a four-byte integer containing the response length.
5. The server sends the command output or an error message.
6. The client displays the response in the active window.

If a client disconnects unexpectedly, only its assigned server thread is
terminated; other connected clients remain unaffected.

## Project Structure

```text
RemMux/
├── Proiect - RemMux C++/
│   ├── clientRemMux.cpp
│   ├── serverRemMux.cpp
│   └── My_Classes/
│       ├── Commandments.cpp
│       ├── Commandments.h
│       ├── my_windows.cpp
│       └── my_windows.h
├── Raportul_Tehnic___RemMux.pdf
└── README.md
```

- `serverRemMux.cpp` — networking, concurrency, process creation, command
  execution, redirection, and timeout handling
- `clientRemMux.cpp` — TCP client and terminal interaction loop
- `Commandments` — command parsing and operator detection
- `my_windows` — creation, resizing, navigation, history, and scrolling for
  `ncurses` windows

## Requirements

RemMux is intended to run on Linux or WSL and requires:

- GCC with C++17 support
- POSIX threads
- `ncurses` and panel development libraries

On Debian, Ubuntu, or WSL:

```bash
sudo apt update
sudo apt install g++ libncurses-dev
```

## Building

Enter the source directory:

```bash
cd "Proiect - RemMux C++"
```

Compile the server:

```bash
g++ -std=c++17 serverRemMux.cpp My_Classes/Commandments.cpp \
    -pthread -o serverRemMux
```

Compile the client:

```bash
g++ -std=c++17 clientRemMux.cpp My_Classes/my_windows.cpp \
    -lncurses -lpanel -o clientRemMux
```

## Running

Start the server:

```bash
./serverRemMux
```

In another terminal, start a client by providing the server address and port:

```bash
./clientRemMux 127.0.0.1 2908
```

Additional clients can be started from other terminals using the same command.

## Client Controls

| Key | Action |
| --- | --- |
| `Enter` | Connect to the server and enter a command |
| `+` or `=` | Create a new window |
| `Tab` | Switch to the next window |
| `Up Arrow` | Scroll up in the active window |
| `Down Arrow` | Scroll down in the active window |
| `q` | Exit the client |

Terminal resize events are detected automatically, and the active windows are
redrawn while preserving their previous content.

## Example Commands

```bash
ls -la
pwd
ls | wc -l
mkdir example && ls
cat input.txt > output.txt
cat missing.txt 2> errors.txt
```

## Limitations and Security Notice

RemMux is an educational systems and networking project, not a production
remote-access tool.

- It does **not** implement the SSH protocol.
- Communication is not encrypted.
- The application does not provide user authentication or authorization.
- Command lookup currently targets executables located in `/usr/bin`.
- Full-screen interactive programs such as `vim`, `nano`, `top`, and `tmux`
  are not supported.
- The application should only be tested in a local or otherwise trusted
  environment.

## Technical Report

A detailed description of the architecture, protocol, execution flow, and usage
scenarios is available in
[`Raportul_Tehnic___RemMux.pdf`](./Raportul_Tehnic___RemMux.pdf).

## Author

**George-Claudiu Pleșca**  
Faculty of Computer Science  
Alexandru Ioan Cuza University of Iași

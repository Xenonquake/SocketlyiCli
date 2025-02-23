Socket Communication Layer
A simple TCP-based client-server communication layer built for educational purposes. This project demonstrates basic socket programming in C, allowing a client (cliently) to send debug messages to a server (socketly), which echoes them back. Written and Debugged by Xenon this project evolved through iterative debugging and enhancement to solve real-world networking challenges.

Overview
  Purpose: Educational tool to learn socket programming, TCP communication, and network debugging.
  Components:
        cliently: A client that connects to a server, sends user-entered messages, and displays responses.
        socketly: A server that listens for connections, receives messages, and echoes them back to the client.
  Features:
        Supports a --debug flag for verbose logging on both client and server.
        Uses POSIX sockets and signal handling for robust operation.
        Built with C23 standard using Clang, emphasizing modern C practices.

License
This project is licensed under the MIT License. See the LICENSE (#license) section below for details.
Prerequisites

  Operating System: Unix-like (e.g., Linux, macOS). Tested on Void Linux.
  Compiler: Clang with C23 support (-std=c23).
  Libraries: 
      libreadline-dev (for cliently’s interactive input).
      Standard POSIX libraries (included in most Unix-like systems).

Install dependencies on Void Linux:

    xbps-install -S clang readline-devel

Setup

  Clone or Copy the Code:
  Save cliently.c and socketly.c in a directory (e.g., socket-layer).
  
  Compile the Programs:
  Client:
    
    clang -std=c23 -o cliently cliently.c -lreadline

Server:
    
    clang -std=c23 -o socketly socketly.c

Network Configuration:

  Ensure client and server machines are on the same network or configure port forwarding if remote.
  Default port: 8080 (TCP). Adjust #define PORT in both files if needed.
  Open port 8080 on the server’s firewall:

        doas ufw allow 8080/tcp
        or.
        doas iptables -A INPUT -p tcp --dport 8080 -j ACCEPT
        doas iptables -L -n | grep 8080


Usage
Running the Server

  Start socketly on the server machine (e.g., HOSTNAME):

    ./socketly --debug

  Output:

        [DEBUG] Server starting
        [DEBUG] Socket created successfully
        [DEBUG] Socket bound to port 8080
        [DEBUG] Server listening on port 8080

Running the Client

  Run cliently on the client machine (e.g., HOSTNAME), specifying the server’s hostname or IP:

    ./cliently SERVER_HOSTNAME --debug

  Replace dkserver with the server’s IP (e.g., xxx.xxx.x.xxx) if needed.
  Output:

        Network Layer Started
        [DEBUG] Argument parsing complete
        [DEBUG] Socket created successfully
        [DEBUG] Hostname resolved
        [DEBUG] Attempting to connect to xxx.xxx.x.xx:xxxx
        Connected to local server!
        Linked to the local socket.  Enter 'quit' to exit.

Sending Messages

  At the Enter command: prompt, type a message (e.g., "Hello Server!") and press Enter.
  The server echoes it back:
  
Client:

    Enter command: Hello Server!
    [DEBUG] Sending command to server
    [DEBUG] Waiting for server response
    Received: Echo: Hello Server!

  Type quit to exit the client (Ctrl-C) is not operational! 

Server:

        [DEBUG] Accepted connection from xxx.xxx.x.xxx:xxxxx
        Received: Hello Server!
        [DEBUG] Command received from client
        [DEBUG] Echo response sent to client

  Type quit to exit the client.

Debugging Tips

  Enable Debug Mode: Use --debug on both client and server to trace execution:

    ./socketly --debug
    ./cliently SERVER_HOSTNAME --debug

  Connection Issues:
        Check server listening: netstat -tlnp | grep xxxx on server.
        Test connectivity: telnet xxx.xxx.x.xx xxxx from hostname.
        Verify firewall: doas ufw status or doas iptables -L -n.
    Router Setup: If remote, ensure port 8080 is forwarded to the server’s IP (e.g., xxx.xxx.x.xxx) on your router.

Project Notes

  Educational Focus: Built to explore socket programming concepts, including TCP sockets, signal handling, and network troubleshooting.
  Development Journey: Started with basic socket code, evolved through debugging duplicate iptables rules, and added --debug for visibility.
  Limitations: Single-client handling in socketly. For multi-client support, consider threads or select.

License
This project is licensed under the MIT License.

MIT License

Copyright (c) 2025 [Xenonquake]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Acknowledgments

  Inspired by the need to debug network communication in a real-world setup.
    Special thanks to the Void Linux environment for testing and development.

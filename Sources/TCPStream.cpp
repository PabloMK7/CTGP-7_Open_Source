/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: TCPStream.cpp
Open source lines: 168/168 (100.00%)
*****************************************************/

#include "TCPStream.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace CTRPluginFramework {

    std::string TCPStream::GetHostIP() {
        #if CITRA_MODE == 0
        struct in_addr host_id;
        host_id.s_addr = gethostid();
        if (host_id.s_addr == INADDR_NONE)
            return "";
        return std::string(inet_ntoa(host_id));
        #else
        return "127.0.0.1";
        #endif
    }

    static bool g_true = true;
    TCPStream::TCPStream(u16 port) {
        runCondition = &g_true;
        listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd < 0) {
            lastErrno = errno;
            return;
        }
        if (!SetBlocking(listen_fd, false)) {
            return;
        }
        BindPort(port);
    }

    TCPStream::~TCPStream() {
        if (listen_fd >= 0) {
            close(listen_fd);
        }
        if (socket_fd >= 0) {
            shutdown(socket_fd, SHUT_RDWR);
            close(socket_fd);
        }
    }

    bool TCPStream::SetBlocking(int fd, bool blocking) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0) {
            lastErrno = errno;
            return false;
        }
        if (!blocking) {
            flags |= O_NONBLOCK;
        } else {
            flags &= ~O_NONBLOCK;
        }
        int res = fcntl(fd, F_SETFL, flags);
        if (res < 0) {
            lastErrno = errno;
            return false;
        }
        return true;
    }

    bool TCPStream::BindPort(u16 port) {
        struct sockaddr_in servaddr = {0};
        servaddr.sin_family      = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port        = htons(port);
        int res = bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
        if (res < 0) {
            lastErrno = errno;
            return false;
        }
        return true;
    }

    bool TCPStream::WaitConnection() {
        int res = listen(listen_fd, 1);
        if (res < 0) {
            lastErrno = errno;
            return false;
        }
        struct sockaddr_in peeraddr = {0};
        socklen_t peeraddr_len = sizeof(peeraddr);
        bool error = false;
        while (true) {
            socket_fd = accept(listen_fd, (struct sockaddr *) &peeraddr, &peeraddr_len);
            if (socket_fd < 0 || peeraddr_len == 0) {
                if (errno == EWOULDBLOCK && *runCondition) {
                    svcSleepThread(1000000);
                    continue;
                }
                error = true;
                break;
            }
            break;
        }
        if (error) {
            lastErrno = errno;
            return false;
        }

        if (!SetBlocking(socket_fd, false))
            return false;

        if (listen_fd >= 0) {
            close(listen_fd);
            listen_fd = -1;
        }
        return true;
    }

    bool TCPStream::Read(void* buffer, size_t size) {
        size_t read_bytes = 0;
        int new_read = 0;
        while (read_bytes != size) {
            new_read = recv(socket_fd, (void*)((uintptr_t)buffer + read_bytes), size - read_bytes, 0);
            if (new_read < 0) {
                if (errno == EWOULDBLOCK && *runCondition) {
                    svcSleepThread(1000000);
                    continue;
                }
                read_bytes = 0;
                break;
            }
            read_bytes += new_read;
        }
        if (new_read < 0) {
            lastErrno = errno;
            return false;
        }
        return read_bytes == size;
    }

    bool TCPStream::Write(void* buffer, size_t size) {
        size_t write_bytes = 0;
        int new_written = 0;
        while (write_bytes != size)
        {
            new_written = send(socket_fd, (void*)((uintptr_t)buffer + write_bytes), size - write_bytes, 0);
            if (new_written < 0) {
                if (errno == EWOULDBLOCK && *runCondition) {
                    svcSleepThread(1000000);
                    continue;
                }
                write_bytes = 0;
                break;
            }
            write_bytes += new_written;
        }
        if (new_written < 0) {
            lastErrno = errno;
            return false;
        }
        return write_bytes == size;
    }
}
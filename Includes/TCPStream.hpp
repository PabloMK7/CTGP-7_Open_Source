/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: TCPStream.hpp
Open source lines: 38/38 (100.00%)
*****************************************************/

#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework {
    class TCPStream {
    private:
        bool* runCondition;

        int listen_fd = -1;
        int socket_fd = -1;
        int lastErrno = 0;

        bool SetBlocking(int fd, bool blocking);
        bool BindPort(u16 port);
    public:

        static std::string GetHostIP();

        TCPStream(u16 port);
        ~TCPStream();

        void SetRunCondition(bool *condition) {runCondition = condition;}

        int GetLastErrno() {return lastErrno;}
        bool WaitConnection();

        bool Read(void* buffer, size_t size);
        bool Write(void* buffer, size_t size);
    };
}
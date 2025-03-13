#pragma once
#include <gdfe/../../../gdfe/include/gdfe/core.h>
#include <gdfe/os/socket.h>

typedef struct GDF_TcpConnection {
    void* internals;
} GDF_TcpConnection;

typedef void (*GDF_TcpConnectionHandlerFP)(u16 someparam);

GDF_TcpConnection* GDF_OpenTCPConnection(const char* addr, u16 port);
void GDF_CloseTCPConnection(GDF_TcpConnection* connection);
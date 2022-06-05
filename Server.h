#ifndef MYCELIUM_SERVER_H
#define MYCELIUM_SERVER_H

#include <cstdio>
#include <cstdlib>
#include <uv.h>
#include <thread>
#include "ByteBuffer.h"

#include <fstream>
std::fstream packet_log;

unsigned int handlePacket(uv_stream_t*, ByteBuffer&);
ByteBuffer decryptPacket(uv_stream_t*, const ByteBuffer&);
void handleDisconnect(uv_stream_t*);

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} tcp_write_req_t;

static uint64_t data_cntr = 0;

void tcp_on_close(uv_handle_t* handle) {
    free(handle);
}

void tcp_post_write(uv_write_t* req, int status) {

    if (status == 0)
        return;

    fprintf(stderr, "uv_write error: %s\n", uv_strerror(status));

    if (status == UV_ECANCELED)
        return;

    if(status != UV_EPIPE) return;
    uv_close((uv_handle_t*)req->handle, tcp_on_close);
}

void tcp_post_shutdown(uv_shutdown_t* req, int status) {
    if(status < 0) {
        fprintf(stderr, "post_shutdown: %s\n", uv_strerror(status));
    }
    data_cntr = 0;
    uv_close((uv_handle_t*)req->handle, tcp_on_close);
    handleDisconnect(req->handle);

}

void tcp_post_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
    uv_shutdown_t* req;

    if(nread == 0)
        return;

    if(nread < 0) {
        fprintf(stderr, "post_read: %s\n", uv_strerror((int)nread));

        req = (uv_shutdown_t*)malloc(sizeof(*req));
        if(!req) return;

        uv_shutdown(req, handle, tcp_post_shutdown);
        return;
    }

    data_cntr += nread;;
    ByteBuffer maybe_encrypted = ByteBuffer((byte_t*)buf->base, nread);
    ByteBuffer decrypted = decryptPacket(handle, maybe_encrypted);

    unsigned long long length = handlePacket(handle, decrypted);
    while(length < decrypted.bytes.size()) {
        ByteBuffer sub = ByteBuffer(decrypted.bytes.data() + length, decrypted.bytes.size() - length);
        length += handlePacket(handle, sub);
    }
}

void tcp_write(uv_stream_t* handle, const ByteBuffer& buf) {
    tcp_write_req_t* wr;
    wr = (tcp_write_req_t*)malloc(sizeof(*wr));
    if(!wr) return;

    wr->buf = uv_buf_init((char*)buf.bytes.data(), buf.bytes.size());

    uv_write(&wr->req, handle, &wr->buf, 1, tcp_post_write);
}

void tcp_alloc_cb(uv_handle_t*, size_t suggested_size, uv_buf_t* buf) {
    buf->base = (char*)malloc(suggested_size);
    if(!buf->base) return;
    buf->len = suggested_size;
}

void tcp_on_connection(uv_stream_t* server, int status) {
    uv_tcp_t* stream;
    int r;

    if(status != 0) return;

    stream = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    if(!stream) return;

    r = uv_tcp_init(uv_default_loop(), stream);
    if(r != 0) return;

    stream->data = server;

    r = uv_accept(server, (uv_stream_t*)stream);
    if(r != 0) return;

    r = uv_read_start((uv_stream_t*)stream, tcp_alloc_cb, tcp_post_read);
    if(r != 0) return;
}

void tcp_start() {
    uv_tcp_t* tcp_server;
    struct sockaddr_in addr{};
    int r;

    r = uv_ip4_addr("0.0.0.0", 25565, &addr);
    if(r != 0) return;

    tcp_server = (uv_tcp_t*)malloc(sizeof(*tcp_server));
    if(!tcp_server) return;

    r = uv_tcp_init(uv_default_loop(), tcp_server);
    if(r != 0) return;

    r = uv_tcp_bind(tcp_server, (const struct sockaddr*)&addr, 0);
    if(r != 0) return;

    r = uv_listen((uv_stream_t*)tcp_server, SOMAXCONN, tcp_on_connection);
    if(r != 0) return;

    r = uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    if(r != 0) return;
}

void tcp_start_thread() {
    printf("Starting TCP server...\n");
    std::thread(tcp_start).detach();
}

#endif //MYCELIUM_SERVER_H

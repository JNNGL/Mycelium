#pragma once

#include "../connection_context.h"
#include "../protocol/serverbound_login_start.h"
#include "../protocol/serverbound_encryption_response.h"

class LoginPacketListener {
public:
    LoginPacketListener();
    ~LoginPacketListener();

public:
    void handleLoginStart(ConnectionContext*, ServerboundLoginStart*);
    void handleEncryptionResponse(ConnectionContext*, ServerboundEncryptionResponse*);
    std::string createHash();

private:
    ByteBuffer sharedSecret;
    ByteBuffer publicKey;

};
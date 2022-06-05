#ifndef MYCELIUM_HEXDIGEST_H
#define MYCELIUM_HEXDIGEST_H

#include <string>
#include <openssl/sha.h>
#include <openssl/bn.h>
#include <string_view>
#include <array>
#include <algorithm>
#include "ByteBuffer.h"

struct DaftHash
{
    DaftHash()
            : ctx_ {}
    {
        SHA1_Init(&ctx_);
    }

    DaftHash(DaftHash const &) = delete;
    DaftHash(DaftHash &&)      = delete;
    DaftHash &operator=(DaftHash const &) = delete;
    DaftHash &operator=(DaftHash &&) = delete;
    ~DaftHash() = default;

    void update(const ByteBuffer& in) {
        SHA1_Update(&ctx_, in.bytes.data(), in.bytes.size());
    }

    std::string finalise() {
        auto result = std::string();
        auto buf = std::array< std::uint8_t, 20 >();
        SHA1_Final(buf.data(), &ctx_);
        BIGNUM *bn = BN_bin2bn(buf.data(), buf.size(), nullptr);
        SHA1_Init(&ctx_);
        if (BN_is_bit_set(bn, 159))
        {
            result += '-';
            auto tmp = std::vector< unsigned char >(BN_num_bytes(bn));
            BN_bn2bin(bn, tmp.data());
            std::transform(tmp.begin(), tmp.end(), tmp.begin(), [](unsigned char b) { return ~b; });
            BN_bin2bn(tmp.data(), (int)tmp.size(), bn);
            BN_add_word(bn, 1);
        }
        auto hex = BN_bn2hex(bn);
        auto view = std::string_view(hex);
        while ((int)view.size() && view[0] == '0')
            view = view.substr(1);
        result.append(view.begin(), view.end());
        OPENSSL_free(hex);
        BN_free(bn);
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        return result;
    }

private:
    SHA_CTX ctx_;
};

#endif //MYCELIUM_HEXDIGEST_H

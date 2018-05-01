#include <cstring>
#include <type_traits>
#include <climits>
#include "sha256.h"

static const std::array<u32, 64> k = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

static const std::array<u32, 8> init = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c,0x1f83d9ab, 0x5be0cd19};

static void process_chunk(const u8* chunk, std::array<u32, 8>& state)
{
    std::array<u32, 64> w;

    for (u32 i = 0; i < 64; i += 4)
        w[i/4] = (chunk[i] << 24) | (chunk[i+1] << 16) | (chunk[i+2] << 8) | chunk[i+3];

    for (u32 i = 16; i < 64; ++i)
    {
        const u32 s0 = ror(w[i-15], 7) ^ ror(w[i-15], 18) ^ (w[i-15] >> 3);
        const u32 s1 = ror(w[i-2], 17) ^ ror(w[i-2], 19) ^ (w[i-2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }

    std::array<u32, 8> tmp = state;

    for (u32 i = 0; i < 64; ++i)
    {
        const u32 S1 = ror(tmp[4], 6) ^ ror(tmp[4], 11) ^ ror(tmp[4], 25);
        const u32 ch = (tmp[4] & tmp[5]) ^ ((~tmp[4]) & tmp[6]);
        const u32 temp1 = tmp[7] + S1 + ch + k[i] + w[i];
        const u32 S0 = ror(tmp[0], 2) ^ ror(tmp[0], 13) ^ ror(tmp[0], 22);
        const u32 maj = (tmp[0] & tmp[1]) ^ (tmp[0] & tmp[2]) ^ (tmp[1] & tmp[2]);
        const u32 temp2 = S0 + maj;
 
        tmp[7] = tmp[6];
        tmp[6] = tmp[5];
        tmp[5] = tmp[4];
        tmp[4] = tmp[3] + temp1;
        tmp[3] = tmp[2];
        tmp[2] = tmp[1];
        tmp[1] = tmp[0];
        tmp[0] = temp1 + temp2;
    }

    for (u32 i = 0; i < tmp.size(); ++i)
        state[i] += tmp[i];
}

std::array<u32, 8> sha256(const u8* data, u64 len)
{
    std::array<u32, 8> state = init;
    u8 tmp_data[64] = {};
    u32 i = 0;

    while ((len - i) >= 64)
    {
        process_chunk(&data[i], state);
        i += 64;
    }

    const u32 rest = len % 64;
    const u32 processed = len / 64;

    std::memcpy(tmp_data, &data[processed * 64], rest);
    tmp_data[rest] = 0x80;

    if (rest >= 64 - 1 - 8)
    {
        process_chunk(tmp_data, state);
        std::memset(tmp_data, 0, 64);       
    }

    len <<= 3;

    tmp_data[56] = (len >> 56) & 0xFF; 
    tmp_data[57] = (len >> 48) & 0xFF;
    tmp_data[58] = (len >> 40) & 0xFF;
    tmp_data[59] = (len >> 32) & 0xFF;
    tmp_data[60] = (len >> 24) & 0xFF; 
    tmp_data[61] = (len >> 16) & 0xFF;
    tmp_data[62] = (len >> 8) & 0xFF;
    tmp_data[63] = len & 0xFF;

    process_chunk(tmp_data, state);

    return state;
}


/**
 * @file TokenStream.cpp
 * @brief Реализация потокового хранилища 3-битных токенов.
 * @author MolNa
 * @date 2026-09-13
 * @version 1.0.0
 * @copyright MIT License
 */

#include "TokenStream.h"

#include <cstring>

/**
    * @brief Запись 3-битного токена. Использует побайтовый memcpy вместо
    *        UB-каста к uint32_t*, который ранее затирал соседний pack.
    * @details Компилятор сводит 3-байтовый memcpy к одной 32-битной загрузке
    *          при -O2 на x86/ARM.
    * @param value Токен 0..7.
    */
void TokenStream::write(uint8_t value) noexcept {
    value &= 0x07u;

    const size_t pack_idx = total_tokens >> 3;        // total_tokens / 8
    const size_t bit_pos  = (total_tokens & 7u) * 3;  // 0, 3, 6, ..., 21

    if (bit_pos == 0) {
        buffer.emplace_back(TripleBytePack{{0, 0, 0}});
    }

    uint8_t* p = buffer[pack_idx].bytes;
    uint32_t pack = 0;
    std::memcpy(&pack, p, 3);

    pack &= ~(0x07u << bit_pos);
    pack |= (static_cast<uint32_t>(value) << bit_pos);

    std::memcpy(p, &pack, 3);
    ++total_tokens;
}

/**
    * @brief Чтение 3-битного токена по логическому индексу. Симметрично write().
    * @param index Логический индекс.
    * @return      Токен 0..7 или 0 при выходе за границы.
    */
uint8_t TokenStream::read(size_t index) const noexcept {
    if (index >= total_tokens) return 0;

    const size_t pack_idx = index >> 3;
    const size_t bit_pos  = (index & 7u) * 3;

    uint32_t pack = 0;
    std::memcpy(&pack, buffer[pack_idx].bytes, 3);
    return static_cast<uint8_t>((pack >> bit_pos) & 0x07u);
}
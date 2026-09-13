#ifndef TOKEN_PACK_CPP
#define TOKEN_PACK_CPP

/**
 * @file TokenPack.h
 * @brief Физический носитель упакованных 3-битных токенов.
 * @author MolNa
 * @date 2026-09-13
 * @version 1.0.0
 * @copyright MIT License
 */

#include <cstdint>

/**
    * @brief 3 байта = 24 бита = 8 упакованных 3-битных токенов.
    * @warning Каст bytes[] к uint32_t некорректен — читает лишний байт.
    *          Обращение всегда через побайтовый memcpy (cм. @see TokenStream).
    */
struct TripleBytePack {
    uint8_t bytes[3];
};

#endif
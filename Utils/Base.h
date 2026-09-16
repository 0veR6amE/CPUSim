#ifndef NAMESPACE_BASE_H
#define NAMESPACE_BASE_H

/**
 * @file Base.h
 * @brief Кодирование чисел в системе счисления по основанию 4.
 * @author MolNa
 * @date 2026-09-16
 * @version 1.0.1
 * @copyright MIT License
 */

#include "../CONST/Enums.h"
#include "../CONST/Number.h"
#include "../Token/TokenStream.h"

#include <array>
#include <cstdint>
#include <vector>

namespace base {

/**
 * @brief Записывает мантиссу в поток токенов в уравновешенной
 *        (balanced) системе счисления по основанию 4.
 * @param s Поток токенов, в который производится запись.
 * @param M Мантисса (целое число со знаком).
 * @note Каждая цифра после преобразования лежит в диапазоне -2..2
 */
inline void writeMantissa(TokenStream& s, int64_t M) {
    std::array<int8_t, 33> buf{};
    std::size_t n = 0;

    int64_t m = M;
    do {
        int32_t r = static_cast<int32_t>(m % 4);
        m /= 4;

        if (r < -2) {
            r += 4;
            --m;
        }

        buf[n++] = static_cast<int8_t>(r);
    } while (m != 0);

    while (n != 0) {
        const int8_t d = buf[--n];
        s.write(LUT_ENCODE_MANTISSA[d + LUT_ENCODE_MANTISSA_OFFSET]);
    }
}

/**
 * @brief Записывает экспоненту в поток токенов в уравновешенной
 *        (balanced) системе счисления по основанию 4.
 * @param s Поток токенов, в который производится запись.
 * @param E Экспонента (целое число со знаком).
 * @note Каждая цифра после преобразования лежит в диапазоне -2..2
 */
inline void writeExponent(TokenStream& s, int32_t E) {
    std::array<int8_t, 17> buf{};
    std::size_t n = 0;

    int32_t e = E;
    do {
        int32_t r = e % 4;
        e /= 4;

        if (r > 2) {
            r -= 4;
            ++e;
        }

        buf[n++] = static_cast<int8_t>(r);
    } while (e != 0);

    while (n != 0) {
        const int8_t d = buf[--n];
        s.write(LUT_ENCODE_EXPONENT[d + LUT_ENCODE_EXPONENT_OFFSET]);
    }
}

} // namespace base

#endif
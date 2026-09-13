/**
 * @file CpuUnit.cpp
 * @brief Реализация потокового АЛУ: сложение и печать чисел.
 * @author MolNa
 * @date 2026-09-13
 * @version 1.0.0
 * @copyright MIT License
 */

#include "CpuUnit.h"

#include <iostream>
#include <vector>

/**
    * @brief Деление мантиссы на 10. Работает в base-4: remainder ∈ [0,9],
    *        частное корректируется, чтобы q*10 + r == cur*4 + r_prev.
    */
TokenStream CpuUnit::divideBy10(const TokenStream& num, const NumberLayout& lay) const {
    TokenStream result;
    int remainder = 0;

    std::vector<uint8_t> temp(lay.mantissa_len);

    for (int i = static_cast<int>(lay.mantissa_len) - 1; i >= 0; --i) {
        const int cur = remainder * 4
                      + LUT_DECODE_MANTISSA[num.read(static_cast<size_t>(i))];
        int q = cur / 10;
        remainder = cur % 10;
        if      (q >  3) { q -= 4; remainder += 10; }
        else if (q < -2) { q += 4; remainder -= 10; }
        temp[static_cast<size_t>(i)] = LUT_ENCODE_MANTISSA[q + 2];
    }
    for (int i = static_cast<int>(lay.mantissa_len) - 1; i >= 0; --i) {
        result.write(temp[static_cast<size_t>(i)]);
    }
    for (size_t i = lay.mantissa_len; i < num.size(); ++i) {
        result.write(num.read(i));
    }
    return result;
}

/**
    * @brief Сложение двух чисел: выравнивание экспонент, поразрядная сумма мантисс,
    *        сборка итогового потока.
    */
TokenStream CpuUnit::add(const TokenStream& numA, const TokenStream& numB) const {
    NumberLayout layA = getLayout(numA);
    NumberLayout layB = getLayout(numB);

    if (layA.is_nan || layB.is_nan) {
        TokenStream nan_res;
        nan_res.write(7); nan_res.write(7);
        return nan_res;
    }

    TokenStream alignedA = numA;
    TokenStream alignedB = numB;
    TokenStream result_exponent;

    // Выравнивание по старшей экспоненте.
    const int comp = compareExponents(numA, layA, numB, layB);
    if (comp > 0) {
        const int shift = exponentToInt(numA, layA) - exponentToInt(numB, layB);
        for (int i = 0; i < shift; ++i) {
            alignedB = divideBy10(alignedB, getLayout(alignedB));
        }
        layB = getLayout(alignedB);
        for (size_t i = layA.mantissa_len; i + 1 < numA.size(); ++i)
            result_exponent.write(numA.read(i));
    } else if (comp < 0) {
        const int shift = exponentToInt(numB, layB) - exponentToInt(numA, layA);
        for (int i = 0; i < shift; ++i) {
            alignedA = divideBy10(alignedA, getLayout(alignedA));
        }
        layA = getLayout(alignedA);
        for (size_t i = layB.mantissa_len; i + 1 < numB.size(); ++i)
            result_exponent.write(numB.read(i));
    } else {
        for (size_t i = layA.mantissa_len; i + 1 < numA.size(); ++i)
            result_exponent.write(numA.read(i));
    }

    // Сумма мантисс в знаковом base-4.
    const size_t max_len = std::max(layA.mantissa_len, layB.mantissa_len);
    int carry = 0;
    std::vector<uint8_t> res;
    res.reserve(max_len + 1);

    for (size_t i = 0; i < max_len || carry != 0; ++i) {
        const int dA = (i < layA.mantissa_len) ? LUT_DECODE_MANTISSA[alignedA.read(i)] : 0;
        const int dB = (i < layB.mantissa_len) ? LUT_DECODE_MANTISSA[alignedB.read(i)] : 0;

        int sum = dA + dB + carry;
        carry = 0;
        if      (sum >  3) { sum -= 4; carry =  1; }
        else if (sum < -2) { sum += 4; carry = -1; }
        res.push_back(LUT_ENCODE_MANTISSA[sum + 2]);
    }

    TokenStream final_result;
    // Мантисса — от старших к младшим.
    for (auto it = res.rbegin(); it != res.rend(); ++it) final_result.write(*it);
    // Окно конфигурации и экспонента.
    for (size_t i = 0; i < result_exponent.size(); ++i)
        final_result.write(result_exponent.read(i));
    final_result.write(7);
    return final_result;
}

/**
    * @brief Печать числа в формате "(d_{n-1} ... d_0)_4 * 10^(e_0 e_1 ...)_4".
    */
void CpuUnit::printNumber(const TokenStream& num) const {
    const NumberLayout lay = getLayout(num);
    if (lay.is_nan) { std::cout << "NaN\n"; return; }

    std::cout << "(";
    for (size_t i = 0; i < lay.mantissa_len; ++i) {
        const int d = LUT_DECODE_MANTISSA[num.read(i)];
        std::cout << (d >= 0 ? "+" : "") << d << " ";
    }
    std::cout << ")_4 * 10^(";

    if (lay.exponent_len == 0) {
        std::cout << "+0";
    } else {
        for (size_t i = 0; i < lay.exponent_len; ++i) {
            const int d = LUT_DECODE_EXPONENT[num.read(lay.exponent_start + i)];
            std::cout << (d >= 0 ? "+" : "") << d << " ";
        }
    }
    std::cout << ")_4\n";
}
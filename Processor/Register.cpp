/**
 * @file Register.cpp
 * @brief Реализация регистра длинного числа.
 * @author MolNa
 * @date 2026-09-13
 * @version 1.0.0
 * @copyright MIT License
 */

#include "Register.h"

#include <array>
#include <cmath>
#include <iostream>

/**
    * @brief Загрузка double в регистр. Мантисса нормализуется к [1,10) в base-10,
    *        затем переводится в base-4 со знаковыми разрядами.
    * @param val Исходное число.
    */
void Register::loadDouble(double val) {
    is_nan        = false;
    base          = MathBase::Base10;
    mantissa.fill(0);
    mantissa_len  = 0;
    exp_value     = 0;

    if (val == 0.0) {
        mantissa_len = 1;
        return;
    }

    exp_value = static_cast<int16_t>(std::floor(std::log10(std::abs(val))));
    double norm = val / std::pow(10.0, static_cast<double>(exp_value));

    // Разворачиваем в фиксированное число base-4 разрядов.
    std::array<int8_t, kMantissaDigits> raw{};
    for (size_t i = 0; i < kMantissaDigits; ++i) {
        const int8_t digit = static_cast<int8_t>(std::floor(norm));
        raw[i] = digit;
        norm = (norm - digit) * 4.0;
    }

    // Балансировка разрядов в [-2..3] с переносами.
    int carry = 0;
    size_t idx = 0;
    for (int i = static_cast<int>(kMantissaDigits) - 1; i >= 0; --i) {
        int cur = raw[i] + carry;
        carry = 0;
        if      (cur >  3) { cur -= 4; carry =  1; }
        else if (cur < -2) { cur += 4; carry = -1; }
        mantissa[idx++] = static_cast<int8_t>(cur);
    }
    if (carry != 0) mantissa[idx++] = static_cast<int8_t>(carry);
    mantissa_len = static_cast<uint8_t>(idx);
}

/**
    * @brief Упаковка регистра: мантисса (MSB first) -> окно 6..6 с базой ->
    *        экспонента в base-4 -> маркер 7.
    * @param stream Целевой поток.
    */
void Register::packToStream(TokenStream& stream) const {
    if (is_nan) { stream.write(7); stream.write(7); return; }

    for (int i = static_cast<int>(mantissa_len) - 1; i >= 0; --i) {
        stream.write(LUT_ENCODE_MANTISSA[mantissa[i] + LUT_ENCODE_MANTISSA_OFFSET]);
    }

    stream.write(6);
    stream.write(static_cast<uint8_t>(base));
    stream.write(6);

    if (exp_value == 0) {
        stream.write(LUT_ENCODE_EXPONENT[0 + LUT_ENCODE_EXPONENT_OFFSET]);
    } else {
        std::array<int8_t, 8> digits{};
        uint8_t n = 0;
        int tmp = exp_value;
        while (tmp != 0) {
            int r = tmp % 4;
            tmp /= 4;
            if      (r >  2) { r -= 4; tmp += 1; }
            else if (r < -2) { r += 4; tmp -= 1; }
            digits[n++] = static_cast<int8_t>(r);
        }
        for (int i = static_cast<int>(n) - 1; i >= 0; --i) {
            stream.write(LUT_ENCODE_EXPONENT[digits[i] + LUT_ENCODE_EXPONENT_OFFSET]);
        }
    }
    stream.write(7);
}

/**
    * @brief Печать регистра в формате "(d_{n-1} ... d_0)_4 * 10^(exp)".
    */
void Register::print() const {
    if (is_nan) { std::cout << "NaN"; return; }
    std::cout << "(";
    for (int i = static_cast<int>(mantissa_len) - 1; i >= 0; --i) {
        std::cout << (mantissa[i] >= 0 ? "+" : "")
                  << static_cast<int>(mantissa[i]) << " ";
    }
    std::cout << ")_4 * 10^(" << exp_value << ")";
}
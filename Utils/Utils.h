#ifndef UTILS_H
#define UTILS_H

/**
 * @file Utils.h
 * @brief Вспомогательные функции: кодирование/декодирование double <-> TokenStream.
 * @author MolNa
 * @date 2026-09-13
 * @version 1.1.0
 * @copyright MIT License
 * @todo Из-за кодирования и декодирования падает точность чисел, исправить
 */


#include "../Token/TokenStream.h"
#include "../CONST/Number.h"

#include <cmath>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>
#include <numbers> // Для std::numbers::pi и e (C++20)

/**
 * @brief Упаковать double в поток 3-битных токенов.
 */
inline TokenStream createStreamFromDouble(double val) {
    TokenStream stream;
    
    // Проверка на NaN
    if (std::isnan(val)) {
        stream.write(7);
        stream.write(7);
        return stream;
    }
    
    // Упрощенный ноль: просто 0 и закрывающая 7
    if (val == 0.0) {
        stream.write(0);
        stream.write(7);
        return stream;
    }

    // Проверка на точное совпадение с системными константами
    if (val == std::numbers::e) {
        stream.write(6); stream.write(4); stream.write(6); stream.write(7);
        return stream;
    }
    if (val == std::numbers::pi) {
        stream.write(6); stream.write(5); stream.write(6); stream.write(7);
        return stream;
    }

    const bool   is_negative = (val < 0);
    const double abs_val     = std::abs(val);

    // Мантисса в [0.1, 1) => ведущий разряд всегда 0
    const int real_exp = static_cast<int>(std::floor(std::log10(abs_val))) + 1;
    double norm = abs_val / std::pow(10.0, real_exp);   // [0.1, 1)

    // 1 ведущий 0 + 8 дробных четверичных разрядов
    std::vector<int8_t> raw;
    raw.push_back(0);
    for (int i = 0; i < 8; ++i) {
        norm *= 4.0;
        const int8_t d = static_cast<int8_t>(std::floor(norm));
        norm -= d;
        raw.push_back(d);
    }

    // Знак: инвертируем все разряды, балансировка вернёт их в [-2..3]
    if (is_negative) {
        for (auto& d : raw) d = static_cast<int8_t>(-d);
    }

    // Балансировка мантиссы (LSB -> MSB)
    std::vector<int8_t> balanced;
    int carry = 0;
    for (auto it = raw.rbegin(); it != raw.rend(); ++it) {
        int cur = *it + carry;
        carry = 0;
        if      (cur >  3) { cur -= 4; carry =  1; }
        else if (cur < -2) { cur += 4; carry = -1; }
        balanced.push_back(static_cast<int8_t>(cur));
    }
    if (carry != 0) balanced.push_back(static_cast<int8_t>(carry));

    // Запись мантиссы MSB -> LSB
    for (auto it = balanced.rbegin(); it != balanced.rend(); ++it) {
        stream.write(LUT_ENCODE_MANTISSA[*it + LUT_ENCODE_MANTISSA_OFFSET]); //
    }

    // Оптимизированный маркер экспоненты (база 10)
    stream.write(6);

    // Балансировка экспоненты (разбиваем на разряды balanced base-4 в диапазоне [-3..2])
    int temp_exp = real_exp;
    std::vector<int8_t> exp_digits;
    while (temp_exp != 0) {
        int rem = temp_exp % 4;
        temp_exp /= 4;
        if      (rem >  2) { rem -= 4; temp_exp += 1; }
        else if (rem < -3) { rem += 4; temp_exp -= 1; } // Синхронизировано с диапазоном [-3..2]
        exp_digits.push_back(static_cast<int8_t>(rem));
    }
    if (exp_digits.empty()) exp_digits.push_back(0);
    
    // Запись экспоненты MSB -> LSB
    for (auto it = exp_digits.rbegin(); it != exp_digits.rend(); ++it) {
        stream.write(LUT_ENCODE_EXPONENT[*it + LUT_ENCODE_EXPONENT_OFFSET]); //
    }

    stream.write(7);
    return stream;
}

/**
 * @brief Декодировать поток токенов обратно в double.
 */
inline double streamToDouble(const TokenStream& stream) {
    if (stream.size() < 1) return 0.0;
    
    // Потоковый NaN (7 7) или пустой маркер
    if (stream.read(0) == 7) {
        if (stream.size() >= 2 && stream.read(1) == 7) return std::nan("");
        return 0.0; 
    }

    // Поиск первой шестерки
    int first_6 = -1;
    for (size_t i = 0; i < stream.size(); ++i) {
        if (stream.read(i) == 6) {
            first_6 = static_cast<int>(i);
            break;
        }
    }

    const int mantissa_len = (first_6 != -1) ? first_6 : static_cast<int>(stream.size()) - 1;

    // Сборка мантиссы через LUT
    double mantissa = 0.0;
    double weight   = 1.0; 
    for (int i = 0; i < mantissa_len; ++i) {
        const int8_t d = LUT_DECODE_MANTISSA[stream.read(static_cast<size_t>(i))]; //
        mantissa += static_cast<double>(d) * weight;
        weight   *= 0.25; 
    }

    int exp = 0;
    double base = 10.0;
    size_t exp_start_idx = 0;

    if (first_6 != -1) {
        // Проверяем, это одиночная '6' или сложное окно '6 [мод] 6'
        if (first_6 + 2 < static_cast<int>(stream.size()) && stream.read(static_cast<size_t>(first_6) + 2) == 6) {
            uint8_t mode = stream.read(static_cast<size_t>(first_6) + 1);
            
            // Если число состоит ТОЛЬКО из окна константы
            if (first_6 == 0) {
                if (mode == 4) return std::numbers::e;
                if (mode == 5) return std::numbers::pi;
            }

            switch (mode) {
                case 1:  base = 2.0;            break;
                case 2:  base = 4.0;            break;
                case 3:  base = std::numbers::e;break;
                default: base = 10.0;           break;
            }
            exp_start_idx = static_cast<size_t>(first_6) + 3;
        } else {
            base = 10.0;
            exp_start_idx = static_cast<size_t>(first_6) + 1;
        }

        // Чтение цифр степени через LUT
        for (size_t i = exp_start_idx; i < stream.size() && stream.read(i) != 7; ++i) {
            int8_t decoded_digit = LUT_DECODE_EXPONENT[stream.read(i)]; //
            exp = exp * 4 + decoded_digit;
        }
    }

    return mantissa * std::pow(base, static_cast<double>(exp));
}

/**
 * @brief Красиво отформатировать число из потока для вывода.
 */
inline std::string streamToPrettyString(const TokenStream& stream, int precision = 6) {
    if (stream.size() >= 2 && stream.read(0) == 7 && stream.read(1) == 7) {
        return "NaN";
    }
    
    if (stream.size() >= 3 && stream.read(0) == 6 && stream.read(2) == 6) {
        const uint8_t c = stream.read(1);
        if (c == 4) return "e";
        if (c == 5) return "\xCF\x80"; // π в UTF-8
    }

    const double v = streamToDouble(stream);

    std::ostringstream oss;
    oss.setf(std::ios::scientific);
    oss.precision(precision);
    oss << v;
    return oss.str();
}

#endif // UTILS_H

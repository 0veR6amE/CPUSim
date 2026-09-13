#ifndef UTILS_H
#define UTILS_H

/**
 * @file Utils.h
 * @brief Вспомогательные функции: кодирование/декодирование double <-> TokenStream.
 * @author MolNa
 * @date 2026-09-13
 * @version 1.0.0
 * @copyright MIT License
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
 * @brief Упаковать double в поток 3-битных токенов (balanced base-4).
 */
inline TokenStream createStreamFromDouble(double val) {
    TokenStream stream;
    
    // Проверка на NaN
    if (std::isnan(val)) {
        stream.write(7);
        stream.write(7);
        return stream;
    }
    
    if (val == 0.0) {
        stream.write(0);
        stream.write(6); stream.write(0); stream.write(6);
        stream.write(0);
        stream.write(7);
        return stream;
    }

    // Проверка на точное совпадение с системными константами
    if (val == std::numbers::pi) {
        stream.write(6); stream.write(5); stream.write(6); stream.write(7);
        return stream;
    }
    if (val == std::numbers::e) {
        stream.write(6); stream.write(4); stream.write(6); stream.write(7);
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

    // Балансировка (LSB -> MSB)
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

    // Запись MSB -> LSB
    for (auto it = balanced.rbegin(); it != balanced.rend(); ++it) {
        stream.write(LUT_ENCODE_MANTISSA[*it + LUT_ENCODE_MANTISSA_OFFSET]);
    }

    // Окно Base-10
    stream.write(6); stream.write(0); stream.write(6);

    // Экспонента с инверсией знаков разрядов (balanced base-4)
    int temp_exp = real_exp;
    std::vector<int8_t> exp_digits;
    while (temp_exp != 0) {
        int rem = temp_exp % 4;
        temp_exp /= 4;
        if      (rem >  2) { rem -= 4; temp_exp += 1; }
        else if (rem < -2) { rem += 4; temp_exp -= 1; }
        exp_digits.push_back(static_cast<int8_t>(rem));
    }
    if (exp_digits.empty()) exp_digits.push_back(0);
    
    for (auto it = exp_digits.rbegin(); it != exp_digits.rend(); ++it) {
        // ИНВЕРСИЯ ЗНАКА: Умножаем разряд на -1 перед кодированием в LUT
        int8_t inverted_digit = static_cast<int8_t>(-(*it));
        stream.write(LUT_ENCODE_EXPONENT[inverted_digit + LUT_ENCODE_EXPONENT_OFFSET]);
    }

    stream.write(7);
    return stream;
}

/**
 * @brief Декодировать поток токенов обратно в double.
 */
inline double streamToDouble(const TokenStream& stream) {
    if (stream.size() < 1) return 0.0;
    
    // Потоковый саморазделяемый NaN (две 7 подряд) или одиночная 7 на старте
    if (stream.read(0) == 7) {
        if (stream.size() >= 2 && stream.read(1) == 7) return std::nan("");
        return 0.0; 
    }

    // Поиск сервисного окна 6..6
    int first_6 = -1, second_6 = -1;
    for (size_t i = 0; i < stream.size(); ++i) {
        if (stream.read(i) == 6) {
            if (first_6 == -1) first_6 = static_cast<int>(i);
            else { second_6 = static_cast<int>(i); break; }
        }
    }

    // Если число начинается сразу с окна константы (например, 6 4 6 7)
    if (first_6 == 0 && second_6 == 2) {
        uint8_t mode = stream.read(1);
        if (mode == 4) return std::numbers::e;
        if (mode == 5) return std::numbers::pi;
    }

    const int mantissa_len = (first_6 != -1) ? first_6
                                             : static_cast<int>(stream.size()) - 1;

    // Сборка мантиссы
    double mantissa = 0.0;
    double weight   = 1.0; 
    for (int i = 0; i < mantissa_len; ++i) {
        const int8_t d = LUT_DECODE_MANTISSA[stream.read(static_cast<size_t>(i))];
        mantissa += static_cast<double>(d) * weight;
        weight   *= 0.25; 
    }

    int exp = 0;
    int base_id = 0;
    
    if (first_6 != -1 && second_6 != -1) {
        base_id = static_cast<int>(stream.read(static_cast<size_t>(first_6) + 1));
        
        // Если это константный режим внутри числа, подменяем мантиссу
        if (base_id == 4) return std::numbers::e;
        if (base_id == 5) return std::numbers::pi;

        // Декодирование экспоненты с учетом поразрядной инверсии знака
        for (size_t i = static_cast<size_t>(second_6) + 1; i < stream.size() && stream.read(i) != 7; ++i) {
            // Читаем сырое сбалансированное значение разряда степени и инвертируем его обратно (-val)
            int8_t decoded_digit = -LUT_DECODE_EXPONENT[stream.read(i)];
            exp = exp * 4 + decoded_digit;
        }
    }

    // Выбор базы математического пространства
    double base = 10.0;
    switch (base_id) {
        case 0: base = 10.0;           break;
        case 1: base =  2.0;           break;
        case 2: base =  4.0;           break;
        case 3: base = std::numbers::e;break;
        default: base = 10.0;          break;
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
    
    // Обработка встроенных констант (исправлены индексы: 4 -> e, 5 -> π)
    if (stream.size() >= 3 && stream.read(0) == 6 && stream.read(2) == 6) {
        const uint8_t c = stream.read(1);
        if (c == 4) return "e";
        if (c == 5) return "\xCF\x80";  // π в UTF-8
    }

    const double v = streamToDouble(stream);

    std::ostringstream oss;
    oss.setf(std::ios::scientific);
    oss.precision(precision);
    oss << v;
    return oss.str();
}

#endif // UTILS_H

#ifndef UTILS_H
#define UTILS_H

/**
 * @file Utils.h
 * @brief Вспомогательные функции: кодирование/декодирование double <-> TokenStream.
 * @author MolNa
 * @date 2026-09-16
 * @version 1.3.0
 * @copyright MIT License
 */


#include "../Token/TokenStream.h"
#include "../CONST/Number.h"

#include "Base.h"

#include <cmath>
#include <cstdint>
#include <sstream>
#include <charconv>
#include <numbers> // Для std::numbers::pi и e (C++20)

/**
 * @brief Разбирает строку в научной нотации в мантиссу и десятичную экспоненту.
 */
inline bool parseScientific(std::string_view sv, int64_t& M, int32_t& E) {
    size_t i = 0;
    bool neg = false;
    if (i < sv.size() && sv[i] == '-') { neg = true; ++i; }
    else if (i < sv.size() && sv[i] == '+') { ++i; }

    int64_t M10         = 0;
    int32_t frac_len    = 0;
    bool    after_dot   = false;

    for (; i < sv.size() && sv[i] != 'e' && sv[i] != 'E'; ++i) {
        const char c = sv[i];
        if (c == '.') {
            after_dot = true;
        } else if (c >= '0' && c <= '9') {
            M10 = M10 * 10 + (c - '0');
            if (after_dot) ++frac_len;
        }
    }

    int32_t exp_read = 0;
    if (i < sv.size()) {
        ++i; // пропускаем 'e' или 'E'
        bool exp_neg = false;
        if (i < sv.size() && sv[i] == '-') { exp_neg = true; ++i; }
        else if (i < sv.size() && sv[i] == '+') { ++i; }

        for (; i < sv.size() && sv[i] >= '0' && sv[i] <= '9'; ++i) {
            exp_read = exp_read * 10 + (sv[i] - '0');
        }
        if (exp_neg) exp_read = -exp_read;
    }

    int32_t E10 = exp_read - frac_len;

    // Удаляем незначащие нули в конце (как в оригинале)
    while (M10 > 0 && M10 % 10 == 0) {
        M10 /= 10;
        ++E10;
    }

    if (neg) M10 = -M10;
    M = M10;
    E = E10;
    return true;
}


/**
 * @brief Упаковать double в поток 3-битных токенов.
 * @note Специальные значения: NaN -> 7 7, +inf -> 7 1 7, -inf -> 7 2 7, 0 -> 0 7. 
 */
inline TokenStream createStreamFromDouble(double val, MathBase base = MathBase::Base10) {
    TokenStream stream;

    if (std::isnan(val)) { 
        stream.write(7); 
        stream.write(7); 
        return stream; 
    }

    if (std::isinf(val)) {
        stream.write(7);
        stream.write(val > 0 ? 1 : 2);
        stream.write(7);
        return stream;
    }

    if (val == 0.0) { 
        stream.write(0); 
        stream.write(7); 
        return stream; 
    }

    if (val == std::numbers::e) {
        stream.write(6); stream.write(4); stream.write(6); stream.write(7);
        return stream;
    }
    if (val == std::numbers::pi) {
        stream.write(6); stream.write(5); stream.write(6); stream.write(7);
        return stream;
    }

    const bool      neg = (val < 0);
    const double    a   = std::abs(val);
    int64_t         M   = 0;
    int32_t         E   = 0;

    switch (base) {
        case MathBase::Base10: {
            char buf[40];
            auto res = std::to_chars(buf, buf + sizeof(buf), val,
                                     std::chars_format::scientific);
            if (res.ec != std::errc{}) { stream.write(7); stream.write(7); return stream; }
            parseScientific(std::string_view(buf, static_cast<size_t>(res.ptr - buf)), M, E);
            break;
        }

        case MathBase::Base2: {
            int32_t e2 = 0;
            double frac = std::frexp(a, &e2);                           // a = frac * 2^e2, frac in [0.5, 1)
            int64_t m2 = static_cast<int64_t>(std::ldexp(frac, 53));    // точное целое
            M = neg ? -m2 : m2;
            E = e2 - 53;
            break;
        }

        case MathBase::Base4: {
            int32_t e2 = 0;
            double frac = std::frexp(a, &e2);
            int64_t m2 = static_cast<int64_t>(std::ldexp(frac, 53));
            int32_t e2s = e2 - 53;
            int32_t e4  = (e2s >= 0) ? (e2s / 2) : ((e2s - 1) / 2);     // floor(e2s/2)
            int32_t rem = e2s - 2 * e4;                                 // 0 или 1
            int64_t m4 = (rem == 1) ? (m2 * 2) : m2;
            M = neg ? -m4 : m4;
            E = e4;
            break;
        }

        case MathBase::BaseE: {
            // val ≈ M * e^E, M ~ 2^52..2^53
            constexpr int32_t K = 36;                                   // e^36 ≈ 4.3e15
            int32_t eg = static_cast<int32_t>(std::floor(std::log(a)));
            E = eg - K;
            double m_approx = a / std::exp(static_cast<double>(E));
            int64_t me = static_cast<int64_t>(std::llround(m_approx));
            M = neg ? -me : me;
            break;
        }
    }

    base::writeMantissa(stream, M);

    if (E != 0) {
        stream.write(6);
        if (base != MathBase::Base10) {
            stream.write(static_cast<uint8_t>(base));
            stream.write(6);
        }
        base::writeExponent(stream, E);
    }

    stream.write(7);
    return stream;
}


inline double streamToDouble(const TokenStream& stream) {
    if (stream.size() < 1) return 0.0;

    // --- специальные маркеры ---
    if (stream.read(0) == 7) {
        if (stream.size() >= 2 && stream.read(1) == 7) return std::nan("");
        if (stream.size() >= 3 && stream.read(2) == 7) {
            const uint8_t k = stream.read(1);
            if (k == 1) return  std::numeric_limits<double>::infinity();
            if (k == 2) return -std::numeric_limits<double>::infinity();
        }
        return 0.0;
    }

    // --- окно констант pi / e ---
    if (stream.size() >= 3 && stream.read(0) == 6 && stream.read(2) == 6) {
        const uint8_t mode = stream.read(1);
        if (mode == 4) return std::numbers::e;
        if (mode == 5) return std::numbers::pi;
    }

    // --- первая шестёрка отделяет мантиссу от экспоненты ---
    int32_t first_6 = -1;
    for (size_t i = 0; i < stream.size(); ++i) {
        if (stream.read(i) == 6) { first_6 = static_cast<int32_t>(i); break; }
    }

    const int32_t mant_len = (first_6 != -1) ? first_6 : static_cast<int32_t>(stream.size()) - 1;

    // --- мантисса как целое в balanced base-4 ---
    int64_t M = 0;
    for (int32_t i = 0; i < mant_len; ++i)
        M = M * 4 + LUT_DECODE_MANTISSA[stream.read(static_cast<size_t>(i))];

    // --- окно экспоненты ---
    int32_t E  = 0;
    double  B  = 10.0;
    size_t  ef = 0;

    if (first_6 != -1) {
        const size_t fi = static_cast<size_t>(first_6);
        if (first_6 + 2 < static_cast<int32_t>(stream.size()) && stream.read(fi + 2) == 6)
        {
            const uint8_t mode = stream.read(fi + 1);
            switch (mode) {
                case 0:  B = 10.0;             break;
                case 1:  B = 2.0;              break;
                case 2:  B = 4.0;              break;
                case 3:  B = std::numbers::e;  break;
                default: B = 10.0;             break;
            }
            ef = fi + 3;
        } else {
            B  = 10.0;   // короткое окно "6" == "6 0 6"
            ef = fi + 1;
        }

        for (size_t i = ef; i < stream.size() && stream.read(i) != 7; ++i)
            E = E * 4 + LUT_DECODE_EXPONENT[stream.read(i)];
    }

    // --- сборка ---
    if (B == 2.0 || B == 4.0)
        return std::ldexp(static_cast<double>(M), E);

    if (B == std::numbers::e)
        return static_cast<double>(M) * std::exp(static_cast<double>(E));

    // B == 10 — точный разбор через snprintf/strtod
    char dbuf[64];
    std::snprintf(dbuf, sizeof(dbuf), "%llde%d", M, E);
    return std::strtod(dbuf, nullptr);
}

/**
 * @brief Формирует человекочитаемое строковое представление потока токенов.
 */
inline std::string streamToPrettyString(const TokenStream& stream, int32_t precision = 17) {
    if (stream.size() >= 2 && stream.read(0) == 7 && stream.read(1) == 7)
        return "NaN";

    if (stream.size() >= 3 && stream.read(0) == 7 && stream.read(2) == 7) {
        if (stream.read(1) == 1) return "inf";
        if (stream.read(1) == 2) return "-inf";
    }

    if (stream.size() >= 3 && stream.read(0) == 6 && stream.read(2) == 6) {
        const uint8_t c = stream.read(1);
        if (c == 4) return "e";
        if (c == 5) return "\xCF\x80";
    }
    
    std::ostringstream oss;
    oss.precision(precision);
    oss << streamToDouble(stream);
    return oss.str();
}
#endif // UTILS_H

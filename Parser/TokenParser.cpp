/**
 * @file TokenParser.cpp
 * @brief Реализация разборщика потока токенов.
 * @author MolNa
 * @date 2026-09-13
 * @version 1.0.0
 * @copyright MIT License
 */

#include "TokenParser.h"

/**
    * @brief Выделить из потока отдельные числа. Маркер 7 закрывает число,
    *        пара 7 7 (на верхнем уровне) трактуется как NaN.
    * @param stream Входной поток.
    * @return       Список подпотоков.
    */
std::vector<TokenStream> TokenParser::parseStream(const TokenStream& stream) const {
    std::vector<TokenStream> results;
    const size_t total = stream.size();
    results.reserve(total / 4 + 1);

    TokenStream current_number;
    size_t i = 0;

    while (i < total) {
        const uint8_t token = stream.read(i);

        // NaN — "7 7" только между числами (буфер current пуст).
        if (token == 7 && current_number.size() == 0
            && (i + 1 < total) && stream.read(i + 1) == 7) {
            TokenStream nan_stream;
            nan_stream.write(7);
            nan_stream.write(7);
            results.push_back(std::move(nan_stream));
            i += 2;
            continue;
        }

        current_number.write(token);

        if (token == 7) {
            results.push_back(std::move(current_number));
            current_number = TokenStream{};
        }
        ++i;
    }
    return results;
}

/**
    * @brief Заглушка приватного декодера мантиссы — прямой доступ к LUT.
    */
int TokenParser::decodeMantissaDigit(uint8_t token) const noexcept {
    return LUT_DECODE_MANTISSA[token & 0x07u];
}

/**
    * @brief Заглушка приватного декодера экспоненты — прямой доступ к LUT.
    */
int TokenParser::decodeExponentDigit(uint8_t token) const noexcept {
    return LUT_DECODE_EXPONENT[token & 0x07u];
}
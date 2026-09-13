#ifndef TOKENS_PARSER_H
#define TOKENS_PARSER_H

/**
 * @file TokenParser.h
 * @brief Разбор непрерывного потока токенов на отдельные числа.
 * @author MolNa
 * @date 2026-09-13
 * @version 1.0.1
 * @copyright MIT License
 */

#include "../CONST/Number.h"
#include "../Token/TokenStream.h"

#include <vector>

/**
    * @brief Разбирает непрерывный поток токенов на отдельные числа.
    * @details Разделители: одиночная 7 — конец числа, пара 7 7 — NaN.
    */
class TokenParser {
private:
    /**
        * @brief Декодировать 3-битный токен мантиссы в знаковый разряд [-2..3].
        * @param token Токен.
        * @return      Разряд.
        */
    [[nodiscard]] int decodeMantissaDigit(uint8_t token) const noexcept;

    /**
        * @brief Декодировать 3-битный токен экспоненты в знаковый разряд [-3..2].
        * @param token Токен.
        * @return      Разряд.
        */
    [[nodiscard]] int decodeExponentDigit(uint8_t token) const noexcept;

public:
    TokenParser() = default;

    /**
        * @brief Разобрать поток на список подпотоков-чисел.
        * @param stream Исходный поток.
        * @return       Список чисел.
        */
    [[nodiscard]] std::vector<TokenStream> parseStream(const TokenStream& stream) const;
};

#endif
#ifndef NUMBER_CPP
#define NUMBER_CPP

/**
 * @file NUMBER.h
 * @brief Таблицы декодирования и раскладка числа в потоке токенов.
 * @author MolNa
 * @date 2026-09-13
 * @version 1.0.0
 * @copyright MIT License
 */

#include "Enums.cpp"

#include <cstddef>
#include <cstdint>

/**
    * @brief LUT декодирования мантиссы: 3-битный токен -> знаковый разряд [-2..3].
    * @details Индексы 6 и 7 намеренно равны 0 — это маркеры окна, а не цифры.
    */
inline constexpr int8_t LUT_DECODE_MANTISSA[8] = { 0, 1, 2, 3, -1, -2, 0, 0 };

/**
    * @brief Смещение для кодирования мантиссы (диапазон [-2..3] -> [0..5]).
    */
inline constexpr uint8_t LUT_ENCODE_MANTISSA_OFFSET = 2;

/**
    * @brief LUT кодирования мантиссы: знаковый разряд [-2..3] -> 3-битный токен.
    */
inline constexpr uint8_t LUT_ENCODE_MANTISSA[6] = { 5, 4, 0, 1, 2, 3 };

/**
    * @brief LUT декодирования экспоненты: 3-битный токен -> знаковый разряд [-3..2].
    */
inline constexpr int8_t LUT_DECODE_EXPONENT[8] = { 0, -1, -2, -3, 1, 2, 0, 0 };

/**
    * @brief Смещение для кодирования экспоненты (диапазон [-3..2] -> [0..5]).
    */
inline constexpr uint8_t LUT_ENCODE_EXPONENT_OFFSET = 3;

/**
    * @brief LUT кодирования экспоненты: знаковый разряд [-3..2] -> 3-битный токен.
    */
inline constexpr uint8_t LUT_ENCODE_EXPONENT[6] = { 3, 2, 1, 0, 4, 5 };

/**
    * @brief Раскладка разобранного числа: где в потоке лежат мантисса и экспонента.
    * @details Тривиально копируемая POD-структура, поэтому передаётся по значению.
    */
struct NumberLayout {
    size_t mantissa_len   = 0;      ///< длина мантиссы в токенах
    size_t exponent_start = 0;      ///< индекс первого токена экспоненты
    size_t exponent_len   = 0;      ///< длина экспоненты в токенах
    bool   is_nan         = false;  ///< флаг NaN
};

#endif
#ifndef REGISTER_H
#define REGISTER_H

/**
 * @file Register.h
 * @brief Регистр длинного числа в нормализованной base-4 форме.
 * @author MolNa
 * @date 2026-09-13
 * @version 1.0.0
 * @copyright MIT License
 */

#include "../CONST/Number.cpp"
#include "../Token/TokenStream.h"

#include <array>
#include <cstddef>
#include <cstdint>

/**
    * @brief Регистр длинного числа: base-4 мантисса со знаковыми разрядами [-2..3],
    *        десятичная экспонента, флаг NaN.
    */
class Register {
public:
    static constexpr size_t kMantissaCapacity = 32; ///< ёмкость мантиссы
    static constexpr size_t kMantissaDigits   = 12; ///< значащих разрядов при loadDouble

    std::array<int8_t, kMantissaCapacity> mantissa{};   ///< разряды (0 = младший)
    uint8_t   mantissa_len = 0;                         ///< длина мантиссы
    int16_t   exp_value    = 0;                         ///< десятичная экспонента
    MathBase  base         = MathBase::Base10;          ///< база экспоненты
    bool      is_nan       = false;                     ///< флаг NaN

    /**
        * @brief Загрузить double, нормализовав мантиссу в знаковый base-4.
        * @param val Исходное число.
        */
    void loadDouble(double val);

    /**
        * @brief Упаковать регистр в поток (закрывается маркером 7).
        * @param stream Целевой поток.
        */
    void packToStream(TokenStream& stream) const;

    /**
        * @brief Печать в stdout в человекочитаемом виде.
        */
    void print() const;
};

#endif
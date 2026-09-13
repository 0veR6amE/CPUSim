#ifndef ENUMS_CPP
#define ENUMS_CPP

/**
 * @file ENUMS.h
 * @brief 
 * @author MolNa
 * @date 2026-09-13
 * @version 1.0.0
 * @copyright MIT License
 */

 #include <cstdint>

/** 
    * @brief @ru:   База системы счисления для экспоненциального множителя (10^n, 2^n, 4^n, e^n).
    * @details      Значения специально совпадают с управляющим токеном внутри окна 6..6,
    *               чтобы декодирование не требовало switch по MathBase.
    * @note @en:    Numeric base for the exponential multiplier. 
    * @details      Enum values intentionally mirror the control tokens used inside the "6..6" window, 
    *               so decoding never needs a MathBase switch. 
    */
enum class MathBase : uint8_t {
    Base10 = 0, // 6 0 6 -> 10^n
    Base2  = 1, // 6 1 6 -> 2^n
    Base4  = 2, // 6 2 6 -> 4^n
    BaseE  = 3  // 6 3 6 -> e^n
};

#endif
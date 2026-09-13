#ifndef CPU_UNIT_H
#define CPU_UNIT_H

/**
 * @file CpuUnit.h
 * @brief Потоковое АЛУ: сложение чисел прямо в токенном представлении.
 * @author MolNa
 * @date 2026-09-13
 * @version 1.0.0
 * @copyright MIT License
 */

#include "../CONST/Number.h"
#include "../Token/TokenStream.h"

#include <algorithm>
#include <cstddef>

/**
    * @brief Арифметико-логическое устройство, работающее напрямую с потоками токенов.
    * @details Поддерживает сложение двух чисел и печать. Делит мантиссу на 10.
    */
class CpuUnit {
private:
    /**
        * @brief Поиск окна 6..6 и заполнение NumberLayout.
        * @param num Поток числа.
        * @return    Раскладка.
        */
    [[nodiscard]] inline NumberLayout getLayout(const TokenStream& num) const noexcept {
        NumberLayout layout;
        if (num.size() >= 2 && num.read(0) == 7 && num.read(1) == 7) {
            layout.is_nan = true;
            return layout;
        }
        int first_6 = -1, second_6 = -1;
        for (size_t i = 0; i < num.size(); ++i) {
            if (num.read(i) == 6) {
                if (first_6 == -1) first_6 = static_cast<int>(i);
                else { second_6 = static_cast<int>(i); break; }
            }
        }
        if (first_6 != -1 && second_6 != -1) {
            layout.mantissa_len   = static_cast<size_t>(first_6);
            layout.exponent_start = static_cast<size_t>(second_6 + 1);
            layout.exponent_len   = num.size() - 1 - layout.exponent_start;
        } else {
            layout.mantissa_len = num.size() - 1;
        }
        return layout;
    }

    /**
        * @brief Поразрядное сравнение экспонент (от старших к младшим).
        * @return  1, 0 или -1.
        */
    [[nodiscard]] inline int compareExponents(const TokenStream& a, const NumberLayout& la,
                                              const TokenStream& b, const NumberLayout& lb) const noexcept {
        const size_t max_len = std::max(la.exponent_len, lb.exponent_len);
        for (int i = static_cast<int>(max_len) - 1; i >= 0; --i) {
            const int dA = (i < static_cast<int>(la.exponent_len))
                ? LUT_DECODE_EXPONENT[a.read(la.exponent_start + i)] : 0;
            const int dB = (i < static_cast<int>(lb.exponent_len))
                ? LUT_DECODE_EXPONENT[b.read(lb.exponent_start + i)] : 0;
            if (dA != dB) return dA > dB ? 1 : -1;
        }
        return 0;
    }

    /**
        * @brief Экспонента как int (только для счётчика шагов сдвига).
        */
    [[nodiscard]] inline int exponentToInt(const TokenStream& num, const NumberLayout& lay) const noexcept {
        int val = 0, power = 1;
        for (size_t i = 0; i < lay.exponent_len; ++i) {
            val += LUT_DECODE_EXPONENT[num.read(lay.exponent_start + i)] * power;
            power *= 4;
        }
        return val;
    }

    /**
        * @brief Деление мантиссы на 10 в столбик с сохранением окна экспоненты.
        */
    [[nodiscard]] TokenStream divideBy10(const TokenStream& num, const NumberLayout& lay) const;

public:
    CpuUnit() = default;

    /**
        * @brief Сложение двух чисел-потоков.
        * @return Новый поток-число.
        */
    [[nodiscard]] TokenStream add(const TokenStream& a, const TokenStream& b) const;

    /**
        * @brief Печать числа из упакованного потока.
        */
    void printNumber(const TokenStream& num) const;
};

#endif
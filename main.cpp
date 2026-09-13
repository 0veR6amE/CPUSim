/**
 * @file main.cpp
 * @brief Демонстрация потоковой арифметики: сложение двух чисел.
 * @author MolNa
 * @date 2026-09-13
 * @version 1.0.0
 * @copyright MIT License   
 * @mainpage
 */

#include "Token/TokenStream.h"
#include "Parser/TokenParser.h"
#include "Processor/CpuUnit.h"

#include <iostream>
#include <clocale>
#include <cmath>

// Вспомогательная функция для генерации TokenStream из обычного double
TokenStream createStreamFromDouble(double val) {
    TokenStream stream;
    if (val == 0.0) {
        stream.write(0); // Мантисса 0
        stream.write(6); stream.write(0); stream.write(6); // Контекст Base-10
        stream.write(0); // Экспонента 0
        stream.write(7);
        return stream;
    }

    // 1. Вычисляем десятичный масштаб (экспоненту)
    int real_exp = static_cast<int>(std::floor(std::log10(std::abs(val))));
    double norm_mantissa = val / std::pow(10.0, real_exp);

    // 2. Генерируем временные четверичные разряды мантиссы
    std::vector<int8_t> raw_b4;
    for (int i = 0; i < 8; ++i) { // Глубина точности: 8 разрядов
        int8_t digit = static_cast<int8_t>(std::floor(norm_mantissa));
        raw_b4.push_back(digit);
        norm_mantissa = (norm_mantissa - digit) * 4.0;
    }

    // Балансируем разряды мантиссы в диапазон [-2, 3]
    std::vector<int8_t> balanced_mantissa;
    int carry = 0;
    for (auto it = raw_b4.rbegin(); it != raw_b4.rend(); ++it) {
        int current = *it + carry;
        carry = 0;
        if (current > 3)       { current -= 4; carry = 1; }
        else if (current < -2) { current += 4; carry = -1; }
        balanced_mantissa.push_back(current);
    }
    if (carry != 0) balanced_mantissa.push_back(carry);

    // Записываем мантиссу в поток (от старших к младшим)
    const uint8_t LUT_M_ENC[] = { 5, 4, 0, 1, 2, 3 }; // Соответствие для [-2..3]
    for (auto it = balanced_mantissa.rbegin(); it != balanced_mantissa.rend(); ++it) {
        stream.write(LUT_M_ENC[*it + 2]);
    }

    // 3. Записываем конфигурационное окно (Base-10 для экспоненты)
    stream.write(6);
    stream.write(0); // 0 соответствует MathBase::Base10
    stream.write(6);

    // 4. Генерируем и балансируем четверичные разряды экспоненты
    std::vector<int8_t> exp_b4;
    int temp_exp = real_exp;
    while (temp_exp != 0) {
        int remainder = temp_exp % 4;
        temp_exp /= 4;
        if (remainder > 2)       { remainder -= 4; temp_exp += 1; }
        else if (remainder < -2) { remainder += 4; temp_exp -= 1; }
        exp_b4.push_back(remainder);
    }
    if (exp_b4.empty()) exp_b4.push_back(0);

    // Записываем экспоненту в поток (от старших к младшим)
    const uint8_t LUT_E_ENC[] = { 3, 2, 1, 0, 4, 5 }; // Соответствие для [-3..2]
    for (auto it = exp_b4.rbegin(); it != exp_b4.rend(); ++it) {
        stream.write(LUT_E_ENC[*it + 3]);
    }

    // 5. Закрываем число разделителем 7
    stream.write(7);
    return stream;
}


int MAIN() {
    std::cout << "=== ТЕСТИРОВАНИЕ ПОТОКОВОЙ АРХИТЕКТУРЫ БЕЗ КЛАССА NUMBER ===\n\n";

    // 1. Создаем два числа-потока разных порядков
    TokenStream regA = createStreamFromDouble(0.0003);    // 3 * 10^-4
    TokenStream regB = createStreamFromDouble(0.0000012); // 1.2 * 10^-6

    // 2. Выводим информацию об исходных потоках токенов
    CpuUnit alu;
    
    std::cout << "Регистр A (0.0003):\n";
    std::cout << "  Математический вид: "; alu.printNumber(regA);
    std::cout << "  Сырые токены в памяти: ";
    for (size_t i = 0; i < regA.size(); ++i) std::cout << (int)regA.read(i) << " ";
    std::cout << "\n  Физический размер: " << regA.sizeInBytes() << " байт\n\n";

    std::cout << "Регистр B (0.0000012):\n";
    std::cout << "  Математический вид: "; alu.printNumber(regB);
    std::cout << "  Сырые токены в памяти: ";
    for (size_t i = 0; i < regB.size(); ++i) std::cout << (int)regB.read(i) << " ";
    std::cout << "\n  Физический размер: " << regB.sizeInBytes() << " байт\n\n";

    std::cout << "--------------------------------------------------\n";
    std::cout << "[ALU] Выполнение поразрядного сложения прямо в потоках...\n";
    std::cout << "--------------------------------------------------\n\n";

    // 3. Сложение силами ALU напрямую (Поток + Поток = Новый Поток)
    TokenStream regResult = alu.add(regA, regB);

    // 4. Анализ результата
    std::cout << "Результат сложения в выходном регистре:\n";
    std::cout << "  Математический вид: "; alu.printNumber(regResult);
    std::cout << "  Итоговые токены в потоке: ";
    for (size_t i = 0; i < regResult.size(); ++i) std::cout << (int)regResult.read(i) << " ";
    std::cout << "\n  Физический размер результата: " << regResult.sizeInBytes() << " байт\n\n";

    // 5. Демонстрация работы парсера потока данных
    std::cout << "[Parser] Имитация сквозного чтения структуры парсером:\n";
    TokenParser parser;
    
    // Передаем итоговый поток в парсер
    std::vector<TokenStream> parsed_numbers = parser.parseStream(regResult);
    std::cout << "  Парсер успешно выделил из буфера чисел: " << parsed_numbers.size() << "\n";
    std::cout << "  Содержимое выделенного числа: "; alu.printNumber(parsed_numbers[0]);
    return 0;
}


int main() {
    std::setlocale(LC_ALL, "Russian");
    return MAIN();
}
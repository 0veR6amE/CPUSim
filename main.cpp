/**
 * @file main.cpp
 * @brief Демонстрация потоковой арифметики: сложение двух чисел.
 * @author MolNa aka 0veR6amE
 * @date 2026-09-13
 * @version 1.0.0
 * @copyright MIT License   
 * @mainpage
 */

#include "Token/TokenStream.h"
#include "Parser/TokenParser.h"
#include "Processor/CpuUnit.h"
#include "Utils/Utils.h"

#include <iostream>
#include <clocale>


/*
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
*/

int MAIN(){
    TokenStream regA = createStreamFromDouble(0.0003);    // 3 * 10^-4
    
    CpuUnit alu;
    std::cout << "Регистр A (0.0003):\n";
    std::cout << "  Математический вид: "; alu.printNumber(regA);
    std::cout << "  Сырые токены в памяти: ";
    for (size_t i = 0; i < regA.size(); ++i) std::cout << (int)regA.read(i) << " ";
    std::cout << "\n  Физический размер: " << regA.sizeInBytes() << " байт\n\n";

    std::cout << streamToPrettyString(regA, 12) << "\n";


    return 0;
}


int main() {
    std::setlocale(LC_ALL, "Russian");
    return MAIN();
}
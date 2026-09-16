/**
 * @file main.cpp
 * @brief Демонстрация потоковой арифметики: сложение двух чисел.
 * @author MolNa aka 0veR6amE
 * @date 2026-09-16
 * @version 1.0.2
 * @copyright MIT License   
 * @mainpage
 */

#include "Token/TokenStream.h"
#include "Parser/TokenParser.h"
#include "Processor/CpuUnit.h"
#include "Utils/Utils.h"

#include <iostream>
#include <clocale>


void test(double val) {
    TokenStream reg = createStreamFromDouble(val);
    
    CpuUnit alu;
    std::cout << "Регистр (" << val << "):\n";
    std::cout << "  Математический вид: "; alu.printNumber(reg);
    std::cout << "  Сырые токены в памяти: ";
    for (size_t i = 0; i < reg.size(); ++i) std::cout << (int)reg.read(i) << " ";
    std::cout << "\n  Физический размер: " << reg.sizeInBytes() << " байт\n";
    std::cout << "  Обратный перевод: " << streamToPrettyString(reg, 12) << "\n\n";

}




int main() {
    std::setlocale(LC_ALL, "Russian");
    test(3);
    test(std::numbers::e);
    test(20);
    test(0.000000324);
    return 0;
}
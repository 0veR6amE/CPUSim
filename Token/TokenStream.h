#ifndef TOKENS_STREAM_H
#define TOKENS_STREAM_H

/**
 * @file TokenStream.h
 * @brief Поток 3-битных токенов с упаковкой 8 в 3 байта.
 * @author MolNa
 * @date 2026-09-13
 * @version 1.0.1
 * @copyright MIT License
 */

#include "../CONST/TokenPack.h"

#include <cstddef>
#include <cstdint>
#include <vector>

/**
    * @brief Поток 3-битных токенов (0..7), упакованных по 8 штук в TripleBytePack.
    * @details Предоставляет доступ по логическому индексу токена (0-based).
    */
class TokenStream {
private:
    std::vector<TripleBytePack> buffer;     ///< упакованные байты
    size_t total_tokens = 0;                ///< число записанных токенов

public:
    TokenStream() = default;

    /**
        * @brief Записать один 3-битный токен (значения > 7 маскируются).
        * @param value Токен 0..7.
        */
    void write(uint8_t value) noexcept;

    /**
        * @brief Прочитать 3-битный токен по логическому индексу.
        * @param index Логический индекс.
        * @return      Токен 0..7 или 0 при выходе за границы.
        */
    [[nodiscard]] uint8_t read(size_t index) const noexcept;

    /// @brief Кол-во записанных токенов.
    [[nodiscard]] size_t size() const noexcept { return total_tokens; }

    /// @brief Физический размер буфера в байтах.
    [[nodiscard]] size_t sizeInBytes() const noexcept {
        return buffer.size() * sizeof(TripleBytePack);
    }

    /// @brief Сырой указатель на байты.
    [[nodiscard]] const uint8_t* data() const noexcept {
        return reinterpret_cast<const uint8_t*>(buffer.data());
    }

    /// @brief Очистить поток, сохранив объект пригодным к использованию.
    void clear() noexcept { buffer.clear(); total_tokens = 0; }
};

#endif
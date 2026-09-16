#ifndef REGISTER_H
#define REGISTER_H

/**
 * @file Register.h
 * @brief Регистр общего назначения
 * @author MolNa
 * @date 2026-09-16
 * @version 1.0.0
 * @copyright MIT License
 */

#include "../Token/TokenStream.h"


class Register {
private:
    TokenStream _data;

public:
    Register() = default;

    inline void                 write(uint8_t token)    { _data.write(token);           }

    inline uint8_t              read(size_t i)          { return _data.read(i);         }
    inline size_t               size()                  { return _data.size();          }
    inline size_t               sizeInBytes()           { return _data.sizeInBytes();   }
    inline const TokenStream&   stream()                { return _data;                 }

    inline bool                 isEmpty()               { return _data.size() == 0;     }


    inline bool isNaN()  {
        return _data.size() >= 2 
            && _data.read(0) == 7 && _data.read(1) == 7;
    }

    inline bool isInf()  {
        return _data.size() >= 3 
            && _data.read(0) == 7 && _data.read(2) == 7
            && (_data.read(1) == 1 || _data.read(1) == 2);
    }

    inline bool isZero()  {
        return _data.size() >= 2
            && _data.read(0) == 0 && _data.read(1) == 7;
    }

    inline bool isConstant()  {
        return _data.size() >= 4
            && _data.read(0) == 6 && _data.read(2) == 6
            && (_data.read(1) == 4 || _data.read(1) == 5);
    }
};

#endif // REGISTER_H
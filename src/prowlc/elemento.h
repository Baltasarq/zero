#ifndef ELEMENTO_H
#define ELEMENTO_H

#include <string>
#include <vector>

#include "excep.h"

class ElementoLexico {
public:
    ElementoLexico(const std::string &e, size_t l, size_t c)
        : elemento( e ), linea( l ), columna( c )
        {}
    virtual ~ElementoLexico()
        {}

    size_t getLinea() const
        { return linea; }
    size_t getColumna() const
        { return columna; }
    const std::string &getElemento() const
        { return elemento; }

private:
    std::string elemento;
    size_t linea;
    size_t columna;
};

class NoIdentificado : public ElementoLexico {
public:
    NoIdentificado(size_t l, size_t c)
        : ElementoLexico( "", l, c )
        {}
};

// ------------------------------------------------------------ Identificadores
class Identificador : public ElementoLexico {
public:
    Identificador(const std::string &id, size_t l, size_t c)
        : ElementoLexico( id, l, c )
        {}
};

class PalabraReservada : public ElementoLexico {
public:
    PalabraReservada(const std::string &k, size_t l, size_t c)
        : ElementoLexico( k, l, c )
        {}
};


// ---------------------------------------------------------- Valores literales
class Literal : public ElementoLexico {
public:
    Literal(const std::string &n, size_t l, size_t c)
        : ElementoLexico( n, l, c )
        {}
};

class Cadena : public Literal {
public:
    Cadena(const std::string &cadena, size_t l, size_t c)
        : Literal( cadena, l, c )
        {}

};

class Numero : public Literal {
public:
    Numero(const std::string &n, size_t l, size_t c)
        : Literal( n, l, c )
        {}
};

class NumeroEntero : public Numero {
public:
    NumeroEntero(const std::string &n, size_t l, size_t c)
        : Numero( n, l, c )
        {}
};

class NumeroReal : public Numero {
public:
    NumeroReal(const std::string &n, size_t l, size_t c)
        : Numero( n, l, c )
        {}
};



#endif // ELEMENTO_H

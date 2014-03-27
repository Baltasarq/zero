#ifndef ELEMENTO_H
#define ELEMENTO_H

#include <string>
#include <vector>

#include "excep.h"

class ElementoLexico {
public:
    ElementoLexico(const std::string &e, unsigned int l, unsigned int c)
        : elemento( e ), linea( l ), columna( c )
        {}
    virtual ~ElementoLexico()
        {}

    unsigned int getLinea() const
        { return linea; }
    unsigned int getColumna() const
        { return columna; }
    const std::string &getElemento() const
        { return elemento; }

private:
    std::string elemento;
    unsigned int linea;
    unsigned int columna;
};

class NoIdentificado : public ElementoLexico {
public:
    NoIdentificado(unsigned int l, unsigned int c)
        : ElementoLexico( "", l, c )
        {}
};

// ------------------------------------------------------------ Identificadores
class Identificador : public ElementoLexico {
public:
    Identificador(const std::string &id, unsigned int l, unsigned int c)
        : ElementoLexico( id, l, c )
        {}
};

class PalabraReservada : public ElementoLexico {
public:
    PalabraReservada(const std::string &k, unsigned int l, unsigned int c)
        : ElementoLexico( k, l, c )
        {}
};


// ---------------------------------------------------------- Valores literales
class Literal : public ElementoLexico {
public:
    Literal(const std::string &n, unsigned int l, unsigned int c)
        : ElementoLexico( n, l, c )
        {}
};

class Cadena : public Literal {
public:
    Cadena(const std::string &cadena, unsigned int l, unsigned int c)
        : Literal( cadena, l, c )
        {}

};

class Numero : public Literal {
public:
    Numero(const std::string &n, unsigned int l, unsigned int c)
        : Literal( n, l, c )
        {}
};

class NumeroEntero : public Numero {
public:
    NumeroEntero(const std::string &n, unsigned int l, unsigned int c)
        : Numero( n, l, c )
        {}
};

class NumeroReal : public Numero {
public:
    NumeroReal(const std::string &n, unsigned int l, unsigned int c)
        : Numero( n, l, c )
        {}
};



#endif // ELEMENTO_H

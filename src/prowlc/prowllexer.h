#ifndef PROWLLEXER_H
#define PROWLLEXER_H

#include "elemento.h"
#include "lexer.h"
#include "excep.h"

class ProwlLexer : public GeneralLexer {
public:
    static const std::string PalabrasReservadas;
    static const std::string Operadores;
    static const std::string Begin;
    static const std::string ReservadaObject;
    static const std::string ReservadaObj;
    static const std::string ReservadaMethod;
    static const std::string ReservadaMth;
    static const std::string ReservadaAttribute;
    static const std::string ReservadaAttr;
    static const std::string ReservadaEndObject;
    static const std::string ReservadaEndObj;
    static const std::string ReservadaReference;
    static const std::string ReservadaRef;
    static const std::string ReservadaRet;
    static const std::string ReservadaIf;
    static const std::string ReservadaWhile;
    static const std::string ReservadaGoto;
    static const std::string MarcaEtiqueta;
    static const std::string OperadorHerencia;
    static const std::string OperadorVisibilidadPrivada;
    static const std::string OperadorVisibilidadPublica;
    static const std::string OperadorFinInstruccion;
    static const std::string OperadorEnumeracion;
    static const std::string OperadorIniParams;
    static const std::string OperadorFinParams;
    static const std::string OperadorAcceso;
    static const std::string OperadorAsigna;
    static const std::string OperadorSeparadorInstr;
    static const std::string DelimitadorTexto;
    static const std::string End;

    static const bool ClaveLeida = true;
    static const bool ClaveNoLeida = false;

    ProwlLexer(const std::string &fn)
        : GeneralLexer( fn, PalabrasReservadas )
        {}

    std::auto_ptr<ElementoLexico> getSigElemento();

    /// Pasa el fin de instr (';') o lanza excp.
    void pasarFinInstr();
};

// -------------------------------------------------------------------- Bloques
class Control : public ElementoLexico {
public:
    Control(const std::string &ctrl, unsigned int l, unsigned int c)
        : ElementoLexico( ctrl, l, c )
        {}
};

class AperturaDeBloque : public Control {
public:
    AperturaDeBloque(unsigned int l, unsigned int c)
        : Control( ProwlLexer::Begin, l, c )
        {}
};

class CierreDeBloque : public Control {
public:
    CierreDeBloque(unsigned int l, unsigned int c)
        : Control( ProwlLexer::Begin, l, c )
        {}
};

// ----------------------------------------------------------------- Operadores
class Operador : public ElementoLexico {
public:
    Operador(const std::string &ctrl, unsigned int l, unsigned int c)
        : ElementoLexico( ctrl, l, c )
        {}
};

#endif // PROWLLEXER_H

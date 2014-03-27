// generadorcodigozero.h
/*
    Declara la clase que genera cod. para Zero
*/

#ifndef GENERADORCODIGOZERO_H
#define GENERADORCODIGOZERO_H

#include "generadorcodigo.h"

#include <string>

class Obj;
class Mth;

class GeneradorCodigoZero : public GeneradorCodigo {
public:
    GeneradorCodigoZero(const std::string &f, const AST * r) : GeneradorCodigo( f, r )
        {}

    void generarCodigo();

    std::string generarCodigoMiembros(const Obj * obj);
    std::string generarCodigoMetodo(const Mth * mth);
};

#endif // GENERADORCODIGOZERO_H

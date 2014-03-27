// generadorcodigo.h
/*
    Clase padre de todos los generadores de código.
*/

#ifndef GENERADORCODIGO_H
#define GENERADORCODIGO_H

#include <string>
#include <memory>

class AST;
class OutputFile;

class GeneradorCodigo {
public:
    GeneradorCodigo(const std::string &f, const AST *r);

    virtual ~GeneradorCodigo()
        {}

    const AST * getRaiz() const
        { return raiz; }

    OutputFile & getFichero()
        { return *out; }

    const std::string &getNombreFichero() const;

    virtual void generarCodigo() = 0;

protected:
    std::string buffer;
private:
    const AST * raiz;
    std::auto_ptr<OutputFile> out;
};

#endif // GENERADORCODIGO_H

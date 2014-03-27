#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <cstdio>
#include <memory>

#include "fileio.h"
#include "lex.h"

#include "excep.h"
#include "ast.h"
#include "lexer.h"

class Parser {
public:
    Parser(const std::string &n) throw( Zero::EInputOutput );
    virtual ~Parser()
        {}

    virtual void compilar() throw(Zero::Excepcion) = 0;
    virtual void chk() throw(Zero::Excepcion) = 0;
    virtual void transformar() = 0;

    const std::string &getNombreArchivoEntrada() const
        { return flex->getNombreArchivo(); }

    unsigned int getNumeroLinea() const
        { return flex->getNumLinea(); }

    unsigned int getPosLinea() const
        { return flex->getPosicion(); }

    std::string getLineaActual() const
        { return flex->getLineaActual(); }

    const std::string &getNombreModulo() const
        { return nombreModulo; }

    bool fueCompilacionCorrecta() const
        { return success; }

    void ponCompilacionCorrecta(bool correcta = true)
        { success = correcta; }

    unsigned int getNumeroDeObjetos() const
        { return ast->objetos.size(); }

    const AST * getAST() const
        { return ast.get(); }

    AST * getAST()
        { return ast.get(); }

    virtual const Obj * getObjPpal() const = 0;

private:
    bool success;

protected:
    std::string nombreModulo;
    std::auto_ptr<GeneralLexer> flex;
    std::auto_ptr<AST> ast;
};

#endif // PARSER_H

#ifndef PARSERPROWL_H
#define PARSERPROWL_H

#include "ast.h"
#include "parser.h"
#include "prowllexer.h"

#include <stack>

class ParserProwl : public Parser {
public:
    struct Bloque {
        enum TipoBloque{ If, While, For };
        static const std::string EtqTipoBloque[];

        TipoBloque tipoBloque;
        Instr * instr;
    };

    enum Status {
            StTopLevel, StInObject, StInMethod
    };

    ParserProwl(const std::string &fn) throw( Zero::EInputOutput );
    ~ParserProwl();

    Status getStatus() const
        { return status; }

    void compilar() throw(Zero::Excepcion);
    void chk() throw(Zero::Excepcion);
    void localizarObjPpal();
    void transformar();

    const Obj * getObjPpal() const
        { return objPpal; }

protected:
    Status status;

    Obj * objPpal;
    Obj * objetoActual;
    Mth * metodoActual;

    std::stack<Bloque> bloqueActual;

    void leerFinDeclObjeto();
    Obj * leerDeclObjeto();
    Mth * leerDeclMetodo();

    std::string buscarObj();
    std::string buscarMthAttr();

    void crearAst() throw(Zero::Excepcion);
    void crearAstObj() throw(Zero::Excepcion);
    void crearAstMth() throw(Zero::Excepcion);
    void crearAstAtr() throw(Zero::ESintaxis);
    void crearAstInstr() throw(Zero::ESintaxis);
    Expr * crearAstSubMsg() throw(Zero::ESintaxis);
    Expr * crearAstMsg(const std::string &refInit = "") throw(Zero::ESintaxis);

    /// Comprueba si lo siguiente es un literal, id, ref o mensaje, y lo crea.
    Expr * crearAstExpr() throw(Zero::ESintaxis);

    /// Lee una expr. condicional
    Expr * crearAstExprCond() throw(Zero::ESintaxis);
    Ret * crearAstRet() throw(Zero::ESintaxis);
    If * crearAstIf() throw(Zero::ESintaxis);
    const ListaAst<Expr> &crearAstParams(Msg *) throw(Zero::ESintaxis);
    Lit * crearAstLit() throw(Zero::ESintaxis);
    Asg * crearAstAsg(const std::string &id) throw(Zero::ESintaxis);
    Def * crearAstRef() throw(Zero::ESintaxis);

    void lanzaBugChar();
    bool esFinObj();
    bool esFinObj(const std::string &);
    void pasarFinInstr()
        { (( ProwlLexer *) flex.get() )->pasarFinInstr(); }
};

#endif // PARSERPROWL_H

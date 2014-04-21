#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <vector>
#include <string>

#include "ast.h"

/**
    Usada en las transformaciones, lleva la cuenta de los registros ya empleados,
    y por tanto, cuándo es necesario crear una ref local.
*/
class PilaLoc {
public:
    static const std::string PrefijoLoc;

    PilaLoc(Mth * = NULL);

    const std::string getSigLoc(size_t &);

    void reiniciar()
        { numLocReg = 0; }

    void setMetodo(Mth * mth);

private:
    Mth * mth;
    size_t numLoc;
    size_t numLocReg;
};

/**
    Clase base para todas las transformaciones que se le quieran
    realizar al AST.
    Habrá transformaciones a nivel de objeto, met. e instr.
*/
class Transform {
public:
    Transform(AST * arbol, PilaLoc * pl) : ast( arbol ), pila( pl )
        {}
    virtual ~Transform()
        {}

    const AST * getAST() const
        { return ast; }

    /**
        Hace la transf necesaria, a nivel de instr, mth u obj
        @param ord El orden en el que se encuentra el elemento actual
        @param elem El elemento a considerar
        @param env El elemento en el que se encuentra incluido elem
        @return true si es necesario recomenzar la transf. debido a cambios, false en otro caso
    */
    virtual bool doIt(size_t &ord, ElementoAst * elem, ElementoAst * env) = 0;

    static std::string simplificaRef(size_t &ord, const Ref * ref, Mth * mth, PilaLoc *);
protected:
    AST * ast;
    PilaLoc * pila;
};

/**
    Transforma los literales en las intrucciones de un met.
    Es necesario insertar opcodes que creen esos literales.
    Nivel: instr.
*/
class TransformLits : public Transform {
public:
    TransformLits(AST * ast, PilaLoc * pl) : Transform( ast, pl )
        {}

    bool doIt(size_t &ord, ElementoAst * elem, ElementoAst * env);
};

/**
    Transforma los params. de un met., simplificandolos.
    Es necesario insertar SET para ellos.
    Nivel: instr.
*/
class TransformSimplificaParams : public Transform {
public:
    TransformSimplificaParams(AST * ast, PilaLoc * pl) : Transform( ast, pl )
        {}

    bool doIt(size_t &ord, ElementoAst * elem, ElementoAst * env);
};

/**
    Transforma los opcodes con Refs, divide entre opcode y MSG
    Nivel: instr.
*/
class TransformConRef : public Transform {
public:
    TransformConRef(AST * ast, PilaLoc * pl) : Transform( ast, pl )
        {}

    bool doIt(size_t &ord, ElementoAst * elem, ElementoAst * env);
};

/**
    Coloca todos los opcodes DEF al comienzo.
    Nivel: met.
*/
class TransformDefsComienzo : public Transform {
public:
    TransformDefsComienzo(AST * ast, PilaLoc * pl) : Transform( ast, pl )
        {}

    bool doIt(size_t &ord, ElementoAst * elem, ElementoAst * env);
};

/**
    Sustituye los id's this por __this, el reg real
    Nivel: instr
*/
class TransformThis : public Transform {
public:
    TransformThis(AST * ast, PilaLoc * pl) : Transform( ast, pl )
        {}

    bool doIt(size_t &ord, ElementoAst * elem, ElementoAst * env);
};

/**
    Encargado de recorrer el AST y disparar las
    transformaciones en cada nivel.
*/
class GestorTransformaciones {
public:
    GestorTransformaciones(AST *ast);
    ~GestorTransformaciones();

    void doIt();

    const AST * getAST() const
        { return ast; }

protected:
    void liberar(std::vector<Transform *> &);

    /**
        Aplica las transformaciones a nivel de instr, mth u obj
        @param ord El orden del elemento a considerar
        @param elem El elemento a considerar
        @param env El elemento donde se encuentra elem
        @param lst La lista de transf. a aplicar
        @return true si es necesario recomenzar por cambios, false en otro caso.
    */
    bool aplicaTransformaciones(size_t &ord, ElementoAst *elem, ElementoAst *env, const std::vector<Transform *> &lst);

    Obj * objetoActual;
    Mth * metodoActual;
    std::auto_ptr<PilaLoc> pila;

private:
    std::vector<Transform *> transfsInstr;
    std::vector<Transform *> transfsMth;
    std::vector<Transform *> transfsObj;
    AST * ast;
};

#endif // TRANSFORM_H

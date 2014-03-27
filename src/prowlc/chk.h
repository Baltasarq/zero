#ifndef CHK_H
#define CHK_H

#include <vector>

#include "excep.h"

class ElementoAst;
class Obj;
class Parser;
class Atr;
class Mth;
class Instr;

class Chk {
public:
    Chk(Parser * p) : parser( p )
        {}
    virtual ~Chk()
        {}

    virtual void doIt(const ElementoAst * elem, const ElementoAst * env) throw(Zero::Excepcion) = 0;

    virtual const Parser * getParser() const
        { return parser; }

    static std::string crearPath(const std::string &id, const Mth *, const Obj *);

private:
    Parser * parser;
};

class ChkAtrs : public Chk {
public:
    ChkAtrs(Parser * p) : Chk( p )
        {}

    void doIt(const ElementoAst * elem, const ElementoAst * env) throw(Zero::Excepcion);
};

class ChkRets : public Chk {
public:
    ChkRets(Parser * p) : Chk( p )
        {}

    void doIt(const ElementoAst * elem, const ElementoAst * env) throw(Zero::Excepcion);
};

class ChkMsgArgs : public Chk {
public:
    ChkMsgArgs(Parser * p) : Chk( p )
        {}

    void doIt(const ElementoAst * elem, const ElementoAst * env) throw(Zero::Excepcion);
};

class ChkDefs: public Chk {
public:
    ChkDefs(Parser * p) : Chk( p )
        {}

    void doIt(const ElementoAst * elem, const ElementoAst * env) throw(Zero::Excepcion);
};

class ChkJmps: public Chk {
public:
    ChkJmps(Parser * p) : Chk( p )
        {}

    void doIt(const ElementoAst * elem, const ElementoAst * env) throw(Zero::Excepcion);
};

class GestorChk {
public:
    GestorChk(Parser *p);
    ~GestorChk();

    void doIt() throw( Zero::Excepcion );
    void chk(const ElementoAst * elem, const ElementoAst * env, const std::vector<Chk *> &chks) throw( Zero::Excepcion );

    const Parser * getParser() const
        { return parser; }

protected:
    void liberar(std::vector<Chk *> &listaChks);

private:
    Parser * parser;
    std::vector<Chk *> chksObj;
    std::vector<Chk *> chksAtr;
    std::vector<Chk *> chksMth;
    std::vector<Chk *> chksInstr;
};


#endif // CHK_H

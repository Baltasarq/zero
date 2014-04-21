// ast.h
/*
    Arbol abstracto de sintaxis
*/

#ifndef AST_H_INCLUDED
#define AST_H_INCLUDED

#include <string>
#include <vector>
#include <memory>

#include "mnemos.h"

class ElementoAst {
public:
    ElementoAst(const std::string &n, size_t l, size_t c)
        : marca( MarcaVivo ), numLinea( l ), numCol( c )
        { nombre.reset( new Zero::NombreIdentificador( n ) ); }

    virtual ~ElementoAst()
        { marca = MarcaMuerto; }

    const std::string &getNombre() const
        { return nombre->getNombre(); }

    size_t getNumLinea() const
        { return numLinea; }

    size_t getNumColumna() const
        { return numCol; }

    virtual std::string generarCodigo() const = 0;

protected:
    static const size_t MarcaVivo = 0xF00DBEEF;
    static const size_t MarcaMuerto = 0xDEADBEEF;
private:
    size_t marca;
    std::auto_ptr<Zero::NombreIdentificador> nombre;
    size_t numLinea;
    size_t numCol;
};

template<typename T>
class ListaAst {
public:
    typedef std::vector<T *> ListaElementos;

    ListaAst()
        {}

    virtual ~ListaAst()
    {
        for(size_t i = 0; i < size(); ++i) {
            delete (*this)[ i ];
        }

        elementos.clear();
    }

    size_t size() const
        { return elementos.size(); }

    T * operator[](size_t i)
        { return elementos[ i ]; }

    const T * operator[](size_t i) const
        { return elementos[ i ]; }

    void insertar(T *e)
        { elementos.push_back( e ); }

    void insertar(size_t i, T *e)
        { elementos.insert( elementos.begin() + i, e ); }

    void modificar(size_t i, T *e)
        { delete elementos[ i ]; elementos[ i ] = e; }

    T * extraer(size_t i)
        { T * toret = elementos[ i ]; elementos[ i ] = NULL; return toret; }

    void mover(size_t org, size_t dest)
        {
            if ( org != dest ) {
                T * aux = elementos[ org ];
                elementos.erase( elementos.begin() + org );
                this->insertar( dest, aux );
            }
        }

private:
    ListaElementos elementos;
};

class Obj;

class AST {
public:
    ListaAst<Obj> objetos;

    AST(const std::string &n)
        { nombre.reset( new Zero::NombreIdentificador( n ) ); }

    const std::string &getNombre() const
        { return nombre->getNombre(); }

private:
    std::auto_ptr<Zero::NombreIdentificador> nombre;
};

class Mth;
class Atr;

class Obj : public ElementoAst {
public:
    Obj(const std::string &no, const std::string &nop, size_t l, size_t c, bool k)
        : ElementoAst( no, l, c ), llave( k )
        { nombreObjetoPadre.reset( new Zero::NombreIdentificador( nop ) ); }

    const std::string &getNombreObjetoPadre() const
        { return nombreObjetoPadre->getNombre(); }

    bool declaradoConLlave()
        { return llave; }

    std::string generarCodigo() const;

    ListaAst<Mth> metodos;
    ListaAst<Atr> atributos;

private:
    std::auto_ptr<Zero::NombreIdentificador> nombreObjetoPadre;
    bool llave;
};

class Instr;

class Mth : public ElementoAst {
public:
    Mth(const std::string &n, bool v, size_t l, size_t c)
        : ElementoAst( n, l, c ), visibility( v )
        {}

    bool getVisibility() const
        { return visibility; }

    const std::vector<std::string> &getParams() const
        { return params; }

    void setParams(const std::vector<std::string> pars);

    std::string generarCodigo() const;

    ListaAst<Instr> instrucciones;

    bool esLocal(const std::string &id) const;
    bool esDefLocal(const std::string &id) const;
    bool esParametro(const std::string &id) const;
private:
    std::vector<std::string> params;
    bool visibility;
};

class Instr : public ElementoAst {
public:
    static const std::string NombreInstr;

    Instr(size_t l, size_t c) : ElementoAst( NombreInstr, l, c )
        {}
protected:
    Instr(const std::string &n, size_t l, size_t c)
        : ElementoAst( n, l, c )
        {}
};

class Jmp : public Instr {
public:
    static const std::string NombreJmp;

    Jmp(const std::string &id, size_t l, size_t c) : Instr( NombreJmp, l, c )
        { this->id.reset( new Zero::NombreIdentificador( id ) ); }

    const std::string &getId() const
        { return id->getNombre(); }

    std::string generarCodigo() const;

private:
    std::auto_ptr<Zero::NombreIdentificador> id;
};

class Etq : public Instr {
public:
    static const std::string NombreEtq;

    Etq(const std::string &id, size_t l, size_t c) : Instr( NombreEtq, l, c )
        { this->id.reset( new Zero::NombreIdentificador( id ) ); }

    const std::string &getId() const
        { return id->getNombre(); }

    std::string generarCodigo() const;

private:
    std::auto_ptr<Zero::NombreIdentificador> id;
};

class Lit;

class CreacionLit : public Instr {
public:
    static const std::string NombreCreaLit;

    CreacionLit(const Lit * lit);

    virtual const std::string &getLiteral() const
        { return literal; }
protected:
    CreacionLit(const std::string &nombre, const Lit * lit);

private:
    std::string literal;
};

class LitNumero;

class CreacionLitNumero : public CreacionLit {
public:
    CreacionLitNumero(const LitNumero * lit)
        : CreacionLit( (const Lit *) lit )
        {}

    std::string generarCodigo();

protected:
    CreacionLitNumero(const std::string nombre, const LitNumero *lit)
        : CreacionLit( nombre, (const Lit *) lit )
        {}
};

class CreacionLitNumeroEntero : public CreacionLitNumero {
public:
    static const std::string NombreCreaNumEntero;

    CreacionLitNumeroEntero(const LitNumero *lit) : CreacionLitNumero( NombreCreaNumEntero, lit )
        {}

    std::string generarCodigo() const;
};

class CreacionLitNumeroFlotante : public CreacionLitNumero {
public:
    static const std::string NombreCreaNumFlotante;

    CreacionLitNumeroFlotante(const LitNumero *lit) : CreacionLitNumero( NombreCreaNumFlotante, lit )
        {}

    std::string generarCodigo() const;
};

class LitTexto;

class CreacionLitTexto : public CreacionLit {
public:
    static const std::string NombreCreaTexto;

    CreacionLitTexto(const LitTexto * lit) : CreacionLit( NombreCreaTexto, (Lit *) lit )
        {}

    std::string generarCodigo() const;
};

class Expr : public Instr {
public:
    static const std::string NombreExpr;

    Expr(size_t l, size_t c) : Instr( NombreExpr, l, c )
        { numExpr = ++NumExpresiones; }
protected:
    Expr(const std::string &n, size_t l, size_t c)
        : Instr( n, l, c )
        {}

private:
    size_t numExpr;
    static size_t NumExpresiones;
};

class MixinRef {
public:
    const Expr * getExpr() const
        { return expr.get(); }

    Expr * getExpr()
        { return expr.get(); }

    Expr * extractExpr()
        { Expr * toret = expr.get(); expr.release(); return toret; }

    void setExpr(Expr *expr)
        { this->expr.reset( expr );}
private:
    std::auto_ptr<Expr> expr;
};

class Msg : public Expr, public MixinRef {
public:
    static const std::string NombreMsg;

    Msg(size_t l, size_t c) : Expr( NombreMsg, l, c )
        {}

    void setMsgInfo(const std::vector<std::string> &vIds);

    std::string generarCodigo() const;

    void setRef(const std::string &r)
        { ref.reset( Zero::Nombre::creaNombreAdecuado( r ) ); }
    void setNombreMth(const std::string &mth)
        { nombreMth = mth; }
    const std::string &getRef() const
        { return ref->getNombre(); }

    const std::string &getNombreMth() const
        { return nombreMth; }

    ListaAst<Expr> parametros;
private:
    std::auto_ptr<Zero::Nombre> ref;
    std::string nombreMth;
};

class Ref : public Expr {
public:
    static const std::string NombreRef;

    Ref(const std::string &l, size_t nl, size_t p);
    Ref(const std::vector<std::string> &l, size_t nl, size_t p);

    const std::string &getRef() const
        { return ref->getNombre(); }

    static std::string toString(const std::vector<std::string> &ids);

    std::string generarCodigo() const
        { return getRef(); }
protected:
    std::auto_ptr<Zero::Nombre> ref;
};

class Id : public Ref {
public:
    Id(const std::string &l, size_t nl = 0, size_t p = 0);

    const std::string &getId() const
        { return getRef(); }

    std::string generarCodigo() const
        { return getId(); }

    static void chkUsrId(const std::string &) throw(Zero::ESintaxis);
};

class Reg : public Id {
public:
    enum GpReg { Gp1, Gp2, Gp3, Gp4 };
    static const size_t NumGpRegs;
    static const std::string * NombreGpReg[];
    static const std::string This;

    static const std::string &getAcc();
    static const std::string &getThis();
    static const std::string &getGp1();
    static const std::string &getGp2();
    static const std::string &getGp3();
    static const std::string &getGp4();
    static const std::string &getGp(GpReg reg);

    Reg(const std::string &id) : Id( id, 0, 0 )
        {}

    static bool esReg(const std::string &);
};

class Ret : public Instr, public MixinRef {
public:
    Ret(size_t l, size_t p);

    std::string generarCodigo() const;
};

class If : public Instr, public MixinRef {
public:
    static const std::string NombreIf;

    If(size_t l, size_t p);

    std::string generarCodigo() const;
private:
    size_t numIf;
    std::string etqBase;

    static size_t NumIfs;
};

class Asg : public Instr, public MixinRef {
public:
    static const std::string NombreAsg;

    Asg(const std::string &n, size_t l, size_t c) : Instr( NombreAsg, l, c )
        { id.reset( new Zero::NombreIdentificador( n ) );
          setExpr( new Reg( Reg::getAcc() ) );
        }

    Asg(const std::string &n) : Instr( NombreAsg, 0, 0 )
        { id.reset( new Zero::NombreIdentificador( n ) );
          setExpr( new Reg( Reg::getAcc() ) );
        }

    std::string generarCodigo() const;

    const std::string &getId() const
        { return id->getNombre(); }

    void setId(const std::string &);

private:
    std::auto_ptr<Zero::NombreIdentificador> id;
};

class Set : public Instr, public MixinRef {
public:
    static const std::string NombreSet;

    Set(const std::string &n)
        : Instr( NombreSet, 0, 0 )
        { id.reset( new Zero::NombreIdentificador( n ) );
          reg.reset( new Zero::NombreIdentificador( Reg::getAcc() ) );
        }

    Set(const std::string &n, const std::string &r)
        : Instr( 0, 0 )
        { id.reset( new Zero::NombreIdentificador( n ) );
          reg.reset( new Zero::NombreIdentificador( r ) );
        }

    std::string generarCodigo() const;

    const std::string &getId() const
        { return id->getNombre(); }
    const std::string &getReg() const
        { return reg->getNombre(); }

    void setId(const std::string &);
    void setReg(const std::string &);

private:
    std::auto_ptr<Zero::NombreIdentificador> id;
    std::auto_ptr<Zero::NombreIdentificador> reg;
};

class Lit : public Expr {
public:
    static const std::string NombreLit;

    Lit(const std::string &l, size_t nl, size_t p) : Expr( NombreLit, nl, p ), literal( l )
        {}
    const std::string &getLiteral() const
        { return literal; }

    std::string generarCodigo() const
        { return getLiteral(); }

    virtual CreacionLit * crearLiteral() const = 0;
private:
    std::string literal;
};

class LitNumero : public Lit {
public:
    LitNumero(const std::string &n, size_t nl, size_t p) : Lit( n, nl, p )
        {}

    CreacionLit * crearLiteral() const;
};

class LitTexto : public Lit {
public:
    LitTexto(const std::string &t, size_t nl, size_t p) : Lit( t, nl, p )
        {}
    std::string generarCodigo() const
        { return ( std::string( "\"" ) + getLiteral() + "\"" ); }

    CreacionLit * crearLiteral() const;
};

class Def : public Instr, public MixinRef {
public:
    static const std::string NombreDef;

    Def(const std::string &id, size_t nl, size_t nc);

    const std::string &getId() const
        { return id; }
    void setId(const std::string &id);

    std::string generarCodigo() const;
private:
    std::string id;
};

class DefInterno : public Def {
public:
    DefInterno(const std::string &id) : Def( id, 0, 0 )
        {}
};

class DefUsr : public Def {
public:
    DefUsr(const std::string &id, size_t nl, unsigned nc) : Def( id, nl, nc )
        { Id::chkUsrId( id ); }
};

class Atr : public ElementoAst, public MixinRef {
public:
    const static bool Public = true;
    const static bool Private = false;

    Atr(const std::string &n, bool v, size_t l, size_t c)
        : ElementoAst( n, l, c ), visibility( v )
        {}

    bool getVisibility() const
        { return visibility; }

    std::string generarCodigo() const;

private:
    bool visibility;
};

#endif // AST_H_INCLUDED

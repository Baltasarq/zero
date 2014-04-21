// ast.cpp

#include "ast.h"
#include "prowllexer.h"

#include <string>

// ------------------------------------------------------------------------ Obj
std::string Obj::generarCodigo() const
{
    std::string toret = Zero::NMObj::mnemoStr;

    toret.push_back( ' ' );
    toret.append( getNombre() );

    if ( !getNombreObjetoPadre().empty() ) {
        toret += ' ';
        toret += getNombreObjetoPadre();
    }

    return toret;
}

// ------------------------------------------------------------------------ Atr
std::string Atr::generarCodigo() const
{
    std::string toret = Zero::NMAtr::mnemoStr;

    toret.push_back( ' ' );

    // Visibilidad
    if ( this->getVisibility() )
            toret.append( Zero::NMAtr::PUBLICO );
    else    toret.append( Zero::NMAtr::PRIVADO );

    // Nombre
    toret.push_back( ' ' );
    toret.append( getNombre() );
    toret.push_back( ' ' );

    if ( getExpr() != NULL )
            toret.append( getExpr()->generarCodigo() );
    else    toret.append( Zero::OBJ_NOTHING );

    toret.push_back( '\n' );
    return toret;
}

// ------------------------------------------------------------------------ Mth
bool Mth::esLocal(const std::string &id) const
{
    return ( Reg::esReg( id ) || esDefLocal( id ) || esParametro( id ) );
}

bool Mth::esParametro(const std::string &id) const
{
    size_t i = 0;
    const std::vector<std::string> &parametros = getParams();
    const size_t NumParametros = parametros.size();

    for(; i < NumParametros; ++i) {
        if ( parametros[ i ] == id ) {
            break;
        }
    }

    return ( i < NumParametros );
}

bool Mth::esDefLocal(const std::string &id) const
{
    size_t i = 0;
    const size_t NumInstrucciones = instrucciones.size();

    for(; i < NumInstrucciones; ++i) {
        if ( dynamic_cast<const Def *>( instrucciones[ i ] ) != NULL ) {
            const Def * def = (const Def *) instrucciones[ i ];

            if ( def->getId() == id ) {
                break;
            }
        }
    }

    return ( i < NumInstrucciones );
}

std::string Mth::generarCodigo() const
{
    std::string toret;

    // Declara
    toret.append( Zero::NMMth::mnemoStr );
    toret.push_back( ' ' );
    toret.append( getVisibility() ? Zero::NMMth::PUBLICO : Zero::NMMth::PRIVADO );
    toret.push_back( ' ' );
    toret.append( getNombre() );
    ;

    // Params
    if ( getParams().size() > 0 ) {
        toret.push_back( ' ' );

        for(size_t i = 0; i < getParams().size(); ++i) {
            toret.append( getParams()[ i ] );
            toret.push_back( ' ' );
        }
    }

    toret.push_back( '\n' );

    return toret;
}

void Mth::setParams(const std::vector<std::string> pars)
{
    params.clear();
    params.insert( params.begin(), pars.begin(), pars.end() );

    std::vector<std::string>::const_iterator it = pars.begin();
    for(; it != pars.end(); ++it) {
        Zero::NombreIdentificador::chkId( *it );
    }
}

// ---------------------------------------------------------------------- Instr
const std::string Instr::NombreInstr = "instr";

// ------------------------------------------------------------------ LitNumero
const std::string Lit::NombreLit = "lit";

CreacionLit * LitNumero::crearLiteral() const
{
    if ( getLiteral().find( '.' ) != std::string::npos )
            return new CreacionLitNumeroFlotante( this );
    else    return new CreacionLitNumeroEntero( this );
}

// ------------------------------------------------------------------- LitTexto
CreacionLit * LitTexto::crearLiteral() const
{
    return new CreacionLitTexto( this );
}

// ------------------------------------------------------------------------ Ref
const std::string Ref::NombreRef = "ref";

Ref::Ref(const std::string &l, size_t nl, size_t p)
    : Expr( NombreRef, nl, p )
{
    ref.reset( new Zero::NombreReferencia( l ) );
}

Ref::Ref(const std::vector<std::string> &refs, size_t nl, size_t p)
    : Expr( NombreRef, nl, p )
{
    ref.reset( new Zero::NombreReferencia( Ref::toString( refs ) ) );
}

std::string Ref::toString(const std::vector<std::string> &vIds)
{
    std::string ref;
    const size_t numIds = vIds.size();

    if ( numIds > 0 ) {
        ref.reserve( numIds * 10 );

        // Recuperar la referencia
        for(size_t i = 0; i < numIds; ++i) {
            ref.append( vIds[ i ] );

            if ( i < ( numIds -1 ) ) {
                ref.append( ProwlLexer::OperadorAcceso );
            }
        }

    } else throw Zero::ESintaxis( "referencia incorrecta" );

    return ref;
}

// ------------------------------------------------------------------------ Jmp
const std::string Jmp::NombreJmp = "jmp";

std::string Jmp::generarCodigo() const
{
    std::string toret;

    // registro dest
    toret.append( Zero::NMJmp::mnemoStr );
    toret.push_back( ' ' );
    toret.append( getId() );

    return toret;
}

// ------------------------------------------------------------------------ Etq
const std::string Etq::NombreEtq = "etq";

std::string Etq::generarCodigo() const
{
    std::string toret;

    // registro dest
    toret.append( Zero::NMEtq::mnemoStr );
    toret.push_back( ' ' );
    toret.append( getId() );

    return toret;
}

// ----------------------------------------------------------------------- Expr
const std::string Expr::NombreExpr = "expr";
size_t Expr::NumExpresiones = 0;

// ------------------------------------------------------------------------ Msg
const std::string Msg::NombreMsg = "msg";

void Msg::setMsgInfo(const std::vector<std::string> &vIds)
{
    size_t numIds = vIds.size() -1;

    if ( numIds > 0 ) {
        // Tomar la referencia (menos id final nombre mth)
        std::string strRef = Ref::toString( std::vector<std::string>( vIds.begin(), --( vIds.end() ) ) );
        ref.reset( Zero::Nombre::creaNombreAdecuado( strRef ) );

        // ID final es metodo
        setNombreMth( vIds[ numIds ] );
    } else throw Zero::ESintaxis( "referencia incorrecta" );
}

std::string Msg::generarCodigo() const
{
    std::string toret = Zero::NMMsg::mnemoStr;

    // Volcar referencia y nombre de mth
    toret.push_back( ' ' );
    toret.append( getRef() );
    toret.push_back( ' ' );
    toret.append( getNombreMth() );
    toret.push_back( ' ' );

    // Volcar los params
    for(size_t i = 0; i < parametros.size(); ++i) {
        toret.append( parametros[ i ]->generarCodigo() );
        toret.push_back( ' ' );
    }

    return toret;
}


// ------------------------------------------------------------------------ Ret
Ret::Ret(size_t l, size_t p)
    : Instr( ProwlLexer::ReservadaRet, l, p )
{
}

std::string Ret::generarCodigo() const
{
    std::string toret = Zero::NMRet::mnemoStr;

    if ( getExpr() != NULL ) {
        toret.push_back( ' ' );
        toret.append( getExpr()->generarCodigo() );
    }

    return toret;
}

// ------------------------------------------------------------------------ Asg
const std::string Asg::NombreAsg = "asg";

std::string Asg::generarCodigo() const
{
    std::string toret;
    std::string org = "";

    // registro org / expresion anterior
    if ( dynamic_cast<const Reg *>( getExpr() ) != NULL ) {
        if ( ( (Reg *) getExpr() )->getId() != Reg::getAcc() ) {
            org = getExpr()->generarCodigo();
        }
    } else  toret = getExpr()->generarCodigo() + '\n';

    // registro dest
    toret.append( Zero::NMAsg::mnemoStr );
    toret.push_back( ' ' );
    toret.append( this->getId() );

    // registro org
    if ( !org.empty() ) {
        toret.push_back( '\n' );
        toret.append( org );
    }

    return toret;
}

void Asg::setId(const std::string &id)
{
    this->id.reset( new Zero::NombreIdentificador( id ) );
}

// ------------------------------------------------------------------------ Set
const std::string Set::NombreSet = "set";

std::string Set::generarCodigo() const
{
    std::string toret;

    // registro dest
    toret.append( Zero::NMSet::mnemoStr );
    toret.push_back( ' ' );
    toret.append( getReg() );
    toret.push_back( ' ' );
    toret.append( getId() );

    return toret;
}

void Set::setId(const std::string &id)
{
    this->id.reset( new Zero::NombreIdentificador( id ) );
}

void Set::setReg(const std::string &id)
{
    if ( Reg::esReg( id ) )
            this->id.reset( new Zero::NombreIdentificador( id ) );
    else    throw Zero::ESintxIdNoValido( id.c_str() );
}

// ------------------------------------------------------------------------- Id
Id::Id(const std::string &l, size_t nl, size_t p) : Ref( l, nl, p )
{
    ref.reset( new Zero::NombreIdentificador( l ) );
}

void Id::chkUsrId(const std::string &id) throw(Zero::ESintaxis)
{
    Zero::NombreIdentificador::chkId( id );

    if ( id.length() > 2
      && id.substr( 0, 2 ) == "__" )
    {
        throw Zero::ESintxIdNoValido( id .c_str() );
    }
}


// ------------------------------------------------------------------------ Reg
const size_t Reg::NumGpRegs = 4;

/// This tal cual aparecera en el fuente
const std::string Reg::This = "this";

const std::string * Reg::NombreGpReg[] = {
    &Zero::LOC_GP1,
    &Zero::LOC_GP2,
    &Zero::LOC_GP3,
    &Zero::LOC_GP4,
};

const std::string &Reg::getAcc()
{
    return Zero::LOC_ACC;
}

/// El registro this a generar como ensambladorS
const std::string &Reg::getThis()
{
    return Zero::LOC_THIS;
}

const std::string &Reg::getGp1()
{
    return *Reg::NombreGpReg[ Reg::Gp1 ];
}

const std::string &Reg::getGp2()
{
    return *Reg::NombreGpReg[ Reg::Gp2 ];
}

const std::string &Reg::getGp3()
{
    return *Reg::NombreGpReg[ Reg::Gp3 ];
}

const std::string &Reg::getGp4()
{
    return *Reg::NombreGpReg[ Reg::Gp4 ];
}

const std::string &Reg::getGp(Reg::GpReg reg)
{
    return *Reg::NombreGpReg[ reg ];
}

bool Reg::esReg(const std::string &id)
{
    size_t i = 0;

    for(; i < NumGpRegs; ++i) {
        if ( *NombreGpReg[ i ] == id ) {
            break;
        }
    }

    return ( i < NumGpRegs );
}

// ---------------------------------------------------------------- CreacionLit
const std::string CreacionLit::NombreCreaLit = "Lit";

CreacionLit::CreacionLit(const Lit * lit)
    : Instr( NombreCreaLit, 0, 0 ), literal( lit->getLiteral() )
{
}

CreacionLit::CreacionLit(const std::string &nombre, const Lit * lit)
    : Instr( nombre, 0, 0 ), literal( lit->getLiteral() )
{
}

// ---------------------------------------------------- CreacionLitNumeroEntero
const std::string CreacionLitNumeroEntero::NombreCreaNumEntero = "int";

std::string CreacionLitNumeroEntero::generarCodigo() const
{
    std::string toret = Zero::NMInt::mnemoStr;

    toret.push_back( ' ' );
    toret.append( getLiteral() );

    return toret;
}

// -------------------------------------------------- CreacionLitNumeroFlotante
const std::string CreacionLitNumeroFlotante::NombreCreaNumFlotante = "flt";

std::string CreacionLitNumeroFlotante::generarCodigo() const
{
    std::string toret = Zero::NMFlt::mnemoStr;

    toret.push_back( ' ' );
    toret.append( getLiteral() );

    return toret;
}

// ----------------------------------------------------------- CreacionLitTexto
const std::string CreacionLitTexto::NombreCreaTexto = "str";

std::string CreacionLitTexto::generarCodigo() const
{
    std::string toret = Zero::NMStr::mnemoStr;

    toret.push_back( ' ' );
    toret.push_back( '"' );
    toret.append( getLiteral() );
    toret.push_back( '"' );

    return toret;
}

// ------------------------------------------------------------------------ Def
const std::string Def::NombreDef = "def";

Def::Def(const std::string &id, size_t l, size_t c) : Instr( NombreDef, l, c )
{
    setId( id );
}

void Def::setId(const std::string &id)
{
    Zero::NombreIdentificador::chkId( id );
    this->id = id;
}

std::string Def::generarCodigo() const
{
    std::string toret = Zero::NMDef::mnemoStr;

    toret.push_back( ' ' );
    toret.append( getId() );
    toret.push_back( '\n' );

    if ( getExpr() != NULL ) {
        toret.append( getExpr()->generarCodigo() );
        toret.append( Zero::NMAsg::mnemoStr );
        toret.push_back( ' ' );
        toret.append( getId() );
        toret.push_back( '\n' );
    }

    return toret;
}

// ------------------------------------------------------------------------- If
size_t If::NumIfs = 0;
const std::string NombreIf = "if";

If::If(size_t l, size_t p) : Instr( l, p )
{
    numIf = ++NumIfs;
    etqBase = "__prowlc_if_" + StringMan::toString( numIf );
}

std::string If::generarCodigo() const
{
    throw Zero::EInterno( "invocado if::generarCodigo" );
}

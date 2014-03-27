#include "parserprowl.h"
#include "prowllexer.h"
#include "reservadas.h"
#include "chk.h"
#include "transform.h"

//void informe(const std::auto_ptr<GeneralLexer> &flex)
//{
//    std::printf( "Ln:%d:'%s' (pos %d) ('%c')\n",
//                flex->getNumLinea(),
//                flex->getLineaActual().c_str(),
//                flex->getPosicion(),
//                flex->getCaracterActual()
//    );
//}

ParserProwl::ParserProwl(const std::string &fn) throw( Zero::EInputOutput )
    : Parser( fn ), status( StTopLevel )
{
    flex.reset( new ProwlLexer( fn ) );
}

ParserProwl::~ParserProwl()
{
}

const std::string ParserProwl::Bloque::EtqTipoBloque[] = { "if", "while", "for" };

void ParserProwl::compilar() throw(Zero::Excepcion)
{
    crearAst();
    localizarObjPpal();
    transformar();
    chk();
}

void ParserProwl::localizarObjPpal()
{
    Obj * obj;
    objPpal = NULL;

    for(unsigned int i = 0; i < getAST()->objetos.size(); ++i) {
        obj = getAST()->objetos[ i ];

        if ( obj->getNombre() == getAST()->getNombre() ) {
            objPpal = obj;
        }
    }

    return;
}

void ParserProwl::chk() throw(Zero::Excepcion)
{
    GestorChk gestor( this );

    gestor.doIt();
}

void ParserProwl::transformar()
{
    GestorTransformaciones transf( getAST() );

    transf.doIt();
}

void ParserProwl::crearAstMth() throw(Zero::Excepcion)
{
    // Chk
    if ( !bloqueActual.empty() ) {
        std::string msg;

        Bloque bloque = bloqueActual.top();
        msg = Bloque::EtqTipoBloque[ bloque.tipoBloque ];
        throw Zero::ESintxAnidamiento( ( msg + " anterior sin cerrar" ).c_str() );
    }

    // Crear mth en objeto
    objetoActual->metodos.insertar( metodoActual );

    // Leer las instrucciones mientras falte el cierre de mth }
    unsigned int oldPos = flex->getPosicion();
//printf( "crearAstMth:\n");
//informe( flex );
    while ( !flex->chkSig( ProwlLexer::End ) ) {
        flex->setPosicion( oldPos );
        crearAstInstr();

        oldPos = flex->getPosicion();
//printf( "crearAstMth:\n");
//informe( flex );

    }

    // Pasar la marca de fin de mth
    if ( flex->chkSig( ProwlLexer::End ) ) {
        // Chk
        if ( !bloqueActual.empty() ) {
            std::string msg;
            Bloque bloque;

            // Encontrar el bloque exterior sin cerrar, no el final
            while( !bloqueActual.empty() ) {
                bloque = bloqueActual.top();
                bloqueActual.pop();
            }

            msg = Bloque::EtqTipoBloque[ bloque.tipoBloque ] + " anterior sin cerrar";
            throw Zero::ESintxAnidamiento( msg.c_str() );
        }

        // Decidido, pasar la marca
        flex->avanzar();
    } else {
        throw Zero::ESintaxis( "esperando '}'" );
    }

    return;
}

void ParserProwl::crearAstInstr() throw(Zero::ESintaxis)
{
    unsigned int oldPos = flex->getPosicion();
    std::auto_ptr<ElementoLexico> elemento = flex->getSigElemento();
    Instr * instr = NULL;

//std::printf( "crearAstInstr:\n" );
//informe( flex );

    // ¿Se trata de un ref, if o while?
    if ( dynamic_cast<PalabraReservada *>( elemento.get() ) != NULL ) {
        if ( elemento->getElemento() == ProwlLexer::ReservadaRef
          || elemento->getElemento() == ProwlLexer::ReservadaReference )
        {
            instr = crearAstRef();
        }
        else
        if ( elemento->getElemento() == ProwlLexer::ReservadaGoto )
        {
            instr = new Jmp( flex->getIdentificador( GeneralLexer::Required ), flex->getNumLinea(), flex->getPosicion() );
            pasarFinInstr();
        }
        else
        if ( elemento->getElemento() == ProwlLexer::ReservadaIf )
        {
            instr = crearAstIf();
        }
        else
        if ( elemento->getElemento() == ProwlLexer::ReservadaWhile )
        {
            throw Zero::ESintaxis( "No implementado" );
        }
        else
        if ( elemento->getElemento() == ProwlLexer::ReservadaRet )
        {
            instr = crearAstRet();
        }
        else throw Zero::ESintaxis( "Instr debe ser mensaje, asig., if, while, for, goto o return." );
    }
    else {
        if ( flex->getCaracterActual() == ProwlLexer::MarcaEtiqueta[ 0 ] )
        {
            flex->avanzar();
            instr = new Etq( flex->getIdentificador( GeneralLexer::Required ), flex->getNumLinea(), flex->getPosicion() );
            pasarFinInstr();
        } else {
            // Determinar si es una asig. o mensaje
            flex->setPosicion( oldPos );
            std::string id = flex->getIdentificador( GeneralLexer::NotRequired );

            if ( flex->chkSig( ProwlLexer::OperadorAsigna ) ) {
                // Asg
                instr = crearAstAsg( id );
            } else {
                // Expr
                flex->setPosicion( oldPos );
                instr = crearAstMsg();
                pasarFinInstr();
            }
        }
    }

    // Insertar instr
    if ( instr != NULL ) {
        metodoActual->instrucciones.insertar( instr );
    }
    else throw Zero::ESintaxis( "Instr no reconocida" );

    return;
}

Def * ParserProwl::crearAstRef() throw(Zero::ESintaxis)
{
    std::string id = flex->getIdentificador( ProwlLexer::Required );
    Def * toret = new DefUsr( id, flex->getNumLinea(), flex->getPosicion() );

    // Pasar signo igual, si existe
    flex->pasarDelimitadores();
    if ( flex->getCaracterActual() == ProwlLexer::OperadorAsigna[ 0 ] )
    {
        flex->avanzar();

        // Leer la expresión
        toret->setExpr( crearAstExpr() );
    }

    pasarFinInstr();
    return toret;
}

If * ParserProwl::crearAstIf() throw(Zero::ESintaxis)
{
    unsigned int pos = flex->getPosicion();
    If * toret = new If( flex->getNumLinea(), flex->getPosicion() );
    Bloque bloque;

    // Guardar el bloque en la pila de bloques
    bloque.tipoBloque = Bloque::If;
    bloque.instr = toret;
    bloqueActual.push( bloque );

    // Pasar "if"
    if ( flex->getReservada( ProwlLexer::NotRequired ) != ProwlLexer::ReservadaIf ) {
        flex->setPosicion( pos );
    }

    // Pasar el PARI
    if ( !flex->chkSig( ProwlLexer::OperadorIniParams ) ) {
        throw Zero::ESintaxis( "se esperaba '('" );
    }

    // Pasar la expr
    flex->pasarDelimitadores();
    flex->avanzar();
    toret->setExpr( crearAstExpr() );

    // Pasar el PARD
    if ( !flex->chkSig( ProwlLexer::OperadorFinParams ) ) {
        throw Zero::ESintaxis( "se esperaba ')'" );
    }

    // Pasar el BEGIN
    flex->pasarDelimitadores();
    flex->avanzar();
    if ( !flex->chkSig( ProwlLexer::Begin ) ) {
        throw Zero::ESintaxis( "se esperaba '{'" );
    }

    flex->pasarDelimitadores();
    flex->avanzar();
    return toret;
}

Ret * ParserProwl::crearAstRet() throw(Zero::ESintaxis)
{
    Ret * toret = new Ret( flex->getNumLinea(), flex->getPosicion() );
    unsigned int pos = flex->getPosicion();

//std::printf( "crearAstRet(begin):\n");
//informe( flex );

    // Pasar "return"
    if ( flex->getReservada( ProwlLexer::NotRequired ) != ProwlLexer::ReservadaRet ) {
        flex->setPosicion( pos );
    }

    // Es el final ';'?
    if ( !flex->chkSig( ProwlLexer::OperadorSeparadorInstr ) ) {
        toret->setExpr( crearAstExpr() );
    }

    // Ahora pasar ;
    if ( flex->chkSig( ProwlLexer::OperadorSeparadorInstr ) ) {
        flex->avanzar();
    }

    return toret;
}

const ListaAst<Expr> &ParserProwl::crearAstParams(Msg * msg) throw(Zero::ESintaxis)
{
    // Pasar el PARI, si existe
    if ( flex->chkSig( ProwlLexer::OperadorIniParams ) ) {
        flex->avanzar();
    }

    // Leer los params.
    while ( !flex->chkSig( ProwlLexer::OperadorFinParams ) ) {
        msg->parametros.insertar( crearAstExpr() );
        if ( !flex->chkSig( ProwlLexer::OperadorFinParams ) ) {
            if ( !flex->chkSig( ProwlLexer::OperadorEnumeracion ) ) {
                throw Zero::ESintaxis( "esperando ')' o ',' en lista params." );
            } else flex->avanzar();
        }
    }

    // Pasar el PARD
    flex->avanzar();

    return msg->parametros;
}

Lit * ParserProwl::crearAstLit() throw(Zero::ESintaxis)
{
    Lit * toret = NULL;
    Lexer::TokenType tipoToken = flex->getSigTipoToken();
//printf( "crearAstLit:\n");
//informe( flex );

    if ( tipoToken == Lexer::Number ) {
        toret = new LitNumero( flex->getNumero(), flex->getNumLinea(), flex->getPosicion() );
    }
    else
    if ( tipoToken == Lexer::Text ) {
        // Pasar delimitador (")
        flex->avanzar();

        std::string literal = flex->getTexto( ProwlLexer::DelimitadorTexto );
        toret = new LitTexto( literal, flex->getNumLinea(), flex->getPosicion()  );

        if ( flex->chkSig( ProwlLexer::DelimitadorTexto ) ) {
            flex->avanzar();
        }

//printf( "crearAstLit: leido literal '%s'\n", toret->getLiteral().c_str() );
//informe( flex );

    }

    return toret;
}

Expr * ParserProwl::crearAstExpr() throw(Zero::ESintaxis)
{
    std::string id;
    Expr * toret = crearAstLit();

    if ( toret == NULL ) {
        unsigned int oldPos = flex->getPosicion();
        id = flex->getIdentificador( ProwlLexer::NotRequired );

        if ( flex->getCaracterActual() == ProwlLexer::OperadorAcceso[ 0 ] )
        {
            flex->setPosicion( oldPos );
            toret = crearAstMsg();
        }
        else toret = new Id( id, flex->getNumLinea(), flex->getPosicion() );
    }

    return toret;
}

Expr * ParserProwl::crearAstExprCond() throw(Zero::ESintaxis)
{
    Expr * toret = NULL;
    std::string id;
    unsigned int oldPos = flex->getPosicion();
    id = flex->getIdentificador( ProwlLexer::NotRequired );

    if ( flex->getCaracterActual() == ProwlLexer::OperadorAcceso[ 0 ] )
    {
        flex->setPosicion( oldPos );
        toret = crearAstMsg();
    }
    else toret = new Id( id, flex->getNumLinea(), flex->getPosicion() );

    return toret;
}

Expr * ParserProwl::crearAstSubMsg() throw(Zero::ESintaxis)
{
    Expr * toret = crearAstMsg( Zero::LOC_RR );

    // Otro subMsg?
    if ( flex->chkSig( ProwlLexer::OperadorAcceso ) ) {
        if ( dynamic_cast<Msg *>( toret ) != NULL ) {
            flex->avanzar();
            ( (Msg *) toret )->setExpr( crearAstSubMsg() );
        }
        else throw Zero::ESintaxis( "submensaje incorrecto" );
    }

    return toret;
}

Expr * ParserProwl::crearAstMsg(const std::string &refInit) throw(Zero::ESintaxis)
{
    Expr * toret = new Msg( flex->getNumLinea(), flex->getPosicion() );
    int separadores = 0;
    std::vector<std::string> ref;
    bool huboParametros = false;

    // Tomar la ref inicial, si existe
    if ( !refInit.empty() ) {
        ref.push_back( refInit );
    }

//std::printf( "crearAstMsg:\n" );
//informe( flex );

    // Tomar la instr hasta el fin de lin. o ;
    std::string id = flex->getIdentificador();
    if ( !id.empty() ) {
        ref.push_back( id );
        separadores = 1;
    }

    while( !flex->esFin() ) {

        if ( flex->chkSig( ProwlLexer::OperadorAcceso ) ) {
            --separadores;
            flex->avanzar();

            if ( separadores != 0 ) {
                throw Zero::ESintaxis( ".. encontrado" );
            }
        }
        else
        if ( flex->chkSig( ProwlLexer::OperadorIniParams ) ) {
            flex->avanzar();
            huboParametros = true;
            crearAstParams( (Msg *) toret );

            if ( flex->chkSig( ProwlLexer::OperadorAcceso ) ) {
                // Presencia de submsg
                flex->avanzar();
                ( ( Msg *) toret )->setExpr( crearAstSubMsg() );
            }
        }
        else
        if ( flex->chkSig( ProwlLexer::OperadorFinParams ) ) {
            break;
        }
        else
        if ( flex->chkSig( ProwlLexer::OperadorSeparadorInstr ) ) {
            break;
        }
        else lanzaBugChar();

        // Tomar sig token
        id = flex->getIdentificador();
        if ( !id.empty() ) {
            ref.push_back( id );
            ++separadores;
        }
    }

    if ( huboParametros ) {
        ( (Msg *) toret )->setMsgInfo( ref );
    } else {
        delete toret;
        toret = new Ref( ref, flex->getNumLinea(), flex->getPosicion() );
    }

    return toret;
}

Asg * ParserProwl::crearAstAsg(const std::string &id) throw(Zero::ESintaxis)
{
    Asg * toret = new Asg( id, flex->getNumLinea(), flex->getPosicion() );

    // Pasar '='
    flex->avanzar();

    // Tomar la expr
    toret->setExpr( crearAstExpr() );

    pasarFinInstr();
    return toret;
}

Mth * ParserProwl::leerDeclMetodo()
{
    Mth * toret = NULL;
    bool esPublico = true;
    std::vector<std::string> parametros;
    std::string nombreMetodo;
    std::string parametro;
    unsigned int numLinea = 0;
    unsigned int pos = 0;

//std::printf( "leerDeclMth( comienzo ):\n" );
//informe( flex );

    // Leer visibilidad
    flex->pasarDelimitadores();
    if( flex->getCaracterActual() == ProwlLexer::OperadorVisibilidadPrivada[ 0 ] ) {
        esPublico = false;
    } else {
        if( flex->getCaracterActual() != ProwlLexer::OperadorVisibilidadPublica[ 0 ] ) {
            throw Zero::ESintaxis( "operador de visibilidad esperado: '+' o '-'" );
        }
    }

    // Pasar visibilidad
    flex->avanzar();

    // Leer el nombre del mth
    try {
        nombreMetodo = flex->getIdentificador( true );
        numLinea = flex->getNumLinea();
        pos = flex->getPosicion();
    } catch(const Zero::ESintaxis &e) {
        throw Zero::ESintaxis( "esperando nombre de mth." );
    }

    // Pasar el parent izq
    if ( !flex->chkSig( ProwlLexer::OperadorIniParams ) ) {
        throw Zero::ESintaxis( "esperando '(' para iniciar argumentos formales" );
    } else flex->avanzar();

    // Leer los params
    parametro = flex->getIdentificador( false );
    while ( !parametro.empty() ) {
        parametros.push_back( parametro );

        if ( !flex->chkSig( ProwlLexer::OperadorEnumeracion ) ) {
            if ( !flex->chkSig( ProwlLexer::OperadorFinParams ) ) {
                    throw Zero::ESintaxis( "esperando ',' o ')' en lista de argumentos formales" );
            }
        } else {
            flex->avanzar();
        }

        parametro = flex->getIdentificador( false );
    }

//std::printf( "leerDeclMth( pasar PARD ):\n" );
//informe( flex );

    // Pasar el parent drcho
    if ( !flex->chkSig( ProwlLexer::OperadorFinParams ) ) {
        throw Zero::ESintaxis( "esperando ')' para fin de argumentos formales" );
    }
    else flex->avanzar();

//std::printf( "leerDeclMth( pasar { ):\n" );
//informe( flex );

    // Pasar la llave de apertura
    if ( !flex->chkSig( ProwlLexer::Begin ) ) {
        throw Zero::ESintaxis( "esperando '{' de comienzo mth." );
    }
    else flex->avanzar();

//std::printf( "leerDeclMth( fin ):\n" );
//informe( flex );

    // Preparar elemento
    toret = new Mth( nombreMetodo, esPublico, numLinea, pos );
    toret->setParams( parametros );

    return toret;
}

void ParserProwl::crearAstAtr() throw(Zero::ESintaxis)
{
    std::string nombre;
    bool esPublico = true;
    Atr * toret = NULL;
    Expr * expr = NULL;

    // Leer visibilidad
    flex->pasarDelimitadores();
    if( flex->getCaracterActual() == ProwlLexer::OperadorVisibilidadPrivada[ 0 ] ) {
        esPublico = false;
    } else {
        if( flex->getCaracterActual() != ProwlLexer::OperadorVisibilidadPublica[ 0 ] ) {
            throw Zero::ESintaxis( "operador de visibilidad esperado: '+' o '-'" );
        }
    }

    // Pasar visibilidad
    flex->avanzar();

    // Leer el nombre del atr
    try {
        nombre = flex->getIdentificador( true );
    } catch(const Zero::ESintaxis &e) {
        throw Zero::ESintaxis( "esperando nombre de mth: " );
    }

    // Leer literal o id
    if ( !flex->chkSig( ProwlLexer::OperadorFinInstruccion ) ) {
        if ( !flex->chkSig( ProwlLexer::OperadorAsigna ) ) {
            throw Zero::ESintaxis( "se esperaba '='" );
        }

        flex->avanzar();
        expr = crearAstExpr();
    }

    // Meter en la lista de atributos
    toret = new Atr( nombre, esPublico, flex->getNumLinea(), flex->getPosicion() );
    toret->setExpr( expr );
    objetoActual->atributos.insertar( toret );

    // Pasar el fin de lin.
    pasarFinInstr();
}

void ParserProwl::crearAstObj() throw(Zero::Excepcion)
{
//std::printf( "creaAstObj (buscar obj):\n" );
//informe( flex );
    std::string reservada = buscarMthAttr();

    while ( !esFinObj()
         && !esFinObj( reservada )
         && !flex->esFin() )
    {
        if ( reservada == ProwlLexer::ReservadaAttr
          || reservada == ProwlLexer::ReservadaAttribute )
        {
            crearAstAtr();
        }
        else
        if ( reservada == ProwlLexer::ReservadaMth
          || reservada == ProwlLexer::ReservadaMethod )
        {
            metodoActual = leerDeclMetodo();
//std::printf( "creasAstObj (crear mth):\n" );
//informe( flex );

            status = StInMethod;
            crearAstMth();
            status = StInObject;
            metodoActual = NULL;
        }
        else throw Zero::ESintaxis( "miembro debe ser met. o atr." );

//std::printf( "crearAstObj(sig miembro):\n");
//informe( flex );
        if ( !flex->chkSig( ProwlLexer::End ) ) {
            reservada = buscarMthAttr();
        }
    }

    // Pasar }
    if ( flex->chkSig( ProwlLexer::End ) ) {
        flex->avanzar();
    }
    else
    if ( esFinObj( reservada ) )
    {
        if ( objetoActual->declaradoConLlave() )
        {
            throw Zero::ESintaxis( "se esperaba '}'. No es posible mezclar estilos de sintaxis." );
        }
    }

    status = StTopLevel;
    objetoActual = NULL;
    metodoActual = NULL;
    return;
}

bool ParserProwl::esFinObj()
{
    return  ( flex->chkSig( ProwlLexer::End )
           || flex->chkSig( ProwlLexer::ReservadaEndObj )
           || flex->chkSig( ProwlLexer::ReservadaEndObject )
    );
}

bool ParserProwl::esFinObj(const std::string &reservada)
{
    return  ( reservada == ProwlLexer::ReservadaEndObj
           || reservada == ProwlLexer::ReservadaEndObject
    );
}

std::string ParserProwl::buscarMthAttr()
{
    std::string toret;

    try {
        toret = flex->getReservada( GeneralLexer::Required );
    } catch(const Zero::ESintaxis &e) {
        toret = "";
    }

    return toret;
}

std::string ParserProwl::buscarObj()
{
    std::string toret;

    try {
        toret = flex->getReservada( GeneralLexer::Required );
    } catch(const Zero::ESintaxis &e) {
        throw Zero::ESintaxis( "esperando 'obj'/'endobj' de objeto" );
    }

    return toret;
}

void ParserProwl::crearAst() throw(Zero::Excepcion)
{
    status = StTopLevel;
    std::string reservada = buscarObj();

    while ( !flex->esFin() ) {
            if ( reservada == ProwlLexer::ReservadaObject
              || reservada == ProwlLexer::ReservadaObj )
            {
                if ( getStatus() == StTopLevel ) {
                    objetoActual = leerDeclObjeto();
                    metodoActual = NULL;

                    status = StInObject;
                    crearAstObj();
                    objetoActual = NULL;
                } else throw Zero::ESintaxis( "se esperaba objeto en toplevel" );
            }

            if ( !flex->chkSig( ProwlLexer::End )
              && !flex->esFin() )
            {
                reservada = buscarObj();
            }
            else reservada = "";
    }

    return;
}

Obj * ParserProwl::leerDeclObjeto()
{
    Obj * toret = NULL;
    std::string nombreObjeto;
    std::string nombreObjetoPadre;

    // Leer el nombre del objeto
    nombreObjeto = flex->getIdentificador( GeneralLexer::Required );

    if ( nombreObjeto.empty() ) {
        throw Zero::ESintaxis( "se esperaba id (nombre de objeto)" );
    }

    if ( flex->chkSig( ProwlLexer::OperadorHerencia ) ) {
        flex->avanzar( ProwlLexer::OperadorHerencia.length() );
        nombreObjetoPadre = flex->getIdentificador( GeneralLexer::Required );
    } else {
        nombreObjetoPadre = Zero::OBJ_OBJECT;
    }

    // Crear el objeto declarado
    bool tieneLlave = flex->chkSig( ProwlLexer::Begin );

    if ( tieneLlave ) {
        if ( dynamic_cast<AperturaDeBloque *>( flex->getSigElemento().get() ) == NULL ) {
            throw Zero::ESintaxis( "se esperaba {" );
        }
    }

    // construir el elemento sintaxis para AST
    toret = new Obj( nombreObjeto, nombreObjetoPadre,
                     flex->getNumLinea(), flex->getPosicion(), tieneLlave
    );
    getAST()->objetos.insertar( toret );

    return toret;
}

void ParserProwl::lanzaBugChar()
{
    char *finMsg;
    static char msg[80];

    // Cons. mensaje de error
    std::strcpy( msg, "char inesperado: " );
    finMsg = msg + std::strlen( msg );
    *( finMsg++ ) = flex->getCaracterActual();
    *finMsg = '\0';

    throw Zero::ESintaxis( msg );
}

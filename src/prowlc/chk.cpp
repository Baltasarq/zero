// chk.cpp
/*
    Comprobaciones sobre el AST

    jbgarcia@uvigo.es
*/

#include "chk.h"
#include "ast.h"
#include "parser.h"
#include "parserprowl.h"

#include <set>

// ------------------------------------------------------------------------ Chk
std::string Chk::crearPath(const std::string &id, const Mth *mth, const Obj *obj)
{
    std::string toret = id;

    toret.push_back( '@' );
    toret.append( obj->getNombre() );
    toret.push_back( '.' );
    toret.append( mth->getNombre() );
    toret.push_back( '(' );
    toret.push_back( ')' );


    return toret;
}

// -------------------------------------------------------------------- ChkAtrs
void ChkAtrs::doIt(const ElementoAst * elem, const ElementoAst * env) throw( Zero::Excepcion )
{
    const Atr * atr = dynamic_cast<const Atr *>( elem );

    if ( atr != NULL ) {
        if ( atr->getExpr() != NULL
          && dynamic_cast<const Lit *>( atr->getExpr() ) == NULL
          && dynamic_cast<const Ref *>( atr->getExpr() ) == NULL )
        {
            std::string nombre = env->getNombre();

            nombre.push_back( '.' );
            nombre.append( atr->getNombre() );

            throw Zero::ESemantico( "atributos no pueden tener un valor inicial diferente de "
                                    "literal o referencia",
                                    nombre.c_str()
            );
        }
    }
    else throw Zero::EInterno( "ChkAtrs incorrecto" );

    return;
}

// --------------------------------------------------------------------- ChkRet
void ChkRets::doIt(const ElementoAst * elem, const ElementoAst * env) throw( Zero::Excepcion )
{
    const Mth * mth = dynamic_cast<const Mth *>( elem );

    if ( mth != NULL ) {
        const Instr * instr = mth->instrucciones[ mth->instrucciones.size() -1 ];
        std::string nombre = env->getNombre();
        nombre.push_back( '.' );
        nombre.append( mth->getNombre() );

        if ( dynamic_cast<const Ret *>( instr ) == NULL ) {
            throw Zero::ESemantico( "mets. deben terminar con 'return'", nombre.c_str() );
        }
    } else throw Zero::EInterno( "ChkRets incorrecto" );

    return;
}

// ----------------------------------------------------------------- ChkMsgArgs
void ChkMsgArgs::doIt(const ElementoAst * elem, const ElementoAst * env) throw( Zero::Excepcion )
{
    const Instr * instr = dynamic_cast<const Instr *>( elem );
    const Mth * mth = dynamic_cast<const Mth *>( env );

    if ( instr != NULL
      || mth != NULL )
    {
        const Msg * msg = dynamic_cast<const Msg *>( instr );

        if ( msg != NULL ) {
            for(unsigned int i = 0; i < msg->parametros.size(); ++i) {
                const std::string &id = ( (Id * ) msg->parametros[ i ] )->getId();

                if ( dynamic_cast<const Id *>( msg->parametros[ i ] ) != NULL
                  && !mth->esLocal( id ) )
                {
                    std::string path = id;


                    path.push_back( '@' );
                    path.append( mth->getNombre() );
                    path.push_back( ':' );
                    path.push_back( ' ' );
                    path.append( msg->getRef() );
                    path.push_back( '.' );
                    path.append( msg->getNombreMth() );
                    path.push_back( '(' );
                    path.push_back( ')' );
                    throw Zero::ESemantico( "arg. no es ref. local", path.c_str() );
                }
            }
        }
    } else throw Zero::EInterno( "ChkMsgArgs incorrecto" );

    return;
}

// -------------------------------------------------------------------- ChkDefs
void ChkDefs::doIt(const ElementoAst * elem, const ElementoAst * env) throw( Zero::Excepcion )
{
    std::set<std::string> defs;
    const Mth * mth = dynamic_cast<const Mth *>( elem );

    if ( mth != NULL ) {
        for(unsigned int i = 0; i < mth->instrucciones.size(); ++i) {
            const Def * def = dynamic_cast<const Def *>( mth->instrucciones[ i ] );

            if ( def != NULL ) {
                const std::string &id = def->getId();

                // Comprobar si está, si no, meter
                if ( defs.find( id ) != defs.end() ) {
                    throw Zero::ESemantico( "ref. local repetida", Chk::crearPath( id, mth, (const Obj *) env ).c_str() );
                }
                else defs.insert( id );
            }
        }
    }
    else throw Zero::EInterno( "ChkMsgArgs incorrecto" );
}

// -------------------------------------------------------------------- ChkJmps
void ChkJmps::doIt(const ElementoAst * elem, const ElementoAst * env) throw( Zero::Excepcion )
{
    std::set<std::string> etqs;
    const Mth * mth = dynamic_cast<const Mth *>( elem );
    std::vector<const Jmp *> jmps;

    if ( mth != NULL ) {
        // Chk no hay etqs repetidas. Recolectar Jmps
        for(unsigned int i = 0; i < mth->instrucciones.size(); ++i) {
            const Etq * etq = dynamic_cast<const Etq *>( mth->instrucciones[ i ] );
            const Jmp * jmp = dynamic_cast<const Jmp *>( mth->instrucciones[ i ] );

            if ( etq != NULL ) {
                const std::string &id = etq->getId();

                // Comprobar si está, si no, meter
                if ( etqs.find( id ) != etqs.end() ) {
                    throw Zero::ESemantico( "etq. repetida", Chk::crearPath( id, mth, (const Obj *) env ).c_str() );
                }
                else etqs.insert( id );
            }
            else
            if ( jmp != NULL ) {
                jmps.push_back( jmp );
            }
        }

        // Chk saltos correctos, a etqs que existen
        std::vector<const Jmp *>::const_iterator it = jmps.begin();
        for(; it != jmps.end(); ++it) {
            const std::string &id = (*it)->getId();

            if ( etqs.find( id ) == etqs.end() ) {
                throw Zero::ESemantico( "goto a etq. inexistente", Chk::crearPath( id, mth, (const Obj *) env ).c_str() );
            }
        }
    }
    else throw Zero::EInterno( "ChkMsgArgs incorrecto" );
}

// ------------------------------------------------------------------ GestorChk
GestorChk::GestorChk(Parser *p) : parser( p )
{
    chksAtr.push_back( new ChkAtrs( parser ) );
    chksMth.push_back( new ChkRets( parser ) );
    chksMth.push_back( new ChkDefs( parser ) );
    chksMth.push_back( new ChkJmps( parser ) );
    chksInstr.push_back( new ChkMsgArgs( parser ) );
}

GestorChk::~GestorChk()
{
    liberar( chksObj );
    liberar( chksAtr );
    liberar( chksMth );
    liberar( chksInstr );
}

void GestorChk::liberar(std::vector<Chk *> &listaChks)
{
    std::vector<Chk *>::iterator it = listaChks.begin();

    for(; it != listaChks.end(); ++it) {
        delete (*it);
    }

    listaChks.clear();
    return;
}

void GestorChk::doIt() throw ( Zero::Excepcion )
{
    const ListaAst<Obj> &objetos = getParser()->getAST()->objetos;

    // Chk preliminates
    if ( objetos.size() == 0 ) {
        throw Zero::ESintaxis( "sin objetos que compilar" );
    }

    if ( getParser()->getObjPpal() == NULL ) {
        throw Zero::ESemantico( "objeto principal no encontrado" );
    }

    // Chk objetos
    for(unsigned int i = 0; i < objetos.size(); ++i) {
        const Obj * obj = objetos[ i ];

        chk( obj, NULL, chksObj );

        // Chk attrs
        const ListaAst<Atr> &atrs = obj->atributos;

        for(unsigned int j = 0; j < atrs.size(); ++j) {
            chk( atrs[ j ], obj, chksAtr );
        }

        // Chk mths
        const ListaAst<Mth> &mths = obj->metodos;
        for(unsigned int j = 0; j < mths.size(); ++j) {
            const Mth * mth = mths[ j ];

            chk( mth, obj, chksMth );

            // Chk instrucciones
            for(unsigned int k = 0; k < mth->instrucciones.size(); ++k) {
                chk( mth->instrucciones[ k ], mth, chksInstr );
            }
        }
    }
}

void GestorChk::chk(
        const ElementoAst * elem,
        const ElementoAst * env,
        const std::vector<Chk *> &chks) throw( Zero::Excepcion )
{
    std::vector<Chk *>::const_iterator it = chks.begin();

    for(; it != chks.end(); ++it) {
        (*it)->doIt( elem, env );
    }

    return;
}

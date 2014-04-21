#include "transform.h"
#include "stringman.h"

// -------------------------------------------------------------------- PilaLoc
const std::string PilaLoc::PrefijoLoc = "__prowlc_aux_loc_";

PilaLoc::PilaLoc(Mth * m) : mth( m ), numLoc( 0 ), numLocReg( 1 )
{
    reiniciar();
}

const std::string PilaLoc::getSigLoc(size_t & i)
{
    std::string toret;

    if ( numLocReg < Reg::NumGpRegs ) {
        toret = Reg::getGp( (Reg::GpReg) numLocReg );
        ++numLocReg;
    } else {
        toret = PrefijoLoc + StringMan::toString( numLoc );
        toret.push_back( '_' );

        mth->instrucciones.insertar( 0, new DefInterno( toret ) );
        ++i;
        ++numLoc;
    }

    return toret;
}

void PilaLoc::setMetodo(Mth *mth)
{
    this->mth = mth;
    reiniciar();
    this->numLocReg = 1;
}

// ------------------------------------------------------------------ Transform
std::string Transform::simplificaRef(size_t &i, const Ref * ref, Mth * mth, PilaLoc * pila)
{
    std::string loc = pila->getSigLoc( i );
    Set * set = NULL;

    if ( Reg::esReg( loc ) ) {
        set = new Set( ref->getRef(), loc );
        mth->instrucciones.insertar( i, set );
        ++i;
    } else {
        // insertar SET
        set = new Set( ref->getRef() );
        mth->instrucciones.insertar( i, set );
        ++i;
        mth->instrucciones.insertar( i, new Asg( loc ) );
        ++i;
    }

    return loc;
}

// -------------------------------------------------------------- TransformLits
bool TransformLits::doIt(size_t &i, ElementoAst * elem, ElementoAst *env)
{
    Def * def = dynamic_cast<Def *>( elem );
    Msg * msg = dynamic_cast<Msg *>( elem );
    Mth * metodoActual = dynamic_cast<Mth *>( env );

    if ( metodoActual == NULL ) {
        throw Zero::Excepcion( "Error en TransformLits::doIt" );
    }

    if ( msg != NULL ) {
        for(size_t j = 0; j < msg->parametros.size(); ++j) {
            Lit * lit = dynamic_cast<Lit *>( msg->parametros[ j ] );

            if ( lit != NULL ) {
                std::string loc = pila->getSigLoc( i );

                metodoActual->instrucciones.insertar( i, lit->crearLiteral() );
                metodoActual->instrucciones.insertar( i +1, new Asg( loc ) );
                msg->parametros.modificar( j, new Reg( loc ) );

                i += 2;
            }
        }
    }
    else
    if ( def != NULL ) {
        Lit * lit = dynamic_cast<Lit *>( def->getExpr() );

        if ( lit != NULL )
        {
            metodoActual->instrucciones.insertar( i, lit->crearLiteral() );
            metodoActual->instrucciones.insertar( i +1, new Asg( def->getId() ) );
            def->setExpr( NULL );

            --i;
        }
    }


    return false;
}

// -------------------------------------------------- TransformSimplificaParams
bool TransformSimplificaParams::doIt(size_t &i, ElementoAst * elem, ElementoAst *env)
{
    bool toret = false;
    Msg * msg = dynamic_cast<Msg *>( elem );
    Mth * metodoActual = dynamic_cast<Mth *>( env );

    if ( metodoActual == NULL ) {
        throw Zero::Excepcion( "Error en TransformSimplificaParams::doIt" );
    }

    if ( msg != NULL ) {

        for(size_t j = 0; j < msg->parametros.size(); ++j) {
            Ref * ref = dynamic_cast<Ref *>( msg->parametros[ j ] );
            Msg * subMsg = dynamic_cast<Msg *>( msg->parametros[ j ] );

            // Es una referencia
            if ( ref != NULL ) {
                Id * id = dynamic_cast<Id *>( ref );

                // Si es local, no hacer nada.
                if ( id == NULL
                  || !metodoActual->esLocal( id->getId() ) )
                {
                    msg->parametros.modificar(
                            j,
                            new Id( Transform::simplificaRef( i, ref, metodoActual, pila ), 0, 0 )
                    );
                }
            }
            else
            // Es un mensaje
            if ( subMsg != NULL ) {
                std::string loc = pila->getSigLoc( i );

                metodoActual->instrucciones.insertar( i, msg->parametros.extraer( j ) );
                metodoActual->instrucciones.insertar( i +1, new Asg( loc ) );
                msg->parametros.modificar( j, new Id( loc ) );

                i += 2;
                toret = true;
            }
        }

    }

    return toret;
}

// ------------------------------------------------------ TransformDefsComienzo
bool TransformDefsComienzo::doIt(size_t &ord, ElementoAst * elem, ElementoAst *env)
{
    Mth * mth = dynamic_cast<Mth *>( elem );

    if ( mth != NULL ) {

        for(size_t i = 0; i < mth->instrucciones.size(); ++i) {
            Def * def = dynamic_cast<Def *>( mth->instrucciones[ i ] );

            if ( def != NULL ) {
                mth->instrucciones.mover( i, 0 );
            }
        }

    }
    else throw Zero::Excepcion( "Mal TransformDefsComienzo" );

    return false;
}

// -------------------------------------------------------------- TransformThis
bool TransformThis::doIt(size_t &ord, ElementoAst * elem, ElementoAst *env)
{
   Instr * instr = dynamic_cast<Instr *>( elem );
   Mth * mth = dynamic_cast<Mth *>( env );

   if ( instr != NULL
     && mth != NULL )
   {
       MixinRef * instrConRef = dynamic_cast<MixinRef *>( instr );
       Msg * msg = dynamic_cast<Msg *>( instr );

       // Comprobar la referencia y los params. del mensaje
       if ( msg != NULL ) {
            // Ref ?
            if ( msg->getRef() == Reg::This ) {
                msg->setRef( Reg::getThis() );
            }

            // Params ?
            for(size_t i = 0; i < msg->parametros.size(); ++i) {
                Expr * param = msg->parametros[ i ];

                if ( dynamic_cast<Id *>( param ) != NULL ) {
                    Id * id = (Id *) param;

                    if ( id->getId() == Reg::This ) {
                        msg->parametros.modificar( i, new Reg( Reg::getThis() ) );
                    }
                }
            }
       }
       else
       // Comprobar las expresiones
       if ( instrConRef != NULL ) {
            // Expresiones
            Id * id = dynamic_cast<Id *>( instrConRef->getExpr() );
            if ( id != NULL ) {
                if ( id->getId() == Reg::This ) {
                   instrConRef->setExpr( new Reg( Reg::getThis() ) );
                }
            }
       }
    }
    else throw Zero::Excepcion( "Mal TransformThis" );

    return false;
}

// --------------------------------------------------------------- TransformAsg
bool TransformConRef::doIt(size_t &ord, ElementoAst * elem, ElementoAst *env)
{
    Instr * instr = dynamic_cast<Instr *>( elem );
    Mth * mth = dynamic_cast<Mth *>( env );
    bool toret = false;

    if ( instr != NULL
      && mth != NULL )
    {
        MixinRef * mix = dynamic_cast<MixinRef *>( instr );
        Msg * msgMix = dynamic_cast<Msg *>( instr );
        Set * setMix = dynamic_cast<Set *>( instr );
        Asg * asgMix = dynamic_cast<Asg *>( instr );

        // Si es un Set o un Asg, no hacer nada
        if ( setMix != NULL
          || asgMix != NULL )
        {
            return false;
        }

        // Las subexpresiones en Msg hay que tratarlas diferente
        if ( msgMix != NULL ) {
            if ( msgMix->getExpr() != NULL ) {
                mth->instrucciones.insertar( ord +1, new Asg( Zero::LOC_RR ) );
                mth->instrucciones.insertar( ord +2, msgMix->extractExpr() );
            }
        }
        else
        if ( mix != NULL ) {

            Msg * msg = dynamic_cast<Msg *>( mix->getExpr() );
            Ref * ref = dynamic_cast<Ref *>( mix->getExpr() );

            if ( msg != NULL ) {
                std::printf( "transformConRef: #%d: %s@%s()\n", ord, elem->getNombre().c_str(), env->getNombre().c_str() );
                mth->instrucciones.insertar( ord, mix->extractExpr() );

                // Si es una def, inserta un ASG
                if ( dynamic_cast<Def *>( instr ) != NULL ) {
                    mth->instrucciones.insertar( ord +1, new Asg( ( (Def *) mix )->getId() ) );

                    ++ord;
                } else {
                    // Si no es un Def, hay que poner lo anterior a ACC
                    mix->setExpr( new Reg( Reg::getAcc() ) );
                }

                ++ord;
                toret = true;
            }
            else
            if ( ref != NULL ) {
                Id * id = dynamic_cast<Id *>( ref );

                // Si es local, no hacer nada.
                if ( id == NULL
                  || !mth->esLocal( id->getId() ) )
                {
                    mix->setExpr(
                            new Id( Transform::simplificaRef( ord, ref, mth, pila ), 0, 0 )
                    );
                }
            }
        }
    }
    else throw Zero::Excepcion( "Mal TransformConRef" );

    return toret;
}

// ----------------------------------------------------- GestorTransformaciones
GestorTransformaciones::GestorTransformaciones(AST * arbol) : ast(arbol)
{
    pila.reset( new PilaLoc );

    transfsInstr.push_back( new TransformConRef( ast, pila.get() ) );
    transfsInstr.push_back( new TransformThis( ast, pila.get() ) );
    transfsInstr.push_back( new TransformLits( ast, pila.get() ) );
    transfsInstr.push_back( new TransformSimplificaParams( ast, pila.get() ) );
    transfsMth.push_back( new TransformDefsComienzo( ast, pila.get() ) );
}

void GestorTransformaciones::liberar(std::vector<Transform *> &listaTrans)
{
    std::vector<Transform *>::iterator it = listaTrans.begin();

    for(; it != listaTrans.end(); ++it) {
        delete (*it);
    }

    listaTrans.clear();
    return;
}

GestorTransformaciones::~GestorTransformaciones()
{
    liberar( transfsInstr );
    liberar( transfsMth );
    liberar( transfsObj );
}

bool GestorTransformaciones::aplicaTransformaciones(size_t &ord, ElementoAst * elem, ElementoAst *env, const std::vector<Transform *> &listaTrans)
{
    std::vector<Transform *>::const_iterator it = listaTrans.begin();
    bool toret = false;

    for(; it != listaTrans.end(); ++it) {
        if ( (*it)->doIt( ord, elem, env ) ) {
            toret = true;
        }
    }

    return toret;
}

void GestorTransformaciones::doIt()
{
    bool reaplicarInstrs;

    for(size_t i = 0; i < getAST()->objetos.size(); ++i) {
        objetoActual = ast->objetos[ i ];
        ListaAst<Mth> &metodos = ast->objetos[ i ]->metodos;

        for(size_t j = 0; j < metodos.size(); ++j) {
            metodoActual = metodos[ j ];
            pila->setMetodo( metodoActual );

            do {
                reaplicarInstrs = false;
std::printf( "Vuelta: instrs en %s(): %d instrs.\n", metodoActual->getNombre().c_str(), metodoActual->instrucciones.size() );
for(size_t k = 0; k < metodoActual->instrucciones.size(); ++k) {
    std::printf( "#%d: %s\n", k, metodoActual->instrucciones[ k ]->generarCodigo().c_str() );
}
std::printf( "Empieza\n" );

                for(size_t k = 0; k < metodoActual->instrucciones.size(); ++k) {
                    Instr * instr = metodoActual->instrucciones[ k ];
                    pila->reiniciar();

                    std::printf( "Aplica transfs a: #%d: %s\n", k, metodoActual->instrucciones[ k ]->getNombre().c_str() );
                    if ( aplicaTransformaciones( k, instr, metodoActual, transfsInstr ) )
                    {
                        reaplicarInstrs = true;
                    }
                }
            } while( reaplicarInstrs );

            aplicaTransformaciones( j, metodoActual, objetoActual, transfsMth );
        }

        aplicaTransformaciones( i, objetoActual, NULL, transfsObj );
    }

    return;
}


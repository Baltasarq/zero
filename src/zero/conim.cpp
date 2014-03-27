// conim.cpp

#include "conim.h"
#include "runtime.h"

namespace Zero {

// ====================================================================== Conim
Conim *Conim::conim = NULL;

// ------------------------------------------------------------- Conim::Conim()
Conim::Conim()
{
    conim  = this;

    lConts = (ListaAlmacenes *) new(std::nothrow) char[ sizeof(ListaAlmacenes) * 4 ];

    if ( lConts != NULL ) {
        lConts = new( lConts ) ListaAlmacenes( ListaAlmacenes::NO_PROPIETARIO );
        lPersConts = new( lConts + 1 )
                    ListaAlmacenes( ListaAlmacenes::NO_PROPIETARIO )
        ;

        lTransConts = new( lConts + 2 )
                    ListaAlmacenes( ListaAlmacenes::NO_PROPIETARIO )
        ;

        lSincroConts = new( lConts + 3 )
                    ListaAlmacenes( ListaAlmacenes::NO_PROPIETARIO )
        ;
    } else throw ENoHayMemoria( "iniciando Conim" );
}

Conim::~Conim()
{
// Lo siguiente no compila en BC++ 5.2 (no es capaz de asociar el typedef)
// Compila perfectamente en gcc
//    lConts->~ListaAlmacenes();
//    lTransConts->~ListaAlmacenes();
//    lPersConts->~ListaAlmacenes();
//    lSincroConts->~ListaAlmacenes();

    lConts->~AsocNombrePuntero<AlmacenObjetos>();
    lTransConts->~AsocNombrePuntero<AlmacenObjetos>();
    lPersConts->~AsocNombrePuntero<AlmacenObjetos>();
    lSincroConts->~AsocNombrePuntero<AlmacenObjetos>();

    delete[] ((char *) lConts);
}

// ------------------------------------------------------ Conim::eliminaTodos()
void Conim::eliminaTodos()
/**
 *  eliminaTodos()
 *
 *  Elimina todos los nodos de la lista, pero no los contenedores mismos.
 *  Los contenedores siguen otro proceso de eliminación distinto.
 *
 *  Probablemente, no es necesario este método.
*/
{
    lConts->eliminaTodos();
    lPersConts->eliminaTodos();
    lTransConts->eliminaTodos();
    lSincroConts->eliminaTodos();
}

// -------------------------------------------------------------- Conim::sync()
void Conim::sincronizar()
/**
 *  sincronizar()
 *
 *  Sincroniza todos los contenedores (persistentes), obligándoles a guardarse
 *  en disco.
 *  Debe pasar todos los contenedores a la lista de sincronizados.
*/
{
    AlmacenObjetos *almacen;

    lSincroConts->eliminaTodos();

    for(register unsigned int i = 0; i < lPersConts->getNumero(); ++i) {
        almacen = lPersConts->getElementoNumero( i );

        almacen->guardaContenedor();
        ponSincronizado( almacen );
    }

    return;
}

// ----------------------------------------------------------- Conim::inserta()
void Conim::inserta(AlmacenObjetos *almacen) throw(EInterno, EDuplicado)
/**
 * Conim::inserta()
 *
 * Se espera que este método sea llamado por el constructor del Contenedor
 * @param almacen El contenedor (persistente o no) a guardar en la lista
 *
 * Inserta un elemento. Se cuida de que:
 *  <ul>
 *      <li>No exista ya en las listas de contenedores
 *      <li>Lo inserta en la lista general (lConts)
 *      <li>Lo inserta en la lPersConts si es classof Container
 *      <li>Lo inserta en la lTransConts si es classof TransientContainer
 *      <li>Lo inserta en la lista de contenedores sincros si está sincro,
 *          es decir, no hay cambios respecto a la versión en disco.
 *  </ul>
*/
{
 /*   bool insertado;

    if ( !( lConts->inserta( almacen->getNombre(), almacen ) ) ) {
        throw ENoHayMemoria( "insertando en Conim" );
    }

    if ( !( almacen->fueModificadoDesdeCarga() ) )
    {
        if ( !( lSincroConts->inserta( almacen->getNombre(), almacen ) ) ) {
            throw ENoHayMemoria( "insertando en Conim" );
        }
    }

    if ( dynamic_cast<Container *>( almacen ) != NULL )
            insertado = lPersConts->inserta( almacen->getNombre(), almacen );
    else    insertado = lTransConts->inserta( almacen->getNombre(), almacen );

    if ( !insertado ) {
        throw ENoHayMemoria( "insertando en Conim" );
    }
 */
 throw EInterno( "NO IMPLEMENTADO" );
}

// --------------------------------------------------- Conim::getNombreAlmacen()
inline
const std::string *Conim::getNombreAlmacen(AlmacenObjetos *c)
{
    if ( c->esAlcanzable() ) {
        return &( c->getNombreCompleto() );
    } else {
        return &( ServidorDeNombres::getSigNombreUnico() );
    }
}

// --------------------------------------- Conim::insertaContainer(Container *)
void Conim::insertaContainer(Container *almacen) throw(EInterno, EDuplicado)
{
    // Insertar el contenedor
    if ( !( lConts->inserta( *getNombreAlmacen( almacen ), almacen ) ) ) {
        throw EDuplicado( "insertando en Conim" );
    }

    if ( !( almacen->fueModificadoDesdeCarga() ) )
    {
        if ( !( lSincroConts->inserta( *getNombreAlmacen( almacen ), almacen ) ) ) {
            throw EDuplicado( "insertando en Conim" );
        }
    }

    if ( !( lPersConts->inserta( *getNombreAlmacen( almacen ), almacen ) ) ) {
        throw EDuplicado( "insertando en Conim" );
    }

    return;
}

// ------------------------------ Conim::insertaTransientContainer(Container *)
void Conim::insertaTransientContainer(TransientContainer *almacen)
        throw(EDuplicado, EInterno)
{
    // Insertar el contenedor
    if ( !lConts->inserta( *getNombreAlmacen( almacen ), almacen ) ) {
        throw EDuplicado( "insertando en Conim" );
    }

    if (!( almacen->fueModificadoDesdeCarga() ) )
    {
        if ( ! ( lSincroConts->inserta( *getNombreAlmacen( almacen ), almacen ) ) ) {
            throw EDuplicado( "insertando en Conim" );
        }
    }

    if ( !( lTransConts->inserta( *getNombreAlmacen( almacen ), almacen ) ) ) {
        throw EDuplicado( "insertando en Conim" );
    }

    return;
}

// ----------------------------------------------------------- Conim::inserta()
void Conim::elimina(AlmacenObjetos *almacen) throw(EInterno)
/**
 * Conim::elimina()
 *
 * Este método será llamado por el destructor de un contenedor
 * @param almacen El contenedor (persistente o no) a eliminar de la lista
 *
 * Elimina un container. Se cuida de que:
 *  <ul>
     *      <li>Lo elimina en la lista general (lConts)
     *      <li>Lo elimina de la lPersConts si es classof Container
     *      <li>Lo elimina de la lTransConts si es classof TransientContainer
     *      <li>Lo elimina de la lista de contenedores sincros si está sincro,
     *          es decir, no hay cambios respecto a la versión en disco.
     *  </ul>
 */
{
    lConts->borra( almacen );
    lSincroConts->borra( almacen );
    lPersConts->borra( almacen );
    lTransConts->borra( almacen );

    return;
}

// --------------------------------------------------- Conim::ponSincronizado()
void Conim::ponSincronizado(AlmacenObjetos *almacen) throw(EInterno)
{
    lSincroConts->inserta( *getNombreAlmacen( almacen ), almacen );
}

// ------------------------------------------------- Conim::ponDesSincronizado()
void Conim::ponDesSincronizado(AlmacenObjetos *almacen) throw(EInterno)
{
    lSincroConts->borra( almacen );
}

} // namespace Zero


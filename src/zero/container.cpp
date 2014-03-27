// container.cpp
/*
        Implantación de contenedores en Zero

        jbgarcia@uvigo.es
*/

#include <string>

#include "container.h"
#include "runtime.h"
#include "persistentstore.h"

namespace Zero {

// -------------------------------------------------------- Objeto::insertando()
bool AlmacenObjetos::insertando(Atributo *atr)
{
    AlmacenObjetos * container = NULL;
    Atributo *atrOwner;

    if ( dynamic_cast<AtributoEspecial *>( atr ) == NULL ) {
        // Si se puede llegar a él, comprobar si es container
        if ( atr->swizzled() ) {
            container = dynamic_cast<AlmacenObjetos *>( atr->getObjeto() );
        }

        // Si es container, enlazar del todo
        if ( container != NULL )
        {
            // Asignar a este container
            container->asignarA( (AlmacenObjetos *) this );

            // Modificar su atributo "containerOwner"
            atrOwner = container->lAtrs.busca( ATR_OWNER );
            if ( atrOwner != NULL ) {
                    atrOwner->ponReferencia( getPerteneceA() );
            }
            else Runtime::ponExcepcionObjeto( ATR_OWNER + " no encontrado" );
        }
    }

    return true;
}

// ---------------------------------------------- AlmacenObjetos::esAlcanzable()
bool AlmacenObjetos::esAlcanzable()
/**
 * Determina si este contenedor es alcanzable desde la raíz de persistencia
 *
 * @return true si es alcanzable, false en otro caso
 */
{
    AlmacenObjetos * aux  = this;
    AlmacenObjetos * raiz = Runtime::rt()->getContainerRaiz();

    while( aux->getPerteneceA() != NULL
        && aux != raiz )
    {
        aux = aux->getPerteneceA();
    }

    return ( raiz == aux );
}

// ============================================================= AlmacenObjetos
void AlmacenObjetos::init()
{
    AlmacenObjetos * root = Runtime::rt()->getContainerRaiz();
    Objeto *owner = getPerteneceA();
    AtributoEspecial *atrEsp;

    // Crear atributo del ps (¡cuidado, puede ser el propio root!)
    if ( root != NULL )
            atrEsp = new(std::nothrow) AtributoEspecial( this, ATR_PS, root );
    else    atrEsp = new(std::nothrow) AtributoEspecial( this, ATR_PS, "." );

    // Insertar atributo del ps: psRoot
    if ( atrEsp != NULL ) {
        if ( !lAtrs.inserta( ATR_PS, atrEsp ) ) {
            throw ENoHayMemoria( std::string( "creando AlmacenObjetos: " + getNombre() ).c_str() );
        }
    } else throw ENoHayMemoria( std::string( "creando AlmacenObjetos: " + getNombre() ).c_str() );

    // Crear atributo del owner
    if ( owner == NULL ) {
        owner = Runtime::objetoNulo;
    }

    if ( owner != NULL )
            atrEsp = new(std::nothrow) AtributoEspecial( this, ATR_OWNER, owner );
    else    atrEsp = new(std::nothrow) AtributoEspecial( this, ATR_OWNER, OBJ_NOTHING );

    // Insertar owner
    if ( atrEsp != NULL ) {
        if ( !lAtrs.inserta( ATR_OWNER, atrEsp ) ) {
            throw ENoHayMemoria( std::string( "creando AlmacenObjetos: " + getNombre() ).c_str() );
        }
    } else throw ENoHayMemoria( std::string( "creando AlmacenObjetos: " + getNombre() ).c_str() );

    // Crear el atributo del this
    atrEsp = new(std::nothrow) AtributoEspecial( this, ATR_THISCONT, this );

    // Insertar owner
    if ( atrEsp != NULL ) {
        if ( !lAtrs.inserta( ATR_THISCONT, atrEsp ) ) {
            throw ENoHayMemoria( std::string( "creando AlmacenObjetos: " + getNombre() ).c_str() );
        }
    } else throw ENoHayMemoria( std::string( "creando AlmacenObjetos: " + getNombre() ).c_str() );
}

AlmacenObjetos::~AlmacenObjetos()
{
    Runtime::conim->elimina( this );
}

// ------------------------------------------------ AlmacenObjetos::actualiza()
void AlmacenObjetos::actualizar(Observable *obj, Observador::TipoCambio t)
/**
 * Este método, derivado de observador, es ejecutado cuando alguno de sus
 * objetos cambia.
 * @param obj Un contenedor sólo contiene, y, por tanto observa, objetos
 * @param t El tipo de cambio producido
 */
{
	notificar( Observador::CambioEnElemento );
        ponModificadoDesdeCarga();
}

// ----------------------------------------------- AlmacenObjetos::creaObjeto()
Objeto *AlmacenObjetos::crea(std::string n, Objeto *padre)
{
	// Comprobar el nombre del objeto a crear
	if ( n.empty() ) {
        	n = ServidorDeNombres::getSigNombreUnico();
	}

	// Comprobar el padre del objeto a crear
        if ( padre == NULL )
        {
                padre = Runtime::objetoRaiz;
        }

        Objeto *toret = new(std::nothrow) Objeto( this, n, padre );

	if ( toret != NULL )
	{
		if ( inserta( toret ) == NULL ) {
			// Limpiar y devolver
                        // (el objeto ya fue borrado)
			Runtime::ponExcepcion(
                                Runtime::objetoExcepcion,
                                EXC_NOMEM
                        );
			toret = NULL;
		}
	}

	return toret;
}

// -------------------------------------------- AlmacenObjetos::inserta()
Objeto *AlmacenObjetos::inserta(Objeto *obj)
{
       if ( obj != NULL ) {
            Atributo *atr = new(std::nothrow) Atributo( this, obj->getNombre(), obj );

            if (  atr == NULL
              || !( lAtrs.inserta( obj->getNombre(), atr ) ) )
            {
                    if ( atr == NULL ) {
                        Runtime::ponExcepcion( Runtime::objetoExcepcion, EXC_NOMEM );
                    }

                    delete atr;
                    return NULL;
            }

            obj->asignarA( this );
	    observar( obj );
        }

        return obj;
}

// -------------------------------------------------- AlmacenObjetos::getRuta()
std::string AlmacenObjetos::getRuta() const
/**
	Devuelve la ruta hasta este contenedor, SIN INCLUIRLO.
           o lo que es lo mismo, hasta el padre.
	Así, para el contenedor .Exe.C1 se devolverá .Exe

        @return La ruta completa hasta el contenedor, o "" si no es alcanzable.
*/
{
    std::string toret;
    const AlmacenObjetos * cact = this;
    const AlmacenObjetos * raiz = Runtime::rt()->getContainerRaiz();

    if ( cact != raiz )
    {
        // Encontrar los componentes de la ruta
        do
        {
            // Buscar el padre
            cact = getPerteneceA();

            if ( cact != NULL ) {
                // Insertar el separador '.'
                toret.insert( toret.begin(), CHR_SEP_IDENTIFICADOR );

                // Si es el root, no introducir el nombre
                if ( cact != raiz ) {
                    toret = cact->getNombre() + toret;
                }
            }
            else toret.clear();

        } while( cact != raiz
              && cact != NULL );
    }

    return toret;
}

// --------------------------------------------------- AlmacenObjetos::elimina()
void AlmacenObjetos::elimina(Objeto *x)
{
    if ( !( lAtrs.borra( x->getNombre() ) ) ) {
        Runtime::ponExcepcionObjeto( x->getNombre() );
    }
}

// ----------------------------------------------------- AlmacenObjetos::nuevo()
AlmacenObjetos * AlmacenObjetos::nuevo(
        AlmacenObjetos * contPadre,
        const std::string &nombre)
/**
        Crea un nuevo container, teniendo en cuenta si tiene que ser
        transient o no.
*/
{
    AlmacenObjetos * toret;

    // Crear
    try {
        if ( VMCap::PS
          && contPadre != Runtime::rt()->getContainerEjecucion() )
        {
                toret = new(std::nothrow) Container(
                        (Container *) contPadre, nombre, Runtime::objetoContainer
                );
        }
        else {
                toret =
                    new(std::nothrow) TransientContainer(
                            contPadre, nombre, Runtime::objetoContainer
                );
        }
    } catch(...) {
        toret = NULL;
    }

    // Comprobar
    if ( toret == NULL ) {
        Runtime::ponExcepcion( Runtime::objetoExcepcion, EXC_NOMEM );
    }

    return toret;
}

// ================================================================== Container
// ----------------------------------------------------- Container::Container()
Container::Container(Container *pc, const std::string &n, const std::string &p)
    : AlmacenObjetos(pc, n, p)
{
    Runtime::conim->insertaContainer( this );
}

Container::Container(Container *pc, const std::string &n, Objeto * p)
    : AlmacenObjetos(pc, n, p)
{
    Runtime::conim->insertaContainer( this );
}

// ---------------------------------------------- Container::guardaContenedor()
void Container::guardaContenedor()
{
	PersistentStore *ps = PersistentStore::ps();

	if ( ps != NULL )
                // Guarda contenedor (atn. excepcion)
                ps->guardaContenedor( this );
	else 	throw EInterno( "Guardando contenedor sin PS" );
}

// ----------------------------------- Container::cambiarNombreObjetoEnMnemos()
void Container::cambiarNombreObjetoEnMnemos(
					Objeto *obj,
					Atributo * atr,
					Objeto::ListaMnemosObjeto * lmo)
{
	std::string nomPadre;

	if ( obj == NULL ) {
		throw EInterno( "guardando container: cambiando nombre sin objeto de"
					    " referencia"
		);
	}

	if ( atr == NULL ) {
		throw EInterno( "guardando container: cambiando nombre sin atributo"
					    " de referencia"
		);
	}

	if ( lmo == NULL ) {
		throw EInterno( "guardando container: cambiando nombre sin lista "
						"de mnemos de referencia"
		);
	}

	if ( obj->nombre != atr->getNombre() ) {
		if ( dynamic_cast<NMObj *>( (*lmo)[ 0 ] ) != NULL ) {
			// Obtener el nombre del padre
			nomPadre = ( (NMObj *) (*lmo)[ 0 ] )->getNomPadre();
			delete (*lmo)[ 0 ];

			// Crear el nuevo mnemo de Obj
			(*lmo)[ 0 ] = new(std::nothrow)
					NMObj( atr->getNombre(), nomPadre )
			;

			if ( (*lmo)[ 0 ] == NULL ) {
				throw ENoHayMemoria( "guardando container" );
			}
		}
		else throw EInterno( "guardando container, primer mnemo no es OBJ" );
	}
}

// -------------------------------------------------- Container::fueProcesado()
bool Container::fueProcesado(Objeto *obj, const std::string &nombre)
{
	bool toret = false;

	if ( objsProcsStr.find( nombre ) != objsProcsStr.end() )
			toret = true;
	else    objsProcsStr.insert( nombre );

	if ( objsProcsPtr.find( obj ) != objsProcsPtr.end() )
			toret = true;
	else    objsProcsPtr.insert( obj );

	return toret;
}

// -------------------------------------------------------- Container::salvar()
Objeto::ListaMnemosObjeto &Container::salvar(ListaRefExternas &refExt)
{
    // Inicializar
    objsProcsStr.clear();
    objsProcsPtr.clear();
    Objeto::ListaMnemosObjeto *lmo;
    Atributo *atr;
    Objeto *obj;
    register unsigned int numObjsEnCont = lAtrs.getNumero();

    if ( PersistentStore::debeSerGuardado( this ) ) {
        // Limpiar
        mnemosObjeto.clear();
        refsExtObjeto.clear();

        for(register unsigned int i = 0; i < numObjsEnCont; ++i)
        {
            atr = lAtrs.getElementoNumero( i );

			// ¿Es el atributo parent?, ¿Es un objeto del contenedor?
			if ( dynamic_cast<AtributoParent *>( atr ) != NULL
		     || !( atr->swizzled() ) )
			{
				continue;
			}

            // ¿Ha sido procesado ya?
            obj = atr->getObjeto();
			if ( fueProcesado( obj, atr->getNombre() ) ) {
				continue;
			}

            if ( dynamic_cast<ObjetoSistema *>( obj ) == NULL
              && dynamic_cast<AlmacenObjetos *>( obj ) == NULL )
            {
                // Obtener los opcodes
                lmo = &( obj->salvar( refExt ) );

				// Darle el nombre adecuado
				cambiarNombreObjetoEnMnemos( obj, atr, lmo );

				// Insertar los opcodes en la lista de opcodes a guardar
                mnemosObjeto.insert(
                        mnemosObjeto.end(),
                        lmo->begin(),
                        lmo->end()
                );
            }
        }

        // Conseguir las referencias "externas" (i.e., en el Exe)
        Objeto::ListaRefExternas::const_iterator it = refExt.begin();
        for(; it != refExt.end(); ++it )
        {
            // ¿Ha sido procesado ya?
            if ( fueProcesado( *it, (*it)->getNombre() ) ) {
                   	continue;
			}

            lmo = &( (*it)->salvar( refExt ) );

            mnemosObjeto.insert(
                    mnemosObjeto.end(),
                    lmo->begin(),
                    lmo->end()
            );
        }
    }

    return mnemosObjeto;
}

// ------------------------------------------------- ponNoModificadoDesdeCarga()
/** Cambia el indicador de modificado.
  * Sólo puede ser llamado desde el cargador de objetos
  * (al terminar la carga), desde el propio objeto (después
  * de salvar() ) y desde el almacenamiento persistente
  * (al terminar el guardado).
  * Cualquier otra llamada es un error.
*/
void Container::ponNoModificadoDesdeCarga()
{
    Objeto::ponNoModificadoDesdeCarga();
    Runtime::conim->ponSincronizado( this );
}

// ---------------------------------------- Container::ponModificadoDesdeCarga()
void Container::ponModificadoDesdeCarga()
/// Indica que ha sido modificado
{
    Objeto::ponModificadoDesdeCarga();
    Runtime::conim->ponDesSincronizado( this );
}

// ========================================================= TransientContainer
// ----------------------------------- TransientContainer::TransientContainer()
TransientContainer::TransientContainer(
        AlmacenObjetos *pc,
        const std::string &n,
        const std::string &p)
    : AlmacenObjetos(pc, n ,p)
{
    Runtime::conim->insertaTransientContainer( this );
}

TransientContainer::TransientContainer(
        AlmacenObjetos *pc,
        const std::string &n,
        Objeto *p)
    : AlmacenObjetos(pc, n ,p)
{
    Runtime::conim->insertaTransientContainer( this );
}

} // namespace Zero

// referencia.cpp
/*
        Implantación de las referencias.


        Forma parte de zvm

        jbgarcia@uvigo.es
*/

#include "mnemos.h"
#include "referencia.h"
#include "container.h"
#include "runtime.h"
#include "metodos.h"

namespace Zero {

// =========================================== Referencia::ListaIdentificadores
// -------------------------------- Referencia::ListaIdentificadores::reserva()
inline
void Referencia::ListaIdentificadores::reserva(size_t tam)
{
    ids = new(std::nothrow) char *[ tam ];

    if ( ids == NULL ) {
        throw ENoHayMemoria( "procesando ref(reservando)" );
    }
}

// ----------------------------- Referencia::ListaIdentificadores::calculaIds()
inline
void Referencia::ListaIdentificadores::calculaIds(const std::string & ref)
{
    char * ptr;
    const char * org;
    unsigned int tamRef = ref.size();
    unsigned int posIds[ tamRef + 1 ];

    init();
    lista = new(std::nothrow) char[ tamRef + 1 ];

    if ( lista != NULL ) {
        // Copiar la cadena, hallando el num de ids.
        num = 1;
        ptr = lista;
        org = ref.c_str();
        posIds[ 0 ] = 0;
        while( ( *ptr = *org ) != '\0' ) {
            if ( *ptr == CHR_SEP_IDENTIFICADOR ) {
                // Guardar la posición del id
                posIds[ num++ ] = ( ( ptr + 1 ) - lista );

                // Cambiar el punto por un 0
                *ptr = 0;
            }
            ++ptr, ++org;
        }

        // Reservar la memoria necesaria
        reserva( alinea( num ) );

        // Establecer los id's
        for(unsigned int i = 0; i < num; ++i) {
            // Guardar índice de id en vector de posiciones
            ids[ i ] =  lista + posIds[ i ];
        }

        // Si la referencia es '.'
        if ( tamRef == 1
          && *lista == CHR_SEP_IDENTIFICADOR )
        {
            num = 1;
        }
    }
}

// ================================================================= Referencia
// ----------------------------------------------------- Referencia::Referencia
/*
        ObjAlQuePertenece
                siempre es distinto de NULL, ya que una referencia
                siempre está dentro de un objeto.
        MetAlQuePertenece
                puede ser NULL si la referencia es un atributo

        Si el puntero dir contiene NULL, entonces es que no se ha hecho
        swizzling. En caso contrario, sí se ha hecho.
*/

Referencia::Referencia(Objeto *po, Metodo *mo, const std::string &n, Objeto *d, bool prop)
        : ParNombreObjeto<Objeto>(n, d), esPropietario(prop), objAlQuePertenece(po),
          metAlQuePertenece(mo)
{
	if ( esPropietario ) {
		d->incrementaReferencias();
	}
}

Referencia::Referencia(Objeto *po, Metodo *mo, const std::string &n, const std::string &o, bool prop)
        : ParNombreObjeto<Objeto>(n, NULL), esPropietario(prop), objetoNoSwizz(o),
          objAlQuePertenece(po), metAlQuePertenece(mo)
{
}

// ----------------------------------------------------- Referencia::~Referencia
Referencia::~Referencia()
{
	if ( swizzled() ) {
		if ( esPropietario ) {
			dir->decrementaReferencias();
		}
	}
}

// ----------------------------------------------------- Referencia::alCambiar()
inline
void Referencia::alCambiar()
{
    metAlQuePertenece->deSincroniza();
}

// --------------------------------------------------- Referencia::ponReferencia
void Referencia::ponReferencia(Objeto *x)
{
	Objeto * antDir = dir;

	// Incrementar el nuevo
    objetoNoSwizz.clear();
    dir = x;
	if ( esPropietario ) {
		dir->incrementaReferencias();
	}

	// Decrementar el antiguo
	if ( antDir != NULL ) {
		if ( esPropietario ) {
			antDir->decrementaReferencias();
		}
	}

	alCambiar();
}

// --------------------------------------------------- Referencia::ponReferencia
void Referencia::ponReferencia(const std::string &obj)
{
		if ( swizzled()
		&& esPropietario )
		{
				dir->decrementaReferencias();
		}

        lIds.init();
        objetoNoSwizz = obj;
        dir           = NULL;

		alCambiar();
}

// ------------------------------------- Referencia::getNombreObjetoReferenciado
const std::string &Referencia::getNombreObjetoReferenciado()
{
        if ( objetoNoSwizz.empty() ) {
                if ( swizzled() ) {
                        objetoNoSwizz = dir->getNombre();
                }
        }

        return objetoNoSwizz;
}

// ---------------------------------------------- Referencia::buscaVbleOAtributo
Referencia *Referencia::buscaVbleOAtributo(Objeto *od, Metodo *md,
                                           const std::string &nombre)
{
        Objeto * objThis = od;
        Referencia *toret = NULL;

	if ( !( nombre.empty() ) )
	{
		// Quizás, es una variable local del método
                if ( md != NULL ) {
                    toret = md->vblesLocales.busca( nombre );
                }

		if ( toret == NULL )
		{
                        if ( md != NULL ) {
                            objThis = md->getLocThis()->getObjeto();
                        }

			toret = buscaAtributo( objThis, nombre );

			// A lo mejor, es un atributo del objeto
			while ( toret   == NULL
                            &&  objThis != NULL
			    &&  objThis != Runtime::objetoRaiz )
			{
                                if ( objThis->getAtrObjetoPadre() != NULL )
                                    objThis = objThis
                                                    ->getAtrObjetoPadre()
                                                    ->getObjeto()
                                    ;
                                else    objThis = NULL;

                                if ( objThis != NULL ) {
                                    toret = buscaAtributo( objThis, nombre );
                                }
			}
		}
	}

        return toret;
}

// -------------------------------- Referencia::buscaObjetoEnContainerActualYLib
Referencia *
Referencia::buscaObjetoEnContainerActualYLib(
                AlmacenObjetos *cd,
                const std::string &nombreObj)
/**
	Cuando una referencia no está cualificada,
        entonces se busca primero en el
	contenedor actual y después en el de la librería.

	Es posible que una referencia no tenga un
        contenedor asociado (NULL), al ser posible
	tener objetos fuera de cualquier contenedor.
        Estos objetos son aquellos que
 *      SEGURO, SIN DUDA, son temporales (transient), por ahora, y
 *      se considera pertenecen a Exe.
 *
 *      @param cd El container donde se ubica la referencia
 *      @param nombreObj el nombre del objeto a buscar
 *      @return La referencia al objeto en el container, o NULL
*/
{
    Referencia * toret;

    if ( cd == NULL ) {
            cd = Runtime::rt()->getContainerEjecucion();
    }

    toret = buscaAtributo( cd, nombreObj );

    if ( toret == NULL ) {
            toret = Runtime::rt()->getContainerIntStdLib()->lAtrs.busca(
                        nombreObj
            );
    }

    return toret;
}

// ------------------------------------------------- Referencia::buscaAtributo()
inline
Referencia * Referencia::buscaAtributo(Objeto *od, const std::string &nombre)
{
        Referencia * toret = od->lAtrs.busca( nombre );

        if ( toret == NULL ) {
                toret = cargaContenedor( od, nombre );
        }

        return toret;
}

// ----------------------------------------------- Referencia::cargaContenedor()
inline
Referencia * Referencia::cargaContenedor(Objeto *od, const std::string &nombre)
{
    Referencia * toret = NULL;

    if ( VMCap::PS ) {
        AlmacenObjetos * contDest;
        Container * cont = dynamic_cast<Container *>( od );

        if( cont != NULL ) {
            try {
                contDest = Runtime::ps->cargaContenedor( nombre, cont, NULL );

                if ( contDest != NULL ) {
                    toret = od->lAtrs.busca( nombre );
                }
            }
            catch(const EInputOutput &e) {
                toret = NULL;
            }
            catch(...) {
                throw;
            }
        }
    }

    return toret;
}

// ----------------------------------------------- Referencia::buscaReferencia()
Referencia *
Referencia::buscaReferencia(const std::string &nombreRef, Objeto *od, Metodo *md)
/**
 * Este método es un envoltorio conveniente del algoritmo real.
 * @see buscaReferencia( ListaIdentificadores &, Objeto *, Metodo *)
*/
{
      if ( !nombreRef.empty() ) {
        // Obtener las Ids del objeto xx.yy.zz. ...
        ListaIdentificadores lIds( nombreRef );

       return buscaReferencia( lIds, od, md );
      }

      return NULL;
}

Referencia *
Referencia::buscaReferencia(
        const ListaIdentificadores &lIds,
        Objeto *od,
        Metodo *md)
/**
 * Éste es el algoritmo principal de swizzling:
 * busca la referencia a través de los componentes ya divididos
 * (sólo se dividen una vez)
 * @param lIds La lista de identificadores de la referencia xx.y -> {xx,y}
 * @param od Es el objeto en el que reside la referencia
 * @param md Es el método en el que reside la referencia (NULL para attrs.)
 * @return La referencia que está justo antes de llegar al objetivo
*/
{
      Referencia           *toret = NULL;
      Objeto               *obj   = NULL;
      unsigned int         idN    = 0;
      unsigned int         numIds = lIds.getNumero();

      // Buscar el primer objeto
      // Si la búsqueda es global, entonces empezar en el contenedor raíz
      // La búsqueda es global cuando el identificador empieza por '.' :
      // ".IntStdLib.System"
      if ( *( lIds.getId( 0 ) ) == '\0' )
      {
      	   ++idN;

	   // Es la referencia a '.', el contenedor root ?
	   if ( numIds == 1 )
	   {
	   	++idN;
		toret = Runtime::rt()->getRefAContainerRaiz();
	   }
	   else {
             	toret = buscaObjetoEnContainerActualYLib(
	     		        Runtime::rt()->getContainerRaiz(),
                                lIds.getId( 1 ) )
		;
	   }
      }
      else {
        // ¿Es atributo?
        if ( md == NULL )
                toret = buscaObjetoEnContainerActualYLib(
                                od->getPerteneceA(), lIds.getId( 0 ) )
		;
        else    toret = hazSwizzlingDirecto( od, md, lIds.getId( idN ) );
      }

      if ( toret != NULL )
      {
          // Buscar el siguiente ID (atributo)
	  if ( idN < numIds ) {
          	obj = toret->getObjeto();
          	++idN;
	  }

          while( idN < numIds )
          {
              toret = buscaAtributo( obj, lIds.getId( idN ) );

              if ( toret != NULL ) {
                    if ( Atributo::chkVisible( (Atributo *) toret, od ) ) {
                            obj = toret->getObjeto();
                    }
                    else {
                        obj = Runtime::objetoNulo;
                        Runtime::ponExcepcion(
                                Runtime::objetoEPrivado,
                                EXC_NOVISIBLE + toret->getNombre() + '\''
                        );
                    }
              }
              else {
                    toret = NULL;
                    Runtime::ponExcepcionObjeto( lIds.getId( idN ) );

                    break;
              }

              ++idN;
          }
      } else {
              toret = NULL;
              Runtime::ponExcepcionObjeto( lIds.getId( idN ) );
      }

      return toret;
}

// ---------------------------------------------------- Referencia::hazSwizzling
void Referencia::hazSwizzling()
{
        // Hay algo que hacer ?
        if ( swizzled() ) {
            return;
        }

        Referencia *ref;

        // En qué objeto está esta referencia ?
        Objeto    *od  = getPerteneceAlObjeto();
        Metodo    *md  = getPerteneceAlMetodo();

        if ( lIds.getNumero() == 0 ) {
            // Crear los identificadores de la referencia
            lIds.calculaIds( objetoNoSwizz );
        }

        // Buscar la referencia en cuestión
        ref = buscaReferencia( lIds, od, md );

        // ¿Encontrada?
        if ( ref != NULL )
            dir = ref->getObjeto();
        else {
            dir = Runtime::objetoNulo;
            Runtime::ponExcepcionObjeto( getNombreObjetoReferenciado() );
        }

	return;
}

// --------------------------------------------- Referencia::hazSwizzlingDirecto
Referencia *
Referencia::hazSwizzlingDirecto(Objeto *od, Metodo *md, const std::string &n)
/**
        Hace el swizzling de esta misma referencia.
*/
{
        Referencia *toret;

        // ¿ Es una variable local del método o un atributo ?
        if ( ( toret = buscaVbleOAtributo( od, md, n ) ) == NULL )
        {
            // En otro caso, busca en el contenedor actual o en el
            // de la librería
            toret = buscaObjetoEnContainerActualYLib( od->getPerteneceA(), n );
        }

        return toret;
}

// ---------------------------------------------------- Referencia::getObjeto()
Objeto *Referencia::getObjeto()
{
	if ( !swizzled() ) {
		hazSwizzling();

		if ( esPropietario ) {
			dir->incrementaReferencias();
		}
	}

	if ( !( dir->estaVivo() ) ) {
			dir = Runtime::objetoNulo;
			dir->incrementaReferencias();
	}

	return dir;
}

// --------------------------------------------------------- Referencia::copia()
Referencia *Referencia::copia(Objeto *obj, Metodo *met)
{
        try {
            Referencia * toret = new Referencia( *this );

            toret->objAlQuePertenece  = obj;
            toret->metAlQuePertenece  = met;

            return toret;
        }
        catch(const std::bad_alloc &)
        {
                Runtime::ponExcepcion(Runtime::objetoExcepcion, "Sin memoria");
                return NULL;
        }
}

// ============================================================= RefParadojaRaiz
RefParadojaRaiz::RefParadojaRaiz(AlmacenObjetos *root)
	:  Referencia(NULL, NULL, root->getNombre(), root)
{}

inline
Objeto *RefParadojaRaiz::getObjeto() {
	return dir;
}

void RefParadojaRaiz::hazSwizzling()
{
    throw EInterno( "ref paradoja invocada" );
}

void RefParadojaRaiz::ponReferencia(Objeto *)
{
    throw EInterno( "ref paradoja invocada" );
}

void RefParadojaRaiz::ponReferencia(const std::string &)
{
    throw EInterno( "ref paradoja invocada" );
}

} // namespace Zero


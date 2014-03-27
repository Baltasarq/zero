// persistentstore.cpp
/*
	Representa al almacenamiento persistente.
	Es el encargado de recuperar contenedores y guardarlos a memoria
	secundaria.

	jbgarcia@uvigo.es
*/

#include "persistentstore.h"
#include "excep.h"
#include "reservadas.h"
#include "cargaobj.h"
#include "directorio.h"
#include "runtime.h"
#include "vmcap.h"

namespace Zero {

// ============================================================ PersistentStore
// ------------------------------------------------------ PersistentStore::ps()
PersistentStore *PersistentStore::ps()
{
	return PersistentStoreEnDisco::ps();
}

// ---------------------------------------- PersistentStore::guardaContenedor()
void PersistentStore::guardaContenedor(Container *c)
/** No realiza todavía ninguna operación de guardado, sino que organiza
  * todos los opcodes del contenedor y los deja listos para guardar
  * en mnemosContainer.
*/
{
    // Limpiar los mnemos y las referencias externas
    mnemosContainer.clear();
    refExt.clear();

    // ¿Hay algo que hacer?
    if ( debeSerGuardado( c ) ) {
        // Conseguir todos los objetos del contenedor
        Objeto::ListaMnemosObjeto *lmo = &( c->salvar( refExt ) );
        mnemosContainer.insert(
                        mnemosContainer.end(),
                        lmo->begin(),
                        lmo->end()
        );
    }

    return;
}

// ===================================================== PersistentStoreEnDisco
PersistentStoreEnDisco * PersistentStoreEnDisco::perstor = NULL;
char PersistentStoreEnDisco::barraDir = VMCap::getDirMark();

// --------------------------- PersistentStoreEnDisco::PersistentStoreEnDisco()
PersistentStoreEnDisco::PersistentStoreEnDisco(const std::string & dirDePS)
{
	// ¿Dónde está inicialmente?
	if ( dirDePS.empty() ) {
        dirBase = Directorio::getNombreDirectorioActual();
	} else {
	    dirBase = dirDePS;
	}

	// ¿Es necesario crear el PS?
	dirPS = Directorio(
		dirBase.getNombre() + NOMBRE_DIR_PS,
		Directorio::CREAR
	);
}

PersistentStoreEnDisco::~PersistentStoreEnDisco()
/**
 *      Al eliminar, guardar los contenedores en memoria.
 *      Pero ésto se hace desde el Runtime.
 */
{

}

// -------------------------------------------------------- PersistentStore::ps
PersistentStoreEnDisco * PersistentStoreEnDisco::ps()
/** 	Singleton pattern
	Creamos un PersistentStoreEnDisco, que es el que se va a utilizar.
	Se le pasa la dirección del almacenamiento persistente.
*/
{
	if ( perstor == NULL ) {
		perstor = new(std::nothrow) PersistentStoreEnDisco( VMCap::PSDir );

		if ( perstor == NULL ) {
			throw ENoHayMemoria( EXC_NOMEM.c_str() );
		}
	}

	return perstor;
}

// ----------------------------------------- PersistentStore::cargaContenedor()
Container *
PersistentStoreEnDisco::cargaContenedor(
            const std::string &nom,
            Container * padre,
            Container * hijo,
            bool esLib )
/**
	Carga un container dado por un nombre directorio de un container
	padre pasado por parámetro. Lo inserta en el contenedor padre.
	@param nom el nombre del contenedor a cargar
	@param padre un puntero al contenedor padre
        @param hijo, un puntero al contenedor donde cargar, si es NULL, se crea.
	@return Un container si éxito, si fracaso:
        <ul>
                NULL si el contenedor no es alcanzable (no está guardado)
                NULL + EMedioNoEncontrado si no existe el contenedor
                NULL + ENoHayMemoria si no se puede crear el hijo
                NULL + EInterno de cualquier otra forma.
        </ul>
*/
{
    Container *toret = NULL;
    std::string ruta;

    if ( !( padre->esAlcanzable() ) ) {
        toret = NULL;
        goto END;
    }

    ruta = padre->getRuta();

    if ( padre == NULL ) {
        padre = (Container *) Runtime::rt()->getContainerRaiz();
    }

    if ( ( toret = ( (Container *) padre->busca( nom ) ) ) == NULL )
    {
        try {
            // Obtener la ruta definitiva
            cnvtRutaEnDir( ruta );
            ruta += padre->getNombre();

            // Buscar el directorio
            Directorio dir( ruta );

            if ( dir.existeArchivo( nom + EXT_ARCHIVOS_ZERO ) ) {

                // Crear el container
                if ( hijo == NULL )
                        toret = (Container *) AlmacenObjetos::nuevo( padre, nom );
                else    toret = hijo;

                if ( toret == NULL ) {
                    throw ENoHayMemoria( std::string( "creando " + nom ).c_str() );
                }

                // Insertarlo en el padre
                padre->inserta( toret );

                try {
                        // Cargarlo
                        cargarContenedorDeFichero( *toret, dir, esLib );
                        toret->ponNoModificadoDesdeCarga();
                }
                catch(...) {
                        toret->ponNoModificadoDesdeCarga();
                        padre->elimina( toret );
                        toret = NULL;
                        throw;
                }

            } else throw EMedioNoEncontrado( nom.c_str() );
        }
        catch( Excepcion &e ) {
            delete toret;
            throw;
        }
        catch( ... ) {
            delete toret;
            throw EInterno( std::string( "Cargando" + ruta ).c_str() );

        }
    }

    END:
    return toret;
}

// ------------------------- PersistentStoreEnDisco::cargaContenedorDeFichero()
void PersistentStoreEnDisco::cargarContenedorDeFichero(
				Container &c,
				const Directorio &dir,
                                bool esLib)
/**
 * 	Toma un contenedor y lo llena con los objetos
 * 	del fichero .zbj del mismo nombre.
 * 	Puede provocar una excepción al no encontrar el fichero.
 */
{
	std::string nomArchivoCont = dir.getNombre()
			+ c.getNombre()
			+ EXT_ARCHIVOS_ZERO
	;

	FicheroSoporte ms( nomArchivoCont, FicheroSoporte::Existente );

	CargadorObjetos cargador( &ms );

	Runtime::rt()->prepara(
		cargador.getObjetosCargados(),
		esLib,
		NULL,
		&c
	);
}

// ---------------------------------- PersistentStoreEnDisco::salvarEnFichero()
void PersistentStoreEnDisco::salvarContenedorEnFichero(
				const Container &c,
				const Directorio &dirCont,
				Objeto::ListaMnemosObjeto &mnemosContainer )
/**
 * La escritura de archivos es atómica, es decir, hasta que no se completa el
 * guardado del archivo, no se destruye el contenedor anterior.
 *
*/
{
	// Nombre verdadero
	std::string nombreArchivoFinal = dirCont.getNombre()
		+ c.getNombre()
		+ EXT_ARCHIVOS_ZERO
	;

	// Nombre temporal
	std::string nombreArchivo = nombreArchivoFinal + EXT_ARCHIVOS_TEMP_ZERO;

	// Hacer el guardado del archivo
	{
	  FicheroSoporte f( nombreArchivo, FicheroSoporte::Nuevo );

          Cabecera().escribe( &f );

	  Objeto::ListaMnemosObjeto::const_iterator it = mnemosContainer.begin();
	  for( ; it != mnemosContainer.end(); ++it ) {
		(*it)->escribe( &f );
	  }
	}

	// Una vez guardado el archivo, darle el verdadero nombre (atómico)
	VMCap::moverArchivo( nombreArchivo, nombreArchivoFinal );
}

// ---------------------------------------- PersistentStore::guardaContenedor()
void PersistentStoreEnDisco::guardaContenedor(Container *c)
{
    // ¿Hay algo que hacer?
    if ( debeSerGuardado( c ) ) {
        PersistentStore::guardaContenedor( c );

        // Guardar en disco
        std::string ruta = c->getRuta();
        cnvtRutaEnDir( ruta );

        // Llegar hasta el directorio del contenedor  (atn excepción)
        Directorio dirCont = Directorio( ruta );

        // Guardar archivo
        salvarContenedorEnFichero( *c, dirCont, mnemosContainer );
        c->ponNoModificadoDesdeCarga();
    }

    return;
}

// ------------------------------------ PersistentStoreEnDisco::cnvtRutaEnDir()
std::string &PersistentStoreEnDisco::cnvtRutaEnDir(std::string &ruta)
/**
 * A partir de una ruta de un contenedor, construye el directorio
 * correspondiente
 * @param ruta
 * @return la ruta ya convertida en directorio (modificada "in situ")
 */
{
    // Ver si es la raiz (entonces no hay que hacer nada)
    if ( !ruta.empty() ) {
        ruta = OBJ_ROOT + cnvtSeparadoresEnBarraDirs( ruta );
    }

    ruta = dirPS.getNombre() + ruta;

    return ruta;
}

// ------------------------------------ PersistentStoreEnDisco::cnvtDirEnRuta()
std::string &PersistentStoreEnDisco::cnvtDirEnRuta(std::string &ruta)
/**
 * A partir de una ruta completa a un directorio, construye el directorio
 * correspondiente
 * @param ruta
 * @return el directorio ya convertido en ruta (modificada "in situ")
 */
{
    // Convertir en directorio y quitar el directorio del PS (más la barra)
    cnvtBarraDirsEnSeparadores( ruta );
    ruta.erase( 0, dirPS.getNombre().size() );

    // Si queda algo, no es el raiz.
    if ( !ruta.empty() ) {
        ruta.erase( 0, OBJ_ROOT.size() );
    }

    return ruta;
}

// ---------------------------------- PersistentStoreEnDisco::cnvtSeparadores()
std::string &PersistentStoreEnDisco::cnvtSeparadores(std::string &ruta, char sp1, char sp2)
{
    size_t posSep = ruta.find( sp1 );

    while( posSep != std::string::npos ) {
        ruta[posSep] = sp2;
        posSep = ruta.find( sp1, posSep + 1 );
    }

    return ruta;
}

} // namespace Zero

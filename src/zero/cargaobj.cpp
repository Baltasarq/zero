// cargaobj.cpp
/*
        El cargador de objetos "desaplana" los objetos en disco para pasarlos
        a memoria (unflattening).
*/

#include "cargaobj.h"
#include "metodos.h"
#include "runtime.h"

namespace Zero {

// ============================================================ CargadorObjetos
// ----------------------------------------- CargadorObjetos::CargadorObjetos()
CargadorObjetos::CargadorObjetos(MedioSoporte *ms)
        : m(ms)
{
        compruebaOrigen();
}

// -------------------------------------- CargadorObjetos::getObjetosCargados()
CargadorObjetos::ListaObjetos *CargadorObjetos::getObjetosCargados()
/**
	Devuelve los objetos almacenados en el cargador de objetos.
	Si todavía no han sido cargados, los carga
*/
{
	if ( vObj.empty() ) {
		unflatten();
	}

	return &vObj;
}

// -------------------------------------------- CargadorObjetos::eliminaTodos()
void CargadorObjetos::eliminaTodos()
/**
	Elimina los objetos almacenados en el cargador de objetos
*/
{
	if ( vObj.size() > 0 ) {
		// Eliminar cada objeto
		ListaObjetos::const_iterator it = vObj.begin();
		for (;it != vObj.end(); ++it) {
				delete it->second;
		}

		// Borrar las posiciones
		vObj.clear();
	}

	// Las listas de identificadores
	idsAtributos.clear();
	idsMetodos.clear();

	// Lista de métodos
	mnemosMet.clear();
}

// ----------------------------------------------- CargadorObjetos::unflatten()
void CargadorObjetos::unflatten()
/**
      Pasa los opcodes a objetos.
      Se trata realmente de identificar los objetos,
      los atributos, y los métodos, y construir las clases adecuadas.
*/
{
        Mnemotecnico *it;

        // Reiniciar lista
        objetoActual = NULL;
        metodoActual = NULL;
        eliminaTodos();

        // Leer los opcodes y crear los objetos necesarios
        try {
          it = getSigMnemo();
          while( it != NULL )
          {
                if ( dynamic_cast<CtrOb *>( it ) != NULL )
                {
                    // Es un opcode de ctrl de objetos: empezar, terminar, atr.
                    if ( dynamic_cast<NMEno *>( it ) != NULL )
                    {
			guardaObjeto();
                    }
                    else if ( dynamic_cast<NMObj *>( it ) != NULL ){
			registraNuevoObjeto( ( (NMObj *) it ) );
                    }
                    else if ( dynamic_cast<NMAtr*>( it ) != NULL ) {
			guardaAtributo( ( (NMAtr *) it ) );
                    }
                    else throw EOpcodeIlegal( "cargando opcode de control de"
					      "objetos desconocido"
			  );
                }
                else
                if ( dynamic_cast<CtrMt *>( it ) != NULL )
                {
                    // Es un opcode de ctrl de métodos: empezar, terminar.
                    if ( dynamic_cast<NMEnm *>( it ) != NULL )
                    {
			guardaMetodo();
                    }
                    else
                    if ( dynamic_cast<NMMth *>( it ) != NULL )
                    {
			registraNuevoMetodo( (NMMth *) it );
                    }
                    else throw EOpcodeIlegal( "cargando control de métodos" );
                }
                else
                if ( dynamic_cast<NMMta *>( it ) != NULL
                  && objetoActual != NULL
                  && metodoActual == NULL )
                {
			guardaMeta( ( (NMMta *) it ) );
			it = NULL;
                }
                else
                if ( dynamic_cast<Instr *>( it ) != NULL
                  || dynamic_cast<NMMta *>( it ) != NULL )  // los MTA@método
                {
                        if ( metodoActual == NULL ) {
                                throw EIlegal( "cargando opcode fuera de Método" );
                        }

                        mnemosMet.push_back( it );
                }
                else throw EOpcodeIlegal( "cargando instrucciones" );

                // Borrar si no es instr ni meta (si no, está en la lista)
                if ( dynamic_cast<Instr *>( it ) == NULL
                  && dynamic_cast<NMMta *>( it ) == NULL )
                {
                        delete it;
                }

                it = getSigMnemo();
          }
        }
	catch(const Excepcion &e) {
		throw;
	}
	catch(...) {
                throw EInterno( "FATAL cargando objetos" );
    }

	// Comprobar que todos los objetos cargados están cerrados
	if ( objetoActual != NULL
	  || metodoActual != NULL )
	{
		throw ENoEsZero( "cargando, fin inesperado de objeto" );
	}

	// Comprobar herencias cíclicas
	compruebaHerenciaCiclica();
}

// ----------------------------------------- CargadorObjetos::compruebaOrigen()
void CargadorObjetos::compruebaOrigen() throw (ENoEsZero, EVersionIlegal)
/**
	Comprueba que el origen de los objetos sea correcto.
	En este caso, se trata de comprobar si el fichero está
	correctamente abierto.
*/
{
      Cabecera cab;

      // Leer la cabecera
      cab.lee( m );

      // Comprobar cabecera
      if ( cab.esZeroLegal() )
      {
        // Comparar las versiones
        if ( cab.hiVersion() > hver )
        {
                std::string version;

                // Construye la comparación de versiones
                version = "carga, error de versionado: ";
                version.append( Zero::AnalizadorLexico::toString( cab.hiVersion(), 2 ) );
                version.push_back( '.' );
                version.append( Zero::AnalizadorLexico::toString( cab.loVersion(), 2 ) );
                version.push_back( '/' );
                version.append( Zero::AnalizadorLexico::toString( (unsigned int) hver, 2 ) );
                version.push_back( '.' );
                version.append( Zero::AnalizadorLexico::toString( (unsigned int) lver, 2 ) );

                throw Zero::EVersionIlegal( version.c_str() );
        }
      }
      else throw Zero::ENoEsZero( "comprobando al cargar origen" );

      return;
}

// ----------------------------------------- CargadorObjetos::compruebaOrigen()
Mnemotecnico *CargadorObjetos::getSigMnemo()
{
	if ( !m->esFinal() )
		return Mnemotecnico::cargar( m );
	else	return NULL;
}

// -------------------------------------- CargadorObjetos::registraNuevoObjeto()
void CargadorObjetos::registraNuevoObjeto(NMObj *mnemo)
{
	if ( mnemo == NULL ) {
		throw EOpcodeIlegal( "cargando: mnemo OBJ nulo" );
	}

	if ( objetoActual != NULL ) {
		throw EOpcodeIlegal( "cargando: OBJ antes de ENO" );
	}

	metodoActual = NULL;

	if ( mnemo->getNombre() == ATR_PARENT ) {
		throw EIlegal( "cargando: objeto nombrado como atributo parent" );
	}

	if ( vObj.find( mnemo->getNombre() ) == vObj.end() ) {
		objetoActual = new(std::nothrow)
			Objeto( NULL, mnemo->getNombre(), mnemo->getNomPadre() )
		;

		if ( objetoActual == NULL ) {
			throw ENoHayMemoria( std::string( "cargando objeto: "
				+ mnemo->getNombre() ).c_str()
			);
		}
	} else throw EIlegal( std::string( "cargando objeto duplicado: " + mnemo->getNombre() ).c_str() );
}

// -------------------------------------------- CargadorObjetos::guardaObjeto()
void CargadorObjetos::guardaObjeto()
{
	// Eliminar los registros de ids
	idsMetodos.clear();
	idsAtributos.clear();

	// Comprobar
	if ( objetoActual == NULL ) {
		throw EOpcodeIlegal( "cargando: ENO antes de OBJ" );
	}

	// Guardar el método de inicialización
	Metodo * met = objetoActual->lMets.busca( MET_ALCARGAR );
	if ( met != NULL ) {
		Runtime::rt()->guardaMetInic( met );
	}

	// Guardar
	vObj.insert(
		ListaObjetos::value_type( objetoActual->getNombre(), objetoActual )
	);
	objetoActual->ponNoModificadoDesdeCarga();
	objetoActual = NULL;
	metodoActual = NULL;
}

// ------------------------------------------ CargadorObjetos::guardaAtributo()
void CargadorObjetos::guardaAtributo(NMAtr *mnemo)
{
	if ( idsAtributos.find( mnemo->getNombre() ) == idsAtributos.end() ) {

		if ( mnemo->getNombre() == ATR_PARENT ) {
			throw EIlegal( "cargando: objeto nombrado como atributo parent" );
		}

		idsAtributos.insert( mnemo->getNombre() );

		Atributo *atr = new(std::nothrow) Atributo(
			objetoActual,
			mnemo->getNombre(),
			mnemo->getRef(),
			mnemo->esAccesoPublico()
		);

		if ( atr == NULL ) {
			throw ENoHayMemoria( std::string( "cargando atributo"
					    + mnemo->getNombre() ).c_str()
			);
		}

		objetoActual->lAtrs.inserta( mnemo->getNombre(), atr );
	}
	else throw EIlegal( std::string( "cargando objeto duplicado: " + mnemo->getNombre() ).c_str() );
}

// ------------------------------------- CargadorObjetos::registraNuevoMetodo()
void CargadorObjetos::registraNuevoMetodo(NMMth * met)
{
	if ( idsMetodos.find( met->getNombre() ) == idsMetodos.end() ) {
		Metodo::Acceso ac = Metodo::getAccesoCorrespondiente( met );

		if ( metodoActual != NULL ) {
			throw EOpcodeIlegal( "cargando MTH antes de ENM" );
		}

		metodoActual = new(std::nothrow) Metodo(
				met->getNombre(),
				objetoActual,
				*(met->getArgs()), ac
		);

		if ( metodoActual == NULL ) {
			throw ENoHayMemoria( std::string( "cargando método "
					    + met->getNombre() ).c_str()
			);
		}
	}
	else throw EIlegal( std::string( "cargando método duplicado: " + met->getNombre() ).c_str() );
}

// -------------------------------------------- CargadorObjetos::guardaMetodo()
void CargadorObjetos::guardaMetodo()
{
	if ( metodoActual == NULL ) {
		throw EOpcodeIlegal( "cargando ENM antes de MTH" );
	}

	// Guardar
	metodoActual->masInstrucciones( mnemosMet );
	objetoActual->lMets.inserta( metodoActual->getNombre(), metodoActual );

	// Limpiar
	metodoActual = NULL;
	mnemosMet.clear();
}

// ---------------------------------------------- CargadorObjetos::guardaMeta()
void CargadorObjetos::guardaMeta(NMMta * meta)
{
	// El MTA describe propiedades del objeto
	if ( meta->getPragma() == NMMta::Objeto ) {
		// Es una property a añadir
		objetoActual->masPropiedades( meta );
		delete meta;      // Meta no se borran por defecto
		meta = NULL;
	} else {
		// Es otra cosa
		throw EChkMetaInvalido(
			"cargando: pragmas internos en objetos"
			" no soportados"
		);
	}
}

// -------------------------------- CargadorObjetos::compruebaHerenciaCiclica()
void CargadorObjetos::compruebaHerenciaCiclica()
{
	std::set<std::string> nomObjs;
	std::string nomPadre;
	Objeto * obj;
	ListaObjetos::const_iterator it = vObj.begin();
	ListaObjetos::const_iterator busq;

	for(; it != vObj.end(); ++it) {
		obj = it->second;

		// Seguir los enlaces de herencia
		nomObjs.clear();
		nomPadre = obj->getAtrObjetoPadre()->getNombreObjetoReferenciado();
		busq = vObj.find( nomPadre);
		nomObjs.insert( obj->getNombre() );

		while( busq != vObj.end()
		    && nomPadre != OBJ_OBJECT )
		{
			if ( nomObjs.find( nomPadre ) == nomObjs.end() )
					nomObjs.insert( nomPadre );
			else 	throw EChkHerenciaCiclica( std::string( "en " + it->second->getNombre() ).c_str() );

			obj = busq->second;
			nomPadre = obj->getAtrObjetoPadre()->getNombreObjetoReferenciado();
			busq = vObj.find( nomPadre);
		}
	}
}

}

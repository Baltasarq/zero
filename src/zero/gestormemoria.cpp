// gestormemoria.cpp

#include "excep.h"

// ===================================================================== Pagina
// ----------------------------------------------------------- Pagina::Pagina()
Pagina::Pagina(unsigned int max) 
	: maximo(max)
{
	// Crear el espacio en memoria
	ptr = (T*) malloc( sizeof( T ) * maximo );
	
	if ( ptr == NULL ) {
		throw ENoHayMemoria( "creando página de memoria" );
	}
	
	// Preparar la lista de huecos disponibles
	for( unsigned int i = 0; i < maximo; ++i)
	{
		huecosDisponibles.push_back( i );
	}
}

Pagina::~Pagina()
{
	free( espacio );
}

// ------------------------------------------------------------ Pagina::crear()
T *Pagina::crear()
{
	T* toret = NULL;
	
	if ( hayMemoriaDisponible() )
	{
		toret = espacio + ( *huecosDisponibles.begin() );
		huecosDisponibles.erase( huecosDisponibles.begin() );
	}
	
	return toret;
}

// --------------------------------------------------------- Pagina::eliminar()
void Pagina::eliminar(T* x)
{
	if ( pertenece( x ) )
	{
		huecosDisponibles.push_back( x - espacio );
	}
	else throw EMemPaginaErronea( "eliminando objeto" );
}

// ============================================================== GestorMemoria
// --------------------------------------------- GestorMemoria::GestorMemoria()
GestorMemoria::GestorMemoria()
{
	crearNuevaPagina();
}

GestorMemoria::~GestorMemoria()
{
	for( unsigned int i = 0 ; i < paginasMemoria.size(); ++i)
	{
		delete p;
	}
}

// ------------------------------------------ GestorMemoria::crearNuevaPagina()
T* GestorMemoria::crearNuevaPagina()
{
	Pagina * p = new Pagina();
	
	if ( p != NULL )
	{
		paginasMemoria.push_back( p );
	}
	else throw ENoHayMemoria( "creando página para gestor de memoria" );
	
	return p;
}

// ----------------------------------------------------- GestorMemoria::crear()
T* GestorMemoria::crear()
{
	Pagina * p = paginasMemoria[ paginasMemoria.size() - 1];
	
	// Comprobar si hay memoria disponible en página actual
	if ( !p->hayMemoriaDisponible() ) {
		p = NULL;
		
		// ... y en el resto de las páginas ?
		for( unsigned int i = 0; i < paginasMemoria.size() ) {
			if ( paginasMemoria[i].hayMemoriaDisponible() )
			{
				p = paginasMemoria[i];
				break;
			}
		}
		
		// ... no hay. Crear nueva página
		if ( p == NULL) {
			p = crearNuevaPagina();
		}
	}
	
	return p->crear();
}

// -------------------------------------------------- GestorMemoria::eliminar()
void GestorMemoria::eliminar(T * x)
{
	if ( paginasMemoria.size() == 1 )
	{
		paginasMemoria[0]->eliminar( x );
	}
	else
	if ( paginasMemoria.size() == 2 )
	{
		if ( paginasMemoria[0]->pertenece( x ) )
			paginasMemoria[0]->eliminar( x );
		else    paginasMemoria[1]->eliminar( x );
	}
	else {
		// Pagina * p = buscaPagina( x );
	}
}

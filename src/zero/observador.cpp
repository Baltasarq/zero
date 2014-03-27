// observador.cpp
/*
	Implementa el patrón observador.
*/

#include "observador.h"

namespace Zero {

// ================================================================= Observador

// ----------------------------------------------------- Observable::observar()
void Observador::observar(Observable * x)
{
        if ( !enEliminacion ) {
	        lServ.insert( lServ.end(), x );

	        x->masObservadores( this );
        }
}

// ------------------------------------------- Observable::eliminarObservador()
void Observador::dejarDeObservar(Observable *x)
{
        if ( !enEliminacion ) {
                x->eliminarObservador( this );
	        lServ.remove( x );
        }
}

// ------------------------------------------ Observador::dejarDeObservarTodos()
void Observador::dejarDeObservarTodos()
{
	ListaObservables::const_iterator it = lServ.begin();

        enEliminacion = true;
	while ( it != lServ.end() ) {
	    	(*it)->eliminarObservador( this );
		++it;
	}
        enEliminacion = false;
}

// -------------------------------------------------- Observador::~Observador()
Observador::~Observador()
{
        dejarDeObservarTodos();
}

// ================================================================= Observable
// -------------------------------------------------- Observable::~Observable()
Observable::~Observable()
{
        enEliminacion = true;
	ListaObservadores::const_iterator it = lObs.begin();

	while ( it != lObs.end() ) {
		(*it)->dejarDeObservar( this );
		(*it)->actualizar( this, Observador::BorradoElemento );
		++it;
	}
}

// ---------------------------------------------- Observable::masObservadores()
void Observable::masObservadores(Observador * x)
{
	lObs.insert( lObs.end(), x );
}

// ----------------------------------------------------- Observable::notificar()
void Observable::notificar(Observador::TipoCambio t)
{
	ListaObservadores::const_iterator it = lObs.begin();

	while ( it != lObs.end() ) {
		(*it)->actualizar( this, t );
		++it;
	}
}

// ------------------------------------------- Observable::eliminarObservador()
void Observable::eliminarObservador(Observador *x)
{
        if ( !enEliminacion ) {
	        lObs.remove( x );
        }
}

} // namespace Zero

// observador.h
/*
	Implementa el patrón observador.
*/

#ifndef __OBSERVADOR__H__
#define __OBSERVADOR__H__

#include <list>

namespace Zero {

class Observable;

/**
	El observador es el que se encarga de registrar a aquellos objetos
	a los que va a observar. El enlace recíproco se hace de manera
	transparente. Así, sólo es necesario que se haga:
		obs->observar( obj );
	Para que todos los enlaces se actualicen.
	Si el observador o el observado se eliminan, éstos son avisados
	convenientemente.
*/
class Observador {
public:
	typedef
		enum { NuevoElemento, CambioEnElemento, BorradoElemento }
	TipoCambio;

	typedef std::list<Observable *> ListaObservables;
private:
	ListaObservables lServ;
        bool enEliminacion;
public:
	Observador() : enEliminacion( false )
                {}
	virtual ~Observador();

	virtual void actualizar(Observable *rmt, TipoCambio t) = 0;

	void observar(Observable *);
	void dejarDeObservar(Observable *);
        void dejarDeObservarTodos();
};

/**
	Cualquier objeto que pueda ser susceptible
	de ser observado por otro.
	Por ejemplo, un método, para que pueda notificar
	al objeto al que pertenece que ha cambiado.
*/
class Observable {
friend class Observador;
public:
	typedef std::list<Observador *> ListaObservadores;

private:
        bool enEliminacion;
	ListaObservadores lObs;

protected:
	Observable() : enEliminacion( false )
                {}
	void masObservadores(Observador *x);
	void eliminarObservador(Observador *);

public:
	virtual ~Observable();

	virtual void notificar(Observador::TipoCambio =
					Observador::CambioEnElemento)
	;
};

}

#endif

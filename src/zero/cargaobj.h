// cargaobj.h
/*
        Carga los objetos en la memoria para que el runtime
        pueda incorporarlos a la ejecución.
*/

#ifndef CARGADOR_OBJETOS_H
#define CARGADOR_OBJETOS_H

#include "mnemos.h"
#include "excep.h"

#include <vector>
#include <map>
#include <set>
#include <string>

namespace Zero {

class Objeto;


/**
        Esta clase es la encargada de pasar los objetos a memoria.
        Acepta en el constructor un MedioSoporte que proporciona abstracción
 * 	sobre el origen.

        La clase lee los objetos del medio de soporte,
 	y nunca los borra, a no ser que
        se eliminen explícitamente con "eliminaTodos()".
*/

class Metodo;
class Objeto;

class CargadorObjetos {
public:
    typedef std::map<std::string, Objeto *> ListaObjetos;
protected:
    Objeto * objetoActual;
    Metodo * metodoActual;
    std::vector<Mnemotecnico *> mnemosMet;
	MedioSoporte *m;

	void compruebaOrigen() throw (ENoEsZero, EVersionIlegal);
	virtual Mnemotecnico *getSigMnemo();

	std::set<std::string> idsAtributos;
	std::set<std::string> idsMetodos;

    ListaObjetos vObj;

	void guardaObjeto();
	void registraNuevoObjeto(NMObj *);
	void guardaAtributo(NMAtr *);
	void guardaMetodo();
	void registraNuevoMetodo(NMMth *);
	void guardaMeta(NMMta *);
	void compruebaHerenciaCiclica();
public:
	CargadorObjetos(MedioSoporte *);
	virtual ~CargadorObjetos() {}

	void unflatten();
	void eliminaTodos();
	ListaObjetos *getObjetosCargados();
};

}

#endif

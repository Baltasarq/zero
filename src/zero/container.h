// container.h
/**
        Interfaz de los contenedores en Zero

        Pertenece a zvm

        jbgarcia@uvigo.es
*/

#ifndef CONTAINER_H
#define CONTAINER_H

#include <set>
#include "coleccion.h"
#include "objeto.h"

namespace Zero {

// -------------------------------------------------------------- AlmacenObjetos
class AlmacenObjetos : public Objeto {
/**
        Los contenedores son representados por esta clase.
	Los contenedores son almacenes de objetos. El contenedor es la unidad
	mínima de almacenamiento de objetos en el almacenamiento persistente
*/
private:
        void init();

public:
        AlmacenObjetos(AlmacenObjetos *pc, const std::string &n, const std::string &p)
            : Objeto(pc, n, p)
        { init(); }

        AlmacenObjetos(AlmacenObjetos *pc, const std::string &n, Objeto *p)
            : Objeto(pc, n, p)
        { init(); }

        virtual ~AlmacenObjetos();

        Objeto *crea(std::string n = NULL, Objeto *p = NULL);
        Objeto *inserta(Objeto *);
	void elimina(Objeto *);
	void eliminaTodos();
	Objeto *busca(const std::string &x)
		{ Atributo *atr = lAtrs.busca( x );
		  return atr == NULL ? NULL : atr->getObjeto();
		}

	ListaMnemosObjeto &salvar(ListaRefExternas &)
                { return mnemosObjeto; }

	std::string getRuta() const;

	void actualizar(Observable *, Observador::TipoCambio t);

        bool insertando(Atributo *);
        bool esAlcanzable();

	virtual void guardaContenedor() = 0;

        static AlmacenObjetos * nuevo(AlmacenObjetos *, const std::string &);
};

// ------------------------------------------------------------------- Container
class Container : public AlmacenObjetos {
private:
	std::set<Objeto *> objsProcsPtr;
	std::set<std::string> objsProcsStr;

	static void cambiarNombreObjetoEnMnemos(Objeto *,
											Atributo *,
											Objeto::ListaMnemosObjeto *
	);

	bool fueProcesado(Objeto *, const std::string &);
public:
        Container(Container *pc, const std::string &n, const std::string &p);
        Container(Container *pc, const std::string &n, Objeto * p);

         /// Al eliminar el contenedor de memoria, guardarlo en disco
	~Container()
		{ mnemosObjeto.clear(); refsExtObjeto.clear(); guardaContenedor(); }

        ListaMnemosObjeto &salvar(ListaRefExternas &);

		void guardaContenedor();

        void ponNoModificadoDesdeCarga();
        void ponModificadoDesdeCarga();
};

// ---------------------------------------------------------- TransientContainer
class TransientContainer : public AlmacenObjetos {
/**
	Aquellos contenedores que no van a ser hechos persistentes, deben ser
	creados con esta clase. El ejemplo paradigmático es el contenedor de
	ejecución, Exe.
**/
public:
    TransientContainer(AlmacenObjetos *pc, const std::string &n, const std::string &p);
    TransientContainer(AlmacenObjetos *pc, const std::string &n, Objeto *p);

    void guardaContenedor()
        {}

    ~TransientContainer()
        {}
};

}

#endif

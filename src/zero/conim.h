// conim.h
/*
	Se encarga de llevar el control de todos los contenedores
	abiertos en el sistema.
	La principal funcionalidad es:
		- Encontrar contenedores para descargar cuando hay
		  poca memoria.
		- Realizar el guardado de los contenedores cuando se
		  cierra todo el sistema.
*/

#ifndef _CONIM__
#define _CONIM__

#include <vector>
#include <string>
#include "excep.h"
#include "container.h"

/**
	AlmacenesEnMemoria
	Lleva un control de los AlmacenesObjeto en memoria.
 *
 * 	Incluye:
 * 	<ul>
 * 		<li>Lista de contenedores total
 * 		<li>Lista de contenedores transient
 * 		<li>Lista de contenedores persistentes
 * 		<li>Lista de contenedores sincronizados
 * 	</ul>
 *
 * 	Funcionalidad:
 * 	<ul>
 * 		<li>Incluir contenedores al ser creados, en <b>listas</b>
 * 		<li>Eliminar contenedores al ser eliminados, en <b>listas</b>
 * 		<li>Cuando cont pasa a estar sincronizado, meterlo en la lista
 * 		<li>Cuando cont no sincronizado, sacarlo de la lista
 * 	</ul>
*/

namespace Zero {

class Conim {
public:
    typedef AsocNombrePuntero<AlmacenObjetos> ListaAlmacenes;
private:
    static Conim *conim;

    Conim();
    const std::string *getNombreAlmacen(AlmacenObjetos *);

public:
    ListaAlmacenes *lConts;
    ListaAlmacenes *lPersConts;
    ListaAlmacenes *lTransConts;
    ListaAlmacenes *lSincroConts;

    ~Conim();
    void eliminaTodos();

    void inserta(AlmacenObjetos *) throw(EInterno, EDuplicado);
    void insertaContainer(Container *) throw(EInterno, EDuplicado);
    void insertaTransientContainer(TransientContainer *)
            throw(EDuplicado, EInterno);
    void elimina(AlmacenObjetos *) throw(EInterno);

    void ponSincronizado(AlmacenObjetos *) throw(EInterno);
    void ponDesSincronizado(AlmacenObjetos *) throw(EInterno);

    void sincronizar();

    static Conim * getConim()
            { return conim != NULL ? conim : new(std::nothrow) Conim; }
};

}

#endif

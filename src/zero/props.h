// props.h

#ifndef PROPIEDADES_H_
#define PROPIEDADES_H_

#include <vector>
#include <string>
#include "mnemos.h"

namespace Zero {

class Objeto;

class GestorPropiedades {
public:
	typedef std::vector<NMMta *> ListaPropiedadesIniciales;
private:
	std::string nomHost;
    ListaPropiedadesIniciales vPropsIniciales;
	Objeto * props;
	const std::string &getNombre() const;
public:
	GestorPropiedades(const std::string &n = "");
	~GestorPropiedades();

	void ponNombre(const std::string &n) throw (EInterno);

    void masPropiedades(NMMta *);
	bool tieneProps() const;
	bool existeObjetoProps() const { return ( props != NULL ); }

    const ListaPropiedadesIniciales &getPropsIniciales() const
        { return vPropsIniciales; }

	/// CUIDADO: En cuanto se le llama, inicializa las propiedades
	Objeto *getProps();
    void prepararProps();
};

}

#endif

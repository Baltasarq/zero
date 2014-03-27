// ident.h
/*
	Control de listas de identificadores
*/

#ifndef __IDENT__H__
#define __IDENT__H__

#include <map>
#include <string>
#include <set>

#include "mnemos.h"


namespace Zero {

/**
 * La clase Literal
 * Lleva los literales de los atributos, al compilar.
 * Comprueba de qué tipo son los literales.
*/
class Literal {
	public:
		typedef enum { LitNumInt, LitNumFlt, LitStr } Tipo;
	private:
		std::string atributo;
		std::string lit;
		Tipo tipo;
	public:
		Literal(const std::string &n, const std::string &l);

		const std::string &getNombreAtributo() const
			{ return atributo; }
		const std::string &getLiteral() const
			{ return lit; }
		const Tipo &getTipo() const
			{ return tipo; }
		std::string getLiteralConTipo();
};

/**
	La clase Identificadores.
	Guarda las ref. locales de un obj, para saber
	si se definen identificadores repetidos ... etc.

*/
class Identificadores {
public:
        typedef std::set<std::string> ListaIds;

        Identificadores(const std::string & = "");
        Identificadores(const Identificadores &);
        virtual ~Identificadores() {}

        void reset();
        virtual bool busca(const std::string &) const;
        virtual bool inserta(const std::string &);
        virtual std::string getID(size_t i) const;

        size_t getNumero()  const
            { return listaIds.size(); }
        const std::string &getNombre() const
            { return nombre; }

        Identificadores &operator=(const Identificadores &);
private:
        ListaIds listaIds;
        std::string nombre;
};

/**
	Añade a la clase Identificadores la posibilidad de que estos
	puedan estar ocultos o no.
*/
class IdentificadoresVisibles : public Identificadores {
public:
        IdentificadoresVisibles(const std::string & = "");

        bool busca(const std::string &) const;
        bool inserta(const std::string &, bool = true);
        std::string getID(size_t i) const;
        bool esVisible(size_t i) const;

private:
    using Identificadores::inserta;
};

/**
	Guarda los identificadores dentro de un objeto, típicamente es una
	lista de identificadores por método (Identificadores)
*/
class IdsPorObjeto {
        std::string nombre;
        std::string padre;
        IdentificadoresVisibles *nombresDeAtributos;
        IdentificadoresVisibles *nombresDeMetodos;
        typedef std::map <std::string, Identificadores *> Metodos;
	Metodos metodos;

public:
        IdsPorObjeto(const std::string &, const std::string &);
        IdsPorObjeto(const IdsPorObjeto &);
        ~IdsPorObjeto();

        IdentificadoresVisibles *getAtributos()
            { return nombresDeAtributos; }

        IdentificadoresVisibles *getMetodos()
            { return nombresDeMetodos; }

        bool buscaAtributo(const std::string &) const;
        bool insertaAtributo(const std::string &, bool visibilidad);
        std::string getAtributo(size_t i);
        size_t getNumeroAtributos() const
                { return nombresDeAtributos->getNumero(); }

        bool buscaMetodo(const std::string &) const;
        Identificadores * getIdsMetodo(const std::string &) const;
        bool buscaVbleMetodo(const std::string &, const std::string &) const;
        bool insertaMetodo(const std::string &,
			   bool,
			   const MixinConArgumentos::Argumentos &
        );
        bool insertaVbleMetodo(const std::string &, const std::string &);
        size_t getNumeroMetodos() const
                { return nombresDeAtributos->getNumero(); }

        IdsPorObjeto &operator=(const IdsPorObjeto &);

        const std::string &getNombre() const
            { return nombre; }
        const std::string &getNombrePadre() const
            { return padre; }
        std::string toXML() const;
};

class IdsPorPrograma {
private:
        typedef std::map<std::string, IdsPorObjeto *> ListaIdsPorObj;
        ListaIdsPorObj vIdsObj;
public:
        IdsPorPrograma();
        ~IdsPorPrograma();

        IdsPorObjeto *buscaObjeto(const std::string &) const;
        bool insertaObjeto(const std::string &, const std::string &);
        size_t getNumero()  const
            { return vIdsObj.size(); }
        std::string toXML() const;
};

}

#endif

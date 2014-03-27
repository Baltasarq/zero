// referencia.h
/*
        Soporte de referencias en la máquina virtual Zero

        Pertenece a zvm

        jbgarcia@uvigo.es
*/

#ifndef REFERENCIA_H
#define REFERENCIA_H

#include "coleccion.h"

namespace Zero {

class Objeto;
class Metodo;
class AlmacenObjetos;

class RefGenerica {
protected:
	virtual void hazSwizzling()                             = 0;
public:
        static char getChSeparador() { return CHR_SEP_IDENTIFICADOR; }
        virtual void ponReferencia(Objeto *)                    = 0;
        virtual void ponReferencia(const std::string &)              = 0;
        virtual const std::string &getNombreObjetoReferenciado()     = 0;
        virtual Objeto *getObjeto()                             = 0;
	virtual void alCambiar()				= 0;

	virtual ~RefGenerica() {}
};

class Referencia : public RefGenerica, public ParNombreObjeto<Objeto> {
public:
	static const bool Propietario = true;
	static const bool NoPropietario = false;

    class ListaIdentificadores {
    private:
        char * lista;
        size_t num;
        char ** ids;

        void reserva(size_t);

    public:
        unsigned int alinea(size_t tam)
            { return ( ( tam / sizeof( int ) ) + 1 ); }

        const char * getId(size_t i) const
            {
                if ( i < getNumero() )
                        return ids[ i ];
                else    return NULL;
            }

        size_t getNumero() const
            { return num; }

        ListaIdentificadores(const std::string &ref) : lista(NULL),ids(NULL)
            { calculaIds( ref ); }

        ListaIdentificadores() : lista(NULL), ids(NULL)
            {   init(); }

        void calculaIds(const std::string &);
        void init()
            {
                delete[] lista;
                delete[] ids;
                lista = NULL; ids = NULL;
                num = 0;
            }
    };


protected:
		bool esPropietario;
        ListaIdentificadores lIds;
        std::string objetoNoSwizz;
        Objeto *objAlQuePertenece;
        Metodo *metAlQuePertenece;

        void hazSwizzling();

        static Referencia * cargaContenedor(Objeto *, const std::string &);
        static Referencia * buscaAtributo( Objeto *, const std::string &);

public:

        Referencia(Objeto *po, Metodo *mo, const std::string &n, Objeto *d, bool = Propietario);
        Referencia(Objeto *po, Metodo *mo, const std::string &n, const std::string &o, bool = Propietario);
        virtual ~Referencia();


        Objeto *getPerteneceAlObjeto() { return objAlQuePertenece; }
        Metodo *getPerteneceAlMetodo() { return metAlQuePertenece; }
        bool    swizzled() const       { return ( dir != NULL ); }

        static Referencia *
        buscaVbleOAtributo(Objeto *, Metodo *, const std::string &);

        static Referencia *
        buscaObjetoEnContainerActualYLib(AlmacenObjetos *, const std::string &);

        static Referencia *
        hazSwizzlingDirecto(Objeto *, Metodo *, const std::string &);

        static Referencia *
        buscaReferencia(const std::string &, Objeto *, Metodo *);

        static Referencia *
        buscaReferencia(const ListaIdentificadores &, Objeto *, Metodo *);

        virtual void ponReferencia(Objeto *);
        virtual void ponReferencia(const std::string &);
        const std::string &getNombreObjetoReferenciado();

        Referencia *copia(Objeto *, Metodo *);

	void alCambiar();

	virtual Objeto *getObjeto();
};

/**
 * Las referencias constantes son aquellas que después de ser asignadas,
 * no cambian nunca el objeto al que apuntan hasta ser destruídas.
 * Deben ser asignadas con ponReferencia() (nunca mediante constructor),
 * y por tanto sólo una vez (las subsiguientes son ignoradas).
*/
class ReferenciaConstante : public Referencia {
private:
	bool asignada;
public:
	ReferenciaConstante(Objeto *po, Metodo *mo, const std::string &n)
		: Referencia(po, mo, n, ""), asignada(false)
	{}

	void ponReferencia(Objeto *obj)
	{
		if ( !asignada ) {
			Referencia::ponReferencia( obj );
			asignada = true;
		}
	}
	void ponReferencia(const std::string &s)
	{
		if ( !asignada ) {
			Referencia::ponReferencia( s );
			asignada = true;
		}
	}

	void alCambiar() {}
};

/**
	Es necesario agarrar el contenedor root pq se da una pequeña
	paradoja: cuando se crea esta referencia, estamos en el constructor del
	runtime, por lo que Runtime::rt()->getContainerEjecucion(), no funcionará
*/
class RefParadojaRaiz : public Referencia {
protected:
	void hazSwizzling();
public:
	RefParadojaRaiz(AlmacenObjetos *root);

	Objeto *getObjeto();
        void ponReferencia(Objeto *);
        void ponReferencia(const std::string &);
};

}

#endif

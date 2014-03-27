// objeto.h
/*
        Interfaz de los objetos Zero

        Pertenece a zvm

        jbgarcia@uvigo.es
*/

#ifndef OBJETO_H
#define OBJETO_H

#include <vector>
#include <string>

#include "excep.h"
#include "coleccion.h"
#include "mnemos.h"
#include "referencia.h"
#include "compilador.h"
#include "props.h"
#include "observador.h"

namespace Zero {

class Objeto;
class AlmacenObjetos;
class PersistentStore;
class LiteralVector_Zero;

// ================================ Atributos ===========================
class Atributo : public Referencia, public Observable {
private:
        NMAtr * atr;
        bool acceso;
public:
	const static bool PRIVADO = false;
	const static bool PUBLICO = true;

        Atributo(Objeto *, const std::string &, Objeto *, bool = PUBLICO, bool = Propietario);
        Atributo(Objeto *, const std::string &, const std::string &, bool = PUBLICO, bool = Propietario);

        ~Atributo();

        Atributo *copia(Objeto *);
        void alCambiar();

        bool esPublico() const { return acceso;  }
        bool esPrivado() const { return !acceso; }

        static bool chkVisible(Atributo * atr, Objeto *inicio)
            { return ( atr->esPublico() || ( atr->getPerteneceAlObjeto() == inicio ) ); }
};

class AtributoEspecial : public Atributo {
    public:
        AtributoEspecial(Objeto *po, const std::string &n, Objeto *d, bool acc = true, bool = NoPropietario);
        AtributoEspecial(Objeto *po,
                       const std::string &n,
                       const std::string &o,
                       bool acc = true,
					   bool = NoPropietario);
};

class AtributoParent : public AtributoEspecial {
public:
        AtributoParent(Objeto *po, const std::string &n, Objeto *d, bool acc = true, bool prop = Propietario)
                : AtributoEspecial(po, n, d, acc, prop)
        {}

        AtributoParent(Objeto *po,
		       const std::string &n,
		       const std::string &o,
		       bool acc = true)
                : AtributoEspecial(po, n, o, acc)
        {}
};

// ================================ Objetos ===================================
class Objeto : public Observador, public Observable {
friend class Container;
public:
	static const bool SYSTEM_COPY = true;

    typedef AsocNombrePuntero<Atributo> ListaAtributos;
    typedef AsocNombrePuntero<Metodo>   ListaMetodos;
	typedef Compilador::ListaMnemos     ListaMnemosObjeto;
	typedef std::list<Objeto *>         ListaRefExternas;

	class AccesoListaAtributos {
		friend class Objeto;
		private:
			Objeto *perteneceA;
			ListaAtributos *la;

		public:
			size_t getNumero() const
				{ return la->getNumero(); }
			const std::string &getNombreElementoNumero(size_t i) const
				{ return la->getNombreElementoNumero( i ); }
			Atributo * getElementoNumero(size_t i) const
				{ return la->getElementoNumero( i ); }
			size_t buscaIndicePorNombre(const std::string &x) const
				{ return la->buscaIndicePorNombre( x ); }
			Atributo * busca(const std::string &x) const
				{ return la->busca( x ); }
			bool inserta(const std::string &n, Atributo *atr)
				{ if ( perteneceA->insertando( atr )
                    && la->inserta( n, atr ) )
                  {
                      perteneceA->observar( atr );
                      perteneceA->actualizar( atr, NuevoElemento );
                      return true;
                  }
			  	  return  false; }
			bool borra(Atributo *atr)
				{  if ( la->busca( atr ) ) {
                                        perteneceA->actualizar( atr,
                                                    BorradoElemento
                                        ); // No actualizar después borrar
                                        la->borra( atr );
                                        perteneceA->actualizar( perteneceA,
                                                        CambioEnElemento
                                        );

                                        return true;
                                   }
                                   return false; }
			bool borra(const std::string &n)
				{ return borra( la->busca( n ) ); }

			void eliminaTodos()
				{ la->eliminaTodos();
                                  perteneceA->actualizar( NULL, BorradoElemento );
                                }
	};

	class AccesoListaMetodos {
		friend class Objeto;
		private:
			Objeto *perteneceA;
			ListaMetodos *lm;

		public:
			size_t getNumero() const;
			const std::string &getNombreElementoNumero(size_t i) const;
			Metodo * getElementoNumero(size_t i) const;
			size_t buscaIndicePorNombre(const std::string &x) const;
			Metodo * busca(const std::string &x) const;
			bool inserta(const std::string &n, Metodo *met);
			bool borra(Metodo *met);
			bool borra(const std::string &n);
			void eliminaTodos();
	};

protected:
        ListaMnemosObjeto mnemosObjeto;
        ListaRefExternas refsExtObjeto;
private:
	bool modificadoDesdeCarga;
	Metodo *metHerDin;
	GestorPropiedades gProps;
	std::string nombre;
	std::string nombreCompleto;
	int referenceCount;
	AlmacenObjetos *contAlQuePertenece;
	Atributo *atrObjetoPadre;
	Objeto *props;
	ListaAtributos la;
	ListaMetodos   lm;
	int numVectoresAux;

	Objeto * copiaSatelite(Objeto *objSatelite, Atributo *atr);

	void init();

        std::string getSigNombreVectorAux();
        void guardaPropiedades(const GestorPropiedades &);
        void listaMnemosDeMetodos(ListaRefExternas &);
        void listaMnemosDeAtributos(ListaAtributos &, ListaRefExternas &);
        void listaMnemosMetodoAlCargar(
                            const ListaAtributos &,
                            ListaRefExternas &
        );

        void listaMnemosCargarVector(
                const std::string &,
                LiteralVector_Zero *,
                ListaRefExternas &r,
                bool
        );

        void generaMnemosPrimitivo(
                const std::string &,
                Objeto *,
                ListaRefExternas &,
                bool &,
                bool
        );

public:
	AccesoListaAtributos lAtrs;
	AccesoListaMetodos lMets;

	static const bool NOREF_NOBORRAR = false;

//      void * operator new(size_t size);
//      void operator delete(void *ptr);

        Objeto(AlmacenObjetos *pc, const std::string &n, const std::string &p);
	Objeto(AlmacenObjetos *pc, const std::string &n, Objeto *p);
        virtual ~Objeto();

        bool estaVivo() const
                { return referenceCount >= 0; }

        const std::string &getNombreCompleto();
        Atributo *getAtrObjetoPadre() const
		{ return atrObjetoPadre; }
        const std::string &getNombre()  const
		{ return nombre;             }
        AlmacenObjetos *getPerteneceA() const
		{ return contAlQuePertenece; }
        void asignarA(AlmacenObjetos *c)
		{ contAlQuePertenece = c; }

	void decrementaReferencias(bool borrable = true);
        void incrementaReferencias()
		{ ++referenceCount; }
        int  getReferenceCount()  const
		{ return referenceCount; }

	Objeto *getProps();
	void masPropiedades(NMMta *meta) { gProps.masPropiedades( meta ); }
	bool tieneProps() { return gProps.tieneProps(); }

	Metodo * getMetHerenciaDinamica()
		{ return metHerDin; }
	void procHerenciaDinamica();
	void actualizar(Observable *rmt, TipoCambio t);

        bool copia(Objeto *, const std::string &, bool copiaSistema = false);
        Objeto * copiaSistema(const std::string &);

        /// Si es objeto especial, lo prepara para tener valor nativo
        bool preparaObjetoEspecial(Objeto *org, Objeto *dest);

        bool esDescendienteDe(AlmacenObjetos *, const std::string &);
        bool esDescendienteDe(Objeto *);

        /// Devuelve si el objeto fue modificado desde su carga
	bool fueModificadoDesdeCarga() const
		{ return modificadoDesdeCarga; }

        /// ¿Insertando un contenedor?
        virtual bool insertando(Atributo *);

        /// Cambia el indicador de modificado.
        /// Sólo puede ser llamado desde el cargador de objetos
        /// (al terminar la carga), y desde el propio objeto (después
        /// de salvar() ).
        /// Cualquier otra llamada es un error.
        virtual void ponNoModificadoDesdeCarga()
                { modificadoDesdeCarga = false; }

        /// Al crear container, indicar grabar
        virtual void ponModificadoDesdeCarga()
                { modificadoDesdeCarga = true; notificar( CambioEnElemento ); }

        virtual ListaMnemosObjeto &salvar(ListaRefExternas &);
        void eliminarMnemosObjeto();
        static bool esObjetoTemporal(Objeto *);
        static bool esObjetoProximo(Objeto *);
};


/** ObjetoSistema
 *  Esta clase símplemente sirve para distinguir entre objetos creados
 *  durante el bootstrap (como "vm" o "__zero_console__") y que no deben ser
 *  nunca guardados.
*/
class ObjetoSistema : public Objeto {
public:
    ObjetoSistema(AlmacenObjetos *pc, const std::string &n, const std::string &p)
        : Objeto( pc, n, p )
    {}

    ObjetoSistema(AlmacenObjetos *pc, const std::string &n, Objeto *p)
        : Objeto( pc, n, p )
    {}
};

}

#endif

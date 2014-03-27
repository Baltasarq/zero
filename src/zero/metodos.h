// Métodos.h
/*
        Lleva la parte de definición y ejecución de métodos
*/

#ifndef METODOS_H
#define METODOS_H

#include <string>
#include <vector>

#include "coleccion.h"
#include "excep.h"
#include "opcodes.h"
#include "referencia.h"
#include "runtime.h"
#include "observador.h"

namespace Zero {

class Objeto;
class Zero_Data_;

// ================================ Metodos ===============================
class Metodo : public Observable {
friend class GestorExcepciones;
public:
    friend class Runtime;
    friend class Opcode;
	friend class OcFlw;

    enum Acceso { PRIVADO, PUBLICO };
    typedef Opcode ** CuerpoMetodo;
    typedef Opcode ** IterMet;
    typedef AsocNombrePuntero<Referencia> ListaVblesLocales;
	typedef std::vector<Referencia *> Argumentos;
    static  Argumentos argumentosNulos;
	typedef Referencia ** Registros;
	MixinConArgumentos::Argumentos argumentosFormales;

protected:
    bool _enExcepcion;
    bool enFase;
	Referencia * reg[NombreRegistro::numRegistros];
    CuerpoMetodo instr;
    Acceso tipoAcceso;
    std::string nombre;
    Objeto * objetoAlQuePertenece;
	unsigned int numLlamadasRecursivas;
	int despuesLiterales;               /// La instrucción después de los literales
    IterMet excep;
    IterMet ip;
    IterMet comienzo;
    IterMet final;
    IterMet finMet;

    Objeto * props;
	Metodo * recursivo;
	Metodo * metodoAnterior;
	std::string listaOpcodes;
	GestorPropiedades gProps;
	bool herenciaDinamica;

    /**
            Prepara el método para ser ejecutado, eliminando las
            instrucciones del cuerpo del método
    */
    void inicializar()
        { inicializarComportamiento(); inicializarEstado(); }

    void inicializarComportamiento();
    void inicializarEstado();

	void preparaEstadoMetodo(Objeto *);
    void preparaVariablesLocales(Objeto *rthis);
    void gestionExcepciones();
    bool convertirArgumentosAFormales(Argumentos *args);

    void asignaIP(IterMet dir)
        { ip = dir; }

    static void copiaEstado(Metodo * org, Metodo * dest);
public:
    ListaVblesLocales vblesLocales;

    Metodo(const std::string &n, Objeto *rthis,
    const MixinConArgumentos::Argumentos &, Acceso = PUBLICO);

    virtual void deSincroniza(Metodo * met = NULL);
    virtual ~Metodo();

	bool esRecursivo() const
        { return ( recursivo != NULL ); }
    void descuentaRecursivo()
        { --( recursivo->numLlamadasRecursivas ); }
    Metodo * getMetodoOrgRecursivo() const
        { return recursivo; }
	unsigned int getNumLlamadasRecursivas() const
		{ return numLlamadasRecursivas; }
	void anotaNuevoRecursivo()
        { ++numLlamadasRecursivas; }
    Metodo * getMetodoAnterior()
        { return metodoAnterior; }

	static Metodo * duplicaPorRecursivo(Metodo * met);
	static Metodo * buscaMetodo(Objeto *, const std::string &);
	static Metodo * encuentraMetodo(Objeto * obj, const std::string & n)
        {
            return encuentraMetodo( buscaMetodo( obj, n ) );
        }
	static Metodo * encuentraMetodo(Metodo * met)
        {   return ( met != NULL ?
                    (met->ejecutando() ? duplicaPorRecursivo( met ):
                                         met)
			    : NULL );
        }

    bool tieneGestorExcepciones()
        { return ( excep < finMet ); }

    virtual Metodo *copia(Objeto *obj);

    bool getFase() const { return enFase; }

    virtual bool ejecutandoSeccPpal() const
        { return ip != NULL ? ( ip >= instr && ip < excep ) : false; }
    virtual bool enExcepcion() const
        { return _enExcepcion; }
    virtual bool ejecutandoSeccExcp() const
        { return ip != NULL ? ( ip >= excep && ip < finMet ) : false; }
    bool ejecutando() const
		{ return ( ejecutandoSeccPpal() || ejecutandoSeccExcp() ); }

    void finEjecucion();

    Referencia *getLocAcc()  const { return reg[NombreRegistro::__ACC];  }
	Referencia *getLocThis() const { return reg[NombreRegistro::__THIS]; }
	Referencia *getLocRR()   const { return reg[NombreRegistro::__RR];   }
	Referencia *getLocGP1()  const { return reg[NombreRegistro::__GP1];  }
	Referencia *getLocGP2()  const { return reg[NombreRegistro::__GP2];  }
	Referencia *getLocGP3()  const { return reg[NombreRegistro::__GP3];  }
	Referencia *getLocGP4()  const { return reg[NombreRegistro::__GP4];  }
	Referencia *getLocExc()  const { return reg[NombreRegistro::__EXC];  }

    void ponRetorno(Objeto *x)
        { getLocRR()->ponReferencia(x);
          getLocAcc()->ponReferencia(x);  }

    static Acceso getAccesoCorrespondiente(NMMth *met)
        { return ( met->esAccesoPublico() ? PUBLICO : PRIVADO ); }

    const std::string &getNombre()     const { return nombre; }
	Objeto * getPerteneceA()      const { return objetoAlQuePertenece; }
    Objeto * resultadoEjecucion() const { return getLocRR()->getObjeto(); }

	void chk();

	/**
        @brief Meter los opcodes en el cuerpo del método (instr).
        Una vez que se lea el primer RET, el resto va a la sección de excep.

        Recibe una lista de mnemos y lanza el chk().
        @note Borra los mnemos de la lista a medida que crea los opcodes.
        @param l La lista de mnemos a convertir en opcodes y almacenar para ejecutar
    */
    void masInstrucciones(Compilador::ListaMnemos &l);

    IterMet getFinMet()
        { return finMet; }

    IterMet getComienzoMet()
        { return comienzo; }

	CuerpoMetodo getInstrucciones()
        { return instr; }

    unsigned int getNumInstrucciones()
        { return ( finMet - instr ); }

	const std::string &getListaOpcodes();

	Objeto *getProps();
	void masPropiedades(NMMta *meta) { gProps.masPropiedades( meta ); }
    const GestorPropiedades &getGestorPropiedades() const
        { return gProps; }

	bool tieneProps() { return gProps.tieneProps(); }
    void procPragma(NMMta *, unsigned int);

    void masVblesLocales(Referencia *r)
        { if ( !vblesLocales.inserta(r->getNombre(), r) )
                throw ENoHayMemoria("creando variable local"); }

    virtual Objeto * ejecuta(Argumentos *args, Objeto *rthis);

    Acceso getTipoAcceso()    const    { return tipoAcceso; }
    bool esPublico()          const	{ return ( tipoAcceso == PUBLICO ); }
	bool esHerenciaDinamica() const	{ return herenciaDinamica; }
	static bool chkVisible(Metodo * met, Objeto * obj)
        { return ( met->esPublico() || ( met->getPerteneceA() == obj ) ); }
};

// ==================================================================== MetodoMV
class MetodoMV : public Metodo {
private:
        bool enEjecucion;
        MetodoRuntime met;
public:
        friend class Runtime;

        void deSincroniza(Metodo * = NULL);

        MetodoMV(const std::string &n, Objeto *rthis, MetodoRuntime,
		 const MixinConArgumentos::Argumentos &,
                 Acceso = PUBLICO);

        Objeto * ejecuta(Argumentos * args, Objeto *rthis);

        bool ejecutandoSeccPpal() const	{ return enEjecucion; }
        bool ejecutandoSeccExcp() const	{ return false;       }

	MetodoMV *copia(Objeto *obj);
};

}

#endif

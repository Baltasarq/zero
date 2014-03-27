// optimizador.h

#ifndef __OPTIMIZADOR_H_
#define __OPTIMIZADOR_H_

#include "partescompilador.h"
#include "compilador.h"

#include <list>
#include <map>
#include <set>

namespace Zero {

class Optimizacion;

/**
	La clase Optimizador realiza varias operaciones sobre los mnemos,
	de manera que trata de reducirlos en número y complejidad lo
	más posible.
	Las operaciones vienen definidas por clases singleton, derivadas de
	Optimizacion.
*/
class Optimizador : public PartesCompilador {
public:
	typedef std::list<Mnemotecnico *> ListaMnemos;
	typedef ListaMnemos::iterator MnemoIterator;
private:
    TipoOptimizacion opt;
	Compilador *comp;
	ListaMnemos * listaMnemos;
	Compilador::ListaMnemos * listaMnemosCmp;
	Identificadores *argsFormales;
	Identificadores *atrsObj;

	static void borrarTodosOpts();
public:
	Optimizador(Compilador * = NULL, TipoOptimizacion = opt2);
	Optimizador(Compilador::ListaMnemos &, TipoOptimizacion = opt2);
    ~Optimizador();

	// El núcleo, va llamando en orden a las optimizaciones
	void optimizar(
                Identificadores *,
                Identificadores *,
                Compilador::ListaMnemos * = NULL,
                Optimizador::TipoOptimizacion = opt2);

	// El vector de optimizaciones
	static std::vector<Optimizacion *> vOpts;

	static void iniciaVectorOpts(TipoOptimizacion opt = opt2);
	static void registraOpt(Optimizacion *);
};

/**
	La clase Optimizacion es la clase base de cualquier
	posible opt.. Se pueden añadir optimizaciones de esta forma
	de manera altamente flexible.
	Singleton
*/
class Optimizacion {
	protected:
                Identificadores *atrsObj;
                Identificadores *argsFormales;
		Optimizador::ListaMnemos * listaMnemos;

		Optimizacion(Optimizador::ListaMnemos * l)
			: listaMnemos(l)
		{}

	public:
		void ponListaMnemos(Optimizador::ListaMnemos * l) {
			listaMnemos = l;
		}

                void ponIds(Identificadores *args, Identificadores * atrs) {
                        atrsObj = atrs;
                        argsFormales = args;
                }

		virtual ~Optimizacion() {}

		virtual void hazOpt() = 0;
};

/**
	La clase OptChkMta sirve para realizar varias comprobaciones
	semánticas: metainformación correcta y que los defs no contienen palabras
	reservadas, como registros o literales.
*/
class OptChkMta : public Optimizacion {
public:
    ~OptChkMta() { opt = NULL; }
    void hazOpt();
    void chkId(const std::string &id);
            bool getHayLiterales() { return hayLiterales; }

    static OptChkMta *getOpt();
private:
    bool hayLiterales;
    static Optimizacion * opt;

    OptChkMta(Optimizador::ListaMnemos * l)
        : Optimizacion(l)
    {}
};

/**
	La clase OptDefPpio sirve para la opt. simple que
	consiste en mover todos los DEF's al principio del método.
	Singleton
*/
class OptDefPpio : public Optimizacion {
                static Optimizacion * opt;

		OptDefPpio(Optimizador::ListaMnemos * l)
			: Optimizacion(l)
		{}

	public:
                ~OptDefPpio() { opt = NULL; }
		void hazOpt();
		static OptDefPpio *getOpt();
};

/**
	La clase OptLitAg sirve para la opt. simple que
	consiste en mover todas las literales al principio del método.
	Singleton
*/
class OptLitAg : public Optimizacion {
                static Optimizacion * opt;

		OptLitAg(Optimizador::ListaMnemos * l)
			: Optimizacion(l)
		{}
	public:
		class OperadorMenor {
		public:
			bool operator()(const std::string *a, const std::string *b) const
			{
				return *a < *b;
			}
		};

		static const std::string *nombreLit;
		typedef std::map<std::string * const, std::string, OperadorMenor> ListaIds;

	private:
		unsigned int numLits;

		std::string getNuevoIdLit();

		const std::string *buscaLiteral(Lit *);
		const std::string &creaLiteral(Lit *);
		const std::string &registraLit(Lit *);
		void colocaConstEnd();

		ListaIds idsInt;
		ListaIds idsFlt;
		ListaIds idsStr;
	public:
                ~OptLitAg() { opt = NULL; }
		void hazOpt();
		static OptLitAg *getOpt();
};

/**
	La clase OptRedccOp sirve para la opt. que consiste
	en intentar reducir al máximo los movimientos de referencias entre
	registros.
	Singleton
*/
class OptRedccOp : public Optimizacion {
	public:
		typedef std::set<std::string> TablaRefsLocales;

		enum TipoSentencia {
			_sentencia_MSG, _sentencia_TRW, _sentencia_RET, _sentencia_ERROR
		};

		class Objetivo {
		public:
			std::set<std::string> regsReducidos;
			Optimizador::MnemoIterator mnemoActual;
			TipoSentencia tipoSentencia;
			NombreRegistro::CodigoRegistro reg;
			std::string destino;

			bool fueReducido(const std::string &reg)
				{ return ( regsReducidos.find( reg ) != regsReducidos.end() ); }

			void preparaSigBusqueda() {
				tipoSentencia = _sentencia_ERROR;
				--mnemoActual;
				reg = NombreRegistro::__NOREG;
				destino.erase();
				regsReducidos.clear();
			}

			void init() {
				tipoSentencia = _sentencia_ERROR;
				mnemoActual = ( Optimizador::MnemoIterator )NULL;
				reg = NombreRegistro::__NOREG;
				destino.erase();
				regsReducidos.clear();
			}
		};

        	~OptRedccOp() { opt = NULL; }
		void hazOpt();
		static OptRedccOp *getOpt();

	private:
		bool comprobarSiAcumuladorPosible(Optimizador::MnemoIterator, Optimizador::MnemoIterator);
        	static Optimizacion * opt;

		OptRedccOp(Optimizador::ListaMnemos * l)
			: Optimizacion(l)
		{}

		TablaRefsLocales refsLocales;
		Objetivo objetivo;

		void reducir();
		Optimizador::MnemoIterator reduceSentenciaActual();
		NombreRegistro::CodigoRegistro buscaSigRegistro();
		NombreRegistro::CodigoRegistro buscaSigRegistroMsg(NMMsg *);
		NombreRegistro::CodigoRegistro buscaSigRegistroTrw(NMTrw *);
		NombreRegistro::CodigoRegistro buscaSigRegistroRet(NMRet *);

		bool esMovimientoRegs(Optimizador::MnemoIterator);
		bool esMovimientoRelevante(
				Optimizador::MnemoIterator it,
				NombreRegistro::CodigoRegistro &obj,
				bool &eliminar,
				bool &acumuladorModificado)
		;

		bool esRefLocal(const std::string &ref)
			{ return ( refsLocales.find( ref ) != refsLocales.end() ); }

		static void aplicarCambio(Optimizador::MnemoIterator,
			NombreRegistro::CodigoRegistro org,
			const std::string &dest)
		;

		static void aplicarCambioMsg(NMMsg *,
			NombreRegistro::CodigoRegistro org,
			const std::string &dest)
		;

		static void aplicarCambioTrw(NMTrw *,
			NombreRegistro::CodigoRegistro org,
			const std::string &dest)
		;

		static void aplicarCambioRet(NMRet *,
			NombreRegistro::CodigoRegistro org,
			const std::string &dest)
		;

		static void aplicarCambioReg(Nombre **,
			NombreRegistro::CodigoRegistro org,
			const std::string &dest)
		;
};

/**
	La clase OptCnvtEtq sirve para la opt. que consiste
	en convertir todas las etiquetas en desplazamientos relativos,
	eliminando los mnemos ETQ.
	Singleton.
*/
class OptCnvtEtq : public Optimizacion {
	public:
		typedef std::map<std::string, unsigned int> TablaEtiquetas;
	private:
        static Optimizacion * opt;

		TablaEtiquetas etqs;

		OptCnvtEtq(Optimizador::ListaMnemos * l)
			: Optimizacion(l)
		{}

	public:
        ~OptCnvtEtq() { opt = NULL; }
		void hazOpt();
		static OptCnvtEtq *getOpt();
};

}

#endif

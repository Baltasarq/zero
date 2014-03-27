// zm.h
/*
      Cabecera del objeto ensamblador Zero

      jbgarcia@uvigo.es
*/

#ifndef ZA_H
#define ZA_H

#include <iostream>
#include <cstdio>
#include <cctype>
#include <vector>
#include <stack>
#include <map>
#include <string>

#include "analizadorlexico.h"
#include "compilador.h"
#include "ident.h"

#define TEXT_UI_MAIN      // Provee de una interfaz de texto, con fn. main()

// ================================ Constantes ===============================
extern const char   MARCACOMENTARIO;

// ================================ Log ========================================
class Log {
  private:
  	std::string nombre;
	std::FILE *f;
  public:
  	Log(const std::string &n) {
		//nombre = n.substr(0, n.find('.')) + ".log";
		nombre = n;

		f = fopen(nombre.c_str(), "wt");

		(*this)("\n;# zm Log " + n + '\n');
	}

	~Log() {
		if (f != NULL) {
			(*this)("\n\n;# zm log eof ok\n");
			fclose(f);
		}
	}

	void operator()(const std::string &texto){
		if (f != NULL) {
			fputs(texto.c_str(), f);
			fflush(f);
		}
	}

};

// ================================ Metodo =====================================
class Metodo {
  public:
    typedef Zero::Compilador::ListaMnemos CuerpoMetodo;
  private:
    CuerpoMetodo cuerpo;
    std::string nombreObjeto;
    std::string nombreMetodo;
    Zero::IdsPorObjeto * idsObj;
  public:
    Metodo(const std::string &no, const std::string &nm, Zero::IdsPorObjeto *, Zero::MedioSoporte *);
    ~Metodo();

    void masMnemos(Zero::Mnemotecnico *);
    void masMnemos(CuerpoMetodo const &);

    void escribe();
    static void listar(Log &, CuerpoMetodo const &);
};

// ================================ GestorNombresLoc ===========================
class GestorNombresLoc {
  private:
  	static const std::string nombreLiteral;
	std::queue<std::string> nombres;
	std::stack<std::string> regGenericos;
  public:
  	GestorNombresLoc();
	const std::string &crearNuevo();
	const std::string &insertarNuevo(const std::string &);
	unsigned int getNumero()     const { return nombres.size(); }
	std::string getSigNombre();
	bool esVacio()               const { return nombres.empty(); }
};

// ================================ MacroEnsamZ ================================
class MacroEnsamZ {
public:
	typedef  enum{ Jmp, Jot, Jof } TipoSalto;
private:
    Zero::AnalizadorLexico *lex;
    FILE *ent;
    Zero::MedioSoporte *sal;
    std::string fichsalida;

    int lin;
    std::string buffer;

    typedef enum { TOPLEVEL, ENOBJETO, ENCUERPOMETODO, ENEXCEPMETODO,
                   ENFINMETODO } ESTADO;

    ESTADO estado;

	Log *log;

    void procesaLinea();

	void compNMObj();
	void compNMMth();
	void compNMEnm();
	void compNMAtr();
	void compNMEno();
	void compNMRet();
	void compNMTrw();
	void compNMDef();
	void compNMJmp(TipoSalto);
	void compNMIof();
	void compNMEtq();
	void compNMMta();

	void compExpr();

        typedef std::vector<std::string> argumentos;

        bool compruebaParam(const std::string &s) const;
        bool compruebaFlotante(const std::string &s) const;

        std::string getAtributo(const std::string &, unsigned int);
        bool buscaVbleAtributo(const std::string &, const std::string &, const std::string &) const;
        bool buscaVble(const std::string &, const std::string &, const std::string &) const;
        bool buscaMetodo(const std::string &, const std::string &) const;

        Metodo * mnemosMetActual;
        void compilaExpresion(const std::string &, Metodo *&, Metodo::CuerpoMetodo &, bool=false);
	void compilaExpresionOLiteral(Metodo::CuerpoMetodo &);
public:
        Zero::Mnemotecnico * parseLiteral();

        std::vector<Zero::Literal> literalesPorObjeto;
        std::string objetoActual;
        std::string metodoActual;
        Zero::IdsPorPrograma idsProg;
	void leeLinea();

        MacroEnsamZ(FILE *e, const std::string &f);
        ~MacroEnsamZ();

        const int getNumeroLinea() const { return lin; }
        const std::string &getLineaActual() const { return buffer; }
        void sigLinea() { ++lin; }
        void generaCodigo();

	const std::string &getTokenActual() const {
		if ( lex != NULL )
			return lex->getTokenActual();
		else    return buffer;
	}

	static Zero::Optimizador *opt;
};

// Interface ===================================================================
extern void warning(MacroEnsamZ *, const std::string &);
extern void warning(const std::string &);

#endif

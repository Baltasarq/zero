// za.h
/*
      Cabecera del ensamblador Zero

      jbgarcia@uvigo.es
*/

#ifndef ZA_H
#define ZA_H

#include <vector>
#include <string>

#include "analizadorlexico.h"
#include "mnemos.h"
#include "compilador.h"
#include "optimizador.h"
#include "ident.h"

#define TEXT_UI_MAIN      // Provee de una interfaz de texto, con fn. main()

// ================================ Constantes ===============================
extern const std::string MARCANASG;
extern const std::string MARCANATR;
extern const std::string MARCANCMP;
extern const std::string MARCANCND;
extern const std::string MARCANDEF;
extern const std::string MARCANENM;
extern const std::string MARCANENO;
extern const std::string MARCANFLT;
extern const std::string MARCANIOF;
extern const std::string MARCANMTH;
extern const std::string MARCANMSG;
extern const std::string MARCANNEW;
extern const std::string MARCANOBJ;
extern const std::string MARCANRET;
extern const std::string MARCANSET;
extern const std::string MARCANSTR;
extern const std::string MARCANTRW;
extern const std::string MARCANELS;
extern const std::string MARCANENC;
extern const std::string MARCANWHL;
extern const std::string MARCANLOP;
extern const std::string MARCANNOP;
extern const char   MARCACOMENTARIO;

// ================================ EnsamZ ================================
class EnsamZ {
public:
	typedef enum {
		SALTO_COND_FALSE, SALTO_COND_TRUE, SALTO_ABSOLUTO
	} TipoSalto;

    typedef enum { TOPLEVEL, ENOBJETO, ENCUERPOMETODO, ENEXCEPMETODO,
                       ENFINMETODO
	} Estado;

public:
        std::vector<Zero::Literal> literalesPorObjeto;
        std::string objetoActual;
        std::string metodoActual;
        Zero::IdsPorPrograma idsProg;

        EnsamZ(FILE *e, Zero::MedioSoporte *f, const std::string &nomf);
        ~EnsamZ();

        const int getNumeroLinea() const;
        unsigned int getNumeroLineaMetodo() const { return c->getNumLinea(); }
        std::string &getLinActual();
        void generaCodigo(Zero::Optimizador::TipoOptimizacion);

        std::string getAtributo(const std::string &, unsigned int);
        bool buscaVbleAtributo(const std::string &, const std::string &, const std::string &) const;
        Zero::Identificadores * buscaMetodo(const std::string &, const std::string &) const;
        Zero::Identificadores * buscaAtributosObjeto(const std::string &);
        Zero::IdsPorObjeto * buscaObjeto(const std::string &x)
            { return idsProg.buscaObjeto( x ); }

private:
        FILE *ent;
        Zero::MedioSoporte *sal;
        std::string fichsalida;
        unsigned int lin;
        std::string buffer;
        std::string txtMet;
        static Zero::Compilador *c;

        Estado estado;

        void procesaLinea(Zero::Optimizador::TipoOptimizacion);
        void sigLinea();

        void compNMObj();
        void compNMEno();
        void compNMMth();
        void compNMEnm(Zero::Optimizador::TipoOptimizacion opt);
        void compNMAtr();
        void compNMMta();

        void chkNMMsg(Zero::NMMsg *);
        void compilaMetodo(Zero::Optimizador::TipoOptimizacion opt);

};

// Interface ===================================================================
extern void warning(EnsamZ *, const std::string &);
extern void warning(const std::string &);

#endif

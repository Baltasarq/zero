// compilador.h
/*
	El compilador toma una cadena,
	que debe contener los mnemotécnicos en formato ensamblador,
	y la compila como cuerpo de un método.

	jbgarcia@uvigo.es
*/


#ifndef _COMPILADOR_H__
#define _COMPILADOR_H__

#include "mnemos.h"
#include "analizadorlexico.h"
#include "ident.h"
#include "partescompilador.h"

namespace Zero {

class Optimizador;

/**
	La clase Compilador.
	Se encarga de compilar unos mnemotécnicos que vienen en formato de texto,
	separados por cambios de línea \n.
	Se asume que los mnemotécnicos son los derivados de Instr, es decir,
	los que pueden ir dentro de un método, pues es lo que se compila.
*/
class Compilador : PartesCompilador {
friend class Optimizador;
public:
    typedef std::vector<Mnemotecnico *> ListaMnemos;
	Optimizador *opt;
	AnalizadorLexico *lex;
	std::string cadAviso;
private:
	unsigned int lin;
	std::string *txt;
	ListaMnemos listaMnemos;
	Identificadores * args;   /// argumentos formales del método
	Identificadores * atrs;   /// atributos del objeto del método
	Identificadores ids;      /// Guarda todos los ids de una compilación

public:
	Compilador( std::string *                   = NULL,
	            Optimizador *              = NULL,
                Identificadores *          = NULL,
                Identificadores *          = NULL )
	;

	~Compilador();

	/// Preparar el compilador para comenzar
	void reset( std::string *,
	            Identificadores * = NULL,
                Identificadores * = NULL )
	;

	/// Obtener el optimizador
	const Optimizador *getOptimizador() const { return opt; }

	/// Compilar
	void prepararCompilacion();
	ListaMnemos &compilar(TipoOptimizacion);
	ListaMnemos &getListaMnemos() { return listaMnemos; }

	/// Realizar comprobaciones
	void chkMnemo(Mnemotecnico *);

	/// Comprobar los mnemotécnicos Def
	void chkNMDef(NMDef *);

	/// Comprobar los mnemotécnicos Ret
	void chkNMRet(NMRet *);

	/// Comprobar los mnemotécnicos Asg
	void chkNMAsg(NMAsg *);

	/// Comprobar los mnemotécnicos Msg
	void chkNMMsg(NMMsg *);

	/// Obtener la cadena de avisos dados por el compilador
	const std::string &getCadenaAvisos() const { return cadAviso; }

	/// Obtener el número de línea que está compilando
	unsigned int getNumLinea() const { return lin; }
};

}

#endif

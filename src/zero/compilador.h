// compilador.h
/*
	El compilador toma una cadena,
	que debe contener los mnemot�cnicos en formato ensamblador,
	y la compila como cuerpo de un m�todo.

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
	Se encarga de compilar unos mnemot�cnicos que vienen en formato de texto,
	separados por cambios de l�nea \n.
	Se asume que los mnemot�cnicos son los derivados de Instr, es decir,
	los que pueden ir dentro de un m�todo, pues es lo que se compila.
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
	Identificadores * args;   /// argumentos formales del m�todo
	Identificadores * atrs;   /// atributos del objeto del m�todo
	Identificadores ids;      /// Guarda todos los ids de una compilaci�n

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

	/// Comprobar los mnemot�cnicos Def
	void chkNMDef(NMDef *);

	/// Comprobar los mnemot�cnicos Ret
	void chkNMRet(NMRet *);

	/// Comprobar los mnemot�cnicos Asg
	void chkNMAsg(NMAsg *);

	/// Comprobar los mnemot�cnicos Msg
	void chkNMMsg(NMMsg *);

	/// Obtener la cadena de avisos dados por el compilador
	const std::string &getCadenaAvisos() const { return cadAviso; }

	/// Obtener el n�mero de l�nea que est� compilando
	unsigned int getNumLinea() const { return lin; }
};

}

#endif

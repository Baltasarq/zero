// analizadorlexico.h
/*
	Cabecera del analizador l�xico.

	jbgarcia@uvigo.es
*/

#ifndef ANALIZADOR_LEXICO_H
#define ANALIZADOR_LEXICO_H

#include <string>

namespace Zero {

/***
	Clase AnalizadorLexico.
	Esta clase es necesaria para compilar ensamblador.
	Hay que tener en cuenta que la m�quina virtual debe
	poder ensamblar c�digo en tiempo de compilaci�n.
*/

class AnalizadorLexico {
private:
    static char buffer[];
	unsigned int pos;
	std::string *txt;
	std::string token;

public:
	/// Tipos de literales
    enum TipoToken { NADA, IDENTIFICADOR, LITNUMERICO, LITCADENA };

    /// Tipo de avance
    enum Avance { Adelante, Atras };

	/// Constructor del AnalizadorLexico
	AnalizadorLexico(std::string *cad);

	/// Devuelve la l�nea actual
	const std::string &getLinea() const { return *txt; };

	/// Devuelve el token actual
	std::string &getTokenActual() { return token; };

	/// Determina si el token actual es un flotante
	bool esTokenActualFlotante() const { return compruebaFlotante( token ); }

	/// Devuelve el siguiente token
	std::string &getToken();

	/// Devuelve el siguiente n�mero
	std::string &getNumero();

	/// Pasa los espacios (y cualquier otro car�cter que no intervenga)
	/// a partir de la posici�n inicial
	void pasaEsp();

	/// Pasa s�lo los posibles espacios y tabuladores que haya
	void pasaSoloEsp();

	/// Devolver el tipo del siguiente token sin leerlo
	TipoToken getTipoSiguienteToken();

	/// Devolver el siguiente literal
	std::string &getLiteral(char delim);

	/// �Es fin de l�nea?
	bool esEol() const;

	/// Avanza en el texto
	void avanza(int avance = 1);

	/// Devuelve la pos actual dentro del texto
	unsigned int getPosActual() const { return pos; }

	/// Coloca el indicador de posici�n a cero
	void reset() { pos = 0; }

	/// Devuelve el car�cter que ser� evaluado a continuaci�n
	char getCaracterActual() const;

	// Herramientas �tiles --------------------------------------
	// Convertir a may�sculas
	static std::string mays(const std::string &x);
	static std::string &maysCnvt(std::string &x);

	// Elimina espacios y caracteres in�tiles
	static std::string &rTrimCnvt(std::string &x);
	static std::string &lTrimCnvt(std::string &x);
	static std::string &trimCnvt(std::string &x);

	static std::string rTrim(const std::string &x);
	static std::string lTrim(const std::string &x);
	static std::string trim(const std::string &x);

	// �tiles para analizar
	static std::string &getNumero(const std::string &lin, unsigned int &pos, std::string &num);
	static std::string &getToken(const std::string &lin, unsigned int &pos, std::string &token);
	static void pasaEsp(const std::string &lin, unsigned int &pos, int avance = 1);
	static void pasaSoloEsp(const std::string &lin, unsigned int &pos, int avance = 1);
	static std::string &getLiteral(const std::string &lin, unsigned int &pos,
	                          char delim, std::string &lit);
	static bool compruebaFlotante(const std::string &s);
	static std::string toString(void *);
	static std::string toString(int, int = 0);
	static std::string toString(unsigned int, int = 0);
	static std::string toString(unsigned long int, int = 0);
	static std::string toString(double, int = 0, int = 0);
	static int toInt(const std::string &);
};

}

#endif

// mediosoporte.cpp
/*
	Implementaci�n de los diferentes medios de soporte
	para objetos Zero.
*/

#include <cstdlib>
#include <cmath>

#include "mediosoporte.h"
#include "excep.h"

namespace Zero {

// =============================================================== MedioSoporte
// ------------------------------------------------------------ codificarReal()
REAL MedioSoporte::decodificarReal(INT32 mantisa, INT32 exponente)
/**
 * Recompone un n�mero real a partir de su mantisa y exponente
 * @return el n�mero real recompuesto
 *
*/
{
	return std::ldexp( ( ( (REAL) mantisa ) / precisionFlt ), exponente );
}


void MedioSoporte::codificarReal(REAL x, INT32 &mantisa, INT32 &exponente)
/**
 * Descompone un n�mero real en su mantisa y exponente.
 * La mantisa y el exponente son devueltos por refencia.
 * @return por referencia, la mantisa y el exponente del n�mero real
 *
*/
{
	// Descomponer
	x       =  std::frexp( x, &exponente );
	mantisa = ( (int) ( x * precisionFlt ) );
}

// ------------------------------------------- MedioSoporte::copiarArgumentos()
MedioSoporte::~MedioSoporte()
{
}

// ============================================================= FicheroSoporte
const std::string FicheroSoporte::aperturaNuevo                     = "wb";
const std::string FicheroSoporte::aperturaExistente                 = "rb";

// ------------------------------------------- FicheroSoporte::FicheroSoporte()
FicheroSoporte::FicheroSoporte(const std::string &n, ModoApertura ap)
	: nomf(n)
{
	f = fopen( nomf.c_str(),
	           ap == Nuevo ?
			aperturaNuevo.c_str()
		      : aperturaExistente.c_str()
		)
	;

	if ( f == NULL ) {
		throw EMedioNoEncontrado( nomf.c_str() );
	}
}

FicheroSoporte::~FicheroSoporte()
{
	fclose( f );
}

// -------------------------------------------------- FicheroSoporte::esFinal()
bool FicheroSoporte::esFinal()
{
	return feof( f );
}

// --------------------------------------------------- FicheroSoporte::existe()
bool FicheroSoporte::existe(const std::string &n)
{
	bool toret = false;

	FILE * f = fopen( n.c_str(), aperturaExistente.c_str() );

    if ( f != NULL ) {
        fclose( f );
        toret = true;
    }

	return toret;
}

// ------------------------------------------------------ M�todos de bajo nivel
// --------------------------------------------- FicheroSoporte::leeBajoNivelUINT8
UINT8 FicheroSoporte::leeBajoNivelUINT8()
/**
        Este m�todo cambiar� seg�n la implementaci�n de UINT8
*/
{
        size_t leidos;
        UINT8 toret;

        leidos = fread( &toret, 1, 1, f );

        if ( leidos != 1
          && !feof( f ) )
        {
            throw EInputOutput( "se esperaba leer 1 byte" );
        }

        return toret;
}

// ----------------------------------------- FicheroSoporte::escribeBajoNivelUINT8
void FicheroSoporte::escribeBajoNivelUINT8(UINT8 x)
/**
        Este m�todo cambiar� seg�n la implementaci�n de UINT8
*/
{
        fwrite( &x, 1, 1, f );
}

// --------------------------------------- FicheroSoporte::escribeBajoNivelUINT16
void FicheroSoporte::escribeBajoNivelUINT16(UINT16 x)
/**
        Este m�todo cambiar� seg�n la implementaci�n de UINT16
*/
{
        UINT8 aux = x & 0x00FF;          // Parte m�s baja del int16

        escribeBajoNivelUINT8( aux );

        aux = x >> 8;                    // Parte m�s alta del int16

        escribeBajoNivelUINT8( aux );

}

// ------------------------------------------FicheroSoporte::leeBajoNivelUINT16()
UINT16 FicheroSoporte::leeBajoNivelUINT16()
/**
        Este m�todo cambiar� seg�n la implementaci�n de UINT8 y la de UINT16
*/
{
        UINT16 toret;
        UINT8  aux;

        toret = leeBajoNivelUINT8();     // Leer la parte m�s baja del int16
        aux   = leeBajoNivelUINT8();     // Leer la parte m�s alta del int16

        return (toret ^= (aux << 8));    // "Colocar" en la parte alta
}

// -------------------------------------- FicheroSoporte::escribeBajoNivelINT16()
void FicheroSoporte::escribeBajoNivelINT16(INT16 x)
/**
        Este m�todo cambiar� seg�n la implementaci�n de INT16
        Se guarda el n�mero entero seg�n la estrategia signo + m�dulo
*/
{
        bool   negativo     = ( x < 0 );
        UINT16 numero       = std::abs( x );

        // Preparar la codificaci�n del n�mero
        if ( negativo ) {
                numero ^= 0x8000;
        }

        // Guardar la codificaci�n del n�mero
        UINT16 aux = numero & 0x00FF;          // Parte m�s baja del int32

        escribeBajoNivelUINT8( aux );

        aux = numero >> 8;                     // Parte m�s alta del int32

        escribeBajoNivelUINT8( aux );
}

// ------------------------------------------ FicheroSoporte::leeBajoNivelINT16()
INT16 FicheroSoporte::leeBajoNivelINT16()
/**
        Este m�todo cambiar� seg�n la implementaci�n de UINT8
        Se guarda el n�mero entero seg�n la estrategia signo + m�dulo
*/
{
        INT16  toret;
        UINT16 numero;
        UINT8 aux;

        numero  = leeBajoNivelUINT8();   // Leer la parte m�s baja del int16
        aux     = leeBajoNivelUINT8();   // Leer la parte m�s alta del int16

        numero ^= aux << 8;                 // "Colocar" en la parte alta

        // Es negativo ?
        if ( ( numero & 0x8000 ) > 0 )
                // Quitar el c�digo del signo y poner negativo
                toret = (numero & 0x7FFF) * -1;
        else    toret = numero;

        return toret;
}

// -------------------------------------- FicheroSoporte::escribeBajoNivelINT32()
void FicheroSoporte::escribeBajoNivelINT32(INT32 x)
/**
        Este m�todo cambiar� seg�n la implementaci�n de UINT32
        Se guarda el n�mero entero seg�n la estrategia signo + m�dulo
*/
{
        bool negativo       = ( x < 0 );
        UINT32 numero       = abs( x );

        // Preparar la codificaci�n del n�mero
        if ( negativo ) {
                numero ^= 0x80000000;
        }

        // Guardar la codificaci�n del n�mero
        UINT16 aux = numero & 0x0000FFFF;          // Parte m�s baja del int32

        escribeBajoNivelUINT16( aux );

        aux = numero >> 16;                        // Parte m�s alta del int32

        escribeBajoNivelUINT16( aux );
}

// ------------------------------------------ FicheroSoporte::leeBajoNivelINT32()
INT32 FicheroSoporte::leeBajoNivelINT32()
/**
        Este m�todo cambiar� seg�n la implementaci�n de UINT8 y la de UINT16
        Se lee un n�mero entero en formato signo + m�dulo
*/
{
        INT32  toret;
        UINT32 numero;
        UINT16 aux;

        numero  = leeBajoNivelUINT16();      // Leer la parte m�s baja del int32
        aux     = leeBajoNivelUINT16();      // Leer la parte m�s alta del int32

        numero ^= aux << 16;                 // "Colocar" en la parte alta

        // Es negativo ?
        if ( ( numero & 0x80000000 ) > 0 )
                // Quitar el c�digo del signo y poner negativo
                toret = ( numero & 0x7FFFFFFF ) * -1;
        else    toret = numero;

        return toret;
}

// ------------------------------------------- FicheroSoporte::leeBajoNivelREAL()
REAL FicheroSoporte::leeBajoNivelREAL()
/**
	Lee un real desde el bytecode.
	La funci�n ldexp() devuelve un n�mero real a partir de una
	mantisa entre 1/2 y 1 y un exponente.
		numero = mantisa * 2^exponente
	As�, la mantisa es un n�mero real que se divide por una precisi�n
	determinada (1 mill�n)
	para obtener un n�mero flotante.
	Se consigue una representaci�n portable basada en 2 enteros
	con signo.
*/
{
	// Leer
	INT32 mantisa    = leeBajoNivelINT32();
	INT32 exponente  = leeBajoNivelINT32();

	return decodificarReal( mantisa, exponente );
}

// --------------------------------------- FicheroSoporte::escribeBajoNivelREAL()
void FicheroSoporte::escribeBajoNivelREAL(REAL x)
/**
	Escribe un real desde el bytecode.
	La funci�n frexp() particiona un n�mero real en una
	mantisa entre 1/2 y 1 y un exponente.
		numero = mantisa * 2^exponente
	As�, la mantisa es un n�mero real que se multiplica por una precisi�n
	determinada (1 mill�n)
	para obtener un n�mero entero.
	Se consigue una representaci�n portable basada en 2 enteros
	con signo.
*/
{
	INT32 mantisa;
	INT32 exponente;

	codificarReal( x, mantisa, exponente );

	// Escribir
	escribeBajoNivelINT32( mantisa );
	escribeBajoNivelINT32( exponente );
}

// ---------------------------------------- FicheroSoporte::escribeBajoNivelStr()
void FicheroSoporte::escribeBajoNivelStr(const std::string &x)
/** Escribe una cadena como bytecode, finaliz�ndola con un byte 0 */
{
	// IMPORTANTE: escribe el 0. DEBE HACERLO.
    fwrite( x.c_str(), x.length() + 1, 1, f );
}

// -------------------------------------------- FicheroSoporte::leeBajoNivelStr()
std::string &FicheroSoporte::leeBajoNivelStr(std::string &str)
/**
	Lee una cadena como bytecode, leyendo hasta encontrar un cero.
	Hay que pasarle la cadena destino como referencia, para evitar
	copias innecesarias.
*/
{
    int c;
    str.clear();

        // Leer la cadena
    do {
		c = fgetc( f );

       	if ( c != '\0'
		  && c != EOF )     // Si hemos llegado al final, no a�adir, �eh?
              	{
	                str.push_back( c );
              	}
        }
    while ( c != '\0'
	 &&     c != EOF );     // Hasta encontrar el 0

    return str;
}


// ================== Funciones de nivel medio: guardan los formatos requeridos
// -------------------------------------------- FicheroSoporte::escribeRegistro()
void FicheroSoporte::escribeRegistro(UINT8 cr)
/// Escribe la referencia a un registro de la m�quina
{
        escribeBajoNivelUINT8( 0 );
        escribeBajoNivelUINT8( 1 );
        escribeBajoNivelUINT8( 0 );
        escribeBajoNivelUINT8( (UINT8) cr );
}

// ------------------------------------------------ FicheroSoporte::leeRegistro()
bool FicheroSoporte::leeRegistro(UINT8 & reg)
/// Lee la referencia a un registro de la m�quina
{
        bool toret       = false;
        size_t       pos = ftell( f );

        reg = leeBajoNivelUINT8();
        if ( reg == 0 )
        {
                reg = leeBajoNivelUINT8();
                if ( reg == 1 )
                {
                        reg = leeBajoNivelUINT8();
                        if ( reg == 0 )
                        {
                                toret = true;
                                reg   = leeBajoNivelUINT8();
                        }
                }
        }

	// No era un registro !
        if ( !toret ) {
                fseek( f, pos, SEEK_SET );
        }

        return toret;
}

// --------------------------------------- FicheroSoporte::escribeIdentificador()
void FicheroSoporte::escribeIdentificador(const std::string &id)
/// Escribe un identificador (vble local, por ejemplo) al bytecode
{
        escribeBajoNivelStr( id );
}

// ------------------------------------------- FicheroSoporte::leeIdentificador()
std::string &FicheroSoporte::leeIdentificador(std::string &id)
/// Lee un identificador (vble local, por ejemplo) del bytecode
{
        return leeBajoNivelStr( id );
}

// ------------------------------------------ FicheroSoporte::escribeReferencia()
void FicheroSoporte::escribeReferencia(const std::string &id)
/// Escribe una referencia ( System.console, por ejemplo) al bytecode
{
        escribeBajoNivelStr( id );
}

// -------- --------------------------------------FicheroSoporte::leeReferencia()
std::string &FicheroSoporte::leeReferencia(std::string &ref)
/// Lee una referencia ( System.console, por ejemplo) del bytecode
{
        return leeBajoNivelStr( ref );
}

// ================ Funciones de alto nivel: leen los tipos de datos soportados
// ------------------------------------------------- FicheroSoporte::escribeStr()
inline
void FicheroSoporte::escribeStr(const std::string &x)
/**
	Escribe una cadena al bytecode
*/
{
        escribeBajoNivelStr( x );
}

// ----------------------------------------------------- FicheroSoporte::leeStr()
inline
std::string &FicheroSoporte::leeStr(std::string &str)
/**
	Lee una cadena del bytecode
*/
{
        return leeBajoNivelStr( str );
}

// --------------------------------------------- FicheroSoporte::escribeNumReal()
inline
void FicheroSoporte::escribeNumReal(REAL x)
/// Escribe un n�mero real al bytecode
{
        escribeBajoNivelREAL( x );
}

inline
REAL FicheroSoporte::leeNumReal()
/// Lee un n�mero real guardado utilizando el complemento a 2
{
        return leeBajoNivelREAL();
}

// ------------------------------------------- FicheroSoporte::escribeNumEntero()
inline
void FicheroSoporte::escribeNumEntero(INT32 x)
/// Escribe un n�mero entero de 32 bits al bytecode
{
        escribeBajoNivelINT32( x );
}

// ----------------------------------------------- FicheroSoporte::leeNumEntero()
inline
INT32 FicheroSoporte::leeNumEntero()
/// Lee un n�mero entero de 32 bits del bytecode
{
        return leeBajoNivelINT32();
}

}

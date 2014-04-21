// mediosoporte.h
/*
	Lleva los diferentes soportes posibles para objetos Zero.
*/

#ifndef __MEDIOSOPORTE__H
#define __MEDIOSOPORTE__H

#include <cstdio>
#include <string>

namespace Zero {

// Tipos  ---------------------------------------------------------------------
typedef unsigned char   UINT8;
typedef uint16_t        UINT16;
typedef int16_t         INT16;
typedef int32_t         INT32;
typedef uint32_t        UINT32;
typedef double          REAL;

/**
	La clase MedioSoporte representa el soporte de los mnemos. Un
	soporte popular es el archivo, pero derivando de esta clase pueden
	aparecer otros soportes, como por ejemplo, la red.
*/
class MedioSoporte {
public:
	static const unsigned int precisionFlt = 100000000;

	static REAL decodificarReal(INT32 mantisa, INT32 exponente);
	static void codificarReal(REAL x, INT32 &mantisa, INT32 &exponente);

	virtual ~MedioSoporte() = 0;

	// Bajo nivel
        virtual UINT8   leeBajoNivelUINT8()                              = 0;
        virtual UINT16  leeBajoNivelUINT16()                             = 0;
        virtual REAL    leeBajoNivelREAL()                               = 0;
        virtual INT16   leeBajoNivelINT16()                              = 0;
        virtual INT32   leeBajoNivelINT32()                              = 0;
        virtual std::string &leeBajoNivelStr(std::string &)                   = 0;
        virtual void escribeBajoNivelUINT8(UINT8)                        = 0;
        virtual void escribeBajoNivelUINT16(UINT16)                      = 0;
        virtual void escribeBajoNivelINT16(INT16)                        = 0;
        virtual void escribeBajoNivelINT32(INT32)                        = 0;
        virtual void escribeBajoNivelStr(const std::string &)                 = 0;

	// Datos usuario
        virtual void escribeStr(const std::string &x)                    = 0;
        virtual std::string &leeStr(std::string &)                            = 0;

        virtual REAL leeNumReal()                                        = 0;
        virtual void escribeNumReal(REAL x)                              = 0;
        virtual void escribeBajoNivelREAL(REAL)                          = 0;

        virtual void escribeNumEntero(INT32 x)                           = 0;
        virtual INT32 leeNumEntero()                                     = 0;

	// Datos de la máquina virtual
        virtual void escribeRegistro(UINT8)                              = 0;
        virtual bool leeRegistro(UINT8 &)                                = 0;

        virtual void escribeIdentificador(const std::string &)                = 0;
        virtual std::string &leeIdentificador(std::string&)                        = 0;

        virtual void escribeReferencia(const std::string &)                   = 0;
        virtual std::string &leeReferencia(std::string &)                          = 0;

	virtual bool esFinal()						 = 0;
};

/**
	La clase FicheroSoporte se encarga de dar soporte a la carga y
	salvaguarda de los mnemos mediante el sistema de ficheros.
*/
class FicheroSoporte : public MedioSoporte {
public:
	typedef enum { Existente, Nuevo } ModoApertura;
private:
	FILE *f;
	std::string nomf;

	static const std::string aperturaNuevo;
	static const std::string aperturaExistente;

public:
	FicheroSoporte(const std::string &, ModoApertura);
	~FicheroSoporte();

	const std::string &getNombre() const
		{ return nomf; }

	static bool existe(const std::string &);

	// Bajo nivel
        virtual UINT8   leeBajoNivelUINT8();
        virtual UINT16  leeBajoNivelUINT16();
        virtual REAL    leeBajoNivelREAL();
        virtual INT16   leeBajoNivelINT16();
        virtual INT32   leeBajoNivelINT32();
        virtual std::string &leeBajoNivelStr(std::string &);
        virtual void escribeBajoNivelUINT8(UINT8);
        virtual void escribeBajoNivelUINT16(UINT16);
        virtual void escribeBajoNivelINT16(INT16);
        virtual void escribeBajoNivelINT32(INT32);
        virtual void escribeBajoNivelStr(const std::string &);

	// Datos usuario
        virtual void escribeStr(const std::string &x);
        virtual std::string &leeStr(std::string &);

        virtual REAL leeNumReal();
        virtual void escribeNumReal(REAL x);
	virtual void escribeBajoNivelREAL(REAL);

        virtual void escribeNumEntero(INT32 x);
        virtual INT32 leeNumEntero();

	// Datos de la máquina virtual
        virtual void escribeRegistro(UINT8);
        virtual bool leeRegistro(UINT8 &);

        virtual void escribeIdentificador(const std::string &);
        virtual std::string &leeIdentificador(std::string&);

        virtual void escribeReferencia(const std::string &);
        virtual std::string &leeReferencia(std::string &);

	virtual bool esFinal();
};

}

#endif


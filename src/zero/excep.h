// excep.h
/*
        Trata los posibles errores (excepciones)
        que pueden suceder dentro de la máquina virtual.

        Forma parte de zmv.

        jbgarcia@uvigo.es
*/

#ifndef EXCEP_H
#define EXCEP_H

#include <string>
#include <cstring>
#include <exception>
#include <stdexcept>

namespace Zero {

class Excepcion : public std::runtime_error {
public:
        static const unsigned int MaxTamDetalles = 1024;

        Excepcion() : std::runtime_error( "ERROR interno" )
            { *detalles = 0; }
        Excepcion(const char * y) : std::runtime_error( "ERROR interno" )
            { std::strcpy( detalles, y ); }
        Excepcion(const char * x, const char * y)
                : std::runtime_error( x )
            { std::strcpy( detalles, y ); }
        Excepcion(const Excepcion &exc) : runtime_error( exc )
            { std::strcpy( detalles, exc.getDetalles() ); }

        virtual ~Excepcion() throw()
            {}

        virtual const char * getMensaje(void) const throw()
            { return what(); }

        virtual const char * getDetalles(void) const throw()
            { return detalles; }

        bool hayDetalles() const throw()
            { return ( *detalles != 0 ); }

        std::string toString() const
            { return std::string( what() )
                    + ( hayDetalles()? "": '(' + std::string( detalles ) + ')'
              );
            }
protected:
        char detalles[MaxTamDetalles];
};

class EIlegal : public Excepcion {
public:
        EIlegal(const char * x) : Excepcion( "ERROR mnemo ilegal", x )
                {}
        EIlegal(const char * x, const char * y) : Excepcion( x, y )
                {}
};

class EDuplicado : public EIlegal {
public:
        EDuplicado(const char * x) : EIlegal( "ERROR identificador duplicado", x )
                {}
};

class ENoEsZero : public EIlegal {
public:
        ENoEsZero(const char *x)
            : EIlegal( "ERROR medio no contiene objetos Zero", x )
            {}
};

class EVersionIlegal : public EIlegal {
public:
        EVersionIlegal(const char *x) : EIlegal( "ERROR mv no soporta vers.", x )
            {}
};

class ELibIlegal : public EIlegal {
public:
        ELibIlegal(const char * x) : EIlegal ( "ERROR lib incongruente", x)
            {}
};

class EOpcodeIlegal : public EIlegal {
public:
        EOpcodeIlegal(const char * x) : EIlegal( "ERROR opcode desconocido", x )
            {}
};

class EBootstrap: public Excepcion {
public:
        EBootstrap(const char * x) : Excepcion( "FATAL Imposible completar bootstrap", x)
            {}
        EBootstrap(const char * x, const char * y) : Excepcion( x, y )
            {}
};

class EEjecucion: public Excepcion {
public:
        EEjecucion(const char * x): Excepcion( "FATAL Imposible completar mth", x )
            {}
};

class EInterno : public Excepcion {
public:
        EInterno(const char * x): Excepcion( "ERROR interno", x )
              {}
        EInterno(const char * x, const char * y): Excepcion( x, y )
              {}
};

class ENoEsNumero : public EInterno {
public:
        ENoEsNumero(const char * x): EInterno( "ERROR esperando num.", x )
              {}
};

class EDivisionPorCero : public EInterno {
public:
        EDivisionPorCero(const char * x): EInterno( "ERROR div. por cero", x )
              {}
};

class ECompilacion : public Excepcion {
private:
	int lin;
public:
    ECompilacion(const char * x): Excepcion( "ERROR Compilando", x ), lin(-1)
              {}
    ECompilacion(const char * x, const char * y): Excepcion( x, y ), lin(-1)
              {}
	unsigned int getLinMetodo()
		{ return lin; }
	void putLinMetodo(unsigned int x)
		{ lin = ( ( lin < 0 ) ? x : lin ); }
};

class ESintaxis : public ECompilacion {
public:
        ESintaxis(const char * x): ECompilacion( "ERROR Sintaxis incorrecta", x )
              {}
        ESintaxis(const char * x, const char * y): ECompilacion( x, y )
              {}
};

class ESintxMnemoInvalido: public ESintaxis {
public:
        ESintxMnemoInvalido(const char * x): ESintaxis( "ERROR mnemo desconocido", x )
               {}
};

class ESintxFltNoValido : public ESintaxis {
public:
        ESintxFltNoValido(const char * x): ESintaxis( "ERROR Se esperaba un num. flotante", x )
               {}
};

class ESintxComillasEsperadas: public ESintaxis {
public:
        ESintxComillasEsperadas(const char * x): ESintaxis( "ERROR Se esperaban comillas (\")", x )
              {}
};

class ESintxIdNoValido : public ESintaxis {
public:
        ESintxIdNoValido(const char * x): ESintaxis( "ERROR Identificador incorrecto", x )
              {}
};

class ESintxNumNoValido : public ESintaxis {
public:
        ESintxNumNoValido(const char * x): ESintaxis( "ERROR Literal flotante incorrecto", x )
              {}
};

class ESintxAnidamiento : public Excepcion {
public:
        ESintxAnidamiento(const char * x): Excepcion( "ERROR Anidamiento incorrecto", x )
              {}
        ESintxAnidamiento(const char * x, const char * y): Excepcion( x, y )
              {}
};

class ESintxObjNoPermitido : public ESintxAnidamiento {
public:
        ESintxObjNoPermitido(const char * x): ESintxAnidamiento( "ERROR Anidamiento. OBJ no permitido", x )
              {}
};

class ESintxFltNoPermitido : public ESintxAnidamiento {
public:
        ESintxFltNoPermitido(const char * x): ESintxAnidamiento( "ERROR Anidamiento. FLT no permitido", x )
              {}
};

class ESintxIntNoPermitido : public ESintxAnidamiento {
public:
        ESintxIntNoPermitido(const char * x): ESintxAnidamiento( "ERROR Anidamiento. INT no permitido", x )
              {}
};

class ESintxMthNoPermitido : public ESintxAnidamiento {
public:
        ESintxMthNoPermitido(const char * x): ESintxAnidamiento( "ERROR Def. MTH no permitido", x )
              {}
};


class ESintxDefNoPermitido : public ESintxAnidamiento {
public:
        ESintxDefNoPermitido(const char * x): ESintxAnidamiento( "ERROR Def. REF no permitido", x )
              {}
};

class ESintxAtrNoPermitido : public ESintxAnidamiento {
public:
        ESintxAtrNoPermitido(const char * x): ESintxAnidamiento( "ERROR Def. ATR no permitido", x )
              {}
};

class ESintxRetNoPermitido : public ESintxAnidamiento {
public:
        ESintxRetNoPermitido(const char * x): ESintxAnidamiento( "ERROR RET fuera de mth o duplicado", x )
              {}
};

class ESintxSetNoPermitido: public ESintxAnidamiento {
public:
        ESintxSetNoPermitido(const char * x): ESintxAnidamiento( "ERROR SET fuera de mth", x )
              {}
};

class ESintxAsgNoPermitido : public ESintxAnidamiento {
public:
        ESintxAsgNoPermitido(const char * x): ESintxAnidamiento( "ERROR ASG fuera de mth", x )
              {}
};

class ESintxMsgNoPermitido : public ESintxAnidamiento {
public:
        ESintxMsgNoPermitido(const char * x): ESintxAnidamiento( "ERROR MSG fuera de mth", x )
              {}
};

class ESintxJmpNoPermitido : public ESintxAnidamiento {
public:
        ESintxJmpNoPermitido(const char * x): ESintxAnidamiento( "ERROR JMP fuera de mth", x )
              {}
};

class ESintxJotNoPermitido : public ESintxAnidamiento {
public:
        ESintxJotNoPermitido(const char * x): ESintxAnidamiento( "ERROR JOT fuera de def. de mth", x )
              {}
};

class ESintxJofNoPermitido : public ESintxAnidamiento {
public:
        ESintxJofNoPermitido(const char * x): ESintxAnidamiento( "ERROR JOF fuera de def. de mth", x )
              {}
};


class ESintxIofNoPermitido : public ESintxAnidamiento {
public:
        ESintxIofNoPermitido(const char * x): ESintxAnidamiento( "ERROR IOF fuera de mth", x )
              {}
};


class ESintxEnoNoPermitido : public ESintxAnidamiento {
public:
        ESintxEnoNoPermitido(const char * x): ESintxAnidamiento( "ERROR ENO sin OBJ", x )
              {}
};

class ESintxEnmNoPermitido : public ESintxAnidamiento {
public:
        ESintxEnmNoPermitido(const char * x): ESintxAnidamiento( "ERROR ENM inesperado, o sin MTH", x )
              {}
};

class ESintxTrwNoPermitido : public ESintxAnidamiento {
public:
        ESintxTrwNoPermitido(const char * x): ESintxAnidamiento( "ERROR TRW no permitido", x )
              {}
};

class ESintxStrNoPermitido : public ESintxAnidamiento {
public:
        ESintxStrNoPermitido(const char * x): ESintxAnidamiento( "ERROR STR fuera de mth", x )
              {}
};

class ESintxNopNoPermitido : public ESintxAnidamiento {
public:
        ESintxNopNoPermitido(const char * x): ESintxAnidamiento( "ERROR NOP fuera de mth", x )
              {}
};

class ESintxEtqNoPermitido : public ESintxAnidamiento {
public:
        ESintxEtqNoPermitido(const char * x): ESintxAnidamiento( "ERROR ETQ fuera de mth", x )
              {}
};

class ESintxMtaNoPermitido : public ESintxAnidamiento {
public:
        ESintxMtaNoPermitido(const char * x): ESintxAnidamiento( "ERROR MTA fuera de mth y/o de objeto", x )
              {}
};

class EGestionMemoria : public Excepcion {
public:
        EGestionMemoria(const char * x) : Excepcion( "ERROR manejo de memoria", x )
                {}
        EGestionMemoria(const char * x, const char * y) : Excepcion( x, y )
                {}
};

class ENoHayMemoria: public EGestionMemoria {
public:
        ENoHayMemoria(const char * x): EGestionMemoria( "ERROR No hay suficiente memoria", x )
            {}
};

class EMemPaginaErronea: public EGestionMemoria {
public:
        EMemPaginaErronea(const char * x): EGestionMemoria( "ERROR Dir. no correspondiente a pag.", x)
            {}
};

class EChkError : public ECompilacion {
public:
        EChkError(const char * x) : ECompilacion( "ERROR en chequeo de mnemos", x)
                {}
        EChkError(const char * x, const char * y) : ECompilacion( x, y )
                {}
};

class EChkEtqNoDefinida : public EChkError {
public:
        EChkEtqNoDefinida(const char * x) : EChkError( "ERROR chequeo: etiqueta no definida", x )
                {}
};

class EChkEtqRedefinida : public EChkError {
public:
        EChkEtqRedefinida(const char * x) : EChkError( "ERROR chequeo: etiqueta duplicada", x )
                {}
};

class EChkSaltoInvalido : public EChkError {
public:
        EChkSaltoInvalido(const char * x) : EChkError( "ERROR chequeo: salto a 0 o incorrecto", x )
                {}
};

class EChkMetaInvalido : public EChkError {
public:
        EChkMetaInvalido(const char * x) : EChkError( "ERROR chequeo: MTA incorrecto", x )
                {}
};

class EChkArgInvalido : public EChkError {
public:
        EChkArgInvalido(const char * x) : EChkError( "ERROR chequeo: argumento en MSG incorrecto", x )
                {}
};

class EChkHerenciaCiclica : public EChkError {
public:
	EChkHerenciaCiclica(const char * x) : EChkError( "ERROR chequeo: herencia en bucle", x )
				{}
};

class EInputOutput : public Excepcion {
public:
        EInputOutput(const char * x): Excepcion( "ERROR Problema de entrada/salida", x )
              {}
        EInputOutput(const char * x, const char * y): Excepcion( x, y )
              {}
};

class ERutaInvalida : public EInputOutput {
public:
        ERutaInvalida(const char * x): EInputOutput( "ERROR ruta indicada incompleta o incorrecta", x )
              {}
};

class EMedioNoEncontrado : public EInputOutput {
public:
        EMedioNoEncontrado(const char * x): EInputOutput( "ERROR medio de soporte inexistente", x )
            {}
};

class ESeccPersNoExiste : public EInputOutput {
public:
        ESeccPersNoExiste(const char * x): EInputOutput( "ERROR Secc. persistente inexistente", x )
            {}
};

class ESemantico : public Excepcion {
public:
        ESemantico(const char * x) : Excepcion( "ERROR semantico", x )
            {}
        ESemantico(const char * x, const char * y) : Excepcion( x, y )
            {}
};

class EPersistencia : public Excepcion {
public:
        EPersistencia(const char * x) : Excepcion( "ERROR indefinido en persistencia", x )
            {}
        EPersistencia(const char * x, const char * y) : Excepcion( x, y )
            {}
};

class ELibNoEnPS : public EPersistencia {
public:
        ELibNoEnPS(const char * x) : EPersistencia( "ERROR IntStdLib no encontrada en PS", x )
                {}
};

}

#endif


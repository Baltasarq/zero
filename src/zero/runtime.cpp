// runtime.cpp
/*
        Implementación del runtime para zvm

        jbgarcia@uvigo.es
*/

#include <string>
#include <ctime>
#include <cmath>
#include "mnemos.h"
#include "runtime.h"
#include "metodos.h"
#include "objeto.h"
#include "uintf.h"
#include "persistentstore.h"
#include "directorio.h"
#include "optimizador.h"

namespace Zero {

// Versión de la máquina virtual,
// coincide con la de los opcodes
extern const UINT8 hver;
extern const UINT8 lver;

// =================================================================== Operando
Operando::Operando(Objeto *obj)
{
	this->obj = obj;

	// ¿Es un flotante?
	if ( Runtime::gestorNumerosFlotantes->busca( obj ) ) {
		esFlotante = true;
	}
	else
	if ( Runtime::gestorNumerosEnteros->busca( obj ) ) {
		esFlotante = false;
	}
	else throw ENoEsNumero( obj->getNombre().c_str() );
}

// ---------------------------------------------------------- Operando::toInt()
inline
INT32 Operando::toInt() const
{
	if ( esFlotante )
            return (INT32) *( Runtime::gestorNumerosFlotantes->busca( obj ) );
	else 	return *( Runtime::gestorNumerosEnteros->busca( obj ) );
}

// -------------------------------------------------------- Operando::toFloat()
inline
REAL Operando::toFloat() const
{
	if ( esFlotante )
            return *( Runtime::gestorNumerosFlotantes->busca( obj ) );
	else 	return (REAL) *( Runtime::gestorNumerosEnteros->busca( obj ) );
}

// -------------------------------------------------------- Operando::Asignar()
inline
bool Operando::asignar(const Operando &op)
{
	if ( esFlotante )
            return Runtime::gestorNumerosFlotantes->modifica( obj, op.toFloat() );
	else	return Runtime::gestorNumerosEnteros->modifica( obj, op.toInt() );
}

// ----------------------------------------------------- Operando::esNegativo()
inline
bool Operando::esNegativo() const
{
	if ( esFlotante )
			return ( *( Runtime::gestorNumerosFlotantes->busca( obj ) ) < 0.0 );
	else 	return ( *( Runtime::gestorNumerosEnteros->busca( obj ) ) < 0 );
}

// ------------------------------------------------------------ Operando::abs()
inline
Objeto * Operando::abs()
{
	Objeto * toret = NULL;

	if ( esFlotante ) {
			REAL num = std::fabs( *( Runtime::gestorNumerosFlotantes->busca( obj ) ) );

			toret = Runtime::gestorNumerosFlotantes->nuevo( "", num );
	}
	else {
			INT32 num = std::abs( *( Runtime::gestorNumerosEnteros->busca( obj ) ) );

			toret = Runtime::gestorNumerosEnteros->nuevo( "", num );
	}

	return toret;
}

// ------------------------------------------------------ Operando::toString()
inline
Objeto *Operando::toString()
{
	char buffer[ 256 ];

	if ( esFlotante ) {
		if ( sprintf( buffer, "%f", toFloat() ) == EOF ) {
			Runtime::ponExcepcionTipoNumero( obj->getNombre() );
			return Runtime::objetoNulo;
		}

		return Runtime::gestorCadenas->nuevo( "", buffer );
	}
	else {
		if ( sprintf( buffer, "%d", toInt() ) == EOF ) {
			Runtime::ponExcepcionTipoNumero( obj->getNombre() );
			return Runtime::objetoNulo;
		}

		return Runtime::gestorCadenas->nuevo( "", buffer );
	}
}

// -------------------------------------------------- Operando::opMatBinaria()
Objeto * Operando::opMatBinaria(const Operando &op, char opr)
{
	Objeto * toret = NULL;

	if ( esFlotante ) {
		REAL num = *( Runtime::gestorNumerosFlotantes->busca( obj ) );
		REAL divisor;

		switch( opr ) {
			case SUMA:
				  num += op.toFloat();
				  break;
			case RESTA:
				  num -= op.toFloat();
				  break;
			case MULTIPLICACION:
				  num *= op.toFloat();
				  break;
			case DIVISION:
				  divisor = op.toFloat();
				  if ( divisor == 0.0 ) {
					throw EDivisionPorCero( "ops matemáticas flt nativas" );
				  }
				  num /= divisor;
				  break;
			default:
				  throw EInterno( "opMatBinaria: operación no soportada" );
		}

		toret = Runtime::gestorNumerosFlotantes->nuevo( "", num );
	}
	else {
		INT32 num = *( Runtime::gestorNumerosEnteros->busca( obj ) );
		INT32 divisor;

		switch( opr ) {
			case SUMA:
				  num += op.toInt();
				  break;
			case RESTA:
				  num -= op.toInt();
				  break;
			case MULTIPLICACION:
				  num *= op.toInt();
				  break;
			case DIVISION:
				  divisor = op.toInt();
				  if ( divisor == 0 ) {
					throw EDivisionPorCero( "ops matemáticas int nativas" );
				  }
				  num /= divisor;
				  break;
			default:
				  throw EInterno( "opMatBinaria: operación no soportada" );
		}

		toret = Runtime::gestorNumerosEnteros->nuevo( "", num );
	}

	return toret;
}

// -------------------------------------------------- Operando::opRelBinaria()
bool Operando::opRelBinaria(const Operando &op, char opr)
{
	bool toret = false;

	if ( esFlotante ) {
		REAL num = *( Runtime::gestorNumerosFlotantes->busca( obj ) );

		switch( opr ) {
			case LT:
				  toret = ( num < op.toFloat() );
				  break;
			case GT:
				  toret = ( num > op.toFloat() );
				  break;
			case EQ:
				  toret = ( num == op.toFloat() );
				  break;
			default:
				  throw EInterno( "opRelBinaria: operación no soportada" );
		}
	}
	else {
		INT32 num = *( Runtime::gestorNumerosEnteros->busca( obj ) );

		switch( opr ) {
			case LT:
				  toret = ( num < op.toInt() );
				  break;
			case GT:
				  toret = ( num > op.toInt() );
				  break;
			case EQ:
				  toret = ( num == op.toInt() );
				  break;
			default:
				  throw EInterno( "opRelBinaria: operación no soportada" );
		}
	}

	return toret;
}

// ================================================================ Zero_String_
// -------------------------------------------------- Zero_String_::Zero_String_
Zero_String_::Zero_String_()
{
        Objeto * obj;

        // Meter la cadena vacía
        zero = NULL;
        zero = nuevo( OBJ_STRINGZERO, "", Zero_Data_::ES_SISTEMA );
        Runtime::rt()->getContainerIntStdLib()->inserta( zero );

        // La referencia de la "clase"
        cads[ Runtime::objetoCadena ] = "";

        // Crear pool
        obj = new Objeto( NULL, "", Runtime::objetoRaiz );
        Runtime::objetoCadena->copia( obj, "", Objeto::SYSTEM_COPY );
        obj->incrementaReferencias();
        cads[ obj ] = " ";
        pool[ unEspacio ] = obj;

        obj = new Objeto( NULL, "", Runtime::objetoRaiz );
        Runtime::objetoCadena->copia( obj, "", Objeto::SYSTEM_COPY );
        obj->incrementaReferencias();
        cads[ obj ] = ".";
        pool[ unPunto ] = obj;

        obj = new Objeto( NULL, "", Runtime::objetoRaiz );
        Runtime::objetoCadena->copia( obj, "", Objeto::SYSTEM_COPY );
        obj->incrementaReferencias();
        cads[ obj ] = ",";
        pool[ unaComa ] = obj;

        obj = new Objeto( NULL, "", Runtime::objetoRaiz );
        Runtime::objetoCadena->copia( obj, "", Objeto::SYSTEM_COPY );
        obj->incrementaReferencias();
        cads[ obj ] = "\"";
        pool[ unasComillas ] = obj;

        obj = new Objeto( NULL, "", Runtime::objetoRaiz );
        Runtime::objetoCadena->copia( obj, "", Objeto::SYSTEM_COPY );
        obj->incrementaReferencias();
        cads[ obj ] = "'";
        pool[ unApostrofe ] = obj;

        obj = new Objeto( NULL, "", Runtime::objetoRaiz );
        Runtime::objetoCadena->copia( obj, "", Objeto::SYSTEM_COPY );
        obj->incrementaReferencias();
        cads[ obj ] = ":";
        pool[ unosDosPuntos ] = obj;
}

Zero_String_::~Zero_String_()
{
    // Eliminar los objetos
    eliminaTodos();

    // Eliminar el pool
    for(size_t i = 0; i < FinPool; ++i) {
        delete pool[ i ];
    }
}

// -------------------------------------------------- Zero_String_::eliminaTodos
void Zero_String_::eliminaTodos()
{
        cads.clear();
}

// ------------------------------------------------------- Zero_String_::elimina
void Zero_String_::elimina(Objeto *obj)
{
        TablaCadenas::iterator it = cads.find( obj );

        if ( it != cads.end() ) {
                cads.erase( it );
        }
}

// ------------------------------------------------------ Zero_String_::modifica
bool Zero_String_::modifica(Objeto *obj, const std::string &valor)
{
        bool toret = true;

        if ( busca(obj) != NULL )
             cads[obj] = valor;
        else toret = false;

        return toret;
}

// --------------------------------------------------------- Zero_String_::nuevo
Objeto *Zero_String_::nuevo(
                    const std::string &nombre,
                    const std::string &valor,
                    bool sys)
{
    Objeto * obj = NULL;

    try {
        // Revisar el pool
        if ( valor.length() == 0 ) {
            obj = zero;
        }
        else
        if ( valor.length() == 1 ) {
            for(unsigned int i = 0; i < FinPool; ++i) {
                const std::string &s = cads[ pool[ i ] ];

                if ( s[ 0 ] == valor[ 0 ] ) {
                    obj = pool[ i ];
                    break;
                }
            }
        }

        if ( obj == NULL ) {
            // Crear el objeto
            if ( sys )
                    obj = new ObjetoSistema( NULL, "", Runtime::objetoRaiz );
            else    obj = new Objeto( NULL, "", Runtime::objetoRaiz );

            Runtime::objetoCadena->copia( obj, nombre, Objeto::SYSTEM_COPY );

            // Darle el valor adecuado
            cads[ obj ] = valor;
        }
    } catch(std::bad_alloc &)
    {
        obj = NULL;
        Runtime::ponExcepcion( Runtime::objetoExcepcion, EXC_NOMEM );
    }

    return obj;
}

// --------------------------------------------------------- Zero_String_::busca
void *Zero_String_::buscarPorObjeto(Objeto *obj)
{
        std::string *toret = NULL;
        TablaCadenas::iterator it;

        it = cads.find(obj);

        if (it != cads.end()) {
                toret = &(it->second);
        }

        return toret;
}

// -------------------------------------------------------- Zero_String_::equals
bool Zero_String_::equals(Objeto * arg1, Objeto * arg2)
{
        bool toret = false;
        std::string * s1;
        std::string * s2;

        s1 = busca(arg1);
        s2 = busca(arg2);

        if (s1 != NULL
         && s2 != NULL)
        {
                if ((*s1) == (*s2))
                        toret = true;
        }

        return toret;
}

// ================================================================= Zero_Float_
// ---------------------------------------------------- Zero_Float_::Zero_Float_
Zero_Float_::Zero_Float_()
{
        Objeto * obj;
        REAL r = 1.0;

        // El "zero"
        zero = NULL;
        zero = nuevo( OBJ_FLOATZERO, 0.0, Zero_Data_::ES_SISTEMA );
        Runtime::rt()->getContainerIntStdLib()->inserta( zero );

        // La "clase"
        nums[ Runtime::objetoFlotante ] = 0.0;

        // Preparar el pool
        for(unsigned int i = 0; i < MaxPoolNums; ++i) {
            obj = new Objeto( NULL, "", Runtime::objetoRaiz );
            Runtime::objetoFlotante->copia( obj, "", Objeto::SYSTEM_COPY );
            obj->incrementaReferencias();
            nums[ obj ] = ( r++ );
            pool[ i ] = obj;
        }
}

Zero_Float_::~Zero_Float_()
{
    // Eliminar los objetos
    eliminaTodos();

    // Eliminar el pool
    for(unsigned int i = 0; i < MaxPoolNums; ++i) {
        delete pool[ i ];
    }

}

// --------------------------------------------------- Zero_Float_::eliminaTodos
void Zero_Float_::eliminaTodos()
{
        nums.clear();
}

// -------------------------------------------------------- Zero_Float_::elimina
void Zero_Float_::elimina(Objeto *obj)
{
        TablaFlotantes::iterator it = nums.find( obj );

        if ( it != nums.end() ) {
                nums.erase( it );
        }
}


// ---------------------------------------------------------- Zero_Float_::busca
void * Zero_Float_::buscarPorObjeto(Objeto * obj)
{
        REAL *toret = NULL;
        TablaFlotantes::iterator it;

        it = nums.find(obj);

        if (it != nums.end()) {
                toret = &( it->second );
        }

        return toret;
}

// ------------------------------------------------------- Zero_Float_::modifica
bool Zero_Float_::modifica(Objeto *obj, const REAL &valor)
{
        bool toret = true;

        if (busca( obj ) != NULL)
             nums[ obj ] = valor;
        else toret = false;

        return toret;
}

// ---------------------------------------------------------- Zero_Float_::nuevo
Objeto *Zero_Float_::nuevo(const std::string &nombre, const REAL &valor, bool sys)
{
    Objeto *obj = NULL;

    try {
        // Mirar en el pool
        if ( valor == 0.0 ) {
            obj = zero;
        }
        else
        if ( valor <= MaxPoolNums
          && valor == trunc( valor ) )
        {
            obj = pool[ ( (size_t) trunc( valor ) ) - 1 ];
        }

        if ( obj == NULL ) {
            // Crear el objeto
            if ( sys )
                    obj = new ObjetoSistema( NULL, "", Runtime::objetoRaiz );
            else    obj = new Objeto( NULL, "", Runtime::objetoRaiz );

            Runtime::objetoFlotante->copia( obj, nombre, Objeto::SYSTEM_COPY );

            // Darle el valor adecuado
            nums[obj] = valor;
        }
    } catch(std::bad_alloc &) {
        obj = NULL;
        Runtime::ponExcepcion( Runtime::objetoExcepcion, EXC_NOMEM );
    }

    return obj;
}

// --------------------------------------------------------- Zero_Float_::equals
bool Zero_Float_::equals(Objeto * arg1, Objeto * arg2)
{
        bool toret = false;
        REAL * s1;
        REAL * s2;

        s1 = busca(arg1);
        s2 = busca(arg2);

        if (s1 != NULL
         && s2 != NULL)
        {
                if ((*s1) == (*s2))
                        toret = true;
        }

        return toret;
}

// ================================================================= Zero_Int_
// ---------------------------------------------------- Zero_Int_::Zero_Int_
Zero_Int_::Zero_Int_()
{
    Objeto * obj;
    INT32 v = 1;

    // El "zero"
    zero = NULL;
    zero   = nuevo( OBJ_INTZERO, 0, Zero_Data_::ES_SISTEMA );
    Runtime::rt()->getContainerIntStdLib()->inserta( zero );

    // La "clase"
    nums[ Runtime::objetoEntero ] = 0;

    // Preparar el pool
    for(unsigned int i = 0; i < MaxPoolNums; ++i) {
        obj = new Objeto( NULL, "", Runtime::objetoRaiz );
        Runtime::objetoEntero->copia( obj, "", Objeto::SYSTEM_COPY );
        obj->incrementaReferencias();
        nums[ obj ] = ( v++ );
        pool[ i ] = obj;
    }
}

Zero_Int_::~Zero_Int_()
{
    // Eliminar los objetos
    eliminaTodos();

    // Eliminar el pool
    for(unsigned int i = 0; i < MaxPoolNums; ++i) {
        delete pool[ i + 1 ];
    }
}

// --------------------------------------------------- Zero_Int_::eliminaTodos
void Zero_Int_::eliminaTodos()
{
    nums.clear();
}

// -------------------------------------------------------- Zero_Int_::elimina
void Zero_Int_::elimina(Objeto *obj)
{
        TablaEnteros::iterator it = nums.find( obj );

        if ( it != nums.end() ) {
                nums.erase( it );
        }
}

// ---------------------------------------------------------- Zero_Int_::busca
void * Zero_Int_::buscarPorObjeto(Objeto * obj)
{
        INT32 *toret = NULL;
        TablaEnteros::iterator it;

        it = nums.find(obj);

        if (it != nums.end()) {
                toret = &(it->second);
        }

        return toret;
}

// ------------------------------------------------------- Zero_Int_::modifica
bool Zero_Int_::modifica(Objeto *obj, const INT32 &valor)
{
        bool toret = true;

        if (busca(obj) != NULL)
             nums[obj] = valor;
        else toret = false;

        return toret;
}

// ---------------------------------------------------------- Zero_Int_::nuevo
Objeto *Zero_Int_::nuevo(const std::string &nombre, const INT32 &valor, bool sys)
{
    Objeto *obj = NULL;

    try {
        // Mirar en el pool
        if ( valor == 0 ) {
            obj = zero;
        }
        else
        if ( ( (size_t) valor ) <= MaxPoolNums ) {
            obj = pool[ valor - 1 ];
        }

        if ( obj == NULL ) {
            // Crear el objeto
            if ( sys )
                    obj = new ObjetoSistema( NULL, "", Runtime::objetoRaiz );
            else    obj = new Objeto( NULL, "", Runtime::objetoRaiz );

            Runtime::objetoEntero->copia( obj, nombre, Objeto::SYSTEM_COPY );

            // Darle el valor adecuado
            nums[obj] = valor;
        }
    } catch(std::bad_alloc &)
    {
        obj = NULL;
        Runtime::ponExcepcion( Runtime::objetoExcepcion, EXC_NOMEM );
    }

    return obj;
}

// --------------------------------------------------------- Zero_Int_::equals
bool Zero_Int_::equals(Objeto * arg1, Objeto * arg2)
{
        bool toret = false;
        INT32 * s1;
        INT32 * s2;

        s1 = busca( arg1 );
        s2 = busca( arg2 );

        if ( s1 != NULL
         &&  s2 != NULL )
        {
                if ( ( *s1 ) == ( *s2 ) )
                        toret = true;
        }

        return toret;
}

// ================================================================ Zero_Vector_
// ---------------------------------------------------------- LiteralVector_Zero

// ------------------------------------------------------------------ Cons & Des
LiteralVector_Zero::LiteralVector_Zero(Objeto *asoc) : objetoAsociado(asoc)
{
}

LiteralVector_Zero::~LiteralVector_Zero()
{
	eliminaTodos();
}

void LiteralVector_Zero::eliminaTodos()
{
	// Cargarse todas las referencias
	for(register size_t i = 0;i < longitud(); ++i) {
		eliminaRef( v[i] );
	}

	// Cargase todas las posiciones
	v.clear();
}

// ------------------------------------------------------------- Crea & Des Ref's

/** Solucionamos aquí un problema repetitivo: crear y destruir las referencias
   que serán utilizadas para almacenar los punteros a objetos en el vector.
   Es interesante hacerlo con referencias, en lugar de con punteros directos,
   porque de esta forma el conteo de referencias en el objeto se establece
   de manera natural, sin "violar" más el modelo de memoria.

   Así, cada referencia está ligada al objeto asociado en el mundo de
   objetos Zero en el que el vector va a trabajar.
*/

inline
Referencia * LiteralVector_Zero::creaRef(Objeto *obj)
{
	Referencia * ref;

	try {
		ref = new Atributo(objetoAsociado,
				 ServidorDeNombres::getSigNombreUnico(),
				 obj)
		;
	} catch(...) {
		ref = NULL;
	}

	return ref;
}

inline
void LiteralVector_Zero::eliminaRef(Referencia *ref)
{
	delete ref;
}

// ------------------------------------------------ LiteralVector_Zero::insert()
inline
bool LiteralVector_Zero::insertaElemento(size_t i, Objeto * obj)
{
	Referencia * ref = creaRef( obj );
	bool toret = ( ( ref != NULL ) && ( i < longitud() ) );

	if ( toret ) {
		try {
			v.insert( v.begin() + i, ref );
		} catch (...) {
			eliminaRef( ref );
			toret = false;
		}
	}

	return toret;
}

// ------------------------------------------------ LiteralVector_Zero::remove()
inline
bool LiteralVector_Zero::eliminaPosicion(size_t i)
{
	bool toret = ( i < longitud() );

	if ( toret ) {
		try {
			VectorImplementacion::iterator it = v.begin() + i;

			eliminaRef( *it ); 	// Cargarse la referencia
			v.erase( it ); 		// Liberar la posición
		} catch(...) {
			toret = false;
		}
	}

	return toret;
}

// ------------------------------------------------ LiteralVector_Zero::remove()
inline
bool LiteralVector_Zero::modificaPosicion(size_t i, Objeto *obj)
{
	bool toret = ( i < longitud() );

	if (toret) {
		try {
			v[i]->ponReferencia(obj);	// Modificar la posición
		} catch(...) {
			toret = false;
		}
	}

	return toret;
}

// --------------------------------------------------- LiteralVector_Zero::mas()
bool LiteralVector_Zero::mas(Objeto *obj)
{
	bool toret = true;
	Referencia * ref = creaRef(obj);

	if (ref != NULL)
	{
		try {
			v.push_back(ref);
		}
		catch(...) {
			eliminaRef(ref);
			toret = false;
		}

	} else toret = false;

	return toret;
}

// ------------------------------------------------ LiteralVector_Zero::equals()
bool LiteralVector_Zero::equals(LiteralVector_Zero *v)
{
	bool toret = false;
	size_t lon = this->longitud();
	Objeto * item1;
	Objeto * item2;
	Metodo * metIqualQue;
	std::vector<Referencia *> arg;
    Atributo *ref;
    unsigned char buffer[ sizeof( Atributo ) ];  // Espacio para atr.

	// Preparar el paso de parámetros
	arg.push_back( NULL );

	if ( lon == v->longitud() )
	{
		size_t i = 0;
		for (; i < lon; ++i)
		{
			item1 = this->getElemento(i)->getObjeto();
			item2 = v->getElemento(i)->getObjeto();

			// Preparar los parámetros
			ref = new(&buffer) Atributo( item1, ARG_X, item2 );
			arg[0] = ref;

			// Localizar método "igual que" en item1
			metIqualQue = Metodo::encuentraMetodo(item1, MET_IQUALQUE);

			if (metIqualQue != NULL)
			{
				// Hacer la comparación
				metIqualQue->ejecuta( &arg, item1 );

				// Decidir
				if (metIqualQue->resultadoEjecucion() != Runtime::objetoTrue) {
					metIqualQue->finEjecucion();
					break;
				}
				metIqualQue->finEjecucion();
			}

                        // Borrar la info del atributo
                        ref->~Atributo();
		}

		if ( i == lon ) {
			toret = true;
		}
	}

	return toret;
}

// ----------------------------------------------- LiteralVector_Zero::copiaEn()
bool LiteralVector_Zero::copiaEn(LiteralVector_Zero *v)
{
	bool toret          = false;
	size_t lon          = longitud();
	Objeto *obj;
	Objeto *item;

	v->eliminaTodos();
	size_t i = 0;
	for (; i < lon; ++i)
	{
		// Copiar objeto
		obj = getElemento(i)->getObjeto();
		item = new(std::nothrow) Objeto(NULL, "", Runtime::objetoRaiz);

		if (item != NULL)
		{
			// Hacer la copia
			obj->copia(item, "");

			// introducirlo en el vector
			toret = v->mas(item);
			if (!toret) {
				break;
			}
		} else break;
	}

	if (i < lon) {
		Runtime::ponExcepcion(Runtime::objetoExcepcion, EXC_NOMEM);
	}

	return toret;
}

// ---------------------------------------------------------------- Zero_Vector_
// ------------------------------------------------------------------ Cons & Des
Zero_Vector_::Zero_Vector_()
{
    LiteralVector_Zero * vect_zero =
            new(std::nothrow) LiteralVector_Zero( Runtime::objetoVector )
    ;

    if ( vect_zero == NULL ) {
        throw ENoHayMemoria( "creando vectores Zero" );
    }

    mas( Runtime::objetoVector, vect_zero );
}

void Zero_Vector_::eliminaTodos()
{
	// Eliminar cada uno de los vectores
	TablaVectores::iterator it = vects.begin();

	for(;it != vects.end(); ++it)
	{
		delete it->second;
                it->second = NULL;
	}


	// Eliminar el esqueleto en sí
	vects.clear();
}

// ----------------------------------------------------- Zero_Vector_::elimina()
void Zero_Vector_::elimina(Objeto *obj)
{
        TablaVectores::iterator it = vects.find( obj );

        if ( it != vects.end()
          && it->second != NULL )
        {
            delete it->second;	// Borrar el vector
            vects.erase( it );	// Borrar su entrada
        }
}

// ------------------------------------------------------- Zero_Vector_::nuevo()
Objeto * Zero_Vector_::nuevo(const std::string &nombre)
{
    Objeto *obj;

    try {
        // Crear el objeto
        obj = new Objeto( NULL, "", Runtime::objetoNulo );

        Runtime::objetoVector->copia(
                                obj,
                                nombre,
                                Objeto::SYSTEM_COPY
        );
    } catch(std::bad_alloc &)
    {
            obj = NULL;
            Runtime::ponExcepcion(Runtime::objetoExcepcion, EXC_NOMEM);
    }

    return obj;
}

// --------------------------------------------- Zero_Vector_::buscarPorObjeto()
void   * Zero_Vector_::buscarPorObjeto(Objeto *obj)
{
	LiteralVector_Zero *toret = NULL;
        TablaVectores::iterator it = vects.find(obj);

        if (it != vects.end()) {
		toret = it->second;
        }

	return toret;
}

// ------------------------------------------------------ Zero_Vector_::equals()
bool Zero_Vector_::equals(Objeto *obj1, Objeto *obj2)
{
	bool toret = false;
	LiteralVector_Zero * v1 = busca(obj1);
	LiteralVector_Zero * v2 = busca(obj2);

	if (v1 != NULL
	 && v2 != NULL)
	{
		toret = v1->equals(v2);
	}

	return toret;
}

bool Zero_Vector_::copiaEn(Objeto *obj1, Objeto *obj2)
{
	bool toret = false;
	LiteralVector_Zero * v1 = busca(obj1);
	LiteralVector_Zero * v2 = busca(obj2);

	if (v1 != NULL
	 && v2 != NULL)
	{
		toret = v1->copiaEn(v2);
	}

	return toret;
}

// ---------------------------------------------------- Zero_Vector_::toString()
Objeto * toString(Objeto *obj)
{
	return NULL;
}

// =============================================================== Zero_Console_
const bool Zero_Console_::CR = true;

// --------------------------------------------------------- Zero_Console_::open
void Zero_Console_::open()
{
        if ( !VMCap::consola )
        {
                Runtime::ponExcepcion( Runtime::objetoExcepcion, EXC_CONENABLED );
        }
}

// -------------------------------------------------------- Zero_Console_::close
void Zero_Console_::close()
{
        if ( VMCap::consola )
        {
          	fflush( stdout );
        }
}

// --------------------------------------------------------- Zero_Console_::read
Objeto *Zero_Console_::read()
{
        std::string leido;
        Objeto *toret = Runtime::objetoNulo;

        if ( VMCap::consola )
        {
            std::fflush( stdout );
            std::fflush( stdin  );

            try {
                leido = getLineaInput();

                toret = Runtime::gestorCadenas->nuevo( "", leido );

                if ( toret == NULL ) {
                    Runtime::ponExcepcion( Runtime::objetoExcepcion, EXC_NOMEM );
                }
            } catch(const EInputOutput &e) {
                Runtime::ponExcepcion( Runtime::objetoExcepcion, e.getDetalles() );
            }
        }

        return toret;
}

// -------------------------------------------------------- Zero_Console_::write
void Zero_Console_::write(Objeto *obj, bool hazCR = false)
{
        std::string *str;

        if ( VMCap::consola )
        {
            if ( obj != NULL )
            {
                str = Runtime::gestorCadenas->busca( obj );

                if ( str != NULL ) {
                        putLineaOutput( *str, hazCR );
                }
                else
                {
                        Runtime::ponExcepcion( Runtime::objetoExcepcion,
                                              EXC_CONDISPLAY )
			;
                }
            }
            else { putLineaOutput( "", true ); }
        }
}

// =========================================================== GestorExcepciones
GestorExcepciones::GestorExcepciones()
{
        objetoMensajeroExcepciones = NULL;
}

GestorExcepciones::~GestorExcepciones()
{
}

Objeto * GestorExcepciones::getExcepcion()
{
	return objetoMensajeroExcepciones;
}

void GestorExcepciones::ponExcepcion(Objeto *mensajero, Objeto *mensaje)
{
		Atributo * atrTrwer   = NULL;
		Atributo * atrDetails = NULL;
		Atributo * atr        = NULL;
		Atributo * atrObjeto  = NULL;
		Objeto   * nombreMet  = NULL;
		Atributo * atrMetodo  = NULL;

		// Donde ha sucedido
        Objeto * quien = Runtime::gestorStack->getObjetoActual();
        Metodo * donde = Runtime::gestorStack->getMetodoActual();

        if ( quien == NULL ) {
                quien = Runtime::objetoNulo;
        }

		// El tipo de excepcion
        if ( mensajero != NULL ) {
		 try {
          if ( mensajero->esDescendienteDe(
                        Runtime::rt()->getContainerIntStdLib(),
                        OBJ_PADRE_EXCEPCIONES ) )
          {
            // Prepara al objeto
	    	objetoMensajeroExcepciones = new Objeto( NULL,
                            ServidorDeNombres::getSigNombreUnico(),
                            mensajero )
            ;

            // Añádele los datos
            if ( mensaje == NULL )
            {
               	mensaje = Runtime::objetoNulo;
            }

            atrTrwer  = new Atributo( objetoMensajeroExcepciones,
                                ATR_EXCTHROWER, quien
			);

            if ( donde == NULL ) {
                atrObjeto = new Atributo( objetoMensajeroExcepciones,
                                ATR_EXCOBJETO, Runtime::objetoNulo
                );
                nombreMet = Runtime::objetoNulo;
            } else {
                atrObjeto = new Atributo( objetoMensajeroExcepciones,
                                ATR_EXCOBJETO, donde->getPerteneceA()
                );
                nombreMet = Runtime::gestorCadenas->nuevo( "", donde->getNombre() );
            }


            atrMetodo = new Atributo(
                objetoMensajeroExcepciones, ATR_EXCMETODO, nombreMet
            );

            atrDetails = new Atributo(
                objetoMensajeroExcepciones, ATR_EXCDETALLES, mensaje
            );

            // Guardar el nombre del objeto donde se produjo la ofensa
			if ( !( objetoMensajeroExcepciones->lAtrs.inserta(
								ATR_EXCTHROWER, atrTrwer ) )
				)
			{
				if ( ( atr = objetoMensajeroExcepciones
                        ->lAtrs.busca( ATR_EXCTHROWER ) ) != NULL )
				{
						atr->ponReferencia( quien );
						delete atrTrwer;
						atrTrwer = NULL;
				}
			}

            // Guardar el nombre del objeto al que pertenece el mth ofensivo
			if ( !( objetoMensajeroExcepciones->lAtrs.inserta(
						ATR_EXCOBJETO, atrObjeto )
				  )
				)
			{
				if ( ( atr = objetoMensajeroExcepciones
                        ->lAtrs.busca( ATR_EXCOBJETO ) ) != NULL )
                {
					atr->ponReferencia( atrObjeto->getObjeto() );
					delete atrObjeto;
					atrObjeto = NULL;
				}
			}

            // Guardar los detalles de la excep
			if ( !( objetoMensajeroExcepciones->lAtrs.inserta(
						ATR_EXCDETALLES, atrDetails )
				) )
			{
				if ( ( atr = objetoMensajeroExcepciones
                        ->lAtrs.busca( ATR_EXCDETALLES ) ) != NULL )
				{
						atr->ponReferencia( mensaje );
						delete atrDetails;
						atrDetails = NULL;
				}
			}

              // Guardar el mth donde se produjo la excep
			if ( !( objetoMensajeroExcepciones->lAtrs.inserta(
						ATR_EXCMETODO, atrMetodo )
				  )
			   )
			{
				if ( ( atr = objetoMensajeroExcepciones
                        ->lAtrs.busca( ATR_EXCMETODO ) ) != NULL )
				{
						atr->ponReferencia( nombreMet );
						delete atrMetodo;
						atrMetodo = NULL;
				}

			}
		  } else objetoMensajeroExcepciones = mensajero;
	   } catch(...) {
			objetoMensajeroExcepciones = mensajero;
			objetoMensajeroExcepciones = mensajero;
			delete atrTrwer;
			delete atrDetails;
			delete atrObjeto;
			delete nombreMet;
			delete atrMetodo;
	   }

	   // Insertarlo en el Exe
	   Runtime::rt()->getContainerEjecucion()->inserta(
	   		objetoMensajeroExcepciones )
	   ;

	   // Prepara objeto recipiente de excepción
	   preparaObjetoRecipienteExcepcion( donde );
      } else {
                throw ELibIlegal("FATAL fue imposible generar excep., "
                                 "debido a IntStdLib inicializada "
                                 "incorrectamente"
                      );
      }
}

void GestorExcepciones::preparaObjetoRecipienteExcepcion(Metodo *donde)
{
	// Preparar el gestor de excepciones del método donde se produjo la excepción
	donde->gestionExcepciones();

	if ( !( donde->enExcepcion() )
	  && donde->tieneGestorExcepciones() )
	{
			objetoMensajeroExcepciones = NULL;
	}
	else
	// Recorrer el stack para poner todos los métodos en estado de excpeción
	for( int i = Runtime::gestorStack->getNumero() - 2;  i >= 0; --i) {
		donde = Runtime::gestorStack->getElementoNumero( i )->getMetodo();
		donde->gestionExcepciones();

		if ( !( donde->enExcepcion() )
             && donde->tieneGestorExcepciones() )
        {
			objetoMensajeroExcepciones = NULL;
			break;
		}
	}
}

// ======================================================================= Stack
/**
   El stack guarda la información de donde se ha estado anteriormente.
   En esta implementación, solamente hace eso, ya que se utiliza el propio stack
   proporcionado por el lenguaje.
*/

// --------------------------------------------------------------- StackInfoItem
Stack::StackInfoItem::StackInfoItem(Objeto *obj, Metodo * met)
    : quien( obj ), metodo( met ), numRecursivo( 0 )
{
    donde = met->getPerteneceA();

    quien->incrementaReferencias();
    donde->incrementaReferencias();

    if ( met->esRecursivo() ) {
        metodo       = met->getMetodoOrgRecursivo();
        numRecursivo = metodo->getNumLlamadasRecursivas();
    }
}

inline
std::string Stack::StackInfoItem::consNombre(Metodo *met, unsigned int numRec)
{
    std::string toret = met->getNombre();

    if ( numRec > 0 ) {
        toret += '\'';
        toret += AnalizadorLexico::toString( numRec );
    }

    return toret;
}

Metodo * Stack::StackInfoItem::getMetodo() const
/**
    Devuelve el mth del marco del stack.
    En caso de resultar ser un mth recursivo, devuelve el mth
    origen de la recursividad.
    @return un ptr al mth concreto o al mth org de recursividad.
*/
{
    Metodo * toret = metodo;

    if ( esMetodoRecursivo() ) {
        std::string nombre = consNombre( metodo, getNivelRecursivo() );
        Metodo * met = getObjetoDeMetodo()->lMets.busca( nombre );

        if ( met != NULL ) {
            toret = met;
        }
    }

    return toret;
}


std::string Stack::StackInfoItem::getNombreMetodo() const
/**
    Utilizar siempre este mth para recuperar el nombre del metodo,
    pues ya trata de manera transparente el caso de metodos recursivos.
    @return el nombre del metodo de este frame del stack, siempre correcto
*/
{
    std::string toret = getMetodo()->getNombre();

    if ( esMetodoRecursivo() ) {
        toret = consNombre( metodo, getNivelRecursivo() );
    }

    return toret;
}

// ----------------------------------------------------------------------- Stack
Stack::~Stack()
{
	eliminaTodos();
}

void Stack::eliminaTodos()
{
	while( !stackInfo.empty() ) {
	    delete stackInfo[ stackInfo.size() -1 ];
		stackInfo.pop_back();
	}
}

void Stack::call(Objeto *o, Metodo *m)
{
    try {
        stackInfo.push_back( new StackInfoItem( o, m ) );
    } catch(...) {
        throw ENoHayMemoria( "actualizando stack" );
    }
}

Objeto * Stack::getObjetoActual() const
{
        if ( getNumero() > 0 )
                return stackInfo[ getNumero() - 1 ]->getObjeto();
        else    return NULL;
}

Metodo * Stack::getMetodoActual() const
{
        if ( getNumero() > 0 )
                return stackInfo[ getNumero() - 1 ]->getMetodo();
        else    return NULL;
}

void Stack::ret(Objeto *obj, Metodo *met)
/**
        Elimina elementos del stack hasta encontrarse con el "estado"
        pasado.
*/
{
        // "Llegar" hasta el método actual
        if ( obj != NULL )
        {
            while (getMetodoActual()  != met
                && getObjetoActual()  != obj
                && getNumero() > 0)
            {
                    delete stackInfo[ stackInfo.size() - 1 ];
                    stackInfo.pop_back();
            }
        }

	if ( getNumero() > 0 ) {
		// Eliminar el método actual del stack
		delete stackInfo[ stackInfo.size() - 1 ];
		stackInfo.pop_back();
	}
}

const Stack::StackInfoItem * Stack::getElementoNumero(unsigned int i) const
{
        if ( i < getNumero() )
                return stackInfo[ i ];
        else    return NULL;
}

// ===================================================================== Runtime
Zero_String_ *Runtime::gestorCadenas          = NULL;
Zero_Float_ *Runtime::gestorNumerosFlotantes  = NULL;
Zero_Int_ *Runtime::gestorNumerosEnteros      = NULL;
GestorExcepciones *Runtime::gestorExcepciones = NULL;
Zero_Vector_ *Runtime::gestorVectores         = NULL;
Runtime *Runtime::onert                       = NULL;
Zero_Console_ *Runtime::console               = NULL;
Stack *Runtime::gestorStack                   = NULL;
Objeto *Runtime::objetoRaiz                   = NULL;
Objeto *Runtime::objetoNulo                   = NULL;
Objeto *Runtime::objetoSistema                = NULL;
Objeto *Runtime::objetoCadena                 = NULL;
Objeto *Runtime::objetoFlotante               = NULL;
Objeto *Runtime::objetoEntero                 = NULL;
Objeto *Runtime::objetoVector                 = NULL;
Objeto *Runtime::objetoExcepcion              = NULL;
Objeto *Runtime::objetoEPrivado               = NULL;
Objeto *Runtime::objetoEMismatch              = NULL;
Objeto *Runtime::objetoEMath                  = NULL;
Objeto *Runtime::objetoEObjNoEncontrado       = NULL;
Objeto *Runtime::objetoEMetNoEncontrado       = NULL;
Objeto *Runtime::objetoTrue                   = NULL;
Objeto *Runtime::objetoFalse                  = NULL;
Objeto *Runtime::objetoInfoAtr                = NULL;
Objeto *Runtime::objetoInfoMth                = NULL;
Objeto *Runtime::objetoFecha                  = NULL;
Objeto *Runtime::objetoPropiedades	          = NULL;
Objeto *Runtime::objetoContainer	          = NULL;
Compilador *Runtime::compilador               = NULL;
Conim *Runtime::conim                         = NULL;
PersistentStore *Runtime::ps                  = NULL;


// ----------------------------------------------- Runtime::esObjetoPrimitivo()
bool Runtime::esObjetoPrimitivo(Objeto *obj)
{
	return (   gestorNumerosEnteros->buscarPorObjeto( obj )     != NULL
            || gestorNumerosFlotantes->buscarPorObjeto( obj )   != NULL
            || gestorCadenas->buscarPorObjeto( obj )            != NULL
            || gestorVectores->buscarPorObjeto( obj )           != NULL )
	;
}
// -------------------------------------------- Runtime::preparaObjetoTopLevel()
Metodo * Runtime::preparaObjetoTopLevel(MensajeArranque *m)
{
        AlmacenObjetos  *cont;
        Metodo          *met;
        Objeto          *tl;
        Mnemotecnico    *instr;
        std::string      metodo;
        Compilador::ListaMnemos cuerpoMetTL;

        try {
            cont = rt()->getContainerEjecucion();

            if ( cont == NULL )
            {
                    throw EBootstrap(
                          "FATAL Creando objeto TopLevel demasiado pronto (?)"
                    );
            }

            tl = cont->busca( OBJ_TOPLEVEL );

            if ( tl == NULL ) {
                tl = creaObjetoTopLevel();
            }

            if ( tl != NULL )
            {
                met = tl->lMets.busca( MET_HAZLO );

                if ( met != NULL )
                {
                        if ( m == NULL ) {
                            goto END;
                        }

                        // Preparar el método
                        if ( m->size() == 1 )
                                metodo = MET_HAZLO;
                        else    metodo = (*m)[1];

                        // Meter los opcodes del MSG de arranque
                        instr = new NMMsg( (*m)[0], metodo );

                        // Argumentos del MSG
                        Mnemotecnico *arg;
                        for (size_t i = 2; i < m->size(); ++i)
                        {
                                std::string nomArg = ARG_ARG;
                                nomArg.append( AnalizadorLexico::toString( i ) );

                                // Crear un DEF
                                arg = new NMDef( nomArg );
                                cuerpoMetTL.insert( cuerpoMetTL.begin(), arg );

                                // Crear un SET
                                arg = new NMSet(
                                        NombreRegistro::__ACC,
                                        (*m)[i]
                                );
                                cuerpoMetTL.push_back( arg );

                                // Crear un ASG
                                arg = new NMAsg(
                                        nomArg,
                                        NombreRegistro::__ACC
                                );
                                cuerpoMetTL.push_back( arg );

                                // Meter el nombre del argumento
                                ( (NMMsg *) instr )->masArgumentos( nomArg );
                        }
                        cuerpoMetTL.push_back( instr );

                        // Meter opcodes de gestión de excepciones "uncaught"
                        // RET acc
                        instr = new NMRet( NombreRegistro::__ACC );
                        cuerpoMetTL.push_back( instr );

                        // STR "Uncaught exception: "
                        instr = new NMStr( "\nUncaught exception:\n" );
                        cuerpoMetTL.push_back( instr );

                        // MSG System.console write __acc
                        instr = new NMMsg( "System.console", "write" );
                        ( (NMMsg *)instr )->masArgumentos( LOC_ACC );
                        cuerpoMetTL.push_back( instr );

                        // MSG System.console write __exc
                        instr = new NMMsg( "System.console", "write" );
                        ( (NMMsg *)instr )->masArgumentos( LOC_EXC );
                        cuerpoMetTL.push_back( instr );

                        // MSG System.console lf
                        instr = new NMMsg( "System.console", "lf" );
                        cuerpoMetTL.push_back( instr );

                        // MSG System.console lf
                        instr = new NMMsg( "System.console", "lf" );
                        cuerpoMetTL.push_back( instr );

                        // MSG System.stack toString
                        instr = new NMMsg( "System.stack", MET_TOSTRING );
                        cuerpoMetTL.push_back( instr );

                        // MSG System.console write __acc
                        instr = new NMMsg( "System.console", "write" );
                        ( (NMMsg *)instr )->masArgumentos( LOC_ACC );
                        cuerpoMetTL.push_back( instr );

                        // MSG System.console lf
                        instr = new NMMsg( "System.console", "lf" );
                        cuerpoMetTL.push_back( instr );

                        // Intro instrucciones en método
                        met->masInstrucciones( cuerpoMetTL );
                }
                else throw EBootstrap("TopLevel sin método doIt() (?)");
            }
            else throw EBootstrap("TopLevel no encontrado");
        }
        catch(const std::bad_alloc &) {
                throw ENoHayMemoria( "preparando TopLevel" );
        }
        catch(Excepcion &e)
        {
                throw;
        }
        catch(...)
        {
                throw EBootstrap(
                       "FATAL Preparando objeto TopLevel demasiado pronto (?)");
        }

        END:
        return met;
}

// ----------------------------------------------- Runtime::creaObjetoTopLevel()
Objeto *Runtime::creaObjetoTopLevel()
{
        AlmacenObjetos *cont;
        Metodo    *met;
        Objeto    *toret = NULL;

        try {
            cont = rt()->getContainerEjecucion();

            if ( cont == NULL )
            {
                    throw EBootstrap(
                              "FATAL Creando objeto TopLevel demasiado pronto");
            }

            toret = cont->crea( OBJ_TOPLEVEL, Runtime::objetoRaiz );

            if ( toret == NULL) {
                    throw ENoHayMemoria("Creando TopLevel");
            }

            met   = new(std::nothrow) Metodo( MET_HAZLO,
                        toret,
                        MixinConArgumentos::argumentosNulos
            );

            if ( met == NULL ) {
                throw ENoHayMemoria("Creando TopLevel");
            }

            toret->lMets.inserta( MET_HAZLO, met );
        }
        catch(Excepcion &e)
        {
                throw;
        }
        catch(...)
        {
                throw EBootstrap(
                             "FATAL Creando objeto TopLevel demasiado pronto");
        }

        return toret;
}

// --------------------------------------------- Runtime::iniciarRuntimeConPS()
inline
void Runtime::iniciarRuntimeConPS()
/*
 * Este método privado es llamado sólo desde el constructor.
 * Cualquier otra invocación es un error.
 */
{
    PersistentStoreEnDisco *ps;
	std::string dirAct = Directorio::getNombreDirectorioActual();
    std::string lib = OBJ_INTSTDLIB + EXT_ARCHIVOS_ZERO;
    std::string dirExe = VMCap::getNombreDirActual();

	// Crear el almacenamiento persistente
	this->ps = ps = (PersistentStoreEnDisco *) PersistentStore::ps();

	// Crear el container raiz
	raizCnt    = new(std::nothrow) Container(NULL, OBJ_ROOT, OBJ_CONTAINER);
	refRaizCnt = new(std::nothrow) RefParadojaRaiz( raizCnt );

	if ( raizCnt    == NULL
	  || refRaizCnt == NULL ) {
		throw ENoHayMemoria( "FATAL Creando el runtime" );
	}

	raizCnt->asignarA( raizCnt );

	try {
		ps->cargarContenedorDeFichero(
			(Container &) *raizCnt,
			ps->getDirPS()
		);
	} catch(const EMedioNoEncontrado &e)
	{
		// No existe, pero si es pq el PS se ha creado, no pasa nada
        raizCnt->ponModificadoDesdeCarga();
	}

	// Crear contenedor temporal ("ejecucionCnt")
	ejecucionCnt = new(std::nothrow) TransientContainer( raizCnt,
						OBJ_EXE,
						OBJ_CONTAINER )
	;

	if ( ejecucionCnt == NULL ) {
	    delete raizCnt;
	    raizCnt = NULL;
		throw ENoHayMemoria( "FATAL Creando el runtime" );
	}

	if ( raizCnt->inserta( ejecucionCnt ) == NULL ) {
	    delete raizCnt;
	    raizCnt = NULL;
		throw ENoHayMemoria( "FATAL Creando runtime: metiendo Exe en ." );
	}

    // Crear contenedor de librería estándar interna ("intStdLib")
    intStdLibCnt = new(std::nothrow) Container(
                                    (Container *) raizCnt,
                                    OBJ_INTSTDLIB,
                                    OBJ_CONTAINER )
    ;

    if ( intStdLibCnt == NULL ) {
        delete raizCnt;
	    raizCnt = NULL;
        throw ENoHayMemoria( std::string( "creando " + lib ).c_str() );
    }

	try {
		intStdLibCnt = ps->cargaContenedor(
			OBJ_INTSTDLIB,
			(Container *) raizCnt,
                        (Container *) intStdLibCnt,
                        ES_LIB
		);
	} catch (const EMedioNoEncontrado &e){
		// Tratar de cargarlo del directorio inicial
		Directorio dir( dirAct );

		if ( dir.existeArchivo( lib ) ) {
			// Cargarlo desde el sitio original
			ps->cargarContenedorDeFichero(
				(Container &) *intStdLibCnt,
				dir,
				ES_LIB
			);

			intStdLibCnt->ponModificadoDesdeCarga();

			// Insertar la librería en el raíz
			if ( raizCnt->inserta( intStdLibCnt ) == NULL ) {
                    delete raizCnt;
                    raizCnt = NULL;
                    delete intStdLibCnt;
                    intStdLibCnt = NULL;
					throw ENoHayMemoria(
							"FATAL Creando runtime: metiendo "
							"Lib en Root"
					);
			}

		} else {
		    delete raizCnt;
            raizCnt = NULL;
            delete intStdLibCnt;
            intStdLibCnt = NULL;
		    throw ELibNoEnPS( lib.c_str() );
		}
	}

    VMCap::cambiarDirectorio( dirExe );
    return;
}

// --------------------------------------------- Runtime::iniciarRuntimeSinPS()
inline
void Runtime::iniciarRuntimeSinPS()
/*
 * Este método privado es llamado sólo desde el constructor.
 * Cualquier otra invocación es un error.
 */
{
	// Crear contenedor raiz
	raizCnt    = new(std::nothrow) TransientContainer( NULL, OBJ_ROOT, OBJ_CONTAINER );
	refRaizCnt = new(std::nothrow) RefParadojaRaiz( raizCnt );

	if ( raizCnt    == NULL
	  || refRaizCnt == NULL ) {
		throw ENoHayMemoria( "FATAL Creando el runtime" );
	}

	raizCnt->asignarA( raizCnt );

	// Crear contenedor temporal ("ejecucionCnt")
	ejecucionCnt = new(std::nothrow) TransientContainer( raizCnt,
						OBJ_EXE,
						OBJ_CONTAINER )
	;

	if ( ejecucionCnt == NULL ) {
		throw ENoHayMemoria( "FATAL Creando el runtime" );
	}

	if ( !( raizCnt->inserta( ejecucionCnt ) ) ) {
		throw
                    ENoHayMemoria( "FATAL Creando runtime: metiendo Exe en Root" )
                ;
	}

	// Crear contenedor de librería estándar interna ("intStdLib")
	intStdLibCnt = new(std::nothrow) TransientContainer( raizCnt,
						OBJ_INTSTDLIB,
						OBJ_CONTAINER )
	;

	if ( intStdLibCnt == NULL ) {
		throw ENoHayMemoria( "FATAL Creando el runtime" );
	}

	if ( !( raizCnt->inserta( intStdLibCnt ) ) ) {
		throw
                    ENoHayMemoria( "FATAL Creando runtime: metiendo Lib en Root" )
                ;
	}
}
// ---------------------------------------------------------- Runtime::Runtime()
Runtime::Runtime()
{
    console          = NULL;
    compilador       = NULL;
    raizCnt          = intStdLibCnt = ejecucionCnt = NULL;
    enInicializacion = true;
    onert	         = this;

    // Asignar los atajos
    gestorExcepciones = &(this->gExcep);
    gestorStack       = &(this->gStack);
    console           = &(this->gConsola);
    conim             = Conim::getConim();


    if ( VMCap::PS )
            iniciarRuntimeConPS();
    else    iniciarRuntimeSinPS();

    // Preparar la referencia de resultado
    resultadoEjecucion = new(std::nothrow) Atributo( ejecucionCnt,
                                       OBJ_RESEXE,
                                       OBJ_NOTHING )
    ;

    // Crear el compilador
    compilador         = new(std::nothrow) Compilador();

    if ( compilador == NULL
      || resultadoEjecucion == NULL)
    {
        throw ENoHayMemoria("FATAL Creando el compilador");
    }
}

// --------------------------------------------------------- Runtime::~Runtime()
Runtime::~Runtime()
/**
	Cuidado !!
	El orden de eliminación de los contenedores importa, y mucho !!
	El último en ser eliminado debe ser IntStdLib, justo después de Exe.
*/
{
    enInicializacion = false;
    enFinalizacion   = true;

    // Eliminar gestor excepciones, resultado, stack
    delete resultadoEjecucion;

    // Guardar los contenedores persistentes en memoria
    conim->sincronizar();

    // Eliminar el gestor de vectores (contienen referencias)
    delete gestorVectores;
	gestorVectores = NULL;

    // Eliminar la raiz. Eliminacion total de contenedores.
    eliminaSubcontenedoresRaiz();
    delete refRaizCnt;
    refRaizCnt = NULL;
    raizCnt    = NULL;


    // Eliminar el compilador
    delete compilador;

	// Eliminar la gestión de contenedores en memoria
	delete conim;

    // Eliminar el resto de gestores
    delete gestorNumerosEnteros;
    delete gestorNumerosFlotantes;
    delete gestorCadenas;
    gestorNumerosFlotantes = NULL;
    gestorNumerosEnteros   = NULL;
    gestorCadenas          = NULL;
	compilador			   = NULL;
    enFinalizacion         = false;
}

void Runtime::eliminaSubcontenedoresRaiz()
/**
 * eliminaSubContenedoresRaiz()
 *
 * Este método privado toma el contenedor raiz y elimina ordenadamente
 * sus contenedores.
 *
 * La razón de ser de este método es que el Exe y el IntStdLib deben ser los
 * últimos en ser eliminados.
 *
*/
{
    int numConts = raizCnt->lAtrs.getNumero();
    AlmacenObjetos **aEliminar;
    register AlmacenObjetos * cont;
    register size_t i;
    Atributo *atr;
    size_t utilizados = 0;

    // Crear la lista de posibles contenedores a eliminar
    aEliminar = new(std::nothrow) AlmacenObjetos*[ numConts ];

    if ( aEliminar == NULL ) {
        throw ENoHayMemoria( "cerrando sistema -eliminando containers-" );
    }

    // Nos los cargamos todos menos Exe e IntStdLib
    // Buscarlos
    for(i = 0; i <  raizCnt->lAtrs.getNumero(); ++i) {
        atr = raizCnt->lAtrs.getElementoNumero( i );

        if ( dynamic_cast<AtributoEspecial *>( atr ) == NULL ) {
            cont = dynamic_cast<AlmacenObjetos *>( atr->getObjeto() );

            if ( cont != NULL
              && cont != ejecucionCnt
              && cont != intStdLibCnt )
            {
                    aEliminar[ utilizados++ ] = cont;
            }
        }
    }

    // Eliminarlos
    for(i = 0; i < utilizados; ++i) {
        raizCnt->elimina( aEliminar[ i ] );
    }

	delete[] aEliminar;

    // Eliminar Exe
    raizCnt->elimina( getContainerEjecucion() );

    // Eliminar la librería estándar
    raizCnt->elimina( getContainerIntStdLib() );
}

// -------------------------------------------------- Runtime::ejecutaMetsInic()
void Runtime::ejecutaMetsInic()
{
        Objeto * objetoActual;
        ListaMetsInic::const_iterator it = lMetsInic.begin();

        for(; it != lMetsInic.end(); ++it) {
          objetoActual = ( *it )->getPerteneceA();

          // Asegurar que no será borrado
          objetoActual->incrementaReferencias();

          // Ejecutar el método para asignar atr's.
          ( *it )->ejecuta( &Metodo::argumentosNulos, objetoActual );
          ( *it )->finEjecucion();

          // Borrarlo y dejarlo como estaba
          objetoActual->lMets.borra( *it );
          objetoActual->decrementaReferencias( Objeto::NOREF_NOBORRAR );
        }

        lMetsInic.clear();
}

// ---------------------------------------------------------- Runtime::prepara()
Metodo * Runtime::prepara(
		CargadorObjetos::ListaObjetos *l,
		bool esLib,
		MensajeArranque *m,
		AlmacenObjetos * dest)
/**
	Pasa una serie de objetos situados en un cargador de objetos
	a un contenedor previamente creado.
	En el caso particular de la librería estándar, se realiza el bootstrapping.
*/
{
        AlmacenObjetos *destino;
        Metodo * toret = NULL;

        try {
            // A qué contenedor va ?
            if ( esLib ) {
                destino = getContainerIntStdLib();
                enInicializacion = true;
	    	}
            else {
                if ( dest == NULL )
                        destino = getContainerEjecucion();
                else	destino = dest;
            }

            // Recorrer los objetos y "pasarlos" al contenedor de destino
            CargadorObjetos::ListaObjetos::iterator it = l->begin();
            for(; it != l->end(); ++it)
            {
                    if ( !( destino->inserta( it->second ) ) ) {
                            throw ENoHayMemoria(
                                "FATAL Pasando objetos al contenedor."
                            );
                    }
            }

            // Preparar el objeto TopLevel
            if ( m != NULL ) {
                toret = preparaObjetoTopLevel( m );
            }

            // Hacer el bootstrapping, si procede
            if ( esLib )
            {
                    // Crear los objetos de servicio de la máquina virtual
                    bootstrap();

                    enInicializacion = false;
            }

            // Ejecutar las inicializaciones pendientes
            if ( !enInicializacion ) {
                ejecutaMetsInic();
            }
        }
        catch (Excepcion &e)
        {
                throw;
        }
        catch (...)
        {
                throw EBootstrap("FATAL error indefinido haciendo bootstrapping");
        }

        return toret;
}

// --------------------------------------------------------- Runtime::ejecutar()
Objeto * Runtime::ejecutar(Metodo *met)
{
        Objeto * tl = NULL;

        try {
            if ( met == NULL ) {
                tl = getContainerEjecucion()->busca( OBJ_TOPLEVEL );

                if ( tl != NULL )
                {
                    met = tl->lMets.busca( MET_HAZLO );
                } else throw EEjecucion("FATAL Falta objeto TopLevel");
            }
            else tl = met->getPerteneceA();

            if (met != NULL) {
                gestorStack->eliminaTodos();

                resultadoEjecucion->ponReferencia(
                    met->ejecuta( &Metodo::argumentosNulos, tl )
                );

                met->finEjecucion();
            } else throw EEjecucion("FATAL Falta TopLevel.doIt()");
        }
        catch (Excepcion &e)
        {
                throw;
        }
        catch (...)
        {
                throw EEjecucion("FATAL Error crítico en ejecución");
        }

        return resultadoEjecucion->getObjeto();
}

// ----------------------------------------------- Runtime::preparaArgumentos()

/**
	Las siguientes funciones se emplean para hacer el bootstrap.
	Sólo para eso.
*/

inline
void Runtime::liberaArgumentos(MixinConArgumentos::Argumentos &args)
{
	if ( args.size() > 0 )
	{
		MixinConArgumentos::Argumentos::iterator it = args.begin();

		while( it != args.end() ) {
			delete *it;

			++it;
		}

		args.clear();
	}
}

inline
void Runtime::preparaArgumentos(
		MixinConArgumentos::Argumentos &args,
		const std::string &arg1)
{
	liberaArgumentos( args );

	NombreIdentificador *r1 = new(std::nothrow) NombreIdentificador( arg1 );

	if ( r1 == NULL ) {
		throw ENoHayMemoria( "creando argumentos de bootstrap" );
	}

        args.push_back( r1 );
}

inline
void Runtime::preparaArgumentos(MixinConArgumentos::Argumentos &args,
                                const std::string &arg1,
                                const std::string &arg2)
{
	liberaArgumentos( args );

        NombreIdentificador *r1 = new(std::nothrow) NombreIdentificador( arg1 );
        NombreIdentificador *r2 = new(std::nothrow) NombreIdentificador( arg2 );

	if ( r1 == NULL
	  || r2 == NULL )
	{
		throw ENoHayMemoria( "creando argumentos de bootstrap" );
	}

        args.push_back( r1 );
        args.push_back( r2 );
}

inline
void Runtime::preparaArgumentos(
		MixinConArgumentos::Argumentos &args,
                const std::string &arg1,
                const std::string &arg2,
                const std::string &arg3)
{
	liberaArgumentos( args );

	NombreIdentificador *r1 = new(std::nothrow) NombreIdentificador( arg1 );
        NombreIdentificador *r2 = new(std::nothrow) NombreIdentificador( arg2 );
        NombreIdentificador *r3 = new(std::nothrow) NombreIdentificador( arg3 );

	if ( r1 == NULL
	  || r2 == NULL
	  || r3 == NULL )
	{
		throw ENoHayMemoria( "creando argumentos de bootstrap" );
	}

        args.push_back( r1 );
        args.push_back( r2 );
        args.push_back( r3 );
}

// -------------------------------------------------------- Runtime::bootstrap()
void Runtime::bootstrap()
/**
        Crea, en el contenedor destinado a la librería estándar, los objetos de
        interfaz con la máquina virtual.
        Estos objetos son los que dan acceso a la consola, a los tipos de datos
        básicos ...
*/
{
        MixinConArgumentos::Argumentos args;
        AlmacenObjetos *std = rt()->getContainerIntStdLib();
        Objeto    *obj;
        MetodoMV  *metMV;
        Atributo  *atr;

	Objeto *objetoGestorCadenas;
	Objeto *objetoGestorFlotantes;
	Objeto *objetoGestorEnteros;
	Objeto *objetoGestorVectores;
	Objeto *objetoGestorStack;
	Objeto *objetoGestorConsola;
        Objeto *objetoGestorPS;

        try {
            // Iniciar los atajos a los objetos más usados
            objetoNulo      = std->busca( OBJ_NOTHING );
            objetoRaiz      = std->busca( OBJ_OBJECT );
            objetoSistema   = std->busca( OBJ_SYSTEM );

	    // Comprobar que existen
	    if ( objetoNulo    == NULL
	     ||  objetoRaiz    == NULL
	     ||  objetoSistema == NULL )
	    {
	    	throw ELibIlegal( EXC_SYSOBJNOENCONTRADOS.c_str() );
	    }

	    // Comprobar que tienen los atributos
	    {
	    	Atributo * sysExcepcionInterna =
			objetoSistema->lAtrs.busca(ATR_SYSEXCEPCIONINTERNA);
		Atributo * sysExcepcionMismatch =
			objetoSistema->lAtrs.busca( ATR_SYSEXCEPIONMISMATCH );
		Atributo * sysExcepcionMates =
			objetoSistema->lAtrs.busca( ATR_SYSEXCEPIONMATES );
		Atributo * sysExcepcionObj     =
			objetoSistema->lAtrs.busca(ATR_SYSEOBJ);
		Atributo * sysExcepcionMth     =
			objetoSistema->lAtrs.busca(ATR_SYSEMTH);
		Atributo * sysCadenaInterna =
			objetoSistema->lAtrs.busca(ATR_SYSCADENAINTERNA);
		Atributo * sysRealInterno =
			objetoSistema->lAtrs.busca(ATR_SYSREALINTERNO);
		Atributo * sysIntInterno =
			objetoSistema->lAtrs.busca(ATR_SYSINTINTERNO);
		Atributo * sysVectorInterno =
			objetoSistema->lAtrs.busca(ATR_SYSVECTINTERNO);
		Atributo * sysTrueInterno =
			objetoSistema->lAtrs.busca(ATR_SYSTRUEINTERNO);
		Atributo * sysFalseInterno =
			objetoSistema->lAtrs.busca(ATR_SYSFALSEINTERNO);
		Atributo * sysFecha   =
			objetoSistema->lAtrs.busca(ATR_SYSFECHA);
		Atributo * sysExcepcionPrv =
			objetoSistema->lAtrs.busca(ATR_SYSEPRV);
		Atributo * sysInfoMth =
			objetoSistema->lAtrs.busca( ATR_SYSINFOMTH );
		Atributo * sysInfoAtr =
			objetoSistema->lAtrs.busca( ATR_SYSINFOATR );
		Atributo * sysProperties = objetoSistema->lAtrs.busca( ATR_SYSPRP );

		if ( sysExcepcionInterna == NULL
		  || sysExcepcionMismatch == NULL
		  || sysExcepcionMates    == NULL
		  || sysExcepcionObj     == NULL
		  || sysExcepcionMth     == NULL
		  || sysExcepcionPrv     == NULL
		  || sysCadenaInterna    == NULL
		  || sysRealInterno      == NULL
		  || sysIntInterno       == NULL
		  || sysVectorInterno    == NULL
		  || sysTrueInterno      == NULL
		  || sysFalseInterno     == NULL
		  || sysFecha            == NULL
		  || sysInfoMth          == NULL
	      || sysInfoAtr          == NULL
		  || sysProperties 	     == NULL )
		{
			throw ELibIlegal( EXC_FALTANATRS.c_str() );
		}

		// Asignar finalmente los objetos
		objetoExcepcion        = sysExcepcionInterna->getObjeto();
		objetoEObjNoEncontrado = sysExcepcionObj->getObjeto();
		objetoEMetNoEncontrado = sysExcepcionMth->getObjeto();
		objetoEPrivado         = sysExcepcionPrv->getObjeto();
		objetoEMismatch        = sysExcepcionMismatch->getObjeto();
		objetoEMath            = sysExcepcionMates->getObjeto();
		objetoCadena           = sysCadenaInterna->getObjeto();
		objetoFlotante         = sysRealInterno->getObjeto();
		objetoEntero           = sysIntInterno->getObjeto();
		objetoVector           = sysVectorInterno->getObjeto();
		objetoTrue             = sysTrueInterno->getObjeto();
		objetoFalse            = sysFalseInterno->getObjeto();
		objetoFecha            = sysFecha->getObjeto();
		objetoInfoAtr          = sysInfoAtr->getObjeto();
		objetoInfoMth          = sysInfoMth->getObjeto();
		objetoPropiedades      = sysProperties->getObjeto();
        objetoContainer        = intStdLibCnt->lAtrs.busca(
                        OBJ_CONTAINER )->getObjeto()
                ;
	    }

            // Crear el gestor de cadenas
            gestorCadenas  = new Zero_String_;

            // Crear el acceso a las cadenas desde el programa del usuario
            obj   = objetoGestorCadenas
                  = new ObjetoSistema(std, OBJ_STRING, objetoRaiz)
            ;

            metMV = new MetodoMV("zero", obj, zeroString,
                                 MixinConArgumentos::argumentosNulos
            );
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("length", obj, lengthOfString, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("toFloat", obj, toFloatString, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("toInt", obj, toIntString, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_N);
            metMV = new MetodoMV("getPosition", obj, getPositionString, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_N1, ARG_N2);
            metMV = new MetodoMV("substring", obj, substringString, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("isEqualTo", obj, isEqualToString, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("isLessThan", obj, isLessThanString, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("isGreaterThan", obj, isGreaterThanString, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("assign", obj, assignString, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("concat", obj, concatString, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X, ARG_Y);
            metMV = new MetodoMV("seqFind", obj, seqFindInString, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X, ARG_Y);
            metMV = new MetodoMV("seqFindLast", obj, seqFindLastInString, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            std->inserta( obj );

            // Crear el gestor de números reales
            gestorNumerosFlotantes = new Zero_Float_;

            // Crear el acceso a los reales desde el programa del usuario
            obj   = objetoGestorFlotantes
                  = new ObjetoSistema(std, OBJ_FLOAT, objetoRaiz)
            ;

            metMV = new MetodoMV("zero", obj, zeroFloat,
                                              MixinConArgumentos::argumentosNulos);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("toString", obj, toString, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("toInt", obj, toIntFloat, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("abs", obj, abs, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("isNegative", obj, isNegative, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("assign", obj, assign, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("isLessThan", obj, isLessThan, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("isGreaterThan", obj, isGreaterThan, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("isEqualTo", obj, isEqualTo, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("multiplyBy", obj, multiplyBy, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("divideBy", obj, divideBy, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("sum", obj, sum, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("sub", obj, sub, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            std->inserta( obj );

            // Crear el gestor de números enteros
            gestorNumerosEnteros = new Zero_Int_;

            // Crear el acceso a los enteros desde el programa del usuario
            obj   = objetoGestorEnteros
                  = new ObjetoSistema(std, OBJ_INT, objetoRaiz)
            ;

            metMV = new MetodoMV("zero", obj, zeroInt,
                                              MixinConArgumentos::argumentosNulos);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("toString", obj, toString, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("toFloat", obj, toFloatInt, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("abs", obj, abs, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("isNegative", obj, isNegative, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("assign", obj, assign, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("isLessThan", obj, isLessThan, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("isGreaterThan", obj, isGreaterThan, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("isEqualTo", obj, isEqualTo, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("multiplyBy", obj, multiplyBy, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("divideBy", obj, divideBy, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("mod", obj, modInt, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("sum", obj, sum, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("sub", obj, sub, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            std->inserta( obj );

            // Crear el gestor de vectores
            gestorVectores   = new Zero_Vector_;

            // Crear el acceso a los vectores desde el programa del usuario
            obj   = objetoGestorVectores
                  = new ObjetoSistema(std, OBJ_VECTOR, objetoRaiz)
            ;

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("size", obj, sizeVector, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("toString", obj, toStringVector, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("clear", obj, clearVector, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_ARG1);
            metMV = new MetodoMV("add", obj, addObjectVector, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_N);
            metMV = new MetodoMV("erase", obj, erasePositionVector, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_N);
            metMV = new MetodoMV("getPosition", obj, getPositionVector, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_N, ARG_X);
            metMV = new MetodoMV("putPosition", obj, putPositionVector, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_N, ARG_X);
            metMV = new MetodoMV("insert", obj, insertObjectVector, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("isEqualTo", obj, isEqualToVector, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X, ARG_Y);
            metMV = new MetodoMV("seqFind", obj, seqFindInVector, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X, ARG_Y);
            metMV = new MetodoMV("seqFindLast", obj, seqFindLastInVector, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("process", obj, processVector, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            std->inserta( obj );

            // Crear el acceso a la consola desde el programa del usuario
            preparaArgumentos(args, ARG_ARG);
            obj   = objetoGestorConsola
                  = new ObjetoSistema(std, OBJ_CONSOLE, objetoRaiz)
            ;

            metMV = new MetodoMV("write", obj, writeToDefaultConsole, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            metMV = new MetodoMV("read", obj, readFromDefaultConsole,
                                              MixinConArgumentos::argumentosNulos);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            metMV = new MetodoMV("lf", obj, lfDefaultConsole,
                                              MixinConArgumentos::argumentosNulos);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            metMV = new MetodoMV("open", obj, openDefaultConsole,
                                              MixinConArgumentos::argumentosNulos);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            metMV = new MetodoMV("close", obj, closeDefaultConsole,
                                              MixinConArgumentos::argumentosNulos);
            obj->lMets.inserta(metMV->getNombre(), metMV);


            std->inserta( obj );

            // Crear el acceso al stack del sistema
            obj   = objetoGestorStack
                  = new ObjetoSistema(std, OBJ_STACK, objetoRaiz)
            ;

            metMV = new MetodoMV("toString", obj, toStringStack,
                                              MixinConArgumentos::argumentosNulos);
            obj->lMets.inserta(metMV->getNombre(), metMV);
            std->inserta( obj );

            // Crear el acceso al PS
            obj   = objetoGestorPS
                        = new ObjetoSistema( std, OBJ_PS, objetoRaiz )
            ;

            metMV = new MetodoMV( "getPersistentRoot", obj, getPersistentRoot,
                                 MixinConArgumentos::argumentosNulos );
            obj->lMets.inserta( metMV->getNombre(), metMV );

            metMV = new MetodoMV( "isPSPresent", obj, isPSPresent,
                                  MixinConArgumentos::argumentosNulos );
            obj->lMets.inserta( metMV->getNombre(), metMV );

            // Crear el objeto "VM",
            // que tiene acceso a todos los objetos de la MV
            obj = new ObjetoSistema(std, OBJ_VM, objetoRaiz);

            atr = new Atributo( obj, "Console", objetoGestorConsola );
            obj->lAtrs.inserta( atr->getNombre(), atr );

            atr = new Atributo(obj, "Stack", objetoGestorStack );
            obj->lAtrs.inserta( atr->getNombre(), atr );

            atr = new Atributo(obj, "String", objetoGestorCadenas );
            obj->lAtrs.inserta( atr->getNombre(), atr );

            atr = new Atributo( obj, "Float", objetoGestorFlotantes );
            obj->lAtrs.inserta( atr->getNombre(), atr );

            atr = new Atributo( obj, "Int", objetoGestorEnteros );
            obj->lAtrs.inserta( atr->getNombre(), atr );

            atr = new Atributo( obj, "Vector", objetoGestorVectores );
            obj->lAtrs.inserta( atr->getNombre(), atr );

            atr = new Atributo( obj, "PS", objetoGestorPS );
            obj->lAtrs.inserta( atr->getNombre(), atr );

            metMV = new MetodoMV("getPlatformName", obj, getPlatformName,
                                              MixinConArgumentos::argumentosNulos);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            metMV = new MetodoMV("getHiPlatformCode", obj, getHiPlatformCode,
                                              MixinConArgumentos::argumentosNulos);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            metMV = new MetodoMV("getLoPlatformCode", obj, getLoPlatformCode,
                                              MixinConArgumentos::argumentosNulos);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            metMV = new MetodoMV("getCurrentTime", obj, getCurrentTime,
                                              MixinConArgumentos::argumentosNulos);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            metMV = new MetodoMV("getLoVersionNumber", obj, getLoVersionNumber,
                                              MixinConArgumentos::argumentosNulos);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            metMV = new MetodoMV("getHiVersionNumber", obj, getHiVersionNumber,
                                              MixinConArgumentos::argumentosNulos);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X);
            metMV = new MetodoMV("copyObject", obj, copyObject, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("getNameOf", obj, getNameOf, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X);
            metMV = new MetodoMV("createChildOf", obj, createChildOf, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_ARG2);
            metMV = new MetodoMV("compareReferences", obj, compareReferences,
                                                                          args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG1, ARG_X, ARG_ARG2);
            metMV = new MetodoMV("callMethodNumber", obj, callMethodNumberOfObj,
                                                                          args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X);
            metMV = new MetodoMV("getMethodNumberByNameYOfX", obj,
                                               getMethodNumberByNameYOfX, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X);
            metMV = new MetodoMV("getMethodPropertiesYOfX", obj,
                                               getMethodPropertiesYOfX, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X);
            metMV = new MetodoMV("getAttributeNumberYOfX", obj,
                                                   getAttributeNumberYOfX, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("getNumberOfMethodsOf", obj,
                                                 getNumberOfMethodsOf, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("getObjectPropertiesOf", obj,
                                                 getObjectPropertiesOf, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("getNumberOfAttributesOf", obj,
                                                 getNumberOfAttributesOf, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG);
            metMV = new MetodoMV("getNumberOfParentAttributeOf", obj,
                                                 getNumberOfParentAttributeOf, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X);
            metMV = new MetodoMV("getNameOfAttributeNumberYOfX", obj,
                                            getNameOfAttributeNumberYOfX, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X);
            metMV = new MetodoMV("getNameOfMethodNumberYOfX", obj,
                                            getNameOfMethodNumberYOfX, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X);
            metMV = new MetodoMV("getOpcodesOfMethodNumberYOfX", obj,
                                            getOpcodesOfMethodNumberYOfX, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X);
            metMV = new MetodoMV("getAttributeNumberByNameYOfX", obj,
                                            getAttributeNumberByNameYOfX, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X, ARG_Y);
            metMV = new MetodoMV("addAttributeYOfXValueZ", obj,
                                                  addAttributeYOfXValueZ, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X);
            metMV = new MetodoMV("deleteAttributeNumberYOfX", obj,
                                               deleteAttributeNumberYOfX, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_N, ARG_X);
            metMV = new MetodoMV("compileToMethodYOfX", obj,
                                               compileToMethodYOfX, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X, ARG_Y);
            metMV = new MetodoMV("addMethodYInX", obj,
                                                  addMethodYInX, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X);
            metMV = new MetodoMV("deleteMethodNumberYOfX", obj,
                                               deleteMethodNumberYOfX, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X);
            metMV = new MetodoMV("getInfoOnAttributeNumberYOfX", obj,
                                            getInfoOnAttributeNumberYOfX, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos(args, ARG_ARG, ARG_X);
            metMV = new MetodoMV("getInfoOnMethodNumberYOfX", obj,
                                            getInfoOnMethodNumberYOfX, args);
            obj->lMets.inserta(metMV->getNombre(), metMV);

            preparaArgumentos( args, ARG_ARG );
            metMV = new MetodoMV("getOwner", obj, getOwner, args );

            obj->lMets.inserta( metMV->getNombre(), metMV );

	    // Crear el objeto VM
            std->inserta( obj );

	    // Liberar
	    liberaArgumentos( args );

        } catch (std::bad_alloc &)
        {
                throw ENoHayMemoria("FATAL sin memoria en bootstrapping");
        }
        catch(Excepcion &e)
        {
                throw;
        }
        catch(...)
        {
		liberaArgumentos( args );

                throw EBootstrap("FATAL error crítico bootstrapping");
        }
}
// ------------------------------------------------------- Runtime::ponExcepcion
void Runtime::ponExcepcion(Objeto *objExcep, const std::string &mensaje)
{
        Objeto * objetoMensaje = NULL;

        if ( !mensaje.empty() ) {
                objetoMensaje = Runtime::gestorCadenas->nuevo( "", mensaje );
        }

        onert->gExcep.ponExcepcion( objExcep, objetoMensaje );
}

void Runtime::ponExcepcionTipoCadena(const std::string &nombreObjeto)
{
        ponExcepcion(
                Runtime::objetoEMismatch,
                EXC_NOCADENA
	          		+ ':' + ' ' + '\'' + nombreObjeto + '\'' )
	;
}

void Runtime::ponExcepcionTipoVector(const std::string &nombreObjeto)
{
        ponExcepcion(
                Runtime::objetoEMismatch,
                EXC_NOVECTOR
	          		+ ':' + ' ' + '\'' + nombreObjeto + '\'' )
	;
}

void Runtime::ponExcepcionTipoNumero(const std::string &nombreObjeto)
{
        ponExcepcion(
                Runtime::objetoEMismatch,
                EXC_NONUMERO
	          		+ ':' + ' ' + '\'' + nombreObjeto + '\'' )
	;
}

void Runtime::ponExcepcionMates(const std::string &error, const std::string &obj)
{
        ponExcepcion(
                Runtime::objetoEMath,
                error
	          		+ ':' + ' ' + '\'' + obj + '\'' )
	;
}

void Runtime::ponExcepcionObjeto(const std::string &nombreObjeto)
{
        ponExcepcion(
                Runtime::objetoEObjNoEncontrado,
                EXC_OBJNOTFOUND
	          + ':' + ' ' + '\'' + nombreObjeto + '\'' )
	;
}

void Runtime::ponExcepcionMetodo(const std::string &nombreMetodo)
{
        Runtime::ponExcepcion(
                Runtime::objetoEMetNoEncontrado,
                EXC_MTHNOTFOUND
	          + ':' + ' '+ '\'' + nombreMetodo + '\'' )
	;
}

// ========================================= Funcionalidad de la Máquina Virtual
// -------------------------------------------------------- Funciones auxiliares
Objeto * Runtime::getObjetoArgumento(Metodo *met, const std::string &x)
{
        Objeto     *toret = NULL;
        Referencia *ref   = met->vblesLocales.busca( x );

        if (ref != NULL) {
                toret = ref->getObjeto();
        }
        else throw ELibIlegal( x.c_str() );

        return toret;
}

void * Runtime::getDatoArgumento(Metodo *met, Zero_Data_ *dat, const std::string &x)
{
        Objeto     *obj   = NULL;
        Referencia *ref   = met->vblesLocales.busca( x );

        if ( ref != NULL )
             obj = ref->getObjeto();
        else throw ELibIlegal( x.c_str() );

        return dat->buscarPorObjeto( obj );
}

// ------------------------------------------- Runtime::getPlatformName() et al.
void Runtime::getPlatformName(Metodo *met)
{
        met->ponRetorno(Runtime::gestorCadenas->nuevo( "",
	                                           VMCap::getPlaformName() ) );
}

void Runtime::getHiPlatformCode(Metodo *met)
{
        met->ponRetorno(Runtime::gestorNumerosEnteros->nuevo( "",
						 VMCap::getHiPlatformCode() ) );
}

void Runtime::getLoPlatformCode(Metodo *met)
{
        met->ponRetorno(Runtime::gestorNumerosEnteros->nuevo( "",
	                                         VMCap::getLoPlatformCode() ) );
}

// ----------------------------------------------- Runtime::getLoVersionNumber()
void Runtime::getLoVersionNumber(Metodo *met)
{
        met->ponRetorno(Runtime::gestorNumerosEnteros->nuevo("", lver));
}

// ----------------------------------------------- Runtime::getHiVersionNumber()
void Runtime::getHiVersionNumber(Metodo *met)
{
        met->ponRetorno(Runtime::gestorNumerosEnteros->nuevo("", hver));
}

// --------------------------------------------------- Runtime::getCurrentTime()
void Runtime::getCurrentTime(Metodo *met)
{
	Objeto * obj;
	int tiempo;
	Atributo *atr;
	Objeto *objLiteral;
	std::time_t tiempoSistema;

	try {
                obj = new(std::nothrow) Objeto(NULL, "", Runtime::objetoRaiz);

		if ( obj != NULL ) {
			std::time ( &tiempoSistema );
			tm * instante = std::localtime( &tiempoSistema );

			tiempo   = ( ( instante->tm_hour * 3600 )
				+ ( instante->tm_min * 60 )
				+ instante->tm_sec
				)
			;

			objetoFecha->copia( obj, "", Objeto::SYSTEM_COPY );

			// Asignar la hora
			objLiteral = gestorNumerosEnteros->nuevo( "", tiempo );
			if ( objLiteral != NULL ) {
				atr = new
				    Atributo( obj, "instantInDay", objLiteral )
				;
				obj->lAtrs.inserta(
				     atr->getNombre(), atr )
				;
			}

			// Asignar el año
			objLiteral =
			   gestorNumerosEnteros->nuevo( "",
			                               instante->tm_year + 1900)
			;

			if ( objLiteral != NULL ) {
				atr = new
				    Atributo( obj, "year", objLiteral )
				;
				obj->lAtrs.inserta(
				     atr->getNombre(), atr )
				;
			}

			// Asignar el mes
			objLiteral =
			   gestorNumerosEnteros->nuevo( "",
			                                instante->tm_mon + 1)
			;

			if ( objLiteral != NULL ) {
				atr = new
				    Atributo( obj, "month", objLiteral )
				;
				obj->lAtrs.inserta(
				     atr->getNombre(), atr )
				;
			}

			// Asignar el día
			objLiteral =
			   gestorNumerosEnteros->nuevo( "", instante->tm_mday )
			;

			if ( objLiteral != NULL ) {
				atr = new
				    Atributo( obj, "day", objLiteral )
				;
				obj->lAtrs.inserta(
				     atr->getNombre(), atr )
				;
			}
		}
		else {
			obj = objetoNulo;
			ponExcepcion( objetoExcepcion, EXC_NOMEM );
		}
	}
        catch(const std::bad_alloc &) {
                throw ENoHayMemoria( "FATAL sin memoria en getCurrentTime" );
        }
        catch (...) {
                throw ELibIlegal( "FATAL error en getCurrentTime" );
	}

        met->ponRetorno( obj );
}

// ------------------------------------------------------ Runtime::createChild()
void Runtime::createChildOf(Metodo *met)
{
    std::string nombreObj;
    Objeto * toret;

    try {
        AlmacenObjetos *c = Runtime::rt()->getContainerIntStdLib();
        Objeto *obj    = getObjetoArgumento(met, ARG_ARG);
        std::string *nombre = static_cast<std::string *>(getDatoArgumento(met,
                                            Runtime::gestorCadenas, ARG_X));

        if ( nombre != NULL
          && obj    != NULL )
        {
            // Comprobar el nombre
            if ( !( nombre->empty() )
              && !NombreIdentificador::compruebaId( *nombre ) )
            {
                ponExcepcion( objetoExcepcion, EXC_NOID + ':' + ( *nombre ) );
                return;
            }

            // Preparar el nombre
            if ( nombre->empty() )
                    nombreObj = ServidorDeNombres::getSigNombreUnico();
            else    nombreObj = *nombre;

            // Preparar el objeto o contenedor
            if ( obj != objetoContainer )
                    toret = new(std::nothrow) Objeto( NULL, nombreObj, obj );
            else    toret = AlmacenObjetos::nuevo( NULL, nombreObj );

            if ( toret != NULL ) {
                    // Crear su objeto primitivo si es necesario
                    if ( toret->esDescendienteDe( c, OBJ_PADRE_CADENAS ) ) {
                            gestorCadenas->mas( toret, "" );
                    }
                    else
                    if ( toret->esDescendienteDe( c, OBJ_PADRE_ENTEROS ) ) {
                            gestorNumerosEnteros->mas( toret, 0 );
                    }
                    else
                    if ( toret->esDescendienteDe( c, OBJ_PADRE_FLOTANTES ) ) {
                            gestorNumerosFlotantes->mas( toret, 0 );
                    }
                    else
                    if ( toret->esDescendienteDe( c, OBJ_PADRE_VECTORES ) ) {
                            LiteralVector_Zero *v =
                                new(std::nothrow) LiteralVector_Zero( toret )
                            ;

                            if ( v != NULL ) {
                                gestorVectores->mas( toret, v );
                            }
                            else ponExcepcion( objetoExcepcion, EXC_NOMEM );
                    }

                    toret->ponModificadoDesdeCarga();
                    met->ponRetorno( toret );
            } else ponExcepcion( objetoExcepcion, EXC_NOMEM );

        }  else ponExcepcionObjeto( ARG_ARG );
    }
    catch(const ENoHayMemoria &) {
        throw;
    }
    catch(Excepcion &e)
    {
            throw ELibIlegal(
                    ( std::string( "error en argumentos en createChildOf: " ) + e.getDetalles() ).c_str() );
    }
    catch(const std::bad_alloc &) {
        throw ENoHayMemoria( "createChildOf()" );
    }
    catch(...)
    {
            throw ELibIlegal("FATAL error en createChildOf");
    }
}

// -------------------------------------------------------- Runtime::getOwner()
void Runtime::getOwner(Metodo *met)
{
    try {
        Objeto *obj = getObjetoArgumento( met, ARG_ARG );

        if ( obj != NULL ) {
                AlmacenObjetos * toret = obj->getPerteneceA();

                if ( toret == NULL ) {
                    toret = Runtime::rt()->getContainerEjecucion();
                }

                met->ponRetorno( toret );
        }
        else    ponExcepcionObjeto( ARG_ARG );
    }
    catch (Excepcion &e)
    {
        throw ELibIlegal(
                  ( std::string( "error en argumentos en getOwner: " )
                + e.getDetalles() ).c_str() )
        ;
    }
    catch (...)
    {
        throw ELibIlegal("FATAL error en getOwner");
    }
}

// ----------------------------------------------- Runtime::getPersistentRoot()
void Runtime::getPersistentRoot(Metodo *met)
{
    try {
        met->ponRetorno( Runtime::rt()->getContainerRaiz() );
    }
    catch (Excepcion &e)
    {
        throw ELibIlegal(
                ( std::string( "error en argumentos en getPersistentRoot: " )
                + e.getDetalles() ).c_str()
        );
    }
    catch (...)
    {
        throw ELibIlegal( "FATAL error en getPersistentRoot" );
    }
}

// ----------------------------------------------------- Runtime::isPSPresent()
void Runtime::isPSPresent(Metodo *met)
{
    try {
        met->ponRetorno(
                VMCap::PS ? Runtime::objetoTrue : Runtime::objetoFalse
        );
    }
    catch (Excepcion &e)
    {
        throw ELibIlegal(
                ( std::string( "error en argumentos en isPSPresent: " )
                + e.getDetalles() ).c_str()
        );
    }
    catch (...)
    {
        throw ELibIlegal("FATAL error en isPSPresent");
    }
}

// ------------------------------------------------------- Runtime::copyObject()
void Runtime::copyObject(Metodo *met)
{
    Objeto * toret;

    try {
        Objeto *obj    = getObjetoArgumento( met, ARG_ARG );
        std::string *nombre = static_cast<std::string *>( getDatoArgumento( met,
                                    Runtime::gestorCadenas, ARG_X )
        );

        if ( nombre != NULL
          && obj    != NULL )
        {
            // Comprobar el nombre
            if ( !( nombre->empty() )
              && !NombreIdentificador::compruebaId( *nombre ) )
            {
                ponExcepcion( objetoExcepcion, EXC_NOID + ':' + ( *nombre ) );
                return;
            }

            // ¿Es un contenedor?
            if ( obj->esDescendienteDe( objetoContainer ) ) {
                toret = AlmacenObjetos::nuevo( NULL, *nombre );

                if ( toret == NULL ) {
                    ponExcepcion( objetoExcepcion, EXC_NOMEM );
                    toret = objetoNulo;
                }

                met->ponRetorno( toret );
            }
            // ¿Es cualquier otro objeto?
            else {
                toret = new(std::nothrow) Objeto( NULL, "", obj );

                if ( toret != NULL ) {
                    obj->copia( toret, *nombre );
                    met->ponRetorno( toret );
                }
                else ponExcepcion( objetoExcepcion, EXC_NOMEM );
            }

            toret->ponModificadoDesdeCarga();  // Indicar que se guarde
        }  else ponExcepcionObjeto( ARG_ARG );
    }
    catch (Excepcion &e)
    {
            throw ELibIlegal(
                    ( std::string( "error en argumentos en copyObject: " ) +
                                                            e.getDetalles()).c_str()
            );
    }
    catch (...)
    {
            throw ELibIlegal("FATAL error en copyObject");
    }
}

// -------------------------------------------------------- Runtime::getNameOf()
void Runtime::getNameOf(Metodo *met)
{

        Referencia * ref = met->vblesLocales.busca(ARG_ARG);

        if (ref != NULL)
        {
                Objeto * toret = Runtime::gestorCadenas->nuevo("",
                                                 ref->getObjeto()->getNombre());

                met->ponRetorno(toret);
        }
        else {
                met->ponRetorno(Runtime::objetoNulo);
                ponExcepcionObjeto(ARG_ARG);
        }
}

// ------------------------------------------------ Runtime::compareReferences()
void Runtime::compareReferences(Metodo *met)
{
        try {
            Objeto *objeto1 = getObjetoArgumento(met, ARG_ARG1);
            Objeto *objeto2 = getObjetoArgumento(met, ARG_ARG2);

            if (objeto1 == objeto2)
            {
                    met->ponRetorno(Runtime::objetoTrue);
            }  else met->ponRetorno(Runtime::objetoFalse);
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string( "error en argumentos en compareReferences: " )
                        + e.getDetalles()).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en compareReferences");
        }
}

// ------------------------------------ Runtime::getNumberOfParentAttributeOf()
void Runtime::getNumberOfParentAttributeOf(Metodo * met)
{
        try {
            Objeto *obj          = getObjetoArgumento(met, ARG_ARG);

            if (obj != NULL)
            {
                met->ponRetorno(gestorNumerosEnteros->nuevo("",
			obj->lAtrs.buscaIndicePorNombre(ATR_PARENT)))
		;
            }
            else Runtime::ponExcepcionObjeto(ARG_ARG);
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string( "error en argumentos en getNumberOfAttributesOf: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en getNumberOfParentAttributeOf");
        }
}

// ------------------------------------------ Runtime::getNumberOfAttributesOf()
void Runtime::getNumberOfAttributesOf(Metodo *met)
{
        try {
            Objeto *obj          = getObjetoArgumento(met, ARG_ARG);

            if (obj != NULL)
            {
                met->ponRetorno(
				gestorNumerosEnteros->nuevo(
					"",
					obj->lAtrs.getNumero()
				)
		);
            }
            else Runtime::ponExcepcionObjeto(ARG_ARG);
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string( "error en argumentos en getNumberOfAttributesOf: " )
                                + e.getDetalles() ).c_str()
                         );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en getNumberOfAttributesOf");
        }
}

// -------------------------------------------- Runtime::getObjectPropertiesOf()
void Runtime::getObjectPropertiesOf(Metodo *met)
{
        try {
            Objeto *obj          = getObjetoArgumento(met, ARG_ARG);

            if (obj != NULL)
            {
                // Modificar para que devuelva el objeto (vector) de propiedades
		met->ponRetorno( obj->getProps() );
            }
            else Runtime::ponExcepcionObjeto( ARG_ARG );
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string( "error en argumentos en getNumberOfAttributesOf: " )
                                + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en getObjectPropertiesOf");
        }
}

// ------------------------------------------ Runtime::getMethodPropertiesYOfX()
void Runtime::getMethodPropertiesYOfX(Metodo *met)
{
        try {
            Objeto *obj          = getObjetoArgumento(met, ARG_ARG);
            INT32  *x            = static_cast<INT32 *>(getDatoArgumento
                                      (met, Runtime::gestorNumerosEnteros, ARG_X));

            if (obj != NULL
             && x   != NULL)
            {
                // Buscar el método
                Metodo * metObjetivo = obj->lMets.getElementoNumero( *x );

                // Cambiar y devolver las propiedades del método
		met->ponRetorno( metObjetivo->getProps() );
            }
            else Runtime::ponExcepcionObjeto(ARG_ARG);
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string( "error en argumentos en getMethodPropertiesYOfX: " )
                        + e.getDetalles()).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal( "FATAL error en getMethodPropertiesYOfX" );
        }
}

// --------------------------------------------- Runtime::getNumberOfMethodsOf()
void Runtime::getNumberOfMethodsOf(Metodo *met)
{
        try {
            Objeto *obj          = getObjetoArgumento(met, ARG_ARG);

            if (obj != NULL)
            {
                met->ponRetorno(gestorNumerosEnteros->nuevo("", obj->lMets.getNumero()));
            }
            else Runtime::ponExcepcionObjeto(ARG_ARG);
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string( "error en argumentos en getNumberOfAttributesOf: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en getNumberOfAttributesOf");
        }
}

// ------------------------------------------- Runtime::deleteMethodNumberYOfX()
void Runtime::deleteMethodNumberYOfX(Metodo *met)
{
        try {
            Objeto *obj          = getObjetoArgumento(met, ARG_ARG);
            INT32  *x            = static_cast<INT32 *>(getDatoArgumento
                                      (met, Runtime::gestorNumerosEnteros, ARG_X));

            if (obj != NULL
             && x   != NULL)
            {
                Metodo * metObj = obj->lMets.getElementoNumero((size_t) *x);
                if (metObj != NULL)
                {
			if (obj->lMets.borra( metObj/*->getNombre()*/ ) )
                    {
                            met->ponRetorno(Runtime::objetoTrue);
                    }
                    else Runtime::ponExcepcion(Runtime::objetoExcepcion, EXC_NOMETODO);
                } else Runtime::ponExcepcionMetodo(EXC_NOMETODO);
            }
            else Runtime::ponExcepcionObjeto(ARG_ARG);
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                       ( std::string("error en argumentos en deleteMethodNumberYOfX: " )
                        + e.getDetalles()).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en deleteMethodNumberYOfX");
        }
}

// ---------------------------------------- Runtime::getMethodNumberByNameYOfX()
void Runtime::getMethodNumberByNameYOfX(Metodo *met)
{
        try {
            Objeto *obj          = getObjetoArgumento(met, ARG_ARG);
            std::string *str     = static_cast<std::string *>(getDatoArgumento
                                      (met, Runtime::gestorCadenas, ARG_X));

            if ( obj != NULL
             &&  str != NULL )
            {
                size_t pos = obj->lMets.buscaIndicePorNombre(*str);

                if ( pos < obj->lMets.getNumero() ) {
                    met->ponRetorno( Runtime::gestorNumerosEnteros->nuevo( "", pos ) );
                }
                else Runtime::ponExcepcion( objetoExcepcion, EXC_FUERADERANGO );
            }
            else Runtime::ponExcepcionObjeto(ARG_ARG);
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                       ( std::string("error en argumentos en getMethodNumberByNameYOfX: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en getMethodNumberByNameYOfX");
        }
}

// -------------------------------------------- Runtime::callMethodNumberOfObj()
void Runtime::callMethodNumberOfObj(Metodo *met)
{
        try {
            Objeto *obj          = getObjetoArgumento(met, ARG_ARG1);
            INT32  *numeroMetodo = static_cast<INT32 *>(getDatoArgumento
                                      (met, Runtime::gestorNumerosEnteros, ARG_X));
	    LiteralVector_Zero * v  = static_cast<LiteralVector_Zero *>(
                       getDatoArgumento(met, Runtime::gestorVectores, ARG_ARG2))
            ;

            if ( numeroMetodo != NULL
              && obj          != NULL )
            {
	    	Objeto * toret   = Runtime::objetoNulo;
                Metodo * metDest = obj->lMets.getElementoNumero(*numeroMetodo);

		metDest = Metodo::encuentraMetodo( metDest );  // Quizás recurs.
		if (metDest != NULL)
		{
			// Ejecutar
			if ( v != NULL ) {
			  toret = metDest->ejecuta(
				(Metodo::Argumentos *)
				                   v->getVectorImplementacion(),
				obj )
			  ;
			}
			else {
			 toret = metDest->ejecuta(&Metodo::argumentosNulos, obj);
			}

			// El retonno
			met->ponRetorno( toret );

			// Indicar fin de ejecución
			metDest->finEjecucion();
		}
		else ponExcepcion( objetoExcepcion, EXC_FUERADERANGO );
            }
            else Runtime::ponExcepcionObjeto(ARG_ARG1 + ',' + ARG_X + ',' + ARG_ARG2);
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en callMethodNumberOfObj: "  )
                         + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en callMethodNumberOfObj");
        }
}

// ------------------------------------------- Runtime::addAttributeYOfXValueZ()
void Runtime::addAttributeYOfXValueZ(Metodo *met)
{
        try {
            Objeto *obj = getObjetoArgumento(met, ARG_ARG);
            std::string *nombreAtributo = static_cast<std::string *>(
	    		getDatoArgumento( met, Runtime::gestorCadenas, ARG_X ) );
            Objeto *ref = getObjetoArgumento(met, ARG_Y);


            if (nombreAtributo != NULL
             && ref            != NULL
             && obj            != NULL)
            {
               Atributo *atr = new(std::nothrow) Atributo( obj, *nombreAtributo, ref );

	       if ( atr != NULL )
	       {
			if (obj->lAtrs.inserta(*nombreAtributo, atr))
				met->ponRetorno(obj);
			else {
				delete atr;
				ponExcepcion(Runtime::objetoExcepcion, EXC_YAEXISTE);
			}
	       } else ponExcepcion( Runtime::objetoExcepcion, EXC_NOMEM );
            }
            else Runtime::ponExcepcionObjeto(ARG_ARG);
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en addAtributeYToXValueZ: " )
                         + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en addAtributeYToXValueZ");
        }
}

// ---------------------------------------- Runtime::deleteAttributeNumberYOfX()
void Runtime::deleteAttributeNumberYOfX(Metodo *met)
{
        try {
            Objeto *obj = getObjetoArgumento(met, ARG_ARG);
            INT32  *numeroAtributo = static_cast<INT32 *>(getDatoArgumento
                                      (met, Runtime::gestorNumerosEnteros, ARG_X));

            if (numeroAtributo != NULL
             && obj            != NULL)
            {
               Atributo *atr = obj->lAtrs.getElementoNumero(
                                                (unsigned int) *numeroAtributo);
               if ( dynamic_cast<AtributoEspecial*>( atr ) == NULL ) {
                    if ( obj->lAtrs.borra( atr ) ) {
                                onert->deSincronizaMetodos();
                                met->ponRetorno( objetoTrue );
                    } else Runtime::ponExcepcion(objetoExcepcion, EXC_NOATR);
               }
               else met->ponRetorno( objetoFalse );
            }
            else Runtime::ponExcepcionObjeto(ARG_ARG);
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en deleteAttributeNumberYOfX: " )
                         + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en deleteAttributeNumberYOfX");
        }
}

// ---------------------------------------------------- Runtime::addMethodYInX()
void Runtime::addMethodYInX(Metodo *met)
{
        try {
            Objeto *obj = getObjetoArgumento( met, ARG_ARG );
            std::string *nombreMetodo = static_cast<std::string *>(
	    			getDatoArgumento(
					met,
					Runtime::gestorCadenas,
					ARG_X
				)
	    );
	    LiteralVector_Zero * args  = static_cast<LiteralVector_Zero *>(
                       		getDatoArgumento(
					met,
					Runtime::gestorVectores,
					ARG_Y )
	    );


            if ( nombreMetodo != NULL
              && args         != NULL
              && obj          != NULL )
            {
		// Crear los argumentos
		MixinConArgumentos::Argumentos argsFormales;
		Metodo *mth;
		Nombre *fref;

		for(register size_t i = 0; i < args->longitud(); ++i) {
			std::string * arg = gestorCadenas->busca(
					args->getElemento( i )->getObjeto()
			);

			if ( arg != NULL ) {
				fref = Nombre::creaNombreAdecuado( *arg );

				if ( fref != NULL ) {
					argsFormales.push_back( fref );
				} else {
					ponExcepcion(
						Runtime::objetoExcepcion,
						EXC_NOMEM
					);
					goto FIN;
				}
			}
			else {
				ponExcepcion(
					Runtime::objetoExcepcion,
					EXC_NOCADENA
				);
				goto FIN;
			}
		}

		// Crear el método
		mth = new(std::nothrow) Metodo( *nombreMetodo, obj, argsFormales );

		if ( mth != NULL )
		{
			// Insertar el método
			if ( obj->lMets.inserta( *nombreMetodo, mth ) ) {
				met->ponRetorno(obj);
			}
			else {
				delete mth;
				ponExcepcion( Runtime::objetoExcepcion,
						EXC_YAEXISTE )
				;
			}
		} else ponExcepcion( Runtime::objetoExcepcion, EXC_NOMEM );

		FIN:
		for( MixinConArgumentos::Argumentos::iterator it = argsFormales.begin();
		     it != argsFormales.end();
		     ++it)
		{
			delete *it;
		}

		return;
            }
            else Runtime::ponExcepcionObjeto( ARG_ARG );
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string( "error en argumentos en addMethodYInX: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en addMethodYInX");
        }
}

// ---------------------------------------- Runtime::deleteAttributeNumberYOfX()
void Runtime::compileToMethodYOfX(Metodo *met)
/**
	Acepta los argumentos ARG_ARG, ARG_N y ARG_X
	Compila una cadena ARG_X a un [número de] método ARG_N, en un
	objeto ARG_ARG
*/
{
        try {
            Objeto *obj = getObjetoArgumento(met, ARG_ARG);
            INT32  *numeroMetodo = static_cast<INT32 *>(
	    			      getDatoArgumento(
				      	met,
				       	Runtime::gestorNumerosEnteros,
					ARG_N )
            );
            std::string *cuerpoMetodo = static_cast<std::string *>(
                             getDatoArgumento(
                            met,
                        Runtime::gestorCadenas,
                        ARG_X )
            );

            if ( numeroMetodo   != NULL
              && obj            != NULL
              && cuerpoMetodo   != NULL )
            {
                // Buscar el método
                Metodo * mth = obj->lMets.getElementoNumero(
                                                  (unsigned int) *numeroMetodo )
                ;

                // Crear los identificadores de argumentos formales
                Identificadores args;
                MixinConArgumentos::Argumentos::iterator itArgs =
                                mth->argumentosFormales.begin()
                ;

                while ( itArgs != mth->argumentosFormales.end() ) {
                    args.inserta( ( *itArgs )->getNombre() );
                    ++itArgs;
                }

                // Crear los identificadores de atributos del objeto
                Identificadores atrs;

                for(register size_t i = 0; i < obj->lAtrs.getNumero(); ++i) {
                    atrs.inserta( obj->lAtrs.getNombreElementoNumero( i ) );
                }

                // Compilar
                Runtime::compilador->reset( cuerpoMetodo, &args, &atrs );
                Compilador::ListaMnemos &mnemos =
                        Runtime::compilador->compilar( Optimizador::opt2 )
                ;

                // Preparar mth, por si hay contenido algo
                mth->inicializar();

                // Meterle los nuevos opcodes
                mth->masInstrucciones( mnemos );

                // Poner el resultado
                met->ponRetorno( Runtime::gestorCadenas->nuevo(
                            "",
                            Runtime::compilador->getCadenaAvisos() )
                );
            }
            else Runtime::ponExcepcionObjeto( ARG_ARG );
        }
	catch (ECompilacion &e) {
		// Lanzar una excepción Zero
		Runtime::ponExcepcion( objetoExcepcion, e.getDetalles() );
	}
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en compileToMethodYofX: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en compileToMethodYofX");
        }
}

// ---------------------------------------- Runtime::getInfoOnMethodNumberYOfX()
void Runtime::getInfoOnMethodNumberYOfX(Metodo *met)
{
    Metodo *metObj;
	std::string ops;

        try {
                Objeto *objeto = getObjetoArgumento(met, ARG_ARG);
                INT32  *numero = static_cast<INT32 *>(
                            getDatoArgumento(met, Runtime::gestorNumerosEnteros, ARG_X));

                if (objeto != NULL
                 && numero != NULL)
                {
                    // Buscar el objeto de deseo
                    metObj = objeto->lMets.getElementoNumero(*numero);

                    if (metObj != NULL)
                    {
		    	Objeto *info = new Objeto( NULL, "", Runtime::objetoRaiz );
			Runtime::objetoInfoMth->copia( info, "", Objeto::SYSTEM_COPY );

			Referencia * atrAcceso  = info->lAtrs.busca( "access" );
			Referencia * atrNombre  = info->lAtrs.busca( "name" );
			Referencia * atrParams  = info->lAtrs.busca( "parameters" );
			Referencia * atrOrdinal = info->lAtrs.busca( "ordinal" );
			Referencia * atrOpsList = info->lAtrs.busca( "opcodeList" );
			Referencia * atrPropiet = info->lAtrs.busca( "owner" );

			// Comprobar que los campos existen
			if ( atrAcceso   == NULL
			  || atrNombre   == NULL
			  || atrParams   == NULL
			  || atrOrdinal  == NULL
			  || atrOpsList  == NULL
			  || atrPropiet  == NULL )
			{
				ponExcepcion(Runtime::objetoExcepcion, EXC_FALTANATRS);
				delete info;
				info = Runtime::objetoNulo;
			}
			else {
				// Rellenar los atributos del objeto de información
				// Objeto dueño del atributo
				atrPropiet->ponReferencia( objeto );

				// Acceso
				if (metObj->esPublico())
					atrAcceso->ponReferencia(
						Runtime::objetoTrue
					);
				else    atrAcceso->ponReferencia(
						Runtime::objetoFalse
					);

				// Nombre del método
				Objeto *mthNombre = Runtime::gestorCadenas->
						nuevo( "",
						       metObj->getNombre() )
				;
				atrNombre->ponReferencia( mthNombre );

				// Parámetros
				Objeto *vParams = gestorVectores->nuevo("");
				Objeto *str;
				if (vParams != NULL)
				{
					for(register size_t i = 0;
					    i < metObj->argumentosFormales.size();
					    ++i)
					{
						str = gestorCadenas->nuevo("",
							metObj->
							 argumentosFormales[i]->
							  getNombre()
						      )
						;

						if (str != NULL)
							gestorVectores
							 ->busca( vParams )
							  ->mas( str );
						else    gestorVectores
						          ->busca(vParams)
							    ->mas(objetoNulo);
					}

					atrParams->ponReferencia(vParams);
				}
				else atrParams->ponReferencia(objetoNulo);

				// Lista de opcodes
				ops = metObj->getListaOpcodes();

				Objeto *objLOpcodes = Runtime::gestorCadenas->
						nuevo( "", ops )
				;

				atrOpsList->ponReferencia( objLOpcodes );

				// Su número en el objeto
				Objeto *numeroMth =
					 Runtime::gestorNumerosEnteros
					 	->nuevo("",
						   objeto
						     ->lMets.buscaIndicePorNombre(
							metObj->getNombre()
					)
				);

				atrOrdinal->ponReferencia(numeroMth);
			}
                        // Fin
                        met->ponRetorno(info);
                    } else ponExcepcionMetodo(ARG_X);
                } else ponExcepcionObjeto(ARG_ARG);
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en getInfoOnAttributeNumberYOfX: " )
                         + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en getInfoOnAttributeNumberYOfX");
        }
}

// ------------------------------------- Runtime::getInfoOnAttributeNumberYOfX()
void Runtime::getInfoOnAttributeNumberYOfX(Metodo *met)
{
        Atributo *atr;

        try {
                Objeto *objeto = getObjetoArgumento(met, ARG_ARG);
                INT32  *numero = static_cast<INT32 *>(
                            getDatoArgumento(met, Runtime::gestorNumerosEnteros, ARG_X));

                if (objeto != NULL
                 && numero != NULL)
                {
                    // Buscar el objeto de deseo
                    atr = objeto->lAtrs.getElementoNumero(*numero);

                    if (atr != NULL)
                    {
                        Objeto *info   = new Objeto( NULL, "", Runtime::objetoRaiz );
			Runtime::objetoInfoAtr->copia( info, "", Objeto::SYSTEM_COPY );

			// Comprobar que los campos existen
			if (info->lAtrs.busca("access")      == NULL
			 || info->lAtrs.busca("name")        == NULL
			 || info->lAtrs.busca("references")  == NULL
			 || info->lAtrs.busca("ordinal")     == NULL)
			{
				ponExcepcion(Runtime::objetoExcepcion, EXC_FALTANATRS);
				delete info;
				info = Runtime::objetoNulo;
			}
			else {
				// Rellenar los atributos del objeto de información
				// Objeto dueño del atributo
				info->lAtrs.busca("owner")->ponReferencia(objeto);

				// Acceso
				if (atr->esPublico())

					info->lAtrs.busca("access")->ponReferencia(
								Runtime::objetoTrue);
				else    info->lAtrs.busca("access")->ponReferencia(
								Runtime::objetoFalse);

				// Nombre del atributo
				Objeto *atrNombre = Runtime::gestorCadenas->nuevo("",
								atr->getNombre());
				info->lAtrs.busca("name")->ponReferencia(atrNombre);

				// A dónde señala
				info->lAtrs.busca("references")->ponReferencia(
								atr->getObjeto());

				// Su número en el objeto
				Objeto *numeroAtr = Runtime::gestorNumerosEnteros->nuevo("",
					objeto->lAtrs.buscaIndicePorNombre(
								atr->getNombre() ) );
				info->lAtrs.busca("ordinal")->ponReferencia(numeroAtr);
			}
			// Fin
			met->ponRetorno(info);
                    } else ponExcepcionObjeto(EXC_NOATR);
                } else ponExcepcionObjeto(ARG_ARG);
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string( "error en argumentos en getInfoOnAttributeNumberYOfX: " )
                         + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en getInfoOnAttributeNumberYOfX");
        }
}

// ------------------------------------- Runtime::getAttributeNumberByNameYOfX()
void Runtime::getAttributeNumberByNameYOfX(Metodo *met)
{
        try {
            Objeto *obj            = getObjetoArgumento(met, ARG_ARG);
            std::string *nombreAtributo = static_cast<std::string *>(getDatoArgumento
                                      (met, Runtime::gestorCadenas, ARG_X));

            if (nombreAtributo != NULL
             && obj            != NULL)
            {
                size_t i = obj->lAtrs.buscaIndicePorNombre(*nombreAtributo);

		// Si i es igual
		// al número de atributos en el objeto, error
                if (i == obj->lAtrs.getNumero())
                        Runtime::ponExcepcionObjeto(ARG_ARG);
                else    met->ponRetorno(Runtime::gestorNumerosEnteros->nuevo("", i));
            }
            else Runtime::ponExcepcionObjeto(ARG_ARG);
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en getAttributeNumberByName: "  )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en getAttributeNumberByName");
        }
}

// ------------------------------------------- Runtime::getAttributeNumberYOfX()
void Runtime::getAttributeNumberYOfX(Metodo *met)
{
        try {
            Objeto *obj = getObjetoArgumento(met, ARG_ARG);
            INT32  *numeroAtributo = static_cast<INT32 *>(getDatoArgumento
                                      (met, Runtime::gestorNumerosEnteros, ARG_X));

            if (numeroAtributo != NULL
             && obj            != NULL)
            {
	    	// Si numeroAtributo es igual
            // al número de atributos en el objeto, error
                if ( ( (size_t ) *numeroAtributo ) == obj->lAtrs.getNumero() )
                     Runtime::ponExcepcionObjeto(ARG_ARG);
                else {
                        Atributo * atr =
                            obj->lAtrs.getElementoNumero( *numeroAtributo );
                        met->ponRetorno( atr->getObjeto() );
                }
            }
            else Runtime::ponExcepcionObjeto(ARG_ARG);
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en getAttributeNumber: " )
                         + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en getAttributeNumber");
        }
}

// ------------------------------------- Runtime::getNameOfAttributeNumberYOfX()
void Runtime::getNameOfAttributeNumberYOfX(Metodo * met)
{
        try {
            Objeto  *obj = getObjetoArgumento(met, ARG_ARG);
            INT32   *numeroAtributo = static_cast<INT32 *>(getDatoArgumento
                                      (met, gestorNumerosEnteros, ARG_X));

            if (numeroAtributo != NULL
             && obj            != NULL)
            {
                std::string nombre =obj->lAtrs.getNombreElementoNumero(*numeroAtributo);

                if (nombre.empty())
                     Runtime::ponExcepcionObjeto(ARG_ARG);
                else met->ponRetorno(gestorCadenas->nuevo("", nombre));
            }
            else Runtime::ponExcepcionObjeto(ARG_ARG);
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string( "error en argumentos en getNameOfAttributeNumberYOfX: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en getNameOfAttributeNumberYOfX");
        }
}

// ---------------------------------------- Runtime::getNameOfMethodNumberYOfX()
void Runtime::getNameOfMethodNumberYOfX(Metodo *met)
{
        try {
            Objeto *obj       = getObjetoArgumento(met, ARG_ARG);
            INT32  *numMetodo = static_cast<INT32 *>(getDatoArgumento
                                      (met, gestorNumerosEnteros, ARG_X));

            if (numMetodo != NULL
             && obj != NULL)
            {
                std::string nombre = obj->lMets.getNombreElementoNumero( *numMetodo );

                if ( nombre.empty() )
                     Runtime::ponExcepcionMetodo(ARG_ARG);
                else met->ponRetorno( gestorCadenas->nuevo( "", nombre ) );
            }
            else Runtime::ponExcepcionObjeto( ARG_ARG );
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en getNameOfAttributeNumberYOfX: " )
                         + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en getNameOfAttributeNumberYOfX");
        }
}

// ------------------------------------- Runtime::getOpcodesOfMethodNumberYOfX()
void Runtime::getOpcodesOfMethodNumberYOfX(Metodo *met)
{
        try {
            Objeto *obj       = getObjetoArgumento(met, ARG_ARG);
            INT32  *numMetodo = static_cast<INT32 *>(getDatoArgumento
                                      (met, gestorNumerosEnteros, ARG_X));

            if ( numMetodo != NULL
             &&  obj       != NULL )
            {
                Metodo * metDest = obj->lMets.getElementoNumero( *numMetodo );

                if ( metDest == NULL ) {
                     Runtime::ponExcepcionMetodo( ARG_ARG );
		}
                else {
			// Retorno
			met->ponRetorno( gestorCadenas->nuevo( "",
						 metDest->getListaOpcodes() )
			);
		}
            }
            else Runtime::ponExcepcionObjeto( ARG_ARG );
        }
        catch ( Excepcion &e )
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en getOpcodesOfAttributeNumberYOfX: " )
                         + e.getDetalles() ).c_str()
                );
        }
        catch ( ... )
        {
                throw ELibIlegal("FATAL error en getOpcodesOfAttributeNumberYOfX");
        }
}

// ================================================= Funcionalidad de la Consola
// ----------------------------------------------- Runtime::openDefaultConsole()
void Runtime::openDefaultConsole(Metodo * met)
/**
        En esta implantación, sólo texto, este método no hace casi nada, pues
        la consola (la del sistema) ya debería estar abierta.
*/
{
        met->ponRetorno( objetoNulo );
        console->open();
}

// ---------------------------------------------- Runtime::closeDefaultConsole()
void Runtime::closeDefaultConsole(Metodo * met)
/**
        En esta implantación, sólo texto, este método no hace nada, pues
        la consola (la del sistema) se cerrará cuando termine el programa.
*/
{
        met->ponRetorno( objetoNulo );
        console->close();
}

// ------------------------------------------- Runtime::readFromDefaultConsole()
void Runtime::readFromDefaultConsole(Metodo *met)
{
        Objeto *obj = console->read();

        if ( obj != NULL )
        {
                met->ponRetorno( obj );
        }
        else Runtime::ponExcepcion( Runtime::objetoExcepcion );
}

// -------------------------------------------- Runtime::writeToDefaultConsole()
void Runtime::writeToDefaultConsole(Metodo *met)
{
        Objeto     * obj = NULL;
        Referencia * ref = met->vblesLocales.busca( "obj" );

        met->ponRetorno( Runtime::objetoNulo );

        if ( ref != NULL ) {
                obj = ref->getObjeto();
	}

        if ( obj != NULL )
                console->write( obj );
        else    throw ELibIlegal("writeToDefaultConsole() incorrecto");
}

// ----------------------------------------------- Runtime::lfToDefaultConsole()
void Runtime::lfDefaultConsole(Metodo *met)
{
        met->ponRetorno( Runtime::objetoNulo );

        console->write( NULL, Zero_Console_::CR );
}


// ===================================================== Funcionalidad del stack
// ---------------------------------------------------- Runtime::toStringStack()
std::string Stack::toString() const
{
    std::string st;
    for(register size_t i = 0; i < getNumero(); ++i)
    {
        const Stack::StackInfoItem * sti = getElementoNumero( i );

        st   += sti->getObjeto()->getNombre();
        st   += " executing ";

        Metodo * metDonde = sti->getMetodo();

        st   += sti->getObjetoDeMetodo()->getNombre();
        st   += ".";

        if ( metDonde != NULL )
                st   += sti->getNombreMetodo();
        else    st   += "<unexisting method>";

        st   += "()\n";
    }

    return st;
}

void Runtime::toStringStack(Metodo *met)
{
        std::string toret = Runtime::gestorStack->toString();

        toret += '\n';

        Objeto * retorno = Runtime::gestorCadenas->nuevo( "", toret );

        if ( retorno == NULL )
                met->ponRetorno( Runtime::gestorCadenas->getZero() );
        else    met->ponRetorno( retorno );
}

// ================================================ Funcionalidad de las cadenas
// -------------------------------------------------- Runtime::isEqualToString()
void Runtime::isEqualToString(Metodo *met)
{
        std::string * str1;
		std::string * str2;

        try {
            str1 = static_cast<std::string *>(
                         getDatoArgumento( met, Runtime::gestorCadenas, ARG_ARG1 ) );

            if ( str1 != NULL )
            {
                    str2  = static_cast<std::string *>(
                         getDatoArgumento( met, Runtime::gestorCadenas, ARG_ARG2 ) );

                    if ( str2 != NULL )
                    {
                        if ( (*str1) == (*str2) )
                                met->ponRetorno( Runtime::objetoTrue );
                        else    met->ponRetorno( Runtime::objetoFalse );

                    }
                    else Runtime::ponExcepcionTipoCadena( ARG_ARG2 );
            } else Runtime::ponExcepcionTipoCadena( ARG_ARG1 );
        }
        catch ( Excepcion &e )
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en isEqualToString: " )
                         + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en isEqualToString");
        }
}

// ------------------------------------------------- Runtime::isLessThanString()
void Runtime::isLessThanString(Metodo *met)
{
		std::string * str1;
        std::string * str2;

        try {
            str1 = static_cast<std::string *>( getDatoArgumento(met,
                                               Runtime::gestorCadenas, ARG_ARG1 ) );

            if ( str1 != NULL )
            {
                    str2 = static_cast<std::string *>( getDatoArgumento(met,
                                               Runtime::gestorCadenas, ARG_ARG2 ) );

                    if ( str2 != NULL )
                    {
                        if ( (*str1) < (*str2) )
                                met->ponRetorno( Runtime::objetoTrue );
                        else    met->ponRetorno( Runtime::objetoFalse );
                    }
                    else Runtime::ponExcepcionTipoCadena( ARG_ARG2 );
            } else Runtime::ponExcepcionTipoCadena( ARG_ARG1 );
        }
        catch ( Excepcion &e )
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en isLessThanString: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en isLessThanString");
        }
}

// ------------------------------------------------- Runtime::isGreaterThanString()
void Runtime::isGreaterThanString(Metodo *met)
{
	std::string *str1;
    std::string *str2;

    try {
            str1 = static_cast<std::string *>( getDatoArgumento(met,
                                               Runtime::gestorCadenas, ARG_ARG1 ) );

            if ( str1 != NULL )
            {
                    str2 = static_cast<std::string *>( getDatoArgumento(met,
                                               Runtime::gestorCadenas, ARG_ARG2 ) );

                    if ( str2 != NULL )
                    {
                        if ( (*str1) > (*str2) )
                                met->ponRetorno( Runtime::objetoTrue );
                        else    met->ponRetorno( Runtime::objetoFalse );
                    }
                    else Runtime::ponExcepcionTipoCadena( ARG_ARG2 );
            } else Runtime::ponExcepcionTipoCadena( ARG_ARG1 );
        }
        catch ( Excepcion &e )
        {
                throw ELibIlegal(
                        ( std::string( "error en argumentos en isGreaerThanString: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en isGreaterThanString");
        }
}

// ----------------------------------------------------- Runtime::assignString()
void Runtime::assignString(Metodo *met)
{
        try {
            Objeto     * obj1 = getObjetoArgumento( met, ARG_ARG1 );
            std::string     * str  = static_cast<std::string *>(
                       getDatoArgumento( met, Runtime::gestorCadenas, ARG_ARG2 ) );

            if ( obj1 == NULL ) {
				Runtime::ponExcepcionObjeto( ARG_ARG1 );
			}
			else
            if( str != NULL) {
                Runtime::gestorCadenas->modifica( obj1, *str );
                met->ponRetorno( obj1 );
            }
            else Runtime::ponExcepcionTipoCadena( ARG_ARG2 );
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en assignString: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en assignString");
        }
}

// ----------------------------------------------------- Runtime::concatString()
void Runtime::concatString(Metodo *met)
{
    std::string *str1;
	std::string *str2;

        try {
            str1 = (static_cast<std::string *>(
                        getDatoArgumento(met, Runtime::gestorCadenas, ARG_ARG1)));

            if (str1 != NULL)
            {
                str2 = static_cast<std::string *>(
                         getDatoArgumento(met, Runtime::gestorCadenas, ARG_ARG2));

                if (str2 != NULL)
                {
                    met->ponRetorno(
                          Runtime::gestorCadenas->nuevo("", (*str1) + (*str2) ) );
                }
                else Runtime::ponExcepcionTipoCadena( ARG_ARG2 );
            } else Runtime::ponExcepcionTipoCadena( ARG_ARG1 );
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string( "error en argumentos en assignString: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en assignString");
        }
}

// ------------------------------------------------------- Runtime::zeroString()
void Runtime::zeroString(Metodo *met)
{
        met->ponRetorno(Runtime::gestorCadenas->getZero());
}

// --------------------------------------------------- Runtime::lengthOfString()
void Runtime::lengthOfString(Metodo *met)
{
	try {
		Objeto * obj = getObjetoArgumento(met, ARG_ARG);

		if (obj != NULL)
		{
			std::string *s = Runtime::gestorCadenas->busca(obj);
			met->ponRetorno(
				Runtime::gestorNumerosEnteros
					             ->nuevo( "", s->length() )
			);
		} else Runtime::ponExcepcionTipoCadena( ARG_ARG );
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en lengthOfString: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en lengthOfString");
        }

}

// ------------------------------------------------ Runtime::getPositionString()
void Runtime::getPositionString(Metodo *met)
{
        Objeto     * obj = NULL;
        Referencia * ref = met->vblesLocales.busca( ARG_ARG );
        Referencia * refn= met->vblesLocales.busca( ARG_N );
        std::string *s   = NULL;
        INT32 *pos = NULL;

        if ( ref != NULL ) {
            obj = ref->getObjeto();

            if ( obj != NULL )
                    s = Runtime::gestorCadenas->busca( obj );
            else    Runtime::ponExcepcionObjeto( ARG_ARG );

            if ( s == NULL ) {
                    Runtime::ponExcepcionTipoCadena( ARG_ARG );
            }
        } else throw ELibIlegal("getPositionString(): argumentos incorrectos");

        if ( refn != NULL ) {
            if ( ref != NULL ) {
                pos = Runtime::gestorNumerosEnteros->busca( refn->getObjeto() );
            } else Runtime::ponExcepcionObjeto( ARG_N );

            if ( pos == NULL ) {
                    Runtime::ponExcepcionTipoNumero( ARG_N );
            }
        } else throw ELibIlegal("getPositionString(): argumentos incorrectos");

        // Comprobar límites
        if ( *pos < 0 ) {
            *pos = 0;
        }

        if ( ( (size_t) *pos ) >= s->length() ) {
            *pos = s->length() - 1;

        }

        // Operar
        met->ponRetorno(Runtime::gestorCadenas
                ->nuevo( "", s->substr( *pos, 1 ) ) )
        ;
}

// ---------------------------------------------------- Runtime::toIntString()
void Runtime::toIntString(Metodo * met)
{
        Objeto     * obj = NULL;
        Referencia * ref = met->vblesLocales.busca( ARG_ARG );
        std::string * str;
        char       * ch;
        INT32 num;

        if (ref != NULL) {
                obj = ref->getObjeto();
	}

        if (obj != NULL)
        {
                str = gestorCadenas->busca(obj);
                if (str != NULL)
                {
                    num = (INT32) strtod(str->c_str(), &ch);

                    if (*ch == '\0')
                    {
                        met->ponRetorno(Runtime::gestorNumerosEnteros->nuevo("", num));
                    }
                    else Runtime::ponExcepcionTipoNumero( obj->getNombre() );
                }
                else Runtime::ponExcepcionObjeto( obj->getNombre() );
        }
        else throw ELibIlegal("toIntString() incorrecto");
}

// ---------------------------------------------------- Runtime::toFloatString()
void Runtime::toFloatString(Metodo * met)
{
        Objeto     * obj = NULL;
        Referencia * ref = met->vblesLocales.busca( ARG_ARG );
        std::string * str;
        char       * ch;
        REAL num;


        if (ref != NULL)
                obj = ref->getObjeto();

        if (obj != NULL)
        {
                str = gestorCadenas->busca(obj);
                if (str != NULL)
                {
                    num = strtod(str->c_str(), &ch);

                    if (*ch == '\0')
                    {
                        met->ponRetorno(Runtime::gestorNumerosFlotantes->nuevo("", num));
                    }
                    else Runtime::ponExcepcionTipoNumero( obj->getNombre() );
                }
                else Runtime::ponExcepcionObjeto( ARG_ARG );
        }
        else throw ELibIlegal("toFloatString(): argumentos incorrectos");
}

// ----------------------------------------------- Runtime::seqFindInString()
void Runtime::seqFindInString(Metodo *met)
{
	try {
        std::string * s  = static_cast<std::string *>(
                   getDatoArgumento( met, Runtime::gestorCadenas, ARG_ARG ) );
        std::string * x  = static_cast<std::string *>(
                   getDatoArgumento( met, Runtime::gestorCadenas, ARG_X ) );
	    int * y  = static_cast<int*>( getDatoArgumento( met,
						Runtime::gestorNumerosEnteros,
						ARG_Y )
        );

		if ( s == NULL ) {
			ponExcepcionObjeto( ARG_ARG );
		}
		else
		if ( x == NULL ) {
			ponExcepcionObjeto( ARG_X );
		}
		else
		if ( y == NULL ) {
			ponExcepcionTipoNumero( ARG_Y );
		}
		else {
		    // Comprobar límites
		    if ( *y < 0 ) {
		        *y = 0;
		    }

		    if ( ((size_t) *y) >= s->length() ) {
		        *y = s->length() - 1;
		    }

			// Buscar
			std::string::const_iterator it = s->begin();
			advance( it, *y );
			char ch = (*x)[ 0 ];
			for(; it != s->end(); ++it)
			{
					// Si encontrado, salir
					if ( *it == ch ) {
						break;
                    }
			}

			// Fin
			if ( it != s->end() )
                    met->ponRetorno(
                        gestorNumerosEnteros->nuevo( "", it - s->begin() )
                    );
			else 	met->ponRetorno( objetoNulo );
		}
	}
	catch (Excepcion &e)
	{
			throw ELibIlegal(
				( std::string( "error en argumentos en seqFindInString: " )
                + e.getDetalles() ).c_str()
			);

	}
	catch (...)
	{
			throw ELibIlegal( "FATAL error en seqFindInString" );
	}
}

// ---------------------------------------------- Runtime::seqFindLastInString()
void Runtime::seqFindLastInString(Metodo *met)
{
	try {
        std::string * s  = static_cast<std::string *>(
                   getDatoArgumento( met, Runtime::gestorCadenas, ARG_ARG ) );
        std::string * x  = static_cast<std::string *>(
                   getDatoArgumento( met, Runtime::gestorCadenas, ARG_X ) );
	    int * y  = static_cast<int*>( getDatoArgumento( met,
						Runtime::gestorNumerosEnteros,
						ARG_Y )
        );

		if ( s == NULL ) {
			ponExcepcionObjeto( ARG_ARG );
		}
		else
		if ( x == NULL ) {
			ponExcepcionObjeto( ARG_X );
		}
		else
		if ( y == NULL ) {
			ponExcepcionTipoNumero( ARG_Y );
		}
		else {
		    // Comprobar límites
		    if ( *y < 0 ) {
		        *y = 0;
		    }

		    if ( ((size_t) *y) >= s->length() ) {
		        *y = s->length() - 1;
		    }

			// Buscar
			std::string::const_iterator it = s->begin();
			advance( it, *y );
			char ch = (*x)[ 0 ];
			for(; it != s->begin(); --it)
			{
					// Si encontrado, salir
					if ( *it == ch ) {
						break;
                    }
			}

			// Fin
			if ( it != s->begin() )
                    met->ponRetorno( gestorNumerosEnteros->nuevo( "", it - s->begin() ) );
			else {
                    if ( *it == (*x)[ 0 ] )
                            met->ponRetorno( gestorNumerosEnteros->nuevo( "", 0 ) );
			    	else    met->ponRetorno( objetoNulo );
			}
		}
	}
	catch (Excepcion &e)
	{
			throw ELibIlegal(
				( std::string( "error en argumentos en seqFindInString: " )
                + e.getDetalles() ).c_str()
			);

	}
	catch (...)
	{
			throw ELibIlegal( "FATAL error en seqFindInString" );
	}
}

// -------------------------------------------------- Runtime::substringString()
void Runtime::substringString(Metodo *met)
{
        Objeto     * obj = NULL;
        Objeto     * beg = NULL;
        Objeto     * end = NULL;
        Referencia * ref = met->vblesLocales.busca( ARG_ARG );
        INT32 * beginSub;
        INT32 * endSub;

        if ( ref != NULL )
        {
                obj = ref->getObjeto();
                beg = met->vblesLocales.busca( ARG_N1 )->getObjeto();
                end = met->vblesLocales.busca( ARG_N2 )->getObjeto();
        }

        if ( obj != NULL
		  && beg != NULL
  		  && end != NULL )
        {
                std::string *s = Runtime::gestorCadenas->busca( obj );
                beginSub       = Runtime::gestorNumerosEnteros->busca( beg );
                endSub         = Runtime::gestorNumerosEnteros->busca( end );

				if ( s == NULL ) {
					ponExcepcionObjeto( ARG_ARG );
				}
				else
				if ( beginSub == NULL ) {
					ponExcepcionTipoNumero( ARG_N1 );
				}
				else
				if ( endSub == NULL ) {
					ponExcepcionTipoNumero( ARG_N2 );
				}
				else {
                    // Comprobar límites
                    if ( *beginSub < 0 ) {
                         *beginSub = 0;
                    }

                    if ( ((size_t) *beginSub) >= s->length() ) {
                        *beginSub = s->length() - 1;
                    }

                    met->ponRetorno(Runtime::gestorCadenas->nuevo("",
                                                  s->substr( *beginSub, *endSub )));
				}
        }
        else    throw ELibIlegal("getPositionString(): argumentos incorrectos");
}

// ================================================ Funcionalidad de los enteros
// ---------------------------------------------------------------- toFloatInt()
void Runtime::toFloatInt(Metodo *met)
{
        try {
	        INT32 *valor = static_cast<INT32 *>(
       	        getDatoArgumento( met, Runtime::gestorNumerosEnteros, ARG_ARG ) );

            if (valor != NULL) {
                met->ponRetorno(Runtime::gestorNumerosFlotantes->nuevo("", (REAL) *valor));
            }
            else Runtime::ponExcepcionObjeto( ARG_ARG );
        }
        catch (Excepcion &e)
        {
              throw ELibIlegal(
                        ( std::string("error en argumentos en toFloatInt: " )
                        + e.getDetalles() ).c_str()
              );
        }
        catch(...)
        {
            throw ELibIlegal("FATAL error en toFloatInt");
        }
}

// -------------------------------------------------------- Runtime::zeroInt()
void Runtime::zeroInt(Metodo *met)
{
        met->ponRetorno(gestorNumerosEnteros->getZero());
}

// ------------------------------------------------------- Runtime::toString()
void Runtime::toString(Metodo *met)
{
        try {
				Objeto * valor = getObjetoArgumento( met, ARG_ARG );

                if ( valor != NULL ) {
					met->ponRetorno( Operando(valor ).toString() );
                }
                else Runtime::ponExcepcionObjeto( ARG_ARG );
        }
		catch (ENoEsNumero &e)
		{
			Runtime::ponExcepcionTipoNumero( e.getDetalles() );
		}
        catch (Excepcion &e)
        {
              throw ELibIlegal(
                        ( std::string( "error en argumentos en toString: " )
                        + e.getDetalles() ).c_str()
              );
        }
        catch(...)
        {
            throw ELibIlegal("FATAL error en toString");
        }
}

// -------------------------------------------------- Runtime::multiplyBy()
void Runtime::multiplyBy(Metodo *met)
{
        try {
			Objeto *valor1 = getObjetoArgumento( met, ARG_ARG1 );

            if ( valor1 != NULL )
            {
                    Objeto * valor2 = getObjetoArgumento( met, ARG_ARG2 );

                    if ( valor2 != NULL )
                    {
                        Operando op1( valor1 );
						Operando op2( valor2 );

                        met->ponRetorno( op1.opMatBinaria( op2, Operando::MULTIPLICACION ) );
                    }
                    else Runtime::ponExcepcionTipoNumero( ARG_ARG2 );
            } else Runtime::ponExcepcionObjeto( ARG_ARG1 );
        }
		catch (ENoEsNumero &e) {
			ponExcepcionTipoNumero( e.getDetalles() );
		}
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string( "error en argumentos en multiplyBy: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en multiplyBy");
        }
}

// ------------------------------------------------------ Runtime::divideBy()
void Runtime::divideBy(Metodo *met)
{
		Objeto * valor2 = NULL;
		Objeto * valor1 = NULL;

        try {
			valor1 = getObjetoArgumento( met, ARG_ARG1 );

            if ( valor1 != NULL )
            {
                    valor2 = getObjetoArgumento( met, ARG_ARG2 );

                    if ( valor2 != NULL )
                    {
                        Operando op1( valor1 );
						Operando op2( valor2 );

                        met->ponRetorno( op1.opMatBinaria( op2, Operando::DIVISION ) );
                    }
                    else Runtime::ponExcepcionTipoNumero( ARG_ARG2 );
            } else Runtime::ponExcepcionObjeto( ARG_ARG1 );
        }
		catch (ENoEsNumero &e) {
			ponExcepcionTipoNumero( e.getDetalles() );
		}
		catch(EDivisionPorCero &) {
			ponExcepcionMates( EXC_DIVZERO, valor2->getNombre() );
		}
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string( "error en argumentos en divideBy: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en divideBy");
        }
}

// ----------------------------------------------------------- Runtime::modInt()
void Runtime::modInt(Metodo *met)
{
        INT32 valor1;
        INT32 valor2;

        try {
			INT32 *valor = static_cast<INT32 *>(
							getDatoArgumento(
					met,
					Runtime::gestorNumerosEnteros, ARG_ARG1 ) )
			;

            if (valor != NULL)
            {
                    valor1 = *valor;

                    valor  = static_cast<INT32 *>(
                         	getDatoArgumento(
				    met,
				    Runtime::gestorNumerosEnteros, ARG_ARG2 ) )
		    ;

                    if (valor != NULL)
                    {
                        valor2 = *valor;

                        if (valor2 != 0)
                               met->ponRetorno(Runtime::gestorNumerosEnteros->nuevo("",
                                                              valor1 % valor2));

                        else   ponExcepcionMates( ARG_ARG2, EXC_DIVZERO );
                    }
                    else Runtime::ponExcepcionTipoNumero( ARG_ARG2 );
            } else Runtime::ponExcepcionObjeto( ARG_ARG1 );

        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en mod: " )
                         + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en mod");
        }
}

// --------------------------------------------------------- Runtime::subInt()
void Runtime::sub(Metodo *met)
{
        try {
			Objeto *valor1 = getObjetoArgumento( met, ARG_ARG1 );

            if ( valor1 != NULL )
            {
                    Objeto * valor2 = getObjetoArgumento( met, ARG_ARG2 );

                    if ( valor2 != NULL ) {
                        Operando op1( valor1 );
						Operando op2( valor2 );

                        met->ponRetorno( op1.opMatBinaria( op2, Operando::RESTA ) );
                    }
                    else Runtime::ponExcepcionTipoNumero( ARG_ARG2 );
            } else Runtime::ponExcepcionObjeto( ARG_ARG1 );
        }
		catch (ENoEsNumero &e) {
			ponExcepcionTipoNumero( e.getDetalles() );
		}
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en sub: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en sub");
        }
}

// -------------------------------------------------- Runtime::isNegativeInt()
void Runtime::isNegative(Metodo *met)
{
        try {
			Objeto * obj = getObjetoArgumento( met, ARG_ARG );

            if ( obj != NULL ) {
                if ( Operando( obj ).esNegativo() )
                        met->ponRetorno( Runtime::objetoTrue );
                else    met->ponRetorno( Runtime::objetoFalse );
            }  else Runtime::ponExcepcionObjeto( ARG_ARG );
        }
		catch (ENoEsNumero &e) {
			ponExcepcionTipoNumero( e.getDetalles() );
		}
		catch(Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en isNegative: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch(...)
        {
                throw ELibIlegal("FATAL error en isNegative");
        }
}

// --------------------------------------------------------- Runtime::sumInt()
void Runtime::sum(Metodo *met)
{

        try {
			Objeto *valor1 = getObjetoArgumento( met, ARG_ARG1 );

            if ( valor1 != NULL )
            {
                    Objeto * valor2 = getObjetoArgumento( met, ARG_ARG2 );

                    if ( valor2 != NULL )
                    {
                        Operando op1( valor1 );
						Operando op2( valor2 );

                        met->ponRetorno( op1.opMatBinaria( op2, Operando::SUMA ) );
                    }
                    else Runtime::ponExcepcionTipoNumero( ARG_ARG2 );
            } else Runtime::ponExcepcionObjeto( ARG_ARG1 );
        }
		catch (ENoEsNumero &e) {
			ponExcepcionTipoNumero( e.getDetalles() );
		}
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string( "error en argumentos en sum: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en sum:");
        }
}

// --------------------------------------------------------- Runtime::absInt()
void Runtime::abs(Metodo *met)
{
        try {
				Objeto *obj = getObjetoArgumento( met, ARG_ARG );

                if ( obj != NULL ) {
                        met->ponRetorno( Operando( obj ).abs() );
                }
                else Runtime::ponExcepcionObjeto(ARG_ARG);
        }
		catch (ENoEsNumero &e) {
			ponExcepcionTipoNumero( e.getDetalles() );
		}
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en abs: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en abs");
        }
}

// --------------------------------------------------- Runtime::isEqualToInt()
void Runtime::isEqualTo(Metodo *met)
{
        try {
  	    Objeto *valor1 = getObjetoArgumento( met, ARG_ARG1 );

            if ( valor1 != NULL )
            {
                    Objeto * valor2 = getObjetoArgumento( met, ARG_ARG2 );

                    if ( valor2 != NULL ) {
                        Operando op1( valor1 );
                        Operando op2( valor2 );

                        if ( op1.opRelBinaria( op2, Operando::EQ ) )
                                met->ponRetorno( Runtime::objetoTrue );
                        else	met->ponRetorno( Runtime::objetoFalse );
                    }
                    else Runtime::ponExcepcionTipoNumero( ARG_ARG2 );
            } else Runtime::ponExcepcionObjeto( ARG_ARG1 );
        }
	catch (ENoEsNumero &e) {
		ponExcepcionTipoNumero( e.getDetalles() );
	}
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en isEqualTo: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en isEqualTo");
        }
}

// -------------------------------------------------- Runtime::isLessThanInt()
void Runtime::isLessThan(Metodo *met)
{
        try {
			Objeto *valor1 = getObjetoArgumento( met, ARG_ARG1 );

            if ( valor1 != NULL )
            {
                    Objeto * valor2 = getObjetoArgumento( met, ARG_ARG2 );

                    if ( valor2 != NULL )
                    {
                        Operando op1( valor1 );
						Operando op2( valor2 );

                        if ( op1.opRelBinaria( op2, Operando::LT ) )
								met->ponRetorno( Runtime::objetoTrue );
						else	met->ponRetorno( Runtime::objetoFalse );

                    }
                    else Runtime::ponExcepcionTipoNumero( ARG_ARG2 );
            } else Runtime::ponExcepcionObjeto( ARG_ARG1 );
        }
		catch (ENoEsNumero &e) {
			ponExcepcionTipoNumero( e.getDetalles() );
		}
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en isLessThan: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en isLessThan");
        }
}

// -------------------------------------------------- Runtime::isGreaterThanInt()
void Runtime::isGreaterThan(Metodo *met)
{
        try {
			Objeto *valor1 = getObjetoArgumento( met, ARG_ARG1 );

            if ( valor1 != NULL )
            {
                    Objeto * valor2 = getObjetoArgumento( met, ARG_ARG2 );

                    if ( valor2 != NULL )
                    {
                        Operando op1( valor1 );
						Operando op2( valor2 );

                        if ( op1.opRelBinaria( op2, Operando::GT ) )
								met->ponRetorno( Runtime::objetoTrue );
						else	met->ponRetorno( Runtime::objetoFalse );

                    }
                    else Runtime::ponExcepcionTipoNumero( ARG_ARG2 );
            } else Runtime::ponExcepcionObjeto( ARG_ARG1 );
        }
		catch (ENoEsNumero &e) {
			ponExcepcionTipoNumero( e.getDetalles() );
		}
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en isGreaterThan " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en isGreaterThan");
        }
}

// ------------------------------------------------------ Runtime::assign()
void Runtime::assign(Metodo *met)
{
        try {
            Objeto * obj1  = getObjetoArgumento( met, ARG_ARG1 );
            Objeto * obj2  = getObjetoArgumento( met, ARG_ARG2 );

			if ( obj1 != NULL ) {
				if ( obj2 != NULL ) {
					Operando( obj1 ).asignar( Operando( obj2 ) );

					met->ponRetorno( obj1 );
				} else ponExcepcionTipoNumero( ARG_ARG2 );
			} else Runtime::ponExcepcionObjeto(ARG_ARG1);
		}
		catch (ENoEsNumero &e) {
			ponExcepcionTipoNumero( e.getDetalles() );
		}
        catch (Excepcion &e) {
                throw ELibIlegal(
                        ( std::string("error en argumentos en assign: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en assign");
        }
}

// ============================================== Funcionalidad de los flotantes
// -------------------------------------------------------- Runtime::toIntFloat()
void Runtime::toIntFloat(Metodo *met)
{
        REAL *valor = static_cast<REAL *>(
                         getDatoArgumento(met, Runtime::gestorNumerosFlotantes, ARG_ARG));


        try {
                if (valor != NULL) {
                    met->ponRetorno(Runtime::gestorNumerosEnteros->nuevo("", (INT32) *valor));
                }
                else Runtime::ponExcepcionObjeto( ARG_ARG );
        }
        catch (Excepcion &e)
        {
              throw ELibIlegal(
                        ( std::string("error en argumentos en toInt: " )
                        + e.getDetalles() ).c_str()
              );
        }
        catch(...)
        {
            throw ELibIlegal("FATAL error en toInt");
        }
}

// -------------------------------------------------------- Runtime::zeroFloat()
void Runtime::zeroFloat(Metodo *met)
{
        met->ponRetorno(gestorNumerosFlotantes->getZero());
}

// ============================================== Funcionalidad de los vectores
// ------------------------------------------------------ Runtime::sizeVector()
void Runtime::sizeVector(Metodo *met)
{
        try {
            LiteralVector_Zero * v  = static_cast<LiteralVector_Zero *>(
                       getDatoArgumento(met, Runtime::gestorVectores, ARG_ARG));

            if (v != NULL)
            {
               	met->ponRetorno( Runtime::gestorNumerosEnteros->nuevo("", v->longitud()) );
	    	}
            else Runtime::ponExcepcionObjeto( ARG_ARG );
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en sizeVector: " )
                        + e.getDetalles() ).c_str()
                );

        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en sizeVector");
        }
}

// ------------------------------------------------------- Runtime::clearVector()
void Runtime::clearVector(Metodo *met)
{
        try {
            LiteralVector_Zero * v  = static_cast<LiteralVector_Zero *>(
                       getDatoArgumento(met, Runtime::gestorVectores, ARG_ARG));

            if (v != NULL) {
				// Borra, destruye, elimina
				v->eliminaTodos();

				// Devolver el vector como resultado
				met->ponRetorno(met->vblesLocales.busca(ARG_ARG)->getObjeto());
	    	}
            else Runtime::ponExcepcionObjeto(ARG_ARG);
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en clearVector: " )
                        + e.getDetalles() ).c_str()
                );

        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en clearVector");
        }
}

// -------------------------------------------------- Runtime::addObjectVector()
void Runtime::addObjectVector(Metodo *met)
{
	try {
    	LiteralVector_Zero * v  = static_cast<LiteralVector_Zero *>(
        			getDatoArgumento( met, Runtime::gestorVectores, ARG_ARG )
		);
	    Objeto * obj = getObjetoArgumento( met, ARG_ARG1 );

        if ( v == NULL ) {
			ponExcepcionObjeto( ARG_ARG );
		}
		if ( obj == NULL ) {
			ponExcepcionObjeto( ARG_ARG1 );
		}
		else {
	    	// Insertarlo
	    	if ( v->mas( obj ) ) {
				// Devolver el vector como resultado
				met->ponRetorno(met->vblesLocales.busca(ARG_ARG)->getObjeto());
			}
		}
	}
	catch (Excepcion &e)
	{
		throw ELibIlegal(
					( std::string("error en argumentos en addObjectVector: " )
                    + e.getDetalles() ).c_str()
		);
	}
	catch (...)
	{
		throw ELibIlegal("FATAL error en addObjectVector");
	}
}

// ---------------------------------------------- Runtime::erasePositionVector()
void Runtime::erasePositionVector(Metodo *met)
{
        try {
            LiteralVector_Zero * v  = static_cast<LiteralVector_Zero *>(
                       getDatoArgumento(met, Runtime::gestorVectores, ARG_ARG));
            INT32 *valor = static_cast<INT32 *>(
                         getDatoArgumento(met, Runtime::gestorNumerosEnteros, ARG_N));

            if (  v == NULL ) {
				ponExcepcionObjeto( ARG_ARG );
			}
			else
			if ( valor == NULL ) {
				ponExcepcionTipoNumero( ARG_N );
			}
			else {
                // Borrarlo
                if ( v->eliminaPosicion(*valor) )
                        // Devolver el vector como resultado
                        met->ponRetorno(met->vblesLocales.busca(ARG_ARG)->getObjeto());
                else 	ponExcepcion(Runtime::objetoExcepcion, EXC_FUERADERANGO);
			}
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                        ( std::string("error en argumentos en erasePositionVector: " )
                        + e.getDetalles() ).c_str()
                );
        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en erasePositionVector");
        }
}

// ------------------------------------------------ Runtime::getPositionVector()
void Runtime::getPositionVector(Metodo *met)
{
	try {
	    LiteralVector_Zero * v  = static_cast<LiteralVector_Zero *>(
                       getDatoArgumento(met, Runtime::gestorVectores, ARG_ARG));
        INT32 *valor = static_cast<INT32 *>(
                         getDatoArgumento(met, Runtime::gestorNumerosEnteros, ARG_N));

        if (  v == NULL ) {
			ponExcepcionObjeto( ARG_ARG );
		}
		else
        if ( valor == NULL ) {
			ponExcepcionTipoNumero( ARG_N );
		}
		else {
			// Cogerlo
			Referencia *ref = v->getElemento(*valor);
			if ( ref != NULL )
				// Devolver la posición del vector como resultado
				met->ponRetorno( ref->getObjeto() );
			else ponExcepcion( Runtime::objetoExcepcion, EXC_FUERADERANGO );
		}
	}
	catch (Excepcion &e)
	{
			throw ELibIlegal(
					( std::string("error en argumentos en getPositionVector: " )
                    + e.getDetalles() ).c_str()
            );
	}
	catch (...)
	{
			throw ELibIlegal("FATAL error en getPositionVector");
	}
}

// ------------------------------------------------ Runtime::putPositionVector()
void Runtime::putPositionVector(Metodo *met)
{
	try {
	    LiteralVector_Zero * v  = static_cast<LiteralVector_Zero *>(
                       getDatoArgumento(met, Runtime::gestorVectores, ARG_ARG));
        INT32 *valor = static_cast<INT32 *>(
                         getDatoArgumento(met, Runtime::gestorNumerosEnteros, ARG_N));
	    Objeto *obj = getObjetoArgumento(met, ARG_X);

        if ( v == NULL ) {
			ponExcepcionObjeto( ARG_ARG );
		}
		else
		if ( valor == NULL ) {
			ponExcepcionTipoNumero( ARG_N );
		}
		else
		if ( obj == NULL) {
			ponExcepcionObjeto( ARG_X );
		}
		else
		// Modificarlo
		if ( v->modificaPosicion( *valor, obj ) )
				// Devolver el vector como resultado
				met->ponRetorno(met->vblesLocales.busca(ARG_ARG)->getObjeto());
		else	Runtime::ponExcepcion(Runtime::objetoExcepcion, EXC_FUERADERANGO);
	}
	catch (Excepcion &e)
	{
			throw ELibIlegal(
					( std::string("error en argumentos en putPositionVector: " )
                    + e.getDetalles() ).c_str()
            );
	}
	catch (...)
	{
			throw ELibIlegal("FATAL error en putPositionVector");
	}
}

// ----------------------------------------------- Runtime::insertObjectVector()
void Runtime::insertObjectVector(Metodo *met)
{
	try {
            LiteralVector_Zero * v  = static_cast<LiteralVector_Zero *>(
                       getDatoArgumento(met, Runtime::gestorVectores, ARG_ARG));
            INT32 *valor = static_cast<INT32 *>(
                         getDatoArgumento(met, Runtime::gestorNumerosEnteros, ARG_N));
	    	Objeto *obj = getObjetoArgumento(met, ARG_X);

            if ( v == NULL ) {
				ponExcepcionObjeto( ARG_ARG );
			}
	     	else
			if ( valor == NULL ) {
				ponExcepcionTipoNumero( ARG_N );
			}
			else
	     	if ( obj == NULL) {
				ponExcepcionObjeto( ARG_X );
			}
			else
			// Insertarlo
			if ( *valor >= 0
			  && ( (size_t) *valor ) < v->longitud() )
			{
				if (v->insertaElemento(*valor, obj))
					// Devolver el vector como resultado
					met->ponRetorno(met->vblesLocales.busca(ARG_ARG)->getObjeto());
				else Runtime::ponExcepcion( Runtime::objetoExcepcion, EXC_NOMEM );
			} else Runtime::ponExcepcion( Runtime::objetoExcepcion, EXC_FUERADERANGO );
	}
	catch (Excepcion &e)
	{
			throw ELibIlegal(
					( std::string("error en argumentos en insertObjectVector: " )
                    + e.getDetalles() ).c_str()
            );
	}
	catch (...)
	{
			throw ELibIlegal("FATAL error en insertObjectVector");
	}
}

// ----------------------------------------------- Runtime::isEqualToVector()
void Runtime::isEqualToVector(Metodo *met)
{
	try {
            LiteralVector_Zero * v1  = static_cast<LiteralVector_Zero *>(
                     getDatoArgumento(met, Runtime::gestorVectores, ARG_ARG1));
            LiteralVector_Zero * v2  = static_cast<LiteralVector_Zero *>(
                     getDatoArgumento(met, Runtime::gestorVectores, ARG_ARG2));

            if ( v1 == NULL ) {
				ponExcepcionObjeto( ARG_ARG1 );
			}
			else
			if ( v2 == NULL ) {
				ponExcepcionTipoVector( ARG_ARG2 );
            }
			else {
	    		// Compararlo
	    		if ( v1->equals( v2 ) )
						met->ponRetorno( Runtime::objetoTrue );
				else    met->ponRetorno( Runtime::objetoFalse );
			}
	}
	catch (Excepcion &e)
	{
			throw ELibIlegal(
					( std::string("error en argumentos en isEqualToVector: " )
                    + e.getDetalles() ).c_str()
            );
	}
	catch (...)
	{
			throw ELibIlegal("FATAL error en isEqualToVector");
	}
}

// ----------------------------------------------- Runtime::seqFindInVector()
void Runtime::seqFindInVector(Metodo *met)
{
	try {
		LiteralVector_Zero * v  = static_cast<LiteralVector_Zero *>(
                   getDatoArgumento( met, Runtime::gestorVectores, ARG_ARG ) );
        	Objeto * x  = getObjetoArgumento( met, ARG_X );
	    	int * y  = static_cast<int*>( getDatoArgumento( met,
						Runtime::gestorNumerosEnteros,
						ARG_Y )
	    	);

		if ( v == NULL ) {
			ponExcepcionObjeto( ARG_ARG );
		}
		else
		if ( x == NULL ) {
			ponExcepcionObjeto( ARG_X );
		}
		else
		if ( y == NULL ) {
			ponExcepcionTipoNumero( ARG_Y );
		}
		else {
			// Coger el método a ejecutar
			Metodo * metIgualQue = Metodo::encuentraMetodo( x, MET_IQUALQUE );

			if ( metIgualQue != NULL )
			{
				Objeto *objV = getObjetoArgumento( met, ARG_ARG );
				std::vector<Referencia *> arg;
				Atributo ref( objV, ARG_X, objetoNulo );

                const LiteralVector_Zero::VectorImplementacion *vector =
                    v->getVectorImplementacion()
                ;
				LiteralVector_Zero::VectorImplementacion::const_iterator it;

				arg.push_back( &ref );

				// Buscar
				if ( *y < 0 ) {
				    *y = 0;
				}
                if ( ((size_t) *y) >= v->longitud() ) {
                    *y = v->longitud() - 1;
                }

                it = vector->begin();
				advance( it, *y );
				for(; it != vector->end(); ++it)
				{
					// Crear argumento
					ref.ponReferencia( (*it)->getObjeto() );

					// Ejecutar el método
					metIgualQue->ejecuta( &arg, x );

					// Si encontrado, salir
					if ( metIgualQue->resultadoEjecucion() == objetoTrue ) {
						metIgualQue->finEjecucion();
						break;
					}

					metIgualQue->finEjecucion();
				}

				// Fin
				if ( it < vector->end() )
                        met->ponRetorno( gestorNumerosEnteros->nuevo( "",
                            it - vector->begin() )
                        );
				else 	met->ponRetorno( objetoNulo );
			} else Runtime::ponExcepcionMetodo( MET_IQUALQUE );
		}
	}
	catch (Excepcion &e)
	{
			throw ELibIlegal(
                    ( std::string( "error en argumentos en seqFindInVector: " )
                    + e.getDetalles() ).c_str()
			);

	}
	catch (...)
	{
			throw ELibIlegal( "FATAL error en seqFindInVector" );
	}
}

// ---------------------------------------------- Runtime::seqFindLastInVector()
void Runtime::seqFindLastInVector(Metodo *met)
{
	try {
	    LiteralVector_Zero * v  = static_cast<LiteralVector_Zero *>(
        			getDatoArgumento( met, Runtime::gestorVectores, ARG_ARG )
	    );
        Objeto * x  = getObjetoArgumento( met, ARG_X );
	    int * y  = static_cast<int*>( getDatoArgumento( met,
					  Runtime::gestorNumerosEnteros, ARG_Y )
	    );

        if ( v == NULL ) {
			ponExcepcionObjeto( ARG_ARG );
		}
		else
		if ( x == NULL ) {
			ponExcepcionObjeto( ARG_X );
		}
		else
		if ( y == NULL )  {
			ponExcepcionTipoNumero( ARG_Y );
		}
		else {
	    	// Coger el método a ejecutar
			Metodo * metIgualQue = Metodo::encuentraMetodo( x, MET_IQUALQUE );

			if ( metIgualQue != NULL )
			{
				Objeto *objV = getObjetoArgumento( met, ARG_ARG );
				std::vector<Referencia *> arg;
				Atributo ref( objV, ARG_X, objetoNulo );

				arg.push_back( &ref );

				// Buscar
				if ( *y < 0 ) {
				    *y = 0;
				}
                if ( ((size_t) *y) >= v->longitud() ) {
                    *y = v->longitud() - 1;
                }

                const LiteralVector_Zero::VectorImplementacion *vector =
                    v->getVectorImplementacion()
                ;
				LiteralVector_Zero::VectorImplementacion::const_iterator it;

                it = vector->begin();
				advance( it, *y );
				for(; it != vector->begin() ; --it)
				{
					// Crear argumento
					ref.ponReferencia( (*it)->getObjeto() );

					// Ejecutar el método
					metIgualQue->ejecuta( &arg, x );

					// Si encontrado, salir
					if (metIgualQue->resultadoEjecucion() == objetoTrue) {
						metIgualQue->finEjecucion();
						break;
					}
					metIgualQue->finEjecucion();
				}

				// Fin
				if ( it != vector->begin() ) {
					met->ponRetorno( gestorNumerosEnteros->nuevo( "",
                            it - vector->begin() )
                    );
				}
				else {
				    // Repetir para el primero
				    ref.ponReferencia( (*it)->getObjeto() );
				    metIgualQue->ejecuta( &arg, x );
				    if ( metIgualQue->resultadoEjecucion() == objetoTrue )
                            met->ponRetorno( gestorNumerosEnteros->nuevo( "", 0 ) );
                    else    met->ponRetorno( objetoNulo );
				}
			} else Runtime::ponExcepcionMetodo( MET_IQUALQUE );
	    }
	}
	catch (Excepcion &e)
	{
			throw ELibIlegal(
					( std::string( "error en argumentos en seqFindLastInVector: " )
                    + e.getDetalles() ).c_str()
			);
	}
	catch (...)
	{
			throw ELibIlegal( "FATAL error en seqFindLastInVector" );
	}
}

// ---------------------------------------------- Runtime::processVector()
void Runtime::processVector(Metodo *met)
{
	try {
            LiteralVector_Zero * v  = static_cast<LiteralVector_Zero *>(
                       getDatoArgumento(met, Runtime::gestorVectores, ARG_ARG1));
	    Objeto * objProc  = getObjetoArgumento( met, ARG_ARG2 );

        if ( v != NULL
	     &&  objProc != NULL )
        {
	    // Coger el método a ejecutar
		Metodo * metHazlo = Metodo::encuentraMetodo( objProc, MET_HAZLO );

		if (metHazlo != NULL)
		{
			Objeto *objV = getObjetoArgumento( met, ARG_ARG1 );
			std::vector<Referencia *> arg;
			Atributo ref( objV, ARG_X, objetoNulo );

			arg.push_back(&ref);

			// Buscar
			register size_t i = 0;
			for(; i < v->longitud(); ++i)
			{
				// Crear argumento
				ref.ponReferencia( v->getElemento(i)->getObjeto() );

				// Ejecutar el método
				metHazlo->ejecuta( &arg, objProc );

				// Si se indica parar, hacerlo
				if ( metHazlo->resultadoEjecucion() == objetoFalse
                  || metHazlo->enExcepcion() )
				{
					metHazlo->finEjecucion();
					break;
				}

				metHazlo->finEjecucion();
			}

			// Fin - devolver el número de elementos procesados
			met->ponRetorno( gestorNumerosEnteros->nuevo( "", i ) );
		}
		else Runtime::ponExcepcionMetodo( MET_HAZLO );
	    }
            else Runtime::ponExcepcionObjeto( ARG_ARG1 + ", " + ARG_ARG2 );
        }
        catch (Excepcion &e)
        {
                throw ELibIlegal(
                    ( std::string("error en argumentos en processVector: " )
                    + e.getDetalles() ).c_str()
                );

        }
        catch (...)
        {
                throw ELibIlegal("FATAL error en processVector");
        }
}

// ---------------------------------------------- Runtime::toStringVector()
void Runtime::toStringVector(Metodo *met)
{
	std::string str;
    Metodo * metToString;
	Objeto * obj;

	try {
		LiteralVector_Zero * v  = static_cast<LiteralVector_Zero *>(
                       getDatoArgumento(met, Runtime::gestorVectores, ARG_ARG )
		);

        if (  v == NULL ) {
			ponExcepcionObjeto( ARG_ARG );
		}
		else {
			// Recorrer todo el vector para generar la cadena
			register size_t i = 0;
			for(; i < v->longitud(); ++i)
			{
				// Ejecutar el método
				obj = v->getElemento(i)->getObjeto();
				metToString = Metodo::encuentraMetodo( obj, MET_TOSTRING );

				if ( metToString != NULL ) {
					// Crear la cadena para ese objeto
					metToString->ejecuta( &Metodo::argumentosNulos, obj );

					// Adjuntar a esta cadena
					str.append( *(gestorCadenas->busca(
								metToString->resultadoEjecucion() ) )
					);

					// concatenar la coma, si procede
					if ( i < ( v->longitud() - 1 ) ) {
						str.push_back( ',' );
						str.push_back( ' ' );
					}

					// Sacabó con éste
					metToString->finEjecucion();
				}
				else Runtime::ponExcepcionMetodo( MET_TOSTRING );
			}

			// Fin - devolver la cadena generada
			met->ponRetorno( gestorCadenas->nuevo( "", str ) );
	    }
	}
	catch (Excepcion &e)
	{
			throw ELibIlegal(
                ( std::string("error en argumentos en toStringVector: " )
                + e.getDetalles() ).c_str()
			);
	}
	catch (...)
	{
			throw ELibIlegal("FATAL error en toStringVector");
	}
}

// ---------------------------------------------- Runtime::deSincronizaMetodos()
void Runtime::deSincronizaMetodos()
{
        const Stack::StackInfoItem * sti;

        for(register size_t i = 0; i < gestorStack->getNumero(); ++i) {
        	sti = gestorStack->getElementoNumero(i);

			if ( sti->getMetodo() != NULL ) {
                	sti->getMetodo()->deSincroniza();
			}
        }
}

} // namespace Zero

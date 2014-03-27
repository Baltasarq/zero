// objeto.cpp
/*
      Implementación del soporte de objetos en Zero.

      Pertenece a zvm

      jbgarcia@uvigo.es
*/

#include "objeto.h"
#include "container.h"
#include "runtime.h"
#include "metodos.h"
#include "mnemos.h"

namespace Zero {

// ==================================================================== Atributo
// ---------------------------------------------------------- Atributo::Atributo
Atributo::Atributo(Objeto *po, const std::string &n, Objeto *d, bool acc, bool prop)
        : Referencia(po, NULL, n, d, prop), acceso(acc)
{}
Atributo::Atributo(Objeto *po, const std::string &n, const std::string &o, bool acc, bool prop)
        : Referencia(po, NULL, n, o, prop), acceso(acc)
{}

Atributo::~Atributo()
{
}

// ------------------------------------------------------------- Atributo::copia
Atributo *Atributo::copia(Objeto *obj)
{
        try {
	    Atributo *toret;

	    if (dynamic_cast<AtributoParent*>(this)) {
	      if (swizzled())
            	   toret = new AtributoParent(this->getPerteneceAlObjeto(),
		                            this->getNombre(),
					    this->getObjeto(),
					    this->esPublico());
	      else toret = new AtributoParent(this->getPerteneceAlObjeto(),
		                            this->getNombre(),
					    this->getNombreObjetoReferenciado(),
					    this->esPublico())
	      ;
	    }
	    else {
	      if ( swizzled() )
            	   toret = new Atributo(this->getPerteneceAlObjeto(),
		                            this->getNombre(),
					    this->getObjeto(),
					    this->esPublico());
	      else toret = new Atributo(this->getPerteneceAlObjeto(),
		                            this->getNombre(),
					    this->getNombreObjetoReferenciado(),
					    this->esPublico())
	      ;
	    }

            return toret;
        }
        catch(const std::bad_alloc &)
        {
                Runtime::ponExcepcion(Runtime::objetoExcepcion, EXC_NOMEM);
                return NULL;
        }
}

// ------------------------------------------------------ Atributo::alCambiar()
void Atributo::alCambiar()
{
	notificar( Observador::CambioEnElemento );
}

// =========================================================== AtributoEspecial
AtributoEspecial::AtributoEspecial(Objeto *po, const std::string &n, Objeto *d, bool acc, bool prop)
                : Atributo(po, n, d, acc, prop)
{
}

AtributoEspecial::AtributoEspecial(Objeto *po,
                       const std::string &n,
                       const std::string &o,
                       bool acc, bool prop)
                : Atributo(po, n, o, acc, prop)
{
}

// ====================================================================== Objeto
/*
// ------------------------------------------------------------ Objeto::new

void * Objeto::operator new(size_t size)
{
	Objeto * toret = (Objeto *) ::new char[size + 1];

	return toret;
}

// ------------------------------------------------------------ Objeto::delete
void Objeto::operator delete(void *ptr)
{

	::delete[] ((char *) ptr);
}
*/

// -------------------------------------------------- Objeto::AccesoListaMetodos
size_t Objeto::AccesoListaMetodos::getNumero() const
{
	return lm->getNumero();
}

const std::string &
Objeto::AccesoListaMetodos::getNombreElementoNumero(size_t i) const
{
	return lm->getNombreElementoNumero( i );
}

Metodo * Objeto::AccesoListaMetodos::getElementoNumero(size_t i) const
{
	return lm->getElementoNumero( i );
}

size_t
Objeto::AccesoListaMetodos::buscaIndicePorNombre(const std::string &x) const
{
	return lm->buscaIndicePorNombre( x );
}

Metodo * Objeto::AccesoListaMetodos::busca(const std::string &x) const
{
	return lm->busca( x );
}

bool Objeto::AccesoListaMetodos::inserta(const std::string &n, Metodo *met)
/// No se puede actualizar después de borrar
{
        if ( lm->inserta( n, met ) ) {
	        perteneceA->observar( met );
	        perteneceA->actualizar( met, NuevoElemento );

                return true;
        }

	return false;
}

bool Objeto::AccesoListaMetodos::borra(Metodo *met)
{
        if ( lm->busca( met ) ) {
                perteneceA->actualizar( met, BorradoElemento );
                lm->borra( met );
                perteneceA->actualizar( perteneceA, CambioEnElemento );

                return true;
        }

	return false;
}

bool Objeto::AccesoListaMetodos::borra(const std::string &n)
{
	return borra( lm->busca( n ) );
}

void Objeto::AccesoListaMetodos::eliminaTodos()
{
	lm->eliminaTodos();
        perteneceA->actualizar( NULL, BorradoElemento );
}

// ------------------------------------------------------------ Objeto::Objeto()
void Objeto::init()
{
	mnemosObjeto.clear();
	refsExtObjeto.clear();

    // Preparar conteo de referencias (objeto activo)
    referenceCount = 0;

	// Preparar los wrappers
	lAtrs.perteneceA = this;
	lAtrs.la = &la;
	lMets.perteneceA = this;
	lMets.lm = &lm;

	// Comprobar que el atributo para el objeto padre existe
	if ( atrObjetoPadre == NULL ) {
		throw ENoHayMemoria( std::string( "creando objeto: " + getNombre() ).c_str() );
	}

	if ( !lAtrs.inserta( ATR_PARENT, atrObjetoPadre ) ) {
		throw ENoHayMemoria( std::string( "creando objeto: " + getNombre() ).c_str() );
	}
}


Objeto::Objeto(AlmacenObjetos *pc, const std::string &n, const std::string &p) :
		modificadoDesdeCarga(false), metHerDin(NULL), nombre(n),
		contAlQuePertenece(pc), props(NULL)
{
	// ¿Herencia cíclica?
	if ( n == p ) {
		throw EChkHerenciaCiclica( n.c_str() );
	}

	// Crear atributo parent
    atrObjetoPadre = new(std::nothrow) AtributoParent( this, ATR_PARENT, p );

	if ( atrObjetoPadre == NULL ) {
			throw ENoHayMemoria( std::string( "creando: '" + n + '\'' ).c_str() );
	}

	init();
}

Objeto::Objeto(AlmacenObjetos *pc, const std::string &n, Objeto *p) :
		modificadoDesdeCarga(false), metHerDin(NULL), nombre(n),
		contAlQuePertenece(pc), props(NULL)
{
	// ¿Herencia cíclica?
	if ( n == p->getNombre() ) {
		throw EChkHerenciaCiclica( n.c_str() );
	}

	// Crear atributo parent
	atrObjetoPadre = new(std::nothrow) AtributoParent( this, ATR_PARENT, p );

	if ( atrObjetoPadre == NULL ) {
			throw ENoHayMemoria( std::string( "creando: '" + n + '\'' ).c_str() );
	}

	init();
}

Objeto::~Objeto()
{
	// Elimina la lista de mnemos para persistencia
	eliminarMnemosObjeto();

    // Eliminar los atributos del objeto
	lAtrs.eliminaTodos();

    // Eliminar los métodos del objeto
	lMets.eliminaTodos();

	// Flecos
	referenceCount     = -110;      // Objeto muerto
	contAlQuePertenece = NULL;
	atrObjetoPadre     = NULL;
}

// ------------------------------------------------------- Objeto::insertando()
bool Objeto::insertando(Atributo *atr)
{
    bool toret = true;

    if ( dynamic_cast<AtributoEspecial *>( atr ) != NULL ) {
        // Si se puede llegar a él, y es container, enlazar correctamente
        if ( atr->swizzled()
          && dynamic_cast<AlmacenObjetos *>( atr->getObjeto() ) )
        {
                Runtime::ponExcepcion( Runtime::objetoExcepcion, EXC_NOCONTAINER );
                toret = false;
        }
    }

    return toret;
}

// -------------------------------------------------------- Objeto::actualiza()
void Objeto::actualizar(Observable *rmt, TipoCambio t)
/**
 * Este método deriva de Observador. Será ejecutado cuando algún atributo
 * o método cambie.
 * @param rmt El miembro que ha cambiado (un atributo o un método)
 * @param t El tipo de cambio
 */
{
        modificadoDesdeCarga = true;

        if ( t != BorradoElemento ) {
        	// Notificar al contenedor
                ponModificadoDesdeCarga();
        }

	// Cuidado, atajo para evitar dynamic_cast()
	if ( t != NuevoElemento
	  && t != BorradoElemento )
	{
		goto END;
	}

	// Si ha sido adicionado, avisar
	if ( t == NuevoElemento
	 && dynamic_cast<Metodo *>( rmt ) != NULL )
	{
		if ( ( (Metodo *)rmt )->esHerenciaDinamica() )
		{
			metHerDin = (Metodo *) rmt;
		}
	}
	else
	// Comprobar si existe un método de herencia dinámica
	if ( t == BorradoElemento
	  && dynamic_cast<Metodo *>( rmt ) != NULL )
	{
		if ( ( (Metodo *)rmt )->esHerenciaDinamica() )
		{
			metHerDin = NULL;
		}
	}

	END:
	return;
}

// --------------------------------------------------------- Objeto::getProps()
Objeto *Objeto::getProps()
{
	if ( !gProps.existeObjetoProps() ) {
		gProps.ponNombre( getNombre() );
	}

	return gProps.getProps();
}

// ------------------------------------------------- Objeto::esDescendienteDe()
bool Objeto::esDescendienteDe(AlmacenObjetos *c, const std::string &nomPadre)
{
	Objeto *obj = c->busca( nomPadre );

	if ( obj != NULL )
		return esDescendienteDe( obj );
	else	throw EInterno( "petición esDescendienteDe( inexistente ) " );
}

bool Objeto::esDescendienteDe(Objeto *padre)
/**
 * Comprueba si un objeto a es descendiente de b. o si inclusive
 * son el mismo.
 * @param padre El padre del que queremos comprobar si desciende this
 * @return true si es descendiente o el mismo, false en otro caso
*/
{
        bool toret   = false;
        Objeto * obj = this;

        while( padre != NULL
           &&  obj   != Runtime::objetoRaiz
           &&  obj   != NULL )
        {
                if ( padre == obj )
                {
                    // Si encontrado, poner a True
                    toret = true;
                    break;
                }

                if ( obj->getAtrObjetoPadre() != NULL )
                        obj = obj->getAtrObjetoPadre()->getObjeto();
                else    obj = NULL;

                if ( obj == NULL ) {
                    Runtime::ponExcepcionObjeto(
                        "[IOF] parent not found: " + padre->getNombre()
                    );
                }
        }

        return toret;
}

// --------------------------------------------- Objeto::procHerenciaDinamica()
void Objeto::procHerenciaDinamica()
{
	if ( metHerDin != NULL ) {
		metHerDin->ejecuta( &Metodo::argumentosNulos, this );
	}
	else throw EInterno( std::string( "método de herencia dinámica en '"
			 	+ getNombre()
				+ "' ejecutado no existe" ).c_str() );
}

// --------------------------------------------- Objeto::decrementaReferencias()
void Objeto::decrementaReferencias(bool borrable)
{
	// Decrementar el número de referencias
    --referenceCount;

	// Si la referencia es 0, entonces deshacerse de él
	if ( referenceCount <= 0
	  && borrable )
	{
		bool esContainer = ( dynamic_cast<AlmacenObjetos *>( this ) != NULL );

	 	// Llamar al método de finalización (aka destructor)
		// Siempre que ésto no sea un contenedor, y que el objeto no
		// pertenezca a la librería estándar
		if ( !esContainer
		 &&  getPerteneceA() != Runtime::rt()->getContainerIntStdLib()
		 && !Runtime::rt()->estaEnFinalizacion() )
		{
			Metodo * metFin =
			  Metodo::encuentraMetodo( this, MET_DESTRUCTOR );

			if ( metFin != NULL )
			{
				referenceCount = 1;
                metFin->ejecuta( &Metodo::argumentosNulos, this );
				metFin->finEjecucion();
/*
				if ( Runtime::gestorExcepciones
				                    ->getExcepcion() != NULL ) {
					Runtime::gestorStack->ret();
				}
*/

				referenceCount = 0;
			}
		}

	 	// Borrar el objeto en sí
		if ( referenceCount <= 0 ) {
            // Si es un número o una cadena ...
			if ( !esContainer ) {
				Runtime::gestorCadenas->elimina( this );
				Runtime::gestorNumerosEnteros->elimina( this );
				Runtime::gestorNumerosFlotantes->elimina( this );

				if ( !Runtime::rt()->estaEnFinalizacion() ) {
                	Runtime::gestorVectores->elimina( this );
				}

			}

			delete this;
		}
	}
}

// ----------------------------------------------------- Objeto::copiaSatelite()
inline
Objeto * Objeto::copiaSatelite(Objeto *objSatelite, Atributo *atr)
{
	Objeto * objNuevo = new(std::nothrow) Objeto( NULL, "", Runtime::objetoRaiz );

	if ( objNuevo != NULL ) {
		if ( objSatelite->copia( objNuevo, "", SYSTEM_COPY ) )
			atr->ponReferencia( objNuevo );
		else {
			delete objNuevo;
			objNuevo = NULL;
		}

	}

	return objNuevo;
}

// ------------------------------------------------------------- Objeto::copia()
bool Objeto::copia(Objeto *dest, const std::string &n, bool copiaSistema)
{
	Objeto   *objSatelite;
	Objeto   *objNuevo;
    Metodo   *met;
    Atributo *atr;
	std::vector<Referencia *> args;
    bool      toret = true;
	register unsigned int i;
	Atributo atrAux( dest, "auxCopyAtr", Runtime::objetoRaiz );

	// Preparar los argumentos para las llamadas a copy("")
	if ( !copiaSistema ) {
		atrAux.ponReferencia( Runtime::gestorCadenas->getZero() );
		args.push_back( &atrAux );
	}

	// Si el destino no existe, no continuar
	if ( dest == NULL ) {
		toret = false;
		goto FIN;
	}

        try {
            // Nombre ?
            if ( n.empty() )
                 dest->nombre = ServidorDeNombres::getSigNombreUnico();
	    else dest->nombre = n;

	    // Preparar ciertos detalles
            dest->contAlQuePertenece = NULL;
            dest->referenceCount     = 0;

            // Copiar todos los métodos
            dest->lMets.eliminaTodos();
            for(i = 0; i < lMets.getNumero(); ++i)
            {
                    met = lMets.getElementoNumero(i)->copia(dest);

                    if (met != NULL)
                         dest->lMets.inserta(met->getNombre(), met);
                    else throw std::bad_alloc();
            }

            // Copiar los atributos
            dest->lAtrs.eliminaTodos();
            for(i = 0; i < lAtrs.getNumero(); ++i)
            {
                    // Copiar el atributo
                    atr = lAtrs.getElementoNumero(i)->copia(dest);

                    if (atr != NULL)
                    {
                        // Insertarlo
                        dest->lAtrs.inserta(atr->getNombre(), atr);

			// Es el padre ?
			if (dynamic_cast<AtributoParent *>(atr) != NULL) {
				// Entonces, apuntarlo
				dest->atrObjetoPadre = atr;
			}
			else {
				// Si no es el padre,
                        	// Copiar el objeto apuntado por él
				objSatelite = atr->getObjeto();

				if ( objSatelite != Runtime::objetoNulo
				 &&  objSatelite != this)
				{
				   if ( !copiaSistema )
				   {
					met = Metodo::encuentraMetodo(
					                        objSatelite,
								MET_COPY )
					;

					if ( met != NULL ) {
					  objNuevo = met->ejecuta( &args,
					                         objSatelite )
					  ;
					  atr->ponReferencia( objNuevo );
					  met->finEjecucion();
					}
					else {
					  if ( copiaSatelite( objSatelite, atr ) == NULL )
					  {
						throw std::bad_alloc();
					  }
				     }
				   } else {
				   	if ( copiaSatelite( objSatelite, atr ) == NULL )
					{
						throw std::bad_alloc();
					}
				   }
				}
			}
                    }
                    else throw std::bad_alloc();
            }

            toret = preparaObjetoEspecial( this, dest );
        } catch(std::bad_alloc &)
        {
                toret = false;
        }

	FIN:
        return toret;
}

Objeto * Objeto::copiaSistema(const std::string &nombre)
{
    Objeto *dest = new(std::nothrow) Objeto( getPerteneceA(),
                        "",
                        Runtime::objetoNulo
    );

    if ( dest == NULL ) {
        throw ENoHayMemoria( std::string( "copiando objeto: " + getNombre() ).c_str() );
    }

    if ( !( copia( dest, nombre , SYSTEM_COPY ) ) ) {
        throw ENoHayMemoria( std::string( "copiando objeto: " + getNombre() ).c_str() );
    }

    return dest;
}

inline
bool Objeto::preparaObjetoEspecial(Objeto *org, Objeto *dest)
{
    bool toret = true;

    // Es éste un objeto especial, conteniendo una cadena ?
    if (Runtime::gestorCadenas != NULL) {
        std::string *str;
        if ((str = Runtime::gestorCadenas->busca(org)) != NULL) {
            Runtime::gestorCadenas->mas(dest, *str);
        }
    }

    // Es éste un objeto especial, conteniendo un número flotante ?
    if (Runtime::gestorNumerosFlotantes != NULL) {
        REAL *num;
	if ((num = Runtime::gestorNumerosFlotantes->busca(org)) != NULL) {
            Runtime::gestorNumerosFlotantes->mas(dest, *num);
        }
    }

    // Es éste un objeto especial, conteniendo un número entero ?
    if (Runtime::gestorNumerosEnteros != NULL) {
        INT32 *num;
        if ((num = Runtime::gestorNumerosEnteros->busca(org)) != NULL) {
            Runtime::gestorNumerosEnteros->mas(dest, *num);
        }
    }

    // Es éste un objeto especial, conteniendo un vector ?
    if (Runtime::gestorVectores != NULL) {
        LiteralVector_Zero *v;
	if ((v = Runtime::gestorVectores->busca(org)) != NULL) {
            LiteralVector_Zero *v2 = new(std::nothrow) LiteralVector_Zero(dest);

            if (v2 != NULL) {
                v->copiaEn(v2);
                Runtime::gestorVectores->mas(dest, v2);
            }
            else toret = false;
        }
    }

    return toret;
}

// ------------------------------------------------ Objeto::getNombreCompleto()
const std::string &Objeto::getNombreCompleto()
{
    if ( getPerteneceA() == NULL ) {
        nombreCompleto = nombre;
        goto FIN;
    }

    // Tomar el nombre del contenedor
    if ( getPerteneceA() == Runtime::rt()->getContainerRaiz() )
    {
        nombreCompleto.clear();
    }
    else
    if ( getPerteneceA()->getPerteneceA() == Runtime::rt()->getContainerRaiz() )
    {
        nombreCompleto = getPerteneceA()->getRuta()
                         + getPerteneceA()->getNombre();
    }
    else {
        nombreCompleto =
                getPerteneceA()->getRuta()
              + CHR_SEP_IDENTIFICADOR + getPerteneceA()->getNombre();
    }

    // Tomar el nombre del objeto
    nombreCompleto += CHR_SEP_IDENTIFICADOR;
    nombreCompleto += getNombre();

    FIN:
    return nombreCompleto;
}

// -------------------------------------------------- Objeto::esObjetoProximo()
bool Objeto::esObjetoProximo(Objeto *obj)
/** * Un objeto es próximo sii:
    * <ul>
    *      <li> No es un objeto primitivo
    *      <li> Es un objeto temporal
    * </ul>
    * Especialmente, no
    * <ul>
        <li> Cuando está en el mismo contenedor: ¡será procesado!
    * </ul>
*/
{
    if ( Runtime::esObjetoPrimitivo( obj ) )
            return false;
    else    return esObjetoTemporal( obj );
}

// ------------------------------------------------- Objeto::esObjetoTemporal()
bool Objeto::esObjetoTemporal(Objeto *obj)
/** * Un objeto es temporal sii:
    * <ul>
    *      <li> Su contenedor es NULL (es decir, es .Exe)
    *      <li> Su contenedor es classof Transient
    * </ul>
*/
{
    return ( obj->getPerteneceA() == NULL
          || dynamic_cast<TransientContainer *>( obj->getPerteneceA() ) != NULL )
    ;
}

// -------------------------------------------- Objeto::getSigNombreVectorAux()
inline
std::string Objeto::getSigNombreVectorAux()
{
    std::string str = OBJ_VECTORAUX;

    str.append( AnalizadorLexico::toString( ++numVectoresAux ) );
    str.push_back( '_' );
    str.push_back( '_' );

    return str;
}

// ------------------------------------------ Objeto::listaMnemosCargarVector()
void Objeto::listaMnemosCargarVector(
                        const std::string &nomAtributo,
                        LiteralVector_Zero * v,
                        ListaRefExternas &r,
                        bool paraVector)
/**
 * @param nomAtributo Nombre del atributo dest. / nombre vector dest.
 * @param paraVector Quizás no se va guardar en atributo, si no en vector
 * @param v El vector a escribir en el listado de mnemotécnicos
 * @param r Las refs externas a escribir
 */
{
    bool eraVector;
    bool esPrimitivo;
    const std::string *nombre = NULL;
    Mnemotecnico *mnemo;
    Objeto *obj;
    std::string nombreVector;

	try {
		// Crear el vector
		// MSG String zero
		mnemo = new NMMsg( Runtime::objetoCadena->getNombreCompleto(), "zero" );
		mnemosObjeto.push_back( mnemo );

		// MSG VectorInstance copy ""
		mnemo = new NMMsg( Runtime::objetoVector->getNombreCompleto(), MET_COPY );
		( (NMMsg *) mnemo )->masArgumentos( NombreRegistro::__ACC );
		mnemosObjeto.push_back( mnemo );

		// Crear el nombre del vector, si es necesario
		if ( paraVector ) {
			nombreVector = getSigNombreVectorAux();
			mnemo = new NMDef( nombreVector );
			mnemosObjeto.push_back( mnemo );

			mnemo = new NMAsg( nombreVector, NombreRegistro::__ACC );
			mnemosObjeto.push_back( mnemo );
		}
		else nombreVector = nomAtributo;

		// Asignárselo al atributo o al vector.
		if ( !paraVector ) {
			// ASG ${obj}
			mnemo = new NMAsg( nomAtributo, NombreRegistro::__ACC );
			mnemosObjeto.push_back( mnemo );
		} else {
			// MSG ${obj} add __acc
			mnemo = new NMMsg( nomAtributo, MET_INSERTAR );

			( (NMMsg *) mnemo )->masArgumentos( NombreRegistro::__ACC );
			mnemosObjeto.push_back( mnemo );
		}


		// Introducir los elementos en el vector
		for(register unsigned int i = 0; i < v->longitud(); ++i)
		{
			eraVector = false;

			// Meterlo como referencia externa
			obj = v->getElemento( i )->getObjeto();

			// Conseguir nombre y guardar ref ext
			esPrimitivo = Runtime::esObjetoPrimitivo( obj );

			if ( !esPrimitivo
			&& esObjetoProximo( obj ) )
			{
				r.push_back( obj );
				nombre = &( obj->getNombre() );
			}
			else
			if ( esPrimitivo ) {
				generaMnemosPrimitivo( nombreVector, obj, r, eraVector, true );
			}
			else {
					if ( obj->getPerteneceA() == this->getPerteneceA() )
							nombre = &( obj->getNombre() );
					else    nombre = &( obj->getNombreCompleto() );
			}

			// Mnemo para método alCargar: SET <nombre_obj>
			if ( !esPrimitivo ) {
				mnemo = new NMSet( NombreRegistro::__ACC, *nombre );

				mnemosObjeto.push_back( mnemo );
			}

			if ( !eraVector ) {
				// Mnemo para método alCargar: MSG v add __acc
				mnemo = new NMMsg( nombreVector, MET_INSERTAR );

				((NMMsg *) mnemo )->masArgumentos( NombreRegistro::__ACC );
				mnemosObjeto.push_back( mnemo );
			}
		}
	} catch(const std::bad_alloc &e) {
		throw ENoHayMemoria( "guardando vector" );
	}
	catch(...) {
		throw EInterno( "guardando vector" );
	}

    return;
}

// -------------------------------------------- Objeto::generaMnemosPrimitivo()
void Objeto::generaMnemosPrimitivo(
        const std::string &nombre,
        Objeto *obj,
        ListaRefExternas &r,
        bool &eraVector,
        bool paraVector)
/**
 * genera los mnemotécnicos necesarios para un objeto primitivo,
 * guardándolos en this->mnemosObjeto
 * @param nombre nombre de atributo dest./vector dest. para el vector
 * @param obj El objeto para el cual se generan los mnemotécnicos
 * @param r colocar ahí las refs externas, si se encuentran (sólo vector)
 * @param eraVector true si el primitivo era vector, falso en otro caso
 */
{
    Mnemotecnico * mnemo;
    INT32  * ent           = NULL;
    REAL  * flt            = NULL;
    std::string * str           = NULL;
    LiteralVector_Zero * v = NULL;

    eraVector = false;

    if ( ( ent = Runtime::gestorNumerosEnteros->busca( obj ) ) != NULL )
    {
        mnemo = new(std::nothrow) NMInt( *ent );

        if ( mnemo != NULL )
                mnemosObjeto.push_back( mnemo );
        else    throw ENoHayMemoria( "guardando entero" );
    }
    else
    if ( ( flt = Runtime::gestorNumerosFlotantes->busca( obj ) ) != NULL )
    {
        mnemo = new(std::nothrow) NMFlt( *flt );

        if ( mnemo != NULL )
                mnemosObjeto.push_back( mnemo );
        else    throw ENoHayMemoria( "guardando flotante" );
    }
    else
    if ( ( str = Runtime::gestorCadenas->busca( obj ) ) != NULL )
    {
        mnemo = new(std::nothrow) NMStr( *str );

        if ( mnemo != NULL )
                mnemosObjeto.push_back( mnemo );
        else    throw ENoHayMemoria( "guardando cadena" );
    }
    else
    if ( ( v = Runtime::gestorVectores->busca( obj ) ) != NULL )
    {
        eraVector = true;
        listaMnemosCargarVector( nombre, v, r, paraVector );
    }
}

// ---------------------------------------- Objeto::listaMnemosMetodoAlCargar()
void Objeto::listaMnemosMetodoAlCargar(
                        const ListaAtributos &atrsPrimitivos,
                        ListaRefExternas &r)
/**
 * genera los mnemotécnicos necesarios para el método de preejecución, que asigna
   los valores correctos para los atributos.
 * Añade los mnemotécnicos del método de carga a la lista de mnemotécnicos
 * de este objeto.
 * @param atrsPrimitivos aquellos atributos que señalan a valores primitivos
*/
{
    Mnemotecnico * mnemo;
    Atributo *atr          = NULL;
    Objeto *obj            = NULL;
    register unsigned int numAtrs = atrsPrimitivos.getNumero();
    bool eraVector;

    try {
      if ( numAtrs > 0 ) {
        // Para el método alCargar
        numVectoresAux = 0;

        // Crear el método de atributos primitivos
        mnemosObjeto.push_back( new NMMth( NMMth::PUBLICO, MET_ALCARGAR ) );

        for(register unsigned int i = 0; i < numAtrs; ++i)
        {
            // ¿Qué tipo de valor primitivo es señalado?
            atr = atrsPrimitivos.getElementoNumero( i );
            obj = atr->getObjeto();

            generaMnemosPrimitivo( atr->getNombre(), obj, r, eraVector, false );

            // ¿A qué atributo? (si no es un vector)
            if ( !eraVector ) {
                mnemo = new NMAsg( atr->getNombre(), NombreRegistro::__ACC );
                mnemosObjeto.push_back(  mnemo );
            }
        }

        mnemosObjeto.push_back( new NMEnm() );
      }
    } catch(const std::bad_alloc &) {
        throw ENoHayMemoria( "creando lista opcodes" );
    }

    return;
}

// ------------------------------------------------ Objeto::guardaPropiedades()
void Objeto::guardaPropiedades(const GestorPropiedades &gProps)
/**
 * Guarda las propiedades, bien de un método, bien de un objeto
 * @param gProps el gestor de propiedades de donde guardarlas
*/
{
    GestorPropiedades::ListaPropiedadesIniciales::const_iterator it =
            gProps.getPropsIniciales().begin()
    ;

    for(; it != gProps.getPropsIniciales().end(); ++it) {
        mnemosObjeto.push_back( *it );
    }
}

// --------------------------------------------- Objeto::listaMnemosDeMetodos()
void Objeto::listaMnemosDeMetodos(ListaRefExternas &r)
/**
 * listaMnemosDeMetodos()
 * Compila la lista de mnemotécnicos para todos los métodos de este objeto
 *
 * @param r lista de referencias externas, a llenar si hay obj ext próximos
 * @param l lista de mnemoténicos que componen el objeto, a rellenar
 */
{
    NMMth * mth;
    Mnemotecnico *mnemo;
    Objeto *objExt;
    Referencia *refExt;
    Metodo * met;

    // Guardar los métodos
    try {
      for(register unsigned int i = 0; i < lMets.getNumero(); ++i)
      {
        // El método en cuestión
        met = lMets.getElementoNumero( i );

        // Cabecera del método
        mth = new NMMth(
                met->esPublico() ?
                                NMMth::PUBLICO
                              : NMMth::PRIVADO,
                met->getNombre()
        );

        mth->copiarArgumentos( *( mth->getArgs() ), met->argumentosFormales );

        mnemosObjeto.push_back( mth );

        // Sus propiedades
        guardaPropiedades( met->getGestorPropiedades() );

        // Meter el cuerpo del método
        Metodo::IterMet it    = met->getInstrucciones();
        Metodo::IterMet final = met->getFinMet();

        while( it != final ) {
                mnemo  = (*it)->getMnemo();

                // Guardarlo
                if ( it == met->getComienzoMet() ) {
                    Mnemotecnico * mnemo =
                            new NMMta( NMMta::FinalDeclConstantes, "" )
                    ;

                    mnemosObjeto.push_back( mnemo );
                }

                mnemosObjeto.push_back( mnemo );

                // Meter referencias externas (SET y MSG)
                if ( dynamic_cast<NMSet *>( mnemo ) != NULL ) {
                        refExt = ( (OcSET *) *it)->getReferencia();

                        if ( refExt != NULL
                          && refExt->swizzled() )
                        {
                            objExt = refExt->getObjeto();

                            if ( esObjetoProximo( objExt ) )
                            {
                                 r.push_back( objExt );
                            }
                        }
                }
                else
                if ( dynamic_cast<NMMsg *>( mnemo ) != NULL ) {
                    // Necesito recuperar la referencia al objeto del MSG
                    // ¿Está siwzzled?
                    refExt = ( (OcMSG *) *it)->getReferencia();

                    if ( refExt != NULL
                      && refExt->swizzled() )
                    {
                        objExt = refExt->getObjeto();

                        if ( esObjetoProximo( objExt ) )
                        {
                             r.push_back( objExt );
                        }
                    }
                }

                ++it;
        }

        // Pie de método
        mnemosObjeto.push_back( new NMEnm() );
      }
    } catch(...) {
        throw ENoHayMemoria( "creando lista opcodes métodos" );
    }

    return;
}

void Objeto::listaMnemosDeAtributos(
                                    Objeto::ListaAtributos &la,
                                    Objeto::ListaRefExternas &r)
/**
 * listaMnemosDeAtributos()
 * Guarda en la lista de Mnemos del objeto los mnemos relativos
 * a sus atributos.
 * Añade los objetos externos próximos en la lista (param) r
 *
 * @param r La lista de referencias externas, i.e., objetos próximos
 * @param la La lista de atributos a producir nombre/objeto*
*/
{
    const std::string *nombreObjeto;
    Atributo *atrOrg;
    NMAtr * atr;
    Objeto * objExt;
    ListaAtributos objsPrimitivos;
    AlmacenObjetos * libCont = Runtime::rt()->getContainerIntStdLib();

    for(register unsigned i = 0; i < lAtrs.getNumero(); ++i)
    {
        objExt = NULL;
        atrOrg = lAtrs.getElementoNumero( i );

        // Si está swizzled, entonces procesarlo
        if ( atrOrg->swizzled() ) {
            objExt = atrOrg->getObjeto();
            if ( esObjetoTemporal( objExt )
              || objExt->getPerteneceA() == getPerteneceA()
              || objExt->getPerteneceA() == libCont )
            {
                 nombreObjeto = &( objExt->getNombre() );
            }
            else nombreObjeto  = &( objExt->getNombreCompleto() );
        }
        else nombreObjeto = &( atrOrg->getNombreObjetoReferenciado() );

        // Si es un objeto de sistema, no procesar en cualquier caso
        if ( dynamic_cast<ObjetoSistema *>( objExt ) != NULL ) {
            continue;
        }

        // Guardar el atributo, si no es el padre
        if ( dynamic_cast<AtributoParent *>( atrOrg ) == NULL ) {

            atr = new(std::nothrow) NMAtr(
                atrOrg->esPublico() ? NMAtr::PUBLICO : NMAtr::PRIVADO,
                atrOrg->getNombre(),
                *nombreObjeto )
            ;

            if ( atr != NULL ) {
                mnemosObjeto.push_back( atr );
            } else throw ENoHayMemoria( "guardando container" );
        }

        // Añadirlo a la lista de referencias externas,
        // o ponerlo como primitivo
        // ObjsPrimitivos
        //      Si está en cont., Exe, NULL u otro transient (esProximo)
        //      y está entre los gestores de objs primitivos
        // RefsExternas
        //      Si esProximo y no es primitivo
        // Cuidado: si accedemos a él podemos provocar la carga
        // de otro container
        if ( objExt != NULL ) {
            if ( esObjetoTemporal( objExt ) )
            {
                // Si es primitivo, hay que tratarlo especialmente,
                // en otro caso, será procesado (no hacer nada)
                if ( Runtime::esObjetoPrimitivo( objExt ) )
                        la.inserta( atrOrg );
                else    r.push_back( objExt );
            }
        }
    }

    return;
}

// ----------------------------------------------------------- Objeto::salvar()
Objeto::ListaMnemosObjeto &Objeto::salvar(ListaRefExternas &r)
/**
 * genera los mnemotécnicos necesarios para guardar una copia exacta del objeto
 * en disco
 * @param r rellenar con las referencias externas desde este objeto
 * @return una lista de mnemotécnicos equivalentes a todo el objeto
 */
{
    std::vector<std::string> nomAtrs;
    ListaAtributos objsPrimitivos( ListaAtributos::NO_PROPIETARIO );

    if ( dynamic_cast<ObjetoSistema *>( this ) == NULL )
    {
		if ( fueModificadoDesdeCarga()
          || mnemosObjeto.empty() )
		{
			eliminarMnemosObjeto();

			// Cabecera
			NMObj *cabecera = new(std::nothrow) NMObj(
							getNombre(),
							getAtrObjetoPadre()->getNombreObjetoReferenciado()
			);

			if ( cabecera != NULL ) {
					mnemosObjeto.push_back( cabecera );
			}
			else throw ENoHayMemoria( "creando cabecera al guardar objeto" );

			// Guardar las propiedades
			guardaPropiedades( gProps );

			// Guardar los atributos
			listaMnemosDeAtributos( objsPrimitivos, r );

			// Crear la lista de métodos
			listaMnemosDeMetodos( r );

			// Crear el método "al cargar"
			listaMnemosMetodoAlCargar( objsPrimitivos, r );

			// Fin de objeto
			Mnemotecnico * mnemo = new(std::nothrow) NMEno();

			if ( mnemo != NULL )
					mnemosObjeto.push_back( mnemo );
			else    throw ENoHayMemoria( "creando fin de objeto al guardar objeto" );

			// Prepararlo para otra
			ponNoModificadoDesdeCarga();
			refsExtObjeto.clear();
			refsExtObjeto.insert( refsExtObjeto.begin(), r.begin(), r.end() );
    	} else {
			// No es necesario repetir el proceso
			r.insert( r.end(), refsExtObjeto.begin(), refsExtObjeto.end() );
		}
    }

    return mnemosObjeto;
}

void Objeto::eliminarMnemosObjeto()
/**
        Elimina los mnemos del objeto.
        En general, se eliminan todos, menos:
        <ul>
                <li>Los meta DE USUARIO (el resto se borran siempre)
                <li>Los mnemos de los métodos (si no es el método de carga)
        </ul>
*/
{
    bool borrar = true;
    ListaMnemosObjeto::const_iterator it = mnemosObjeto.begin();

    for( ;it != mnemosObjeto.end(); ++it) {
        // Las propiedades de usuario, no tocarlas
        if ( dynamic_cast<NMMta*>( *it ) != NULL ) {
                if ( ( (NMMta*) (*it) )->getPragma() == NMMta::Objeto ) {
                        continue;
                }
        }

        // ¿Estamos dentro de un método?
        if ( dynamic_cast<CtrMt*>( *it ) != NULL ) {
            if ( dynamic_cast<NMMth*>( *it ) != NULL ) {
                if ( ( (NMMth *) ( *it ) )->getNombre() != MET_ALCARGAR ) {
                    borrar = false;
                    delete *it;
                    continue;
                }
            }
            else
            if ( dynamic_cast<NMEnm*>( *it ) != NULL ) {
                borrar = true;
            }

        }

        // Borrar
        if ( borrar
          || dynamic_cast<NMMta*>( *it ) != NULL )
        {
            delete *it;
        }
    }

    mnemosObjeto.clear();
}

} // namespace Zero


// metodos.cpp
/*
        Implantación de los métodos

        Pertenece a zvm

        jbgarcia@uvigo.es
*/

#include <cstring>

#include "metodos.h"
#include "uintf.h"

namespace Zero {

// ================================ Metodo =====================================
std::vector<Referencia *> Metodo::argumentosNulos = std::vector<Referencia *>();

// ------------------------------------------------------------ Metodo::Metodo()
Metodo::Metodo(const std::string &n, Objeto *rthis,
	       const MixinConArgumentos::Argumentos &args, Acceso ac)
        : _enExcepcion(false), enFase(true), instr(NULL), tipoAcceso(ac),
            nombre(n), objetoAlQuePertenece(rthis),
            numLlamadasRecursivas(0), despuesLiterales(-1),
            excep(NULL), finMet(NULL), recursivo(NULL), metodoAnterior(NULL),
            herenciaDinamica(false)
{
        // Inicializar registros
        for(register unsigned int i = 0; i < NombreRegistro::numRegistros; ++i) {
            reg[i] = NULL;
        }

        // Guardar los parámetros formales para cuando se produzcan llamadas
        MixinConArgumentos::copiarArgumentos( argumentosFormales, args );

        // Poder identificar si es el método de herencia dinámica
        if ( nombre == MET_HERDINAMICA ) {
            herenciaDinamica = true;
        }
}

Metodo::~Metodo()
/**
	Elimina las variables locales, en caso de existir alguna
	y especialmente, elimina el cuerpo del método
*/
{
	// Eliminar el cuerpo del método
    inicializar();

	// Eliminar los argumentos formales
	MixinConArgumentos::Argumentos::iterator it = argumentosFormales.begin();
	for (; it != argumentosFormales.end(); ++it ) {
		delete *it;
	}

	printf( "\n***ERASE '%s'***\n", getNombre().c_str() );

}

// --------------------------------------------------------- Metodo::inicializar
void Metodo::inicializarEstado()
{
        // Poner los registros como NULL
        std::memset( reg, 0, sizeof( Referencia * ) * NombreRegistro::numRegistros );

        // Eliminar las variables locales
        vblesLocales.eliminaTodos();

        // No hay cuerpo ni sección de excepciones [por ahora]
        // Y el metodo no se esta ejecutando
        despuesLiterales = -1;
}

void Metodo::inicializarComportamiento()
{
        // Borrar las instrucciones del método (si existen)
        if ( !esRecursivo() ) {
            if ( instr != NULL ) {
                    for (IterMet op = instr; *op != NULL; ++op) {
                            delete *op;
                    }
            }

            delete[] instr;
        }

        instr = ip = excep = finMet = NULL;
}

// --------------------------------------------- MetodoMV::duplicaPorRecursivo()
Metodo * Metodo::duplicaPorRecursivo(Metodo * met)
{
	Metodo * toret = met->copia( met->getPerteneceA() );

	if ( toret != NULL ) {
		// Marcar como recursivo
		toret->recursivo = met;
		toret->metodoAnterior = Runtime::gestorStack->getMetodoActual();

		if ( toret->metodoAnterior == NULL ) {
					        printf( "*** ERROR GORDO ***, Set: el metodo anterior es: %p\n", met );
					    }

		toret->nombre = toret->getNombre()
                    + '\''
                    + AnalizadorLexico::toString(
                        met->getNumLlamadasRecursivas() + 1
                    )
        ;

		// Insertarlo como un nuevo método
		met->getPerteneceA()->lMets.inserta( toret->getNombre(), toret );

		met->anotaNuevoRecursivo();
	}
	else {
		Runtime::ponExcepcion( Runtime::objetoExcepcion,
		                       EXC_NOMEM )
		;

		toret = NULL;
	}

printf( "*** CLONE: mth org: '%s' (%p) rec=%p| Copiado en: '%s' (%p) rec=%p|Met anterior: %s (%p)\n",
                met->getNombre().c_str(),
                met,
                met->recursivo,
                toret->getNombre().c_str(),
                toret,
                toret->recursivo,
                toret->metodoAnterior->getNombre().c_str(),
                toret->metodoAnterior
);
	return toret;
}

// -------------------------------------------------------- Metodo::buscaMetodo
Metodo * Metodo::buscaMetodo(Objeto *objetoPartida, const std::string &nombreMet)
{
        Objeto *obj = objetoPartida;

        // Búsqueda del método, desde el objeto de inicio hasta "Object"
        Metodo *toret = obj->lMets.busca( nombreMet );

        while ( obj != NULL
             && toret == NULL
             && obj != Runtime::objetoRaiz )
        {
                if ( obj->getAtrObjetoPadre() != NULL )
                        obj = ( obj->getAtrObjetoPadre() )->getObjeto();
                else    obj = NULL;

                toret = obj->lMets.busca( nombreMet );
        }

        return toret;
}

// ------------------------------------------------------------- Metodo::copia()
void Metodo::copiaEstado(Metodo * org, Metodo * dest)
/**
    Copia las variables locales, de manera que queden igual
    que en el mth original
*/
{
    Referencia * nuevaRef;
    Objeto * perteneceA = org->getPerteneceA();

    // Crear referencias -- si es literal, mantener el objeto literal
    for(unsigned int i = 0; i < org->vblesLocales.getNumero(); ++i) {
        Referencia * ref = org->vblesLocales.getElementoNumero( i );
        const std::string & nombre = org->vblesLocales.getNombreElementoNumero( i );

        // Si es una referencia constante, mantener el objeto apuntado
        if ( dynamic_cast<ReferenciaConstante *>( ref ) != NULL ) {
            nuevaRef = new ReferenciaConstante( perteneceA, dest, nombre );
            nuevaRef->ponReferencia( ref->getObjeto() );
        } else {
            nuevaRef = new Referencia(
                            perteneceA, dest, nombre, Runtime::objetoNulo )
            ;
        }

        // Crear la referencia
        dest->vblesLocales.inserta( nuevaRef->getNombre(), nuevaRef );
    }

    // Colocar registros
    for(register unsigned int i = 0; i < NombreRegistro::numRegistros; ++i)
	{
		dest->reg[i] = dest->vblesLocales.busca(
                    NombreRegistro::cnvtCodigoRegistroANombre(
                        (NombreRegistro::CodigoRegistro) i )
                 )
        ;
	}
}

Metodo * Metodo::copia(Objeto *obj)
/**
    Crea un nuevo mth que es copia del mth actual.
    Se comparten los opcodes
*/
{
        Metodo *met = NULL;

        try {
            met = new Metodo( getNombre(),
                    obj,
                    argumentosFormales,
                    tipoAcceso )
            ;

            // Compartir el cuerpo de instrucciones
            met->inicializarEstado();
            Metodo::copiaEstado( this, met );
            met->instr = instr;         // Compartir el código
            met->finMet = finMet;
            met->excep = excep;
            met->comienzo = instr;
            met->ip = NULL;
            met->despuesLiterales = despuesLiterales;
            met->comienzo = comienzo;
            this->deSincroniza( met );

            return met;
        } catch(std::bad_alloc &)
        {
                Runtime::ponExcepcion (Runtime::objetoExcepcion, EXC_CPYERR
		                                                  + '.' + ' '
								  + EXC_NOMEM )
		;
                return NULL;
        }
}

// -------------------------------------------------- Metodo::masInstrucciones()
void Metodo::masInstrucciones(Compilador::ListaMnemos &mnemos)
{
        // Preparar el nuevo cuerpo del método, si es necesario
        if ( instr == NULL
          || getNumInstrucciones() < mnemos.size() )
        {
            inicializar();

            // Es necesario meter un NULL al final, para saber donde termina
            instr = new(std::nothrow) Opcode *[ mnemos.size() +1 ];

            if ( instr == NULL  ) {
                    throw ENoHayMemoria( "creando cuerpo del met." );
            }
        } else {
            inicializarEstado();
        }

        // Ajustar límites
        const unsigned int numMnemos = mnemos.size();
        unsigned int numOpsCuerpoMetodo = numMnemos;
        finMet  = instr + numOpsCuerpoMetodo;

        try {
            // Cargar los opcodes
            IterMet tope = instr;
            for(register unsigned int i = 0; i < numMnemos; ++i) {

                // Es un META? (describe propiedades del método)
                if ( dynamic_cast<NMMta *>( mnemos[i] ) != NULL ) {
                    if ( ((NMMta *) mnemos[i])->getPragma() == NMMta::Objeto ) {
                        // Es una propiedad a insertar
                        masPropiedades( (NMMta *) mnemos[i] );
                    } else {
                        procPragma( (NMMta *) mnemos[i], i );
                    }

                    --numOpsCuerpoMetodo;
                    --finMet;
                    delete mnemos[i];
                    mnemos[i] = NULL;
                    continue;
                }

                // Crear el Opcode correspondiente
                Opcode *op = Opcode::creaOpcodeParaMnemo( mnemos[i], this );

                // Liberar el mnemo
                delete mnemos[i];

                // Anadir la instruccion
                *( tope ++ ) = op;

                // Si no es un DEF, y es el primero, es el fin de los DEF's
                if ( despuesLiterales == -1
                  && ( dynamic_cast<OcDEF *>( op ) == NULL ) )
                {
                        despuesLiterales = i;
                }

                // Si es un RET, despues irá la seccion de excepciones
                if ( ( dynamic_cast<OcRET *>( op ) != NULL )
                 &&    excep == NULL )
                {
                        excep = tope;
                }
            }
        } catch(const Excepcion &)
        {
                delete[] instr;
                inicializar();
                throw;
        }

        // Comprobar
        *finMet = NULL;
        chk();
}

// --------------------------------------------------------------- Metodo::chk()
void Metodo::chk()
/**
	Este método es llamado por masInstrucc() una vez que se ha llenado
        el método de instrucciones.
	Lo que hace es comprobar que:
	<ul>
		<li>Los saltos van adentro de su sección
		<li>Los saltos no son de valor 0
 *              <li>Poner ya la dirección concreta del salto, menos uno,
 *                  puesto que en cuanto se completa una instrucción
 *                  (un salto, por ejemplo), se incrementa el ip.
	</ul>
*/
{
    // Hacer los ajustes
    IterMet inicio  = instr;
    IterMet final   = excep;
    IterMet nuevoIP;
    IterMet it;

    for(it = inicio; it != finMet; ++it)
    {
        // En qué parte del cuerpo estamos ?
        if ( it == final )
        {
            inicio = excep;
            final  = finMet;
        }

        // Esté donde esté, ¿está dentro?
        if ( dynamic_cast<OcFlw *>( *it ) != NULL )
        {
            nuevoIP = it + ((OcFlw *) *it)->getMnemo()->getNumero();

            if ( nuevoIP  <  inicio
                || nuevoIP  >= final
                || nuevoIP  == it )
            {
                    // Cosa mala ...
                    throw EChkSaltoInvalido(
                            (*it)->getMnemo()->listar( false ).c_str()
                    );
            }
            else ((OcFlw*) (*it))->ponDirDirecta( nuevoIP - 1 );
        }
    }

    notificar( Observador::CambioEnElemento );
}

// ------------------------------------------- Metodo::preparaVariablesLocales()
void Metodo::preparaVariablesLocales(Objeto *rthis)
{
    // Borrar todas las variables
    vblesLocales.eliminaTodos();

    // Crear los registros y meterlos en las variables locales
	for(register unsigned int i = 0; i < NombreRegistro::numRegistros; ++i)
	{
		reg[i] = new(std::nothrow) Referencia(
				rthis,
				this,
				NombreRegistro::cnvtCodigoRegistroANombre(
					(NombreRegistro::CodigoRegistro) i ),
				Runtime::objetoNulo )
		;

		// Comprobar memoria
		if ( reg[i] == NULL ) {
		    throw ENoHayMemoria( std::string( "preparando método " + getNombre() ).c_str() );
		}

		// Insertar en variables locales
		if ( !vblesLocales.inserta(
			NombreRegistro::cnvtCodigoRegistroANombre(
				    	(NombreRegistro::CodigoRegistro) i ),
			reg[i] ) )
		{
			throw ENoHayMemoria(
				std::string( "preparando método " + getNombre() ).c_str()
			);
		}
	}

	// Preparar la referencia a this
	reg[NombreRegistro::__THIS]->ponReferencia( rthis );

	return;
}

// ------------------------------------------------------ Metodo::finEjecucion()
void Metodo::finEjecucion()
{
        // No se está ejecutando
        ip = NULL;

        // Pon lugar en el gestor de de stack (para excepciones) ret to ?
        if ( !enExcepcion() ) {
                Runtime::gestorStack->ret( getLocThis()->getObjeto(), this );
        }

        // Si es un mth auxiliar (debido a llamada recursiva), borrarlo
        if ( !esRecursivo() ) {
            // "Eliminar" todas las referencias, poniéndolas a "Nothing"
            // Las referencias constantes no cambian, en realidad
            for(register unsigned int i = 0; i < vblesLocales.getNumero(); ++i)
            {
                vblesLocales.getElementoNumero( i )
                                        ->ponReferencia( Runtime::objetoNulo )
                ;
            }

            // Que los opcodes recalculen sus referencias
            deSincroniza();
        }

        return;
}

// ------------------------------------------------ Metodo::gestionExcepciones()
void Metodo::gestionExcepciones()
/**
 * Si se ha producido una excepción ponemos el final
 * del método al final del gestor de excepciones, y el ip
 * al comienzo del gestor.
 * Si ya estábamos en el gestor, lo ponemos a su final para
 * que salga.
 * Siempre que modifiquemos el ip hay que ponerlo a -1, puesto que
 * lo siguiente que se hace siempre es incrementar la ip.
 * @param final Final de método, bien del cuerpo o del gestor excepciones
 */
{
    // Estamos en el cuerpo del método ?
    if ( ejecutandoSeccPpal() )
    {

        // Hay sección de gestión de excepciones ?
        if ( tieneGestorExcepciones() )
        {
            Objeto *excepcion = Runtime::gestorExcepciones->getExcepcion();

            // "Colocar" la excepción en el método en ejecución
            getLocAcc()->ponReferencia( excepcion );
            getLocExc()->ponReferencia( excepcion );
        }
		else _enExcepcion = true;

        // Ahora el final es fin secc excepciones
        final = finMet;

        // Ir al principio sección excepciones
        ip    = excep - 1;
    }
    else
    // Exception on exception, salir de método
    if ( ejecutandoSeccExcp() ) {
        ip    = finMet - 1;
		_enExcepcion = true;
    }
    else {
        std::string buffer = "ip=";
        buffer += AnalizadorLexico::toString( ip );
        buffer += " ini=";
        buffer += AnalizadorLexico::toString( comienzo );
        buffer += " exc=";
        buffer += AnalizadorLexico::toString( excep );
        buffer += " end=";
        buffer += AnalizadorLexico::toString( finMet );

        throw EEjecucion( std::string( ("IP ilegal: '" + getNombre() + "', " ) + buffer ).c_str() );
    }
}

// -------------------------------------------- Metodo::optimizacionesLocales()
inline
void Metodo::preparaEstadoMetodo(Objeto * rthis)
/**
	Este método se encarga de, si es la primera vez que se ejecuta el método,
	llamar a preparaVariablesLocales() para crear los registros ... etc.
	También, de existir un .CONSTEND de ejecutar la inic. de constantes.
	En otro caso, sólo prepara el registro THIS.
*/
{
    if ( vblesLocales.getNumero() == 0 ) {
        comienzo = instr;
        preparaVariablesLocales( rthis );

        if ( despuesLiterales != -1 ) {
            const IterMet constEnd = instr + despuesLiterales;

            // Ejecución controlada hasta final de constantes
            for( ip = instr; ip < constEnd && !enExcepcion(); ++ip) {
                (*ip)->ejecuta();
            }

            // Se comienza a ejecutar después de los literales
            comienzo         = constEnd;
            despuesLiterales = -1;
        }
    }
    else reg[ NombreRegistro::__THIS ]->ponReferencia( rthis );
}


/*static std::string getObjetoAsString(Objeto * obj);
static std::string getInstr(Opcode *op)
{
    Metodo * met = op->getPerteneceA();
    Objeto * obj = met->getPerteneceA();
    std::string toret = obj->getNombre();

    toret += '.';
    toret += met->getNombre();
    toret += '(';
    toret += ')';
    toret += ':';
    toret += ' ';
    toret += op->getMnemo()->listar(false);

    return toret;
}

static std::string getObjetoAsString(Objeto * obj)
{
    std::string toret;

    if ( Runtime::esObjetoPrimitivo( obj ) ) {
        if ( Runtime::gestorCadenas->busca( obj ) != NULL ) {
            toret += ' '; toret += '=';
            toret += *( Runtime::gestorCadenas->busca( obj ) );
        }
        if ( Runtime::gestorNumerosEnteros->busca( obj ) != NULL ) {
            toret += ' '; toret += '=';
            toret += AnalizadorLexico::toString( *( Runtime::gestorNumerosEnteros->busca( obj )) );
        }
        if ( Runtime::gestorNumerosFlotantes->busca( obj ) != NULL ) {
            toret += ' '; toret += '=';
            toret += AnalizadorLexico::toString( *( Runtime::gestorNumerosFlotantes->busca( obj )), 10, 2 );
        }
        if ( Runtime::gestorVectores->busca( obj ) != NULL ) {
            LiteralVector_Zero * vector = Runtime::gestorVectores->busca( obj );

            toret += ' ';
            toret += '[';
            printf( " =[ " );
            for(size_t i = 0; i < vector->longitud(); ++i) {
                toret += getObjetoAsString( vector->getElemento( i )->getObjeto() );
                toret += ' ';
            }
            toret += ']';
        }
    }
    else
    if ( obj == Runtime::objetoNulo ) {
        toret += " = Nothing";
    }
    else toret += AnalizadorLexico::toString( obj );

    return toret;
}
*/

// ----------------------------------------------------------- Metodo::ejecuta()
Objeto * Metodo::ejecuta(Argumentos * args, Objeto * rthis)
{
	Objeto *toret;

    // Actualizar el stack
    Runtime::gestorStack->call( rthis, this );

	// Si se está ejecutando ya, es una llamada recursiva (imposible).
	if ( ejecutando() ) {
		Runtime::ponExcepcion( Runtime::objetoExcepcion, EXC_REC );
		toret = NULL;
		goto FIN;
	}

	_enExcepcion = false;

    // Asegurarse de que todos los opcodes van a actualizar sus referencias
    deSincroniza();

    // Preparar final(seccPpal o seccExcep)
    if ( excep == NULL ) {
        excep = finMet;
    }
    final = excep;

    // Preparar las variables locales (cuando sea necesario)
    preparaEstadoMetodo( rthis );

    // Comienzo de ejecucion
    ip = comienzo;

    // Preparar los argumentos
    if ( !convertirArgumentosAFormales( args ) ) {
        Runtime::ponExcepcion( Runtime::objetoExcepcion, EXC_ARGNUMBER );
    }

    // Ejecutar
    #ifdef _UI_DEBUG
    iuComienzaMetodo( rthis, this );
    #endif

    for(; ip != final; ++ip)
    {
        // Avisar a la interfaz
        #ifdef _UI_DEBUG
        iuPasoMetodo( rthis, this, ip - instr );
        #endif

        // Ejecutar la instrucción
        ( *ip )->ejecuta();
    }

    // Resultado de la ejecución
    toret = reg[ NombreRegistro::__RR ]->getObjeto();

    #ifdef _UI_DEBUG
    iuFinMetodo( rthis, this, ip - instr, toret );
    #endif

    FIN:
    return toret;
}

// ------------------------------------------------------ Metodo::deSincroniza()
inline
void Metodo::deSincroniza(Metodo * met)
/**
        Se trata de obligar a todos los opcodes a recalcular sus referencias,
        por ejemplo, tras la eliminación de un atributo.

        Si se llama para que cambien tambien su mth, asegurarse de no llamarlo
        con this->deSincroniza( this ), ya que no es correcto.
*/
{
        // Desincronizar todos los opcodes
        if ( met != this
          && met != NULL )
        {
            for(register IterMet op = instr; *op != NULL; ++op) {
                ( *op )->cambiaEstadoMetodo( met );
            }
        } else {
            if ( (*instr)->getFase() == getFase() ) {
                enFase = !enFase;
            }
        }
}

// -------------------------------------- Metodo::convertirArgumentosAFormales()
bool Metodo::convertirArgumentosAFormales(Argumentos *args)
{
    register Objeto * objArg;
    Referencia * ref;
    register Referencia * vbleLocal;
    bool toret = true;
    const std::string * nomArg;

    // Primero, comprobar los argumentos
    if ( args->size() != argumentosFormales.size() ) {
        toret = false;
    } else {
        // Ir metiendo en vbles locales los args, nombre cambiado
        for(register size_t i = 0; i < args->size(); ++i) {

            nomArg    = &( argumentosFormales[ i ]->getNombre() );
            vbleLocal = vblesLocales.busca( *nomArg );
            objArg    = ( *args )[ i ]->getObjeto();

            if ( vbleLocal == NULL ) {
                ref = new(std::nothrow) Referencia(
                        getPerteneceA(),
                        this,
                        *nomArg,
                        objArg
                );

                if ( ref != NULL ) {
                    vblesLocales.inserta( *( nomArg ), ref );
                } else {
                    Runtime::ponExcepcion( Runtime::objetoExcepcion,
                                    EXC_NOMEM )
                    ;
                    break;
                }
            } else vbleLocal->ponReferencia( objArg );
        }
    }

    return toret;
}


// -------------------------------------------------- Metodo::getListaOpcodes()
const std::string &Metodo::getListaOpcodes()
{
	register IterMet it = instr;

	listaOpcodes.clear();

	while( it != finMet )
	{
		listaOpcodes += (*it)->getMnemo()->listar();
		listaOpcodes += '\n';

		++it;
	}

	return listaOpcodes;
}

// --------------------------------------------------------- Metodo::getProps()
/**
 * Metodo::getProps()
 *
 * Obtiene la lista de propiedades de este método.
 * En el caso de que no existan todavía, las crea (puede que sea lista vacía)
*/
Objeto *Metodo::getProps()
{
	if ( !gProps.existeObjetoProps() ) {
		gProps.ponNombre(
			getPerteneceA()->getNombre()
			+ "__"
			+ getNombre()
			+ "__"
		);
	}

	return gProps.getProps();
}

// -------------------------------------------------------- Metodo::procPragma()
void Metodo::procPragma(NMMta *mta, unsigned int numInstrucciones)
/**
 * Los pragmas como .CONSTEND son procesados por esta función.
 * Se trata de pragmas que afectan a la manera de trabajar del método,
 * pragmas "internos".
 * @param mta El meta a procesar
 * @param numInstrucciones el número de instrucciones de este método
 * 				ya introducidas.
 */
{
	if ( mta->getPragma() == NMMta::FinalDeclConstantes ) {
        despuesLiterales = numInstrucciones;
	}
	else throw EChkMetaInvalido( "pragma interno no soportado" );
}

// ================================ MetodoMV ===================================
// -------------------------------------------------------- MetodoMV::MetodoMV()
MetodoMV::MetodoMV(const std::string &n, Objeto *rthis,
		   MetodoRuntime m,
                   const MixinConArgumentos::Argumentos &args,
		   Acceso ac)
        : Metodo(n, rthis, args, ac), enEjecucion(false), met(m)
{
}

// --------------------------------------------------------- MetodoMV::ejecuta()
Objeto * MetodoMV::ejecuta(Argumentos * args, Objeto *rthis)
/**
        El "puente" entre la máquina virtual (y, por tanto, el ordenador real),
        y el "mundo" de objetos Zero.
        Como el aparato que permite conectarse a Matrix desde el mundo real, que
        se inserta en la nuca :-P
*/
{
    _enExcepcion = false;
	enEjecucion = true;

    #ifdef _UI_DEBUG
        iuComienzaMetodo( rthis, this );
    #endif

    // Actualizar al gestor de excepciones
    Runtime::gestorStack->call(rthis, this);

    // Preparar los Registros
	if ( vblesLocales.getNumero() == 0 ) {
        	preparaVariablesLocales( rthis );
	} else reg[NombreRegistro::__THIS]->ponReferencia( rthis );

    // Lanzar ejecución, tras pasar los argumentos
    if ( convertirArgumentosAFormales(args) ) {
                met( this );
    }
    else Runtime::ponExcepcion(Runtime::objetoExcepcion, EXC_ARGNUMBER);

	enEjecucion = false;

    #ifdef _UI_DEBUG
        iuFinMetodo( rthis, this, 0, getLocRR()->getObjeto() );
    #endif

    return getLocRR()->getObjeto();
}

// ---------------------------------------------------- MetodoMV::deSincroniza()
inline
void MetodoMV::deSincroniza(Metodo * met)
{
    return;
}

// ----------------------------------------------------------- MetodoMV::copia()
MetodoMV *MetodoMV::copia(Objeto *obj)
{
        MetodoMV *toret = NULL;

        try {
            toret = new MetodoMV( getNombre(),
	                          obj, met, argumentosFormales,
	                          tipoAcceso )
	    ;

	    return toret;
        }
	catch(std::bad_alloc &)
        {
                Runtime::ponExcepcion(Runtime::objetoExcepcion, EXC_CPYERR
		                                                  + '.' + ' '
								  + EXC_NOMEM)
		;
                return NULL;
        }
}

} // namespace Zero

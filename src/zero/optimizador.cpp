// optimizador.cpp

#include "optimizador.h"
#include "excep.h"
#include "mnemos.h"
#include "reservadas.h"

namespace Zero {

// ================================================================= Optimizador
std::vector<Optimizacion *> Optimizador::vOpts;

// --------------------------------------------- Optimizador::iniciaVectorOpts()
void Optimizador::iniciaVectorOpts(TipoOptimizacion opt)
/**
	Cuidadín: la optimización de etiquetas (sustituir las etiquetas por los
	saltos relativos) debe hacerse siempre en último lugar, para asegurarse
	de que no habrá movimientos de instrucciones.De otra manera,
	tendríamos saltos que no se corresponderían.
*/
{
    borrarTodosOpts();

    if ( opt == opt0 ) {
        registraOpt( OptChkMta::getOpt() );
        registraOpt( OptCnvtEtq::getOpt() );
    }
    else
    if ( opt == opt1 ) {
        registraOpt( OptChkMta::getOpt() );
        registraOpt( OptLitAg::getOpt() );
        registraOpt( OptDefPpio::getOpt() );
        registraOpt( OptCnvtEtq::getOpt() );
    }
    else
    if ( opt == opt2 ) {
        registraOpt( OptChkMta::getOpt() );
        registraOpt( OptLitAg::getOpt() );
        registraOpt( OptDefPpio::getOpt() );
        registraOpt( OptRedccOp::getOpt() );
        registraOpt( OptCnvtEtq::getOpt() );
    }
}

// --------------------------------------------------- Optimizador::registraOpt()
inline
void Optimizador::registraOpt(Optimizacion *opt)
{
	vOpts.push_back( opt );
}

// --------------------------------------------------- Optimizador::Optimizador()
Optimizador::Optimizador(Compilador *c, Optimizador::TipoOptimizacion nivelOpt)
	: opt( nivelOpt ), comp(c), listaMnemos(NULL), listaMnemosCmp(NULL)
{
	iniciaVectorOpts( opt );
}

Optimizador::Optimizador(Compilador::ListaMnemos &lm, Optimizador::TipoOptimizacion opt)
	: comp(NULL), listaMnemos(NULL)
{
	listaMnemosCmp = &lm;
	iniciaVectorOpts( opt );
}

inline
void Optimizador::borrarTodosOpts()
{
        for(register unsigned int i = 0; i < vOpts.size(); ++i)
        {
                delete vOpts[i];
        }

        vOpts.clear();
}

Optimizador::~Optimizador()
{
    borrarTodosOpts();
}

// --------------------------------------------------- Optimizador::optimizar()
void Optimizador::optimizar(
        Identificadores * args,
        Identificadores * atrs,
        Compilador::ListaMnemos *lm,
        Optimizador::TipoOptimizacion nivelOpt)
{
    // Preparar las optimizaciones
    if ( opt != nivelOpt ) {
        opt = nivelOpt;
        iniciaVectorOpts( opt );
    }

    // Cargar los atributos y argumentos
    argsFormales = args;
    atrsObj      = atrs;

	// Cargar la lista de mnemotécnicos
	if ( lm != NULL ) {
		listaMnemosCmp = lm;
	} else {
		if ( comp != NULL ) {
			listaMnemosCmp = &( comp->listaMnemos );
		}
	}

	if ( listaMnemosCmp != NULL
	  && listaMnemosCmp->size() > 0 )
	{
		// Pasarlo a la lista del optimizador
		listaMnemos = new(std::nothrow) ListaMnemos(
			listaMnemosCmp->begin(),
			listaMnemosCmp->end()
		);

		if ( listaMnemos == NULL ) {
			throw ENoHayMemoria( "preparando optimizador" );
		}

		// Hacer todas las optimizaciones registradas
		for(register unsigned int i = 0; i < vOpts.size(); ++i) {

                        if ( dynamic_cast<OptLitAg *>( vOpts[i] ) != NULL
                          && !( OptChkMta::getOpt()->getHayLiterales() ) )
                        {
                                /*
                                        Si es la optimización de literales,
                                        y no hay literales ... no hacer nada.
                                */
                                continue;
                        }

                        vOpts[i]->ponListaMnemos( listaMnemos );
                        vOpts[i]->ponIds( argsFormales, atrsObj );
                        vOpts[i]->hazOpt();
		}

		// "Pasar" las modificaciones al vector de mnemos
		listaMnemosCmp->clear();
		listaMnemosCmp->insert( listaMnemosCmp->begin(),
						listaMnemos->begin(),
				     	listaMnemos->end()
		);
	}

	return;
}

// ================================================================= OptDefPpio
Optimizacion * OptDefPpio::opt = NULL;

// ------------------------------------------------------- OptDefPpio::getOpt()
OptDefPpio * OptDefPpio::getOpt()
{
        if ( opt == NULL ) {
	        opt = new(std::nothrow) OptDefPpio( NULL );

                if ( opt == NULL ) {
                        throw ENoHayMemoria( "Creando opt. de DEF's" );
                }
        }

	return (OptDefPpio *) opt;
}

// ------------------------------------------------------- OptDefPpio::hazOpt()
void OptDefPpio::hazOpt()
/**
	Busca todos los mnemos DEF del código, y aquellos que no estén situados
	al principio del código, son recolocados: así, toda la primera parte
	del código será una hilera de DEFs.
*/
{
	Optimizador::ListaMnemos::iterator it = listaMnemos->begin();
	Optimizador::ListaMnemos::iterator aBorrar;

	// Pasar todos los DEFs iniciales
	while( dynamic_cast<NMDef *>( *it ) != NULL
	    && it != listaMnemos->end() )
	{
		++it;
	}

	// Avanzar hasta terminar
	while( it != listaMnemos->end() )
	{
		if ( dynamic_cast<NMDef *>( *it ) != NULL )
		{
			// Es un DEF, moverlo al principio
			aBorrar = it++;
			listaMnemos->insert( listaMnemos->begin(), *aBorrar );
			listaMnemos->erase( aBorrar );
		}
		else  ++it;
	}

	return;
}

// ================================================================= OptCnvtEtq
Optimizacion * OptCnvtEtq::opt = NULL;

// ------------------------------------------------------- OptCnvtEtq::getOpt()
OptCnvtEtq * OptCnvtEtq::getOpt()
{
        if ( opt == NULL ) {
	        opt = new(std::nothrow) OptCnvtEtq( NULL );

	        if ( opt == NULL ) {
		        throw ENoHayMemoria( "Creando opt. de etiquetas" );
	        }
        }

	return (OptCnvtEtq *) opt;
}

// ------------------------------------------------------- OptCnvtEtq::hazOpt()
void OptCnvtEtq::hazOpt()
/**
	Resuelve todos los saltos a etiquetas, asignando el número de
	mnemotécnicos positivo o negativo correspondiente.
	Elimina todas las etiquetas
*/
{
	Optimizador::ListaMnemos::iterator it;
	Optimizador::ListaMnemos::iterator aBorrar;
	unsigned int pos;

	// Comprobar que hay algo que hacer
	if ( listaMnemos         == NULL
	  || listaMnemos->size() == 0 )
	{
		goto FIN;
	}

	// Iniciar
	etqs.clear();

	// Buscar todas las etiquetas (y eliminarlas)
	it  = listaMnemos->begin();
	pos = 0;

	while( it != listaMnemos->end() )
	{
		if ( dynamic_cast<NMMta *>( *it ) != NULL ) {
			// Los meta se utilizarán para dar info en
			// tiempo de ejecución, pero serán interpretados
			// al cargar los mnemos del método, por lo que no
			// cuentan como instrucción

			++it;
			continue;
		}

		if ( dynamic_cast<NMEtq *>( *it ) != NULL )
		{
			const std::string &idEtq = ((NMEtq *) (*it))->getNombre();

			if ( etqs.find( idEtq ) == etqs.end() )
			{
				etqs.insert(
				      TablaEtiquetas::value_type( idEtq, pos )
				);
			}
			else throw EChkEtqRedefinida( idEtq.c_str() );

			aBorrar = it++;
			listaMnemos->erase( aBorrar );
		}
		else {
			++it;
			++pos;
		}
	}

	// Localizar todos los saltos, y cambiar etiqueta por núm.
	// de opcodes
	pos = 0;
	for(it = listaMnemos->begin(); it != listaMnemos->end(); ++it )
	{
		if ( dynamic_cast<NMMta *>( *it ) != NULL ) {
			continue;
		}

		if ( dynamic_cast<Flw *>( *it ) != NULL )
		{
			const std::string & idEtq = ((Flw *) *it)->getEtiqueta();

			TablaEtiquetas::const_iterator etq =
			       etqs.find( idEtq )
			;

			if ( etq != etqs.end() ) {
				((Flw *) *it)->putNumero( etq->second - pos );
			}
			else
			throw EChkEtqNoDefinida( idEtq.c_str() );
		}

		++pos;
	}

	FIN:
	return;
}

// ================================================================= OptLitAg
Optimizacion * OptLitAg::opt = NULL;
const std::string * OptLitAg::nombreLit = &LOC_LIT;

// ------------------------------------------------------- OptLitAg::getOpt()
OptLitAg * OptLitAg::getOpt()
{
        if ( opt == NULL )
        {
                opt = new(std::nothrow) OptLitAg( NULL );

                if ( opt == NULL ) {
                        throw ENoHayMemoria( "Creando opt. de constantes" );
                }
        }

	return (OptLitAg *) opt;
}

// --------------------------------------------------- OptLitAg::nuevoIdLit()
std::string OptLitAg::getNuevoIdLit()
{
	std::string toret = *nombreLit;

	toret.append( AnalizadorLexico::toString( numLits++ ) );

	return toret;
}

// ------------------------------------------------- OptLitAg::buscaLiteral()
const std::string *OptLitAg::buscaLiteral(Lit * lit)
{
	const std::string * toret = NULL;
	ListaIds::iterator itStr;
	ListaIds::iterator itInt;
	ListaIds::iterator itFlt;

	// ¿Es una cadena?
	if ( dynamic_cast<NMStr *>( lit ) != NULL ) {
		itStr = idsStr.find(
			(std::string *) &( ( (NMStr *) lit )->getLitAsString() )
		);

		if ( itStr != idsStr.end() ) {
			toret = &( itStr->second );
		}
	}
	else
	// ¿Es un entero?
	if ( dynamic_cast<NMInt *>( lit ) != NULL ) {
		itInt = idsInt.find(
			(std::string *) &( ( (NMInt *) lit )->getLitAsString() )
		);

		if ( itInt != idsInt.end() ) {
			toret = &( itInt->second );
		}
	}
	else
	// ¿Es un flotante?
	if ( dynamic_cast<NMFlt *>( lit ) != NULL ) {
		itFlt = idsFlt.find(
			(std::string *) &( ( (NMFlt *) lit )->getLitAsString() )
		);

		if ( itFlt != idsFlt.end() ) {
			toret = &( itFlt->second );
		}
	}
	else throw EInterno( "OptLitAg: registrando literal: no es literal" );

	return toret;
}

// -------------------------------------------------- OptLitAg::creaLiteral()
const std::string &OptLitAg::creaLiteral(Lit *lit)
{
	std::string nombreId = getNuevoIdLit();

	// Crearlo en la lista de identidicadores
	if ( dynamic_cast<NMStr *>( lit ) != NULL ) {
		return idsStr.insert(
			idsStr.begin(),
			ListaIds::value_type(
				(std::string *)
				     &( ( (NMStr *) lit )->getLitAsString() ),
				nombreId )
		)->second;
	}
	else
	if ( dynamic_cast<NMInt *>( lit ) != NULL ) {
		return idsInt.insert(
			idsInt.begin(),
			ListaIds::value_type(
				(std::string *)
				     &( ( (NMInt *) lit )->getLitAsString() ),
				nombreId )
		)->second;
	}
	else
	if ( dynamic_cast<NMFlt *>( lit ) != NULL ) {
		return idsFlt.insert(
			idsFlt.begin(),
			ListaIds::value_type(
				(std::string *)
				     &( ( (NMFlt *) lit )->getLitAsString() ),
				nombreId )
		)->second;
	}
	else throw EInterno( "OptCnstAg: registrando literal: no es literal" );
}

// -------------------------------------------------- OptLitAg::registraLit()
const std::string &OptLitAg::registraLit(Lit *lit)
{
	NMAsg  * nmAsg;
	const std::string *toret = NULL;
	NMDef * defId;

	toret = buscaLiteral( lit );

	// Si no se ha encontrado, entonces crearlo
	if ( toret == NULL )
	{
		toret = &( creaLiteral( lit ) );

		// Añadir un ASG para meterlo en el id adecuado
		nmAsg = new(std::nothrow) NMAsg( *toret, NombreRegistro::__ACC );
		if ( nmAsg != NULL ) {
			listaMnemos->push_front( nmAsg );
		}
		else throw ENoHayMemoria( "Optimizando constantes" );

		// Insertar creación del literal
		listaMnemos->push_front( lit );

		// Insertar el nuevo mnemo DEF en la lista de Mnemos
		defId = new(std::nothrow) NMDef( *toret );

		if ( defId == NULL ) {
			throw ENoHayMemoria( "optimizando consts" );
		}

		listaMnemos->push_front( defId );
	}
	else delete lit;

	return *toret;
}

void OptLitAg::colocaConstEnd()
/**
        Coloca un mnemo:
                MTA .CONSTEND ""
        Al principio del método. De esta forma, al meter los literales,
        va a ser el primero

        DEF  __lit__0
        STR "kk"
        ASG __lit__0
        MTA .CONSTEND ""
        ; resto del mth
*/
{
        NMMta * mta = new(std::nothrow) NMMta( NMMta::FinalDeclConstantes, "" );

        if ( mta == NULL ) {
                throw ENoHayMemoria( "creando meta decl constantes" );
        }

        listaMnemos->push_front( mta );
}

// ------------------------------------------------------- OptLitAg::hazOpt()
void OptLitAg::hazOpt()
/**
	Agrupa la creación de todas las constantes al comienzo. Si hay varias
	repetidas, entonces evita que se cree más de una.
*/
{
	Optimizador::ListaMnemos::iterator it;
	const std::string * nombreId;
	NMSet  * nmSet;

	// Comprobar que hay algo que hacer
	if ( listaMnemos         == NULL
	  || listaMnemos->size() == 0 )
	{
		goto FIN;
	}

	// Iniciar
	idsInt.clear();
	idsFlt.clear();
	idsStr.clear();
	numLits   = 0;
    colocaConstEnd();

	// Buscar las constantes en el código
	for (it = listaMnemos->begin(); it != listaMnemos->end(); ++it )
	{
		// ¿Es un literal? ... si lo es, actuar ...
		if ( dynamic_cast<Lit *>( *it ) != NULL )
		{
			// Obtener el nombre de la ref local para el lit
			nombreId = &( registraLit( ( (Lit *) *it ) ) );

			// Cambiar esta instrucción por un SET
			nmSet = new(std::nothrow) NMSet( NombreRegistro::__ACC, *nombreId );

			if ( nmSet != NULL ) {
   				*it = nmSet;
			}
			else throw ENoHayMemoria( "Optimizando constantes" );
		}
	}

	FIN:
	return;
}

// ================================================================= OptRedccOp
Optimizacion * OptRedccOp::opt = NULL;

// ------------------------------------------------------- OptRedccOp::getOpt()
OptRedccOp * OptRedccOp::getOpt()
{
        if ( opt == NULL ) {
	        opt = new(std::nothrow) OptRedccOp( NULL );

                if ( opt == NULL ) {
                        throw ENoHayMemoria( "Creando opt. de reducc." );
                }
        }

	return (OptRedccOp *) opt;
}

// ---------------------------------------- OptRedccOp::esMovimientoRelevante()
bool OptRedccOp::esMovimientoRelevante(
				Optimizador::MnemoIterator it,
				NombreRegistro::CodigoRegistro &obj,
				bool &eliminar,
				bool &acumuladorModificado)
{
	NombreRegistro * nr;
	bool toret = false;
	NMSet *set = dynamic_cast<NMSet *>( *it );
	NMAsg *asg = dynamic_cast<NMAsg *>( *it );

	// Por defecto, eliminar la sentencia
	eliminar = true;

	// Es un SET
	if ( set != NULL ) {
		// Buscar el registro origen, si es que es reg
		if ( set->getRegistro()->getCodigoRegistro() == obj )
		{
			if ( obj == NombreRegistro::__ACC )
			{
		//		acumuladorModificado = true;
			}

			toret = true;
			// Es un SET __obj __reg | <referencia>
			// Puede ser el destino, si no es registro, claro.
			nr = dynamic_cast<NombreRegistro *>( set->getReferencia() );
			if ( nr != NULL )
					obj = nr->getCodigoRegistro();

					if ( obj == NombreRegistro::__ACC
					  && acumuladorModificado )
					{
						toret = false;
					}
			else {
				// Se trata de un SET __obj <referencia>
				// Si la referencia es a un reg o ref local, vale, sino hay que tomar
				// el último registro como válido.
				if ( esRefLocal( set->getReferencia()->getNombre() ) ) {
						objetivo.destino = set->getReferencia()->getNombre();
				}
				else {
					eliminar = false;
					objetivo.destino = NombreRegistro::cnvtCodigoRegistroANombre( obj );
				}
			}
		}
	}
	else
	// Es un ASG
	if ( asg != NULL ) {
		nr = dynamic_cast<NombreRegistro *>( asg->getReferencia() );
		// Buscar el registro origen
		if ( nr != NULL
		  && nr->getCodigoRegistro() == obj ) {
			toret = true;

			// Ahora, tomar el nuevo registro objetivo
			obj = asg->getRegistro()->getCodigoRegistro();

			// Nunca eliminar un ASG
			eliminar = false;
		}
	}

	return toret;
}

// ---------------------------------------- OptRedccOp::hazPatronMsgArgRefLoc()
bool OptRedccOp::esMovimientoRegs(Optimizador::MnemoIterator it)
{
	return ( dynamic_cast<NMAsg *>( *it ) != NULL
          || dynamic_cast<NMSet *>( *it ) != NULL )
	;
}

// --------------------------------------------- OptRedccOp::aplicarCambioReg()
void OptRedccOp::aplicarCambioReg(
			Nombre ** reg,
			NombreRegistro::CodigoRegistro org,
			const std::string &dest)
{
	if ( dynamic_cast<NombreRegistro *>( *reg ) != NULL ) {
		if ( (*( ( NombreRegistro **) reg ) )->getCodigoRegistro() == org ) {
			delete *reg;
			*reg = Nombre::creaNombreAdecuado( dest );
		}
	}
}

// --------------------------------------------- OptRedccOp::aplicarCambioMsg()
void OptRedccOp::aplicarCambioMsg(
			NMMsg * msg,
			NombreRegistro::CodigoRegistro org,
			const std::string &dest)
{
	// La referencia al objeto a lanzar el mensaje
	aplicarCambioReg( &( msg->getReferencia() ), org, dest );

	// Los argumentos
	for(unsigned int i = 0; i < msg->getArgs()->size(); ++i) {
		aplicarCambioReg( &( ( *( msg->getArgs() ) )[ i ] ), org, dest );
	}
}

// --------------------------------------------- OptRedccOp::aplicarCambioTrw()
void OptRedccOp::aplicarCambioTrw(
			NMTrw * trw,
			NombreRegistro::CodigoRegistro org,
			const std::string &dest)
{
	// El mensaje a lanzar como excepción
	aplicarCambioReg( &( trw->getReferenciaMensaje() ), org, dest );

	// La referencia al objeto a lanzar
	aplicarCambioReg( &( trw->getReferencia() ), org, dest );
}

// --------------------------------------------- OptRedccOp::aplicarCambioRet()
void OptRedccOp::aplicarCambioRet(
			NMRet * ret,
			NombreRegistro::CodigoRegistro org,
			const std::string &dest)
{
	// El mensaje a lanzar como excepción
	aplicarCambioReg( &( ret->getReferencia() ), org, dest );
}

// ------------------------------------------------ OptRedccOp::aplicarCambio()
void OptRedccOp::aplicarCambio(
			Optimizador::MnemoIterator it,
			NombreRegistro::CodigoRegistro org,
			const std::string &dest)
{
	NMMsg * msg = dynamic_cast<NMMsg *>( *it );
	NMTrw * trw = dynamic_cast<NMTrw *>( *it );
	NMRet * ret = dynamic_cast<NMRet *>( *it );

	if ( msg != NULL ) {
		aplicarCambioMsg( msg, org, dest );
	}
	else
	if ( trw != NULL ) {
		aplicarCambioTrw( trw, org, dest );
	}
	else
	if ( ret != NULL ) {
		aplicarCambioRet( ret, org, dest );
	}
}

// ---------------------------------------- OptRedccOp::hazPatronMsgArgRefLoc()
void OptRedccOp::reducir()
/**
	Este es el meollo. Una vez localizada la instrucción relevante, hay que ir
	hacia arriba y buscar reducir el uso de registros.
*/
{
	bool hayModificacion = false;
	bool acumuladorModificado = false;
	bool eliminarSentencia;
	NombreRegistro::CodigoRegistro obj = objetivo.reg;
	std::vector<Optimizador::MnemoIterator> aBorrar;

	// En cualquier caso, no volver a repetir esta búsqueda
	objetivo.regsReducidos.insert( NombreRegistro::cnvtCodigoRegistroANombre( obj ) );

	// Llegar hasta el primero que puede ser
	Optimizador::MnemoIterator actual = objetivo.mnemoActual;

	// Buscar las sentencias a reducir
	--actual;
	while( actual != listaMnemos->begin()
           &&  esMovimientoRegs( actual ) )
	{
		if ( esMovimientoRelevante( actual, obj, eliminarSentencia, acumuladorModificado ) ) {
			if ( eliminarSentencia ) {
				aBorrar.push_back( actual );
			}
			hayModificacion = true;

			if ( !( objetivo.destino.empty() ) ) {
				break;
			}
		}

		--actual;
	}

	if ( hayModificacion ) {
		// ¿Cuál es el destino?
		if ( objetivo.destino.empty() ) {
			objetivo.destino = NombreRegistro::cnvtCodigoRegistroANombre( obj );
		}

		// Eliminar las sentencias encontradas
		for(unsigned int i = 0 ; i < aBorrar.size(); ++i) {
				listaMnemos->erase( aBorrar[ i ] );
		}
		aBorrar.clear();

		if ( obj == NombreRegistro::__ACC
		  && comprobarSiAcumuladorPosible( actual, objetivo.mnemoActual ) )
		{
			// Aplicar el cambio en la sentencia objetivo
			aplicarCambio( objetivo.mnemoActual, objetivo.reg, objetivo.destino );
		}
	}
}

bool OptRedccOp::comprobarSiAcumuladorPosible(Optimizador::MnemoIterator orig, Optimizador::MnemoIterator dest)
{
	for( ; orig != dest; ++orig ) {
		if ( dynamic_cast<NMTrw *>( *orig ) == NULL
		  && dynamic_cast<Flw *>( *orig )   == NULL
		  && dynamic_cast<NMDef *>( *orig ) == NULL
		  && dynamic_cast<NMNop *>( *orig ) == NULL
		  && dynamic_cast<NMEtq *>( *orig ) == NULL )
		{
			// Una asignac. que modifica el acumulador
			if ( dynamic_cast<NMAsg *>( *orig ) != NULL
			  && ( (NMAsg *) *orig)->getRegistro()->getCodigoRegistro() != NombreRegistro::__ACC )
			{
				continue;
			}

			// Una asignac. que modifica el acumulador
			if ( dynamic_cast<NMSet *>( *orig ) != NULL
			  && ( (NMSet *) (*orig))->getRegistro()->getCodigoRegistro() != NombreRegistro::__ACC )
			{
				continue;
			}

			break;
		}

	}

	return ( orig == dest );
}

// ------------------------------------------ OptRedccOp::buscaSigRegistroMsg()
NombreRegistro::CodigoRegistro OptRedccOp::buscaSigRegistroMsg(NMMsg * msg)
{
	NombreRegistro::CodigoRegistro toret = NombreRegistro::__NOREG;
	NombreRegistro * nr;
	objetivo.tipoSentencia = _sentencia_MSG;

	// Comprobar si la referencia es un registro
	nr = dynamic_cast<NombreRegistro *>( msg->getReferencia() );
	if ( nr != NULL )
	{
		if ( !objetivo.fueReducido( nr->getNombre() ) ) {
			toret = objetivo.reg = nr->getCodigoRegistro();
		}
	}

	// ... o si alguno de los argumentos es un registro
	for(unsigned int i = 0; i < msg->getArgs()->size(); ++i) {
		Nombre * arg = (*( msg->getArgs() ))[ i ];

		if ( dynamic_cast<NombreRegistro *>( arg ) != NULL ) {
			if ( !objetivo.fueReducido( arg->getNombre() ) ) {
				toret =
					objetivo.reg =
						( (NombreRegistro *) arg ) ->getCodigoRegistro()
				;
				break;
			}
		}
	}

	return toret;
}

// ------------------------------------------ OptRedccOp::buscaSigRegistroTrw()
NombreRegistro::CodigoRegistro OptRedccOp::buscaSigRegistroTrw(NMTrw * trw)
{
	NombreRegistro::CodigoRegistro toret = NombreRegistro::__NOREG;
	objetivo.tipoSentencia = _sentencia_TRW;
	NombreRegistro *nr;

	// Comprobar si la referencia es un registro
	nr = dynamic_cast<NombreRegistro *>( trw->getReferencia() );
	if ( nr != NULL )
	{
		if ( !objetivo.fueReducido( nr->getNombre() ) ) {
			toret = objetivo.reg = nr->getCodigoRegistro();
		}
	}

	// Comprobar si el lanzamiento es un registro
	nr = dynamic_cast<NombreRegistro *>( trw->getReferenciaMensaje() );
	if ( nr != NULL )
	{
		if ( !objetivo.fueReducido( nr->getNombre() ) ) {
			toret = objetivo.reg = nr->getCodigoRegistro();
		}
	}

	return toret;
}

// ------------------------------------------ OptRedccOp::buscaSigRegistroRet()
NombreRegistro::CodigoRegistro OptRedccOp::buscaSigRegistroRet(NMRet * ret)
{
	NombreRegistro * reg;
	NombreRegistro::CodigoRegistro toret = NombreRegistro::__NOREG;
	objetivo.tipoSentencia = _sentencia_RET;

	// Comprobar si la referencia es un registro
	reg = dynamic_cast<NombreRegistro *>( ret->getReferencia() );

	if (  reg != NULL ) {
		if ( !objetivo.fueReducido( reg->getNombre() ) ) {
			toret = objetivo.reg = reg->getCodigoRegistro();
		}
	}

	return toret;
}

// --------------------------------------------- OptRedccOp::buscaSigRegistro()
NombreRegistro::CodigoRegistro OptRedccOp::buscaSigRegistro()
{
	NombreRegistro::CodigoRegistro toret = NombreRegistro::__NOREG;
	objetivo.reg = toret;

	if ( dynamic_cast<NMMsg *>( *objetivo.mnemoActual ) != NULL ) {
		toret = buscaSigRegistroMsg( (NMMsg *) ( *objetivo.mnemoActual ) );
	} else
	// ¿Es un throw (lanzamiento de excepciones)?
	if ( dynamic_cast<NMTrw *>( *objetivo.mnemoActual ) != NULL ) {
		toret = buscaSigRegistroTrw( (NMTrw *) ( *objetivo.mnemoActual ) );
	}
	else
	// ¿Es un ret (return)?
	if ( dynamic_cast<NMRet *>( *objetivo.mnemoActual ) != NULL ) {
		toret = buscaSigRegistroRet( (NMRet *) ( *objetivo.mnemoActual ) );
	}

	if ( toret == NombreRegistro::__THIS
      || toret == NombreRegistro::__EXC )
	{
		objetivo.reg = toret = NombreRegistro::__NOREG;
	}

	return toret;
}

// -------------------------------------------  OptRedccOp::buscaSigSentencia()
Optimizador::MnemoIterator OptRedccOp::reduceSentenciaActual()
{
	buscaSigRegistro();

	// Reducir
	while( objetivo.reg != NombreRegistro::__NOREG ) {
		reducir();
		buscaSigRegistro();
	}

	return objetivo.mnemoActual;
}

// ------------------------------------------------------- OptRedccOp::hazOpt()
void OptRedccOp::hazOpt()
/**
	Realiza una serie de reducciones de opcodes. El objetivo es evitar
	el trasiego de referencias entre registros.
*/
{
	// Registrar las referencias locales
	NMDef * ref;
	Optimizador::MnemoIterator it = listaMnemos->begin();

	while( it != listaMnemos->end() ) {
		ref = dynamic_cast<NMDef *>( *it );

		if ( ref != NULL ) {
			refsLocales.insert( ref->getNombre() );
		}

		++it;
	}

	// Reducir opcodes
	objetivo.init();
	objetivo.mnemoActual = listaMnemos->end();
	reduceSentenciaActual();

	while( objetivo.mnemoActual != listaMnemos->begin() ) {
		reduceSentenciaActual();
		objetivo.preparaSigBusqueda();
	}
}

// ================================================================== OptChkMta
Optimizacion * OptChkMta::opt = NULL;

// -------------------------------------------------------- OptChkMta::getOpt()
OptChkMta * OptChkMta::getOpt()
{
        if ( opt == NULL ) {
	        opt = new(std::nothrow) OptChkMta( NULL );

                if ( opt == NULL ) {
                        throw ENoHayMemoria( "Creando opt. de 'metas'" );
                }
        }

	return (OptChkMta *) opt;
}

// --------------------------------------------------------- OptChkMta::chkId()
void OptChkMta::chkId(const std::string &id)
/**
	Comprueba que un identificador
  	<ol>
	<li>no es ni un registro,
	<li>ni un ids de los usados para los literales,
	<li>ni una de las propiedades de método u objeto
	<li>no es el atributo parent
	</ol>
*/
{
        int eq;

	// Comprueba que no es un registro
	for(register unsigned int i = 0; i < NombreRegistro::numRegistros; ++i)
	{
		if ( id == NombreRegistro::cnvtCodigoRegistroANombre(
		   			(NombreRegistro::CodigoRegistro) i ) )
		{
			throw ESintxIdNoValido( std::string( "es registro: " + id ).c_str() );
		}
	}

	// Comprueba que no es un literal
        if ( id.length() > OptLitAg::nombreLit->length() ) {
                eq = id.compare(
                        0,
                        OptLitAg::nombreLit->length(),
                        *OptLitAg::nombreLit
                );
        }
        else eq = id.compare( *OptLitAg::nombreLit );

        if ( eq == 0 )
        {
		throw ESintxIdNoValido( std::string( "parece literal: " + id ).c_str() );
        }

	// Comprobar que no es el atributo parent
	if ( id == ATR_PARENT ) {
		throw ESintxIdNoValido( std::string( "parece atr. parent: " + id ).c_str() );
	}
}

// -------------------------------------------------------- OptChkMta::hazOpt()
void OptChkMta::hazOpt()
/**
	Realiza una serie de comprobaciones semánticas
*/
{
	register unsigned int i;
	Optimizador::ListaMnemos::iterator it;
    hayLiterales = false;

	// Comprobar que hay algo que hacer
	if ( listaMnemos         == NULL
	  || listaMnemos->size() == 0 )
	{
		goto FIN;
	}

	// Buscar entre los identificadores de atributos
	for(i = 0; i < atrsObj->getNumero(); ++i)
	{
                if ( atrsObj->getID( i ) != ATR_PARENT ) {
		        chkId( atrsObj->getID( i ) );
                }
	}

	// Buscar los identificadores de vbles locales (args)
	for(i = 0; i < argsFormales->getNumero(); ++i)
	{
                if ( NombreRegistro::cnvtNombreACodigoRegistro(
                                                argsFormales->getID( i ) )
                        == NombreRegistro::__NOREG
                   )
                {
		        chkId( argsFormales->getID( i ) );
                }
	}

	// Buscar los identificadores de vbles locales en el código
        // y comprobar si hay literales
	for (it = listaMnemos->begin(); it != listaMnemos->end(); ++it )
	{
                if ( dynamic_cast<Lit *>( *it ) != NULL ) {
                        hayLiterales = true;
                }

		if ( dynamic_cast<NMDef *>( *it ) != NULL )
		{
		    	chkId( ( (NMDef *) *it)->getNombre() );
		}
	}

	FIN:
	return;
}

} // namespace Zero

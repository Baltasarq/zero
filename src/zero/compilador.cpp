// compilador.cpp
/*
	El compilador toma una cadena,
	que debe contener los mnemotécnicos en formato ensamblador,
	y la compila como cuerpo de un método.

	jbgarcia@uvigo.es
*/

#include "compilador.h"
#include "optimizador.h"
#include "excep.h"

namespace Zero {

// ================================================================= Compilador
// --------------------------------------------------- Compilador::Compilador()
Compilador::Compilador( std::string *t,
                        Optimizador *o,
                        Identificadores *args,
                        Identificadores *atrs )
	: opt(o), lex(NULL), txt(t)
{
	// Crear el optimizador
	if ( opt == NULL ) {
		opt = new(std::nothrow) Optimizador( this );

		if ( opt == NULL ) {
			throw ENoHayMemoria( "construyendo optimizador" );
		}
	}

	// Resetear el compilador con el código
	reset( txt, args, args );
}

Compilador::~Compilador()
{
	delete lex;
	delete opt;

	lex  = NULL;
	opt  = NULL;
	atrs = NULL;
	args = NULL;
}

// -------------------------------------------------------- Compilador::reset()
void Compilador::reset( std::string *t,
                        Identificadores * args,
                        Identificadores * atrs)
/**
	Coloca una nueva cadena para que sea compilada.
	Implica un nuevo analizador léxico.
	Si no se puede crear, lanza una excepción ENoHayMemoria.

	Cada vez que se llama a compilar, se empieza desde el principio
	de la cadena, por lo que será muy ineficiente e innecesario llamar
	a reset() para compilar la misma cadena.

	Hay que pasarle los atributos del objeto y los argumentos formales,
	para que pueda hacer comprobaciones en aquellos mnemos en los que
	obligatoriamente hay que incluir una ref. local.
*/
{
	// Crear el lexer
	if ( t != NULL ) {
		lin = 0;
		txt = t;
		delete lex;
		lex = new(std::nothrow) AnalizadorLexico( txt );

		if ( lex == NULL ) {
			throw ENoHayMemoria( "construyendo lexer" );
		}

		// Almacenar las refs. locales
		this->args = args;
		for(unsigned int i = 0; i < NombreRegistro::__NOREG; ++i) {
		    args->inserta(
                NombreRegistro::cnvtCodigoRegistroANombre(
                    (NombreRegistro::CodigoRegistro)
                        ( NombreRegistro::__ACC + i ) )
            );
		}

		// Almacenar los atributos
		this->atrs = atrs;
	} else {
		delete lex;
		lex = NULL;
		txt = NULL;
	}
}

// ------------------------------------------ Compilador::prepararCompilacion()
void Compilador::prepararCompilacion()
{
	register unsigned int i;
	cadAviso = "";

	// Ponernos al principio del texto a compilar
	lex->reset();

	// Borrar la lista de mnemotécnicos
	listaMnemos.clear();

	// Preparar los identificadores
	ids.reset();

	// Meter los argumentos formales en los ids
	for(i = 0; i < args->getNumero(); ++i)
	{
		ids.inserta( args->getID( i ) );
	}
}

// ----------------------------------------------------- Compilador::compilar()
Compilador::ListaMnemos &Compilador::compilar(Optimizador::TipoOptimizacion nivelOpt)
{
	if ( txt != NULL )
	{
          try {
	    prepararCompilacion();

	    if ( lex != NULL )
	    {
	      while( !lex->esEol() )
	      {
                ++lin;
                lex->pasaEsp();

                Mnemotecnico *mnemo = Mnemotecnico::compilar( *lex );

                if ( mnemo != NULL) {
                        // Hacer comprobaciones
                        chkMnemo( mnemo );

                        // Guardar el mnemo
                        listaMnemos.push_back( mnemo );
                }
                else {
                        throw ESintxMnemoInvalido(
                                std::string( lex->getTokenActual() ).c_str() )
                        ;
                }
              }
              opt->optimizar( args, atrs, NULL, nivelOpt );
            }
          } catch(ECompilacion &e)
          {
              e.putLinMetodo( lin );
              throw;
          }
	}

	return listaMnemos;
}

// ---------------------------------------------------------- Compilador::chk()
void Compilador::chkMnemo(Mnemotecnico *mnemo)
/**
	Realiza varias comprobaciones.
	Por ejemplo, no tienen sentido:
  	<ul>
		<li>NMObj
		<li>NMAtr
		<li>NMEno
		<li>NMMth
		<li>NMEnm
	</ul>

	Y debido a que utilizan exclusivamente ref. locales, las siguientes
	deben ser comprobadas minuciosamente:
	<ul>
		<li>NMDef
		<li>NMRet
		<li>NMAsg
		<li>NMMsg
	</ul>
*/
{
	if ( dynamic_cast<NMObj *>( mnemo ) != NULL )
	{
		throw ESintxEnoNoPermitido( "compilando mth" );
	}
	else
	if ( dynamic_cast<NMEno *>( mnemo ) != NULL )
	{
		throw ESintxEnoNoPermitido( "compilando mth" );
	}
	else
	if ( dynamic_cast<NMMth *>( mnemo ) != NULL )
	{
		throw ESintxMthNoPermitido( "compilando mth" );
	}
	else
	if ( dynamic_cast<NMEnm *>( mnemo ) != NULL )
	{
		throw ESintxEnmNoPermitido( "compilando mth" );
	}
	else
	if ( dynamic_cast<NMAtr *>( mnemo ) != NULL )
	{
		throw ESintxAtrNoPermitido( "compilando mth" );
	}
	else
	if ( dynamic_cast<NMDef *>( mnemo ) != NULL )
	{
		chkNMDef( (NMDef *) mnemo );
	}
	else
	if ( dynamic_cast<NMRet *>( mnemo ) != NULL )
	{
		chkNMRet( (NMRet *) mnemo );
	}
	else
	if ( dynamic_cast<NMAsg *>( mnemo ) != NULL )
	{
		chkNMAsg( (NMAsg *) mnemo );
	}
	else
	if ( dynamic_cast<NMMsg *>( mnemo ) != NULL )
	{
		chkNMMsg( (NMMsg *) mnemo );
	}

	return;
}

// ----------------------------------------------------- Compilador::chkNMDef()
void Compilador::chkNMDef(NMDef *mnemo)
/// Comprueba que una def. no sea duplicada
{
	if ( ids.busca( mnemo->getNombre() ) ) {
		throw ESintxIdNoValido( std::string( "nombre de ref. local duplicado: "
		                    + mnemo->getNombre()
				    + ", en DEF" ).c_str() );
	}
	else ids.inserta( mnemo->getNombre() );

	return;
}

// ----------------------------------------------------- Compilador::chkNMRet()
void Compilador::chkNMRet(NMRet *mnemo)
/// Comprueba que la referencia del return exista
{
    const std::string &nombreRef = mnemo->getReferencia()->getNombre();

	if ( !ids.busca( nombreRef )
          && !atrs->busca( nombreRef ) )
	{
		cadAviso += "ref. local o atributo indefinido: "
				+ mnemo->getReferencia()->getNombre()
				+ ", en RET\n";
		;
	}

	return;
}

// ----------------------------------------------------- Compilador::chkNMAsg()
void Compilador::chkNMAsg(NMAsg *mnemo)
/// Comprueba que la referencia local exista
{
    const std::string &nombreRef = mnemo->getReferencia()->getNombre();

	if ( !ids.busca( nombreRef )
          && !atrs->busca( nombreRef ) )
	{
		cadAviso += "ref. local indefinida: "
		                    + nombreRef
				    + ", en ASG\n"
		;
	}

	return;
}

// ----------------------------------------------------- Compilador::chkNMMsg()
void Compilador::chkNMMsg(NMMsg *mnemo)
/// Comprueba que los argumentos sean refs. locales o atributos
{
	for(register unsigned int i = 0; i < mnemo->getArgs()->size(); ++i)
	{
		if ( !ids.busca( (*(mnemo->getArgs()))[i]->getNombre() ) )
		{
			throw EChkArgInvalido(
				std::string( "ref. local indefinida: "
			       + (*(mnemo->getArgs()))[i]->getNombre()
			       + ", como argumento en MSG\n" ).c_str()
			);
		}
	}

	return;
}

} // namespace Zero

// zma.cpp
/*
        Macro Ensamblador de la máquina virtual Zero

        jbgarcia@uvigo.es
*/

#include <iostream>
#include <cstdio>
#include <cctype>
#include <vector>
#include <queue>
#include <string>
#include <sstream>

#include "zma.h"
#include "mnemos.h"
#include "excep.h"

// ================================ Constantes ===============================
const std::string sintaxis        = "zma <nombreFichero>";
const int    MAXLIN          = 4096;

const std::string nombre          = "Zma - Macro Ensamblador Zero "
                               "(Zero Macro Assembler)"
;

const std::string version         = "v2.4";
const std::string serial          = "061109";

const std::string defext	     = ".def";

const std::string msgAVISO        = "AVISO (warning) ";

const char   MARCACOMENTARIO = '!';
const char   MarcaComMeta    = '[';
const char   MarcaFinMeta    = ']';

// ================================================================ MacroEnsamZ
Zero::Optimizador * MacroEnsamZ::opt;

// ------------------------------------------------- MacroEnsamZ::MacroEnsamZ()
MacroEnsamZ::MacroEnsamZ(FILE *e, const std::string &f)
	: ent(e),  fichsalida(f), lin(1),estado(TOPLEVEL), lex(NULL)
/* Ya no es necesario que el macroensamblador compile por su cuenta */

{
 /*       sal = new(std::nothrow) FicheroSoporte( fichsalida, FicheroSoporte::Nuevo );

        if ( sal == NULL ) {
                throw EMedioNoEncontrado( "Imposible crear: " + fichsalida );
	}

        opt = new(std::nothrow) Optimizador;

        if ( opt == NULL ) {
                throw ENoHayMemoria( "Sin memoria, creando optimizador" );
	}
*/
	log = new Log( fichsalida );

//     Cabecera().escribe( sal );
}

MacroEnsamZ::~MacroEnsamZ() {
//     delete opt;
// 	delete lex;
	delete log;
// 	delete sal;
}

// --------------------------------------------------- MacroEnsamZ::compNMObj()
void MacroEnsamZ::compNMObj()
{
	if ( estado == TOPLEVEL )
	{
		// Obtiene el nombre del nuevo objeto y del padre
		std::string nombre;
		std::string nombre_padre;

		lex->pasaEsp();
		nombre = lex->getToken();
		if (nombre.empty()) {
			throw Zero::ESintxIdNoValido(
			        "Se esperaba id: nombre de objeto"
			);
		}

		if (!Zero::NombreIdentificador::compruebaId(nombre)) {
			throw Zero::ESintxIdNoValido(nombre.c_str());
		}

		// Preparar indicadores de estado
		estado = ENOBJETO;
		literalesPorObjeto.clear();
		objetoActual = nombre;
		metodoActual = "";

		// Hay los ':' ?
		lex->pasaEsp();

		if ( lex->esEol() ) {
			nombre_padre = Zero::OBJ_OBJECT;
		} else {
			if ( lex->getCaracterActual() != ':' )
				throw Zero::ESintaxis( "se esperaban ':'" );
			else {
				// Coger nombre de obj padre
				lex->avanza();
				lex->pasaEsp();
				nombre_padre = lex->getToken();
			}
		}

		if (!Zero::NombreReferencia::compruebaRef(nombre_padre))
			throw Zero::ESintxIdNoValido(nombre_padre.c_str());
		if (!idsProg.insertaObjeto(nombre, nombre_padre))
			throw Zero::ESintxIdNoValido("Nombre de objeto duplicado");

		// Crear
		(*log)( "\t" + Zero::NMObj::mnemoStr + ' '
		             + nombre
			     + ' ' + nombre_padre )
		;

		//(NMObj(nombre, nombre_padre)).escribe( sal );
	} else throw Zero::ESintxObjNoPermitido("");

	return;
}

// --------------------------------------------------- MacroEnsamZ::compNMMth()
void MacroEnsamZ::compNMMth()
{
    Zero::IdsPorObjeto * ids;

	if ( estado == ENOBJETO )
	{
		// Obtener el nombre del méodo,
		// y sus argumentos
		std::string acceso;
		std::string nombre;
		std::string ref;

		estado = ENCUERPOMETODO;

		lex->pasaEsp();
		acceso = lex->getLiteral(' ');
		if (acceso != Zero::NMMth::PRIVADO
		 && acceso != Zero::NMMth::PUBLICO)
		{
			throw Zero::ESintxIdNoValido("Se esperaba marca de acceso");
		}

		lex->pasaEsp();
		nombre = lex->getToken();
		if (nombre.empty()) {
			throw Zero::ESintxIdNoValido("Se esperaba id: "
				"nombre de mth")
			;
		}
		if (!Zero::NombreIdentificador::compruebaId(nombre)) {
			throw Zero::ESintxIdNoValido(nombre.c_str());
		}

		metodoActual = nombre;
		Zero::NMMth mth(acceso, nombre);

		// Llegar hasta el '('
		lex->pasaEsp();
		if ( lex->getCaracterActual() == '(' ) {
			lex->avanza();
			lex->pasaEsp();
		}
		else throw Zero::ESintaxis("Se esperaba '('");


		// Coger los argumentos
		ref = lex->getToken();
		while(!(ref.empty()))
		{
			if (!Zero::NombreReferencia::compruebaRef(ref))
				throw Zero::ESintxIdNoValido(ref.c_str());

			mth.masArgumentos( ref );
			lex->pasaEsp();
			ref = lex->getToken();
		}

                ids = idsProg.buscaObjeto( objetoActual );
		if ( !( ids->insertaMetodo( nombre,
					         ( acceso == Zero::NMMth::PUBLICO ),
						 *mth.getArgs() ) ) )
		{
			throw Zero::ESintxIdNoValido("Nombre de método duplicado");
		}

//		mth.escribe( sal );
		(*log)( '\t' + mth.listar() );
		mnemosMetActual = new(std::nothrow) Metodo( objetoActual, nombre, ids, sal );

		if ( mnemosMetActual == NULL ) {
			throw Zero::ENoHayMemoria( std::string( "creando método"
						+ objetoActual + '.'
						+ nombre + '(' + ')' ).c_str() );
		}
	}
	else throw Zero::ESintxMthNoPermitido("");

	return;
}

// --------------------------------------------------- MacroEnsamZ::compNMAtr()
void MacroEnsamZ::compNMAtr()
{
	if ( estado == ENOBJETO )
	{
		std::string acceso;
		std::string nombre;
		std::string ref = Zero::OBJ_NOTHING;
		std::string lit;
		Zero::AnalizadorLexico::TipoToken tk = Zero::AnalizadorLexico::NADA;

		// Accesibilidad
		lex->pasaEsp();
		acceso = lex->getLiteral(' ');
		if (acceso != Zero::NMAtr::PRIVADO
		 && acceso != Zero::NMAtr::PUBLICO)
		{
			throw Zero::ESintxIdNoValido("Se esperaba marca de acceso");
		}

		// Es un atributo: asegurar que no hay método actual
		metodoActual = "";

		// Nombre del atributo
		lex->pasaEsp();
		nombre = lex->getToken();
		if (!Zero::NombreIdentificador::compruebaId(nombre))
			throw Zero::ESintxIdNoValido(nombre.c_str());
		if (!(idsProg.buscaObjeto(objetoActual)
			->insertaAtributo(nombre, ( acceso == Zero::NMAtr::PUBLICO ))))
		{
			throw Zero::ESintxIdNoValido("Nombre de atributo duplicado");
		}

		// Pasar el '='
		lex->pasaEsp();
		if (lex->getCaracterActual() != '=') {
			throw Zero::ESintxIdNoValido("Se esperaba '='");
		}
		lex->avanza();
		lex->pasaEsp();


		// Identificador del objeto/LitNumerico/LitCadena
		tk = lex->getTipoSiguienteToken();
		if (tk == Zero::AnalizadorLexico::IDENTIFICADOR)
		{
			lex->pasaEsp();
			ref = lex->getToken();
                        lex->getToken(); // Vaciarlo

			if (!Zero::NombreReferencia::compruebaRef(ref)) {
				throw Zero::ESintxIdNoValido(ref.c_str());
            }
		}
		else
		if (tk == Zero::AnalizadorLexico::LITNUMERICO)
		{
			lex->pasaEsp();
			lit = lex->getNumero();
			if ( !Zero::AnalizadorLexico::compruebaFlotante( lit ) ) {
				throw Zero::ESintxFltNoValido( lit.c_str() );
			}

            lit = '+' + lit;
		}
		else
		if (tk == Zero::AnalizadorLexico::LITCADENA)
		{
			// Llegar hasta las comillas
			lex->pasaEsp();
			if (lex->getCaracterActual() != '"')
				throw Zero::ESintxComillasEsperadas( "\"" );

			// Pasar las comillas y obtener literal
			lex->avanza();
			lit = '"' + lex->getLiteral('"');
		}
		else {
			// No hay nada
			throw Zero::ESintxIdNoValido("Se esperaba referencia, "
						"literal de cadena o numérico"
									);
		}

		if (tk != Zero::AnalizadorLexico::NADA
 		 && tk != Zero::AnalizadorLexico::IDENTIFICADOR)
		{
             // Es un literal (cadena, numérico ...)
      		Zero::Literal l( nombre, lit );
			literalesPorObjeto.push_back(l);
			Zero::NMAtr atr( acceso, nombre, Zero::OBJ_NOTHING, lit );
// 			atr.escribe( sal );
			(*log)( '\t' + Zero::NMAtr::mnemoStr + ' ' + acceso + ' '
						 + nombre + ' ' + l.getLiteralConTipo()
			);
        }
		else {
			Zero::NMAtr atr( acceso, nombre, ref );
// 			atr.escribe( sal );
			(*log)( '\t' + atr.listar() );
		}
	}
	else throw Zero::ESintxAtrNoPermitido("");

	return;
}

// --------------------------------------------------- MacroEnsamZ::compNMRet()
void MacroEnsamZ::compNMRet()
{
	if ( estado == ENCUERPOMETODO
	  || estado == ENEXCEPMETODO )
	{
		std::string vble;
		Zero::NMRet *ret;

		if (estado == ENCUERPOMETODO)
                estado = ENEXCEPMETODO;
		else  	estado = ENFINMETODO;

		lex->pasaEsp();
		vble = lex->getToken();

		if ( vble.empty() ) {
			ret = new(std::nothrow) Zero::NMRet( Zero::NombreRegistro::__RR );

			if ( ret == NULL ) {
				throw Zero::ENoHayMemoria( "creando mnemo RET" );
			}
		}
		else {
			Zero::NombreRegistro::CodigoRegistro cr =
				Zero::NombreRegistro::cnvtNombreACodigoRegistro(vble);
			if ( cr != Zero::NombreRegistro::__NOREG) {
				ret = new(std::nothrow) Zero::NMRet( cr );
			} else {
			  Metodo::CuerpoMetodo instrucc;

			  compilaExpresion(
				lex->getTokenActual(),
				mnemosMetActual,
				instrucc
			  );

			  instrucc.push_back(
			  	new Zero::NMAsg( Zero::LOC_RR, Zero::NombreRegistro::__ACC )
			  );

			  mnemosMetActual->masMnemos( instrucc );

			  Metodo::listar( *log, instrucc );

			  ret = new(std::nothrow) Zero::NMRet( Zero::NombreRegistro::__RR );
			}
		}

		if ( ret == NULL ) {
			throw Zero::ENoHayMemoria( "creando mnemoténico RET" );
		}

		(*log)( '\t' + ( ret->listar() ) );
		mnemosMetActual->masMnemos( ret );
	}
	else throw Zero::ESintxRetNoPermitido( "" );

	return;
}

// --------------------------------------------------- MacroEnsamZ::compNMEnm()
void MacroEnsamZ::compNMEnm()
{
	if ( estado == ENEXCEPMETODO
	  || estado == ENFINMETODO )
	{
		// Cambiar de estado
		estado = ENOBJETO;

		// Serializar a disco el método actual
//		mnemosMetActual->escribe();

		// Escribir el mnemo a disco
		(*log)('\t' + Zero::NMEnm::mnemoStr + ' ' + ';' + metodoActual);
// 		( NMEnm().escribe( sal ) );

		// Limpiar
		metodoActual = "";
		delete mnemosMetActual;
		mnemosMetActual = NULL;
	}
	else throw Zero::ESintxEnmNoPermitido("");

	return;
}

// --------------------------------------------------- MacroEnsamZ::compNMEno()
void MacroEnsamZ::compNMEno()
{
	if ( estado == ENOBJETO )
	{
// 		if (literalesPorObjeto.size() > 0)
// 		{
// 			// Crear un método que restaure los atributos del obj
//  			( NMMth( NMMth::PUBLICO, MET_ALCARGAR ) ).escribe( sal );
//
//
// 			// Para cada atributo detectado ...
// 			for(unsigned int n=0;n<literalesPorObjeto.size();++n)
// 			{
//                                 // Una cadena ?
// 				if ( literalesPorObjeto[n].getTipo() ==
// 						               Literal::LitStr )
// 				{
// 					// Es una cadena
// 					( NMStr(
//                                            literalesPorObjeto[n].getLiteral() )
//                                         ).escribe( sal );
// 				}
// 				else
//                                 // Un número ... real ?
//                                 if ( literalesPorObjeto[n].getTipo() ==
//                                                             Literal::LitNumFlt )
//                                 {
//                                         ( NMFlt(
//                                            literalesPorObjeto[n].getLiteral()
//                                         ) ).escribe( sal );
//                                 }
//                                 else
//                                 // Un número ... entero ?
//                                 if ( literalesPorObjeto[n].getTipo() ==
//                                                             Literal::LitNumInt )
//                                 {
// 					( NMInt(
//                                            literalesPorObjeto[n].getLiteral() )
//                                         ).escribe( sal );
// 				}
//
// 				// Asignar al atr correspondiente
// 				( NMAsg(
// 				    literalesPorObjeto[n].getNombreAtributo(),
// 				    NombreRegistro::__ACC )
//                                 ).escribe( sal );
// 			}
//
// 			// Fin del método de restauración de atrs.
// 			NMRet( LOC_ACC ).escribe( sal );
// 			NMEnm().escribe( sal );
// 		}

		// Escribir
		(*log)( '\t' + Zero::NMEno::mnemoStr + ' ' + ';' + objetoActual );
// 		NMEno().escribe( sal );

		// Cambiar de estado
		objetoActual = "";
		estado = TOPLEVEL;
	} else throw Zero::ESintxEnoNoPermitido("");

	return;
}

// --------------------------------------------------- MacroEnsamZ::compNMMta()
void MacroEnsamZ::compNMMta()
{
	Zero::NMMta * mta;
	std::string pragma;
	std::string lit;

	// Comprobar que hay un '['
	lex->pasaEsp();
	if ( lex->getCaracterActual() != MarcaComMeta ) {
		throw Zero::ESintaxis( "Se esperaba " + MarcaComMeta );
	}

	// Pasar el '['
	lex->avanza();
	lex->pasaEsp();

	// Coger el pragma
	pragma = lex->getToken();
	lex->pasaEsp();

	// Pasar el '=' (si existe)
	if ( lex->getCaracterActual() == '=' )
	{
		lex->avanza();
		lex->pasaEsp();

		if ( lex->getCaracterActual() != '"' ) {
			throw Zero::ESintaxis( "Se esperaba '\"'" );
		}

		// Tomar el literal
		lex->avanza();
		lit = lex->getLiteral( '"' );
		lex->avanza();
		lex->pasaEsp();
	}

	if ( lex->getCaracterActual() != MarcaFinMeta ) {
		throw Zero::ESintaxis( "Se esperaba " + MarcaFinMeta );
	}

	mta = new(std::nothrow) Zero::NMMta( pragma, lit );

	if ( mta == NULL ) {
		throw Zero::ENoHayMemoria( "creando mnemoténico MTA" );
	}

        (*log)( '\t' + mta->listar() );

        if ( estado == ENOBJETO ) {
//              mta->escribe( sal );
             delete mta;
        }
        else
        if ( estado == ENCUERPOMETODO ) {
        	mnemosMetActual->masMnemos( mta );
        }
        else throw Zero::ESintxMtaNoPermitido( "" );
}

// --------------------------------------------------- MacroEnsamZ::compNMTrw()
void MacroEnsamZ::compNMTrw()
{
	if ( estado == ENCUERPOMETODO
	  || estado == ENEXCEPMETODO )
	{
		std::string ref;
		std::string mensaje;
		Zero::NMTrw * trw = NULL;

		lex->pasaEsp();
		ref = lex->getToken();

		if (ref.empty()) {
			throw Zero::ESintxIdNoValido(
                    "Se esperaba referencia de objeto"
					" a lanzar como excepción");
                }

		if (!Zero::NombreIdentificador::compruebaId(ref)) {
			throw Zero::ESintxIdNoValido(ref.c_str());
                }

		lex->pasaEsp();
		mensaje = lex->getToken();

		// Según haya mensaje o no, es un tipo u otro
		if ( !mensaje.empty() )
		{
			if (!Zero::NombreIdentificador::compruebaId(mensaje)) {
			        throw Zero::ESintxIdNoValido(mensaje.c_str());
			}

			trw = new(std::nothrow) Zero::NMTrw( ref, mensaje );
		} else trw = new(std::nothrow) Zero::NMTrw( ref );

		if ( trw == NULL ) {
			throw Zero::ENoHayMemoria( "creando mnemoténico TRW" );
		}

		mnemosMetActual->masMnemos( trw );
		(*log)( '\t' + trw->listar() );
	}
	else throw Zero::ESintxTrwNoPermitido("");

	return;
}

// --------------------------------------------------- MacroEnsamZ::compNMDef()
void MacroEnsamZ::compNMDef()
{
	if ( estado == ENCUERPOMETODO
	  || estado == ENEXCEPMETODO )
	{
		Zero::Mnemotecnico * mnemo;
		std::string nom_vble_local;

		lex->pasaEsp();
		nom_vble_local = lex->getToken();

		if (nom_vble_local.empty()) {
			throw Zero::ESintxIdNoValido(
				"Se esperaba id: nombre de referencia local"
			);
		}

		if (!Zero::NombreIdentificador::compruebaId(nom_vble_local))
			throw Zero::ESintxIdNoValido(nom_vble_local.c_str());
		if (!(idsProg.buscaObjeto(objetoActual)->
			insertaVbleMetodo(metodoActual, nom_vble_local)))
			throw Zero::ESintxIdNoValido("nombre de referencia local duplicado");

		lex->pasaEsp();
		Metodo::CuerpoMetodo instrucc;
		mnemo = new Zero::NMDef( nom_vble_local );

		if ( mnemo != NULL )
			instrucc.push_back( mnemo );
		else throw Zero::ENoHayMemoria( "creando mnemo DEF" );

		if ( lex->getCaracterActual() == '=' ) {
			// Pasar el '='
			lex->avanza();

			compilaExpresionOLiteral( instrucc );

			mnemo =
			    new(std::nothrow) Zero::NMAsg( nom_vble_local, Zero::NombreRegistro::__ACC )
			;

			if ( mnemo != NULL )
				instrucc.push_back( mnemo );
			else throw Zero::ENoHayMemoria( "creando mnemo ASG" );
		}

		for (unsigned int i = 0; i < instrucc.size(); ++i) {
			(*log)('\t' + instrucc[i]->listar() + '\n');
		}

		mnemosMetActual->masMnemos( instrucc );
	}
	else throw Zero::ESintxDefNoPermitido("");

	return;
}

// --------------------------------------------------- MacroEnsamZ::compNMIof()
void MacroEnsamZ::compNMIof()
{
	if ( estado == ENCUERPOMETODO
	  || estado == ENEXCEPMETODO )
	{
		Metodo::CuerpoMetodo mnemos;
		std::string ref;
		Zero::NombreRegistro::CodigoRegistro reg = Zero::NombreRegistro::__ACC;
		Zero::NMIof * iof;

		lex->pasaEsp();
		ref = lex->getToken();
		if (ref.empty())
			throw Zero::ESintxIdNoValido( "se esperaba referencia de"
			                        " posible objeto padre" )
			;
		if (!Zero::NombreIdentificador::compruebaId(ref))
			throw Zero::ESintxIdNoValido(ref.c_str());

		lex->pasaEsp();
		lex->getToken();

		// Compilar la expresión que acompaña a IOF
		if (Zero::NombreRegistro::cnvtNombreACodigoRegistro(lex->getTokenActual())
		              != Zero::NombreRegistro::__NOREG)
		{
			reg = Zero::NombreRegistro::cnvtNombreACodigoRegistro(
				lex->getTokenActual() )
			;
		}
		else
		if ( !lex->getTokenActual().empty() ) {
			compilaExpresion(
				lex->getTokenActual(),
				mnemosMetActual,
				mnemos
			);

			// Ponerla en el log
			Metodo::listar( *log, mnemos );

			// Guardar los mnemos
			mnemosMetActual->masMnemos( mnemos );
		}

		// Generar
		iof = new(std::nothrow) Zero::NMIof( ref, reg );
		if ( iof != NULL )
				mnemosMetActual->masMnemos( iof );
		else  	throw Zero::ENoHayMemoria( "creando mnemo IOF" );

		(*log)( '\t' + iof->listar() );
	}
	else throw Zero::ESintxIofNoPermitido("");

	return;
}

// --------------------------------------------------- MacroEnsamZ::compNMJmp()
void MacroEnsamZ::compNMJmp(TipoSalto ts)
{
	std::string nombreMnemo;

	if ( estado == ENCUERPOMETODO
	  || estado == ENEXCEPMETODO )
	{
		Zero::Flw *mnemo;
		std::string etq;

		// Compilar la etiqueta
		lex->pasaEsp();
		etq = lex->getToken();
		if ( etq.empty() )
			throw Zero::ESintxIdNoValido("se esperaba nombre de etiqueta");
		if ( !Zero::NombreIdentificador::compruebaId( etq ) )
			throw Zero::ESintxIdNoValido( etq.c_str() );

		// Crear el mnemo
		if ( ts == Jmp ) {
            nombreMnemo = Zero::NMJmp::mnemoStr;
			mnemo = new Zero::NMJmp( etq );
		}
		else
                if ( ts == Jot ) {
                        nombreMnemo = Zero::NMJot::mnemoStr;
                        mnemo = new(std::nothrow) Zero::NMJot( etq );
                }
                else
                if ( ts == Jof ) {
                        nombreMnemo = Zero::NMJof::mnemoStr;
                        mnemo = new(std::nothrow) Zero::NMJof( etq );
                }
                else throw Zero::EInterno( "Salto extraño (?)" );

		// Poner en log
		(*log)('\t' + nombreMnemo + ' ' + etq);

		// Meterlo en la lista de mnemos
		if ( mnemo != NULL ) {
			mnemosMetActual->masMnemos( mnemo );
		}
		else {
			throw Zero::ENoHayMemoria( std::string( "creando mnemo salto "
                                             + nombreMnemo ).c_str() )
			;
		}
	} else throw Zero::ESintxJmpNoPermitido( nombreMnemo.c_str() );

	return;
}

// --------------------------------------------------- MacroEnsamZ::compNMEtq()
void MacroEnsamZ::compNMEtq()
{
	if ( estado == ENCUERPOMETODO
	  || estado == ENEXCEPMETODO )
	{
		// Se está definiendo una etiqueta: " :LOOP"
		lex->avanza(); // Pasar el ':'
		lex->pasaEsp();
		lex->getToken();

		Zero::NMEtq *etq = new(std::nothrow) Zero::NMEtq( lex->getTokenActual() );

		if ( etq != NULL )
                mnemosMetActual->masMnemos( etq );
		else	throw Zero::ENoHayMemoria( "creando mnemo ETQ" );

		(*log)( '\t' + etq->listar() );
	}
	else Zero::ESintxEtqNoPermitido( "" );

	return;
}

// ---------------------------------------------------- MacroEnsamZ::compExpr()
void MacroEnsamZ::compExpr()
{
	Zero::Mnemotecnico *mnemo;
	Metodo::CuerpoMetodo totalExpr;

	// Tomar el primer id
	std::string id = lex->getTokenActual();

	lex->pasaEsp();

	if ( lex->getCaracterActual() == '=' ) {
		// Comprobar ID
		if ( !buscaVbleAtributo( objetoActual, metodoActual, id ) )
		{
			throw Zero::ESintxIdNoValido( std::string( '\''
					+ id
					+ "' referencia local indefinida" ).c_str()
			);
		}

		// Expresion de asignacion
		// Pasar el '='
		lex->avanza();

		compilaExpresionOLiteral( totalExpr );

		mnemo = new(std::nothrow) Zero::NMAsg( id, Zero::NombreRegistro::__ACC );

		if ( mnemo != NULL )
				totalExpr.push_back( mnemo );
		else 	throw Zero::ENoHayMemoria( "creando mnemo ASG" );
	}
	else {
	// Una expresión que casi seguro resultaréen llamada a un método
	// Es, por tanto, de primer nivel
		compilaExpresion( id, mnemosMetActual, totalExpr, true );
	}

	// Listar en el log los mnemoténicos generados
	// NO APARECEN, por tanto, OPTIMIZADOS !!!
	Metodo::listar( *log, totalExpr );

	// Guardarlos
	mnemosMetActual->masMnemos( totalExpr );

	return;
}

// ------------------------------------------------ MacroEnsamZ::procesaLinea()
void MacroEnsamZ::procesaLinea()
{
        // Pasar los espacios
    lex->pasaEsp();
	std::string maysToken;

        // Es un comentario ? o séo espacios ? ... entonces, no hacer nada
    if ( lex->getCaracterActual() == MARCACOMENTARIO
	  || lex->esEol() )
        {
                goto FIN;
        }


	// Escribir la línea en el log
	{
		std::ostringstream aux;

		aux << "\n\n;ln#" << lin << ": " << getLineaActual() << '\n';

		(*log)(aux.str());
	}


	// A lo mejor es meta-información
	if ( lex->getCaracterActual() == MarcaComMeta ) {
		compNMMta();
		goto FIN;
	}

        // Obtener el token del mnemo
        maysToken = lex->getToken();
		maysToken = Zero::AnalizadorLexico::maysCnvt( maysToken );

        // Segn el mneméénico, actuar
        if ( maysToken == Zero::AnalizadorLexico::mays( Zero::NMObj::formatoXML ) ) {
 			compNMObj();
        }
        else
        if ( maysToken == Zero::AnalizadorLexico::mays( Zero::NMMth::formatoXML ) ) {
			compNMMth();
        }
        else
        if ( maysToken == Zero::AnalizadorLexico::mays( Zero::NMAtr::formatoXML ) ) {
			compNMAtr();
        }
        else
        if ( maysToken == Zero::AnalizadorLexico::mays( Zero::NMRet::formatoXML ) ) {
			compNMRet();
        }
        else
        if ( maysToken == Zero::AnalizadorLexico::mays( Zero::NMEnm::formatoXML ) ) {
			compNMEnm();
        }
        else
        if ( maysToken == Zero::AnalizadorLexico::mays( Zero::NMEno::formatoXML ) ) {
			compNMEno();
        }
        else
        if ( maysToken == Zero::AnalizadorLexico::mays( Zero::NMTrw::formatoXML ) ) {
			compNMTrw();
        }
        else
        if ( maysToken == Zero::AnalizadorLexico::mays( Zero::NMDef::formatoXML ) ) {
			compNMDef();
        }
        else
        if ( maysToken == Zero::AnalizadorLexico::mays( Zero::NMJmp::formatoXML ) ) {
 			compNMJmp( Jmp );
        }
        else
        if ( maysToken == Zero::AnalizadorLexico::mays( Zero::NMJot::formatoXML ) ) {
 			compNMJmp( Jot );
        }
        else
        if ( maysToken == Zero::AnalizadorLexico::mays( Zero::NMJof::formatoXML ) ) {
 			compNMJmp( Jof );
        }
        else
        if ( maysToken == Zero::AnalizadorLexico::mays( Zero::NMIof::formatoXML ) ) {
			compNMIof();
        }
	else
	if ( lex->getCaracterActual() == ':' )
	{
		compNMEtq();
	}
        else
        if ( estado == ENCUERPOMETODO
          || estado == ENEXCEPMETODO )
        {
			compExpr();
        }
        else throw Zero::ESintxMnemoInvalido( lex->getTokenActual().c_str() );

	FIN:
	return;
}

void MacroEnsamZ::compilaExpresionOLiteral(Metodo::CuerpoMetodo &instrucc)
{
	lex->pasaEsp();

	// Quéviene ahora?
	Zero::AnalizadorLexico::TipoToken tk = lex->getTipoSiguienteToken();

	// Coger la expresié
	if (tk == Zero::AnalizadorLexico::IDENTIFICADOR)
	{
		lex->getToken();

		compilaExpresion(lex->getTokenActual(), mnemosMetActual, instrucc);
	}
	else
	if (tk != Zero::AnalizadorLexico::NADA) {
		Zero::Mnemotecnico * lit = parseLiteral();

		if (lit != NULL) {
			instrucc.push_back(lit);
		}
		else throw Zero::ESintaxis("se esperaba literal");
	}
	else throw Zero::ESintaxis("se esperaba expresión");
}

// -------------------------------------------- MacroEnsamZ::compilaExpresion()
void MacroEnsamZ::compilaExpresion(const std::string &tok,
                                   Metodo *& met,
				   Metodo::CuerpoMetodo &instrucc,
				   bool primerNivel)
/*
    compilar una expresion de este estilo:
    		expr ::= [<ref>]+.<metodo>(<expr>)

          System.console.write(x.concat("Mundo!"))
    A:
          STR "Mundo!"
          ASG __gp1
          MSG x concat __gp1
          MSG System.console write __acc
*/
{
	Zero::Mnemotecnico *mnemo;
	GestorNombresLoc nl;

	lex->pasaEsp();

	if ( Zero::NombreIdentificador::compruebaId( tok )
	 || ( Zero::NombreReferencia::compruebaRef( tok )
	   && lex->getCaracterActual() != '(' ) )
	{
		if ( !primerNivel )
		{
			// Es un identificador, se asigna al acc y punto
			mnemo = new(std::nothrow) Zero::NMSet( Zero::NombreRegistro::__ACC, tok );

			if ( mnemo != NULL )
				instrucc.push_back( mnemo );
			else throw Zero::ENoHayMemoria( "creando mnemo SET" );
		} else throw Zero::ESintxIdNoValido(
				"se esperaba asignacié o mensaje"
			);
	}
	else {
		if (Zero::NombreReferencia::compruebaRef( tok ) )
		{
                        // Es un mensaje a un objeto
			std::string refObjeto = tok;
			size_t posUltimoPunto = refObjeto.rfind( '.' );
			std::string nombreMetodo = refObjeto.substr( posUltimoPunto + 1 );

			// borra '*.met'
			refObjeto.erase(posUltimoPunto, refObjeto.size());
			lex->pasaEsp();
                        if( refObjeto.empty() ) {
                            refObjeto = ".";
                        }

			// Pasar el paréntesis, si existe
			if ( lex->getCaracterActual() == '(' ) {
				lex->avanza();
			}
			else throw Zero::ESintxIdNoValido("se esperaba apertura de parétesis '('");

			Zero::AnalizadorLexico::TipoToken tk = lex->getTipoSiguienteToken();
			do {
				// Compilar la sub-expresión
				if (tk == Zero::AnalizadorLexico::LITNUMERICO) {
					Zero::Mnemotecnico * lit = parseLiteral();

					if ( lit == NULL ) {
						throw Zero::ESintxIdNoValido(
							"se esperaba literal"
							"numérico" );
					}

					instrucc.insert(
						instrucc.begin(),
						new Zero::NMAsg( nl.crearNuevo(),
							   Zero::NombreRegistro::__ACC )
					);
					instrucc.insert( instrucc.begin(), lit );
				}
				else
				if (tk == Zero::AnalizadorLexico::LITCADENA) {
					Zero::Mnemotecnico * lit = parseLiteral();

					if (lit == NULL)
						throw Zero::ESintxIdNoValido("se esperaba literal cadena");

					// Guardar el mensaje
					instrucc.insert(instrucc.begin(),
						new Zero::NMAsg( nl.crearNuevo(),
							   Zero::NombreRegistro::__ACC )
					);
					instrucc.insert( instrucc.begin(), lit );
				}
				else
				if (tk == Zero::AnalizadorLexico::IDENTIFICADOR) {
					// Tomar el identificador
					lex->getToken();

					// Si es un identificador, adelante
					if ( Zero::NombreIdentificador::compruebaId(
						 lex->getTokenActual() )
				           )
					{
						if ( buscaVble(
						        objetoActual,
							metodoActual,
						        lex->getTokenActual() ) )
						{
							nl.insertarNuevo(
							  lex->getTokenActual()
							);
						} else {
							compilaExpresion(
								lex->getTokenActual(),
								met,
								instrucc
							);

							instrucc.push_back(
							     new Zero::NMAsg(
							       nl.crearNuevo(),
							       Zero::NombreRegistro::__ACC)
							);
						}
					}
					else {
						if ( Zero::NombreReferencia::compruebaRef(
						           lex->getTokenActual() ) )
						{
							// Es una referencia,
							// llamada recursiva
							compilaExpresion(
								lex->getTokenActual(),
								met,
								instrucc )
							;

							// Pasar el ')'
							if ( lex->getCaracterActual() == ')' ) {
								lex->avanza();
							}

							instrucc.push_back(
							     new Zero::NMAsg(
							       nl.crearNuevo(), Zero::NombreRegistro::__ACC)
							);
						}
						else throw Zero::ESintxIdNoValido("se esperaba un identificador"
						                     " o referencia");
					}
				}

				lex->pasaEsp();
				tk = lex->getTipoSiguienteToken();
			} while(( lex->getCaracterActual() != ')')
			    &&  ( !lex->esEol() ) );

			// Crear el mensaje
			Zero::NMMsg *msg = new Zero::NMMsg( refObjeto, nombreMetodo );

			// Argumentos
			while(!(nl.esVacio())) {
				msg->masArgumentos( nl.getSigNombre() );
			}

			instrucc.push_back( msg );
		}
		else throw Zero::ESintxIdNoValido("expresión debe empezar "
                                            "por referencia o identificador")
		           ;
	}
}

// ------------------------------------------------ MacroEnsamZ::parseLiteral()
Zero::Mnemotecnico * MacroEnsamZ::parseLiteral()
{
	Zero::Mnemotecnico * toret = NULL;

	Zero::AnalizadorLexico::TipoToken tk = lex->getTipoSiguienteToken();
	lex->pasaEsp();

	if (tk == Zero::AnalizadorLexico::LITNUMERICO) {
		lex->getNumero();

		// Verificar
        if ( !Zero::AnalizadorLexico::compruebaFlotante( lex->getTokenActual() ) ) {
 	               throw Zero::ESintxFltNoValido(lex->getTokenActual().c_str());
		}

		// Es un número flotante o uno entero?
        Zero::Literal l( "", std::string( '+' + lex->getTokenActual() ) );
		if ( l.getTipo() == Zero::Literal::LitNumFlt )
			toret = new Zero::NMFlt( lex->getTokenActual() );
		else 	toret = new Zero::NMInt( lex->getTokenActual() );
	}
	else
	if (tk == Zero::AnalizadorLexico::LITCADENA) {
		// Llegar hasta las comillas
		if ( lex->getCaracterActual() != '"' ) {
			throw Zero::ESintxComillasEsperadas( "al inicio de literal "
						       "de cadena" );
		}

		// Pasar las comillas y coger el literal
		lex->avanza();
		std::string str = lex->getLiteral( '"' );

		// Pasar las comillas de cierre
		if ( lex->getCaracterActual() != '\"' ) {
			throw Zero::ESintxComillasEsperadas("al final del literal de"
							" cadena");
		}
		lex->avanza();

		// Guardar el mensaje
		toret = new Zero::NMStr( str );
	}

	return toret;
}

// ------------------------------------------------ MacroEnsamZ::generaCodigo()
void MacroEnsamZ::leeLinea()
/**
	Lee una línea del fichero de entrada y la coloca en el buffer.
	Tb. prepara un analizador léico para esa línea
*/
{
	// Eliminar el actual analizador léxico
	delete lex;

	// Borrar el buffer actual
	buffer.erase( buffer.begin(), buffer.end() );

	// Leer hasta encontrar un cambio de línea
	char ch = fgetc( ent );

	while ( ch != '\n'
	    && !feof( ent ) )
	{
		buffer += ch;

		ch = fgetc( ent );
	}

	// Crear el analizador léxico
	lex = new Zero::AnalizadorLexico( &buffer );

	if ( lex == NULL ) {
		throw Zero::ENoHayMemoria( "creando analizador léxico" );
	}

	return;
}

// ------------------------------------------------ MacroEnsamZ::generaCodigo()
void MacroEnsamZ::generaCodigo()
/**
	Bucle principal de la aplicacié. Lee una léea y la procesa hasta
	terminar el fichero de entrada.
*/
{
        do {
                leeLinea();
                procesaLinea();
                sigLinea();
        } while( !std::feof( ent ) );

//        sal->escribeBajoNivelUINT8( 0 );     // Marca de EOF
}

// ------------------------------------------------- MacroEnsamZ::getAtributo()
std::string MacroEnsamZ::getAtributo(const std::string &objeto, unsigned int i)
{
        Zero::IdsPorObjeto *ob;

        if ( ( ob = idsProg.buscaObjeto( objeto ) ) != NULL)
        {
                return ob->getAtributo( i );
        }
        else throw Zero::EInterno( std::string( "objeto '" + objeto + "' inexistente (?)" ).c_str() );
}

// --------------------------------------------------- MacroEnsamZ::buscaVble()
bool MacroEnsamZ::buscaVble(const std::string &objeto,
                            const std::string &met, const std::string &v) const
{
        Zero::IdsPorObjeto *ob;
        bool toret = false;

        if ( ( ob = idsProg.buscaObjeto( objeto ) ) != NULL )
        {
		if ( ob->buscaVbleMetodo( met, v ) ) {
			toret = true;
		}
	}

	return toret;
}

// ------------------------------------------- MacroEnsamZ::buscaVbleAtributo()
bool MacroEnsamZ::buscaVbleAtributo(const std::string &objeto,
                               const std::string &met, const std::string &v) const
{
        Zero::IdsPorObjeto *ob;
        Zero::IdsPorObjeto *obPadre;
        bool toret = false;

        if ((ob = idsProg.buscaObjeto(objeto)) != NULL)
        {
                if (!ob->buscaAtributo(v))
                {
                  if (!ob->buscaVbleMetodo(met, v))
                  {
                        obPadre = idsProg.buscaObjeto(ob->getNombrePadre());
                        while (obPadre != NULL
                          &&   !toret
                          &&   obPadre->getNombre() != Zero::OBJ_OBJECT)
                        {
                                if (obPadre->buscaAtributo(v)) {
                                        toret = true;
                                }

                                obPadre = idsProg.buscaObjeto(
                                                     obPadre->getNombrePadre());
                        }
                  } else toret = true;
                } else toret = true;
        } else toret = true;

        return toret;
}

// ------------------------------------------------- MacroEnsamZ::buscaMetodo()
bool MacroEnsamZ::buscaMetodo(const std::string &objeto, const std::string &met) const
{
        Zero::IdsPorObjeto *ob;
        Zero::IdsPorObjeto *obPadre;
        bool toret = false;

        if ((ob = idsProg.buscaObjeto(objeto)) != NULL)
        {
                if (!ob->buscaMetodo(met))
                {
                        obPadre = idsProg.buscaObjeto(ob->getNombrePadre());
                        while (obPadre != NULL
                          &&   !toret
                          &&   obPadre->getNombre() != Zero::OBJ_OBJECT)
                        {
                                if (obPadre->buscaMetodo(met))
                                {
                                        toret = true;
                                }

                                obPadre = idsProg.buscaObjeto(
                                                     obPadre->getNombrePadre());
                        }
                } else toret = true;
        } else toret = true;

        return toret;
}

// ================================================================ Metodo ====
// ------------------------------------------------------------------- Metodo()
Metodo::Metodo(
        const std::string &no,
        const std::string &nm,
        Zero::IdsPorObjeto *ids,
        Zero::MedioSoporte *s)
	        : nombreObjeto(no), nombreMetodo(nm), idsObj(ids)
{
        if ( idsObj == NULL ) {
                throw Zero::EInterno( "se esperaban ids de objeto creando Metodo" );
        }

//         if ( sal == NULL ) {
//                 throw EInterno( "se esperaba MedioSoporte creando Metodo" );
//         }
}

Metodo::~Metodo()
{
	for(unsigned int i = 0; i < cuerpo.size(); ++i) {
		delete cuerpo[i];
	}

	cuerpo.clear();
}

// ----------------------------------------------------------- Metodo::listar()
void Metodo::listar(Log &log, CuerpoMetodo const &instrucc)
{
	for(unsigned int i = 0; i < instrucc.size(); ++i) {
		log('\t' + instrucc[i]->listar() + '\n');
	}
}

// -------------------------------------------------------- Metodo::masMnemos()
void Metodo::masMnemos(Zero::Mnemotecnico *x)
{
	cuerpo.push_back( x );
}

// -------------------------------------------------------- Metodo::masMnemos()
void Metodo::masMnemos(CuerpoMetodo const &v)
{
	for(unsigned int i = 0; i < v.size(); ++i) {
		cuerpo.push_back( v[i] );
	}
}

// -------------------------------------------------------- Metodo::masMnemos()
void Metodo::escribe()
{
//         // Buscar los atributos del objeto y las loc refs del método
//         Identificadores * atrs = idsObj->getAtributos();
//         Identificadores * args = idsObj->getIdsMetodo( nombreMetodo );
//
//         if ( atrs == NULL
//           && args != NULL )
//         {
//                 throw EInterno( "ids no encontrados al intentar optimizar" );
//         }
//
//         // Optimizar el cuerpo método (lista mnemos)
// 	MacroEnsamZ::opt->optimizar( args, atrs, &cuerpo );
//
// 	// Escribir los opcodes al fichero
// 	for(unsigned int i = 0; i < cuerpo.size(); ++i) {
// 		cuerpo[i]->escribe( sal );
// 	}
}

// ================================ GestorNombresLoc ===========================
const std::string GestorNombresLoc::nombreLiteral = "_z_lit__";

// --------------------------------------- GestorNombresLoc::GestorNombresLoc()
GestorNombresLoc::GestorNombresLoc()
{
	regGenericos.push(Zero::LOC_GP4);
	regGenericos.push(Zero::LOC_GP3);
	regGenericos.push(Zero::LOC_GP2);
	regGenericos.push(Zero::LOC_GP1);
}

// --------------------------------------------- GestorNombresLoc::crearNuevo()
const std::string &GestorNombresLoc::crearNuevo()
{
	std::ostringstream aux;

	if (!regGenericos.empty()) {
		nombres.push(regGenericos.top());
		regGenericos.pop();
	}
	else {
		aux << nombreLiteral << nombres.size();
		nombres.push(aux.str());
	}

	return nombres.back();
}

// ------------------------------------------ GestorNombresLoc::insertarNuevo()
const std::string &GestorNombresLoc::insertarNuevo(const std::string & x)
{
	nombres.push(x);

	return nombres.back();
}

// ------------------------------------------- GestorNombresLoc::getSigNombre()
std::string GestorNombresLoc::getSigNombre()
{
	std::string toret = nombres.front();

	nombres.pop();

	return toret;
}

#ifdef TEXT_UI_MAIN
// ============================================================= main() ========

void warning(const std::string &x)
{
    std::cout << msgAVISO << x << std::endl;
}

void warning(MacroEnsamZ *zm, const std::string &x)
{

    std::cout << msgAVISO << x << std::endl;

    if ( zm != NULL )
    {
          std::cout  << " en: " << zm->getNumeroLinea()
                << ':'     << ' ' << zm->getLineaActual()
                << " token: \""
                << zm->getTokenActual() << '"' << '\n'
          ;

          if ( !zm->objetoActual.empty() ) {

            std::cout << "Compilando \"" << zm->objetoActual;

            if ( !zm->metodoActual.empty() ) {
                    std::cout   << '.'
                           << zm->metodoActual << "()";
            }
            std::cout << '\"' << '\n';
          }

          std::cout << std::endl;
    }
}

// ============================================================= main() ===
int main(int argc, char *argv[])
{
        std::cout << nombre       << '\n' << '\t' << version
             << " serial "   << serial
             << " Opcodes v" << (int) Zero::hver << '.' << (int) Zero::lver
             << '\n'         << std::endl;

        if (argc != 2)
        {
                std::cout << sintaxis << std::endl;
        }
        else {
		std::string fichBase( argv[1] );

		size_t posPunto = fichBase.rfind( '.' );
		if ( posPunto != std::string::npos ) {
                	fichBase.erase( posPunto, fichBase.length() );
		}

                std::string fichSalida = fichBase;

                MacroEnsamZ *zm = NULL;
				FILE * ent = fopen( argv[1], "rt" );

                try {
						if ( ent == NULL ) {
							throw Zero::EMedioNoEncontrado( argv[1] );
						}

                        zm = new MacroEnsamZ( ent, fichSalida );

                        zm->generaCodigo();

                        delete zm;
                        zm = NULL;
						delete ent;
						ent = NULL;
                        std::cout << "\nFinalizado: " << fichSalida << std::endl;
                }
                catch(const Zero::Excepcion &e)
                {
                        // Ocurrió un error
                        std::cout << e.getMensaje();
						if ( *( e.getDetalles() ) == 0 ) {
							std::cout << '('
											<< e.getDetalles()
							<< ')';
						}
						std::cout << '.' << std::endl;

                        // Montar un mensaje de error significativo
                        if (zm != NULL)
                        {
                              std::cout  << " en: " << zm->getNumeroLinea()
                                    << ':'     << ' ' << zm->getLineaActual()
                                    << "\ntoken: \""
                                    << zm->getTokenActual() << '"' << '\n'
                              ;

                              if (!zm->objetoActual.empty()) {

                                std::cout<< "Compilando \""      << zm->objetoActual;

                                if (!zm->metodoActual.empty()) {
                                        std::cout   << '.'
                                               << zm->metodoActual << "()";
                                }
                                std::cout << '\"' << '\n';
                              }

                              std::cout << std::endl;
                        }


                        std::exit(-1);
                }
				catch(...) {
					std::cout << "\nERROR FATAL: Sin memoria" << std::endl;
				}
        }

        return 0;
}


#endif



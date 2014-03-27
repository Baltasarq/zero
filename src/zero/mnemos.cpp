// mnemos.cpp
/*
      Cuerpos de las clases de gestión de los mnemotécnicos,
      la representación de las instrucciones.
      La funcionalidad reside en opcode.cpp

      jbgarcia@uvigo.es
*/

#include <cctype>
#include "mnemos.h"
#include "analizadorlexico.h"

namespace Zero {

// Variable de buffering global -----------------------------------------------
std::string buffer;  // El objetivo es evitar copias de cadena (costosas).

// Constantes generales -------------------------------------------------------
const UINT16 SIGNATURE  = 0xCED0;         // La firma en la cabecera

const UINT8      hver   = 3;
const UINT8      lver   = 0;

// Mnemotécnicos --------------------------------------------------------------
const UINT16     NOBJ   = 0xFF01;         // Definición de objeto
const UINT16     NATR   = 0xFF02;         // Definición de un atributo
const UINT16     NMTH   = 0xFF03;         // Definición de método
const UINT16     NDEF   = 0xFF04;         // Definición de referencia
const UINT16     NASG   = 0xFF05;         // Asignación: ref :- registro
const UINT16     NMSG   = 0xFF06;         // Mensaje: obj.metodo()
const UINT16     NIOF   = 0xFF07;         // Test de Tipo: <obj>classof<clase>
const UINT16     NRET   = 0xFF08;         // return: fín de cuerpo de un método
const UINT16     NENO   = 0xFF09;         // fín de objeto-prototipo
const UINT16     NENM   = 0xFF0A;         // fín de método
const UINT16     NSTR   = 0xFF0B;         // Crear cadena
const UINT16     NTRW   = 0xFF0C;         // Lanza una excepción
const UINT16     NFLT   = 0xFF0D;         // Introduce un número flotante
const UINT16     NSET   = 0xFF0E;         // Asigna referencia al acumulador
const UINT16     NJMP   = 0xFF0F;         // Salta incondicionalmente x opcodes
const UINT16     NJOT   = 0xFF10;         // Salta condicionalmente x opcodes
const UINT16     NJOF   = 0xFF11;         // Salta condicionalmente x opcodes
const UINT16     NINT   = 0xFF12;         // Introduce un número entero
const UINT16     NETQ   = 0xFF13;         // Representa las etqs en los saltos
const UINT16     NNOP   = 0xFF14;         // Representa la no-operación
const UINT16     NMTA   = 0xFF15;         // Proporciona meta-información

// ========================================================= MixinConArgumentos
const MixinConArgumentos::Argumentos MixinConArgumentos::argumentosNulos;

// ----------------------------------- MixinConArgumentos::copiarArgumentosDe()
void MixinConArgumentos::copiarArgumentosDe(const MixinConArgumentos &x)
{
	copiarArgumentos( *getArgs(), *x.getArgs() );
}

// ------------------------------------- MixinConArgumentos::copiarArgumentos()
void MixinConArgumentos::copiarArgumentos(Argumentos &dest, const Argumentos &orig)
/**
	Copia los argumentos de un contenedor de argumentos
	a otro, presumiblemente de un mnemotécnico a otro.
*/
{
        for(register unsigned int i = 0; i < orig.size(); ++i) {
            dest.push_back( orig[i]->copia() );
        }
}

// ------------------------------------ MixinConArgumentos::eliminaArgumentos()
void MixinConArgumentos::eliminaArgumentos()
/**
	Borra todos los argumentos
*/
{
        for(register unsigned int i = 0; i < args.size(); ++i) {
                delete args[i];
        }

        args.clear();
}

// ---------------------------------------- MixinConArgumentos::masArgumentos()
void MixinConArgumentos::masArgumentos(const std::string &x)
/**
	Añade un argumento más a la colección de argumentos
	de este mnemotécnico
	En este caso, se trata de un identificador
*/
{
	args.push_back( Nombre::creaNombreAdecuado( x ) );
}

// --------------------------------- MixinConArgumentos::masArgumentos()
void MixinConArgumentos::masArgumentos(NombreRegistro::CodigoRegistro x)
/**
	Añade un argumento más a la colección de argumentos
	de este mnemotécnico
	En este caso, se trata de un registro
*/
{
        NombreRegistro * rg = new(std::nothrow) NombreRegistro( x );

        if ( rg == NULL ) {
            throw ENoHayMemoria( std::string( "creando argumento " + rg->getNombre() ).c_str() );
        }

        args.push_back( rg );
}

// ---------------------------------------- MixinConArgumentos::leeVectorArgs()
void MixinConArgumentos::leeVectorArgs(MedioSoporte *m)
{
	leeVectorArgs( m, *getArgs() );
}

// ------------------------------------ MixinConArgumentos::escribeVectorArgs()
void MixinConArgumentos::escribeVectorArgs(MedioSoporte *m)
{
	escribeVectorArgs( m, *getArgs() );
}

// ------------------------------------ MixinConArgumentos::escribeVectorArgs()
void MixinConArgumentos::escribeVectorArgs(
	MedioSoporte *sal,
	const Argumentos &args )
/**
	Escribe un vector de argumentos al bytecode.
	Los vectores de argumentos son utilizados en MTH,
	para guardar los parámetros formales,
	y en MSG para guardar los argumentos reales
*/
{
	UINT8 numArgs = (UINT8) args.size();

        // Número de argumentos
        sal->escribeBajoNivelUINT8( numArgs );

        // Meter los argumentos
        for(register UINT8 i = 0; i < numArgs; ++i)
        {
                args[i]->escribe( sal );
        }
}

// ---------------------------------------- MixinConArgumentos::leeVectorArgs()
void MixinConArgumentos::leeVectorArgs(
	MedioSoporte *ent,
	Argumentos &args)
/**
	Escribe un vector de argumentos al bytecode.
	Los vectores de argumentos son utilizados en MTH,
	para guardar los parámetros formales,
	y en MSG para guardar los argumentos reales
*/
{
        UINT8 numArgs;

        // Número de argumentos
        numArgs = ent->leeBajoNivelUINT8();

        // Leer los argumentos
        for(register UINT8 i = 0; i < numArgs; ++i)
        {
              args.push_back( Nombre::lee( ent ) );
        }
}

// ============================================================== Mnemotecnico
// ----------------------------------------------------- Mnemotecnico::cargar()
Mnemotecnico *Mnemotecnico::cargar(MedioSoporte *m)
/**
	Carga un mnemotécnico de un fichero, creando el objeto adecuado.
	Los mnemotécnicos son básicamente opcodes que no están en memoria.

	El final del fuchero viene marcado como un cero en lo que sería
	el siguiente código de mnemotécnico.
*/
{
      UINT16 mnemo = 0;
      Mnemotecnico *toret = NULL;

      // Leer el mnemotécnico
      mnemo = m->leeBajoNivelUINT16();

      // crear el objeto adecuado
      try {
        switch( mnemo ) {
            case NOBJ: toret = new NMObj();
                       break;
            case NENO: toret = new NMEno();
                       break;
            case NENM: toret = new NMEnm();
                       break;
            case NMTH: toret = new NMMth();
                       break;
            case NATR: toret = new NMAtr();
                       break;
            case NDEF: toret = new NMDef();
                       break;
            case NRET: toret = new NMRet();
                       break;
            case NSET: toret = new NMSet();
                       break;
            case NASG: toret = new NMAsg();
                       break;
            case NSTR: toret = new NMStr();
                       break;
            case NTRW: toret = new NMTrw();
                       break;
            case NMSG: toret = new NMMsg();
                       break;
            case NJMP: toret = new NMJmp();
                       break;
            case NJOT: toret = new NMJot();
                       break;
            case NJOF: toret = new NMJof();
                       break;
            case NFLT: toret = new NMFlt();
                       break;
            case NIOF: toret = new NMIof();
                       break;
            case NINT: toret = new NMInt();
                       break;
            case NNOP: toret = new NMNop();
                       break;
            case NETQ: toret = new NMEtq();
                       break;
			case NMTA: toret = new NMMta();
	    	       break;
            default :  toret = NULL;
        };
      } catch(const std::bad_alloc &) {
                throw ENoHayMemoria( "mnemo.cargar()" );
      }

      // Leer el mnemotécnico real
      if ( toret != NULL ) {
            toret->lee( m );
      }

      return toret;
}

// ---------------------------------------------------- Mnemotecnico::escribe()
void Mnemotecnico::escribe(MedioSoporte *m)
{
	m->escribeBajoNivelUINT16( mnemo );
}

// -------------------------------------------------------- Mnemotecnico::lee()
void Mnemotecnico::lee(MedioSoporte *m, bool leeMnemo)
{
	mnemo = m->leeBajoNivelUINT16();
}

// --------------------------------------------------- Mnemotecnico::compilar()
Mnemotecnico *Mnemotecnico::compilar(AnalizadorLexico &lex)
/**
	Según el primer token de la cadena, devuelve ya el mnemotécnico
	compilado según al que corresponda.
	En caso de error, devuelve NULL
*/
{
	Mnemotecnico * toret = NULL;

    // Obtener el mnemotécnico en formato de cadena
	std::string &mnemo = AnalizadorLexico::maysCnvt( lex.getToken() );

	// Retroceder lo suficiente ...
	lex.avanza( mnemo.length() * -1 );

	// crear el objeto adecuado
	if ( mnemo == NMObj::mnemoStr ) {
		toret = NMObj::compilar( lex );
	}
	else
	if ( mnemo == NMEno::mnemoStr ) {
		toret = NMEno::compilar( lex );
	}
	else
	if ( mnemo == NMEnm::mnemoStr ) {
		toret = NMEnm::compilar( lex );
	}
	else
	if ( mnemo == NMMth::mnemoStr ) {
		toret = NMMth::compilar( lex );
	}
	else
	if ( mnemo == NMAtr::mnemoStr ) {
		toret = NMAtr::compilar( lex );
	}
	else
	if ( mnemo == NMDef::mnemoStr ) {
		toret = NMDef::compilar( lex );
	}
	else
	if ( mnemo == NMRet::mnemoStr ) {
		toret = NMRet::compilar( lex );
	}
	else
	if ( mnemo == NMSet::mnemoStr ) {
		toret = NMSet::compilar( lex );
	}
	else
	if ( mnemo == NMAsg::mnemoStr ) {
		toret = NMAsg::compilar( lex );
	}
	else
	if ( mnemo == NMStr::mnemoStr ) {
		toret = NMStr::compilar( lex );
	}
	else
	if ( mnemo == NMTrw::mnemoStr ) {
		toret = NMTrw::compilar( lex );
	}
	else
	if ( mnemo == NMMsg::mnemoStr ) {
		toret = NMMsg::compilar( lex );
	}
	else
	if ( mnemo == NMJmp::mnemoStr ) {
		toret = NMJmp::compilar( lex );
	}
	else
	if ( mnemo == NMJot::mnemoStr ) {
		toret = NMJot::compilar( lex );
	}
	else
	if ( mnemo == NMJof::mnemoStr ) {
		toret = NMJof::compilar( lex );
	}
	else
	if ( mnemo == NMFlt::mnemoStr ) {
		toret = NMFlt::compilar( lex );
	}
	else
	if ( mnemo == NMIof::mnemoStr ) {
		toret = NMIof::compilar( lex );
	}
	else
	if ( mnemo == NMInt::mnemoStr ) {
		toret = NMInt::compilar( lex );
	}
	else
	if ( mnemo == NMNop::mnemoStr ) {
		toret = NMNop::compilar( lex );
	}
	else
	if ( mnemo == NMEtq::mnemoStr ) {
		toret = NMEtq::compilar( lex );
	}
	else
	if ( mnemo == NMMta::mnemoStr ) {
		toret = NMMta::compilar( lex );
	}

	return toret;
}

// ====================================================================== Nombre
// --------------------------------------------------------------- Nombre::lee()
Nombre * Nombre::lee(MedioSoporte *m)
/// Lee un Nombre, puede ser bien un registro o una referencia
{
        UINT8 rc;
	Nombre * toret;

        if ( m->leeRegistro( rc ) ) {
            toret = new(std::nothrow) NombreRegistro( ( NombreRegistro::CodigoRegistro ) rc );

            if ( toret == NULL ) {
                    throw ENoHayMemoria( "creando registro" );
            }
	}
        else {
		std::string ref;

		toret = creaNombreAdecuado( m->leeReferencia( ref ) );
	}

        return toret;
}

// -------------------------------------- Nombre::creaIdentificadorOReferencia()
Nombre * Nombre::creaNombreAdecuado(const std::string &id)
/**
 * creaNombreAdecuado()
 *
 * Crea un identificador, una referencia o un registro según lo que se le pase:
 * <ul>
 *      <li> __acc, __rr, __gp1 ... Crea un registro
 *      <li> Una cadena sin "puntos" ... Crea un identificador
 *      <li> Una cadena con "puntos" ... Crea una referencia
 * </ul>
 * @param id La cadena a valorar
 * @return Un objeto adecuado de una clase derivada de Nombre
 */
{
	Nombre * toret;
	NombreRegistro::CodigoRegistro cr =
				  NombreRegistro::cnvtNombreACodigoRegistro( id )
	;

	if ( cr != NombreRegistro::__NOREG ) {
		toret = new(std::nothrow) NombreRegistro( cr );
	}
	else
	if ( id.find( CHR_SEP_IDENTIFICADOR ) != std::string::npos )
        	toret =  new(std::nothrow) NombreReferencia( id );
	else	toret =  new(std::nothrow) NombreIdentificador( id );

	if ( toret == NULL ) {
		throw ENoHayMemoria( std::string( "creando reg/ref/id: " + id ).c_str() );
	}

    return toret;
}

// ==================================================================== Registro
// Importante: definir las posiciones en el mismo orden que en el enum
const std::string * NombreRegistro::strReg[NombreRegistro::numRegistros] = {
 	&LOC_ACC,
 	&LOC_THIS,
 	&LOC_RR,
 	&LOC_EXC,
 	&LOC_GP1,
 	&LOC_GP2,
 	&LOC_GP3,
 	&LOC_GP4
};

// ---------------------------------------------------- NombreRegistro::copia()
NombreRegistro * NombreRegistro::copia() const
{
    NombreRegistro * toret = new(std::nothrow) NombreRegistro( *this );

    if ( toret == NULL ) {
        throw ENoHayMemoria( "copiando nombre de registro" );
    }

    return toret;
}

// ------------------------------------------------- NombreRegistro::verifica()
bool NombreRegistro::verifica() const
/// Verifica que sea realmente un registro
{
        if ( rc == __NOREG )
                return false;
        else 	return ( cnvtCodigoRegistroANombre( rc ) != LOC_ERROR );
}

// ------------------------------------------------------ NombreRegistro::lee()
NombreRegistro *NombreRegistro::lee(MedioSoporte *m)
/// Lee un registro del bytecode
{
        UINT8 x;
        NombreRegistro *toret = NULL;

        if ( m->leeRegistro( x ) )
        {
                toret = new(std::nothrow) NombreRegistro(
                                ( NombreRegistro::CodigoRegistro ) x )
                ;

		if ( toret == NULL ) {
			throw ENoHayMemoria( "creando registro" );
		}
        }

        return toret;
}

// -------------------------------------------------- NombreRegistro::escribe()
inline
void NombreRegistro::escribe(MedioSoporte *m) const
/// Escribe un registro al bytecode
{
        m->escribeRegistro( getCodigoRegistro() );
}

// -------------------------------- NombreRegistro::cnvtNombreACodigoRegistro()
NombreRegistro::CodigoRegistro
NombreRegistro::cnvtNombreACodigoRegistro(const std::string &x)
/**
 * Convierte una cadena, si se corresponde con un registro,
 * en su código de registro.
 *
 * En caso de no haber correspondencia, devuelve el código __NOREG
 *
*/
{
	register unsigned int i = 0;

	for(; i < numRegistros; ++i) {
		if ( *( strReg[i] ) == x ) {
			break;
		}
	}

	if ( i < numRegistros )
		return (CodigoRegistro) i;
	else	return __NOREG;
}

// -------------------------------- NombreRegistro::cnvtCodigoRegistroANombre()
const std::string &NombreRegistro::cnvtCodigoRegistroANombre(CodigoRegistro x)
/**
 * Convierte un número (código de registro) en el nombre de su registro
 * correspondiente, en forma de cadena.
 *
 * En caso de no haber correspondencia, devuelve la cadena de error LOC_ERROR
 *
*/
{
	if ( ( (unsigned int) x ) < numRegistros )
            return *( strReg[x] );
	else 	return LOC_ERROR;
}

// ================================================================= Referencia
// --------------------------------------- NombreReferencia::NombreReferencia()
NombreReferencia::NombreReferencia(const std::string &n) : nombre(n)
{
	if ( !verifica() ) {
	   throw ESintxIdNoValido( std::string( "construyendo ref: '" + nombre + '\'' ).c_str() );
	}
}

// ---------------------------------------------------- NombreRegistro::copia()
NombreReferencia * NombreReferencia::copia() const
{
    NombreReferencia * toret = new(std::nothrow) NombreReferencia( *this );

    if ( toret == NULL ) {
        throw ENoHayMemoria( std::string( "copiando nombre de referencia" ).c_str() );
    }

    return toret;
}

// ---------------------------------------------------- NombreReferencia::lee()
NombreReferencia * NombreReferencia::lee(MedioSoporte *m)
/// Lee una referencia del bytecode
{
	NombreReferencia * toret;
        std::string ref;

	toret = new(std::nothrow) NombreReferencia( m->leeReferencia( ref ) );

	if ( toret == NULL ) {
		throw ENoHayMemoria( std::string( "creando ref: " + ref ).c_str() );
	}

	return toret;
}

// ------------------------------------------------ NombreReferencia::escribe()
void NombreReferencia::escribe(MedioSoporte *m) const
/// Escribe una referencia al bytecode
{
        m->escribeReferencia( getNombre() );
}

// ------------------------------------------- NombreReferencia::compruebaRef()
bool NombreReferencia::compruebaRef(const std::string &s)
/**
	Comprueba que la cadena pasada sea una referencia válida.
	P.ej. System.console
*/
{
      bool toret = true;
      size_t posInicial   = 0;
      size_t posSeparador = s.find( CHR_SEP_IDENTIFICADOR );
      std::string id;

      if ( !s.empty() )
      {
	// Quizás es una referencia global, empezando por '.'
	// Esto es correcto. La búsqueda de ids ahora empieza en uno.
	if ( s[0] == CHR_SEP_IDENTIFICADOR ) {
		++posInicial;
		posSeparador = s.find( CHR_SEP_IDENTIFICADOR, 1 );
	}

	// Comprobar el resto del identificador
	while( posInicial < s.length() ) {
		// Quizás es una referencia de un solo identificador ...
		if ( posSeparador == std::string::npos ) {
			posSeparador = s.length();
		}

		// Obtener la secuencia del identificador
		id = s.substr( posInicial, posSeparador - posInicial );

		// Comprobarla
		if ( !NombreIdentificador::compruebaId( id )
		  && !NombreIdentificador::compruebaIdMetodo( id ) )
		{
			toret = false;
			break;
		}

		// Preparar lo que resta
		posInicial   = posSeparador + 1;
		posSeparador = s.find( CHR_SEP_IDENTIFICADOR, posInicial );
	}
      } else toret = false;

      return toret;
}

// =======================================================  NombreIdentificador
// --------------------------------- NombreIdentificador::NombreIdentificador()
NombreIdentificador::NombreIdentificador(const std::string &n) : nombre(n)
{
	if ( !verifica() ) {
	   throw ESintxIdNoValido( std::string( "construyendo id: '" + nombre + '\'' ).c_str() );
	}
}

// ----------------------------------------------- NombreIdentificador::copia()
NombreIdentificador * NombreIdentificador::copia() const
{
    NombreIdentificador * toret = new(std::nothrow) NombreIdentificador( *this );

    if ( toret == NULL ) {
        throw ENoHayMemoria( std::string( "copiando nombre de identificador" ).c_str() );
    }

    return toret;
}

//  ------------------------------------------------ NombreIdentificador::lee()
NombreIdentificador * NombreIdentificador::lee(MedioSoporte *m)
/// Lee un identificador del bytecode
{
    std::string ref;
	NombreIdentificador * toret = new(std::nothrow) NombreIdentificador(
		m->leeIdentificador( ref )
	);

	if ( toret == NULL ) {
		throw ENoHayMemoria( std::string( "creando identificador: " + ref ).c_str() );
	}

        return toret;
}

/// -------------------------------------------- NombreIdentificador::escribe()
void NombreIdentificador::escribe(MedioSoporte *m) const
/// Escribe un identificador al bytecode
{
        m->escribeIdentificador( getNombre() );
}

// ----------------------------------------- NombreIdentificador::compruebaId()
bool NombreIdentificador::compruebaId(const std::string &s)
/**
	Comprueba que el identificador pasado es válido
	P.ej. x, nombreDePrueba, nombre_de_prueba, edad74
*/
{
      bool toret = true;

      if ( s.size() < MaxTamIdentificadores
        && !s.empty()
        && ( isalpha( s[0] )
          || s[0] == '_' ) )
      {
            for(register unsigned int n = 1; n < s.length(); ++n)
            {
                  if ( !isdigit( s[n] )
                    && !isalpha( s[n] )
                    && s[n]!='_' )
                  {
                        toret = false;
                        break;
                  }
            }
      }
      else toret = false;

      return toret;
}

void NombreIdentificador::chkId(const std::string &id) throw(ESintaxis)
{
    if ( !compruebaId( id ) ) {
        throw ESintxIdNoValido( id.c_str() );
    }
}

// ----------------------------------- NombreIdentificador::compruebaIdMetodo()
bool NombreIdentificador::compruebaIdMetodo(const std::string &s)
/**
	Utiliza compruebaID() para saber si el nombre del método es correcto.
	La razón de tener otro es que puede ir precedido de '^'
*/
{
      bool toret = false;

      if ( isalpha( s[0] )
        || s[0] == '_' )
      {
                toret = compruebaId( s );
      }
      else {
        if ( s[0] == CHR_MET_SUPER ) {
                std::string auxNombreMetodo = s.substr( 1, s.size() );
                toret                  = compruebaId( auxNombreMetodo );
        }
      }

      return toret;
}

// =================================================================== Cabecera
// -------------------------------------------------------- Cabecera::escribe()
void Cabecera::escribe(MedioSoporte *m)
/// Escribe una cabecera de bytecodes
{
        Mnemotecnico::escribe( m );

        m->escribeBajoNivelUINT16( byteOrd );
        m->escribeBajoNivelUINT16( ver );
}

// ------------------------------------------------------------ Cabecera::lee()
void Cabecera::lee(MedioSoporte *m, bool leeMnemo)
/// Lee una cabecera de bytecodes.
{
        if ( leeMnemo ) {
                Mnemotecnico::lee( m );
        }

        byteOrd = m->leeBajoNivelUINT16();
        ver     = m->leeBajoNivelUINT16();
}

// --------------------------------------------------------- Cabecera::listar()
std::string Cabecera::listar(bool bello)
/// Produce una línea de comentario con la info de la cabecera
{
      std::string toret;

      toret = "; Zero v";
      toret += AnalizadorLexico::toString( hver, 2 );
      toret += '.';
      toret += AnalizadorLexico::toString( lver, 2 );

      return toret;
}

// -------------------------------------------------- Cabecera::getFormatoXML()
std::string Cabecera::getFormatoXML()
/// Produce un comentario XML con la info de la cabecera
{
      std::string toret;

      toret = "<!-- Zero v" + AnalizadorLexico::toString( hiVersion() );
      toret += '.';
      toret += AnalizadorLexico::toString( loVersion() );

      toret += ' ';
      toret += '-';
      toret += '-';
      toret += '>';

      return toret;
}

// ============================================================== Mnemotécnicos
// ====================================================================== NMMta
/*typedef enum { FinalDeclConstantes, Objeto, Error } Pragma;
*/

const std::string NMMta::formatoXML = "Meta";
const std::string NMMta::mnemoStr   = "MTA";

const std::string NMMta::strPragma[] = {
	".CONSTEND", "OBJ", "ERROR"
};

// ------------------------------------------------------------- NMMta::NMMta()
NMMta::NMMta(const std::string &pr, const std::string &dt)
	: Mnemotecnico(NMTA), datos(dt)
{
    strToPragma( pr );
	verifica();
}

NMMta::NMMta(const Pragma &pr, const std::string &dt)
	: Mnemotecnico(NMTA), pragma(pr), datos(dt)
{
	verifica();
}

NMMta::~NMMta()
{
}

// ---------------------------------------------------- NMMta::pragmaToString()
const std::string &NMMta::getPragmaAsString() const
/**
* Convierte el valor numérico del pragma a una cadena
* @return la cadena correspondiente al pragma del mnemo
*/
{
        if ( pragma == Objeto )
                return getIdObj();
	else    return cnvtPragmaToStr( pragma );
}

// ---------------------------------------------------------- NMMta::verifica()
void NMMta::verifica() throw (ECompilacion)
/**
 * Verifica los datos (en este caso, el pragma) del mnemo
 * Si alguno no cumple con lo establecido, se lanza una excepción.
**/
{
	if ( pragma >= Error
	  || pragma < FinalDeclConstantes )
	{
		throw ESintxIdNoValido( "MTA: pragma incorrecto" );
	}

        if ( pragma == Objeto
          && !NombreIdentificador::compruebaId( idObj ) ) {
                throw ESintxIdNoValido( std::string(
                                        "MTA: es necesario un id correcto "
                                        "si pragma es objeto, no: '"
                                        + idObj + '\'' ).c_str()
                );
        }
}

// ----------------------------------------------------------- NMMta::escribe()
void NMMta::escribe(MedioSoporte *m)
/// Escribe el mnemo en el dispositivo de almacenamiento
{
	verifica();

        Mnemotecnico::escribe( m );

	m->escribeBajoNivelINT16( pragma );
	m->escribeStr( datos );

        if ( pragma == Objeto ) {
                m->escribeStr( idObj );
        }
}

// --------------------------------------------------------------- NMMta::lee()
void NMMta::lee(MedioSoporte *m, bool leeMnemo)
/// Lee el mnemo del dispositivo de almacenamiento
{
	if ( leeMnemo ) {
        	Mnemotecnico::lee( m );
	}

	pragma = (Pragma) m->leeBajoNivelINT16();
	m->leeStr( datos );

        if ( pragma == Objeto ) {
                m->leeStr( idObj );
        }

	verifica();
}

// ------------------------------------------------------------ NMMta::listar()
std::string NMMta::listar(bool bello)
/// Lista el mnemo como texto
{
      std::string toret;

      if ( !bello ) {
        toret = mnemoStr;

        toret += ' ';
        toret += getPragmaAsString();
        toret += ' ';
	toret += '"';
        toret += getDatos();
	toret += '"';
      }
      else {
        toret = '[';

	toret += getPragmaAsString();

	if ( !datos.empty() ) {
		toret += '=';
		toret += '"';
		toret += getDatos();
		toret += '"';
	}

        toret += ']';
      }

      return toret;
}

// ----------------------------------------------------- NMMta::getFormatoXML()
std::string NMMta::getFormatoXML()
/// Descripción XML de este mnemo
{
      std::string toret;

      toret += '<';
      toret += formatoXML;
      toret += " pragma=\"";
      toret += getPragmaAsString();
      toret += "\" data=\"";
      toret += getDatos();
      toret += "\">";

      return toret;
}

// -------------------------------------------------------- NMMta::strToPragma()
NMMta::Pragma NMMta::strToPragma(const std::string &x)
{
        // Limpiar cadena asociada con nombre de objeto propiedades
        idObj.clear();

        // Convertir el pragma
        pragma = cnvtStrToPragma( x );

        // Dar valor a la cadena asociada (si es necesario)
        if ( pragma == Objeto ) {
                idObj = x;
        }

        return pragma;
}

// --------------------------------------------------- NMMta::cnvtPragmaToStr()
const std::string &NMMta::cnvtPragmaToStr(Pragma pragma)
/**
        Convierte pragma a cadena.
        NO devuelve el objeto asociado, si es objeto propiedades
        getIdObj() -- para lo último.
*/
{
	if ( pragma > Error
	  || pragma < FinalDeclConstantes )
	{
		return strPragma[Error];
	}
	else return strPragma[pragma];
}

// --------------------------------------------------- NMMta::cnvtStrToPragma()
NMMta::Pragma NMMta::cnvtStrToPragma(const std::string & x)
/**
        Si la cadena empieza por punto, señala un pragma conocido
        En otro caso, señala un objeto de propiedades
*/
{
	register unsigned int i = 0;

	for(; i < (unsigned int) Error; ++i)
	{
                if ( ( (Pragma) i ) == Objeto
                  && NombreIdentificador::compruebaId( x ) )
                {
                        break;
                }

		if ( strPragma[i] == x) {
			break;
		}
	}

	return (Pragma) i;
}

// ---------------------------------------------------------- NMMta::compilar()
NMMta *NMMta::compilar(AnalizadorLexico &lex)
{
	std::string prg;
	NMMta * toret = NULL;

	AnalizadorLexico::maysCnvt( lex.getToken() );

	if ( lex.getTokenActual() == mnemoStr )
	{
		// Tomar el pragma
		prg = lex.getToken();

		// Llegar hasta las comillas
		lex.pasaEsp();

		// Porque ... hay comillas, ¿no?
		if ( lex.getCaracterActual() != '"')
			throw ESintxComillasEsperadas( std::string(
				"compilando " + mnemoStr +
				", \"" ).c_str() )
			;

		// Pasar las comillas y obtener literal
		lex.avanza();
		lex.getLiteral( '"' );

		if ( lex.esEol() ) {
			throw ESintxComillasEsperadas( std::string(
				"compilando " + mnemoStr
				+ ", \"" ).c_str()
			);
		}

		// Pasa las comillas
		lex.avanza();

		toret = new(std::nothrow) NMMta( prg, lex.getTokenActual() );

		if ( toret == NULL ) {
			throw ENoHayMemoria( "creando NMMta" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ======================================================================= NMObj
const std::string NMObj::formatoXML = "Object";
const std::string NMObj::mnemoStr   = "OBJ";

// -------------------------------------------------------------- NMObj::NMObj()
NMObj::NMObj() : CtrOb(NOBJ), padre(NULL)
{}

NMObj::NMObj(const std::string &nom, const std::string &p) :
	CtrOb(NOBJ), nombre(nom)
{
	padre = Nombre::creaNombreAdecuado( p );

	verifica();
}

NMObj::~NMObj()
{
	delete padre;
	padre = NULL;
}

// ---------------------------------------------------------- NMObj::verifica()
void NMObj::verifica() throw (ECompilacion)
{
	if ( dynamic_cast<NombreRegistro *>( padre ) != NULL ) {
		throw ESintxIdNoValido( "Se esperaba id o ref, no registro." );
	}

	if ( !NombreIdentificador::compruebaId( nombre ) ) {
		throw ESintxIdNoValido( std::string(
			"NMObj: id nombre incorrecto: "
			+ nombre ).c_str()
		);
	}

	if ( nombre == padre->getNombre() ) {
		throw ESintxIdNoValido( std::string( "Id de objeto '"
								+ nombre
								+ "' y padre son iguales" ).c_str()
		);
	}
}

// ------------------------------------------------------------ NMObj::escribe()
void NMObj::escribe(MedioSoporte *m)
/// Escribe un mnemotécnico NMObj al bytecode
{
	verifica();

        Mnemotecnico::escribe( m );

        m->escribeIdentificador( nombre );
	padre->escribe( m );
}

// --------------------------------------------------------------- NMObj::lee()
void NMObj::lee(MedioSoporte *m, bool leeMnemo)
/// Lee un mnemotécnico NMObj del bytecode
{
	if ( leeMnemo ) {
        	Mnemotecnico::lee( m );
	}

	// Recuperar el nombre
        m->leeIdentificador( nombre );

	// Recuperar la referencia al nombre
	padre = NombreReferencia::lee( m );

	verifica();
}

// ------------------------------------------------------------ NMObj::listar()
std::string NMObj::listar(bool bello)
/// Devuelve una descripción de este mnemotécnico en ensamblador
{
      std::string toret;

      if ( !bello ) {
        toret = mnemoStr;

        toret += ' ';
        toret += nombre;
        toret += ' ';
        toret += padre->getNombre();
      }
      else {
        toret = formatoXML;

        toret += ' ';
        toret += nombre;
        toret += ':';
        toret += ' ';
        toret += padre->getNombre();
        toret += " {";
      }


      return toret;
}

// ----------------------------------------------------- NMObj::getFormatoXML()
std::string NMObj::getFormatoXML()
/// Devuelve una descripción de este mnemotécnico en formato XML
{
      std::string toret;

      toret += '<';
      toret += formatoXML;
      toret += " name=\"";
      toret += nombre;
      toret += "\" parent=\"";
      toret += padre->getNombre();
      toret += "\">";

      return toret;
}

// ---------------------------------------------------------- NMObj::compilar()
NMObj *NMObj::compilar(AnalizadorLexico &lex)
/// Genera un mnemotécnico a partir de una cadena
{
	NMObj * toret = NULL;
	std::string nombre;

	AnalizadorLexico::maysCnvt( lex.getToken() );
	if ( lex.getTokenActual() == mnemoStr )
	{
		// Coger nombre de objeto
		lex.getToken();

		if ( lex.getTokenActual().empty() ) {
			throw ESintxIdNoValido(
				   std::string( "compilando " + mnemoStr + ", "
			           "se esperaba nombre de objeto, y no "
				 + lex.getTokenActual() ).c_str() )
			;
		}
		if ( !NombreIdentificador::compruebaId( lex.getTokenActual() ) )
			throw ESintxIdNoValido( lex.getTokenActual().c_str() );

		// Coge nombre de objeto padre
		nombre      = lex.getTokenActual();

		lex.pasaSoloEsp();
		if ( lex.getCaracterActual() == '\n'
		  || lex.esEol() )
		{
			lex.getTokenActual() = OBJ_OBJECT;
		}
		else {
			lex.getToken();

			if ( !NombreReferencia::compruebaRef(
							lex.getTokenActual() ) )
			{
				throw ESintxIdNoValido( lex.getTokenActual().c_str() );
			}
		}

		// Crear
		toret = new(std::nothrow) NMObj( nombre, lex.getTokenActual() );

		if ( toret == NULL ) {
			throw ENoHayMemoria( "creando mnemo Obj" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ====================================================================== NMMth
const std::string NMMth::formatoXML = "Method";
const std::string NMMth::mnemoStr   = "MTH";
const std::string NMMth::PUBLICO    = "+";
const std::string NMMth::PRIVADO    = "-";

// ------------------------------------------------------------- NMMth::NMMth()
NMMth::NMMth(const std::string &acces, const std::string &nom)
                : CtrMt(NMTH), acceso(acces), nombre(nom)
{
	verifica();
}

NMMth::NMMth(const NMMth &x)
	: CtrMt(NMTH)
/// Construye un mnemo a partir de otro
{
        acceso = x.acceso;
        nombre = x.nombre;

        copiarArgumentosDe( x );
}

// ---------------------------------------------------------- NMMth::verifica()
void NMMth::verifica() throw (ECompilacion)
{
	// Comprobar el nombre del met.
	if ( !NombreIdentificador::compruebaId( nombre ) ) {
		throw ESintxIdNoValido( std::string( "NMMth: nombre de met.: " + nombre ).c_str() );
	}

	// Comprobar la visibilidad [+/-]
	if ( acceso != PUBLICO
	  && acceso != PRIVADO )
	{
		throw ESintxIdNoValido( std::string( "NMMth: " + nombre
		                      + " acceso: " + acceso ).c_str()
		);
	}

	// Comprobar los argumentos
	for(register unsigned int i = 0; i < getArgs()->size(); ++i) {
		if ( dynamic_cast<NombreReferencia *>( (*getArgs())[i] ) != NULL )
		{
			throw ESintxIdNoValido( std::string( "NMMth: param. formal: "
						+ (*getArgs())[i]->getNombre() ).c_str()
			);
		}
	}
}

// ----------------------------------------------------------- NMMth::escribe()
void NMMth::escribe(MedioSoporte *m)
/// Escribe un mnemo a bytecode
{
	verifica();

        // el mnemotécnico
        Mnemotecnico::escribe( m );

        // Los parámetros del mnemotécnico
        if ( acceso == PUBLICO )
                m->escribeBajoNivelUINT8( '+' );
        else    m->escribeBajoNivelUINT8( '-' );

        m->escribeIdentificador( nombre );

        // Escribir los argumentos
        escribeVectorArgs( m );
}

// --------------------------------------------------------------- NMMth::lee()
void NMMth::lee(MedioSoporte *m, bool leeMnemo)
/// Lee un mnemo del bytecode
{
        UINT8 acceso_ll;

        // Limpiar
        eliminaArgumentos();

        // El código
	if ( leeMnemo ) {
        	Mnemotecnico::lee( m );
	}

        // El acceso de visibilidad
        acceso_ll = m->leeBajoNivelUINT8();
        if (acceso_ll == '+')
                acceso = PUBLICO;
        else    acceso = PRIVADO;

        // El nombre del método
        m->leeIdentificador( nombre );

	// Argumentos formales
        leeVectorArgs( m );

	verifica();
}

// ------------------------------------------------------------ NMMth::listar()
std::string NMMth::listar(bool bello)
/// Devuelve una descripción de este mnemo en formato ensamblador
{
      register unsigned int n;
      std::string toret;

      if (!bello) {
        toret = mnemoStr;

        toret += ' ';
        toret += acceso;
        toret += ' ';
        toret += nombre;

	for(n = 0;n < getArgs()->size(); ++n)
        {
              toret += ' ';
              toret += (*getArgs())[n]->getNombre();
        }
      }
      else {
        toret  = acceso==PUBLICO? ACC_PUBLICO : ACC_PRIVADO;
        toret += ' ';
        toret += nombre;
        toret += '(';

        for(n = 0; n<getArgs()->size(); ++n )
        {
              toret += (*getArgs())[n]->getNombre();

              if ( n < ( getArgs()->size() - 1 ) )
              {
                    toret += ',';
              }
        }

        toret += ") {";
      }


      return toret;
}

// ----------------------------------------------------- NMMth::getFormatoXML()
std::string NMMth::getFormatoXML()
/// Devuelve una descripción en formato XML de este mnemo
{
      std::string toret;

      toret += '<';
      toret += formatoXML;
      toret += " access=\"";
      toret += acceso==PUBLICO? ACC_PUBLICO : ACC_PRIVADO;
      toret += "\" name=\"";
      toret += nombre;
      toret += "\" arguments=\"";

      for(register unsigned int n = 0 ; n < getArgs()->size() ; ++n)
      {
            toret += (*getArgs())[n]->getNombre();

            if ( n < ( getArgs()->size() - 1 ) )
            {
                  toret += ',';
                  toret += ' ';
            }

      }

      toret += '"';
      toret += '>';

      return toret;
}

// ---------------------------------------------------------- NMMth::compilar()
NMMth * NMMth::compilar(AnalizadorLexico &lex)
/// Genera un nuevo Mnemo a partir de una cadena
{
	bool acceso;
	NMMth * toret = NULL;

	// Buscar el mnemotécnico
	AnalizadorLexico::maysCnvt( lex.getToken() );

	if ( lex.getTokenActual() == mnemoStr )
	{
		lex.pasaEsp();
		lex.getLiteral(' ');

		if ( lex.getTokenActual() == NMMth::PRIVADO )
			acceso = false;
		else if ( lex.getTokenActual() == NMMth::PUBLICO )
			acceso = true;
		else throw ESintxIdNoValido( std::string( "compilando " + mnemoStr + ", "
					     "se esperaba marca de acceso, no: "
		                             + lex.getTokenActual() ).c_str() );

		lex.getToken();
		if ( lex.getTokenActual().empty() )
			throw ESintxIdNoValido( std::string(
					       "compilando " + mnemoStr + ", "
			                       "se esperaba id: nombre de met."
		                               + lex.getTokenActual() ).c_str() );
		if ( !NombreIdentificador::compruebaId( lex.getTokenActual().c_str() ) )
			throw ESintxIdNoValido( lex.getTokenActual().c_str() );

		// Crear el mnemo
		toret = new(std::nothrow) NMMth(
                                acceso ? NMMth::PUBLICO : NMMth::PRIVADO,
				lex.getTokenActual() )
		;

		// Coger los argumentos
		if ( toret != NULL )
		{
			lex.pasaSoloEsp();
			while( lex.getCaracterActual() != '\n'
			    && !lex.esEol() )
			{
				lex.getToken();

				if (!NombreReferencia::compruebaRef(
				                      lex.getTokenActual() )) {
				   throw ESintxIdNoValido( std::string(
				   		    "compilando " + mnemoStr + ", "
				                    "se esperaba ref. "
				                    "local como argumento, "
						    "no: "
						    + lex.getTokenActual() ).c_str() );
				}

				toret->masArgumentos( lex.getTokenActual() );
				lex.pasaSoloEsp();
			}
		} else throw ENoHayMemoria( "Creando Mnemo Mth" );
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ====================================================================== NMDef
const std::string NMDef::formatoXML = "Reference";
const std::string NMDef::mnemoStr   = "DEF";

// ------------------------------------------------------------- NMDef::NMDef()
NMDef::NMDef(const std::string &nom) : Instr(NDEF), nombre(nom)
{
	verifica();
}

// ---------------------------------------------------------- NMDef::verifica()
void NMDef::verifica() throw (ECompilacion)
{
	if ( !NombreIdentificador::compruebaId( nombre ) ) {
		throw ESintxIdNoValido( std::string( "NMDef: nombre " + nombre ).c_str() );
	}
}

// ----------------------------------------------------------- NMDef::escribe()
void NMDef::escribe(MedioSoporte *m)
/// Escribe el mnemoténico al bytecode
{
	verifica();

        Mnemotecnico::escribe( m );
        m->escribeIdentificador( nombre );
}

// --------------------------------------------------------------- NMDef::lee()
void NMDef::lee(MedioSoporte *m, bool leeMnemo)
/// Lee el mnemotécnico del bytecode
{
	if ( leeMnemo ) {
        	Mnemotecnico::lee( m );
	}

        m->leeIdentificador( nombre );

	verifica();
}

// ------------------------------------------------------------ NMDef::listar()
std::string NMDef::listar(bool bello)
/// Devuelve una descripción del mnemotécnico en formato ensamblador
{
      std::string toret;

      if (!bello) {
        toret = mnemoStr;

        toret += ' ';
        toret += nombre;
      }
      else {
        toret = formatoXML;

        toret += '(';
        toret += nombre;
        toret += ')';
      }


      return toret;
}

// ----------------------------------------------------- NMDef::getFormatoXML()
std::string NMDef::getFormatoXML()
{
      std::string toret;

      toret += '<';
      toret += formatoXML;
      toret += " name=\"";
      toret += nombre;
      toret += "\"/>";

      return toret;
}

// ------------------------------------------------- NMDef::compilar(std::string &x)
NMDef * NMDef::compilar(AnalizadorLexico &lex)
/// Toma una cadena con info del mnemo en formato ensamblado y genera el mnemo
{
	NMDef * toret = NULL;

	// Tomar el mnemo
	AnalizadorLexico::maysCnvt( lex.getToken() );

	if ( lex.getTokenActual() == mnemoStr )
	{
		// Busca el nombre de la futura referencia local
		lex.getToken();

		if ( lex.getTokenActual().empty() ) {
			throw ESintxIdNoValido( std::string(
				"compilando " + mnemoStr + ", "
				"se esperaba id: nombre de ref. local, y no: "
			    + lex.getTokenActual() ).c_str()
			);
		}

		if ( !NombreIdentificador::compruebaId( lex.getTokenActual() ) ) {
			throw ESintxIdNoValido( lex.getTokenActual().c_str() );
		}

		toret = new(std::nothrow) NMDef( lex.getTokenActual() );

		if ( toret == NULL ) {
			throw ENoHayMemoria( "construyendo mnemo NMDef" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ---------------------------------------------------------------------- NMRet
const std::string NMRet::formatoXML = "return";
const std::string NMRet::mnemoStr   = "RET";

// ------------------------------------------------------------- NMRet::NMRet()
NMRet::NMRet() : Instr(NRET), nombre(NULL)
{
}

NMRet::NMRet(const std::string &nom) : Instr(NRET)
{
	nombre = Nombre::creaNombreAdecuado( nom );
}

// ------------------------------------------------------------- NMRet::NMRet()
NMRet::NMRet(const NMRet &x) : Instr (NRET)
{
    nombre = x.nombre->copia();
}

// ------------------------------------------------------------- NMRet::NMRet()
NMRet::NMRet(NombreRegistro::CodigoRegistro rc) : Instr(NRET) {
        nombre = new(std::nothrow) NombreRegistro(rc);

	if ( nombre == NULL ) {
		throw ENoHayMemoria( std::string( "creando argumento "
				+ NombreRegistro::cnvtCodigoRegistroANombre( rc ) ).c_str()
		);
	}
}

// ---------------------------------------------------------- NMRet::verifica()
void NMRet::verifica() throw (ECompilacion)
/// En realidad, es imposiblq que sea inválido
{
}

// ----------------------------------------------------------- NMRet::escribe()
void NMRet::escribe(MedioSoporte *m)
/// Escribe un mnemo NMRet al bytecode
{
	verifica();

        Mnemotecnico::escribe( m );

        nombre->escribe( m );
}

// -------------------------------------------------------------- NMRet::lee()
void NMRet::lee(MedioSoporte *m, bool leeMnemo) {
/// Lee un mnemo NMRet del bytecode
        delete nombre;

	if ( leeMnemo ) {
        	Mnemotecnico::lee( m );
	}

        nombre = Nombre::lee( m );

	verifica();
}

// ------------------------------------------------------------ NMRet::listar()
std::string NMRet::listar(bool bello)
/// Devuelve una descripción del mnemo en formato ensamblador
{
      std::string toret;

      if (!bello) {
        toret = mnemoStr;

        toret += ' ';
        toret += nombre->getNombre();
      }
      else {
        toret = formatoXML;

        toret += ' ';
        toret += nombre->getNombre();
        toret += ';';
      }


      return toret;
}

// ----------------------------------------------------- NMRet::getFormatoXML()
std::string NMRet::getFormatoXML()
/// Devuelve una descripción del mnemo en formato XML
{
      std::string toret;

      toret += '<';
      toret += formatoXML;
      toret += " reference=\"";
      toret += nombre->getNombre();
      toret += "\"/>";

      return toret;
}

// ---------------------------------------------------------- NMRet::compilar()
NMRet * NMRet::compilar(AnalizadorLexico &lex)
{
	NMRet * toret = NULL;

	AnalizadorLexico::maysCnvt( lex.getToken() );

	if ( lex.getTokenActual() == mnemoStr )
	{
		// ¿Hay argumento de retorno?
		lex.pasaSoloEsp();

		if ( lex.getCaracterActual() != '\n'
		  && !lex.esEol() )
		{
			lex.getToken();

			if ( !NombreIdentificador::compruebaId(
							lex.getTokenActual() ) )
			{
				throw ESintxIdNoValido( lex.getTokenActual().c_str() );
			}

			toret = new(std::nothrow) NMRet( lex.getTokenActual() );
		}
		else toret = new(std::nothrow) NMRet( NombreRegistro::__RR );

		if ( toret == NULL ) {
			throw ENoHayMemoria( "construyendo NMRet" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}


// ====================================================================== NMAsg
const std::string NMAsg::formatoXML = "assign";
const std::string NMAsg::mnemoStr   = "ASG";

// ------------------------------------------------------------- NMAsg::NMAsg()
NMAsg::NMAsg() : Instr(NASG), nombre(NULL), reg(NULL)
{
}

NMAsg::NMAsg(const std::string &nom, NombreRegistro::CodigoRegistro rc)
        : Instr(NASG)
{
        // Colocar el registro
        reg = new(std::nothrow) NombreRegistro( rc );

        // Colocar el registro / identificador
	nombre = Nombre::creaNombreAdecuado( nom );

	if ( reg == NULL ) {
		throw ENoHayMemoria( std::string(
				"creando reg: "
			      + NombreRegistro::cnvtCodigoRegistroANombre( rc ) ).c_str()
		);
	}
}

// ------------------------------------------------------------- NMAsg::NMAsg()
NMAsg::NMAsg(const NMAsg &x) : Instr(NASG)
{
        reg    = x.reg->copia();
        nombre = x.nombre->copia();
}

// ---------------------------------------------------------- NMAsg::verifica()
void NMAsg::verifica() throw (ECompilacion)
/**
	Es imposible que algo vaya mal, al ser Nombre se comprueban al
	ser creados
*/
{
	if ( dynamic_cast<NombreReferencia *>( nombre ) != NULL ) {
		throw ESintxIdNoValido( "se esperaba id, no referencia" );
	}
}

// ----------------------------------------------------------- NMAsg::escribe()
void NMAsg::escribe(MedioSoporte *m)
/// Escribe el mnemo al bytecode
{
	verifica();

        Mnemotecnico::escribe( m );

        nombre->escribe( m );
        reg->escribe( m );
}

// --------------------------------------------------------------- NMAsg::lee()
void NMAsg::lee(MedioSoporte *m, bool leeMnemo)
/// Lee un mnemo del bytecode
{
        delete reg;
        delete nombre;

	if ( leeMnemo ) {
        	Mnemotecnico::lee( m );
	}

        nombre = Nombre::lee( m );
        reg    = NombreRegistro::lee( m );

	verifica();
}

// ------------------------------------------------------------ NMAsg::listar()
std::string NMAsg::listar(bool bello)
/// Obtiene la info del mnemo en formato ensamblador
{
      std::string toret;

      if (!bello) {
        toret = mnemoStr;

        toret += ' ';
        toret += nombre->getNombre();
        toret += ' ';
        toret += reg->getNombre();
      }
      else {
        toret = nombre->getNombre();
        toret += ' ';
        toret += '=';
        toret += ' ';
        toret += reg->getNombre();
        toret += ';';
      }


      return toret;
}

// ----------------------------------------------------- NMAsg::getFormatoXML()
std::string NMAsg::getFormatoXML()
/// Obtiene la info del mnemo en formato XML
{
      std::string toret;

      toret += '<';
      toret += formatoXML;
      toret += " name=\"";
      toret += nombre->getNombre();
      toret += "\"/>";

      return toret;
}

// ---------------------------------------------------------- NMAsg::compilar()
NMAsg * NMAsg::compilar(AnalizadorLexico &lex)
/// Toma la info de un mnemo de una cadena y lo compila
{
	NMAsg * toret = NULL;
	NombreRegistro::CodigoRegistro rc;
	std::string ref;

	AnalizadorLexico::maysCnvt( lex.getToken() );

	if ( lex.getTokenActual() == mnemoStr )
	{
		lex.getToken();

		// Coger la referencia
		if ( lex.getTokenActual().empty() ) {
			throw ESintxIdNoValido( std::string(
				           "compilando " + mnemoStr + ", "
			                   "se esperaba id: nombre "
			                   "de ref. local o "
		                           " atributo, no: "
					 + lex.getTokenActual() ).c_str() )
			;
		}

		if ( !NombreIdentificador::compruebaId( lex.getTokenActual() ) ) {
			throw ESintxIdNoValido( lex.getTokenActual().c_str() );
		}

		ref = lex.getTokenActual();

		// Coger el registro
		lex.pasaSoloEsp();

		if ( lex.getCaracterActual() == '\n'
		  || lex.esEol() )
		{
			rc = NombreRegistro::__ACC;
		}
		else {
			lex.getToken();

			rc = NombreRegistro::cnvtNombreACodigoRegistro(
					lex.getTokenActual() )
			;

			if ( rc == NombreRegistro::__NOREG )
				throw ESintxIdNoValido( std::string( "se esperaba registro, no: "
				                     + lex.getTokenActual() ).c_str()
				);
		}

		toret = new(std::nothrow) NMAsg( ref, rc );

		if ( toret == NULL ) {
			throw ENoHayMemoria( "construyendo NMAsg" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ====================================================================== NMStr
const std::string NMStr::formatoXML = "New__ZeroLiteralString";
const std::string NMStr::mnemoStr   = "STR";

// ------------------------------------------------------------- NMStr::NMStr()
NMStr::NMStr(const std::string &l) : Lit(NSTR)
{
	lit = l;
	convertirSeqEscapeAStr( lit );
}

// ---------------------------------------------------------- NMStr::verifica()
void NMStr::verifica() throw (ECompilacion)
{
}

// ------------------------------------------------ NMStr::convertirSeqEscape()
std::string & NMStr::convertirSeqEscapeAStr(std::string & lit)
/**
	Convierte los códigos de escape de una
	cadena en sus correspondencias.
*/
{
	size_t pos = lit.find( CHR_ESC_STR );
	size_t numChars;

	while ( pos != std::string::npos
		&&  pos < lit.length() )
	{
		numChars = 1;

		// Convertir
		if ( lit[pos + 1] == CHR_ESCAPE_CR )
		{
			// Es '\n'
			lit[pos] = '\n';
		}
		else
		if ( lit[pos + 1] == CHR_ESCAPE_TAB )
		{
			// Es '\t'
			lit[pos] = '\t';
		}
		else
		if ( lit[pos + 1] == CHR_ESCAPE_COMILLAS )
		{
			// Es '\"'
			lit[pos] = '\"';
		}
		else
		if ( lit[pos + 1] == CHR_ESCAPE_APOSTROFE )
		{
			// Es '\"'
			lit[pos] = '\'';
		}
		else
		if ( lit[pos + 1] == CHR_ESCAPE_DEC )
		{
			// Es '\d000'
			unsigned int num;
			std::string strNum;

			numChars += CHR_ESCAPE_NUMDIGITOS_DEC;

			strNum = lit.substr( pos + 2, CHR_ESCAPE_NUMDIGITOS_DEC );
			num    = atoi( strNum.c_str() );
			lit[pos] = num;
		}

		// Son dos caracteres: borrar siempre el siguiente
		// Así, automáticamente, se gestiona correctamente '\\'
		lit.erase( pos + 1, numChars );

		// Sig
		pos = lit.find( CHR_ESC_STR, pos + 1 );
	}

	return lit;
}

// ------------------------------------------------ NMStr::convertirSeqEscape()
std::string & NMStr::convertirStrASeqEscape(std::string & lit)
/**
	Convierte una cadena con símbolos especiales, es decir, sin
	códigos de escape, a una cadena que los tiene.
*/
{
	unsigned int pos = 0;

	while ( pos < lit.length() )
	{
		// Convertir
		if ( lit[pos] == '\n' )
		{
			// Es '\n'
			lit[pos] = CHR_ESC_STR;
			lit.insert( pos + 1, 1, CHR_ESCAPE_CR );

			++pos;
		}
		else
		if ( lit[pos] == '\t' )
		{
			// Es '\t'
			lit[pos] = CHR_ESC_STR;
			lit.insert( pos + 1, 1, CHR_ESCAPE_TAB );

			++pos;
		}
		else
		if ( lit[pos] == '"' )
		{
			// Es '\"'
			lit[pos] = CHR_ESC_STR;
			lit.insert( pos + 1, 1, CHR_ESCAPE_COMILLAS );

			++pos;
		}
		else
		if ( lit[pos] == '\'' )
		{
			lit[pos] = CHR_ESC_STR;
			lit.insert( pos + 1, 1, CHR_ESCAPE_APOSTROFE );

			++pos;
		}
		else
		if ( !isprint( lit[pos] ) )
		{
			// Es '\d000'
			std::string codifCar;

			codifCar += CHR_ESC_STR;
			codifCar += CHR_ESCAPE_DEC;
            codifCar += AnalizadorLexico::toString( lit[ pos ], 3 );

			// Eliminar el carácter
			lit.erase( pos, 1 );

			// Insertar secuencia de escape
			lit.insert( pos, codifCar );

			pos += codifCar.length() - 1;
		}

		++pos;
	}

	return lit;
}

// ----------------------------------------------------------- NMStr::escribe()
void NMStr::escribe(MedioSoporte *m)
{
	verifica();

    Mnemotecnico::escribe( m );

	m->escribeStr( lit );
}

// --------------------------------------------------------------- NMStr::lee()
void NMStr::lee(MedioSoporte *m, bool leeMnemo)
/// Lee el mnemo del bytecode
{
	if ( leeMnemo ) {
        	Mnemotecnico::lee( m );
	}

	m->leeStr( lit );

	verifica();
}

// ------------------------------------------------------------ NMStr::listar()
std::string NMStr::listar(bool bello)
/// Obtiene la info del mnemo en formato ensamblador
{
      std::string toret;
      std::string cpLiteral = lit;

      if (!bello) {
        toret = mnemoStr;

        toret += " \"";
        toret += convertirStrASeqEscape( cpLiteral );
        toret += '"';
      }
      else {
        toret = formatoXML;

        toret[3] = ' ';
        toret += "(\"";
        toret += convertirStrASeqEscape( cpLiteral );
        toret += "\")";
      }


      return toret;
}

// ----------------------------------------------------- NMStr::getFormatoXML()
std::string NMStr::getFormatoXML()
/// Obtiene la info del mnemo en formato XML
{
      std::string toret;

      toret += '<';
      toret += formatoXML;
      toret += " literal=\"";
      toret += lit;
      toret += "\"/>";

      return toret;
}

// ---------------------------------------------------------- NMStr::compilar()
NMStr * NMStr::compilar(AnalizadorLexico &lex)
{
	NMStr * toret = NULL;

	AnalizadorLexico::maysCnvt( lex.getToken() );

	if ( lex.getTokenActual() == mnemoStr )
	{
		// Llegar hasta las comillas
		lex.pasaEsp();

		// Porque ... hay comillas, ¿no?
		if ( lex.getCaracterActual() != '"')
			throw ESintxComillasEsperadas(
                std::string(
                    "compilando " + mnemoStr +
                    ", \""
                ).c_str()
			);

		// Pasar las comillas y obtener literal
		lex.avanza();
		lex.getLiteral( '"' );

		if ( lex.esEol() ) {
			throw ESintxComillasEsperadas(
				std::string(
                    "compilando " + mnemoStr
                    + ", \"" ).c_str()
                );
		}

		// Pasa las comillas
		lex.avanza();

		toret = new(std::nothrow) NMStr( lex.getTokenActual() );

		if ( toret == NULL ) {
			throw ENoHayMemoria( "creando NMStr" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ====================================================================== NMMsg
const std::string NMMsg::formatoXML = "send";
const std::string NMMsg::mnemoStr   = "MSG";

// ------------------------------------------------------------- NMMsg::NMMsg()
NMMsg::NMMsg() : Instr(NMSG), ref(NULL)
{
}

NMMsg::NMMsg(const std::string &r, const std::string &m)
                : Instr(NMSG), met(m)
{
	ref = Nombre::creaNombreAdecuado( r );

	verifica();
}

NMMsg::NMMsg(const NMMsg &x) : Instr(NMSG)
{
    ref  = x.ref->copia();
        met  = x.met;

        copiarArgumentosDe( x );
}

// ---------------------------------------------------------- NMMsg::verifica()
void NMMsg::verifica() throw (ECompilacion)
{
	// Comprobar el nombre del met.
	if ( !NombreIdentificador::compruebaIdMetodo( met ) ) {
		throw ESintxIdNoValido( std::string( "NMMsg: nombre met. '" + met + '\'' ).c_str() );
	}

	// Comprobar los identificadores
	for(register unsigned int i = 0; i < getArgs()->size(); ++i) {
		if ( dynamic_cast<NombreReferencia *>( (*getArgs())[i] ) != NULL )
		{
			throw ESintxIdNoValido( std::string( "argumento Msg: '"
			                        + (*getArgs())[i]->getNombre()
                                                + '\'' ).c_str()
			);
		}
	}
}

// ----------------------------------------------------------- NMMsg::escribe()
void NMMsg::escribe(MedioSoporte *m)
/// Escribe un MSG al bytecode
{
	verifica();

        // el mnemotécnico
        Mnemotecnico::escribe( m );

        // Los parámetros del mnemotécnico
        ref->escribe( m );

        // El método al que se llama
        m->escribeIdentificador( met );

        // Escribir los argumentos
        escribeVectorArgs( m );
}

// --------------------------------------------------------------- NMMsg::lee()
void NMMsg::lee(MedioSoporte *m, bool leeMnemo)
/// Lee un MSG del bytecode
{
        delete ref;
        eliminaArgumentos();

        // el mnemotécnico
	if ( leeMnemo ) {
        	Mnemotecnico::lee( m );
	}

        // Leer el objeto al que se le envía el mensaje
        ref = Nombre::lee( m );

        // Leer el método al que se llama
        m->leeIdentificador( met );

        // Leer los argumentos
        leeVectorArgs( m );

	verifica();
}

// ------------------------------------------------------------ NMMsg::listar()
std::string NMMsg::listar(bool bello)
{
      std::string toret;

      if (!bello) {
        toret = mnemoStr;

        toret += ' ';
        toret += ref->getNombre();
        toret += ' ';
        toret += met;

        for (unsigned int i = 0;i < getArgs()->size(); ++i)
        {
              toret += ' ';
              toret += (*getArgs())[i]->getNombre();
        }
      }
      else {
        toret  = ref->getNombre();
        toret += '.';
        toret += met;
        toret += '(';

        for(register unsigned int i = 0;i < getArgs()->size(); ++i)
        {
              toret += (*getArgs())[i]->getNombre();
              if (i < ( getArgs()->size() - 1) )
              {
                    toret += ',';
                    toret += ' ';
              }
        }

        toret += ')';
        toret += ';';
      }


      return toret;
}

// ----------------------------------------------------- NMMsg::getFormatoXML()
std::string NMMsg::getFormatoXML()
/// Devuelve la info del mnemo en formato XML
{
      std::string toret;

      toret += '<';
      // Referencia
      toret += formatoXML;
      toret += " reference=\"";
      toret += ref->getNombre();

      // Método
      toret += "\" name=\"";
      toret += met;

      // Argumentos
      toret += "\" args=\"";
      for(register unsigned int n = 0; n < getArgs()->size(); ++n)
      {
            toret += (*getArgs())[n]->getNombre();

            if (n<(getArgs()->size()-1))
            {
                  toret += ',';
                  toret += ' ';
            }
      }

      toret += "\"/>";
      return toret;
}

// ---------------------------------------------------------- NMMsg::compilar()
NMMsg * NMMsg::compilar(AnalizadorLexico &lex)
{
	std::string ref;
	NMMsg * toret = NULL;

	AnalizadorLexico::maysCnvt( lex.getToken() );

	if ( lex.getTokenActual() == mnemoStr )
	{
		lex.getToken();

		// Referencia
		if ( lex.getTokenActual().empty() )
			throw ESintxIdNoValido( std::string(
				"compilando " + mnemoStr + ", "
				"se esperaba ref. local" ).c_str()
			);
		if ( !NombreReferencia::compruebaRef( lex.getTokenActual() ) )
			throw ESintxIdNoValido( std::string(
				"compilando " + mnemoStr + ", "
				"se esperaba ref. local, no: "
			        + lex.getTokenActual() ).c_str()
			);

		ref = lex.getTokenActual();

		// Método
		lex.getToken();
		if ( lex.getTokenActual().empty() )
			throw ESintxIdNoValido( std::string(
				        "compilando " + mnemoStr + ", "
                        "se esperaba nombre de met." ).c_str()
			);
		if ( !NombreIdentificador::compruebaIdMetodo( lex.getTokenActual() ) )
			throw ESintxIdNoValido( std::string(
						"compilando " + mnemoStr + ", "
						"se esperaba nombre de met."
			                        ", no: "
			                        + lex.getTokenActual() ).c_str()
			);


		toret = new(std::nothrow) NMMsg ( ref, lex.getTokenActual() );

		if ( toret != NULL )
		{
			lex.pasaSoloEsp();
			while( lex.getCaracterActual() != '\n'
			    && !lex.esEol() )
			{
				lex.getToken();

				if ( !NombreIdentificador::compruebaId(
						lex.getTokenActual() ) )
				{
					throw ESintxIdNoValido( std::string(
						      "se esperaba ref. local "
						      "como argumento, no: "
					             + lex.getTokenActual() ).c_str()
					);
				}

				toret->masArgumentos( lex.getTokenActual() );
				lex.pasaSoloEsp();
			}
		}
		else throw ENoHayMemoria( "Construyendo NMMsg" );
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ====================================================================== NMEno
const std::string NMEno::formatoXML = "EndObject";
const std::string NMEno::mnemoStr   = "ENO";

// ---------------------------------------------------------- NMEno::Verifica()
void NMEno::verifica() throw (ECompilacion)
{
}

// ----------------------------------------------------------- NMEno::escribe()
void NMEno::escribe(MedioSoporte *m)
{
	verifica();

	Mnemotecnico::escribe( m );
}

// --------------------------------------------------------------- NMEno::lee()
void NMEno::lee(MedioSoporte *m, bool leeMnemo)
{
	if ( leeMnemo ) {
		Mnemotecnico::lee( m );
	}

	verifica();
}

// ------------------------------------------------------------ NMEno::listar()
std::string NMEno::listar(bool bello)
/// Devuelve la info del mnemo en formato ensamblador
{
      std::string toret;

      if ( !bello )
                toret = mnemoStr;
      else      toret = '}';

      return toret;
}

// ----------------------------------------------------- NMEno::getFormatoXML()
std::string NMEno::getFormatoXML()
/// Devuelve la info del mnemo en formato XML
{
      std::string toret;

      toret += '<';
      toret += '/';
      toret += formatoXML;
      toret += '>';

      return toret;
}

// ---------------------------------------------------------- NMEno::compilar()
NMEno * NMEno::compilar(AnalizadorLexico &lex)
{
	AnalizadorLexico::maysCnvt( lex.getToken() );
	NMEno * toret = NULL;

	if ( lex.getTokenActual() == mnemoStr )
	{
		toret = new(std::nothrow) NMEno();

		if ( toret == NULL ) {
			throw ENoHayMemoria( "Construyendo Enm" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ====================================================================== NMEnm
const std::string NMEnm::formatoXML = "EndMethod";
const std::string NMEnm::mnemoStr   = "ENM";

// ---------------------------------------------------------- NMEnm::Verifica()
void NMEnm::verifica() throw (ECompilacion)
{
}

// ----------------------------------------------------------- NMEnm::escribe()
void NMEnm::escribe(MedioSoporte *m)
{
	verifica();

	Mnemotecnico::escribe( m );
}

// --------------------------------------------------------------- NMEnm::lee()
void NMEnm::lee(MedioSoporte *m, bool leeMnemo)
{
	if ( leeMnemo )
		Mnemotecnico::lee( m );
	else	mnemo = NENM;

	verifica();
}

// ------------------------------------------------------------ NMEnm::listar()
std::string NMEnm::listar(bool bello)
/// Devuelve la info del mnemo en formato ensamblador
{
      std::string toret;

      if (!bello)
                toret = mnemoStr;
      else      toret = '}';

      return toret;
}

// ----------------------------------------------------- NMEnm::getFormatoXML()
std::string NMEnm::getFormatoXML()
/// Devuelve la info del mnemo en formato XML
{
      std::string toret;

      toret += '<';
      toret += '/';
      toret += formatoXML;
      toret += '>';

      return toret;
}

// ---------------------------------------------------------- NMEnm::compilar()
NMEnm * NMEnm::compilar(AnalizadorLexico &lex)
{
	AnalizadorLexico::maysCnvt( lex.getToken() );
	NMEnm * toret = NULL;

	if ( lex.getTokenActual() == mnemoStr )
	{
		toret = new(std::nothrow) NMEnm();

		if ( toret == NULL ) {
			throw ENoHayMemoria( "Construyendo Enm" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ====================================================================== NMAtr
const std::string NMAtr::formatoXML = "Attribute";
const std::string NMAtr::mnemoStr   = "ATR";
const std::string NMAtr::PUBLICO    = "+";
const std::string NMAtr::PRIVADO    = "-";

// ------------------------------------------------------------- NMAtr::NMAtr()
NMAtr::NMAtr() : CtrOb(NATR), ref(NULL)
{
}

NMAtr::NMAtr(const std::string &acc,
             const std::string &nom,
	     const std::string &r,
	     const std::string &l)
	: CtrOb(NATR), acceso(acc), nombre(nom), lit(l)
{
	ref = Nombre::creaNombreAdecuado( r );

	verifica();
}

NMAtr::~NMAtr()
{
	delete ref;
	ref = NULL;
}

// ---------------------------------------------------------- NMAtr::verifica()
void NMAtr::verifica() throw (ECompilacion)
{
	// Comprobar visibilidad [+/-]
	if ( acceso != PUBLICO
	  && acceso != PRIVADO )
	{
		throw ESintxIdNoValido( std::string(
                                "NMAtr: " + nombre
		                      + " acceso " + acceso
                ).c_str()
		);
	}

	// Comprobar el nombre
	if ( !NombreIdentificador::compruebaId( nombre ) ) {
		throw ESintxIdNoValido( std::string( "NMAtr: nombre " + nombre ).c_str() );
	}

	// Comprobar el argumento
	if ( dynamic_cast<NombreRegistro *>( ref ) != NULL ) {
		throw ESintxIdNoValido( std::string( "Atr: se esperaba id/ref, no registro" ).c_str() );
	}
}

// ----------------------------------------------------------- NMAtr::escribe()
void NMAtr::escribe(MedioSoporte *m)
/// Escribe un atributo a bytecode
{
	verifica();

        Mnemotecnico::escribe( m );

        if ( acceso == PUBLICO )
                m->escribeBajoNivelUINT8( '+' );
        else    m->escribeBajoNivelUINT8( '-' );

        m->escribeIdentificador( nombre );
	ref->escribe( m );
}

// --------------------------------------------------------------- NMAtr::lee()
void NMAtr::lee(MedioSoporte *m, bool leeMnemo)
/// Lee un atributo del bytecode
{
        // El código
	if ( leeMnemo ) {
        	Mnemotecnico::lee( m );
	}

        // La visibilidad
        UINT8 acceso_ll = m->leeBajoNivelUINT8();

        if ( acceso_ll == '+' )
                acceso = PUBLICO;
        else    acceso = PRIVADO;

        // El nombre del atributo y la referencia
        m->leeIdentificador( nombre );
	ref = Nombre::lee( m );

	verifica();
}

// ------------------------------------------------------------ NMAtr::listar()
std::string NMAtr::listar(bool bello)
/// Devuelve una descripción del bytecode en formato ensamblador
{
      std::string toret;

      if ( !bello ) {
        toret = mnemoStr;

		toret += ' ';
        toret += ( ( acceso == PUBLICO ) ? PUBLICO : PRIVADO );
		toret += ' ';

        toret += ' ';
        toret += nombre;

        toret += ' ';
        toret += ref->getNombre();
      }
      else {
        toret  = ( ( acceso == PUBLICO ) ? ACC_PUBLICO : ACC_PRIVADO );
        toret += ' ';
        toret += nombre;
        toret += ' ';
        toret += '=';
        toret += ' ';
        toret += ref->getNombre();
        toret += ';';
      }


      return toret;
}

// ----------------------------------------------------- NMAtr::getFormatoXML()
std::string NMAtr::getFormatoXML()
/// Devuelve una descripción del mnemo en XML
{
      std::string toret;

      toret += '<';
      toret += formatoXML;
      toret += " access=\"";
      toret += ((acceso==PUBLICO)? ACC_PUBLICO : ACC_PRIVADO);
      toret += "\" name=\"";
      toret += nombre;
      toret += "\" reference=\"";
      toret += ref->getNombre();
      toret += "\"/>";

      return toret;
}

// ---------------------------------------------------------- NMAtr::compilar()
NMAtr *NMAtr::compilar(AnalizadorLexico &lex)
/**
	Convierte info en cadena formato ensamblador a mnemotécnico

	El argumento puede ser un:
  	<ol>
		<li>una referencia
		<li>un literal de cadena
		<li>un literal numérico.
	<ol>
	El primer caso es el "normal"
	En los dos siguientes casos, el primer char del
	argumento es un '"' o un '+', respectivamente.
	En estos dos casos, crear el literal
	correspondiente y asignarlo.
*/
{
	NMAtr* toret = NULL;
	std::string nombre;
	std::string ref;
	bool acceso;
	AnalizadorLexico::TipoToken tk = AnalizadorLexico::NADA;


	AnalizadorLexico::maysCnvt( lex.getToken() );
	if ( lex.getTokenActual() == mnemoStr )
	{
		// Accesibilidad
		lex.pasaEsp();
		lex.getLiteral(' ');
		if ( lex.getTokenActual() == NMAtr::PRIVADO )
			acceso = false;
		else if ( lex.getTokenActual() == NMAtr::PUBLICO )
			acceso = true;
		else throw ESintxIdNoValido( std::string(
			    "compilando " + mnemoStr + ", "
			    "se esperaba marca de acceso, y no "
	                    + lex.getTokenActual() ).c_str()
		     );

		// Nombre del atributo
		lex.getToken();
		if ( !NombreIdentificador::compruebaId( lex.getTokenActual() ) )
			throw ESintxIdNoValido( std::string(
			        "compilando " + mnemoStr + ", "
				"se esperaba nombre de atributo, no: "
	                       + lex.getTokenActual() ).c_str()
			);

		nombre = lex.getTokenActual();

		// Identificador del objeto/LitNumérico/LitCadena
		tk = lex.getTipoSiguienteToken();

		if  (tk == AnalizadorLexico::IDENTIFICADOR )
		{
			lex.getToken();

			if ( !NombreReferencia::compruebaRef(
		                            lex.getTokenActual() ) )
			{
				throw ESintxIdNoValido( std::string(
					"compilando " + mnemoStr + ", "
					"se esperaba referencia"
			                ", no: "
					+ lex.getTokenActual() ).c_str()
				);
			}

			ref = lex.getTokenActual();
			lex.getToken(); // Debería dejar el token vacío
		}
		else
		if ( tk == AnalizadorLexico::LITNUMERICO )
		{
			lex.getNumero();

			if ( !AnalizadorLexico::compruebaFlotante(
					             lex.getTokenActual() ) )
			{
				throw ESintxFltNoValido( std::string(
					"compilando " + mnemoStr + ", "
					"se esperaba un "
				        "num., no: "
				      + lex.getTokenActual() ).c_str()
				);
			}

			// Marcar que es un número
			ref = OBJ_NOTHING;
			lex.getTokenActual().insert( 0, "+" );
		}
		else
		if ( tk == AnalizadorLexico::LITCADENA )
		{
			lex.pasaEsp();

			if ( lex.getCaracterActual() != '"' ) {
				throw ESintxComillasEsperadas("\"");
			}

			// Pasar las comillas y obtener literal
			lex.avanza();
			lex.getLiteral('"');

			// Marcar que es una cadena
			ref = OBJ_NOTHING;
			lex.getTokenActual().insert( 0, "\"" );
		}
		else throw ESintxIdNoValido( std::string(
				"compilando " + mnemoStr + ", "
				"se esperaba referencia, "
				"literal de cadena o num." ).c_str()
			);

		toret = new(std::nothrow) NMAtr(
                                acceso ? NMAtr::PUBLICO : NMAtr::PRIVADO,
				nombre,
				ref,
				lex.getTokenActual() )
		;

		if ( toret == NULL) {
			throw ENoHayMemoria( "Construyendo mnemo NMAtr" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ====================================================================== NMIof
const std::string NMIof::formatoXML = "isInstanceOf";
const std::string NMIof::mnemoStr   = "IOF";

// ------------------------------------------------------------- NMIof::NMIof()
NMIof::NMIof() : Instr(NIOF), ref(NULL), reg(NULL)
{
}

NMIof::NMIof(const std::string &r, NombreRegistro::CodigoRegistro rc)
        : Instr( NIOF )
{
        reg = new(std::nothrow) NombreRegistro( rc );
	ref = Nombre::creaNombreAdecuado( r );

	if ( reg == NULL ) {
		throw ENoHayMemoria( "construyendo NMIof" );
	}
}

NMIof::NMIof(const NMIof &x) : Instr(NIOF)
{
        ref = x.ref->copia();
        reg = x.reg->copia();
}

NMIof::~NMIof()
{
	delete reg;
	delete ref;
	reg = NULL;
	ref = NULL;
}

// ---------------------------------------------------------- NMIof::Verifica()
void NMIof::verifica() throw (ECompilacion)
{
}

// ----------------------------------------------------------- NMIof::escribe()
void NMIof::escribe(MedioSoporte *m)
/// Escribe el mnemo al bytecode
{
	verifica();

        Mnemotecnico::escribe( m );
        ref->escribe( m );
        reg->escribe( m );
}

// -------------------------------------------------------------- NMIof::lee()
void NMIof::lee(MedioSoporte *m, bool leeMnemo)
/// Lee el mnemo del bytecode
{
        delete reg;
	delete ref;

	if ( leeMnemo ) {
        	Mnemotecnico::lee( m );
	}

        ref = Nombre::lee( m );
        reg = NombreRegistro::lee( m );

	verifica();
}

// ------------------------------------------------------------ NMIof::listar()
std::string NMIof::listar(bool bello)
/// Devuelve la info del mnemo en formato ensamblador
{
      std::string toret;

      if (!bello) {
        toret = mnemoStr;

        toret += ' ';
        toret += ref->getNombre();
        toret += ' ';
        toret += reg->getNombre();
      }
      else {
        toret = formatoXML;

        toret += '(';
        toret += ref->getNombre();
        toret += ',';
        toret += ' ';
        toret += reg->getNombre();
        toret += ')';
      }


      return toret;
}

// ----------------------------------------------------- NMIof::getFormatoXML()
std::string NMIof::getFormatoXML()
/// Devuelve la info del mnemo en formato XML
{
      std::string toret;

      toret += '<';
      toret += formatoXML;
      toret += " reference=\"";
      toret += ref->getNombre();
      toret += '\"';
      toret += " register=\"";
      toret += reg->getNombre();
      toret += "\"/>";

      return toret;
}


// ---------------------------------------------------------- NMIof::compilar()
NMIof * NMIof::compilar(AnalizadorLexico &lex)
/// Toma la info del mnemo en formato ensamblador y la compila
{
	NMIof * toret = NULL;
	std::string ref;

	// Convertir a mayúsculas el mnemotécnico
	AnalizadorLexico::maysCnvt( lex.getToken() );

	if ( lex.getTokenActual() == mnemoStr )
	{
		NombreRegistro::CodigoRegistro rc;

		lex.getToken();
		if ( lex.getTokenActual().empty() ) {
			throw ESintxIdNoValido( std::string(
			     "compilando " + mnemoStr + ", "
			     "se esperaba referencia de posible objeto padre" ).c_str()
            );
		}

		if ( !NombreReferencia::compruebaRef( lex.getTokenActual() ) ) {
			throw ESintxIdNoValido( std::string(
				"compilando " + mnemoStr + ", "
				"se esperaba ref. a objeto"
			        " padre, no: "
			      + lex.getTokenActual() ).c_str()
			);
		}

		ref = lex.getTokenActual();

		lex.pasaSoloEsp();
		if ( lex.getCaracterActual() == '\n'
		  || lex.esEol() )
		{
			rc = NombreRegistro::__ACC;
		}
		else {
			lex.getToken();

			rc = NombreRegistro::cnvtNombreACodigoRegistro(
						lex.getTokenActual() )
			;

			if ( rc == NombreRegistro::__NOREG ) {
				throw ESintxIdNoValido( std::string(
				   	"compilando " + mnemoStr + ", "
					"se esperaba registro, no:"
				       + lex.getTokenActual() ).c_str()
				);
			}
		}

		toret = new(std::nothrow) NMIof( ref, rc );

		if ( toret == NULL ) {
			throw ENoHayMemoria( "construyendo NMIof" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ====================================================================== NMFlt
const std::string NMFlt::formatoXML = "New__ZeroLiteralFloat";
const std::string NMFlt::mnemoStr   = "FLT";

// ---------------------------------------------------------- NMFlt::verifica()
void NMFlt::verifica() throw (ECompilacion)
{
}

// ----------------------------------------------------------- NMFlt::escribe()
void NMFlt::escribe(MedioSoporte *m)
/// Escribe el mnemo al bytecode
{
	verifica();

        Mnemotecnico::escribe( m );
        m->escribeNumReal( value );
}

// --------------------------------------------------------------- NMFlt::lee()
void NMFlt::lee(MedioSoporte *m, bool leeMnemo)
/// Lee el mnemo del bytecode
{
	if ( leeMnemo ) {
        	Mnemotecnico::lee( m );
	}

        value = m->leeNumReal();

	verifica();
}

std::string NMFlt::listar(bool bello)
/// Devuelve la info del mnemo en formato ensamblador
{
      std::string toret;

      if (!bello) {
        toret = mnemoStr;

        toret += ' ';
        toret += AnalizadorLexico::toString( value );
      }
      else {
        toret = formatoXML;

        toret[3] = ' ';
        toret += "( ";
        toret += AnalizadorLexico::toString( value );
        toret += " )";
      }


      return toret;
}

// ---------------------------------------------------- NMFlt::getLitAsString()
const std::string &NMFlt::getLitAsString()
{
	if ( lit.empty() ) {
	    lit = AnalizadorLexico::toString( value );
	}

	return lit;
}

// ----------------------------------------------------- NMFlt::getFormatoXML()
std::string NMFlt::getFormatoXML()
// Devuelve la info del mnemo en formato XML
{
      std::string toret;

      toret += '<';
      toret += formatoXML;
      toret += " literal=\"";
      toret += AnalizadorLexico::toString( value );
      toret += "\"/>";

      return toret;
}

// ---------------------------------------------------------- NMFlt::compilar()
NMFlt * NMFlt::compilar(AnalizadorLexico &lex)
{
	NMFlt * toret = NULL;

	AnalizadorLexico::maysCnvt( lex.getToken() );

	if ( lex.getTokenActual() == mnemoStr )
	{
		// obtener literal y comprobar
		lex.pasaEsp();
		lex.getLiteral( '\n' );

		if ( !AnalizadorLexico::compruebaFlotante(
		                                lex.getTokenActual() ) )
		{
			throw ESintxFltNoValido( std::string(
				"compilando " + mnemoStr + ", "
				"se esperaba num. flotante"
			        "y no: "
				+  lex.getTokenActual() ).c_str()
			);
		}

		// Grabar el mnemotécnico
		toret = new(std::nothrow) NMFlt( lex.getTokenActual() );

		if ( toret == NULL ) {
			throw ENoHayMemoria( "creando NMFlt" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ====================================================================== NMInt
const std::string NMInt::formatoXML = "New__ZeroLiteralInteger";
const std::string NMInt::mnemoStr   = "INT";

// ---------------------------------------------------------- NMInt::verifica()
void NMInt::verifica() throw (ECompilacion)
{
}

// ----------------------------------------------------------- NMInt::escribe()
void NMInt::escribe(MedioSoporte *m)
/// Escribe el mnemo al bytecode
{
	verifica();

        Mnemotecnico::escribe( m );
        m->escribeNumEntero( value );
}

// --------------------------------------------------------------- NMInt::lee()
void NMInt::lee(MedioSoporte *m, bool leeMnemo)
/// Lee el mnemo del bytecode
{
	if ( leeMnemo ) {
        	Mnemotecnico::lee( m );
	}

        value = m->leeNumEntero();

	verifica();
}

// ------------------------------------------------------------ NMInt::listar()
std::string NMInt::listar(bool bello)
/// Devuelve la info del mnemo en formato ensamblador
{
      std::string toret;

      if (!bello) {
        toret = mnemoStr;

        toret += ' ';
        toret += AnalizadorLexico::toString( value );
      }
      else {
        toret = formatoXML;

        toret[3] = ' ';
        toret += "( ";
        toret += AnalizadorLexico::toString( value );
        toret += " )";
      }


      return toret;
}

// ---------------------------------------------------- NMInt::getLitAsString()
const std::string &NMInt::getLitAsString()
{
	if ( lit.empty() ) {
	    lit = AnalizadorLexico::toString( value );
	}

	return lit;
}

// ----------------------------------------------------- NMInt::getFormatoXML()
std::string NMInt::getFormatoXML()
/// Devuelve la info del mnemo en formato XML
{
      std::string toret;

      toret += '<';
      toret += formatoXML;
      toret += " literal=\"";
      toret += AnalizadorLexico::toString( value );
      toret += "\"/>";

      return toret;
}

// ---------------------------------------------------------- NMInt::compilar()
NMInt * NMInt::compilar(AnalizadorLexico &lex)
{
	NMInt * toret = NULL;

	AnalizadorLexico::maysCnvt( lex.getToken() );

	if ( lex.getTokenActual() == mnemoStr )
	{
		// obtener literal y comprobar
		lex.pasaEsp();
		lex.getLiteral( '\n' );

		if ( !AnalizadorLexico::compruebaFlotante(
		                                lex.getTokenActual() ) )
		{
			throw ESintxFltNoValido( std::string(
					"compilando " + mnemoStr + ", "
					"se esperaba num. entero, "
			                "no: "
				       + lex.getTokenActual() ).c_str()
			);
		}

		// Grabar el mnemotécnico
		toret = new(std::nothrow) NMInt( lex.getTokenActual() );

		if ( toret == NULL ) {
			throw ENoHayMemoria( "creando NMInt" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ====================================================================== NMSet
const std::string NMSet::formatoXML = "set";
const std::string NMSet::mnemoStr   = "SET";

// ------------------------------------------------------------- NMSet::NMSet()
NMSet::NMSet() : Instr(NSET), reg(NULL), ref(NULL)
{
}

NMSet::NMSet(NombreRegistro::CodigoRegistro rc, const std::string &sref) :
                Instr(NSET)
{
        // Colocar el registro
        reg = new(std::nothrow) NombreRegistro(rc);
	ref = Nombre::creaNombreAdecuado( sref );

	if ( reg == NULL ) {
		throw ENoHayMemoria( "construyendo NMSet" );
	}
}

// ------------------------------------------------------------- NMSet::NMSet()
NMSet::NMSet(const NMSet &x) : Instr(NSET)
{
        reg = x.reg->copia();
        ref = x.ref->copia();
}

// ---------------------------------------------------------- NMSet::Verifica()
void NMSet::verifica() throw (ECompilacion)
{
}

// ----------------------------------------------------------- NMSet::escribe()
void NMSet::escribe(MedioSoporte *m)
/// Escribe el mnemo en bytecode
{
	verifica();

	Mnemotecnico::escribe( m );

	reg->escribe( m );
	ref->escribe( m );
}

// --------------------------------------------------------------- NMSet::lee()
void NMSet::lee(MedioSoporte *m, bool leeMnemo)
/// Lee un mnemo NMSet del bytecode
{
        delete ref;
        delete reg;

	ref = NULL;
	reg = NULL;

	if ( leeMnemo ) {
        	Mnemotecnico::lee( m );
	}

        reg = NombreRegistro::lee( m );
        ref = Nombre::lee( m );

	verifica();
}

// ------------------------------------------------------------ NMSet::listar()
std::string NMSet::listar(bool bello)
/// Devuelve la info del mnemo en formato XML
{
      std::string toret;

      if ( !bello ) {
        toret = mnemoStr;

        toret += ' ';
        toret += reg->getNombre();
        toret += ' ';
        toret += ref->getNombre();
      }
      else {
        toret = reg->getNombre();

        toret += ' ';
        toret += '=';
        toret += ' ';
        toret += ref->getNombre();
        toret += ';';
      }


      return toret;
}

// ----------------------------------------------------- NMSet::getFormatoXML()
std::string NMSet::getFormatoXML()
/// Devuelve la info del mnemo en formato XML
{
      std::string toret;

      toret += '<';
      toret += formatoXML;
      toret += " reference=\"";
      toret += ref->getNombre();
      toret += "\"/>";

      return toret;
}

// ---------------------------------------------------------- NMSet::compilar()
NMSet * NMSet::compilar(AnalizadorLexico &lex)
/// Toma la info en formato ensamblador de un mnemo y lo compila
{
	std::string registro;
	NMSet * toret = NULL;

	AnalizadorLexico::maysCnvt( lex.getToken() );
	if ( lex.getTokenActual() == mnemoStr )
	{
		// Comprobar el registro
		lex.getToken();

		if ( lex.getTokenActual().empty() ) {
			throw ESintxIdNoValido( std::string(
				"compilando " + mnemoStr + ", "
				"se esperaba id: registro, "
				"nombre de "
				"variable o referencia" ).c_str()
			);
		}
		if ( !NombreReferencia::compruebaRef( lex.getTokenActual() ) ) {
			throw ESintxIdNoValido( lex.getTokenActual().c_str() );
		}

		registro = lex.getTokenActual();

		// ¿Hay dos o uno?
		lex.pasaSoloEsp();

		if ( lex.getCaracterActual() == '\n'
		  || lex.esEol() )
		{
			toret = new(std::nothrow) NMSet( NombreRegistro::__ACC, registro );
		}
		else {
			lex.getToken();

			NombreRegistro::CodigoRegistro rc =
			    NombreRegistro::cnvtNombreACodigoRegistro( registro );

			// Comprobar
			if ( rc == NombreRegistro::__NOREG ) {
				throw ESintxIdNoValido( std::string(
					"compilando " + mnemoStr + ", "
					"se esperaba registro"
				        ", no: "
				       + lex.getTokenActual() ).c_str()
				);
			}

			if ( !NombreReferencia::compruebaRef(
						       lex.getTokenActual() ) )
			{
				throw ESintxIdNoValido( std::string(
				        "compilando " + mnemoStr + ", "
					"se esperaba registro"
				        " , no: "
				       + lex.getTokenActual() ).c_str()
				);
			}

			toret = new(std::nothrow) NMSet( rc, lex.getTokenActual() );
		}

		if ( toret == NULL ) {
			throw ENoHayMemoria( "creando NMSet" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ====================================================================== NMTrw
const std::string NMTrw::mensajeNulo = "";
const std::string NMTrw::formatoXML  = "throw";
const std::string NMTrw::mnemoStr    = "TRW";

// ------------------------------------------------------------- NMTrw::NMTrw()
NMTrw::NMTrw() : Instr(NTRW), ref(NULL), refMensaje(NULL)
{
}

NMTrw::NMTrw(const std::string &r, const std::string &m)
        : Instr(NTRW), refMensaje(NULL)
{
	ref = Nombre::creaNombreAdecuado( r );

        if ( !m.empty() ) {
            refMensaje = Nombre::creaNombreAdecuado( m);
        }

	if ( ref == NULL ) {
		throw ENoHayMemoria( "construyendo NMTrw, ref" );
	}
}

NMTrw::NMTrw(const NMTrw &x) : Instr(NTRW), refMensaje(NULL)
{
        ref = x.ref->copia();

	if ( x.refMensaje != NULL ) {
            refMensaje = x.refMensaje->copia();
	}
}

// ---------------------------------------------------------- NMTrw::Verifica()
void NMTrw::verifica() throw (ECompilacion)
{
}

// ----------------------------------------------------------- NMTrw::escribe()
void NMTrw::escribe(MedioSoporte *m)
/// Escribe la info del mnemo al bytecode
{
	verifica();

        Mnemotecnico::escribe( m );
        ref->escribe( m );

        // El identificador del objeto del mensaje se escribe sólo si existe
        if (refMensaje != NULL) {
               refMensaje->escribe( m );
        }
        else m->escribeReferencia( "" );
}

// --------------------------------------------------------------- NMTrw::lee()
void NMTrw::lee(MedioSoporte *m, bool leeMnemo)
/// Lee la info del mnemo del bytecode
{
        delete ref;
        ref = refMensaje = NULL;

	if ( leeMnemo ) {
        	Mnemotecnico::lee( m );
	}

        ref = Nombre::lee( m );

        // Leer el mensaje según exista o no
	try {
        	refMensaje = Nombre::lee( m );
	} catch (const ESintxIdNoValido &e) {
		delete refMensaje;
		refMensaje = NULL;
	}

	verifica();
}

// ------------------------------------------------------------ NMTrw::listar()
std::string NMTrw::listar(bool bello)
/// Devuelve la info del mnemo en formato ensamblador
{
      std::string toret;

      if (!bello) {
        toret = mnemoStr;

        toret += ' ';
        toret += ref->getNombre();

        toret += ' ';
        toret += getMensaje();
      }
      else {
        toret = formatoXML;

        toret += ' ';
        toret += ref->getNombre();
        toret += '(';
        toret += getMensaje();
        toret += ')';
      }


      return toret;
}

// ----------------------------------------------------- NMTrw::getFormatoXML()
std::string NMTrw::getFormatoXML()
{
      std::string toret;

      toret += '<';
      toret += formatoXML;
      toret += " reference=\"";
      toret += ref->getNombre();
      toret += "\" messageReference=\"";
      toret += refMensaje->getNombre();
      toret += "\"/>";

      return toret;
}

// ---------------------------------------------------------- NMTrw::compilar()
NMTrw * NMTrw::compilar(AnalizadorLexico &lex)
/// Toma la info del mnemo de una cadena, en formato ensamblador, y la compila
{
	std::string ref;
	NMTrw * toret = NULL;

	AnalizadorLexico::maysCnvt( lex.getToken() );
	if ( lex.getTokenActual() == mnemoStr )
	{
		lex.getToken();

		if ( lex.getTokenActual().empty() ) {
			throw ESintxIdNoValido( std::string(
                    "compilando " + mnemoStr + ", "
                    "se esperaba referencia de objeto"
                    " a lanzar como 'exception'" ).c_str()
			);
		}

		if ( !NombreIdentificador::compruebaId( lex.getTokenActual() ) ) {
			throw ESintxIdNoValido( std::string(
				        "compilando " + mnemoStr + ", "
                        "se esperaba referencia de objeto"
                        " a lanzar como 'exception', no: "
                        + lex.getTokenActual() ).c_str()
            );
		}

		ref = lex.getTokenActual();
		lex.pasaSoloEsp();

		// Si hay mensaje, es de un tipo, sino, del otro
		// El mensaje es en realidad una referencia a un objeto String
		if ( lex.getCaracterActual() != '\n'
		  && !lex.esEol()  )
		{
			lex.getToken();

			if ( !NombreIdentificador::compruebaId(
							lex.getTokenActual() ))
			{
				throw ESintxIdNoValido( lex.getTokenActual().c_str() );
			}

			toret = new(std::nothrow) NMTrw( ref, lex.getTokenActual() );
		} else toret = new(std::nothrow) NMTrw( ref );

		if ( toret == NULL ) {
			throw ENoHayMemoria( "compilando NMTrw" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ====================================================================== NMNop
const std::string NMNop::formatoXML = "nop";
const std::string NMNop::mnemoStr   = "NOP";

// ------------------------------------------------------------- NMNop::NMNop()
NMNop::NMNop() : Instr(NNOP)
{
}

// ----------------------------------------------------------- NMNop::escribe()
void NMNop::escribe(MedioSoporte *m)
{
	verifica();

	Mnemotecnico::escribe( m );
}

// --------------------------------------------------------------- NMNop::lee()
void NMNop::lee(MedioSoporte *m, bool leeMnemo)
{
	if ( leeMnemo ) {
		Mnemotecnico::lee( m );
	}

	verifica();
}

// ---------------------------------------------------------- NMNop::Verifica()
void NMNop::verifica() throw (ECompilacion)
{
}

// ------------------------------------------------------------ NMNop::listar()
std::string NMNop::listar(bool bello)
/// Devuelve la info del mnemo en formato ensamblador
{
      std::string toret;

      if (!bello)
                toret = mnemoStr;
      else      toret = formatoXML + '(' + ')';

      return toret;
}

// ----------------------------------------------------- NMNop::getFormatoXML()
std::string NMNop::getFormatoXML()
/// Devuelve la info del mnemo en formato XML
{
      std::string toret;

      toret += '<';
      toret += formatoXML;
      toret += '/';
      toret += '>';

      return toret;
}

// ---------------------------------------------------------- NMNop::compilar()
NMNop * NMNop::compilar(AnalizadorLexico &lex)
{
	AnalizadorLexico::maysCnvt( lex.getToken() );
	NMNop * toret = NULL;

	if ( lex.getTokenActual() == mnemoStr )
	{
		toret = new(std::nothrow) NMNop();

		if ( toret == NULL ) {
			throw ENoHayMemoria( "Construyendo Nop" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ---------------------------------------------------------------------- NMEtq
const std::string NMEtq::formatoXML = "etq";
const std::string NMEtq::mnemoStr   = "ETQ";

// ------------------------------------------------------------- NMEtq::NMEtq()
NMEtq::NMEtq(const std::string &nom) : Instr(NETQ), nombreEtq(nom)
{
	verifica();
}

// ---------------------------------------------------------- NMEtq::Verifica()
void NMEtq::verifica() throw (ECompilacion)
{
	if ( !NombreIdentificador::compruebaId( nombreEtq ) ) {
		throw ESintxIdNoValido( std::string( "ETQ: nombre " + nombreEtq ).c_str() );
	}
}

// ----------------------------------------------------------- NMEtq::escribe()
void NMEtq::escribe(MedioSoporte *m)
/// Escribe un mnemo NMEtq al bytecode
{
	verifica();

        Mnemotecnico::escribe( m );

        m->escribeStr( nombreEtq );
}

// --------------------------------------------------------------- NMEtq::lee()
void NMEtq::lee(MedioSoporte *m, bool leeMnemo)
/// Lee un mnemo NMEtq del bytecode
{
	if ( leeMnemo ) {
        	Mnemotecnico::lee( m );
	}

        m->leeStr( nombreEtq );

	verifica();
}

// ------------------------------------------------------------ NMEtq::listar()
std::string NMEtq::listar(bool bello)
/// Devuelve una descripción del mnemo en formato ensamblador
{
	if ( bello )
      		return std::string( ':' + nombreEtq );
	else	return std::string( mnemoStr + ' ' + nombreEtq );
}

// ----------------------------------------------------- NMRet::getFormatoXML()
std::string NMEtq::getFormatoXML()
/// Devuelve una descripción del mnemo en formato XML
{
      std::string toret;

      toret += '<';
      toret += formatoXML;
      toret += " name=\"";
      toret += nombreEtq;
      toret += "\"/>";

      return toret;
}

// ---------------------------------------------------------- NMEtq::compilar()
NMEtq * NMEtq::compilar(AnalizadorLexico &lex)
{
	NMEtq * toret = NULL;

	AnalizadorLexico::maysCnvt( lex.getToken() );

	if ( lex.getTokenActual() == mnemoStr )
	{
		lex.getToken();

		if ( lex.getTokenActual().empty() ) {
			throw ESintxIdNoValido( std::string(
                    "compilando " + mnemoStr + ", "
                    "se esperaba nombre de etiqueta" ).c_str()
			);
		}
		else {
			if ( !NombreIdentificador::compruebaId( lex.getTokenActual() ) )
				throw ESintxIdNoValido( std::string(
				   	"compilando " + mnemoStr + ", "
					"se esperaba nombre de etiqueta"
					+ lex.getTokenActual() ).c_str()
				);

			toret = new(std::nothrow) NMEtq( lex.getTokenActual() );

			if ( toret == NULL ) {
				throw ENoHayMemoria( "construyendo NMEtq" );
			}
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ======================================================================== Flw
// --------------------------------------------------------- Flw::putEtiqueta()
void Flw::putEtiqueta(std::string const & e)
{
	numOpcodes = 0;
	etq = e;

	verifica();
}

void Flw::putNumero(INT16 nop)
{
	etq.clear();
	numOpcodes = nop;
}

// ------------------------------------------------------------ Flw::verifica()
void Flw::verifica() throw (ECompilacion)
{
	if ( getNumero() == 0
	  && etq.empty() )
	{
		throw EChkSaltoInvalido( "salto de 0 opcodes imposible" );
	}

	if ( getNumero() == 0
	  && !NombreIdentificador::compruebaId( etq ) )
    {
		throw ESintxIdNoValido( std::string( "Flw: nombre etq: " + etq ).c_str() );
	}
}

// ------------------------------------------------------------- Flw::escribe()
void Flw::escribe(MedioSoporte *m)
/// Escribe el mnemo al bytecode. Cuidado, no escribe etiquetas.
{
	verifica();

        // el mnemotécnico
        Mnemotecnico::escribe( m );

        // El número de opcodes a desplazarse
        m->escribeBajoNivelINT16( getNumero() );
}

// ----------------------------------------------------------------- Flw::lee()
void Flw::lee(MedioSoporte *m, bool leeMnemo)
/// Lee el mnemo del bytecode. Cuidado, no admite etiquetas.
{
        // el mnemotécnico
	if ( leeMnemo ) {
        	Mnemotecnico::lee( m );
	}

        // El número de opcodes a desplazarse
        putNumero( m->leeBajoNivelINT16() );

	verifica();
}

// ====================================================================== NMJmp
const std::string NMJmp::formatoXML = "jumpTo";
const std::string NMJmp::mnemoStr   = "JMP";

// ------------------------------------------------------------- NMJmp::NMJmp()
NMJmp::NMJmp(const std::string &m) : Flw(NJMP) {
        putNumero( atoi( m.c_str() ) );

        if ( getNumero() == 0 ) {
		putEtiqueta( m );
        }
}

// ------------------------------------------------------------ NMJmp::listar()
std::string NMJmp::listar(bool bello)
/// Devuelve la info del mnemo en formato ensamblador
{
      std::string toret;

      if (!bello) {
        toret = mnemoStr;

        toret += ' ';
        toret += AnalizadorLexico::toString( getNumero() );
      }
      else {
        toret  = formatoXML;

        toret += ' ';
        toret += AnalizadorLexico::toString( getNumero() );
        toret += ';';
      }

      return toret;
}

// ----------------------------------------------------- NMJmp::getFormatoXML()
std::string NMJmp::getFormatoXML()
/// Devuelve la info del mnemo en formato XML
{
       std::string toret;

      toret += '<';

      // Referencia
      toret += formatoXML;
      toret += " value=\"";
      toret += AnalizadorLexico::toString( getNumero() );
      toret += "\"/>";

      return toret;
}

// ---------------------------------------------------------- NMJmp::compilar()
NMJmp * NMJmp::compilar(AnalizadorLexico &lex)
{
	NMJmp * toret = NULL;

	AnalizadorLexico::maysCnvt( lex.getToken() );

	if ( lex.getTokenActual() == mnemoStr )
	{
		// Coger el número de opcodes
		lex.pasaEsp();
		lex.getLiteral( '\n' );

		if ( lex.getTokenActual().empty() ) {
			throw ESintxIdNoValido( std::string(
				"compilando " + mnemoStr + ", "
				"se esperaba etiqueta" ).c_str()
            );
		}
		if ( !NombreIdentificador::compruebaId( lex.getTokenActual() ) )
		{
			throw ESintxIdNoValido( std::string(
				"compilando " + mnemoStr + ", "
				"se esperaba etiqueta, "
				"y no: '"
			       + lex.getTokenActual()
			       + '\'' ).c_str()
			);
		}

		toret = new(std::nothrow) NMJmp( lex.getTokenActual() );

		if ( toret == NULL ) {
			throw ENoHayMemoria( "construyendo NMJmp" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ====================================================================== NMJot
const std::string NMJot::formatoXML = "jumpOnTrueTo";
const std::string NMJot::mnemoStr   = "JOT";

// ------------------------------------------------------------- NMJot::NMJot()
NMJot::NMJot(const std::string &m) : Flw(NJOT) {
              putNumero( atoi( m.c_str() ) );

	      if ( getNumero() == 0 ) {
	      	putEtiqueta( m );
	      }
}

// ------------------------------------------------------------ NMJot::listar()
std::string NMJot::listar(bool bello)
/// Obtener el detalle del mnemo en formato ensamblador
{
      std::string toret;

      if (!bello) {
        toret = mnemoStr;

        toret += ' ';
        toret += AnalizadorLexico::toString( getNumero() );
      }
      else {
        toret  = "if (acc == True) ";
        toret += NMJmp::formatoXML;

        toret += ' ';
        toret += AnalizadorLexico::toString( getNumero() );
        toret += ';';
      }

      return toret;
}

// ----------------------------------------------------- NMJot::getFormatoXML()
std::string NMJot::getFormatoXML()
/// Obtener el detalle del mnemo en formato XML
{
      std::string toret;

      toret += '<';

      // Referencia
      toret += formatoXML;
      toret += " value=\"";
      toret += AnalizadorLexico::toString( getNumero() );
      toret += "\"/>";

      return toret;
}

// ---------------------------------------------------------- NMJot::compilar()
NMJot * NMJot::compilar(AnalizadorLexico &lex)
{
	NMJot * toret = NULL;

	AnalizadorLexico::maysCnvt( lex.getToken() );

	if ( lex.getTokenActual() == mnemoStr )
	{
		// Coger el número de opcodes
		lex.pasaEsp();
		lex.getLiteral( '\n' );

		if ( lex.getTokenActual().empty() ) {
			throw ESintxIdNoValido( std::string(
				"compilando " + mnemoStr + ", "
				"se esperaba etiqueta" ).c_str()
			);
		}
		if ( !NombreIdentificador::compruebaId( lex.getTokenActual() ) )
		{
			throw ESintxIdNoValido( std::string(
				"compilando " + mnemoStr + ", "
				"se esperaba etiqueta, no: "
			      + lex.getTokenActual() ).c_str()
			);
		}

		toret = new(std::nothrow) NMJot( lex.getTokenActual() );

		if ( toret == NULL ) {
			throw ENoHayMemoria( "construyendo NMJot" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

// ====================================================================== NMJof
const std::string NMJof::formatoXML = "jumpOnFalseTo";
const std::string NMJof::mnemoStr   = "JOF";

NMJof::NMJof(const std::string &m) : Flw(NJOF)
{
	putNumero( atoi( m.c_str() ) );

	if ( getNumero() == 0 ) {
	putEtiqueta( m );
	}
}

// ------------------------------------------------------------ NMJof::listar()
std::string NMJof::listar(bool bello)
/// Devuelve la info del mnemo NMJof en formato ensamblador
{
      std::string toret;

      if (!bello) {
        toret = mnemoStr;

        toret += ' ';
        toret += AnalizadorLexico::toString( getNumero() );
      }
      else {
        toret  = "if (acc == False) ";
        toret += NMJmp::formatoXML;

        toret += ' ';
        toret += AnalizadorLexico::toString( getNumero() );
        toret += ';';
      }

      return toret;
}

// ----------------------------------------------------- NMJof::getFormatoXML()
std::string NMJof::getFormatoXML()
/// Devuelve la info del mnemo NMJof en formato XML
{
      std::string toret;

      toret += '<';

      // Referencia
      toret += formatoXML;
      toret += " value=\"";
      toret += AnalizadorLexico::toString( getNumero() );
      toret += "\"/>";

      return toret;
}

// ---------------------------------------------------------- NMJof::compilar()
NMJof * NMJof::compilar(AnalizadorLexico &lex)
{
	NMJof * toret = NULL;

	AnalizadorLexico::maysCnvt( lex.getToken() );

	if ( lex.getTokenActual() == mnemoStr )
	{
		// Coger la etiqueta del salto
		lex.pasaEsp();
		lex.getLiteral( '\n' );

		if ( lex.getTokenActual().empty() ) {
			throw ESintxIdNoValido( std::string(
				"compilando " + mnemoStr + ", "
				"se esperaba etiqueta" ).c_str()
			);
		}
		if ( !NombreIdentificador::compruebaId( lex.getTokenActual() ) )
		{
			throw ESintxIdNoValido( std::string(
				"compilando " + mnemoStr + ", "
				"se esperaba etiqueta, "
				"encontrado: "
			    + lex.getTokenActual() ).c_str()
			);
		}

		toret = new(std::nothrow) NMJof( lex.getTokenActual() );

		if ( toret == NULL ) {
			throw ENoHayMemoria( "construyendo NMJof" );
		}
	} else throw ESintxMnemoInvalido( lex.getTokenActual().c_str() );

	return toret;
}

}

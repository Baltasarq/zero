// zvm.cpp
/*
        Máquina virtual Zero

        La máquina virtual del sistema de programación Zero.
        Interpreta los bytecodes.
*/

#include "excep.h"
#include "cargaobj.h"
#include "runtime.h"
#include "uintf.h"
#include "analizadorlexico.h"
#include "vmcap.h"
#include "metodos.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <memory>

// ================================ Constantes ==============================

const std::string NOMBRE  = "zvm - Máquina virtual Zero (Zero's Virtual Machine)";
const std::string VERSION = "v2.5";
const std::string SERIAL  = "070601";

// Opciones de línea de comando
const std::string VERSION1 = "V";
const std::string VERSION2 = "VERSION";	// Vuelca el número de versión
const std::string NOPS     = "NOPS";		// Inhabilita el uso del PS
const std::string NOCON    = "NOCONSOLE";	// Inhabilita la consola
const std::string CHPSDIR  = "PSDIR";		// El directorio del PS


// ================================ Excepciones propias =======================
class ESintaxisInvocacion : public Zero::Excepcion {
public:
        ESintaxisInvocacion(const char * x) : Excepcion(x)
                { strcpy( detalles,
                         "Sintaxis:\nzvm -v      muestra los # vers. y serie\n"
                         "zvm -nops   ejecuta sin almacenamiento persistente\n"
                         "zvm -noconsole deshabilita la consola\n"
                         "zvm -psdir=path nuevo path para el PS\n"
                         "zvm <nombreArchivo>"
                         " [<nombreMetodo>]"
                         " [<argumento1> [<argumento2>] ...]\n\n"
                  );
                }
};

class ENadaQueHacer : public Zero::EEjecucion {
public:
        ENadaQueHacer(const char * x) : EEjecucion(x)
                { strcpy( detalles, "No hay objetos a ejecutar" ); }
};

// --------------------------------------------------------- procesarOpciones()
void procesarOpciones(char *argv[], unsigned int &numArg, unsigned int maxArg)
/*
	Procesa las opciones
*/
{
	unsigned int numMenos;
	std::string op = argv[1];

	numArg = 1;

	while( op[0] == '-'
	   &&  numArg < maxArg )
	{
		// Quitarle los menos de delante
		numMenos = 1;
		if ( op[1] == '-' ) {
			++numMenos;
		}
		op.erase( op.begin(), op.begin() + numMenos );

		Zero::AnalizadorLexico::maysCnvt( op );

        // Comprobar qué opción es
        if ( op == VERSION1
          || op == VERSION2 )
		{
			// Visualizar mensaje de info
			std::printf( "%s %s Serial %s\n\tOpcodes v %d.%d\n\t%s\n\n",
                    NOMBRE.c_str(), VERSION.c_str(), SERIAL.c_str(),
                    (int) Zero::hver, (int) Zero::lver,
                    VMCap::getInfoCapabilities().c_str()
            );
		}
		else
		if ( op == NOPS ) {
			// Desactiva el almacenamiento persistente (PS)
			VMCap::PS = false;
		}
		else
		if ( op == NOCON ) {
			// Desactiva la consola
			VMCap::consola = false;
		}
		else
		if ( op.substr( 0, CHPSDIR.length() ) == CHPSDIR ) {
			// Proporciona otro PATH al PS
			if ( op[CHPSDIR.length()] == '=' ) {
				op = argv[ numArg ];
				VMCap::PSDir = op.substr(
						CHPSDIR.length() + 2,
						op.length() )
				;
			} else throw ESintaxisInvocacion( "psdir sin '='" );
		}
		else throw ESintaxisInvocacion( std::string( "Opción inválida: " + op ).c_str() );

		++numArg;
		if ( numArg < maxArg ) {
			op = argv[numArg];
		}
	}

	return;
}

void terminar()
{
	printf( "\n*** FATAL: error inesperado en Zero\n" );
}

std::string getNombreObjeto(const std::string &nombreArchivo)
/**
	Pasar de un nombre de archivo del tipo:
		/apps/AplicacionAgenda a: AplicacionAgenda
*/
{
	std::string toret = nombreArchivo;
	size_t posBarra = toret.rfind( VMCap::getDirMark() );

	if ( posBarra != std::string::npos ) {
		toret.erase( 0, posBarra + 1 );
	}

	return toret;
}

// ================================= Main ===================================
int main(int argc, char* argv[])
{
	std::auto_ptr<Zero::Runtime> runtime;
	Zero::Metodo * doIt = NULL;
	std::auto_ptr<Zero::Runtime::MensajeArranque> mensaje;
	unsigned int numArg;
	int toret = EXIT_SUCCESS;

	std::set_unexpected( terminar );
	std::set_terminate( terminar );

	std::string LIB = Zero::OBJ_INTSTDLIB + Zero::EXT_ARCHIVOS_ZERO;

    try {
        if ( argc < 2 )
              throw ESintaxisInvocacion( "" );
        else {
			// Procesar opciones y comprobar que quedan suficientes
			procesarOpciones( argv, numArg, argc );
			if ( numArg >= argc ) {
				throw ENadaQueHacer( "falta argumento: objeto a ejecutar" );
			}

			// Cargar el fichero objeto
			// Preparar el nombre del fichero del sist. de ficheros
			std::string ficheroEntrada = argv[ numArg ];
			ficheroEntrada += Zero::EXT_ARCHIVOS_ZERO;

			// Preparar el nombre del objeto principal
			std::string nombreObjetoPpal = getNombreObjeto( argv[ numArg ] );
			++numArg;

			// Crear el runtime
			runtime.reset( Zero::Runtime::rt() );

			if ( runtime.get() == NULL ) {
			    throw Zero::EInterno( "imposible inicializar la MV Zero" );
			}

			// Cargar la librería estándar
			if ( runtime->estaEnInicializacion() )
			{
				Zero::FicheroSoporte lib( LIB, Zero::FicheroSoporte::Existente );
				Zero::CargadorObjetos cargador( &lib );
				runtime->prepara( cargador.getObjetosCargados(),
								Zero::Runtime::ES_LIB
				);
			}

			// Preparar el mensaje de arranque
			mensaje.reset( new Zero::Runtime::MensajeArranque );
			mensaje->push_back( nombreObjetoPpal );

			for (int i = numArg; i < argc; ++i)  {
					mensaje->push_back( std::string( argv[i] ) );
			}

			// Load & Run
			{
				Zero::FicheroSoporte obj( ficheroEntrada,
							Zero::FicheroSoporte::Existente
				);

				Zero::CargadorObjetos cargador( &obj );
				doIt = runtime->prepara(    cargador.getObjetosCargados(),
                                            Zero::Runtime::NO_ES_LIB,
                                            mensaje.get()
				);
			}

			runtime->ejecutar( doIt );
        }
	} catch (const Zero::Excepcion &e)
	{
            std::printf( "%s", e.getMensaje() );

			if ( e.hayDetalles() ) {
			    std::printf( " (%s)\n", e.getDetalles() );
			}

			toret = EXIT_FAILURE;
	}
	catch (...)
	{
			std::printf( "FATAL Error interno inesperado\n" );
			toret = EXIT_FAILURE;
	}

	return toret;
}

// ==========================================================================

// ------------------------------------------------------ Interfaz de usuario
// Funciones de captura y muestra de información
// --------------------------------------------------------- putLineaOutput()
void putLineaOutput(const std::string &x, bool cr)
/**
	Imprime una lin. de texto para la MV
*/
{
    static const char * fmtNoCr = "%s";
    static const char * fmtCr = "%s\n";
    static const char * fmt = fmtNoCr;

    if ( cr ) {
        fmt = fmtCr;
    }

    std::printf( fmt, x.c_str() );
}

// --------------------------------------------------------- getLineaOutput()
const std::string &getLineaInput(void)
/**
	Obtiene una lin. de texto pedida por la MV
*/
{
    int ch;
	static std::string toret;

    toret.clear();

    try {
        ch = getc( stdin );
        while ( ch != EOF
             && ch != '\n' )
        {
            toret.push_back( ch );
            ch = getc( stdin );
        }
    } catch(const std::exception &e) {
        throw Zero::EInputOutput( ( std::string( "Error de lectura(" ) + e.what() + ')' ).c_str() );
    }
    catch(...) {
        throw Zero::EInputOutput( "Error de lectura" );
    }

	return toret;
}

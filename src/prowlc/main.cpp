#include <cstdio>
#include <cstdlib>
#include <string>
#include <memory>

#include "parserprowl.h"
#include "excep.h"
#include "generadorcodigozero.h"

const std::string TituloApp  = "Prowl - PROtotype-Writing Language";
const std::string NombreApp  = "prowlc";
const std::string Titulo     = "A Zero's programming language";
const std::string Version    = "v2.0 20100831";
const std::string Author     = "jbgarcia@uvigo.es";

const std::string CmdVersion = "version";

void procesarParams(unsigned int argc, char * argv[], std::string &nombreArchivo)
{
    for(unsigned int i = 0; i < argc; ++i) {
        std::string opt = argv[ i ];

        // Es opt
        if ( opt[ 0 ] == '-' ) {
            // Eliminar guiones
            do {
                opt.erase( 0, 1 );
            } while ( opt[ 0 ] == '-' );

            // Ver posibilidades
            if ( opt == CmdVersion ) {
                std::printf( "%s(%s, %s)\n\n", NombreApp.c_str(), Version.c_str(), Author.c_str() );
            }

        } else {
            // Es nombre de archivo de entrada
            nombreArchivo = opt;
        }
    }
}

void abortar(Parser * parser)
{
    delete parser;
    printf( "[abortado]\n" );
    exit( EXIT_FAILURE );
}

std::string crearNombreArchivoSalida(const std::string &nombreArchivo)
{
    std::string toret = nombreArchivo;

    // quitar ext
    unsigned int pos = toret.rfind( '.' );

    if ( pos != std::string::npos ) {
        toret.erase( pos );
    }

    return toret;
}

int main(int argc, char * argv[])
{
    std::string nombreArchivoEntrada;
    std::string nombreArchivoSalida;
    std::auto_ptr<Parser> parser;
    std::auto_ptr<GeneradorCodigo> generador;

    // Inic
    std::printf( "%s - %s\n\t%s\n\n",
                 TituloApp.c_str(), Titulo.c_str(), Version.c_str()
    );

    // Params
    ++argv, --argc; // Salta el nombre del programa
    procesarParams( argc, argv, nombreArchivoEntrada );
    std::printf( "\nf: '%s'\n", nombreArchivoEntrada.c_str() );

    // Compilar
    try {
        try {
            if ( nombreArchivoEntrada.empty() ) {
                throw Zero::EInputOutput( "sin archivo de entrada" );
            }

            nombreArchivoSalida = crearNombreArchivoSalida( nombreArchivoEntrada );
            parser.reset( new ParserProwl( nombreArchivoEntrada ) );
            generador.reset( new GeneradorCodigoZero( nombreArchivoSalida, parser->getAST() ) );

            std::printf( "Compilando...\n" );
            parser->compilar();
            std::printf( "Generando...\n" );
            generador->generarCodigo();
        }
        catch(const Zero::Excepcion &e) {
            if ( parser.get() != NULL ) {
                      std::printf( "Ln %d: %s\n\t%s\n", parser->getNumeroLinea(), e.toString().c_str(), parser->getLineaActual().c_str() );
                      std::printf( "\t" );
                      for(unsigned int i = 0; i < parser->getPosLinea(); ++i) {
                          std::printf( "%c", ' ' );
                      }
                      std::printf( "%c\n", '^' );
            } else    std::printf( "%s\n", e.toString().c_str() );
            abortar( parser.get() );
        }
    }
    catch(const std::bad_alloc &e)
    {
        std::fprintf( stderr, "ERROR: Sin memoria(%s)\n", e.what() );
        abortar( parser.get() );
    }
    catch(const std::exception &e)
    {
        std::fprintf( stderr, "ERROR inesperado(%s)\n", e.what() );
        abortar( parser.get() );
    }
    catch(...) {
       abortar( parser.get() );
    }

    // Fin
    std::printf( "\nObj(s): %2d", parser->getNumeroDeObjetos() );
    std::printf( "\nEOF('%s')\n", generador->getNombreFichero().c_str() );
    return EXIT_SUCCESS;
}

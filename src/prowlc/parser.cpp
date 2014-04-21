// parser.cpp

#include "fileio.h"
#include "parser.h"

Parser::Parser(const std::string &nombreArchivo) throw( Zero::EInputOutput )
    : success( false ), flex( NULL )
{
    std::string nombreModulo = nombreArchivo;
    InputFile f( nombreArchivo );

    if ( !f.isOpen() ) {
        throw Zero::EInputOutput( nombreArchivo.c_str() );
    }

    // Crear nombre modl. (sin ext ni path)
    // quitar ext
    size_t pos = nombreModulo.rfind( '.' );

    if ( pos != std::string::npos ) {
        nombreModulo.erase( pos );
    }

    // quitar el path
    pos = nombreModulo.rfind( '/' );

    if ( pos == std::string::npos ) {
        pos = nombreModulo.rfind( '\\' );
    }

    if ( pos != std::string::npos ) {
        nombreModulo.erase( 0, pos +1 );
    }

    ast.reset( new AST( nombreModulo ) );
}

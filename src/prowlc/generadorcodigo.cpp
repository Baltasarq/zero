// generadorcodigo.cpp
/*
    Implementa algunos mth. del generador de cod. generic.
*/

#include "generadorcodigo.h"

#include "ast.h"
#include "fileio.h"
#include "excep.h"

GeneradorCodigo::GeneradorCodigo(const std::string &f, const AST *r) : raiz( r )
{
    // Abrir archivo de salida
    out.reset( new OutputFile( f ) );

    if ( !out->isOpen() ) {
        throw Zero::EInputOutput( raiz->getNombre().c_str() );
    }
}

const std::string &GeneradorCodigo::getNombreFichero() const
{
    return out->getFileName();
}

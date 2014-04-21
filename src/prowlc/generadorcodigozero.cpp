// generadorcodigozero.cpp
/*
    Implementa la clase que genera cod. para la máquina virtual Zero
*/

#include "generadorcodigozero.h"
#include "ast.h"
#include "fileio.h"
#include "mnemos.h"

void GeneradorCodigoZero::generarCodigo()
{
    OutputFile & out = getFichero();
    const ListaAst<Obj> &objetos = getRaiz()->objetos;

    for(size_t i = 0; i < objetos.size(); ++i) {
        out.writeLn( objetos[ i ]->generarCodigo() );
        out.writeLn( "" );
        out.writeLn( generarCodigoMiembros( objetos[ i ] ) );
        out.writeLn( "" );
        out.writeLn( Zero::NMEno::mnemoStr );
    }

    out.close();
}

std::string GeneradorCodigoZero::generarCodigoMiembros(const Obj *obj)
{
    std::string toret;
    const ListaAst<Mth> &metodos = obj->metodos;
    const ListaAst<Atr> &atributos = obj->atributos;

    // Generar atrs.
    for(size_t i = 0; i < atributos.size(); ++i) {
        toret.append( atributos[ i ]->generarCodigo() );
    }

    // Generar mths.
    for(size_t i = 0; i < metodos.size(); ++i) {
        toret.append( generarCodigoMetodo( metodos[ i ] ) );
    }

    return toret;
}

std::string GeneradorCodigoZero::generarCodigoMetodo(const Mth *mth)
{
    std::string toret = mth->generarCodigo();

    // Recorrer todas las instrucciones
    for(size_t i = 0; i < mth->instrucciones.size(); ++i) {
        toret.append( mth->instrucciones[ i ]->generarCodigo() );
        toret.append( "\n" );
    }

    // fin mth
    toret.append( Zero::NMEnm::mnemoStr );
    toret.push_back( '\n' );

    return toret;
}


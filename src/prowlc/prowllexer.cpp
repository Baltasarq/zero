#include "prowllexer.h"
#include "reservadas.h"

const std::string ProwlLexer::ReservadaObject = "object";
const std::string ProwlLexer::ReservadaObj = "obj";
const std::string ProwlLexer::ReservadaEndObject = "endobject";
const std::string ProwlLexer::ReservadaEndObj = "endobj";
const std::string ProwlLexer::ReservadaMethod = "method";
const std::string ProwlLexer::ReservadaMth = "mth";
const std::string ProwlLexer::ReservadaAttribute = "attribute";
const std::string ProwlLexer::ReservadaAttr = "atr";
const std::string ProwlLexer::ReservadaReference = "reference";
const std::string ProwlLexer::ReservadaRef = "ref";
const std::string ProwlLexer::ReservadaRet = "return";
const std::string ProwlLexer::ReservadaIf = "if";
const std::string ProwlLexer::ReservadaWhile = "while";
const std::string ProwlLexer::ReservadaGoto = "goto";
const std::string ProwlLexer::Begin = "{";
const std::string ProwlLexer::MarcaEtiqueta = ":";
const std::string ProwlLexer::End = "}";
const std::string ProwlLexer::OperadorHerencia = ":";
const std::string ProwlLexer::OperadorAcceso = ".";
const std::string ProwlLexer::OperadorAsigna = "=";
const std::string ProwlLexer::OperadorVisibilidadPrivada = "-";
const std::string ProwlLexer::OperadorVisibilidadPublica = "+";
const std::string ProwlLexer::OperadorSeparadorInstr = ";";
const std::string ProwlLexer::OperadorEnumeracion = ",";
const std::string ProwlLexer::OperadorIniParams = "(";
const std::string ProwlLexer::OperadorFinParams = ")";
const std::string ProwlLexer::Operadores = "+-*%/";
const std::string ProwlLexer::DelimitadorTexto = "\"";
const std::string ProwlLexer::OperadorFinInstruccion = ";";
const std::string ProwlLexer::PalabrasReservadas =
    " obj object endobject endobj atr attribute mth method ref reference return ret if for goto onexception "
    " enforce requires "
;

std::auto_ptr<ElementoLexico> ProwlLexer::getSigElemento()
{
    std::string token;
    std::auto_ptr<ElementoLexico> toret( GeneralLexer::getSigElemento() );

    if ( dynamic_cast<NoIdentificado *>( toret.get() ) != NULL ) {
        unsigned int linea = toret->getLinea();
        unsigned int columna = toret->getColumna();

        if ( Operadores.find( getCaracterActual() ) != std::string::npos ) {
            token.push_back( getCaracterActual() );
            toret.reset( new Operador( token, linea, columna ) );

            avanzar();
        }
        else
        if ( chkSig( Begin ) ) {
            toret.reset( new AperturaDeBloque( linea, columna ) );
            avanzar( Begin.length() );
        }
        else
        if ( chkSig( End ) ) {
            toret.reset( new CierreDeBloque( linea, columna ) );
            avanzar( End.length() );
        }
    }

    return toret;
}

void ProwlLexer::pasarFinInstr()
{
    if ( getCaracterActual() != OperadorFinInstruccion[ 0 ] )
            throw Zero::ESintaxis( "se esperaba ';'" );
    else    avanzar();
}

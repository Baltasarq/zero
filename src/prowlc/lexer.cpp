
#include <iterator>

#include "lexer.h"

const std::string GeneralLexer::AbrirComentario = "/*";
const std::string GeneralLexer::CerrarComentario = "*/";
const std::string GeneralLexer::ComentarioUnaLinea = "//";

GeneralLexer::GeneralLexer(const std::string &fn,
                           const std::string &resv,
                           const std::string &ac,
                           const std::string &cc,
                           const std::string &cul)
    throw( Zero::EInputOutput )
    : reservadas( resv ), abrirComentario( ac ),
      cerrarComentario( cc ), comentarioUnaLinea( cul )
{
    elemento.reset( NULL );

    try {
        flex = new FileLexer( fn );
    } catch(const std::runtime_error &e) {
        throw Zero::EInputOutput( e.what() );
    }
}

GeneralLexer::~GeneralLexer()
{
    delete flex;
}

bool GeneralLexer::chkSig(Lexer &flex, const std::string &ctrl)
{
    // Pasar delimitadores
    flex.skipDelim();

    // Apuntar los iteradores a las cadenas correctas
    const std::string &line = flex.getLine();
    std::string::const_iterator it2 = line.begin();
    std::string::const_iterator it = ctrl.begin();

    std::advance( it2, flex.getCurrentPos() );

    // Comparar
    for(; it != ctrl.end() && it2 != line.end(); ++it, ++it2) {
        if ( *it != *it2 ) {
            break;
        }
    }

    return ( it == ctrl.end() );
}

void GeneralLexer::pasarComentarioUnaLinea()
{
    if ( chkSig( *flex, getComentarioUnaLinea() ) ) {
        flex->skipLine();
    }
}

void GeneralLexer::pasarComentario()
{
    size_t pos;

    if ( chkSig( *flex, getAbrirComentario() ) ) {
        pos = flex->getCurrentLine().rfind( getCerrarComentario() );
        while( pos == std::string::npos ) {
            flex->skipLine();
            pos = flex->getCurrentLine().rfind( getCerrarComentario() );
        }

        flex->reset( pos + getCerrarComentario().length() );
    }
}

std::auto_ptr<ElementoLexico> GeneralLexer::getSigElemento()
{
    std::string token;
    Lexer::TokenType tipoToken = Lexer::Nothing;
    bool eraComentario = false;

    // Pasar los comentarios
    do {
        eraComentario = false;
        flex->skipDelim();
        tipoToken = flex->getNextTokenType();

        if ( tipoToken == Lexer::Nothing ) {

            std::string posibleComentario =
                    flex->getCurrentLine().substr(
                        flex->getCurrentPos(),
                        ComentarioUnaLinea.length()
            );

            if ( posibleComentario == getComentarioUnaLinea() ) {
                flex->advance( -1 );
                pasarComentarioUnaLinea();
                eraComentario = true;
            }
            else
            if ( posibleComentario == getAbrirComentario() ) {
                flex->advance( -1 );
                pasarComentario();
                eraComentario = true;
            }
        }
    } while( eraComentario );

    // Ya se ha encontrado algo
    if ( tipoToken == Lexer::Number ) {
        size_t pos = flex->getCurrentPos();
        Lexer::getNumber( flex->getCurrentLine(), pos, token, Lexer::StandardDelimiters );

        if ( token.find( '.' ) != std::string::npos )
                elemento.reset( new NumeroReal(  token, flex->getLineNumber(), flex->getCurrentPos() ) );
        else    elemento.reset( new NumeroEntero( token, flex->getLineNumber(), flex->getCurrentPos() ) );
    }
    else
    if ( tipoToken == Lexer::Text ) {
        elemento.reset( new Cadena( flex->getLiteral( "\"" ), flex->getLineNumber(), flex->getCurrentPos() ) );
    }
    else
    if ( tipoToken == Lexer::Id ) {
        token = flex->getToken();
        std::string posibleReservada = ' ' + StringMan::mins( token ) + ' ';

        if ( getReservadas().find( posibleReservada ) != std::string::npos )
                elemento.reset( new PalabraReservada( token, flex->getLineNumber(), flex->getCurrentPos() ) );
        else    elemento.reset( new Identificador( token, flex->getLineNumber(), flex->getCurrentPos() ) );
    }
    else elemento.reset( new NoIdentificado( flex->getLineNumber(), flex->getCurrentPos() ) );


    return elemento;
}

std::string GeneralLexer::getReservada(bool required)
{
    std::auto_ptr<ElementoLexico> elemento = getSigElemento();
    std::string toret = "";
    PalabraReservada * key = NULL;

    if ( ( key = dynamic_cast<PalabraReservada *>( elemento.get() ) ) != NULL ) {
        toret = key->getElemento();
    }

    if ( required
      && toret.empty() )
    {
        throw Zero::ESintaxis( "se esperaba palabra reservada" );
    }

    return toret;
}

std::string GeneralLexer::getIdentificador(bool required)
{
    std::auto_ptr<ElementoLexico> elemento = getSigElemento();
    std::string toret;
    Identificador * key;

    if ( ( key = dynamic_cast<Identificador *>( elemento.get() ) ) != NULL ) {
        toret = key->getElemento();
    } else {
        if ( required ) {
            throw Zero::ESintxIdNoValido( "reservado, incorrecto, o inexistente" );
        }
    }

    return toret;
}

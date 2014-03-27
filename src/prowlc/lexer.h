#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <stdexcept>

#include "elemento.h"
#include "lex.h"
#include "excep.h"

class GeneralLexer {
public:
    const static bool Required = true;
    const static bool NotRequired = false;
    const static std::string AbrirComentario;
    const static std::string CerrarComentario;
    const static std::string ComentarioUnaLinea;

    GeneralLexer(const std::string &fn,
                 const std::string &resv,
                 const std::string &ac = AbrirComentario,
                 const std::string &cc = CerrarComentario,
                 const std::string &cul = ComentarioUnaLinea)
        throw( Zero::EInputOutput );
    virtual ~GeneralLexer();

    const std::string &getNombreArchivo() const
        { return flex->getFileName(); }

    const std::string &getComentarioUnaLinea() const
        { return comentarioUnaLinea; }

    const std::string &getAbrirComentario() const
        { return abrirComentario; }

    const std::string &getCerrarComentario() const
        { return cerrarComentario; }

    const std::string &getReservadas() const
        { return reservadas; }

    std::string &getTexto(const std::string &delim)
        { return flex->getLiteral( delim ); }

    std::string &getNumero()
        { return flex->getNumber(); }

    virtual std::auto_ptr<ElementoLexico> getSigElemento();

    void pasarComentarioUnaLinea();
    void pasarComentario();
    void pasarDelimitadores()
        { flex->skipDelim(); }

    void avanzar(unsigned int num = 1)
        { flex->advance( num ); }

    char getCaracterActual() const
        { return flex->getCurrentChar(); }

    std::string getLineaActual() const
        { return flex->getCurrentLine(); }

    unsigned int getPosicion() const
        { return flex->getCurrentPos(); }

    void setPosicion(unsigned int pos)
        { return flex->reset( pos ); }

    unsigned int getNumLinea() const
        { return flex->getLineNumber(); }

    bool chkSig(const std::string &ctrl)
        { return chkSig( *flex, ctrl ); }

    bool esFin() const
        { return flex->isEnd(); }

    std::string getIdentificador(bool req = NotRequired);
    std::string getReservada(bool req = NotRequired);

    Lexer::TokenType getSigTipoToken()
        { return flex->getNextTokenType(); }

    static bool chkSig(Lexer &l, const std::string &ctrl);

protected:
    FileLexer * flex;
    std::auto_ptr<ElementoLexico> elemento;

    std::string reservadas;
    std::string abrirComentario;
    std::string cerrarComentario;
    std::string comentarioUnaLinea;
};

#endif // LEXER_H

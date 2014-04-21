// lex.cpp
/*
	Implementation of the lexical analyzer

	jbgarcia@uvigo.es
*/

#include <cctype>
#include <cstdlib>
#include <string>

#include "lex.h"

// ===================================================================== Lexer
const std::string Lexer::StandardDelimiters = " \t\n\r";

// ----------------------------------------------------------------- getToken()
std::string &Lexer::getToken(const std::string &lin,
                            size_t &pos,
                            std::string &token,
                            const std::string &delims)
/**
	Retrieve next token
*/
{
	token.erase();

	skipDelim( lin, pos, delims );

    char *ndx = const_cast<char *>( lin.c_str() ) + pos;
    char *end = ndx;

    while( ( std::isalpha( *end )
        ||   std::isdigit( *end )
        ||   *end == '_' )
     && *end != '\0' )
    {
        ++end;
    }

    token = lin.substr( pos, end - ndx );
    pos = ( end - lin.c_str() );

    return token;
}

// --------------------------------------------------------------- getLiteral()
std::string &Lexer::getLiteral(const std::string &lin,
				    size_t &pos,
				    const std::string &delim,
				    std::string &lit)
/**
    Retrieve a literal. Any character until delim
    @return the literal read
*/
{
	lit.erase();

    // Read everything until reaching quotes (or whatever)
    size_t posEnd = lin.find( delim, pos );

    if ( posEnd != std::string::npos )
    {
        lit = lin.substr( pos, posEnd - pos );
        posEnd += delim.length();
    }
    else {
        lit = lin.substr( pos, lin.length() );
        posEnd = lin.length();
    }

	pos = posEnd;

    return lit;
}

// ---------------------------------------------------------------- getNumber()
std::string &Lexer::getNumber(const std::string &lin,
				    size_t &pos,
				    std::string &num,
				    const std::string &delims)
/**
    Retrieve any number, decimal or floating point
*/
{
	skipDelim( lin, pos, delims );

	char *ndx = const_cast<char *>( lin.c_str() ) + pos;
	char *end = ndx;
	num.erase();

	// Is there a plus or minus sign ?
	if ( *end == '+'
	  || *end == '-' )
	{
		++end;
	}

    // Leerlo todo hasta encontrar algo que no sea un número
    while( ( std::isdigit( *end )
      || *end == '.' )
      &&   *end != '\0' )
    {
          ++end;
    }

	num = lin.substr( pos, end - ndx );
    pos = end - lin.c_str();

    return num;
}

// ------------------------------------------------------------------- skipDelim()
void Lexer::skipDelim(const std::string &lin, size_t &pos,
                      const std::string &delims, int avance)
/// Skip spaces and other delimiters.
{
	char * index = const_cast<char *>( lin.c_str() ) + pos;

    while( delims.find( *index ) != std::string::npos
        && *index != '\0' )
    {
            index += avance;
    }

	pos = index - lin.c_str();
	return;
}

// --------------------------------------------------------------- skipSpaces()
void Lexer::skipSpaces(const std::string &lin,
                        size_t &pos,
                        int avance)
/**
	Skip only spaces and tabs
*/
{
	char * index = const_cast<char *>( lin.c_str() ) + pos;

    while( ( *index == ' '
        ||   *index == '\t'
        ||   *index == '\r' )
     && *index != '\0' )
    {
            index += avance;
    }

	pos = index - lin.c_str();

	return;
}

bool isDelimiter(int ch, const std::string &delimiters)
{
    return ( delimiters.find( ch ) != std::string::npos );
}

Lexer::TokenType Lexer::getTokenType(int ch, const std::string &delimiters)
{
    TokenType toret;

    if ( std::isdigit( ch ) ) {
        toret = Number;
    }
    else
    if ( std::isalpha( ch ) ) {
        toret = Text;
    }
    else
    if ( isDelimiter( ch, delimiters ) ) {
        toret = Delimiter;
    }
    else toret = SpecialCharacter;


    return toret;
}

// =============================================================== StringLexer
//
StringLexer::StringLexer(std::string *x, const std::string &delims)
    : pos( 0 ), txt( x )
{
    StringMan::trimCnvt( *txt );

    if ( delims.empty() ) {
        delimiters = StandardDelimiters;
    }
}

// --------------------------------------------------------- getNextTokenType()
Lexer::TokenType StringLexer::getNextTokenType()
{
    char *aux;

    Lexer::skipDelim( *txt, pos, delimiters );

	// Get there
	aux = const_cast<char *>( txt->c_str() ) + pos;

	// What is it?
    if ( std::isalpha( *aux )
	  || *aux == '_' )
    {
        return Id;
    }
    else
    if ( std::isdigit( *aux )
     || *aux == '+'
     || *aux == '-' )
    {
        return Number;
    }
    else
    if ( *aux == '"' ) {
        return Text;
    }
    else
    return Nothing;
}

std::string &StringLexer::getLiteral(const std::string &delim)
/// @see Lexer::getLiteral
{
    return Lexer::getLiteral( *txt, pos, delim, token );
}

// ------------------------------------------------------------------ advance()
void StringLexer::advance(int avance)
{
    if ( avance < 0
      && ( (size_t) std::abs( avance ) ) > pos )
    {
        pos = 0;
    }
	else pos += avance;

	if ( pos > txt->length() ) {
		pos = txt->length();
	}
}

// ----------------------------------------------------...---- getCurrentChar()
char StringLexer::getCurrentChar()
{
	return (*txt)[ pos ];
}

// ================================================================== FileLexer
FileLexer::FileLexer(const std::string &fileName, const std::string &delims)
    : num( 0 ), numLinesSkipped( 0 )
/// Builds a file lexer, opening the file
{
    if ( delims.empty() ) {
        delimiters = StandardDelimiters;
    }

    f.open( fileName );

    if ( !f.isOpen() ) {
        throw std::runtime_error( "unable to open file" );
    }

    lex.reset( NULL );
}

void FileLexer::readNextLineFromFile()
{
    numLinesSkipped = 0;

    if ( f.isEof() ) {
        lin.clear();
        lex.reset( new StringLexer( &lin ) );
    }
    else {
        do {
            ++num;
            ++numLinesSkipped;
            f.readLn( lin );
            lex.reset( new StringLexer( &lin ) );
            lex->skipDelim();
        } while( !f.isEof()
              && lex->isEnd() );
    }
}

void FileLexer::getNextLineFromFile()
{
    if ( lex.get() == NULL
      || lex->isEol() )
    {
        readNextLineFromFile();
    }
}

bool FileLexer::delimiterFound(StringLexer &l, const std::string &delim)
{
    size_t pos = l.getCurrentPos();
    std::string linPos = l.getLine().substr( pos - delim.length(), delim.length() );

    return ( linPos == delim );
}

std::string &FileLexer::getLiteral(const std::string &delim)
{
    lex->setToken( lex->getLiteral( delim ) );

    while( !delimiterFound( *lex, delim ) ) {
        readNextLineFromFile();
        lex->appendToToken( lex->getLiteral( delim ) );
    }

    return lex->getCurrentToken();
}

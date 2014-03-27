// analizadorlexico.cpp
/*
	Implementación del analizador léxico.

	jbgarcia@uvigo.es
*/

#include <cstdio>
#include <cctype>
#include "analizadorlexico.h"
#include "reservadas.h"

namespace Zero {

// --------------------------------------------------------- AnalizadorLexico()

const unsigned int MaxBuffer = 128;
char AnalizadorLexico::buffer[MaxBuffer];

AnalizadorLexico::AnalizadorLexico(std::string *x) : pos(0), txt(x)
/**
	Construye un objeto AnalizadorLexico,
	tomando la cadena a analizar. La cadena es copiada,
*/
{
	trimCnvt( *txt );
}

// ---------------------------------------------------------------- getToken()
std::string &AnalizadorLexico::getToken()
{
	return getToken( *txt, pos, token );
}

// ---------------------------------------------------------------- getNumero()
std::string &AnalizadorLexico::getNumero()
{
	return getNumero( *txt, pos, token );
}

// ----------------------------------------------------------------- pasaEsp()
void AnalizadorLexico::pasaEsp()
{
	pasaEsp( *txt, pos );
}

// ------------------------------------------------------------- pasaSoloEsp()
void AnalizadorLexico::pasaSoloEsp()
{
	pasaSoloEsp( *txt, pos );
}

// -------------------------------------------------------------------- esEol()
bool AnalizadorLexico::esEol() const
{
	return ( pos >= txt->length() );
}

// ----------------------------------------------------------- siguienteToken()
AnalizadorLexico::TipoToken AnalizadorLexico::getTipoSiguienteToken()
{
        char *aux;

        pasaEsp( *txt, pos );

	// Llegarnos hasta allí
	aux = const_cast<char *>( txt->c_str() ) + pos;

	// ¿Qué es?
        if ( std::isalpha( *aux )
	  || *aux == '_'
	  || *aux == CHR_SEP_IDENTIFICADOR )
                return IDENTIFICADOR;
        else
        if ( std::isdigit( *aux )
         || *aux == '+'
         || *aux == '-' )
                return LITNUMERICO;
        else
        if ( *aux == '"' )
                return LITCADENA;
        else
        return NADA;
}

// ------------------------------------------------------------------- avanza()
void AnalizadorLexico::avanza(int avance)
{
	pos += avance;

	if ( pos > txt->length() ) {
		pos = txt->length();
	}
}

// -------------------------------------------------------- getCaracterActual()
char AnalizadorLexico::getCaracterActual() const
{
	return (*txt)[pos];
}

// --------------------------------------------------------------- getLiteral()
std::string &AnalizadorLexico::getLiteral(char limitador)
{
	return getLiteral( *txt, pos, limitador, token);
}

// ---------------------------------------------------------------- rTrimCnvt()
std::string &AnalizadorLexico::rTrimCnvt(std::string &x)
{
        unsigned int i      = x.length() - 1;

	pasaEsp( x, i , -1 );

        return x.erase( i + 1, x.length() );
}

// ---------------------------------------------------------------- lTrimCnvt()
std::string &AnalizadorLexico::lTrimCnvt(std::string &x)
{
        unsigned int i      = 0;

	pasaEsp( x, i );

        return x.erase( 0, i );
}

// ----------------------------------------------------------------- trimCnvt()
std::string &AnalizadorLexico::trimCnvt(std::string &x)
{
        rTrimCnvt( x );
        lTrimCnvt( x );

        return x;
}

// -------------------------------------------------------------------- rTrim()
std::string AnalizadorLexico::rTrim(const std::string &x)
{
	std::string cpx = x;

	return rTrimCnvt( cpx );
}

// -------------------------------------------------------------------- lTrim()
std::string AnalizadorLexico::lTrim(const std::string &x)
{
	std::string cpx = x;

	return lTrimCnvt( cpx );
}

// --------------------------------------------------------------------- trim()
std::string AnalizadorLexico::trim(const std::string &x)
{
	std::string cpx = x;

	return trimCnvt( cpx );
}


// ----------------------------------------------------------------- getToken()
std::string &AnalizadorLexico::getToken(const std::string &lin,
                                   unsigned int &pos,
				   std::string &token)
/**
	Obtener el siguiente token, versión estática
*/
{
	token.erase( token.begin(), token.end() );

	pasaEsp( lin, pos );

        char *ndx = const_cast<char *>( lin.c_str() ) + pos;

        while( ( std::isalpha( *ndx )
            ||   std::isdigit( *ndx )
            ||   *ndx == CHR_SEP_IDENTIFICADOR
            ||   *ndx == CHR_MET_SUPER
            ||   *ndx == '_' )
         && *ndx != '\0' )
        {
                token += *( ndx++ );
        }

	pos = ( ndx - lin.c_str() );

        return token;
}

// --------------------------------------------------------------- getLiteral()
std::string &AnalizadorLexico::getLiteral(const std::string &lin,
				    unsigned int &pos,
				    char delim,
				    std::string &lit)
{
	char *ndx = const_cast<char *>( lin.c_str() ) + pos;

	lit.erase( lit.begin(), lit.end() );

    // Leerlo todo hasta encontrar unas comillas (o lo que sea)
    while( ( *ndx        != delim
	   ||    *(ndx - 1)  == CHR_ESC_STR )
        &&   *ndx != '\0' )
    {
              lit += *ndx;
              ++ndx;
    }

	pos = ndx - lin.c_str();
    return lit;
}

// ---------------------------------------------------------------- getNumero()
std::string &AnalizadorLexico::getNumero(const std::string &lin,
				    unsigned int &pos,
				    std::string &num)
{
	pasaEsp( lin, pos );

	char *ndx = const_cast<char *>( lin.c_str() ) + pos;
	num.erase( num.begin(), num.end() );

	// Hay un más o un menos?
	if ( *ndx == '+'
	  || *ndx == '-' )
	{
		num += *ndx;
		++ndx;
	}

        // Leerlo todo hasta encontrar algo que no sea un número
        while( ( std::isdigit( *ndx )
	      || *ndx == '.' )
          &&   *ndx != '\0' )
        {
              num += *ndx;
              ++ndx;
        }

	pos = ndx - lin.c_str();
        return num;
}

// ------------------------------------------------------------------- pasaEsp()
void AnalizadorLexico::pasaEsp(const std::string &lin, unsigned int &pos, int avance)
/// Pasar los espacios (y más, no determinante), versión estática
{
	char * index = const_cast<char *>( lin.c_str() ) + pos;

        while( ( *index == ' '
            ||   *index == '\t'
            ||   *index == ','
	    ||   *index == '\n'
	    ||   *index == '\r' )
         && *index != '\0' )
        {
                index += avance;
        }

	pos = index - lin.c_str();

	return;
}

// ------------------------------------------------------------------ pasaEsp()
void AnalizadorLexico::pasaSoloEsp(const std::string &lin,
                                   unsigned int &pos,
				   int avance)
/**
	Pasar los espacios (sólo espacios), versión estática
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

// -------------------------------------------------------- compruebaFlotante()
bool AnalizadorLexico::compruebaFlotante(const std::string &s)
{
      bool toret = true;

      if ( ( s[0] == '-' )
        || ( std::isdigit( s[0] ) )
        || ( s[0] == '+' ) )
      {
        for(register unsigned int n = 1;n < s.length();++n)
        {
            if (  !std::isdigit( s[n] )
             && ( s[n]!='.' ) )
            {
                  toret = false;
                  break;
            }
        }

	// Veamos los puntos ...
	size_t posPunto = s.find( '.' );
	if ( posPunto != std::string::npos
	  && posPunto != ( s.length() - 1 ) )
	{
		// Si hay otro cualquiera, malo
		if ( s.find( '.', posPunto + 1) != std::string::npos ) {
			toret = false;
		}
	}

       } else toret = false;

      return toret;
}

// ------------------------------------------------------------------ maysCnvt()
std::string &AnalizadorLexico::maysCnvt(std::string &x)
{
       for(register unsigned int i = 0; i < x.length(); ++i)
       {
                x[i] = std::toupper( x[i] );
       }

       return x;
}

// ------------------------------------------------------------------ maysCnvt()
std::string AnalizadorLexico::mays(const std::string &x)
{
	std::string cpx = x;

	return maysCnvt( cpx );
}

// ------------------------------------------------------------------- toString()
std::string AnalizadorLexico::toString(void * x)
/**
    Convierte un puntero a cadena.
    @arg x Puntero a convertir
    @return El num. convertido a cadena
*/
{
    std::sprintf( buffer, "%p", x );

    return std::string( buffer );
}

// ------------------------------------------------------------------- toString()
std::string AnalizadorLexico::toString(int x, int w)
/**
    Convierte un num. a cadena. Si w es mayor que 0, entonces w es el ancho.
    Limitaciones: el ancho no puede ser mayor de 9.
    @arg x Num. a convertir
    @arg w Ancho a convertir
    @return El num. convertido a cadena
*/
{
    std::string fmt = "%d";

    w %= 10;
    if ( w > 0 ) {
        fmt.erase( 1 );
        fmt += '0';
        fmt += '0' + w;
        fmt += 'd';
    }

    std::sprintf( buffer, fmt.c_str(), x );

    return std::string( buffer );
}

// ------------------------------------------------------------------- toString()
std::string AnalizadorLexico::toString(unsigned int x, int w)
/**
    Convierte un num. a cadena. Si w es mayor que 0, entonces w es el ancho.
    Limitaciones: el ancho no puede ser mayor de 9.
    @arg x Num. a convertir
    @arg w Ancho a convertir
    @return El num. convertido a cadena
*/
{
    std::string fmt = "%u";

    w %= 10;
    if ( w > 0 ) {
        fmt.erase( 1 );
        fmt += '0';
        fmt += '0' + w;
        fmt += 'u';
    }

    std::sprintf( buffer, fmt.c_str(), x );

    return std::string( buffer );
}

// ------------------------------------------------------------------- toString()
std::string AnalizadorLexico::toString(unsigned long int x, int w)
/**
    Convierte un num. a cadena. Si w es mayor que 0, entonces w es el ancho.
    Limitaciones: el ancho no puede ser mayor de 9.
    @arg x num. a convertir
    @arg w ancho a convertir
    @return el num. convertido a cadena
*/
{
    std::string fmt = "%lu";

    w %= 10;
    if ( w > 0 ) {
        fmt.erase( 1 );
        fmt += '0';
        fmt += '0' + w;
        fmt += 'u';
    }

    std::sprintf( buffer, fmt.c_str(), x );

    return std::string( buffer );
}

// ------------------------------------------------------------------- toString()
std::string AnalizadorLexico::toString(double x, int wn, int we)
/**
    Convierte un num. a cadena. Si w es mayor que 0, entonces w es el ancho.
    Limitaciones: los anchos no puede ser mayor de 9.
    @arg x Num. a convertir
    @arg wn Ancho a convertir de todo el num. (mantisa + exp)
    @arg we Ancho a convertir del exponente
    @return El num. convertido a cadena
*/
{
    std::string fmt = "%f";

    wn %= 10;
    we %= 10;

    if ( wn > 0
      && we > 0 )
    {
        fmt.erase( 1 );
        fmt += '0';
        fmt += '0' + wn;
        fmt += '.';
        fmt += '0' + we;
        fmt += 'f';
    }

    std::sprintf( buffer, fmt.c_str(), x );

    return std::string( buffer );
}

// ------------------------------------------------------------------- toString()
int AnalizadorLexico::toInt(const std::string &s)
{
    int toret;

    std::sscanf( s.c_str(), "%d", &toret );

    return toret;
}

}

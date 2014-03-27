// zm2html

#include <cstdio>
#include <map>
#include <iostream>
#include <memory>

#include "analizadorlexico.h"

typedef std::map<std::string, std::string> Susts;
Susts cambiosHTML;

void convertirAcentos(std::string &lin)
{
	size_t pos;
	static const std::string acentos = "áéíóúñ";
	static const std::string cnvtAcentos[] = {
		"&aacute;",
		"&eacute;",
		"&iacute;",
		"&oacute;",
		"&uacute;",
		"&nacute;"
	};

	for(unsigned int i = 0; i < acentos.length(); ++i) {
		pos = lin.find( acentos[i] );
		while( pos != std::string::npos ) {
			lin.erase( pos, 1 );
			lin.insert( pos, cnvtAcentos[ i ] );

			pos = lin.find( acentos[i], pos );
		}
	}
}

void leerLinea(FILE  *f, std::string &lin)
{
	int c;

	// Leer una línea
	lin.erase();

	c = fgetc( f );
	while( c != EOF
	&& c != '\n' )
	{
		lin += c;
		c = fgetc( f );
	}
}

void convierteHTML(FILE *f)
{
	std::string unEspacio = "&nbsp;";
	std::string espacios = unEspacio;
    std::string lin;
	std::string token;
    FILE *out = fopen( "out.html", "wt" );
	Zero::AnalizadorLexico::TipoToken tk;
	bool esNuevaLinea;
    Susts::iterator it;

	espacios += unEspacio;

        // Cabecera html
        std::fprintf( out, "<html><head><Title>zm source</Title>\n</head>\n"
	              "<body>\n<code>\n\n"
        );

        while( !std::feof( f ) )
        {
            leerLinea( f, lin );

            // Analizar la lin.
            {
                std::auto_ptr<Zero::AnalizadorLexico> scan( new Zero::AnalizadorLexico( &lin ) );

                scan->pasaEsp();
                std::fprintf( out, "\n<br>" );
		esNuevaLinea = true;

                if ( scan->getCaracterActual() == '/'
   	          && !scan->esEol()
		  && scan->getLinea()[ scan->getPosActual() + 1] == '*' )
                {
			// Es un comentario
			do {
				convertirAcentos( lin );
				std::fprintf( out, "%s", espacios.c_str() );
				std::fprintf( out, "<font color=\"#0000FF\">" );
				std::fprintf( out, "<i>" );
				std::fprintf( out, "%s", lin.c_str() );
				std::fprintf( out, "</i>" );
				std::fprintf( out, "</font><br>" );

				leerLinea( f, lin );
			} while( lin.find( "*/" ) == std::string::npos );

			convertirAcentos( lin );
			std::fprintf( out, "%s", espacios.c_str() );
			std::fprintf( out, "<font color=\"#0000FF\">" );
			std::fprintf( out, "<i>" );
			std::fprintf( out, "%s", lin.c_str() );
			std::fprintf( out, "</i>" );
			std::fprintf( out, "</font><br>" );

			leerLinea( f, lin );

			scan.reset( new Zero::AnalizadorLexico( &lin ) );

			scan->pasaEsp();
			std::fprintf( out, "\n<br>" );
			esNuevaLinea = true;
                }

                if ( scan->getCaracterActual() == '!'
 		  || ( scan->getCaracterActual() == '/'
		    && !scan->esEol()
		    && scan->getLinea()[ scan->getPosActual() + 1] == '/' ) )
                {
			// Es un comentario
			std::fprintf( out, "%s", espacios.c_str() );
			std::fprintf( out, "<font color=\"#0000FF\">" );
                        std::fprintf( out, "<i>" );
                        std::fprintf( out, "%s", lin.c_str() );
                        std::fprintf( out, "</i>" );
			std::fprintf( out, "</font>" );
                }
		else
		if ( scan->getCaracterActual() == ':' )
		{
			// Es una etiqueta
			std::fprintf( out, "%s", espacios.c_str() );
			std::fprintf( out, "<font color=\"#C000C0\">" );
			std::fprintf( out, "%s", "<u>" );
            std::fprintf( out, "%s", lin.c_str() );
            std::fprintf( out, "%s", "</u>" );
			std::fprintf( out, "</font>" );
		}
		else
		if ( scan->getCaracterActual() == '}' )
		{
			// Calcular el nuevo espaciado
			espacios.erase( espacios.length()
				- ( unEspacio.length()
					* 2 ),
				espacios.length()
			);

			// Imprimir el cierre de llave
			std::fprintf( out, "%s", espacios.c_str() );
			std::fprintf( out, "<font color=\"#C000C0\"><b>}</b></font>" );
			std::fprintf( out, "%s", lin.substr( scan->getPosActual() + 1, lin.length() ).c_str() );

			if ( esNuevaLinea ) {
				std::fprintf( out, "%s", espacios.c_str() );
			}
		}
		else
		if ( scan->getCaracterActual() == '{' ) {
			std::string misEspacios = espacios;

			misEspacios.erase( misEspacios.length()
				- ( unEspacio.length()
					* 2 ),
				misEspacios.length()
			);

			// Imprimir la apertura de llave
			std::fprintf( out, "%s", misEspacios.c_str() );
			std::fprintf( out, "<font color=\"#C000C0\"><b>{</b></font>" );
			std::fprintf( out, "%s", lin.substr( scan->getPosActual() + 1, lin.length() ).c_str() );

			// Nuevo espaciado (no se cambia por que ya lo ha hecho method)
			if ( esNuevaLinea ) {
				std::fprintf( out, "%s", espacios.c_str() );
			}
		}
                else {
                    while( !scan->esEol() )
                    {
		    	    // Encontrar el tipo de token y cogerlo
		    	    do {
			      tk = scan->getTipoSiguienteToken();

			      switch ( tk )
			      {
			    	case Zero::AnalizadorLexico::IDENTIFICADOR:
                        scan->getToken();
                        break;
                    case Zero::AnalizadorLexico::LITNUMERICO:
                        scan->getNumero();

                        std::fprintf( out, "<font color=\"#828200\">" );

                        std::fprintf( out, "%s ",
                                 scan->getTokenActual().c_str()
                        );

                        std::fprintf( out, "</font>" );
                        break;
                    case Zero::AnalizadorLexico::LITCADENA:
                        scan->avanza();
                        scan->getLiteral( '"' );
                        scan->avanza();
                        token = scan->getTokenActual();
                        convertirAcentos( token );

                        std::fprintf( out, "<font color=\"#828200\">" );

                        std::fprintf( out, "\"%s\" ", token.c_str() );

                        std::fprintf( out, "</font>" );

                        break;
                    case Zero::AnalizadorLexico::NADA:
                        std::fprintf( out, "%c ",
                                 scan->getCaracterActual()
                        );
                        scan->avanza();
			      }
			    } while ( tk != Zero::AnalizadorLexico::IDENTIFICADOR
			           && !scan->esEol() );

		            // Procesar el token
		 	    if ( tk == Zero::AnalizadorLexico::IDENTIFICADOR
			      || !scan->esEol() )
			    {
                  it = cambiosHTML.find(
                        Zero::AnalizadorLexico::mays( scan->getTokenActual() )
                  );

                  if ( it != cambiosHTML.end() ) {
                    // Es una de las palabras clave
				    if ( it->first == "OBJECT" ) {
				    	if ( esNuevaLinea ) {
					  std::fprintf( out, "%s", espacios.c_str() );
					}
				    	espacios += unEspacio;
					espacios += unEspacio;
				    }
				    else
				    if ( it->first == "METHOD"
 				      || it->first == "DO" )
				    {
				    	if ( esNuevaLinea ) {
					  std::fprintf( out, "%s", espacios.c_str() );
					}
				    	espacios += unEspacio;
					espacios += unEspacio;
				    }
				    else
			            if ( it->first == "ENDOBJECT" ) {
				    	espacios.erase( espacios.length()
					              - ( unEspacio.length()
						         * 2 ),
						        espacios.length()
					);
				    	if ( esNuevaLinea ) {
					  std::fprintf( out, "%s", espacios.c_str() );
					}
				    }
				    else
				    if ( it->first == "ENDMETHOD" )
				    {
				    	espacios.erase( espacios.length()
					              - ( unEspacio.length()
						         * 2 ),
						        espacios.length()
					);
				    	if ( esNuevaLinea ) {
					  std::fprintf( out, "%s", espacios.c_str() );
					}
				    }
				    else {
				    	if ( esNuevaLinea ) {
					  std::fprintf( out, "%s", espacios.c_str() );
					}
				    }

				    esNuevaLinea = false;
                                    std::fprintf( out, "%s ", it->second.c_str() );
			      }
                              else {
			      	    if ( esNuevaLinea ) {
			            	std::fprintf( out, "%s", espacios.c_str() );
					esNuevaLinea = false;
				    }

			    	    std::fprintf( out, "%s ",
			    			  scan->getTokenActual().c_str()
				    );
			      }
			    }
                    }
                }
            }
         }

        // fin html
        std::fprintf( out, "\n\n</code>\n</body>\n</html>\n" );
}

int main(int argc, char *argv[])
{
        if ( argc!= 2 )
        {
                std::cout << "Embellecedor de macroensamblador Zero\n"
                     << "Sintaxis: zm2html <nf>" << std::endl;
        }
        else {
                // Crear los cambios html
                cambiosHTML[ "OBJECT"    ] = "<b>object</b>";
                cambiosHTML[ "ENDOBJECT" ] = "<b>endObject</b>";
                cambiosHTML[ "METHOD"    ] = "<b>method</b>";
                cambiosHTML[ "DO"    ]     = "<b>do</b>";
                cambiosHTML[ "ENDMETHOD" ] = "<b>endMethod</b>";
                cambiosHTML[ "RETURN"    ] = "<b>return</b>";
                cambiosHTML[ "REFERENCE" ] = "<b>reference</b>";
                cambiosHTML[ "JUMPONTRUETO" ]  = "<b>jumpOnTrueTo</b>";
                cambiosHTML[ "JUMPONFALSETO" ] = "<b>jumpOnFalseTo</b>";
                cambiosHTML[ "JUMPTO"    ] = "<b>jumpOnTrueTo</b>";
                cambiosHTML[ "TRUE"      ] = "<b>True</b>";
                cambiosHTML[ "FALSE"     ] = "<b>False</b>";
                cambiosHTML[ "NOTHING"   ] = "<b>Nothing</b>";


                FILE *f = std::fopen( argv[1], "rt" );

                if ( f != NULL )
                {
                        convierteHTML( f );

                        std::fclose( f );
                } else std::cout << "FATAL: fichero no encontrado" << std::endl;
        }
}


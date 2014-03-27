// za.cpp
/*
        Ensamblador de la máquina virtual Zero
        	Compila ficheros de texto con ensamblador nativo.

        jbgarcia@uvigo.es
*/

#include <iostream>
#include <cstdio>
#include <cctype>
#include <vector>
#include <sstream>
#include <memory>

#include "za.h"
#include "excep.h"

using namespace std;

// ================================ Constantes ===============================
const unsigned int MAXLIN    = 4096;

const string nombre          = "Za - Ensamblador Zero (Zero Assembler)";
const string version         = "v2.6";
const string serial          = "120509";
const string opOpt0          = "OPT0";
const string opOpt1          = "OPT1";
const string opOpt2          = "OPT2";
const string defext	         = ".def";

const string sintaxis        = "za [-" + opOpt0 + " | -" + opOpt1 + " | "
                               + opOpt2 + "]"
                               " <nombreFichero>\n\n\t-"
                               + opOpt0 + "\tSin optimizaciones"
                               "\n\t-" + opOpt1 + "\tOptimizaciones min."
                               "\n\t-" + opOpt2 + "\tOptimizaciones max."
                               " (por defecto)\n\n";

const char   MARCACOMENTARIO = ';';
const string VERSION_XML     = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>";

// ===================================================================== EnsamZ
Zero::Compilador *EnsamZ::c = NULL;

// ------------------------------------------------------------ EnsamZ::EnsamZ()
EnsamZ::EnsamZ(FILE *e, Zero::MedioSoporte *f, const std::string &nomf)
	: ent(e), sal(f), fichsalida(nomf), lin(0), estado(TOPLEVEL)
{
        c = new Zero::Compilador;

        if ( c == NULL ) {
                throw Zero::ENoHayMemoria( "Creando compilador" );
        }

        Zero::Cabecera().escribe( sal );
}

EnsamZ::~EnsamZ() {
        delete c;
}

// -------------------------------------------------------- EnsamZ::compNMObj()
void EnsamZ::compNMObj()
{
	if (estado == TOPLEVEL)
	{
		// Compilar
		Zero::AnalizadorLexico lex( &getLinActual() );
		std::auto_ptr<Zero::NMObj> mnemo( Zero::NMObj::compilar( lex ) );

		objetoActual = mnemo->getNombre();
		metodoActual = "";

		// Insertar en control de objetos
		if ( !idsProg.insertaObjeto( objetoActual,
		                             mnemo->getNomPadre() ) )
		{
			throw Zero::ESintxIdNoValido( "Nombre de objeto duplicado" );
		}

		// Preparar los estados
		literalesPorObjeto.clear();
		mnemo->escribe( sal );

		estado = ENOBJETO;
	} else throw Zero::ESintxObjNoPermitido( "" );
}

// -------------------------------------------------------- EsnamZ::compNMAtr()
void EnsamZ::compNMAtr()
{
	if ( estado == ENOBJETO )
	{
		// Es un atributo: asegurarse que no hay método actual
		metodoActual = "";

		// Compilar
		Zero::AnalizadorLexico lex( &getLinActual() );
		std::auto_ptr<Zero::NMAtr> mnemo( Zero::NMAtr::compilar( lex ) );

		if (  !(idsProg.buscaObjeto( objetoActual )->insertaAtributo(
					mnemo->getNombre(),
					mnemo->esAccesoPublico() )
			) )
		{
			throw Zero::ESintxIdNoValido( "Nombre de atributo duplicado" );
		}

		// Guardar el literal, si es necesario
		if ( !( mnemo->getLit().empty() ) )
		{
			Zero::Literal l( mnemo->getNombre(), mnemo->getLit() );

			// Guardar el literal
			literalesPorObjeto.push_back( l );
		}

		// Guardar
		mnemo->escribe( sal );
	}
	else throw Zero::ESintxAtrNoPermitido("");
}

// -------------------------------------------------------- EnsamZ::compNMEno()
void EnsamZ::compNMEno()
{
	if ( estado == ENOBJETO )
	{
        // Preparar el método que restaurará los atributos
		if ( literalesPorObjeto.size() > 0 )
		{
			(Zero::NMMth( Zero::NMMth::PUBLICO, Zero::MET_ALCARGAR )).escribe( sal );

			for(unsigned int n=0;n<literalesPorObjeto.size();++n)
			{
                                // Es una cadena ?
				if ( literalesPorObjeto[n].getTipo()
				                           == Zero::Literal::LitStr )
				{
					(Zero::NMStr(
					  literalesPorObjeto[n].getLiteral() ))
							.escribe( sal )
					;
				}
				else
                                // Número flotante ?
                                if ( literalesPorObjeto[n].getTipo()
                                                       == Zero::Literal::LitNumFlt )
                                {
                                      ( Zero::NMFlt(
                                        literalesPorObjeto[n].getLiteral() ) )
                                                      .escribe( sal )
                                      ;
                                }
                                else
                                // Número entero ?
                                if ( literalesPorObjeto[n].getTipo()
                                                       == Zero::Literal::LitNumInt )
                                {

                                    ( Zero::NMInt(
                                          literalesPorObjeto[n].getLiteral() ) )
                                    .escribe( sal );
                                }
                                else throw Zero::EInterno( "Literal sin sentido (?)" );

				// La asignación al atributo correspondiente
				( Zero::NMAsg(
				  literalesPorObjeto[n].getNombreAtributo(),
				  Zero::NombreRegistro::__ACC ) )
				.escribe( sal );
			}

			// Fin de metodo de restauración del objeto
			Zero::NMRet( Zero::LOC_RR ).escribe( sal );
			Zero::NMEnm().escribe( sal );
		}

		estado = TOPLEVEL;

		Zero::NMEno().escribe( sal );
	} else throw Zero::ESintxEnoNoPermitido( "" );
}

// ----------------------------------------------------------------- compNMth()
void EnsamZ::compNMMth()
{
	if (estado == ENOBJETO)
	{
		// Compilar
		Zero::AnalizadorLexico lex( &getLinActual() );
		std::auto_ptr<Zero::NMMth> mnemo( Zero::NMMth::compilar( lex ) );

		estado       = ENCUERPOMETODO;
		metodoActual = mnemo->getNombre();

		if ( !( idsProg.buscaObjeto( objetoActual )
				->insertaMetodo(
						mnemo->getNombre(),
						mnemo->esAccesoPublico(),
						*mnemo->getArgs()
				)
			)
		)
		throw Zero::ESintxIdNoValido( "Nombre de met. duplicado" );

		mnemo->escribe( sal );

		// Preparar la captura de mnemos del met.
		txtMet.clear();
	}
	else throw Zero::ESintxMthNoPermitido( "" );
}

// ----------------------------------------------------------------- compNMta()
void EnsamZ::compNMMta()
{
	if (estado == ENOBJETO)
	{
		// Compilar
		Zero::AnalizadorLexico lex( &getLinActual() );
		std::auto_ptr<Zero::NMMta> mnemo( Zero::NMMta::compilar( lex ) );

		mnemo->escribe( sal );
	}
	else throw Zero::ESintxMthNoPermitido( "" );
}


// -------------------------------------------------------- EnsamZ::compNMEnm()
void EnsamZ::compNMEnm(Zero::Optimizador::TipoOptimizacion opt)
{
	if (estado == ENEXCEPMETODO
     || estado == ENFINMETODO)
	{
		// Finalmente, compilar el método
		compilaMetodo( opt );

		// Método terminado
		estado = ENOBJETO;
		metodoActual = "";

		// Escribir el fin de metodo
		( Zero::NMEnm().escribe( sal ) );
	}
	else throw Zero::ESintxEnmNoPermitido( "" );
}

// -------------------------------------------------------- Ensamz::chkNMMsg()
void EnsamZ::chkNMMsg(Zero::NMMsg *mnemo)
{
	if ( buscaObjeto( mnemo->getReferencia()->getNombre() ) != NULL
	  && buscaMetodo( mnemo->getReferencia()->getNombre(),
				mnemo->getMetodo() ) == NULL)
	{
		warning( this, "El objeto no contiene met.: "
		            +  mnemo->getMetodo() )
		;
	}
}

// ----------------------------------------------------- EnsamZ::procesaLinea()
void EnsamZ::procesaLinea(Zero::Optimizador::TipoOptimizacion opt)
{
	std::string token;

	// Obtener el mnemo al ppio
	{
		Zero::AnalizadorLexico lex( &buffer );

		// Pasar los espacios
		lex.pasaEsp();

		// Es un comentario ? O una línea con espacios ?
		if ( lex.getCaracterActual() == MARCACOMENTARIO
		  || lex.esEol() )
		{
			goto FIN;
		}

		// Obtener el token del mnemo
		token = Zero::AnalizadorLexico::maysCnvt( lex.getToken() );
	}

	try {

        // Según el mnemónico, actuar
        if ( token == Zero::NMObj::mnemoStr ) {
            compNMObj();
        }
        else
        if ( token == Zero::NMEno::mnemoStr ) {
            compNMEno();
        }
        else
        if ( token == Zero::NMMth::mnemoStr ) {
            compNMMth();
        }
        else
        if ( token == Zero::NMEnm::mnemoStr ) {
            compNMEnm( opt );
        }
        else
        if ( token == Zero::NMAtr::mnemoStr ) {
            compNMAtr();
        }
        else
        if ( token == Zero::NMMta::mnemoStr
          && estado != ENCUERPOMETODO ) {
            compNMMta();
        }
        else
        if ( token == Zero::NMMta::mnemoStr ) {
                txtMet += buffer + '\n';
        }
        else
        if ( token == Zero::NMDef::mnemoStr ) {
            if ( estado == ENCUERPOMETODO
             ||  estado == ENEXCEPMETODO )
            {
                txtMet += buffer + '\n';
            }
            else throw Zero::ESintxDefNoPermitido( "" );
        }
        else
        if ( token == Zero::NMRet::mnemoStr ) {
		if ( estado == ENCUERPOMETODO
         ||  estado == ENEXCEPMETODO )
	        {
                if ( estado == ENCUERPOMETODO ) {
                    estado = ENEXCEPMETODO;
                }
                else {
                    if ( estado == ENEXCEPMETODO ) {
                        estado = ENFINMETODO;
                    }
                }

                txtMet += buffer  + '\n';
		}
		else throw Zero::ESintxRetNoPermitido( "" );
        }
        else
        if ( token == Zero::NMSet::mnemoStr ) {
            if ( estado == ENCUERPOMETODO
             ||  estado == ENEXCEPMETODO )
            {
                txtMet += buffer  + '\n';
            }
            else throw Zero::ESintxSetNoPermitido( "" );
        }
        else
        if ( token == Zero::NMAsg::mnemoStr ) {
            if ( estado == ENCUERPOMETODO
             ||  estado == ENEXCEPMETODO )
            {
                txtMet += buffer  + '\n';
            }
            else throw Zero::ESintxAsgNoPermitido( "" );
        }
        else
        if ( token == Zero::NMJmp::mnemoStr ) {
            if ( estado == ENCUERPOMETODO
             ||  estado == ENEXCEPMETODO )
            {
                txtMet += buffer  + '\n';
            }
            else throw Zero::ESintxJmpNoPermitido( "" );
        }
        else
        if ( token == Zero::NMJot::mnemoStr ) {
		if ( estado == ENCUERPOMETODO
         ||  estado == ENEXCEPMETODO )
        {
			txtMet += buffer + '\n';
		}
		else throw Zero::ESintxJotNoPermitido( "" );
        }
        else
        if ( token == Zero::NMJof::mnemoStr ) {
		if ( estado == ENCUERPOMETODO
         ||  estado == ENEXCEPMETODO )
        {
			txtMet += buffer  + '\n';
		}
		else throw Zero::ESintxJofNoPermitido( "" );
        }
        else
        if ( token == Zero::NMMsg::mnemoStr ) {
            if ( estado == ENCUERPOMETODO
             ||  estado == ENEXCEPMETODO )
            {
                txtMet += buffer  + '\n';
            }
            else throw Zero::ESintxMsgNoPermitido( "" );
        }
        else
        if ( token == Zero::NMStr::mnemoStr ) {
            if ( estado == ENCUERPOMETODO
             ||  estado == ENEXCEPMETODO )
            {
                txtMet += buffer  + '\n';
            }
            else throw Zero::ESintxStrNoPermitido( "" );
        }
        else
        if ( token == Zero::NMFlt::mnemoStr ) {
            if ( estado == ENCUERPOMETODO
             ||  estado == ENEXCEPMETODO )
            {
                txtMet += buffer  + '\n';
            }
            else throw Zero::ESintxFltNoPermitido( "" );
        }
        else
        if ( token == Zero::NMInt::mnemoStr ) {
            if ( estado == ENCUERPOMETODO
             ||  estado == ENEXCEPMETODO )
            {
                txtMet += buffer  + '\n';
            }
            else throw Zero::ESintxIntNoPermitido( "" );
        }
        else
        if ( token == Zero::NMIof::mnemoStr) {
            if ( estado == ENCUERPOMETODO
             ||  estado == ENEXCEPMETODO )
            {
                txtMet += buffer  + '\n';
            }
            else throw Zero::ESintxIofNoPermitido( "" );
        }
        else
        if ( token == Zero::NMTrw::mnemoStr) {
            if ( estado == ENCUERPOMETODO
             ||  estado == ENEXCEPMETODO )
            {
                txtMet += buffer  + '\n';
            }
            else throw Zero::ESintxTrwNoPermitido( "" );
        }
        else
        if ( token == Zero::NMNop::mnemoStr) {
		if ( estado == ENCUERPOMETODO
         ||  estado == ENEXCEPMETODO )
        {
			txtMet += buffer  + '\n';
		}
		else throw Zero::ESintxNopNoPermitido( "" );
        }
        else
        if ( token == Zero::NMEtq::mnemoStr) {
            if ( estado == ENCUERPOMETODO
             ||  estado == ENEXCEPMETODO )
            {
                txtMet += buffer  + '\n';
            }
            else throw Zero::ESintxEtqNoPermitido( "" );
        }
        else throw Zero::ESintxMnemoInvalido( token.c_str() );

	} catch(Zero::ECompilacion &e)
	{
		e.putLinMetodo( getNumeroLinea() );
		throw;
	}

	FIN:
	return;
}

// --------------------------------------------- EnsamZ::buscaAtributosObjeto()
Zero::Identificadores * EnsamZ::buscaAtributosObjeto(const std::string &x)
{
	Zero::IdsPorObjeto    * idsObj = idsProg.buscaObjeto( x );
	Zero::Identificadores * toret  = NULL;

	if ( idsObj != NULL ) {
		toret = idsObj->getAtributos();
	}

	return  toret;
}

// ----------------------------- ---------------------- EnsamZ::compilaMetodo()
void EnsamZ::compilaMetodo(Zero::Optimizador::TipoOptimizacion opt)
{
	Zero::Compilador::ListaMnemos l;
	Zero::Identificadores * idsMet  = buscaMetodo( objetoActual, metodoActual );
	Zero::Identificadores * atrsObj = buscaAtributosObjeto( objetoActual );

	if ( idsMet  != NULL
	  && atrsObj != NULL )
	{
		// Compilar el método
		c->reset( &txtMet, idsMet, atrsObj );
		l = c->compilar( opt );

		// Verif.
		for(unsigned int i = 0; i < l.size(); ++i)
		{
			// Comprueba, si es necesario
			if ( dynamic_cast<Zero::NMMsg*>( l[i] ) != NULL ) {
				chkNMMsg( (Zero::NMMsg *) l[i] );
			}

			// Escribe
			l[i]->escribe( sal );
		}

		// Pon cadena de avisos, si existe
		if ( !c->getCadenaAvisos().empty() )
		{
			warning( this, c->getCadenaAvisos() );
		}
	} else throw Zero::EInterno( ( objetoActual + '.'
                 + metodoActual + '(' + ')'
			     + " no encontrado." ).c_str()
           );

	return;
}

// --------------------------------------------------------- EnsamZ::sigLinea()
inline
void EnsamZ::sigLinea()
{
	char ch = fgetc( ent );

	buffer.erase( buffer.begin(), buffer.end() );

	while ( ch != '\n'
	    && !feof( ent ) )
	{
		buffer += ch;

		ch = fgetc( ent );
	}

        ++lin;
}

// --------------------------------------------------- EnsamZ::getNumeroLinea()
inline
const int EnsamZ::getNumeroLinea() const
{
	return lin;
}

// ----------------------------------------------------- EnsamZ::getLinActual()
inline
string &EnsamZ::getLinActual()
{
	return buffer;
}

// ----------------------------------------------------- EnsamZ::generaCodigo()
void EnsamZ::generaCodigo(Zero::Optimizador::TipoOptimizacion opt)
{
        do {
            // Leer siguiente línea
            sigLinea();

            // Compilar los mnemos
            procesaLinea( opt );
        } while( !feof( ent ) );

        sal->escribeBajoNivelUINT8( 0 );     // Marca de EOF
}

// ---------------------------------------------------- EnsamZ::buscaAtributo()
string EnsamZ::getAtributo(const std::string &objeto, unsigned int i)
{
        Zero::IdsPorObjeto *ob;

        if ( ( ob = idsProg.buscaObjeto( objeto ) ) != NULL)
        {
                return ob->getAtributo( i );
        }
        else throw Zero::EInterno("Objeto inexistente (?)");
}

// ------------------------------------------------ EnsamZ::buscaVbleAtributo()
bool EnsamZ::buscaVbleAtributo(const std::string &objeto,
                               const std::string &met, const std::string &v) const
{
        Zero::IdsPorObjeto *ob;
        Zero::IdsPorObjeto *obPadre;
        bool toret = false;

        ob = idsProg.buscaObjeto( objeto );
        if ( ob != NULL) {
                if ( !ob->buscaAtributo( v )) {
                  if ( !ob->buscaVbleMetodo(met, v) ) {
                        obPadre = idsProg.buscaObjeto(ob->getNombrePadre());
                        while (obPadre != NULL
                          &&   !toret
                          &&   obPadre->getNombre() != Zero::OBJ_OBJECT)
                        {
                                if ( obPadre->buscaAtributo( v ) )
                                {
                                        toret = true;
                                }

                                obPadre = idsProg.buscaObjeto(
                                                     obPadre->getNombrePadre());
                        }
                  } else toret = true;
                } else toret = true;
        } else toret = true;

        return toret;
}

// ------------------------------------------------------ EnsamZ::buscaMetodo()
Zero::Identificadores *
EnsamZ::buscaMetodo(const std::string &objeto, const std::string &met) const
{
        Zero::IdsPorObjeto *ob;
        Zero::IdsPorObjeto *obPadre;
        Zero::Identificadores *toret = NULL;

        if ( ( ob = idsProg.buscaObjeto(objeto)) != NULL )
        {
                if ( ( toret = ob->getIdsMetodo( met ) ) == NULL )
                {
                        obPadre = idsProg.buscaObjeto( ob->getNombrePadre() );

                        while ( obPadre != NULL
                           &&   obPadre->getNombre() != Zero::OBJ_OBJECT )
                        {
                                if ( ( toret = obPadre->getIdsMetodo( met ) )
				                                      != NULL )
                                {
                                    break;
                                }

                                obPadre = idsProg.buscaObjeto(
                                                     obPadre->getNombrePadre());
                        }
                }
        }

        return toret;
}

#ifdef TEXT_UI_MAIN

// =========================================================== Interfaz Consola
// ------------------------------------------------------------------ warning()
void warning(const string &x)
{
    cerr << "AVISO " << x << endl;
}

void warning(EnsamZ *za, const std::string &x)
{

    cerr << "AVISO " << x << endl;

    if ( za != NULL )
    {
          cerr  << " en: " << za->getNumeroLinea()
                << ':'     << ' ' << za->getLinActual()
                << '\n'
          ;

          if ( !za->objetoActual.empty() ) {

            cerr << "Compilando instr. "
	    	 << za->getNumeroLineaMetodo()
             << " de ";

            cerr << "\"" << za->objetoActual;

            if ( !za->metodoActual.empty() ) {
                    cerr   << '.'
                           << za->metodoActual << "()";
            }
            cerr << '\"' << '\n';
          }

          cerr << endl;
    }
}

void error(EnsamZ *za, const Zero::Excepcion &e)
{
	// Hubo un error
	cerr << e.getMensaje()  << ' ' << endl << "  "
		 << '(' << e.getDetalles() << ')'
		 << endl
	;

	// Montar un mensaje de error significativo
	if (za != NULL)
	{
		if ( !za->objetoActual.empty() ) {
			cerr << " compilando instr. ";

			if ( dynamic_cast<const Zero::ECompilacion *>( &e ) != NULL )
			{
				cerr << '+' << ((Zero::ECompilacion *) &e)->getLinMetodo();
			}
			else {
				cerr << za->getNumeroLineaMetodo();
			}

			cerr << " en \"" + za->objetoActual;

			if ( !za->metodoActual.empty() ) {
				cerr   << '.'
				       << za->metodoActual << "()";
			}
			cerr << '\"' << '\n';
		}

		cerr  << " en: " << za->getNumeroLinea()
		      << ':'   << ' ' << za->getLinActual()
		;

		cerr  << '.' << '\n' << endl;
	}
}

void procesarOpciones(int argc, char * argv[],
                      Zero::Optimizador::TipoOptimizacion &opt,
                      std::string &fich)
{
    int i = 1;
    string arg;

    // Procesar todos los argumentos
    for(; i < argc - 1; ++i ) {
        arg = argv[ i ];

        // Eliminar los guiones delante
        while( arg[ 0 ] == '-' ) {
            arg.erase( 0, 1 );
        }

        // Pasar a mays
        for(string::iterator it = arg.begin(); it != arg.end(); ++it) {
            *it = toupper( *it );
        }

        // Comprobar las opciones
        if ( arg == opOpt0 ) {
            opt = Zero::Optimizador::opt0;
        }
        else
        if ( arg == opOpt1 ) {
            opt = Zero::Optimizador::opt1;
        }
        else
        if ( arg == opOpt2 ) {
            opt = Zero::Optimizador::opt2;
        }
    }

    // Indicar el fichero de salida
    fich = argv[ i ];
}

// --------------------------------------------------------------------- main()
int main(int argc, char *argv[])
{
        std::auto_ptr<EnsamZ> za;
        std::auto_ptr<Zero::FicheroSoporte> salida;
//        Zero::Optimizador::TipoOptimizacion opt = Optimizador::opt2;
        Zero::Optimizador::TipoOptimizacion opt = Zero::Optimizador::opt1;

        cout << nombre       << ' ' << version
             << " serial "   << serial
             << " Opcodes v" << (int) Zero::hver << '.' << (int) Zero::lver
             << endl
        ;

        if ( argc != 2
          && argc != 3 )
        {
            cout << sintaxis << endl;
        }
        else {
            string fichSalida  = argv[ 1 ];
            string fichEntrada = argv[ 1 ];

            if ( argc == 3 ) {
                procesarOpciones( argc, argv, opt, fichEntrada );
                fichSalida = fichEntrada;
            }

            // ESTO ES TEMPORAL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            if ( opt > Zero::Optimizador::opt1 ) {
                cout << "Sorry, -opt2 still unstable" << endl;
                opt = Zero::Optimizador::opt1;
            }

            fichSalida += Zero::EXT_ARCHIVOS_ZERO;

            try {
                    salida.reset(
                        new Zero::FicheroSoporte( fichSalida, Zero::FicheroSoporte::Nuevo )
                    );

                    FILE * entrada = fopen( fichEntrada.c_str(), "rt" );

                    if ( entrada == NULL )
                    {
                        throw Zero::EMedioNoEncontrado( fichEntrada.c_str() );
                    }
                    else {
                        za.reset( new EnsamZ( entrada, salida.get(), fichSalida ) );

                        za->generaCodigo( opt );
                        fclose( entrada );

                        // Construir fichero XML de identificadores
                        FILE *ids = fopen( ( argv[1] + defext ).c_str(), "wt" );
                        fprintf( ids, "%s", za->idsProg.toXML().c_str() );
                        fclose( ids );

                        cout << "\n\nopt lvl: " << (int) opt
                             << "\tobjs: " << za->idsProg.getNumero();
                        cout << "\n\nFinalizado: " << fichSalida << endl;
                    }
                }
                catch(const Zero::Excepcion &e)
                {
                        error( za.get(), e );
                        exit(-1);
                }
        }

        return 0;
}

#endif

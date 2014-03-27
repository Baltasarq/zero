// zd.cpp
/*
      Desensamblador de ficheros Zero (zbj)
*/

#include "zd.h"
#include "mnemos.h"
#include "excep.h"

#include <iostream>
#include <vector>
#include <ctime>
#include <memory>

using namespace std;

// ----------------------------------------------------------------- Constantes
const string nombre             = "Zd - Desensamblador Zero (Zero Disassembler)";
const string version            = "v2.3";
const string serial             = "120509";
const string sintaxis           = "zd [-b|-x] <nombre_fichero.zbj>\n\n -b "
                                   "beautifier\n -x XML output";

const string defext		        = ".def";
const string XmlExt		        = ".xml";
const string TxtExt		        = ".txt";

const string COMENTARIO         = "; ";
const string FIN_COMENTARIO     = ".";
const string COMENTARIO_XML     = "<!-- ";
const string FIN_COMENTARIO_XML = " -->";
const string TAB                = "    ";
const string VERSION_XML        = "<?xml version=\"1.0\" encoding=\"ISO-8859-1"
                                   "\"?>";

// ================================================================== DesensamZ
// ----------------------------------------------------- DesensamZ::DesensamZ()
DesensamZ::DesensamZ(Zero::MedioSoporte *f, const std::string &nomf, bool xml, bool bello)
      : ent(f), nf(nomf), embellecedor(bello), fmtXml(xml)
{
      generarListado();
}

DesensamZ::~DesensamZ()
{
}

// ------------------------------------------------ DesensamZ::generarListado()
void DesensamZ::generarListado(void)
{
      std::time_t nHora = time(NULL);
      std::string cHora = ctime(&nHora);
      Zero::Cabecera cab;
      std::vector<Zero::Mnemotecnico *> vNM;
      Zero::Mnemotecnico *x;
      const std::string *empiezaComentario;
      const std::string *acabaComentario;
      std::string espacios;
      Zero::IdsPorObjeto *idsObj	= NULL;
      Zero::Identificadores * idsMet  = NULL;
      bool enObjeto = false;
      bool enMetodo = false;

      // Por ahora, es legal
      legal = true;

      // Decidir los comentarios a utilizar
      if ( !fmtXml ) {
        empiezaComentario = &COMENTARIO;
        acabaComentario   = &FIN_COMENTARIO;
      }
      else {
        empiezaComentario = &COMENTARIO_XML;
        acabaComentario   = &FIN_COMENTARIO_XML;
      }

      // Leer la cabecera
      cab.lee( ent );

      // Quitarle el salto de línea a la hora
      cHora.erase( cHora.length() - 1, 1 );

      // Empezar la transcripción
      if ( cab.esZeroLegal() )
      {

            if ( fmtXml )
            {
                listado << VERSION_XML << '\n' << '\n';
            }

            // Cabecera
            listado << *empiezaComentario   << cHora
                    << *acabaComentario     << endl
                    << *empiezaComentario   << nombre << ' ' << 'v'
                    << (int) Zero::hver     << '.'    << (int) Zero::lver
                    << *acabaComentario     << endl
                    << (fmtXml ? cab.getFormatoXML() : cab.listar())
                    << endl                 << endl   << endl
            ;

            if ( cab.hiVersion() <= Zero::hver )
            {
                  // cargar los opcodes
                  x = Zero::Mnemotecnico::cargar( ent );


		  // Guardar este opcode
                  if ( x != NULL )
                  {
                        vNM.push_back(x);
                  }

                  while ( !ent->esFinal()
                      &&  x != NULL )
                  {
                        x = Zero::Mnemotecnico::cargar( ent );

		  	// Guardar este opcode
                        if (x != NULL)
                        {
                              vNM.push_back(x);
                        }
                  }

                  if ( fmtXml )
                  {
                        listado << "<Program>\n\n"
                                << *empiezaComentario
                                << "Object definitions"
                                << *acabaComentario << '\n' << endl
                        ;
                  }

                  // Crear el listado
                  for(unsigned int i = 0;i < vNM.size(); ++i)
                  {
                         if (dynamic_cast<Zero::CtrOb *>(vNM[i]) != NULL
                          || dynamic_cast<Zero::CtrMt *>(vNM[i]) != NULL)
                         {
                                if (dynamic_cast<Zero::NMEno *>(vNM[i]) != NULL) {
                                        enObjeto = false;
					idsObj = NULL;
                                        espacios.erase(espacios.length() - 1 -
                                                         (TAB.length() - 1),
                                                       TAB.length()
					);
                                }
                                if (dynamic_cast<Zero::NMEnm *>(vNM[i]) != NULL) {
                                        enMetodo = false;
					idsMet   = NULL;
                                        espacios.erase(espacios.length() - 1 -
                                                         (TAB.length() - 1),
                                                       TAB.length());
                                }
                         }

                         if (dynamic_cast<Zero::Instr *>(vNM[i]) != NULL)
                         {
                            // Una instrucción. Debe estar dentro de cuerpo
                            if ( enObjeto
                             &&  enMetodo )
                            {
			     if ( dynamic_cast<Zero::NMDef*>( vNM[i] )  ) {
				     if ( idsMet != NULL )
				     {
				     idsMet->inserta(
					( (Zero::NMDef *) vNM[i])->getNombre()
				     );
				     } else {
					throw Zero::ESintxDefNoPermitido(
						"DEF sin MET"
					);
				     }
			     }

                             if ( fmtXml )
                                    listado << espacios
                                            << vNM[i]->getFormatoXML() << endl;
                             else  {
			     	 listado << espacios
                                            << vNM[i]->listar(embellecedor)
                                            << endl;
			     }
                            } else {
                                listado << *empiezaComentario
                                        << "Instrucc. fuera de mth o/y "
                                           "objeto"
                                        << *acabaComentario   << endl;
                                break;
                            }
                         }
                         else {
                             // Es un opcode de control: NMObj, NMMth ...
                             if ( fmtXml )
                                    listado << espacios
                                            << vNM[i]->getFormatoXML() << endl;
                             else   listado << espacios
                                            << vNM[i]->listar(embellecedor)
                                            << endl;
                         }

                         if (dynamic_cast<Zero::CtrOb *>(vNM[i]) != NULL
                          || dynamic_cast<Zero::CtrMt *>(vNM[i]) != NULL)
                         {
                                if (dynamic_cast<Zero::NMObj *>(vNM[i]) != NULL) {
                                        enObjeto = true;
                                        espacios += TAB;

					// Crear la info adecuada
					idsProg.insertaObjeto(
						( (Zero::NMObj *) vNM[i] )
							->getNombre(),
						( (Zero::NMObj *) vNM[i] )
							->getNomPadre()
					);

					idsObj = idsProg.buscaObjeto(
						( (Zero::NMObj *) vNM[i] )
							->getNombre()
					);
                                }

                                if (dynamic_cast<Zero::NMMth *>(vNM[i]) != NULL) {
                                        enMetodo = true;
                                        espacios += TAB;

					if ( idsObj != NULL ) {
						idsObj->insertaMetodo(
						  ( (Zero::NMMth *) vNM[i] )
							->getNombre(),
						  ( (Zero::NMMth *) vNM[i] )
							->esAccesoPublico(),
						  *( ( (Zero::NMMth *) vNM[i] )
							->getArgs() )
						);

						idsMet = idsObj->getIdsMetodo(
						  ( (Zero::NMMth *) vNM[i] )
							->getNombre()
						);
					} else throw Zero::ESintxMthNoPermitido(
						"MTH sin OBJ"
					       );
                                }
                         }

			 if ( dynamic_cast<Zero::NMAtr *>( vNM[i] ) ) {
				 if ( idsObj != NULL ) {
				   idsObj->insertaAtributo(
					( (Zero::NMAtr *) vNM[i] )->getNombre(),
				 	( (Zero::NMAtr *) vNM[i] )->esAccesoPublico()
				   );
				 } else {
					 throw Zero::ESintxAtrNoPermitido(
							 "ATR sin OBJ"
					);
				 }
			 }

                  }

                  if (fmtXml)
                  {
                     listado << "</Program>\n";
                  }
            }
            else listado << *empiezaComentario << "Versiones distintas: "
	    		 << cab.hiVersion()    << '/' << Zero::hver
                         << *acabaComentario   << endl;
      }
      else legal = false;
}

#ifdef UI_TEXT_MAIN
// ==================================================================== aviso()
void aviso(const string &x)
{
	std::cerr << "AVISO (warning): " << x << std::endl;
}

// ===================================================================== main()
int main(int argc, char *argv[])
{
      bool   embellecedor = false;
      bool   xml          = false;
      string fichentrada;
      string fichsalida;
      FILE *fsal;
      FILE *fxml;


      try {
         // Créditos
         cout << nombre << ' ' << version    << " serial " << serial
              << " Opcodes v"  << (int) Zero::hver << '.'
              << (int) Zero::lver    << endl       << endl;

         // Coger los argumentos
         if ( argc == 2 )
         {
		fichentrada  = fichsalida = argv[1];
		fichentrada += Zero::EXT_ARCHIVOS_ZERO;
		fichsalida  += TxtExt;
		cout << "Salida: ensamblador" << endl;
         }
         else
         if ( argc == 3 )
         {
                fichentrada  = fichsalida = argv[2];
                fichentrada += Zero::EXT_ARCHIVOS_ZERO;

                if (string( argv[1] ) == "-b"
                 || string( argv[1] ) == "/b")
                {
			embellecedor = true;
			fichsalida  += TxtExt;
			cout << "Salida: listado" << endl;
                } else if (string(argv[1])=="-x"
                        || string(argv[1])=="/x")
                        {
                                xml = true;
                                fichsalida  += XmlExt;
                                cout << "Salida: XML" << endl;
                        }
                        else {
				fichsalida  += TxtExt;
                                cout << "Salida: ensamblador" << endl;
                        }
         }
         else std::cout << sintaxis << std::endl << std::endl;

	 // Ahora sí, procesar
         if ( !fichentrada.empty() )
         {
                // Comprobar el fichero de entrada
                std::auto_ptr<Zero::FicheroSoporte> ent(
                    new(std::nothrow) Zero::FicheroSoporte( fichentrada,
                                Zero::FicheroSoporte::Existente
                    )
                );

                if ( ent.get() != NULL )
                {
                      string listado;
                      DesensamZ dsm( ent.get(), fichentrada, xml, embellecedor );
                      fsal = std::fopen( fichsalida.c_str(), "wt" );

                      if ( fsal != NULL )
                      {
                            if ( dsm.esZeroLegal() )
                            {
                                // Escribir listado en el formato pedido
                                fprintf( fsal, "%s", dsm.getListado().c_str() );
                                fclose( fsal );

                                // Escribir fichero .def
                                fxml = fopen(
                                    ( fichsalida + defext ).c_str(),
                                    "wt"
                                );

                                if ( fxml != NULL ) {
                                    fprintf( fxml, "%s",
                                         dsm.idsProg.toXML().c_str()
                                    );
                                    fclose( fxml );
                                } else aviso( "No se pudo crear fichero .def" );
                            }
                            else throw Zero::ENoEsZero( fichentrada.c_str() );
                      }
                      else throw Zero::EMedioNoEncontrado( fichsalida.c_str() );

                      cout << "Finalizado: " << fichsalida << endl;
                }
                else throw Zero::ENoHayMemoria( ( std::string( "abriendo: " ) + fichentrada ).c_str() );
         }
      } catch (const Zero::Excepcion &e)
      {
            std::cerr << e.getMensaje() << '(' << e.getDetalles() << ')' << endl;
            exit( EXIT_FAILURE );
      }
}

#endif


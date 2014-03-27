// opcodes.cpp
/*
        Los opcodes y los mnemotécnicos son partes del mismo concepto, las
	instrucciones.
	De éstas, los primeros constituyen la funcionalidad, y los segundos
	su representación.

        Ésta es la implantación de los opcodes, que es lo que hace que los
        mnemotécnicos puedan realmente "hacer" cosas.

        Necesita mnemos.h y mnemos.cpp como parte fundamental.

        Pertenece a zvm

        jbgarcia@uvigo.es
*/

#include "opcodes.h"
#include "runtime.h"
#include "servnom.h"
#include "metodos.h"

namespace Zero {

// ================================== Opcode ===================================

// -------------------------------------------------------------- Opcode::Opcode
Opcode::Opcode(Metodo *m) : met(m), enFase(false), mnemo(NULL)
{
        deSincroniza();
}

// ------------------------------------------------------------- Opcode::~Opcode
Opcode::~Opcode()
{
        // Borrar mnemotécnico asociado al opcode
        delete mnemo;
        mnemo = NULL;
}

// ------------------------------------------------------  Opcode::estaEnFase()
inline
bool Opcode::estaEnFase() const
{
    return ( enFase == met->getFase() );
}

// -------------------------------------------------- Opcode::creaOpcodeAdecuado
Opcode *Opcode::creaOpcodeParaMnemo(Mnemotecnico *mnemo, Metodo *met)
{
        Opcode *toret;

        try {
            if (dynamic_cast<NMMsg *>(mnemo) != NULL)
            {
                    toret = new OcMSG((NMMsg *) mnemo, met);
            }
            else
            if (dynamic_cast<NMStr *>(mnemo) != NULL) {
                    toret = new OcSTR((NMStr *) mnemo, met);
            }
            else
            if (dynamic_cast<NMRet *>(mnemo) != NULL) {
                    toret = new OcRET((NMRet *) mnemo, met);
            }
            else
            if (dynamic_cast<NMSet *>(mnemo) != NULL) {
                    toret = new OcSET((NMSet *) mnemo, met);
            }
            else
            if (dynamic_cast<NMAsg *>(mnemo) != NULL) {
                    toret = new OcASG((NMAsg *) mnemo, met);
            }
            else
            if (dynamic_cast<NMDef *>(mnemo) != NULL) {
                    toret = new OcDEF((NMDef *) mnemo, met);
            }
            else
            if (dynamic_cast<NMJmp *>(mnemo) != NULL) {
                    toret = new OcJMP((NMJmp *) mnemo, met);
            }
            else
            if (dynamic_cast<NMJot *>(mnemo) != NULL) {
                    toret = new OcJOT((NMJot *) mnemo, met);
            }
            else
            if (dynamic_cast<NMJof *>(mnemo) != NULL) {
                    toret = new OcJOF((NMJof *) mnemo, met);
            }
            else
            if (dynamic_cast<NMTrw *>(mnemo) != NULL) {
                    toret = new OcTRW((NMTrw *) mnemo, met);
            }
            else
            if (dynamic_cast<NMFlt *>(mnemo) != NULL) {
                    toret = new OcFLT((NMFlt *) mnemo, met);
            }
            else
            if (dynamic_cast<NMIof *>(mnemo) != NULL) {
                    toret = new OcIOF((NMIof *) mnemo, met);
            }
            else
            if (dynamic_cast<NMInt *>(mnemo) != NULL) {
                    toret = new OcINT((NMInt *) mnemo, met);
            }
            else
            if (dynamic_cast<NMNop *>(mnemo) != NULL) {
                    toret = new OcNOP((NMNop *) mnemo, met);
            }
            else throw EOpcodeIlegal( mnemo->listar( false ).c_str() );
        } catch(const std::bad_alloc &) {
                throw ENoHayMemoria( "creando opcode de mnemo" );
        }

        return toret;
}

// --------------------------------------------------- Opcode::asignarReferencia
inline
void Opcode::asignarReferencia(Referencia *dest, Referencia *org)
{
	if ( dest != org ) {
		dest->ponReferencia( org->getObjeto() );
	}
}

// --------------------------------------------------------- Opcode::resuelveReg
inline
Referencia *Opcode::resuelveReg(NombreRegistro *reg)
{
    unsigned int numReg = reg->getCodigoRegistro();

    if ( numReg < NombreRegistro::__NOREG )
            return met->reg[ numReg ];
    else    return NULL;
}

// ------------------------------------------------ Opcode::resuelveRefORegEnRef
inline
Referencia *Opcode::resuelveRefORegEnRef(Nombre *reforeg)
{
    NombreRegistro *reg = dynamic_cast<NombreRegistro *>( reforeg );

    if ( reg != NULL)
            return resuelveReg( (NombreRegistro *) reforeg );
    else    return Referencia::buscaReferencia(
                                    reforeg->getNombre(),
                                    met->getPerteneceA(),
                                    met
            );
}

// ------------------------------------------------ Opcode::resuelveRefORegEnLoc
inline
Referencia *Opcode::resuelveRefORegEnLoc(Nombre *reforeg)
{
    NombreRegistro *reg = dynamic_cast<NombreRegistro *>( reforeg );

    if ( reg != NULL)
            return resuelveReg( (NombreRegistro *) reforeg );
    else    return Referencia::buscaVbleOAtributo(
                                    met->getPerteneceA(),
                                    met,
                                    reforeg->getNombre()
            );
}

inline
Referencia *Opcode::resuelveRegORefLoc(Nombre *locOreg)
{
    NombreRegistro *reg = dynamic_cast<NombreRegistro *>( locOreg );

    if ( reg != NULL)
            return resuelveReg( (NombreRegistro *) locOreg );
    else    return met->vblesLocales.busca( locOreg->getNombre() );
}

// ================================== ocSTR ====================================
// ---------------------------------------------------------------- OcSTR::OcSTR
OcSTR::OcSTR(NMStr  *mn , Metodo *m) : Opcode(m)
{
        mnemo = new(std::nothrow) NMStr( *mn );

	if ( mnemo == NULL ) {
		throw ENoHayMemoria( "creando opcode STR" );
	}
}

// --------------------------------------------------------------- OcSTR::~OcSTR
OcSTR::~OcSTR()
{
}

// -------------------------------------------------------------- OcSTR::copia()
OcSTR * OcSTR::copia(Metodo *met)
{
	OcSTR * toret = new(std::nothrow) OcSTR( getMnemo(), met );

	if ( toret == NULL ) {
		throw ENoHayMemoria( "duplicando opcode STR" );
	}

	return toret;
}

// -------------------------------------------------------------- OcSTR::ejecuta
void OcSTR::ejecuta()
{
        std::string * s     = &( const_cast<std::string &>( getMnemo()->getLiteral() ) );
        Objeto * toret = Runtime::gestorCadenas->nuevo( "", *s );

        if ( toret != NULL )
             met->getLocAcc()->ponReferencia( toret );
        else met->getLocAcc()->ponReferencia( Runtime::objetoNulo );
}

// ================================== ocMSG ====================================
// ---------------------------------------------------------------- OcMSG::OcMSG
OcMSG::OcMSG(NMMsg  *mn, Metodo *m)
   : Opcode(m), ref(NULL), huboExcepcion(false), super(false)
{
	const std::string &metMnemo = mn->getMetodo();
    mnemo = new(std::nothrow) NMMsg( *mn );

    args.clear();

	if ( mnemo == NULL ) {
		throw ENoHayMemoria( "creando opcode MSG" );
	}

	// Comprobar si es un método con llamada a super, como ^foo.
	if ( *( metMnemo.c_str() ) == CHR_MET_SUPER ) {
		metodo = ( metMnemo.c_str() + 1 );
		super  = true;
	}
	else	metodo = metMnemo;
}

// ------------------------------------------------------ OcMSG::deSincroniza()
void OcMSG::deSincroniza()
{
    if ( estaEnFase() )
    {
        Opcode::deSincroniza();
        ref = NULL;
    }
}
// --------------------------------------------------------------- OcMSG::~OcMSG
OcMSG::~OcMSG()
{
    ref = NULL;
}

// -------------------------------------------------------------- OcMSG::copia()
OcMSG * OcMSG::copia(Metodo *met)
{
	OcMSG * toret = new(std::nothrow) OcMSG( getMnemo(), met );

	if ( toret == NULL ) {
		throw ENoHayMemoria( "duplicando opcode MSG" );
	}

	return toret;
}

void OcMSG::preparaArgumentos()
/**
        Los argumentos son siempre variables locales,
        así que nunca cambian. Sólo deben de hacerse una vez (siempre y cuando
        el mth no sea recursivo y haya cambiado de mth).
*/
{
        register size_t numArgs = getArgumentos()->size();
		Referencia * refLocObjeto;

        if ( args.empty()
          || met->esRecursivo() )
        {
            if ( args.empty() ) {
                args.reserve( numArgs );
                args.insert( args.begin(), numArgs, NULL );
            }

            for(register size_t n = 0; n < numArgs; ++n)
            {
                // Buscar entre los registros y variables locales
                refLocObjeto = resuelveRegORefLoc( (*getArgumentos())[n] );

                // Argumento no encontrado
                if ( refLocObjeto == NULL ) {
                    Runtime::ponExcepcionObjeto(
                            EXC_SNDMSG
                              + "in '"
                              + met->getPerteneceA()->getNombre()
                              + CHR_SEP_IDENTIFICADOR + met->getNombre()
                              + '(' + "unexisting argument: '"
                              + (*getArgumentos())[n]->getNombre()
                              + '\''+ ')' + '\''
                    );

                    break;
                }

                args[ n ] = refLocObjeto;
            }
        }

        return;
}

// ---------------------------------------------------- OcMSG::preparaArgumentos
void OcMSG::preparaReferencias()
{
        // Encontrar la referencia
        if ( ref == NULL
         || !estaEnFase() )
        {
                deSincroniza();

                if ( getMnemo()->getArgs()->size() != 0 ) {
                    preparaArgumentos();
                }

                // Preparar la referencia
                ref = resuelveRefORegEnRef( getMnemo()->getReferencia() );
        }

        if ( ref == NULL )
        {
                Runtime::ponExcepcionObjeto( EXC_SNDMSG
                                + "to unexisting recipient: "
		                + getMnemo()->getNombreDeObjeto()
				);
				huboExcepcion = true;
        }
		else {
			// Localizar el objeto
			Objeto * objPartida = obj = ref->getObjeto();

			if ( !estaEnFase() ) {
					// Colocar la fase
					enFase = met->getFase();
			}

			// ¿Empezamos desde "abajo", o desde el método actual?
			if ( super ) {
				Referencia * refPadre =
								met->getPerteneceA()->getAtrObjetoPadre()
				;

				if ( refPadre != NULL ) {
					objPartida = refPadre->getObjeto();
				}
			}

			// Buscar el método
			metDest = Metodo::encuentraMetodo( objPartida, metodo );

			if ( metDest == NULL ) {
				huboExcepcion = true;
				Runtime::ponExcepcionMetodo(
								EXC_SNDMSG
								+ "to unexisting method: "
								+ getMetodo()
								+ " in ref: "
								+ getReferencia()->getNombre()
								+ '('
								+ obj->getNombre()
								+ ')'
				);
			}
		}
}

// -------------------------------------------------------------- OcMSG::ejecuta
void OcMSG::ejecuta()
{
        // Preparar las referencias
		huboExcepcion = false;
        preparaReferencias();

        // Ejecución
        if ( !huboExcepcion ) {
                // Comprobar argumentos
                if ( args.size() != metDest->argumentosFormales.size() ) {
                    Runtime::ponExcepcion(
                        Runtime::objetoExcepcion, EXC_ARGNUMBER
                            + metDest->getPerteneceA()->getNombre()
                            + '.' + metDest->getNombre() + '(' + ')'
                    );
                }
                else
                if ( !Metodo::chkVisible( metDest, obj ) ) {
                     Runtime::ponExcepcion(
                        Runtime::objetoEPrivado,
                        EXC_NOVISIBLE + metDest->getNombre() + '\''
                     );
                } else {

    if ( metDest->esRecursivo() ) {
        printf( "***Start EXE mth recursivo, Mth es: '%s'\n", metDest->getNombre().c_str() );
    }
                    // Ejecutar el mth
                    Objeto * toret = metDest->ejecuta( &args, obj );

    if ( metDest->esRecursivo() ) {
        printf( "***End EXE mth recursivo, Mth es: '%s'\n", metDest->getNombre().c_str() );
    }

		   			// Llamar al mth de herencia dinámica
                    if ( obj->getMetHerenciaDinamica() != NULL ) {
		    			obj->procHerenciaDinamica();
					}

					// Si es recursivo, no se necesita más
					if ( metDest->esRecursivo() ) {
					    met = metDest->getMetodoAnterior();

					    if ( met == NULL ) {
					        printf( "\n*** ERROR GORDO ***, el metodo anterior es: NULL\n" );
                            printf( "Mth es: '%s'\n", metDest->getNombre().c_str() );
					        fflush( stdout );
					    } else {
					        printf( "\n*** Eliminando mth recursivo *** mth rec: '%s' mth anterior es '%s'\n",
                                    metDest->getNombre().c_str(), met->getNombre().c_str());
					    }

                        // Almacenar resultado en acc de llamador
                        met->getLocAcc()->ponReferencia( toret );

					    // Indicar fin de ejecución
					    metDest->finEjecucion();

					    metDest->descuentaRecursivo();
					    metDest->deSincroniza( met );
					    metDest->getPerteneceA()->lMets.borra( metDest );
					} else {
                        // Almacenar resultado en acc de llamador
                        met->getLocAcc()->ponReferencia( toret );

					    // Indicar fin de ejecución
					    metDest->finEjecucion();
					}

                }
        }
}

// ================================== OcRET ====================================
// ---------------------------------------------------------------- OcRET::OcRET
OcRET::OcRET(NMRet  *mn , Metodo *m) : Opcode(m)
{
        mnemo = new(std::nothrow) NMRet( *mn );

	if ( mnemo == NULL ) {
		throw ENoHayMemoria( "creando opcode RET" );
	}
}

// --------------------------------------------------------------- OcRET::~OcRET
OcRET::~OcRET()
{
}

// -------------------------------------------------------------- OcRET::copia()
OcRET * OcRET::copia(Metodo *met)
{
	OcRET * toret = new(std::nothrow) OcRET( getMnemo(), met );

	if ( toret == NULL ) {
		throw ENoHayMemoria( "duplicando opcode RET" );
	}

	return toret;
}

// -------------------------------------------------------------- OcRET::ejecuta
void OcRET::ejecuta()
/**
        La referencia en un RET jamás va a estar dentro de un bucle. De hecho,
        sólo hay un RET o como mucho, dos, y distintos, por lo que
        se calcula la referencia sin pretender optimizar.
*/
{
	if ( !( getMnemo()->getReferencia()->getNombre().empty() )
	 &&  NombreRegistro::cnvtNombreACodigoRegistro(
                                getMnemo()->getReferencia()->getNombre() )
	     != NombreRegistro::__RR
	   )
	{
		Referencia *ref = resuelveRefORegEnLoc(
                                          getMnemo()->getReferencia()
                );

		// Establecer el retorno del método
		met->ponRetorno( ref->getObjeto() );
	}
}

// ================================== OcSET ====================================
// ---------------------------------------------------------------- OcSET::OcSET
OcSET::OcSET(NMSet  *mn , Metodo *m) : Opcode(m), reg(NULL), ref(NULL)
{
    mnemo = new(std::nothrow) NMSet( *mn );

	if ( mnemo == NULL ) {
		throw ENoHayMemoria( "creando opcode SET" );
	}
}

// ------------------------------------------------------ OcSET::deSincroniza()
void OcSET::deSincroniza()
{
    if ( estaEnFase() ) {
        reg = ref = NULL;
        Opcode::deSincroniza();
    }
}

// --------------------------------------------------------------- OcSET::~OcSET
OcSET::~OcSET()
{
}

// -------------------------------------------------------------- OcSET::copia()
OcSET * OcSET::copia(Metodo *met)
{
	OcSET * toret = new(std::nothrow) OcSET( getMnemo(), met );

	if ( toret == NULL ) {
        	throw ENoHayMemoria( "duplicando opcode SET" );
	}

	return toret;
}

// --------------------------------------------------- OcSET::preparaReferencias
void OcSET::preparaReferencias()
{
        if ( ref == NULL
         || !estaEnFase() )
        {
               // Preparar las referencias
               reg = resuelveReg(getMnemo()->getRegistro());
               ref = resuelveRefORegEnRef(getMnemo()->getReferencia());

               // Colocar la fase
               enFase = met->getFase();
        }
}

// -------------------------------------------------------------- OcSET::ejecuta
void OcSET::ejecuta()
{
        // Crear la referencia
        preparaReferencias();

        // Localizar el método
        if (ref != NULL)
                if (reg != NULL)
						asignarReferencia( reg, ref );
                else    Runtime::ponExcepcionObjeto(getMnemo()->
                                                    getRegistro()->getNombre());
        else    Runtime::ponExcepcionObjeto(getMnemo()->getReferencia()
                                                                 ->getNombre());
}

// ================================== OcDEF ====================================
// ---------------------------------------------------------------- OcDEF::OcDEF
OcDEF::OcDEF(NMDef  *mn , Metodo *m) : Opcode(m)
{
        mnemo = new(std::nothrow) NMDef( *mn );

	if ( mnemo == NULL ) {
		throw ENoHayMemoria( "creando opcode STR" );
	}
}

OcDEF::~OcDEF()
{
}

// -------------------------------------------------------------- OcDEF::copia()
OcDEF * OcDEF::copia(Metodo *met)
{
	OcDEF * toret = new(std::nothrow) OcDEF( getMnemo(), met );

	if ( toret == NULL ) {
		throw ENoHayMemoria( "duplicando opcode DEF" );
	}

	return toret;
}

// -------------------------------------------------------------- OcDEF::ejecuta
void OcDEF::ejecuta()
/**
 * Se define una referencia local nueva para el método => Referencia.
 * Si es una referencia tipo '__lit__xxx' [xxx=número],
 * entonces es una referencia a un literal que no va a
 * cambiar nunca => ReferenciaConstante.
 * Si se compila con za, entonces está asegurado que nunca
 * llegará una referencia local con un nombre parecido.
*/
{
        const std::string &nombreLoc = getMnemo()->getNombre();

        if ( !met->vblesLocales.busca( nombreLoc ) )
        {
            // Crear la nueva referencia
	    Referencia * ref;

	    if (  nombreLoc.length() > LOC_LIT.length()
	     && ( nombreLoc.compare( 0, LOC_LIT.length(), LOC_LIT ) == 0) )
	    {
		    ref = new(std::nothrow) ReferenciaConstante (
				met->getPerteneceA(),
				met,
				getMnemo()->getNombre()
		    );

	    } else {
		    ref = new(std::nothrow) Referencia (
				met->getPerteneceA(),
				met,
				getMnemo()->getNombre(),
				Runtime::objetoNulo
        	    );
	    }

            if ( ref == NULL ) {
                    throw ENoHayMemoria( "ejecutando DEF" );
            }

            // Crear variable local con el nombre del mnemotécnico
            met->vblesLocales.inserta( nombreLoc, ref );

	} else {
		met->vblesLocales.busca( nombreLoc )->ponReferencia(
			Runtime::objetoNulo )
		;
	}

	return;
}

// ================================== OcASG ====================================
// ---------------------------------------------------------------- OcASG::OcASG
OcASG::OcASG(NMAsg  *mn , Metodo *m) : Opcode(m), ref(NULL)
{
        mnemo = new(std::nothrow) NMAsg( *mn );

	if ( mnemo == NULL ) {
		throw ENoHayMemoria( "creando opcode ASG" );
	}
}

OcASG::~OcASG()
{
}

// ------------------------------------------------------------- OcASG::copia()
OcASG * OcASG::copia(Metodo *met)
{
	OcASG * toret = new OcASG( getMnemo(), met );

	if ( toret == NULL ) {
		throw ENoHayMemoria( "duplicando opcode ASG" );
	}

	return toret;
}

// ------------------------------------------------ OcASG::preparaReferencias()
void OcASG::preparaReferencias()
{
        if (  ref == NULL
         ||  !estaEnFase() )
        {
               // Preparar la referencia
               ref = resuelveRefORegEnLoc( getMnemo()->getReferencia() );
               reg = resuelveReg( getMnemo()->getRegistro() );

	       if (ref != NULL)
	       {
			// CopyOnWrite check
			Objeto *othis = met->getLocThis()->getObjeto();

			if ( ( dynamic_cast<Atributo *>(ref) != NULL  )
			   &&  ref->getPerteneceAlObjeto() != othis )
			{
				Atributo *aux = new(std::nothrow) Atributo(
                                                        othis,
							ref->getNombre(),
							ref->getObjeto()
						)
				;

                                if ( aux != NULL ) {
				  if ( othis->lAtrs.inserta( aux->getNombre(), aux ) )
				  {
					// Si ha sido un éxito,
					// cambiamos la referencia
					ref = aux;
				  }
                                } else throw ENoHayMemoria( "ASG::prepara" );
			}
		}

               // Colocar la fase
               enFase = met->getFase();
        }
}

// -------------------------------------------------------------- OcASG::ejecuta
void OcASG::ejecuta()
{
        // Buscar el nombre de la variable y el objeto al que apunta el acc
        preparaReferencias();

        // Cambiarlo si se ha encontrado
        if ( ref != NULL )
                if ( reg != NULL )
			asignarReferencia( ref, reg );
                else Runtime::ponExcepcionObjeto(getMnemo()->getRegistro()
                                                                 ->getNombre());
        else Runtime::ponExcepcionObjeto(getMnemo()->getReferencia()
                                                                 ->getNombre());
}

// ================================== OcJMP ===================================
// --------------------------------------------------------------- OcJMP::OcJMP
OcFlw::OcFlw(Metodo *m) : Opcode(m), dirDirecta(NULL)
{
}

OcFlw::~OcFlw()
{
}

// ----------------------------------------------------- OcFlw::cambiaIPMetodo
inline
void OcFlw::asignaIPMetodo(void * dir)
/**
        Cambia el indicador de opcode en ejecución de un método.
        Este es el punto de unión con Método, sobre el que Opcode
        tiene privilegios de acceso.
*/
{
        met->asignaIP( (Metodo::IterMet) dir );
}

// ================================== OcJMP ===================================
// --------------------------------------------------------------- OcJMP::OcJMP
OcJMP::OcJMP(NMJmp  *mn , Metodo *m) : OcFlw(m)
{
        mnemo = new(std::nothrow) NMJmp(  *mn  );

	if ( mnemo == NULL ) {
		throw ENoHayMemoria( "creando opcode JMP" );
	}
}

OcJMP::~OcJMP()
{
}

// -------------------------------------------------------------- OcJMP::copia()
OcJMP * OcJMP::copia(Metodo *metAjeno)
{
	OcJMP * toret = new(std::nothrow) OcJMP( getMnemo(), metAjeno );

	if ( toret == NULL ) {
        	throw ENoHayMemoria( "duplicando opcode JMP" );
	}

        // Ajustar el salto
        toret->ponDirDirecta(
                metAjeno->getInstrucciones()
             + ( ((Metodo::IterMet) dirDirecta) - met->getInstrucciones() )
        );

	return toret;
}

// ------------------------------------------------------------- OcJMP::ejecuta
void OcJMP::ejecuta()
/*
        dirDirecta es NULL cuando se crea el objeto derivado de OcFlw
        Cuando el MÉTODO hace el chequeo de opcodes, chk(),
        pone todos los saltos A LA DIRECCIÓN DE MEMORIA DIRECTA,
        de manera que no sea necesario calcular nada, o mejor dicho,
        se calcula sólo una vez, en chk().
        Ésto es aplicable a JOF::ejecuta() y JOT::ejecuta()
*/
{
        asignaIPMetodo( dirDirecta );
}

// ================================== OcJOT ====================================
// ---------------------------------------------------------------- OcJOT::OcJOT
OcJOT::OcJOT(NMJot  *mn , Metodo *m) : OcFlw(m)
{
        mnemo = new(std::nothrow) NMJot(  *mn  );

	if ( mnemo == NULL ) {
		throw ENoHayMemoria( "creando opcode JOT" );
	}
}

OcJOT::~OcJOT()
{
}

// -------------------------------------------------------------- OcJOT::copia()
OcJOT * OcJOT::copia(Metodo *metAjeno)
{
	OcJOT * toret = new(std::nothrow) OcJOT( getMnemo(), metAjeno );

	if ( toret == NULL ) {
		throw ENoHayMemoria( "duplicando opcode JOT" );
	}

        // Ajustar el salto
        toret->ponDirDirecta(
                metAjeno->getInstrucciones()
             + ( ((Metodo::IterMet) dirDirecta) - met->getInstrucciones() )
        );

	return toret;
}

// -------------------------------------------------------------- OcJOT::ejecuta
void OcJOT::ejecuta()
{
        // Salta si verdad: Cámbiese el IP sii lo que hay en el ACC es True
        if ( met->getLocAcc()->getObjeto() == Runtime::objetoTrue )
        {
                asignaIPMetodo( dirDirecta );
        }
        else
        if ( met->getLocAcc()->getObjeto() != Runtime::objetoFalse )
        {
                Runtime::ponExcepcion( Runtime::objetoExcepcion, EXC_NOCOND );
        }
}

// ================================== OcJOF ====================================
// ---------------------------------------------------------------- OcJOF::OcJOF
OcJOF::OcJOF(NMJof  *mn , Metodo *m) : OcFlw(m)
{
        mnemo = new(std::nothrow) NMJof(  *mn  );

	if ( mnemo == NULL ) {
		throw ENoHayMemoria( "creando opcode JOF" );
	}
}

OcJOF::~OcJOF()
{
}

// -------------------------------------------------------------- OcJOF::copia()
OcJOF * OcJOF::copia(Metodo *metAjeno)
{
	OcJOF * toret = new(std::nothrow) OcJOF( getMnemo(), metAjeno );

	if ( toret == NULL ) {
        	throw ENoHayMemoria( "duplicando opcode JOF" );
	}

        // Ajustar el salto
        toret->ponDirDirecta(
                metAjeno->getInstrucciones()
             + ( ((Metodo::IterMet) dirDirecta) - met->getInstrucciones() )
        );

	return toret;
}

// -------------------------------------------------------------- OcJOF::ejecuta
void OcJOF::ejecuta()
{
        // Salta si falso: Cámbiese el IP sii lo que hay en el ACC es False
        if ( met->getLocAcc()->getObjeto() == Runtime::objetoFalse )
        {
                asignaIPMetodo( dirDirecta );
        }
        else
        if ( met->getLocAcc()->getObjeto() != Runtime::objetoTrue )
        {
                Runtime::ponExcepcion( Runtime::objetoExcepcion, EXC_NOCOND );
        }
}

// ================================== OcFLT ====================================
// ---------------------------------------------------------------- OcFLT::OcFLT
OcFLT::OcFLT(NMFlt  *mn , Metodo *m) : Opcode(m)
{
        mnemo = new(std::nothrow) NMFlt(  *mn  );

	if ( mnemo == NULL ) {
		throw ENoHayMemoria( "creando opcode STR" );
	}
}

OcFLT::~OcFLT()
{
}

// -------------------------------------------------------------- OcFLT::copia()
OcFLT * OcFLT::copia(Metodo *met)
{
	OcFLT * toret = new(std::nothrow) OcFLT( getMnemo(), met );

	if ( toret == NULL ) {
        	throw ENoHayMemoria( "duplicando opcode FLT" );
	}

	return toret;
}

// -------------------------------------------------------------- OcFLT::ejecuta
void OcFLT::ejecuta()
{
        double x       = getMnemo()->getLiteral();
        Objeto * toret = Runtime::gestorNumerosFlotantes->nuevo( "", x );

        if ( toret != NULL ) {
             met->getLocAcc()->ponReferencia( toret );
        }
        else {
             met->getLocAcc()->ponReferencia( Runtime::objetoNulo );
             Runtime::ponExcepcion( Runtime::objetoExcepcion, EXC_NOMEM );
        }
}

// ================================== OcINT ====================================
// ---------------------------------------------------------------- OcINT::OcINT
OcINT::OcINT(NMInt  *mn , Metodo *m) : Opcode(m)
{
        mnemo = new(std::nothrow) NMInt(  *mn  );

	if ( mnemo == NULL ) {
		throw ENoHayMemoria( "creando opcode INT" );
	}
}

OcINT::~OcINT()
{
}

// -------------------------------------------------------------- OcINT::copia()
OcINT * OcINT::copia(Metodo *met)
{
	OcINT * toret = new OcINT( getMnemo(), met );

	if ( toret == NULL ) {
		throw ENoHayMemoria( "duplicando opcode INT" );
	}

	return toret;
}

// -------------------------------------------------------------- OcINT::ejecuta
void OcINT::ejecuta()
{
        INT32  x       = getMnemo()->getLiteral();
        Objeto * toret = Runtime::gestorNumerosEnteros->nuevo( "", x );

        if (toret != NULL) {
             met->getLocAcc()->ponReferencia(toret);
        }
        else {
             met->getLocAcc()->ponReferencia(Runtime::objetoNulo);
             Runtime::ponExcepcion(Runtime::objetoExcepcion, EXC_NOMEM);
        }
}

// ================================== OcTRW ====================================
// ---------------------------------------------------------------- OcTRW::OcTRW
OcTRW::OcTRW(NMTrw  *mn , Metodo *m) : Opcode(m)
{
    mnemo = new(std::nothrow) NMTrw( *mn );

	if ( mnemo == NULL ) {
		throw ENoHayMemoria( "creando opcode TRW" );
	}
}

OcTRW::~OcTRW()
{
}

// -------------------------------------------------------------- OcTRW::copia()
OcTRW * OcTRW::copia(Metodo *met)
{
	OcTRW * toret = new(std::nothrow) OcTRW( getMnemo(), met);

	if ( toret == NULL ) {
		throw ENoHayMemoria( "duplicando opcode TRW" );
	}

	return toret;
}

// -------------------------------------------------------------- OcTRW::ejecuta
void OcTRW::ejecuta()
{
        Objeto *obj1 = NULL;
        Objeto *obj2 = NULL;

        // El objeto que se lanza
        Referencia *ref = resuelveRefORegEnRef(getMnemo()->getReferencia());

        if (ref != NULL)
        {
                obj1 = ref->getObjeto();

                // El objeto que sirve de mensaje, si es que existe
                if ( getMnemo()->getReferenciaMensaje() != NULL )
                {
                        Referencia *ref2 =
                                resuelveRefORegEnRef(
                                         getMnemo()->getReferenciaMensaje()
				);

                        if ( ref2 != NULL ) {
                                obj2 = ref2->getObjeto();
                        }
                }
        }

        // Lanzar la excepción
        if ( obj1 != NULL )
        {
                Runtime::gestorExcepciones->ponExcepcion( obj1, obj2 );
        }
        else Runtime::ponExcepcionObjeto(getMnemo()->getReferencia()
                                                            ->getNombre());
}

// ================================== OcIOF ====================================
// ---------------------------------------------------------------- OcIOF::OcIOF
OcIOF::OcIOF(NMIof  *mn , Metodo *m) : Opcode(m)
{
        mnemo = new(std::nothrow) NMIof( *mn );

	if ( mnemo == NULL ) {
		throw ENoHayMemoria( "creando opcode STR" );
	}
}

OcIOF::~OcIOF()
{
}

// -------------------------------------------------------------- OcIOF::copia()
OcIOF * OcIOF::copia(Metodo *met)
{
	OcIOF * toret = new OcIOF( getMnemo(), met );

	if ( toret == NULL ) {
		throw ENoHayMemoria( "duplicando opcode IOF" );
	}

	return toret;
}

// --------------------------------------------------- OcIOF::preparaReferencias
void OcIOF::preparaReferencias()
{
        if (ref == NULL
         || reg == NULL
         || !estaEnFase())
        {
                reg = resuelveReg( getMnemo()->getRegistro() );
                ref = resuelveRefORegEnRef( getMnemo()->getReferencia() );
        }
}

// -------------------------------------------------------------- OcIOF::ejecuta
void OcIOF::ejecuta()
{
    Objeto * obj;

    // Buscar la referencia y el registro en el que se busca
    preparaReferencias();

    // ¿Encontrado?
    if ( ref == NULL ) {
        Runtime::ponExcepcionObjeto( getMnemo()->getReferencia()->getNombre() );
        goto fin;
    }

    if ( reg == NULL ) {
        Runtime::ponExcepcionObjeto( getMnemo()->getRegistro()->getNombre() );
        goto fin;
    }

    // Coger el objeto apuntado en el registro
    obj = reg->getObjeto();

    // Colocar 'False' o 'True' en el acumulador, según corresponda
    if ( obj != NULL ) {
        if ( obj->esDescendienteDe( ref->getObjeto() ) )
                met->getLocAcc()->ponReferencia( Runtime::objetoTrue );
        else    met->getLocAcc()->ponReferencia( Runtime::objetoFalse );
    }
    else met->getLocAcc()->ponReferencia( Runtime::objetoFalse );

    fin:
    return;
}

// =================================== NOP =====================================
// -------------------------------------------------------------- OcNOP::OcNOP()
OcNOP::OcNOP(NMNop *mn, Metodo *m) : Opcode(m)
{
        mnemo = new(std::nothrow) NMNop( *mn );

        if ( mnemo == NULL ) {
                throw ENoHayMemoria( "creando NOP" );
        }
}

} // namespace Zero

// ident.cpp
/*
	Control de listas de identificadores
*/

#include "ident.h"
#include "excep.h"

namespace Zero {

const std::string VERSION_XML     = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>";

// ================================================================= Literal ===
Literal::Literal(const std::string &n, const std::string &l)
        : atributo(n), lit(l)
{
        char tp = lit[0];

        // Quitar la marca
        lit.erase( 0, 1 );

        // De qué tipo es ?
        if ( tp == '+' ) {
                tipo = LitNumInt;
        }
        else
        if ( tp == '"' ) {
                tipo = LitStr;
        }
        else throw EInterno( "Literal: Tipo de literal inesperado." );

        // Bueno, a lo mejor es flotante
        if ( tipo == LitNumInt
          && lit.find( '.' ) != std::string::npos ) {
                tipo = LitNumFlt;
        }
}

// ----------------------------------------------- Literal::getLiteralConTipo()
std::string Literal::getLiteralConTipo()
{
	std::string toret;

	if ( tipo != LitStr )
			toret = getLiteral();
	else	toret = '"' + getLiteral() + '"';

	return toret;
}

// ======================================================== Identificadores ===
// ----------------------------------------- Identificadores::Identificadores()
Identificadores::Identificadores(const Identificadores &x)
{
        listaIds = x.listaIds;
        nombre   = x.nombre;
}

Identificadores::Identificadores(const std::string &n) : nombre(n)
{
        listaIds.clear();
}


// ----------------------------------------------- Identificadores::operator=()
Identificadores &Identificadores::operator=(const Identificadores &x)
{
        listaIds = x.listaIds;
        nombre   = x.nombre;

        return *this;
}

// --------------------------------------------------- Identificadores::busca()

bool Identificadores::busca(const std::string &x) const
{
        return ( (ListaIds::const_iterator) listaIds.find( x ) != listaIds.end() );
}

// ------------------------------------------------- Identificadores::inserta()
bool Identificadores::inserta(const std::string &x)
{
	bool toret = busca( x );

	if ( !toret ) {
        	listaIds.insert( x );
	}

	return !toret;
}

// --------------------------------------------------- Identificadores::getID()
std::string Identificadores::getID(size_t i) const
{
	ListaIds::const_iterator it = listaIds.begin();

	advance( it, i );

	return *( it );
}

// --------------------------------------------------- Identificadores::reset()
void Identificadores::reset()
{
	listaIds.clear();
}

// ==================================================== IdentificadoresVisibles
// ------------------------- IdentificadoresVisibles::IdentificadoresVisibles()
IdentificadoresVisibles::IdentificadoresVisibles(const std::string &n)
	: Identificadores( n )
{
}

// ------------------------------------------- IdentificadoresVisibles::busca()
bool IdentificadoresVisibles::busca(const std::string &x) const
{
        return ( Identificadores::busca( '0' + x )
              || Identificadores::busca( '1' + x ) );
}

// ----------------------------------------- IdentificadoresVisibles::inserta()
bool IdentificadoresVisibles::inserta(const std::string &x, bool visibilidad)
/**
	Acepta un nombre de id y su visibilidad,
	de manera que true es público y false privado.

	Se implementa con un caracter precediendo a la cadena,
		0 privado
		1 público
*/
{
	bool toret = busca( x );

	if ( !toret ) {
		Identificadores::inserta( ( visibilidad ? '1' : '0' ) + x );
	}

	return toret;
}

// ------------------------------------------- IdentificadoresVisibles::getID()
std::string IdentificadoresVisibles::getID(size_t i) const
{
	return std::string( Identificadores::getID( i ).c_str() + 1 );
}

// --------------------------------------- IdentificadoresVisibles::esVisible()
bool IdentificadoresVisibles::esVisible(size_t i) const
{
	return ( Identificadores::getID( i )[0] == '1' );
}

// =========================================================== IdsPorObjeto ===
// ----------------------------------------------- IdsPorObjeto::IdsPorObjeto()
IdsPorObjeto::IdsPorObjeto(const std::string &n, const std::string &p)
	: nombre(n), padre(p)
{
        try {
                nombresDeAtributos = new IdentificadoresVisibles( "" );
                nombresDeMetodos   = new IdentificadoresVisibles( "" );
        } catch(...) {
                throw ENoHayMemoria( "creando idsPorObjeto" );
        }

	insertaAtributo( ATR_PARENT, true );
}

IdsPorObjeto::~IdsPorObjeto()
{
	Metodos::iterator it;

        delete nombresDeAtributos;
        delete nombresDeMetodos;

	for(it = metodos.begin(); it != metodos.end(); ++it ) {
		delete it->second;
	}
}

IdsPorObjeto::IdsPorObjeto(const IdsPorObjeto &x)
{
        nombre  = x.nombre;
        metodos = x.metodos;

        delete nombresDeAtributos;
        delete nombresDeMetodos;

        try {
            nombresDeAtributos =
                    new IdentificadoresVisibles( *( x.nombresDeAtributos ) )
            ;

            nombresDeMetodos =
                    new IdentificadoresVisibles( *( x.nombresDeMetodos ) )
            ;
        } catch(...) {
                throw ENoHayMemoria( "creando idsPorObjetos" );
        }
}

// ---------------------------------------------- IdsPorObjeto::buscaAtributo()
bool IdsPorObjeto::buscaAtributo(const std::string &x) const
{
        return nombresDeAtributos->busca( x );
}

// ------------------------------------------------ IdsPorObjeto::buscaMetodo()
bool IdsPorObjeto::buscaMetodo(const std::string &x) const
{
        return nombresDeMetodos->busca( x );
}

// ----------------------------------------------- IdsPorObjeto::getIdsMetodo()
Identificadores * IdsPorObjeto::getIdsMetodo(const std::string &x) const
{
	Identificadores * toret = NULL;

	Metodos::const_iterator it = metodos.find( x );

	if ( it != metodos.end() ) {
		toret = it->second;
	}

	return toret;
}

// -------------------------------------------- IdsPorObjeto::buscaVbleMetodo()
bool IdsPorObjeto::buscaVbleMetodo(const std::string &x, const std::string &y) const
{
        bool toret = false;

	Metodos::const_iterator it = metodos.find( x );

        if( it != metodos.end() ) {
		toret = it->second->busca( y );
        }

        return toret;
}

// -------------------------------------------- IdsPorObjeto::insertaAtributo()
bool IdsPorObjeto::insertaAtributo(const std::string &x, bool visibilidad)
{
        bool toret = false;

        if ( !buscaAtributo( x ) )
        {
                nombresDeAtributos->inserta( x, visibilidad );
                toret = true;
        }

        return toret;
}

// ---------------------------------------------- IdsPorObjeto::insertaMetodo()
bool IdsPorObjeto::insertaMetodo(const std::string &x,
				 bool visible,
                                 const MixinConArgumentos::Argumentos &args)
{
        Identificadores *ids;
        bool toret = false;

        if ( !buscaMetodo( x ) )
        {
            ids = new(std::nothrow) Identificadores( x );

            if ( ids != NULL )
            {
                nombresDeMetodos->inserta( x, visible );
                metodos.insert(
                    Metodos::value_type( x, ids )
                );

                // Meter las refs. locales (primero los argumentos)
                for(register unsigned int n = 0; n < args.size(); ++n)
                {
                    ids->inserta( args[n]->getNombre() );
                }

                toret = true;
            } else throw ENoHayMemoria( std::string( "creando ids metodo " + x ).c_str() );
        }

        return toret;
}

// ------------------------------------------ IdsPorObjeto::insertaVbleMetodo()
bool IdsPorObjeto::insertaVbleMetodo(const std::string &metodo, const std::string &vble)
{
        bool toret = false;

        if ( !buscaVbleMetodo( metodo, vble ) )
        {
	    Metodos::iterator it = metodos.find( metodo );

            if( it != metodos.end() )
            {
		toret =	it->second->inserta( vble );
	    }
	}

        return toret;
}

// ------------------------------------------------------ IdsPorObjeto::toXML()
std::string IdsPorObjeto::toXML() const
{
	Metodos::const_iterator it;

    std::string toret = "\n<Object name=\"";
    toret.append( getNombre() );
    toret.append( "\" parent=\"" );
    toret.append( getNombrePadre() );
    toret.append( "\">\n" );

    // Poner los atributos
    for(register unsigned int n = 0; n < nombresDeAtributos->getNumero(); ++n)
    {
            toret.append( "<Attribute name=\"" );
            toret.append( nombresDeAtributos->getID( n ) );
            toret.append( "\" visible=\"" );
            toret.push_back( ( nombresDeAtributos->esVisible( n ) ? '1' : '0' ) );
            toret.append( "\"/>\n" );
    }

    // Poner los métodos
    for(register unsigned int n = 0; n < nombresDeMetodos->getNumero(); ++n)
    {
            toret.append( "<Method name=\"" );
            toret.append( nombresDeMetodos->getID( n ) );
            toret.append( "\" visible=\"" );
            toret.append( ( nombresDeMetodos->esVisible( n ) ? "1" : "0" ) );
            toret.append( "\">\n" );

            // Poner las variables locales del método
            it = metodos.find( nombresDeMetodos->getID( n ) );

            if ( it != metodos.end() ) {
                for(unsigned int i = 0; i < it->second->getNumero(); ++i) {
                    toret.append( "<LocalReference name=\"" );
                    toret.append( it->second->getID( i ) );
                    toret.append( "\"/>\n" );
                }
            } else throw EInterno( "Mth no encontrado (?)" );

            toret.append( "</Method>\n" );
    }

    toret.append( "</Object>\n" );

    return toret;
}

// ------------------------------------------------ IdsPorObjeto::getAtributo()
std::string IdsPorObjeto::getAtributo(size_t i)
{
	return nombresDeAtributos->getID( i );
}

// ======================================================== IdsPorPrograma ====
// ------------------------------------------- IdsPorPrograma::IdsPorPrograma()
IdsPorPrograma::IdsPorPrograma()
{
}

IdsPorPrograma::~IdsPorPrograma()
{
	ListaIdsPorObj::iterator it;

        for(it = vIdsObj.begin(); it != vIdsObj.end(); ++it)
        {
                delete it->second;
        }
}

// ---------------------------------------------- IdsPorPrograma::buscaObjeto()
IdsPorObjeto *IdsPorPrograma::buscaObjeto(const std::string &x) const
{
	ListaIdsPorObj::const_iterator it = vIdsObj.find( x );
        IdsPorObjeto * toret = NULL;

	if ( it != vIdsObj.end() ) {
		toret = it->second;
	}

        return toret;
}

// -------------------------------------------- IdsPorPrograma::insertaObjeto()
bool IdsPorPrograma::insertaObjeto(const std::string &x, const std::string &y)
{
        bool toret = false;

        if ( buscaObjeto( x ) == NULL )
        {
                IdsPorObjeto * aux = new(std::nothrow) IdsPorObjeto(x, y);

                if ( aux != NULL ) {
                        vIdsObj.insert(
				ListaIdsPorObj::value_type( x, aux )
			);
                }
                else throw ENoHayMemoria( std::string( "creando ids por objeto " + x ).c_str() );

                toret = true;
        }

        return toret;
}

// --------------------------------------------------- IdsPorPrograma::toXML())
std::string IdsPorPrograma::toXML() const
{
        ListaIdsPorObj::const_iterator it;

        std::string toret = VERSION_XML;
        toret.push_back( '\n' );
        toret.push_back( '\n' );
        toret.append( "\n<Identifiers>\n" );

        for(it = vIdsObj.begin(); it != vIdsObj.end(); ++it)
        {
                toret.append( it->second->toXML() );
        }

        toret.append( "\n</Identifiers>\n" );

        return toret;
}

} // namespace Zero

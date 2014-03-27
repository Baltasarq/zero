// props.cpp
/*
	Gestor de propiedades.
	Las propiedades las tienen los objetos y los métodos.
*/

#include "props.h"
#include "objeto.h"
#include "runtime.h"
#include "servnom.h"

namespace Zero {

// ------------------------------------- GestorPropiedades::GestorPropiedades()
GestorPropiedades::GestorPropiedades(const std::string &n)
	: nomHost(n), props(NULL)
{
}

GestorPropiedades::~GestorPropiedades()
{
	// Eliminar las propiedades (si existen)
	if ( props != NULL ) {
		props->decrementaReferencias();
		props = NULL;
	}
}

// --------------------------------------------- GestorPropiedades::ponNombre()
void GestorPropiedades::ponNombre(const std::string &n) throw (EInterno)
{
	if ( nomHost.empty() ) {
		nomHost = n;
	}
	else throw EInterno( "Imposible renombrar el gestor de propiedades" );
}

// --------------------------------------------- GestorPropiedades::getNombre()
const std::string &GestorPropiedades::getNombre() const
{
	if ( nomHost.empty() ) {
		ServidorDeNombres::getSigNombreUnico();
	}

	return nomHost;
}

// ---------------------------------------- GestorPropiedades::masPropiedades()
/**
        Se encarga de guardar las propiedades que serán introducidas
        al preparar el objeto para ser ejecutado.
*/
void GestorPropiedades::masPropiedades(NMMta *mta)
{
	NMMta * nuevo = new(std::nothrow) NMMta( *mta );

	if ( nuevo == NULL ) {
		throw ENoHayMemoria( std::string( "Creando propiedad '"
				+ mta->listar()
				+ "' en objeto '"
				+ getNombre() + '\''
				   ).c_str() );
	}

	vPropsIniciales.push_back( nuevo );
}

// -------------------------------------------- GestorPropiedades::tieneProps()
bool GestorPropiedades::tieneProps() const
/**
 * Retorna verdadero o falso según si el objeto tiene propiedades o no
 * El objeto tendrá propiedades si a) están por inicializar o b) ya existen
 * @return true si el objeto tiene propiedades, false en otro caso
 */
{
	return ( ( vPropsIniciales.size() > 0 ) || ( props != NULL ) );
}

// ---------------------------------------------- GestorPropiedades::getProps()
Objeto * GestorPropiedades::getProps()
/**
 * Retorna el atributo props. Sólo cuando se realiza una llamada a esta función
 * se preparan todas las propiedades del objeto
 * @return atributo props
 */
{
	if ( props == NULL ) {
		if ( Runtime::rt()->estaEnInicializacion() ) {
			throw EInterno( "propiedades no disponibles en inic." );
		}

		// No existe, hay que hacerlo
		prepararProps();
	}

	return props;
}

// ------------------------------------------ GestorPropiedades::preparaProps()
/**
 * prepararProps()
 * Prepara las propiedades de un objeto. Al terminar este método, el atributo
 * props está apuntando a un vector de Zero con los punteros a las propiedades.
 */
void GestorPropiedades::prepararProps()
{
	register unsigned int n;
	Atributo *atrInfo;
	Objeto *objInfo;
	AlmacenObjetos *cDest = Runtime::rt()->getContainerEjecucion();
	Objeto *objProp;

        // Copiar un vector vacío
	props = Runtime::gestorVectores->nuevo( "__" + getNombre() + "Props__" );

	if ( props != NULL ) {
		props->asignarA( cDest );
		props->incrementaReferencias();
	} else throw ENoHayMemoria( "Creando objeto de propiedades" );

        // Añadir las propiedades iniciales
	LiteralVector_Zero * vReal = Runtime::gestorVectores->busca( props );
	if ( vReal == NULL ) {
		throw ENoHayMemoria( std::string( "Creando propiedades de: " + getNombre() ).c_str() );
	}

	for(n = 0; n < vPropsIniciales.size(); ++n) {
                // Localizar el objeto en cuestión
		objProp = Runtime::rt()->getContainerIntStdLib()->busca(
				vPropsIniciales[n]->getIdObj()
		);

		if    ( objProp == NULL
		  || !( objProp->esDescendienteDe( Runtime::objetoPropiedades ) ) )
		{
			Runtime::ponExcepcionObjeto(
					vPropsIniciales[n]->getIdObj()
						   );
			break;
		}

		// Crear el objeto a meter
		objProp = objProp->copiaSistema(
				getNombre() + objProp->getNombre()
		);

		// Darle la info
		if ( !( vPropsIniciales[n]->getDatos().empty() ) )
		{
			objInfo = Runtime::gestorCadenas->nuevo(
					"",
					vPropsIniciales[n]->getDatos()
			);

			atrInfo = new(std::nothrow) Atributo(
					objProp,
					"info",
					objInfo,
					Atributo::PUBLICO
			);

			if ( objInfo != NULL
			  && atrInfo != NULL )
			{
				objProp->lAtrs.inserta(
					atrInfo->getNombre(),
					atrInfo
				);
			} else throw ENoHayMemoria(
						"creando atributo info en props"
						  );
		}

		vReal->mas( objProp );
	}

        // Eliminar las propiedades iniciales
	for(n = 0; n < vPropsIniciales.size(); ++n) {
		delete vPropsIniciales[n];
	}

	vPropsIniciales.clear();
}

} // namespace Zero

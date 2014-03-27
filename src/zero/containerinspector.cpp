// Containerinspector.cpp
/*
        Lleva todo lo relacionado al inspector de contenedores
*/

#include "containerinspector.h"

// ================================================ ContainerInspector::Miembro
ContainerInspector::TipoObjeto
ContainerInspector::Miembro::getTipoObjeto(Objeto *obj)
{
        TipoObjeto toret     = imposible;
        AlmacenObjetos * lib = Runtime::rt()->getContainerIntStdLib();
        Objeto * app         = lib->busca( "Application" );
        Objeto * appConsola  = lib->busca( "ConsoleApplication" );
        Objeto * appVisual   = lib->busca( "VisualApplication" );

        if ( obj != NULL ) {
                toret = objeto;

                while ( obj != NULL
                     && obj != Runtime::objetoRaiz )
                {
                        if ( dynamic_cast<AlmacenObjetos *>( obj ) != NULL ) {
                                toret = container;
                                break;
                        }
                        if ( obj == app ) {
                                toret = aplicacion;
                                break;
                        }
                        else
                        if ( obj == appConsola ) {
                                toret = aplicacion_consola;
                                break;
                        }
                        else
                        if ( obj == appVisual ) {
                                toret = aplicacion_visual;
                                break;
                        }

                        if ( obj->getAtrObjetoPadre() != NULL )
                                obj = obj->getAtrObjetoPadre()->getObjeto();
                        else    obj = NULL;
                }
        }

        return toret;
}

// ========================================================= ContainerInspector
const string ContainerInspector::strTipoObjeto[] = {
        "Error",
        "Objeto",
        "Contenedor",
        "Aplicación",
        "Aplicación de consola",
        "Aplicación visual"
};

// ----------------------------------- ContainerInspector::ContainerInspector()
ContainerInspector::ContainerInspector()
        : contActual(NULL)
{
}

// -------------------------------------- ContainerInspector::cambiarContainer()
AlmacenObjetos * ContainerInspector::cambiarContainer(AlmacenObjetos *c)
{
        if ( contActual != c ) {
            // Dejar de observar el container actual
            if ( contActual != NULL ) {
                    dejarDeObservar( contActual );
            }

            // Pasar al actual
            observar( c );
            contActual = c;
            actualiza();
        }

        return c;
}

// ------------------------------------- ContainerInspector::cambiarContainer()
AlmacenObjetos * ContainerInspector::cambiarContainer(const string &ruta)
{
        AlmacenObjetos * cont;
        Atributo ref( Runtime::objetoRaiz, "temporal", ruta);

        if ( dynamic_cast<AlmacenObjetos *>( ref.getObjeto() ) == NULL )
                cont = ref.getObjeto()->getPerteneceA();
        else    cont = (AlmacenObjetos *) ref.getObjeto();

        return cambiarContainer( cont );
}

// -------------------------------------------- ContainerInspector::actualiza()
void ContainerInspector::actualiza()
{
        if ( contActual == NULL ) {
                cambiarContainer( Runtime::rt()->getContainerEjecucion() );
        }
        else {
            // Borrar la lista de miembros
            miembros.clear();

            // Buscar los miembros
            for(unsigned int i = 0; i < contActual->lAtrs.getNumero(); ++i)
            {
                    miembros.push_back(
                            Miembro( contActual->lAtrs.getElementoNumero( i )
                                            ->getObjeto(),
                                     contActual->lAtrs.getNombreElementoNumero( i ) )
                    );
            }
        }

        actualizaUIInspectorObjetos();
}

// -----------------------------------------------------------------------------
void ContainerInspector::actualizar(Observable *obj, TipoCambio t)
{
        if (   t == Observador::BorradoElemento
          && obj == contActual )
        {
                cambiarContainer( Runtime::rt()->getContainerEjecucion() );
        }
        else
        if ( t == Observador::CambioEnElemento
          && obj == contActual )
        {
                actualiza();
        }
}




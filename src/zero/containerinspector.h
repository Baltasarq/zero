// containerinspector.h
/*
        Lleva todo lo relacionado al inspector de contenedores
*/

#ifndef CONTAINER_INSPECTOR_H
#define CONTAINER_INSPECTOR_H

#include "container.h"
#include "runtime.h"

class ContainerInspector : public Observador {
public:
        enum TipoObjeto { imposible,
                       objeto,
                       container,
                       aplicacion,
                       aplicacion_consola,
                       aplicacion_visual }
        ;

        static const std::string strTipoObjeto[];

        class Miembro {
        private:
                Objeto * obj;
                AlmacenObjetos * cont;
                TipoObjeto tipo;
                std::string nombreRef;

        public:
                Miembro(Objeto *o, const std::string &n)
                        : obj(o), tipo(imposible), nombreRef(n)
                {
                        if ( obj != NULL ) {
                                cont = obj->getPerteneceA();
                                tipo = getTipoObjeto( obj );
                        }
                }

                Objeto * getObjeto()       const { return obj; }
                const std::string &getNombre()  const { return obj->getNombre(); }
                const std::string &getNombreRef()  const { return nombreRef; }
                AlmacenObjetos *getPerteneceA() const { return cont; }
                TipoObjeto getTipoObjeto() const { return tipo; }

                std::string toString() const {
                        return ( getNombre() + ':' + ' ' + getStrTipoObjeto() );
                }

                std::string getStrTipoObjeto() const {
                        return strTipoObjeto[tipo];
                };

                static TipoObjeto getTipoObjeto(Objeto *);
        };

        typedef vector<Miembro> MiembrosContenedor;
private:
        MiembrosContenedor miembros;
        AlmacenObjetos *contActual;
public:
        ContainerInspector();

        AlmacenObjetos * cambiarContainer(const std::string &);
        AlmacenObjetos * cambiarContainer(AlmacenObjetos *);

        void actualiza();
        void actualizar(Observable *, TipoCambio);

        const MiembrosContenedor &getMiembros() const { return miembros; }
        AlmacenObjetos * getContainerActual() const { return contActual; }
};

extern void actualizaUIInspectorObjetos();

#endif

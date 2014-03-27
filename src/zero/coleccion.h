// coleccion.h
/*
        Interfaz de las clases que permiten guardar colecciones de datos
*/

#include <string>
#include <vector>

#include "servnom.h"

#ifndef COLECCION_H
#define COLECCION_H

namespace Zero {

// ------------------------------------------------------------- ParNombreObjeto
template <class T>
class ParNombreObjeto {
protected:
        std::string nombre;
        T * dir;
public:
        ParNombreObjeto(const std::string &n, T *d) :
                nombre(n), dir(d) {}

        virtual T * getObjeto()
            { return dir; }
        virtual const std::string & getNombre() const
            { return nombre; }

        virtual ~ParNombreObjeto() {};
};

// ----------------------------------------------------------- AsocNombrePuntero
template <class T>
class AsocNombrePuntero {
/*
        Esta clase es una plantilla que permite guardar asociaciones entre
        nombres y objetos.
        Este contenedor mete la dirección del objeto, y no lo copia.
        Sin embargo, cuando este contenedor se elimina, sí se borra.
*/
  public:
        static const bool PROPIETARIO = true;
        static const bool NO_PROPIETARIO = false;
        static const std::string NULO;
        typedef std::vector<ParNombreObjeto<T> *> vectorPtrObj;
  private:
        bool esPropietario;
        vectorPtrObj indicePorPuntero;
        vectorPtrObj indicePorNombre;

        static bool qckCmpEqStr(const std::string &, const std::string &);
        static bool qckCmpLtStr(const std::string &, const std::string &);
        static bool qckCmpGtStr(const std::string &, const std::string &);

        size_t buscaPorNombre(const std::string &) const;
        size_t buscaPorPuntero(T *) const;
  public:
        AsocNombrePuntero(bool prop = PROPIETARIO)
                : esPropietario(prop)
        {};

        ~AsocNombrePuntero()
                { this->eliminaTodos(); }

        void eliminaTodos();

        size_t buscaIndicePorNombre(const std::string &x) const
            { return buscaPorNombre( x ); }
        size_t buscaIndicePorPuntero(T *x) const
            { return buscaPorPuntero( x ); }

        bool inserta(T * x)
                { return inserta(ServidorDeNombres::getSigNombreUnico(), x); }
        bool inserta(const std::string &, T *);
        bool borra(const std::string &);
		bool borra(T *);

        T *busca(T *x) const
            {
                 size_t ind = buscaPorPuntero( x );

                 if ( ind < indicePorPuntero.size() ) {
                    return indicePorPuntero[ind]->getObjeto();
                 } else {
                    return NULL;
                 }
            }


        T *busca(const std::string &x) const
            {
                  size_t ind = buscaPorNombre( x );

                  if ( ind < indicePorNombre.size() ) {
                    return indicePorNombre[ind]->getObjeto();
                  } else {
                    return NULL;
                  }
            }

        size_t getNumero() const
                { return indicePorNombre.size(); }

        T * getElementoNumero(size_t i) const
                {
                  if ( i < indicePorNombre.size() ) {
                    return indicePorNombre[ i ]->getObjeto();
                  } else {
                    return NULL;
                  }
                }

        const std::string &getNombreElementoNumero(size_t i) const
                {
                   if ( i < indicePorNombre.size() ) {
                       return indicePorNombre[ i ]->getNombre();
                   } else {
                       return NULO;
                   }
                }
};

#include "coleccion.cpp"

}

#endif

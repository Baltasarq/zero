// coleccion.cpp
/*
        Implantación de las clases que dan soporte a las colecciones de objetos

        Pertenece a zvm

        jbgarcia@uvigo.es
*/
// =========================================================== AsocNombrePuntero
template <class T>
const std::string AsocNombrePuntero<T>::NULO = "";

// --------------------------------------------- AsocNombrePuntero::eliminaTodos
template <class T>
void AsocNombrePuntero<T>::eliminaTodos()
{
        // Borrar los datos
         while( !indicePorPuntero.empty() )
         {
                 borra( indicePorNombre[0]->getObjeto() );
         }

}

// -------------------------------------------------- AsocNombrePuntero::inserta
template<class T>
bool AsocNombrePuntero<T>::inserta(const std::string &n, T *x)
{
        bool toret = false;
        ParNombreObjeto<T> *dirLocal;
        typename vectorPtrObj::iterator it;

        // Es válido ? Ya existe ?
        if (x         == NULL
	 || busca(n)  != NULL)
        {
                goto FINAL;
        }

        // Crear el nodo
        dirLocal = new ParNombreObjeto<T>(n, const_cast<T *>(x));
        if (dirLocal == NULL)
                goto FINAL;

        // Meter en el índice de nombres
        it = indicePorNombre.begin();
        while(it != indicePorNombre.end()
          &&  (*it)->getNombre() < n)
        {
                ++it;
        }
        indicePorNombre.insert(it, dirLocal);

        // Meter en el índice de punteros
        it = indicePorPuntero.begin();
        while( it != indicePorPuntero.end()
           && (*it)->getObjeto() < x )
        {
                ++it;
        }
        indicePorPuntero.insert(it, dirLocal);

        toret = true;

        // Final
        FINAL:
        return toret;
}

// ------------------------------------------------- AsocNombrePuntero::borra()
template<class T>
bool AsocNombrePuntero<T>::borra(T *x)
{
	size_t indNom;
	size_t indPtr = buscaPorPuntero( x );
	ParNombreObjeto<T> * objetivo = NULL;
	bool toret                    = false;

	if ( indPtr < indicePorNombre.size() )
	{
		objetivo = indicePorPuntero[indPtr];
		indNom   = buscaPorNombre( objetivo->getNombre() );

                // Borrar el soporte y el elemento en sí
                if ( esPropietario ) {
		    delete objetivo->getObjeto();
                }
		delete objetivo;
		toret = true;

                // Borrar las posiciones en los índices
		indicePorNombre.erase( indicePorNombre.begin() + indNom );
		indicePorPuntero.erase( indicePorPuntero.begin() + indPtr );
	}

	return toret;
}

template<class T>
bool AsocNombrePuntero<T>::borra(const std::string &x)
{
	size_t indPtr;
	size_t           indNom = buscaPorNombre( x );
    ParNombreObjeto<T> * objetivo = NULL;
    bool toret                    = false;

	if ( indNom < indicePorNombre.size() )
        {
        	objetivo = indicePorNombre[indNom];
                indPtr   = buscaPorPuntero( objetivo->getObjeto() );

                // Borrar el soporte y el elemento en sí
                if ( esPropietario ) {
                    delete objetivo->getObjeto();
                }
                delete objetivo;
                toret = true;

                // Borrar las posiciones en los índices
                indicePorNombre.erase( indicePorNombre.begin() + indNom );
                indicePorPuntero.erase( indicePorPuntero.begin() + indPtr );
        }

        return toret;
}

// ---------------------------------------------------- AsocNombrePuntero::busca
template<class T> inline
bool AsocNombrePuntero<T>::qckCmpEqStr(const std::string & a, const std::string & b)
{
	if ( *( a.c_str() ) != *( b.c_str() )
	  || a.length() != b.length() )
	{
 		return false;
	}
	else return ( a.compare( b ) == 0 );

}

template<class T> inline
bool AsocNombrePuntero<T>::qckCmpLtStr(const std::string & a, const std::string & b)
{
	if ( *( a.c_str() ) > *( b.c_str() ) )
 		return false;
	else
	if ( *( a.c_str() ) < *( b.c_str() ) )
		return true;
	else	return ( a.compare( b ) < 0 );
}

template<class T> inline
bool AsocNombrePuntero<T>::qckCmpGtStr(const std::string & a, const std::string & b)
{
	if ( *( a.c_str() ) < ( *b.c_str() ) )
 		return false;
	else
	if ( *( a.c_str() ) > *( b.c_str() ) )
 		return true;
	else 	return( a.compare( b ) > 0 );

}

template<class T>
size_t AsocNombrePuntero<T>::buscaPorNombre(const std::string &x) const
/*
        Implanta una búsqueda binaria
*/
{
        size_t bajo  = 0;
        size_t alto  = indicePorNombre.size();
        size_t medio = alto / 2;

        while ( alto  > bajo )
        {
            if ( qckCmpGtStr( indicePorNombre[medio]->getNombre(), x ) ) {
                if ( medio > 0 )
                    alto = medio - 1;
                else	alto = 0;
            }
            else
            if ( qckCmpLtStr( indicePorNombre[medio]->getNombre(), x ) ) {
                 bajo = medio + 1;
            }
            else break;

            medio = ( alto + bajo ) / 2;
        }

        if ( medio < indicePorNombre.size()
          && qckCmpEqStr( indicePorNombre[ medio ]->getNombre(), x ) )
        {
            return medio;
        }

        return indicePorNombre.size();
}

template<class T>
size_t AsocNombrePuntero<T>::buscaPorPuntero(T *x) const
{
        size_t bajo  = 0;
        size_t alto  = indicePorPuntero.size();
        size_t medio = alto / 2;

        while ( alto > bajo )
        {
                if ( indicePorPuntero[ medio ]->getObjeto() > x )
                {
                    if ( medio > 0 ) {
                        alto = medio - 1;
                    } else {
                        alto = 0;
                    }
                }
                else
                if ( indicePorPuntero[ medio ]->getObjeto() < x )
                {
                        bajo = medio + 1;
                }
                else break;

                medio = ( alto + bajo ) / 2;
        }

        if ( medio < indicePorPuntero.size()
          && indicePorPuntero[medio]->getObjeto() == x )
        {
            return medio;
        }

        return indicePorPuntero.size();
}

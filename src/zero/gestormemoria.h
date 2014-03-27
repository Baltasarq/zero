// gestormemoria.h

#ifndef __GESTORMEMORIA__H__
#define __GESTORMEMORIA__H__

#include "excep.h"

template<typename T>
class GestorMemoria {
public:
	static const unsigned int MAXITEMSINIT = 100000;
private:
	typedef unsigned long int Indice;
	typedef Indice * RefIndice;
	
	Indice    maximoPorPagina;
	RefIndice *huecosDisponibles;
	RefIndice *techoHuecos;
	RefIndice *nuevo;
	
	T **espacio;
	T **techoEspacio;
	unsigned int numPaginaActual;
public:
	GestorMemoria(Indice = MAXITEMSINIT);
	T* buscaHuecosLibres();
	
	~GestorMemoria() {
		free( espacio ); espacio = NULL; 
	}
	
	T *crear() {
		if ( nuevo[numPaginaActual] < techoHuecos[numPaginaActual] ) {
		     return ( espacio[numPaginaActual] + ( *( nuevo[numPaginaActual]++ ) ) );
		}
		else {
		     T* toret = buscaHuecosLibres();
		     
		     if ( toret == NULL) {
		     	creaNuevaPagina(); 
		     	return espacio[numPaginaActual] + ( *( nuevo[numPaginaActual]++ ) );
		     } else return toret;
		}
	}
	
	void eliminar(T* x) {
 		if ( pertenece( numPaginaActual, x ) ) {
			*(--nuevo[numPaginaActual]) = x - espacio[numPaginaActual];
 		}
 		else {
			unsigned int numP;
			
			if ( buscaPagina( x, numP) ) {
				*(--nuevo[numP]) = x - espacio[numP];
			}
			else throw EMemPaginaErronea( "eliminando objeto" );
		}
	}
	
	bool buscaPagina(T*, unsigned int &);
	void creaNuevaPagina();
	
	Indice getNumHuecosDisponibles(unsigned int numP)
		{ return ( ( huecosDisponibles[numP] + maximoPorPagina ) 
		           - nuevo[numP] ); }
	unsigned int getMaxItemsPorPagina() const 
		{ return maximoPorPagina; }
	const void *getDirBase(unsigned int nump) const
		{ return espacio[nump]; }
	bool pertenece(unsigned int numP, T* x) const
		{ return ( ( x >= espacio[numP] ) && ( x < techoEspacio[numP] ) ); }
};

#include "gestormemoria.cpp"

#endif

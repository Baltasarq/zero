// servnom.cpp
/*
        Implantación del servicio de nombres únicos.

        Pertenece a zvm

        jbgarcia@uvigo.es
*/


#include "servnom.h"
#include "vmcap.h"

#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <limits>

// =========================================================== ServidorDeNombres
std::string ServidorDeNombres::nombreUnico = "";
unsigned long int ServidorDeNombres::numObjetos;
unsigned int ServidorDeNombres::netMacMark = std::numeric_limits<unsigned int>::max();

// ---------------------------------------- AsocNombrePuntero::getSigNombreUnico
const std::string &ServidorDeNombres::getSigNombreUnico()
{
    static char buf[64];
	char ch = 'A';
	const VMCap::MACId &macId = VMCap::getNetId();

	// Calcular marca de dir internet mac
	if ( netMacMark == std::numeric_limits<unsigned int>::max() ) {
        netMacMark = 0;
        for (unsigned int i = 0; i < VMCap::getNetIdLength(); ++i) {
            netMacMark += macId[ i ];
        }
        netMacMark %= 256;
	}

	// Asegurar una letra distinta
    if (!nombreUnico.empty()) {
          ch = nombreUnico[nombreUnico.size() - 1];

          if (ch == 'Z')
                ch = 'A';
          else  ++ch;
    } else std::srand( time( NULL ) );

	// Crear el siguiente nombre
	nombreUnico.erase();
	nombreUnico.push_back( '_' );

	// Número de objetos
	sprintf( buf, "%ld", numObjetos );
	nombreUnico.append( buf );
	nombreUnico.push_back( '_' );

    // Tiempo (en ticks CPU)
	sprintf( buf, "%ld", clock() );
	nombreUnico.append( buf );
	nombreUnico.push_back( '_' );

    // Tiempo (en segundos)
	sprintf( buf, "%ld", std::time( NULL ) );
	nombreUnico.append( buf );
	nombreUnico.push_back( '_' );

    // Aleatorio
	sprintf( buf, "%d", std::rand() );
	nombreUnico.append( buf );
	nombreUnico.push_back( '_' );

    // Marca de Internet MAC
    sprintf( buf, "%d", netMacMark );
    nombreUnico.append( buf );
	nombreUnico.push_back( '_' );

    // Chr de secuencia
    buf[ 0 ] = ch;
    buf[ 1 ] = 0;
    nombreUnico.append( buf );

	++numObjetos;
    return nombreUnico;
}

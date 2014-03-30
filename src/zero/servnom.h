//servnom.h
/*
        Proporciona nombres únicos.

        Pertenece a Zvm

        jbgarcia@uvigo.es
*/

#ifndef SERVNOM_H
#define SERVNOM_H

#include <string>

// ----------------------------------------------------------- servidorDeNombres
class ServidorDeNombres {
private:
	static unsigned long int numObjetos;
	static unsigned int netMacMark;
    static std::string nombreUnico;
public:
    static const std::string &getSigNombreUnico();
};

#endif


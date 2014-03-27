// uintf.h
/*
	Esta cabecera proporciona el puente entre la máquina virtual
	y el interfaz de usuario
*/

#ifndef UINTF_H_
#define UINTF_H_

#include <string>

class Objeto;
class Metodo;

// Funciones de captura y muestra de información
extern void  putLineaOutput(const std::string &, bool);
extern const std::string &getLineaInput(void);

// Funciones de control de pasos de la MV
#ifdef _UI_DEBUG
extern void iuComienzaMetodo(Objeto *, Metodo *);
extern void iuPasoMetodo(Objeto *, Metodo *, unsigned int paso);
extern void iuFinMetodo(Objeto *, Metodo *, unsigned int paso, Objeto *);
#endif

#endif

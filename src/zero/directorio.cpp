// directorio.cpp

#include "vmcap.h"
#include "directorio.h"
#include "excep.h"


namespace Zero {

char Directorio::barraDir = VMCap::getDirMark();

// --------------------------------------------------- Directorio::Directorio()
Directorio::Directorio(const std::string &n, bool modoApertura)
/**
 * Crea un directorio. Es obligatorio darle como parámetro una ruta completa
 * @param n nombre del directorio a representar
 * @param modoApertura indica si hay que crear el directorio o no
 * @return un objeto directorio, claro
 */
{
	std::string nombreDirectorio;

	// Asignar el nombre del directorio
	if ( n.empty() )
		nombreDirectorio = getNombreDirectorioActual();
	else	nombreDirectorio = n;

	// Buscar la ruta y nombre subdirectorio
	descomponer( nombreDirectorio );

	// Buscar el nombre y comprobar que existe
	if ( !VMCap::buscaEnDirectorio( nombreDir, ruta, VMCap::DIRECTORIO ) ) {
		if ( modoApertura == CREAR ) {
			if ( !crearDirectorio( ruta, nombreDir ) ) {
				throw ESeccPersNoExiste(
					std::string( "No pudo ser creado " + nombre ).c_str()
				);
			}
		}
		else throw ESeccPersNoExiste( nombre.c_str() );
	}

	VMCap::cambiarDirectorio( nombre.substr( 0, nombre.length() - 1) );
}

// -------------------------------------------------- Directorio::descomponer()
void Directorio::descomponer(const std::string &rutaDirectorio)
/**
 * Descompone un nombre de directorio en sus partes constituyentes
 * @param rutaDirectorio la ruta del directorio que se va a descomponer
 */
{
	// Buscar la ruta y nombre subdirectorio
	nombre    = rutaDirectorio;

	// Ruta y nombre
	if ( nombre[ nombre.length() - 1 ] == barraDir ) {
		// Si al final hay una barra, quitarla
		nombre.erase( nombre.length() - 1, 1 );
	}

	// Comprobar las posiciones de los separadores de directorio
	size_t posUltimaBarra = nombre.rfind( barraDir );

	if ( posUltimaBarra == std::string::npos ) {
		// No es una ruta completa
		throw ERutaInvalida( nombre.c_str() );
	}

	// Ahora sí, desgranar
	ruta      = nombre.substr( 0, posUltimaBarra );
	nombreDir = nombre.substr( posUltimaBarra + 1, nombre.length() );

	// Preparar el nombre para futuro uso
	nombre += barraDir;
}

// ------------------------------------------------ Directorio::borrarArchivo()
bool Directorio::borrarArchivo(const std::string &f)
/**
 * Borra un archivo en el directorio
 * @param f el archivo
 * @return true si se borrará, false en otro caso
 */
{
	return VMCap::borrarArchivo( getNombre() + f );
}

// ---------------------------------------------- Directorio::crearDirectorio()
bool Directorio::crearDirectorio(const std::string &d)
/**
 * Crea un directorio en el directorio
 * @param d el nombre del directorio
 * @return true si se pudo crear, false en otro caso
 */
{
	return VMCap::crearDirectorio( getNombre(), d );
}

// -------------------------------------------- Directorio::cambiarDirectorio()
bool Directorio::cambiarDirectorio(const std::string &d)
/**
 * Cambia a un directorio existente. Si la ruta contiene separadores, la deja tal cual
 * 	en caso contrario, le añade la ruta del dir actual
 	Si la ruta está mal creada, puede lanzar una excepción al descomponer
 * @param d el nombre del directorio
 * @return true si se pudo cambiar, false en otro caso
 */
{
	std::string dir = d;

	// Si es relativa, le concatenamos el directorio hasta aquí
	if ( dir.find( barraDir ) == std::string::npos ) {
		dir = getNombre() + dir;
	}

	bool toret = VMCap::cambiarDirectorio( dir );

	if ( toret ) {
		descomponer( dir );
	}

	return toret;
}

// ----------------------------------------------- Directorio::moverArchivoDe()
bool Directorio::moverArchivoDe(const std::string &f, const std::string &dir)
/**
 * Copia un archivo y borra el original.
 * @see moverArchivo
 * @param f el nombre del archivo a mover
 * @param dir el nombre del directorio (ruta completa) de origen
 * @return true si tiene éxito, false en otro caso
 */
{
	return moverArchivo( f, dir, getNombre() );
}

// ------------------------------------------------ Directorio::moverArchivoA()
bool Directorio::moverArchivoA(const std::string &f, const std::string &dir)
/**
 * Copia un archivo y borra el original.
 * @see moverArchivo
 * @param f el nombre del archivo a mover
 * @param dir el nombre del directorio (ruta completa) de destino
 * @return true si tiene éxito, false en otro caso
 */
{
	return moverArchivo( f, getNombre(), dir );
}

// ---------------------------------------------- Directorio::copiarArchivoDe()
bool Directorio::copiarArchivoDe(const std::string &f, const std::string &dir)
/**
 * Copia un archivo
 * @see moverArchivo
 * @param f el nombre del archivo a copiar
 * @param dir el nombre del directorio (ruta completa) de origen
 * @return true si tiene éxito, false en otro caso
 */
{
	return copiarArchivo( f, dir, getNombre() );
}

// ----------------------------------------------- Directorio::copiarArchivoA()
bool Directorio::copiarArchivoA(const std::string &f, const std::string &dir)
/**
 * Copia un archivo
 * @see moverArchivo
 * @param f el nombre del archivo a copiar
 * @param dir el nombre del directorio (ruta completa) de destino
 * @return true si tiene éxito, false en otro caso
 */
{
	return copiarArchivo( f, getNombre(), dir );
}

// --------------------------------------------- Directorio::existeDirectorio()
bool Directorio::existeDirectorio(const std::string &d)
/**
 * Comprueba si existe un subdirectorio en este directorio
 * @param d el directorio a comprobar si existe o no
 * @return true si existe, false en caso contrario
 */
{
	return VMCap::buscaEnDirectorio( d, getNombre(), VMCap::DIRECTORIO );
}

// ------------------------------------------------ Directorio::existeArchivo()
bool Directorio::existeArchivo(const std::string &f)
/**
 * Comprueba si existe un archivo en el directorio actual
 * @param f el archivo a buscar
 * @return true si existe en el directorio, false en otro caso
 */
{
	return VMCap::buscaEnDirectorio( f, getNombre(), VMCap::ARCHIVO );
}

// Funciones estáticas

// ------------------------------------ Directorio::getNombreDirectorioActual()
std::string Directorio::getNombreDirectorioActual()
/**
 * Esta función se utiliza para poder localizar el PS
 * @return el nombre del directorio actual
 */
{
	return VMCap::getNombreDirActual();
}

// ---------------------------------------------- Directorio::crearDirectorio()
bool Directorio::crearDirectorio(const std::string &dir, const std::string &nombreDir)
{
	return VMCap::crearDirectorio( dir, nombreDir );
}

// ------------------------------------------------- Directorio::moverArchivo()
bool Directorio::moverArchivo(const std::string & f,
                              const std::string & dorg,
			      const std::string & ddest)
/**
 *
 * @param f el archivo a mover
 * @param dorg el directorio de origen
 * @param ddest el directorio de destino
 * @return true si se pudo mover, false en otro caso.
 */
{
	std::string rutaOrg;
	std::string rutaDest;

	// Preparar la ruta  de origen
	if ( dorg[dorg.length() - 1] == barraDir )
		rutaOrg = dorg + f;
	else 	rutaOrg = dorg + barraDir + f;

	// Preparar la ruta de destino
	if ( ddest[ddest.length() - 1] == barraDir )
		rutaDest = ddest + f;
	else 	rutaDest = ddest + barraDir + f;

	return VMCap::moverArchivo( rutaOrg, rutaDest );
}

// ------------------------------------------------ Directorio::copiarArchivo()
bool Directorio::copiarArchivo(const std::string & f,
                              const std::string & dorg,
			      const std::string & ddest)
/**
 *
 * @param f el fichero a copiar
 * @param dorg el directorio de origen
 * @param ddest el directorio de destino
 * @return true si se ha copiado, false en otro caso
 */
{
	std::string rutaOrg;
	std::string rutaDest;

	// Preparar la ruta  de origen
	if ( dorg[dorg.length() - 1] == barraDir )
		rutaOrg = dorg + f;
	else 	rutaOrg = dorg + barraDir + f;

	// Preparar la ruta de destino
	if ( ddest[ddest.length() - 1] == barraDir )
		rutaDest = ddest + f;
	else 	rutaDest = ddest + barraDir + f;

	return VMCap::copiarArchivo( rutaOrg, rutaDest );
}

} // namespace Zero


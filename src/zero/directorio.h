//directorio.h
/*
	Encapsula las operaciones sobre directorios
*/


#ifndef DIRECTORIO_H_
#define DIRECTORIO_H_

#include <string>

namespace Zero {

class Directorio {
public:
	static const bool CREAR = true;
	static const bool ABRIR = false;
private:
	/// Es inicializado con la capa de abstracción de la plataforma con la barra
	static char barraDir;

	/// El nombre completo del directorio
	std::string nombre;

	/// Sólo el path
	std::string ruta;

	/// Sólo el nombre del directorio
	std::string nombreDir;

	/// Descompone el nombre del directorio en sus partes
	void descomponer(const std::string &);

public:
	Directorio(const std::string &n = "", bool modo = CREAR);

	const std::string &getNombre()    const { return nombre; }
	const std::string &getRuta()      const { return ruta;   }
	const std::string &getNombreDir() const { return nombreDir; }

	// Funcionalidad
	bool crearDirectorio(const std::string &);
	bool borrarArchivo(const std::string &);
	bool moverArchivoDe(const std::string &f, const std::string &dir);
	bool moverArchivoA(const std::string &f, const std::string &dir);
	bool copiarArchivoDe(const std::string &f, const std::string &dir);
	bool copiarArchivoA(const std::string &f, const std::string &dir);
	bool existeDirectorio(const std::string &d);
	bool existeArchivo(const std::string &f);
	bool cambiarDirectorio(const std::string &);

	// Funciones estáticas
	static bool crearDirectorio(const std::string&, const std::string &);
	static std::string getNombreDirectorioActual();
	static bool moverArchivo(const std::string & f, const std::string & dorg, const std::string &ddest);
	static bool copiarArchivo(const std::string & f, const std::string & dorg, const std::string &ddest);

};

}

#endif

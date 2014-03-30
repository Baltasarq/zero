// vmcap
/*
        Define las capacidades de la máquina virtual
	Comunicación entre la plataforma y la máquina virtual

        Pertenece a Zvm

        jbgarcia@uvigo.es
*/

#ifndef VMCAP_H
#define VMCAP_H

#include <string>

class VMCap {
	public:
        static const unsigned int MaxMACLength = 1024;
        typedef unsigned char MACId[MaxMACLength];
		typedef enum { ARCHIVO, DIRECTORIO } TipoArchivo;

		static const char BARRA_DIR_WINDOWS = '\\';
		static const char BARRA_DIR_UNIX    = '/';

		static const int HW_INTEL          = 0;
		static const int HW_SPARC          = 1;
		static const int HW_NOESPECIFICADO = 98;
		static const int HW_OTRO           = 99;

		static const int SO_LINUXUNIX       = 0;
		static const int SO_WINDOWS         = 1;
		static const int SO_MACOS           = 2;
		static const int SO_NOESPECIFICADO  = 98;
		static const int SO_OTRO            = 99;

		static bool PS;
		static std::string PSDir;
		static bool pantallaGrafica;
		static bool consola;

		static std::string getInfoCapabilities();

		static int getLoPlatformCode();
		static int getHiPlatformCode();
		static std::string getPlaformName();

		// Archivos y directorios
		static bool borrarArchivo(const std::string &);
		static bool copiarArchivo(const std::string &, const std::string &);
		static bool moverArchivo(const std::string &, const std::string &);
		static bool cheapStdCopiarArchivo(const std::string &, const std::string &);

		static char getDirMark();
		static bool buscaEnDirectorio(const std::string &d,
                              const std::string &nombreDir,
			      TipoArchivo ta = ARCHIVO)
		;
		static bool crearDirectorio(const std::string &, const std::string &);
		static bool borrarDirectorio(const std::string &);
		static std::string getNombreDirActual();
		static bool cambiarDirectorio(const std::string &);

		// Net
		static const MACId &getNetId();
        static unsigned int getNetIdLength()
            { return macIdLength; }
		static const MACId &getMACaddress();
private:
        static MACId macId;
        static unsigned int macIdLength;
};

#endif

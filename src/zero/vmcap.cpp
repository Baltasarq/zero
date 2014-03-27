// VMCap.cpp
/*
        Implanta la definición de capacidades de la MV
        Comunicación entre la plataforma y la MV

        Pertenece a zvm

        jbgarcia@uvigo.es
*/

// ======================= Particularidades de la plataforma ===================
/*
        A continuación, se trata de detectar para qué sistema operativo y
        procesador se está compilando.
        Todas estas detecciones se basan en las constantes que el compilador
        otorga.
        Se pueden forzar mediante el flag adecuado durante al compilar.
*/

// ================= Sistema operativo para el que se compila ==================

#ifndef _Z_SO_WINDOWS
    #ifndef _Z_SO_UNIX

        #ifdef __WIN32__
                #define _Z_SO_WINDOWS
        #else
            #ifdef _Windows
                #define _Z_SO_WINDOWS
            #else
                #ifdef __Windows
                    #define _Z_SO_WINDOWS
                #endif
            #endif
        #endif

        #ifdef __linux__
                #define _Z_SO_UNIX
        #else
                #ifdef __unix__
                        #define _Z_SO_UNIX
                #endif
        #endif

        #ifndef _Z_SO_WINDOWS
            #ifndef _Z_SO_UNIX
                    #error "Compilation stopped. No SO identified."
            #endif
        #endif

    #endif
#endif

// ======================== Procesador para el que se compila ==================

#ifndef _Z_HW_INTEL
    #ifndef _Z_HW_SPARC

    #ifdef __x86_64
        #define _Z_HW_INTEL
    #else
        #ifdef __i386__
            #define _Z_HW_INTEL
        #else
            #ifdef _M_IX86
                #define _Z_HW_INTEL
            #else
                #ifdef __sparc__
                        #define _Z_HW_SPARC
                #endif
            #endif
        #endif
    #endif

    #ifndef _Z_HW_INTEL
        #ifndef _Z_HW_SPARC
                #define _Z_HW_NULO
        #endif
    #endif

    #endif
#endif

// ==================== Hacer includes según la plataforma =====================
#ifdef _Z_SO_WINDOWS
	#include <dirent.h>
	#include <windows.h>
	#include <winsock2.h>
  	#include <iphlpapi.h>
#else
    #ifdef _Z_SO_UNIX
	#include <fcntl.h>
	#include <unistd.h>
	#include <dirent.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <sys/mman.h>
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <linux/if.h>
    #endif
#endif
// =============================================================================

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "vmcap.h"
#include "excep.h"

std::string VMCap::PSDir    = "";
bool VMCap::PS              = true;
bool VMCap::pantallaGrafica = false;
bool VMCap::consola         = true;

// ------------------------------------------------------- getInfoCapabilities()
std::string VMCap::getInfoCapabilities()
{
        std::string toret = "Enabled: ";

        if ( PS )  {
                toret += "Persistence, ";
        }

        if ( pantallaGrafica ) {
                toret += "Graphic User Interface, ";
        }

        if ( consola ) {
                toret += "Console.";
        }

        return toret;
}

// --------------------------------------------------------- getLoPlatformCode()
/*
getLoPlatformCode *: devuelve un código según el SO sobre la que se
asienta la máquina virtual Zero. Puede devolver los siguientes valores:
	0: Linux/Unix
	1: Windows
	98: Sin especificar
	99: Otro
*/

int VMCap::getLoPlatformCode()
{
	#ifdef _Z_SO_WINDOWS
		return SO_WINDOWS;
	#else
	      #ifdef _Z_SO_UNIX
	      	return SO_LINUXUNIX;
	      #else
		return SO_NOESPECIFICADO;
	      #endif
	#endif
}

// --------------------------------------------------------- getHiPlatformCode()
int VMCap::getHiPlatformCode()
/**
getHiPlatformCode *: devuelve un código según el hardware sobre el que se
asienta la máquina virtual Zero.
@return <ul>uno de los siguientes valores:
	<li>0: Intel
	<li>1: Sparc
	<li>98: Sin especificar
	<li>99: Otro
</ul>
*/
{
	#ifdef _Z_HW_INTEL
		return HW_INTEL;
        #else
	  #ifdef _Z_HW_SPARC
		return HW_SPARC;
	  #else
		return HW_NOESPECIFICADO;
	  #endif
	#endif
}

// ----------------------------------------------------------- getPlatformName()
std::string VMCap::getPlaformName()
/**
getPlatformName: devuelve el nombre de la plataforma sobre la que se asienta
la máquina virtual Zero.
@return <ul>Puede devolver los siguientes valores:
	<li>Linux/Unix: Lo devuelve una mv que se ejecuta sobre Unix/Linux.
	<li>Windows: Lo devuelve una mv que se ejecuta sobre Windows.
	<li>Other: Lo devuelve una mv que se ejecuta sobre cualquier otra plataforma
</ul>
*/
{
	std::string toret;
	int codigoSO = getLoPlatformCode();
	int codigoHW = getHiPlatformCode();

	switch( codigoHW ) {
		case HW_INTEL: 		toret = "Intel";
				   	break;
		case HW_SPARC:   	toret = "Sparc";
				   	break;
		case HW_NOESPECIFICADO: toret = "Unspecified";
				        break;
		case HW_OTRO:           toret = "Other";
	}

	toret += ' ';
	toret += '/';
	toret += ' ';

	switch( codigoSO ) {
		case SO_LINUXUNIX: 	toret += "Linux-Unix";
				   	break;
		case SO_WINDOWS:   	toret += "Windows";
				   	break;
		case SO_NOESPECIFICADO: toret += "Unspecified";
				        break;
		case SO_OTRO:           toret += "Other";
	}

	return toret;
}

// ----------------------------------------------------- VMCap::copiarArchivo()
bool VMCap::cheapStdCopiarArchivo(const std::string &org, const std::string &dest)
/**
        Si no hay suficiente memoria para copiar un archivo mediante DMA,
        entonces se copia byte a byte.
*/
{
	int c;
	bool toret   = false;
	FILE * forg  = fopen( org.c_str(), "rb" );
	FILE * fdest = fopen( dest.c_str(), "wb" );

	// Copiar, si se ha conseguido abrirlos
	if ( forg  != NULL
	  && fdest != NULL )
	{
		toret = true;

		c = fgetc( forg );

		while ( c != EOF ) {

			fputc( c, fdest );

			c = fgetc( forg );
		}
	}

	// Cerrar los archivos
	if ( forg != NULL ) {
		fclose( forg );
	}

	if ( fdest != NULL ) {
		fclose( fdest );
	}

	return toret;
}

// ------------------------------------------------------ VMCap::moverArchivo()
bool VMCap::moverArchivo(const std::string & org, const std::string & dest)
{
	#ifdef _Z_SO_WINDOWS
		return MoveFileEx(
                                org.c_str(),
                                dest.c_str(),
                                MOVEFILE_REPLACE_EXISTING
                );
	#else
		bool toret = copiarArchivo( org, dest );

		if ( toret ) {
			borrarArchivo( org );
		}

		return toret;
        #endif
}

// ----------------------------------------------------- VMCap::copiarArchivo()
bool VMCap::copiarArchivo(const std::string &archOrg, const std::string &archDest)
{
	#ifdef _Z_SO_WINDOWS
		return CopyFile( archOrg.c_str(), archDest.c_str(), false );
	#else
		bool toret   = false;
		int fichDest = -1;

	  	// Abrir el archivo
	  	int fich = open( archOrg.c_str(), O_RDONLY );

		if ( fich != -1 )
		{
			// Obtener info del del archivo (tamaño)
			struct stat st;
			fstat( fich, &st );

			// Pedir páginas en memoria para el fichero
			caddr_t mem = (caddr_t) mmap(
				0,		// Comienzo en memoria
				st.st_size,	// Tamaño del archivo
				PROT_READ,	// Deben poder ser leídas
				MAP_SHARED,	// Compartido por todos
				fich,		// Descriptor del fichero
				0  		// Offset en el fichero
			);

			if ( mem >= 0 ) {
				int fichDest = open(
                                        archDest.c_str(),
					O_WRONLY | O_CREAT,
                                        0666 )
				;

				if ( fichDest != -1 )
				{
					// Copiar el archivo en un solo bloque
					ssize_t escritos;

					escritos =
					    write( fichDest, mem, st.st_size )
					;

					toret = ( escritos == st.st_size );
				}

				// Eliminar la memoria reservada
				munmap( mem, st.st_size );
			}
			else {
				close( fich );
				toret = cheapStdCopiarArchivo( archOrg, archDest );
				fich = fichDest = -1;
			}
		}

		// Cerrar los archivos
		if ( fich != -1 ) {
			close( fich );
		}

		if ( fichDest != -1 ) {
			close( fichDest );
		}

		return toret;
	#endif
}

// ------------------------------------------------- VMCap::buscaEnDirectorio()
bool VMCap::buscaEnDirectorio(const std::string &d,
                              const std::string &nombreDir,
			      TipoArchivo ta)
/**
  * Esta función busca un directorio determinado en un directorio, haciendo
	uso de las ya famosas opendir() closedir().
	No es un estándar, pero casi ... es "de facto".
	La estructura dirent debe tener un campo d_type, que en algunos
	sistemas puede contener única y exclusivamente DT_UNKNOWN.
 * @param d El "archivo" a buscar
 * @param nombreDir El directorio donde buscar
 * @param ta El "tipo de archivo"
 * @return true si lo encuentra, false en otro caso
 */
{
	dirent *miembro = NULL;
	DIR *dir        = opendir( nombreDir.c_str() );

	if ( dir != NULL )
	{
		miembro = readdir( dir );

		while( miembro != NULL ) {
			if ( d == miembro->d_name ) {
				break;
			}

			miembro = readdir( dir );
		}

		closedir( dir );
	}

	return ( miembro != NULL );
}

// --------------------------------------------------- VMCap::crearDirectorio()
bool VMCap::crearDirectorio(const std::string & dir, const std::string & nombreDir)
{
	std::string dirCompleto;

	if ( dir[ dir.length() - 1 ] == getDirMark() )
		dirCompleto = dir + nombreDir;
	else	dirCompleto = std::string( dir + getDirMark() ) + nombreDir;

	#ifdef _Z_SO_WINDOWS
		return CreateDirectory( dirCompleto.c_str(), NULL );
	#else
		return ( mkdir( dirCompleto.c_str(), 0777 ) == 0 );
	#endif
}

// ----------------------------------------------------- VMCap::borrarArchivo()
bool VMCap::borrarArchivo(const std::string &f)
{
	#ifdef _Z_SO_WINDOWS
		return DeleteFile( f.c_str() );
	#else
		return ( unlink( f.c_str() ) == 0 );
	#endif
}

// -------------------------------------------------- VMCap::borrarDirectorio()
bool VMCap::borrarDirectorio(const std::string &d)
{
	#ifdef _Z_SO_WINDOWS
		return RemoveDirectory( d.c_str() );
	#else
		return ( rmdir( d.c_str() ) == 0);
	#endif
}

// ------------------------------------------------- VMCap::getNombreDirActual()
std::string VMCap::getNombreDirActual()
{
	char *buf;

	#ifdef _Z_SO_WINDOWS
		// Al pasarle NULL, devuelve el tamaño necesario
		int tam = GetCurrentDirectory( 0, NULL ) + 1;

		buf = (char *) malloc( tam + 1 );

		if ( buf == NULL ) {
			throw ENoHayMemoria( "obteniendo directorio actual" );
		}

		if ( GetCurrentDirectory( tam + 1, buf ) == 0 ) {
			throw EInterno( "obteniendo directorio de WIN32 API" );
		}
	#else
		buf = getcwd( NULL, 0 );
        #endif

	std::string toret = buf;

	std::free( buf );

	return toret;
}

// ------------------------------------------------- VMCap::cambiarDirectorio()
bool VMCap::cambiarDirectorio(const std::string &d)
{
	#ifdef _Z_SO_WINDOWS
		return SetCurrentDirectory( d.c_str() );
	#else
		return ( chdir( d.c_str() ) == 0 );
        #endif
}

// -------------------------------------------------------- VMCap::getDirMark()
char VMCap::getDirMark()
/**
 * getDirMArk() devuelve la marca de separación de directorios según SO
 * @return '/' para *NIX , y '\\' para Windows
 */
{
        #ifdef _Z_SO_WINDOWS
            return BARRA_DIR_WINDOWS;
        #else
            return BARRA_DIR_UNIX;
        #endif
}

// -------------------------------------------------------- VMCap::getNetId()
VMCap::MACId VMCap::macId;
unsigned int VMCap::macIdLength;

const VMCap::MACId &VMCap::getNetId()
{
	if ( macIdLength == 0 )
            return getMACaddress();
	else	return macId;
}

// ---------------------------------------------------- VMCap::getMACAddress()
#ifdef _Z_SO_WINDOWS
	const VMCap::MACId &VMCap::getMACaddress()
	{
		// Prepare macId
		std::memset( macId, 0, MaxMACLength );
		macIdLength = 0;

		// Reserve initial memory space
		PIP_ADAPTER_INFO pAdapterInfo;
		ULONG ulBufLen = sizeof( IP_ADAPTER_INFO );
   		pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof( IP_ADAPTER_INFO ) );

		if ( pAdapterInfo == NULL ) {
			goto END;
		}

		// Reserve adequate memory space, if needed
		if ( GetAdaptersInfo(pAdapterInfo, &ulBufLen ) == ERROR_BUFFER_OVERFLOW ) {
			free( pAdapterInfo );
			pAdapterInfo = (IP_ADAPTER_INFO *) malloc( ulBufLen );
			if ( pAdapterInfo == NULL ) {
			    goto END;
			}
		}

		// Retrieve information
		DWORD dwStatus = GetAdaptersInfo(      // Call GetAdapterInfo
			pAdapterInfo,                  // [out] buffer to receive data
			&ulBufLen);                    // [in] size of receive data buffer

		if ( dwStatus == NO_ERROR ) {
			std::memcpy( macId, pAdapterInfo->Address, pAdapterInfo->AddressLength );
			macIdLength = pAdapterInfo->AddressLength;
		}

		END:
		free( pAdapterInfo );
		return macId;
	}
#else
	const VMCap::MACId &VMCap::getMACaddress()
	{
		struct ifreq ifr;
		struct ifreq *IFR;
		struct ifconf ifc;
		char buf[1024];
		int s, i;

		// Init macId
		std::memset( &macId, 0, MaxMACLength );
		macIdLength = 0;

		// ask adapters number
		s = socket( AF_INET, SOCK_DGRAM, 0 );
		if ( s==-1 ) {
			goto END;
		}

		ifc.ifc_len = sizeof( buf );
		ifc.ifc_buf = buf;
		if ( ioctl( s, SIOCGIFCONF, &ifc ) != 0 ) {
			goto CLEAN;
		}

		// Run over all adapters
		IFR = ifc.ifc_req;
		for (i = ifc.ifc_len / sizeof( struct ifreq ); --i >= 0; IFR++) {
			strcpy( ifr.ifr_name, IFR->ifr_name );
			if ( ioctl( s, SIOCGIFFLAGS, &ifr ) == 0 ) {
			    if ( ! ( ifr.ifr_flags & IFF_LOOPBACK ) ) {
                    if ( ioctl(s, SIOCGIFHWADDR, &ifr) == 0 ) {
                        macIdLength = 6;
                        memcpy( macId, ifr.ifr_hwaddr.sa_data, macIdLength );
                        break;
                    }
			    }
			}
		}

		CLEAN:
		close( s );

		END:
		return macId;
	}
#endif


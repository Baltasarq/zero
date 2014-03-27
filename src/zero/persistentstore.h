// persistentstore.h
/*
	Representa al almacenamiento persistente.
	Es el encargado de recuperar contenedores y guardarlos a memoria
	secundaria.

	jbgarcia@uvigo.es
*/

#include "directorio.h"
#include "container.h"

#ifndef __PERSISTENTSTORE__H__
#define __PERSISTENTSTORE__H__

namespace Zero {

class PersistentStore {
protected:
        Objeto::ListaMnemosObjeto mnemosContainer;
        Objeto::ListaRefExternas refExt;
public:
	virtual ~PersistentStore() {}
	virtual Container * cargaContenedor(
                    const std::string &ref,
                    Container *,
                    Container * = NULL,
                    bool esLib = false)                               = 0;

	virtual void guardaContenedor(Container *);

	static PersistentStore *ps();
        static bool debeSerGuardado(Container *c)
                { return ( c->esAlcanzable() && c->fueModificadoDesdeCarga() ); }
};

class PersistentStoreEnDisco : public PersistentStore {
private:
	static char barraDir;
	Directorio dirBase;
	Directorio dirPS;
	static PersistentStoreEnDisco *perstor;

	PersistentStoreEnDisco(const std::string & = "");
public:
	~PersistentStoreEnDisco();

	const Directorio &getDirBase() const { return dirBase; }
	const Directorio &getDirPS()   const { return dirPS;   }

	Container * cargaContenedor(
                                const std:: string &ref,
                                Container *,
                                Container * = NULL,
                                bool esLib = false )
        ;

	void guardaContenedor(Container *);

    std::string &cnvtRutaEnDir(std::string &);
    std::string &cnvtDirEnRuta(std::string &);

	static void cargarContenedorDeFichero(
				Container &c,
				const Directorio &dirCont,
                                bool esLib = false )
	;

	static void salvarContenedorEnFichero(
				const Container &c,
				const Directorio &dirCont,
				Objeto::ListaMnemosObjeto &mnemosContainer )
	;

	static PersistentStoreEnDisco *ps();

	static std::string &cnvtSeparadoresEnBarraDirs(std::string &ruta)
            { return cnvtSeparadores( ruta, CHR_SEP_IDENTIFICADOR, barraDir ); }
    static std::string &cnvtBarraDirsEnSeparadores(std::string &ruta)
            { return cnvtSeparadores( ruta, barraDir, CHR_SEP_IDENTIFICADOR ); }

    static std::string &cnvtSeparadores(std::string &, char, char);
};

}

#endif

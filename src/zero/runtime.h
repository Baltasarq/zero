// runtime.h
/*
        El runtime es el encargado de gestionar el arranque de la MV,
        crear los contenedores adecuados, ... etc.

        jbgarcia@uvigo.es
*/

#ifndef RUNTIME_H
#define RUNTIME_H

#include <map>

#include "vmcap.h"
#include "container.h"
#include "cargaobj.h"
#include "compilador.h"
#include "conim.h"
#include "persistentstore.h"

namespace Zero {

class MetodoMV;
class Objeto;

// Tipos de datos ======================================================
// ================================ Operando ===========================
/*
	La clase operando se utiliza para operar con flotantes y enteros
	indistintamente.
*/

class Operando {
public:
	static const char SUMA = '+';
	static const char RESTA = '-';
	static const char MULTIPLICACION = '*';
	static const char DIVISION = '/';
	static const char LT = '<';
	static const char GT = '>';
	static const char EQ = '=';

	Operando(Objeto *);

	bool asignar(const Operando &op);
	Objeto * opMatBinaria(const Operando &op, char opr);
	bool opRelBinaria(const Operando &op, char opr);
	bool esNegativo() const;
	Objeto * abs();
	Objeto *toString();

	INT32 toInt() const;
	REAL toFloat() const;

private:
	Objeto * obj;
	bool esFlotante;
};

// ================================ Zero_Data_ =========================
class Zero_String_;

class Zero_Data_ {
protected:
        Objeto *zero;
public:
        static const size_t MaxPoolNums = 20;
        static const bool ES_SISTEMA = true;
        static const bool ES_NORMAL = false;

        Zero_Data_() : zero(NULL) {}
        virtual bool equals(Objeto *, Objeto *)         = 0;
        virtual ~Zero_Data_() {}

        virtual void *buscarPorObjeto(Objeto *)         = 0;

        virtual void elimina(Objeto *)                  = 0;
        virtual void eliminaTodos()                     = 0;

        Objeto *getZero() const { return zero; }
};

// ================================ Zero_Float_ ========================
class Zero_Float_ : public Zero_Data_ {
public:
        typedef std::map<Objeto *, REAL> TablaFlotantes;
private:
        TablaFlotantes nums;
        Objeto * pool[MaxPoolNums];
public:
        Zero_Float_();
        ~Zero_Float_();

        void elimina(Objeto *);
        void eliminaTodos();
        bool modifica(Objeto *obj, const REAL &valor);

        void mas(Objeto *obj, const REAL &d) { nums[obj] = d; }

        Objeto * nuevo(const std::string &, const REAL &, bool sys = ES_NORMAL);
        void   * buscarPorObjeto(Objeto *);
        double * busca(Objeto * obj) { return (REAL*) buscarPorObjeto(obj); };

        bool equals(Objeto *, Objeto *);
};

// ================================ Zero_Int_ ========================
class Zero_Int_ : public Zero_Data_ {
public:
        typedef std::map<Objeto *, INT32> TablaEnteros;
private:
        TablaEnteros nums;
        Objeto * pool[MaxPoolNums];
public:
        Zero_Int_();
        ~Zero_Int_();

        void elimina(Objeto *);
        void eliminaTodos();
        bool modifica(Objeto *obj, const INT32 &valor);

        void mas(Objeto *obj, const INT32 &d) { nums[obj] = d; }

        Objeto * nuevo(const std::string &, const INT32 &, bool sys = ES_NORMAL);
        void   * buscarPorObjeto(Objeto *);
        INT32  * busca(Objeto * obj) { return (INT32 *) buscarPorObjeto(obj); };

        bool equals(Objeto *, Objeto *);
};

// ================================ Zero_String_ =======================
class Zero_String_ : public Zero_Data_ {
public:
        typedef std::map<Objeto *, std::string> TablaCadenas;
private:
        TablaCadenas cads;
        enum ElementosPool {
            unEspacio, unPunto, unaComa, unasComillas, unApostrofe,
            unosDosPuntos, FinPool
        };
        Objeto * pool[ FinPool ];
public:
        Zero_String_();
        ~Zero_String_();

        void elimina(Objeto *);
        void eliminaTodos();
        bool modifica(Objeto *obj, const std::string &valor);

        void mas(Objeto *obj, const std::string & s) { cads[obj] = s; }

        Objeto * nuevo(const std::string &, const std::string &, bool sys = ES_NORMAL);
        void   * buscarPorObjeto(Objeto *);
        std::string * busca(Objeto *obj) { return (std::string*) buscarPorObjeto(obj); };


        bool equals(Objeto *, Objeto *);
        Objeto * toString(Objeto *obj)
                { return ( (busca( obj ) == NULL )? NULL : obj ); }
};

// ================================ Zero_Vector_ =======================
class LiteralVector_Zero {
	public:
		typedef std::vector <Referencia *> VectorImplementacion;
	private:
		VectorImplementacion v;
		Objeto * objetoAsociado;

		Referencia * creaRef(Objeto *);
		void eliminaRef(Referencia *);
	public:
		LiteralVector_Zero(Objeto *asoc);
		~LiteralVector_Zero();

		void eliminaTodos();

		Referencia *getElemento(size_t i) const
            { if ( i < longitud() ) return v[ i ]; else return NULL; }

		bool insertaElemento(size_t i, Objeto *);
		bool eliminaPosicion(size_t i);
		bool modificaPosicion(size_t i, Objeto *);
		bool mas(Objeto *);
		bool equals(LiteralVector_Zero *);
		bool copiaEn(LiteralVector_Zero *);

        size_t longitud() const
                    { return v.size(); }

		const VectorImplementacion *getVectorImplementacion() const
            { return &v; }

		Objeto *getObjetoAsociado() const
		    { return objetoAsociado; }

		void cambiaObjetoAsociado(Objeto *obj)
		    { eliminaTodos(); objetoAsociado = obj; }

};

class Zero_Vector_ : public Zero_Data_ {
public:
        typedef std::map<Objeto *, LiteralVector_Zero *> TablaVectores;
private:
        TablaVectores vects;
public:
        Zero_Vector_();
        ~Zero_Vector_() { eliminaTodos(); }

        void elimina(Objeto *);
        void eliminaTodos();

        Objeto * nuevo(const std::string &);
        void mas(Objeto * obj, LiteralVector_Zero *v)
            { vects[obj] = v; }

        void * buscarPorObjeto(Objeto *);

        LiteralVector_Zero * busca(Objeto *obj)
            { return (LiteralVector_Zero *) buscarPorObjeto(obj); }

        bool equals(Objeto *, Objeto *);
        bool copiaEn(Objeto *, Objeto *);
        Objeto * toString(Objeto *obj);
};


// ================================ Zero_Console_ ======================
class Zero_Console_ {
public:
        static const bool CR;

        void open();
        void close();
        Objeto * read();
        void write(Objeto *, bool);
};

// =========================== Gestor de Excepciones ===================
class GestorExcepciones {
        private:
                Objeto * objetoMensajeroExcepciones;
				void preparaObjetoRecipienteExcepcion(Metodo *);
        public:
                GestorExcepciones();
                ~GestorExcepciones();

                Objeto * getExcepcion();
                void ponExcepcion(Objeto *mensajero, Objeto *mensaje = NULL);
};

// ========================== Gestión del Stack ========================
class Stack {
public:

    class StackInfoItem {
    public:
        StackInfoItem(Objeto *obj, Metodo * met);
        ~StackInfoItem()
            { quien->decrementaReferencias();
              donde->decrementaReferencias();
            }

        Objeto *getObjeto() const
            { return quien; }
        Objeto *getObjetoDeMetodo() const
            { return donde; }
        Metodo *getMetodo() const;
        bool esMetodoRecursivo() const
            { return ( numRecursivo > 0 ); }
        unsigned int getNivelRecursivo() const
            { return numRecursivo; }
        std::string getNombreMetodo() const;
    private:
        Objeto * quien;             // this?
        Objeto * donde;             // obj del mth?
        Metodo * metodo;            // mth?
        unsigned int numRecursivo;  // recursivo?

        static std::string consNombre(Metodo *met, unsigned int numRec);
    };


    Stack()
        {}
    ~Stack();

    void eliminaTodos();
    void call(Objeto *o, Metodo *m);
    Objeto * getObjetoActual() const;
    Metodo * getMetodoActual() const;
    void ret(Objeto * = NULL, Metodo * = NULL);

    unsigned int getNumero() const
        { return stackInfo.size(); }

    const StackInfoItem *getElementoNumero(unsigned int i) const;
    std::string toString() const;
private:
    std::vector<StackInfoItem *> stackInfo;
};

// ================================ El Runtime =========================

class Runtime {
public:
        typedef std::vector<std::string> MensajeArranque; // Para arrancar el programa
        typedef std::vector<Metodo*> ListaMetsInic;  // Para los métodos de arranque
private:
	static Runtime *onert;	// Singleton de Runtime

        AlmacenObjetos * intStdLibCnt;
        AlmacenObjetos * ejecucionCnt;
        AlmacenObjetos * raizCnt;
	RefParadojaRaiz *refRaizCnt;
        GestorExcepciones gExcep;
        Stack             gStack;
        Zero_Console_     gConsola;
        ListaMetsInic     lMetsInic;

	bool enInicializacion;
	bool enFinalizacion;

	Runtime();

	// Inicio (llamados desde el constructor)
	void iniciarRuntimeConPS();
	void iniciarRuntimeSinPS();

        // Bootstrap de la mv
	void liberaArgumentos(MixinConArgumentos::Argumentos &args);
	void preparaArgumentos(MixinConArgumentos::Argumentos &args,
                               const std::string &arg1);
	void preparaArgumentos(MixinConArgumentos::Argumentos &args,
                               const std::string &arg1, const std::string &arg2);
	void preparaArgumentos(MixinConArgumentos::Argumentos &args,
                               const std::string &arg1, const std::string &arg2,
                               const std::string &arg3);

        void ejecutaMetsInic();                 // Metodos inic por ejecutar
        void bootstrap();                       // Bootstrapping de la MV
        static Objeto * creaObjetoTopLevel();   // Crea obj para ejecutar
        void eliminaSubcontenedoresRaiz();      // Al eliminar la MV
public:
        Referencia * resultadoEjecucion;

        bool estaEnInicializacion() const { return enInicializacion; }
        bool estaEnFinalizacion()   const { return enFinalizacion;   }

        ~Runtime();

        static Runtime * rt()
            { return ( ( onert == NULL )? ( new(std::nothrow) Runtime() ) : onert ); }

        static const bool NO_ES_LIB = false;    // Es la librería lo cargado ?
        static const bool ES_LIB    = true;

        // Prepara un .zbj para ser ejecutado. Caso particular: la librería
        Metodo * prepara(CargadorObjetos::ListaObjetos *,
                     bool esLib        = NO_ES_LIB,
                     MensajeArranque * = NULL,
                     AlmacenObjetos * dest  = NULL);
        static Metodo * preparaObjetoTopLevel(MensajeArranque *);

        // Ejecutar el objeto cargado
        void guardaMetInic(Metodo *met)
                { lMetsInic.push_back( met ); }

        Objeto * ejecutar(Metodo * = NULL);

        Referencia * getRefAContainerRaiz()      const { return refRaizCnt; }
        AlmacenObjetos * getContainerRaiz()      const { return raizCnt; }
        AlmacenObjetos * getContainerEjecucion() const { return ejecucionCnt; }
        AlmacenObjetos * getContainerIntStdLib() const { return intStdLibCnt; }

        /// Desincroniza todos los mths del stack, si ha cambiado algo
        void deSincronizaMetodos();

        // Auxiliares para llamadas a métodos de la MV
        /// Obtiene un objeto pasado como argumento a un método de la MV
        static Objeto * getObjetoArgumento(Metodo *, const std::string &);

        /// Obtiene un *dato* (=Int, Float, String) pasado como argumento a un MMV
        static void * getDatoArgumento(Metodo *met, Zero_Data_ *,
                                                             const std::string &x);

        // Funcionalidad de la máquina virtual
        static void getHiVersionNumber(Metodo *);
        static void getLoVersionNumber(Metodo *);
        static void getPlatformName(Metodo *);
        static void copyObject(Metodo *met);
        static void getNameOf(Metodo *);
        static void createChildOf(Metodo *);
        static void compareReferences(Metodo *);
        static void getHiPlatformCode(Metodo *);
        static void getLoPlatformCode(Metodo *);
        static void getCurrentTime(Metodo *);

        // Objetos
        static void getNumberOfMethodsOf(Metodo *);
        static void getMethodNumberByNameYOfX(Metodo *);
        static void callMethodNumberOfObj(Metodo *);
        static void deleteMethodNumberYOfX(Metodo *);
        static void getInfoOnMethodNumberYOfX(Metodo *);
        static void getNameOfMethodNumberYOfX(Metodo *);
        static void getOpcodesOfMethodNumberYOfX(Metodo *);
        static void getAttributeNumberYOfX(Metodo *);
        static void getAttributeNumberByNameYOfX(Metodo *);
        static void getNumberOfParentAttributeOf(Metodo *);
        static void getNumberOfAttributesOf(Metodo *);
        static void addAttributeYOfXValueZ(Metodo *);
        static void deleteAttributeNumberYOfX(Metodo *);
        static void getInfoOnAttributeNumberYOfX(Metodo *);
        static void getNameOfAttributeNumberYOfX(Metodo *);
        static void compileToMethodYOfX(Metodo *);
        static void addMethodYInX(Metodo *);
        static void getOwner(Metodo *);

        // PersistentStore
        static void getPersistentRoot(Metodo *);
        static void isPSPresent(Metodo *);

        // Consola
        static void openDefaultConsole(Metodo *);
        static void closeDefaultConsole(Metodo *);
        static void readFromDefaultConsole(Metodo *);
        static void writeToDefaultConsole(Metodo *);
        static void lfDefaultConsole(Metodo *);

        // Stack
        static void toStringStack(Metodo *);

        // Cadenas
        static void zeroString(Metodo *);
        static void lengthOfString(Metodo *);
        static void getPositionString(Metodo *);
        static void substringString(Metodo *);
        static void toFloatString(Metodo *);
        static void toIntString(Metodo *);
        static void isEqualToString(Metodo *);
        static void isLessThanString(Metodo *);
        static void isGreaterThanString(Metodo *);
        static void assignString(Metodo *);
        static void concatString(Metodo *);
        static void seqFindInString(Metodo *);
        static void seqFindLastInString(Metodo *);

		// Valores nums.
        static void multiplyBy(Metodo *);
        static void divideBy(Metodo *);
        static void sub(Metodo *);
        static void sum(Metodo *);
        static void assign(Metodo *);
        static void isLessThan(Metodo *);
        static void isGreaterThan(Metodo *);
        static void isEqualTo(Metodo *);
        static void isNegative(Metodo *);
        static void abs(Metodo *);
        static void toString(Metodo *);

        // Flotantes
        static void zeroFloat(Metodo *);
        static void toIntFloat(Metodo *);

        // Enteros
        static void zeroInt(Metodo *);
        static void modInt(Metodo *);
        static void toFloatInt(Metodo *);

        // Vectores
        static void clearVector(Metodo *);
        static void sizeVector(Metodo *);
        static void addObjectVector(Metodo *);
        static void erasePositionVector(Metodo *);
        static void getPositionVector(Metodo *);
        static void putPositionVector(Metodo *);
        static void insertObjectVector(Metodo *);
        static void isEqualToVector(Metodo *);
        static void seqFindInVector(Metodo *);
        static void seqFindLastInVector(Metodo *);
        static void processVector(Metodo *);
        static void toStringVector(Metodo *);

        // Propiedades
        static void getObjectPropertiesOf(Metodo *);
        static void getMethodPropertiesYOfX(Metodo *);

        // Atajos a los objetos más empleados
        static PersistentStore * ps;
        static Compilador *compilador;
        static Conim * conim;
        static GestorExcepciones *gestorExcepciones;
        static Stack *gestorStack;
        static Zero_Console_ *console;
        static Objeto *objetoRaiz;
        static Objeto *objetoNulo;
        static Objeto *objetoSistema;
        static Objeto *objetoExcepcion;
        static Objeto *objetoEPrivado;
        static Objeto *objetoEMismatch;
        static Objeto *objetoEMath;
        static Objeto *objetoEObjNoEncontrado;
        static Objeto *objetoEMetNoEncontrado;
        static Objeto *objetoInfoAtr;
        static Objeto *objetoInfoMth;
        static Objeto *objetoPropiedades;
        static void ponExcepcion(Objeto *objExcep, const std::string & = "");
        static void ponExcepcionObjeto(const std::string &);
        static void ponExcepcionMetodo(const std::string &);
        static void ponExcepcionTipoNumero(const std::string &);
        static void ponExcepcionTipoCadena(const std::string &);
        static void ponExcepcionTipoVector(const std::string &);
        static void ponExcepcionMates(const std::string &, const std::string &);

        // Las equivalencias de tipos con la VM
        static Objeto *objetoContainer;
        static Objeto *objetoFecha;
        static Objeto *objetoCadena;
        static Objeto *objetoFlotante;
        static Objeto *objetoEntero;
        static Objeto *objetoVector;
        static Objeto *objetoTrue;
        static Objeto *objetoFalse;
        static Zero_String_  *gestorCadenas;
        static Zero_Float_   *gestorNumerosFlotantes;
        static Zero_Int_     *gestorNumerosEnteros;
        static Zero_Vector_  *gestorVectores;

        // Otros
        static bool esObjetoPrimitivo(Objeto *);
};

typedef void (*MetodoRuntime)(Metodo *);

}

#endif

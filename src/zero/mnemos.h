// mnemos.h
/*
        Contiene todo lo relativo a la representación de los opcodes.
        Usado por:
              la máquina virtual Zero
              ensamblador Zero
              desensamblador Zero ...

        jbgarcia@uvigo.es
*/

#ifndef MNEMOS_H
#define MNEMOS_H

#include <cstdlib>
#include <string>
#include <vector>

#include "excep.h"
#include "reservadas.h"
#include "mediosoporte.h"

namespace Zero {

class AnalizadorLexico;

extern const UINT16   SIGNATURE;

extern const UINT8    hver;
extern const UINT8    lver;

// Mnemotécnicos -------------------------------------------------------------

extern const UINT16   NOBJ;
extern const UINT16   NATR;
extern const UINT16   NMTH;
extern const UINT16   NDEF;
extern const UINT16   NASG;
extern const UINT16   NMSG;
extern const UINT16   NIOF;
extern const UINT16   NRET;
extern const UINT16   NENO;
extern const UINT16   NENM;
extern const UINT16   NSTR;
extern const UINT16   NTRW;
extern const UINT16   NFLT;
extern const UINT16   NSET;
extern const UINT16   NJMP;
extern const UINT16   NJOT;
extern const UINT16   NJOF;
extern const UINT16   NINT;
extern const UINT16   NETQ;
extern const UINT16   NNOP;
extern const UINT16   NMTA;

// ================================ Tipos de Datos =============================
/**
	Esta clase proporciona una forma unificada de acceder,
	dentro del formato zbj,
	a nombres de registros de la MV, ids y referencias
	completas (i.e., id.id.id)
*/
class Nombre {
public:
        virtual const std::string &getNombre()      const = 0;
        virtual bool  verifica()                    const = 0;
        virtual void  escribe(MedioSoporte *)       const = 0;
        virtual ~Nombre() {}
        virtual Nombre * copia()                    const = 0;

        static Nombre * lee(MedioSoporte *);
        static Nombre * creaNombreAdecuado(const std::string &);
};

/**
	Esta clase proporciona una forma unificada de acceder,
	dentro del formato zbj, a nombres de registros de la MV
*/
class NombreRegistro : public Nombre {
    public:
        enum CodigoRegistro {__ACC, __THIS, __RR, __EXC,
                  __GP1, __GP2, __GP3, __GP4, __NOREG }
        ;

        static const unsigned int numRegistros = __NOREG;
        static const std::string * strReg[];

    public:
        NombreRegistro(CodigoRegistro cr) : rc(cr) {}
        NombreRegistro(const std::string &nr) {
                rc = cnvtNombreACodigoRegistro( nr );
				verifica();
        }

        bool verifica() const;

        CodigoRegistro getCodigoRegistro() const { return rc; }
        const std::string &getNombre() const
            { return cnvtCodigoRegistroANombre(rc); }

        NombreRegistro * copia() const;
        void escribe(MedioSoporte *f) const;
        static NombreRegistro *lee(MedioSoporte *f);

        static const std::string         &cnvtCodigoRegistroANombre(CodigoRegistro);
        static CodigoRegistro        cnvtNombreACodigoRegistro(const std::string &);

    private:
        CodigoRegistro rc;
};

/**
	Esta clase proporciona una forma unificada de acceder,
	dentro del formato zbj, a referencias
*/
class NombreReferencia : public Nombre {
public:
        NombreReferencia(const std::string &n);

        const std::string &getNombre() const
            { return nombre; }
        bool verifica()           const
            { return compruebaRef(nombre); }

        void escribe(MedioSoporte *f) const;
        NombreReferencia * copia() const;

        static NombreReferencia * lee(MedioSoporte *f);

        static bool compruebaRef(const std::string &);
private:
        std::string nombre;
};

/**
	Esta clase proporciona una forma unificada de acceder,
	dentro del formato zbj, a identificadores
*/
class NombreIdentificador : public Nombre {
private:
        std::string nombre;
public:
        static const unsigned int MaxTamIdentificadores = 64;

        NombreIdentificador(const std::string &n);

        bool verifica() const
            { return compruebaIdMetodo( getNombre() ); }

        const std::string &getNombre() const
            { return nombre; }

        NombreIdentificador * copia() const;
        void escribe(MedioSoporte *f) const;

        static bool compruebaId(const std::string &);
        static bool compruebaIdMetodo(const std::string &);

        static void chkId(const std::string &) throw(ESintaxis);

        static NombreIdentificador * lee(MedioSoporte *);
};

// ================================ Ensamblador ================================
/**
        Los Mnemotécnicos son la representación de los Opcodes
	@see Opcode
*/
// ---------------------------------------------------------------- Mnemotécnico
class Mnemotecnico {
protected:
        UINT16 mnemo;

public:
	static const bool LeeMnemotecnico = true;
	static const std::string mnemoStr;

        Mnemotecnico(int nm) : mnemo(nm) {}
        virtual ~Mnemotecnico() {}

        virtual void verifica() throw(ECompilacion) = 0;

        virtual void escribe(MedioSoporte *);
        virtual void lee(MedioSoporte *, bool leeMnemo = false);

        virtual std::string listar(bool bello = false) = 0;
        virtual std::string getFormatoXML() = 0;

        static Mnemotecnico *cargar(MedioSoporte *);
	static Mnemotecnico *compilar(AnalizadorLexico &);
};

// ---------------------------------------------------------- MixinConArgumentos
class MixinConArgumentos {
public:
    typedef std::vector<Nombre *> Argumentos;
private:
	Argumentos args;
public:
        static const Argumentos argumentosNulos;

	virtual ~MixinConArgumentos() { eliminaArgumentos(); }

        void eliminaArgumentos();
        void masArgumentos(const std::string &x);
        void masArgumentos(NombreRegistro::CodigoRegistro rc);
        bool buscaEnVectorArgs(const std::string &);
	void copiarArgumentosDe(const MixinConArgumentos &);
	void leeVectorArgs(MedioSoporte *);
	void escribeVectorArgs(MedioSoporte *);

        Argumentos *getArgs()             { return &args; }
	const Argumentos *getArgs() const { return &args; }

        static void copiarArgumentos(Argumentos &, const Argumentos &);

        static void leeVectorArgs(MedioSoporte *, Argumentos &args);
        static void escribeVectorArgs(MedioSoporte *, const Argumentos &args);
};

// -------------------------------------------------------------------- Cabecera
class Cabecera : public Mnemotecnico {
        UINT16 byteOrd;                    // Valor 0x00FF
        UINT16 ver;                        // H mayor, L menor
public:
        Cabecera() : Mnemotecnico(SIGNATURE), byteOrd(0x00FF) {
                ver = (hver << 8) + lver;
        }

	void verifica() throw (ECompilacion) 	{}

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool = true);

        bool estaAlReves() const                { return (byteOrd == 0x00FF);  }
        int  hiVersion()   const                { return (ver & 0xFF00) >> 8;  }
        int  loVersion()   const                { return (ver & 0x00FF);       }
        bool esZeroLegal() const                { return (mnemo == SIGNATURE); }

        virtual std::string listar(bool bello = false);
        virtual std::string getFormatoXML();
};

// =============================================================== Mnemotécnicos
// ----------------------------------------------------------------------- NMMta
class NMMta : public Mnemotecnico {
public:
        enum Pragma { FinalDeclConstantes, Objeto, Error };
        static const std::string strPragma[];
private:
        Pragma pragma;
        std::string idObj;
        std::string datos;

        /// Convierte una cadena en un código de pragma y, si corresponde,
        /// su idObj asociado.
        Pragma strToPragma(const std::string &);
public:
        static const std::string formatoXML;
        static const std::string mnemoStr;

        NMMta() : Mnemotecnico(NMTA) {}
        NMMta(const std::string &pr, const std::string &dt);
        NMMta(const Pragma &pr, const std::string &dt);
        ~NMMta();

        void verifica() throw (ECompilacion);

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

        std::string listar(bool bello = false);
        std::string getFormatoXML();

        /// Obtiene una cadena correspondiente con el tipo de pragma almacenado
        /// si el pragma es 'Objeto', entonces devuelve la id del objeto
	const std::string &getPragmaAsString() const;

        /// Si el pragma es 'Objeto', entonces con este método se obtiene
        /// el id de ese objeto.
        const std::string &getIdObj() const { return idObj; }

        const Pragma &getPragma()    const { return pragma; }
        const std::string &getDatos()     const { return datos; }

	static NMMta *compilar(AnalizadorLexico &);

        /// Convierte una cadena en un código de pragma
	static NMMta::Pragma cnvtStrToPragma(const std::string & x);

        /// Convierte un código de pragma a una cadena simple. Si la devolución
        /// es "OBJ", entonces es necesario obtener el id con getIdObjeto()
	static const std::string &cnvtPragmaToStr(Pragma p);
};

// ----------------------------------------------------------------------- CtrOb
class CtrOb : public Mnemotecnico {
public:
        CtrOb(int nm) : Mnemotecnico(nm) {}
	virtual ~CtrOb() {}
};

// ----------------------------------------------------------------------- NMObj
class NMObj : public CtrOb {
        std::string nombre;
        Nombre * padre;
public:
        static const std::string formatoXML;
        static const std::string mnemoStr;

	NMObj();
        NMObj(const std::string &nom, const std::string &p);
	~NMObj();

	void verifica() throw (ECompilacion);

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

        std::string listar(bool bello = false);
        std::string getFormatoXML();

        const std::string &getNombre()    const { return nombre; }
        const std::string &getNomPadre()  const { return padre->getNombre(); }

	static NMObj *compilar(AnalizadorLexico &);
};


// ----------------------------------------------------------------------- NMEno
class NMEno : public CtrOb {
public:
        static const std::string formatoXML;
        static const std::string mnemoStr;

        NMEno() : CtrOb(NENO) {}
        std::string listar(bool bello = false);
        std::string getFormatoXML();

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

	void verifica() throw (ECompilacion);

	static NMEno *compilar(AnalizadorLexico &);
};

// ----------------------------------------------------------------------- NMAtr
class NMAtr : public CtrOb {
        std::string acceso;
        std::string nombre;
		std::string lit;
        Nombre * ref;
public:
        static const std::string formatoXML;
        static const std::string mnemoStr;
        static const std::string PUBLICO;
        static const std::string PRIVADO;

		NMAtr();
        NMAtr(const std::string &acc,
	      const std::string &nom,
	      const std::string &r,
	      const std::string &lit = "");
		~NMAtr();

		void verifica() throw (ECompilacion);

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

        std::string listar(bool bello = false);
        std::string getFormatoXML();

        const std::string &getNombre() const { return nombre; }
        const std::string &getRef()    const { return ref->getNombre(); }
        const std::string &getAcceso() const { return acceso; }
		const std::string &getLit()    const { return lit; }  /// sólo al compilar

        bool esAccesoPublico() { return (acceso == PUBLICO); }
        bool esAccesoPrivado() { return (acceso == PRIVADO); }

		static NMAtr *compilar(AnalizadorLexico &);
};


// ----------------------------------------------------------------------- CtrMt
class CtrMt : public Mnemotecnico {
public:
        CtrMt(int nm) : Mnemotecnico(nm) {}
	virtual ~CtrMt() {}
};

// ----------------------------------------------------------------------- NMMth
class NMMth : public CtrMt, public MixinConArgumentos {
        std::string acceso;
        std::string nombre;
public:
        static const std::string formatoXML;
        static const std::string mnemoStr;
        static const std::string PUBLICO;
        static const std::string PRIVADO;

	NMMth() : CtrMt(NMTH) {}
        NMMth(const std::string &acces, const std::string &nom);
        NMMth(const NMMth &);

	void verifica() throw (ECompilacion);

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

        std::string listar(bool bello = false);
        std::string getFormatoXML();

        bool esAccesoPublico() { return (acceso == PUBLICO); }
        bool esAccesoPrivado() { return (acceso == PRIVADO); }

        const std::string &getNombre()    const { return nombre; };

	static NMMth *compilar(AnalizadorLexico &);
};

// ----------------------------------------------------------------------- NMEnm
class NMEnm : public CtrMt {
public:
        static const std::string formatoXML;
        static const std::string mnemoStr;

        NMEnm() : CtrMt(NENM) {}
        std::string listar(bool bello = false);
        std::string getFormatoXML();

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

	void verifica() throw (ECompilacion);

	static NMEnm *compilar(AnalizadorLexico &);
};

// ----------------------------------------------------------------------- Instr
class Instr : public Mnemotecnico {
public:
        Instr(int nm) : Mnemotecnico(nm) {}

	virtual void verifica() throw (ECompilacion) = 0;

	virtual ~Instr() {}
};

// ----------------------------------------------------------------------- MNMsg
class NMMsg : public Instr, public MixinConArgumentos {
  private:
        Nombre *ref;
        std::string met;

  public:
        static const std::string formatoXML;
        static const std::string mnemoStr;

	NMMsg();
        NMMsg(const std::string &r, const std::string &m);
        NMMsg(const NMMsg &);
        ~NMMsg() { delete ref; eliminaArgumentos(); }

        const std::string &getMetodo()		const 	{ return met; }
        Nombre *&getReferencia()				{ return ref; }
        Nombre * getReferencia()  		const  	{ return ref; }
        const std::string &getNombreDeObjeto() const { return ref->getNombre(); }
	const std::string &getNombre()         const { return getNombreDeObjeto(); }

	void verifica() throw (ECompilacion);

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

        std::string listar(bool bello = false);
        std::string getFormatoXML();

	static NMMsg *compilar(AnalizadorLexico &);
};

// ----------------------------------------------------------------------- NMDef
class NMDef : public Instr {
  private:
        std::string nombre;
  public:
        static const std::string formatoXML;
        static const std::string mnemoStr;

	NMDef() : Instr(NDEF) {}
        NMDef(const std::string &nom);

        const std::string &getNombre()    const { return nombre; };

	void verifica() throw (ECompilacion);

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

        std::string listar(bool bello = false);
        std::string getFormatoXML();

	static NMDef *compilar(AnalizadorLexico &);
};

// ----------------------------------------------------------------------- NMRet
class NMRet : public Instr {
        Nombre * nombre;
public:
        static const std::string formatoXML;
        static const std::string mnemoStr;

	NMRet();
        NMRet(const std::string &nom);
        NMRet(NombreRegistro::CodigoRegistro rc);
        NMRet(const NMRet &);
        ~NMRet() { delete nombre; }

        const std::string &getNombre()    const { return nombre->getNombre(); };

        Nombre * getReferencia() const 	{ return nombre; }
        Nombre *& getReferencia() 		{ return nombre; }

		void verifica() throw (ECompilacion);

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

        std::string listar(bool bello = false);
        std::string getFormatoXML();

	static NMRet *compilar(AnalizadorLexico &);
};

// ----------------------------------------------------------------------- MNAsg
class NMAsg : public Instr {
        Nombre * nombre;
        NombreRegistro *reg;
public:
        static const std::string formatoXML;
        static const std::string mnemoStr;

        NMAsg();
        NMAsg(const std::string &nom, NombreRegistro::CodigoRegistro rc);
        NMAsg(const NMAsg &);
        ~NMAsg() { delete nombre; delete reg; }

        const std::string &getNombre() const { return nombre->getNombre(); }

        NombreRegistro       *getRegistro()   { return reg; }
        Nombre *getReferencia() { return nombre; }

        void verifica() throw (ECompilacion);

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

        std::string listar(bool bello = false);
        std::string getFormatoXML();

	static NMAsg *compilar(AnalizadorLexico &);
};

// ------------------------------------------------------------------------ Lit
class Lit : public Instr {
protected:
	std::string lit;
public:
    Lit(int nm) : Instr(nm) {}
	virtual ~Lit() {}

	virtual const std::string &getLitAsString() = 0;
};

// ---------------------------------------------------------------------- NMStr
class NMStr : public Lit {
public:
	static const char CHR_ESCAPE_CR        = 'n';
	static const char CHR_ESCAPE_TAB       = 't';
	static const char CHR_ESCAPE_COMILLAS  = '\"';
	static const char CHR_ESCAPE_APOSTROFE = '\'';
	static const char CHR_ESCAPE_DEC       = 'd';
	static const char CHR_ESCAPE_NUMDIGITOS_DEC = 3;

	static const std::string formatoXML;
	static const std::string mnemoStr;

	NMStr() : Lit(NSTR) {}
    NMStr(const std::string &l);

	void verifica() throw (ECompilacion);

	void escribe(MedioSoporte *);
	void lee(MedioSoporte *, bool leeMnemo = false);

	std::string listar(bool bello = false);
	std::string getFormatoXML();

	const std::string &getLiteral()     const { return lit; }
	const std::string &getLitAsString() { return getLiteral(); }

	static std::string &convertirSeqEscapeAStr(std::string &lit);
	static std::string &convertirStrASeqEscape(std::string &lit);

	static NMStr *compilar(AnalizadorLexico &);
};

// ----------------------------------------------------------------------- NMFlt
class NMFlt : public Lit {
        double value;
public:
        static const std::string formatoXML;
        static const std::string mnemoStr;

	NMFlt() : Lit(NFLT) {}
        NMFlt(const std::string &l) : Lit(NFLT)
        	{ value = std::atof( l.c_str() ); }

	NMFlt(double v) : Lit(NFLT), value(v)
        	{  }

        double getLiteral() const { return value; }

	const std::string &getLitAsString();

	void verifica() throw (ECompilacion);

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

        std::string listar(bool bello = false);
        std::string getFormatoXML();

	static NMFlt *compilar(AnalizadorLexico &);
};

// ----------------------------------------------------------------------- NMInt
class NMInt : public Lit {
    INT32 value;
public:
    static const std::string formatoXML;
    static const std::string mnemoStr;

	NMInt() : Lit(NINT)
        {}
    NMInt(const std::string &l) : Lit(NINT)
        { value = std::atoi( l.c_str() ); }
	NMInt(INT32 v) : Lit(NINT), value(v)
        {}

    INT32 getLiteral() const { return value; }

	const std::string &getLitAsString();

	void verifica() throw (ECompilacion);

    void escribe(MedioSoporte *);
    void lee(MedioSoporte *, bool leeMnemo = false);

    std::string listar(bool bello = false);
    std::string getFormatoXML();

	static NMInt *compilar(AnalizadorLexico &);
};

// ----------------------------------------------------------------------- NMIof
class NMIof : public Instr {
        Nombre *ref;
        NombreRegistro *reg;
public:
        static const std::string formatoXML;
        static const std::string mnemoStr;

	NMIof();
        NMIof(const std::string &r, NombreRegistro::CodigoRegistro rc);
        NMIof(const NMIof &);
        ~NMIof();

        const std::string &getNombre()         const  { return ref->getNombre(); }

        Nombre * getReferencia() const  { return ref; }
        NombreRegistro       * getRegistro()   const  { return reg; }

	void verifica() throw (ECompilacion);

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

        std::string listar(bool bello = false);
        std::string getFormatoXML();

	static NMIof *compilar(AnalizadorLexico &);
};

// ----------------------------------------------------------------------- NMSet
class NMSet : public Instr {
        NombreRegistro       *reg;
        Nombre *ref;
public:
        static const std::string formatoXML;
        static const std::string mnemoStr;

	NMSet();
        NMSet(NombreRegistro::CodigoRegistro, const std::string &);
        NMSet(const NMSet &);
        ~NMSet() { delete reg; delete ref; }

        const std::string  &getNombre()     const { return ref->getNombre(); }

        Nombre  *getReferencia()  const { return ref; }
        NombreRegistro        *getRegistro()    const { return reg; }

	void verifica() throw (ECompilacion);

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

        std::string listar(bool bello = false);
        std::string getFormatoXML();

	static NMSet *compilar(AnalizadorLexico &);
};

// ----------------------------------------------------------------------- NMNop
class NMNop : public Instr {
public:
        static const std::string formatoXML;
        static const std::string mnemoStr;

        NMNop();

        std::string listar(bool bello = false);
        std::string getFormatoXML();

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

	void verifica() throw (ECompilacion);

	static NMNop *compilar(AnalizadorLexico &);
};

// ----------------------------------------------------------------------- NMEtq
class NMEtq : public Instr {
        std::string nombreEtq;
public:
        static const std::string formatoXML;
        static const std::string mnemoStr;

	NMEtq() : Instr(NETQ) {}
        NMEtq(const std::string &nom);

        const std::string &getNombre()    const { return nombreEtq; };

	void verifica() throw (ECompilacion);

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

        std::string listar(bool bello = false);
        std::string getFormatoXML();

	static NMEtq *compilar(AnalizadorLexico &);
};

// ----------------------------------------------------------------------- NMTrw
class NMTrw : public Instr {
        Nombre *ref;
        Nombre *refMensaje;
public:
	static const std::string mensajeNulo;
        static const std::string formatoXML;
        static const std::string mnemoStr;

	NMTrw();
        NMTrw(const std::string &r, const std::string &m = "");
        ~NMTrw() { delete ref; delete refMensaje; }
        NMTrw(const NMTrw &);


        const std::string &getNombre()     const { return ref->getNombre(); };
        const std::string &getMensaje()    const {
                if (refMensaje != NULL)
                        return refMensaje->getNombre();
                else    return mensajeNulo;
        }

        Nombre *&getReferencia()        		{ return ref; }
        Nombre *&getReferenciaMensaje() 		{ return refMensaje; }

        Nombre * getReferencia()        const 	{ return ref; }
        Nombre * getReferenciaMensaje() const 	{ return refMensaje; }

	void verifica() throw (ECompilacion);

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

        std::string listar(bool bello = false);
        std::string getFormatoXML();

	static NMTrw *compilar(AnalizadorLexico &);
};

// ------------------------------------------------------------------------- Flw
class Flw : public Instr {
private:
	std::string etq;
	INT16 numOpcodes;

public:
	Flw(int n) : Instr(n) {}
	virtual ~Flw() {}

	void verifica() throw (ECompilacion);

        void escribe(MedioSoporte *);
        void lee(MedioSoporte *, bool leeMnemo = false);

	virtual void  putEtiqueta(std::string const & e);
	virtual const std::string &getEtiqueta() const       { return etq;        }
	virtual INT16 getNumero() const 		{ return numOpcodes; }
	virtual void  putNumero(INT16 nop);
};

// ----------------------------------------------------------------------- NMJmp
class NMJmp : public Flw {
public:
        static const std::string formatoXML;
        static const std::string mnemoStr;

	NMJmp() : Flw(NJMP) {}
        NMJmp(const std::string &m);

        std::string listar(bool bello = false);
        std::string getFormatoXML();

	static NMJmp *compilar(AnalizadorLexico &);
};

// ----------------------------------------------------------------------- NMJot
class NMJot : public Flw {
public:
        static const std::string formatoXML;
        static const std::string mnemoStr;

	NMJot() : Flw(NJOT) {}
        NMJot(const std::string &m);

        std::string listar(bool bello = false);
        std::string getFormatoXML();

	static NMJot *compilar(AnalizadorLexico &);
};

// ----------------------------------------------------------------------- MNJof
class NMJof : public Flw {
public:
        static const std::string formatoXML;
        static const std::string mnemoStr;

	NMJof() : Flw(NJOF) {}
        NMJof(const std::string &m);

        std::string listar(bool bello = false);
        std::string getFormatoXML();

	static NMJof *compilar(AnalizadorLexico &);
};

}

#endif

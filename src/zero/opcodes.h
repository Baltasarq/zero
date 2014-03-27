// opcodes.h
/*
       Los opcodes y los mnemotécnicos son partes del mismo concepto, las
	instrucciones.
	De éstas, los primeros constituyen la funcinalidad, y los segundos
	su representación.

        Ésta es la implantación de los opcodes, que es lo que hace que los
        mnemotécnicos puedan realmente "hacer" cosas.

        Necesita mnemos.h y mnemos.cpp como parte fundamental.

        Pertenece a zvm

        jbgarcia@uvigo.es
*/

#ifndef OPCODES_H
#define OPCODES_H

#include "mnemos.h"
#include "referencia.h"
#include "excep.h"

namespace Zero {

class Metodo;

// ================================== Opcode ===================================
/**
	La clase Opcode es clase base de todas las clases que representan
	la funcionalidad de los opcodes reconocidos por la MV.
	La clase Mnemotécnico es la clase que representa estos mismo Opcodes
	en memoria secundaria, es decir, cuando no están en memoria, sino
	en el almacenamiento persistente.

	@see Mnemotecnico
*/

class Opcode {
protected:
        Metodo *met;
        bool enFase;

        /// Solo derivados de Instr y de CtrMg
        Mnemotecnico *mnemo;

		Referencia *resuelveRegORefLoc(Nombre *locOreg);
        Referencia * resuelveRefORegEnRef(Nombre *reforeg);
        Referencia * resuelveRefORegEnLoc(Nombre *reforeg);
        Referencia * resuelveReg(NombreRegistro *reforeg);
public:
        virtual Mnemotecnico *getMnemo() const
            { return mnemo; }

        Opcode(Metodo *);
        virtual ~Opcode();

        Metodo * getPerteneceA() const
            { return met; }

        void cambiaEstadoMetodo(Metodo * met)
            { this->met = met; deSincroniza(); }
        bool getFase() const
            { return enFase; }
        bool estaEnFase() const;
        virtual void deSincroniza()
            { if ( estaEnFase() ) enFase = !enFase; }

        virtual void ejecuta()                    = 0;       // El que trabaja
        virtual void preparaReferencias()         = 0;
		virtual Opcode *copia(Metodo *)           = 0;

        static Opcode *creaOpcodeParaMnemo(Mnemotecnico *, Metodo *);
		static void    asignarReferencia(Referencia *, Referencia *);
};


// ================================== OcNOP ====================================
class OcNOP : public Opcode {
public:
        OcNOP(NMNop *mn, Metodo *m);
        ~OcNOP() {}

        NMNop * getMnemo() const { return static_cast<NMNop *>( mnemo ); }

	OcNOP * copia(Metodo * met) {
		OcNOP * toret = new(std::nothrow) OcNOP( getMnemo(), met );

		if ( toret != NULL ) {
			throw ENoHayMemoria( "duplicando opcode" );
		}

		return toret;
	}

        void ejecuta() {}
	void preparaReferencias() {}
};

// ================================== OcSTR ====================================
class OcSTR : public Opcode {
public:
    OcSTR(NMStr *, Metodo *);
    ~OcSTR();

    NMStr * getMnemo() const { return static_cast<NMStr *>( mnemo ); }

	void preparaReferencias() {}
    void ejecuta();

	OcSTR * copia(Metodo * met);
};

// ================================== OcMSG ====================================
class OcMSG : public Opcode {
    Referencia *ref;
    std::vector<Referencia *> args;
    Metodo *metDest;
    Objeto *obj;
	bool huboExcepcion;
	bool super;
	std::string metodo;

    void preparaArgumentos();
public:
    OcMSG(NMMsg *, Metodo *);
    ~OcMSG();

    void deSincroniza();

    const std::string &getMetodo() const
        { return metodo; }

    MixinConArgumentos::Argumentos *getArgumentos() const
        { return getMnemo()->getArgs(); }

    NMMsg *getMnemo() const
        { return static_cast<NMMsg *>( mnemo ); }

    void preparaReferencias();
    void ejecuta();
	OcMSG * copia(Metodo * met);

    Referencia * getReferencia() const
        { return ref; }

	bool esLlamadaSuper()
        { return super; }
};

// ================================== OcRET ====================================
class OcRET : public Opcode {
public:
        OcRET(NMRet *, Metodo *);
        ~OcRET();

        NMRet *getMnemo() const       { return static_cast<NMRet *>( mnemo ); }

	void preparaReferencias() {}
        void ejecuta();

	OcRET * copia(Metodo * met);
};

// ================================== OcSET ====================================
class OcSET : public Opcode {
        Referencia *reg;
        Referencia *ref;
public:
        OcSET(NMSet *, Metodo *);
        ~OcSET();

        NMSet *getMnemo() const
            { return static_cast<NMSet *>( mnemo ); }

        void ejecuta();
        void deSincroniza();

        OcSET * copia(Metodo * met);

        void preparaReferencias();

        Referencia *getReferencia() const
            { return ref; }
};

// ================================== OcDEF ====================================
class OcDEF : public Opcode {
public:
        OcDEF(NMDef *, Metodo *);
        ~OcDEF();

        NMDef *getMnemo() const       { return static_cast<NMDef *>( mnemo ); }

	void preparaReferencias() {}
        void ejecuta();

	OcDEF * copia(Metodo * met);
};

// ================================== OcASG ====================================
class OcASG : public Opcode {
private:
        Referencia *ref;
        Referencia *reg;
public:
        OcASG(NMAsg *, Metodo *);
        ~OcASG();

        NMAsg *getMnemo() const       { return static_cast<NMAsg *>( mnemo ); }

        void ejecuta();

	OcASG * copia(Metodo * met);

        void preparaReferencias();
};

// ================================== OcJMP ====================================
class OcFlw : public Opcode {
protected:
        void asignaIPMetodo(void *dir);

        void * dirDirecta;

        OcFlw(Metodo *);
	~OcFlw();

public:
        Flw *getMnemo() const           { return static_cast<Flw *>( mnemo ); }

        void ejecuta() = 0;

        void ponDirDirecta(void *dir)   { dirDirecta = dir; }
};

// ================================== OcJMP ====================================
class OcJMP : public OcFlw {
public:
        OcJMP(NMJmp *, Metodo *);
        ~OcJMP();

        NMJmp *getMnemo() const     { return static_cast<NMJmp *>( mnemo ); }

	void preparaReferencias() {}
        void ejecuta();

	OcJMP * copia(Metodo * met);
};

// ================================== OcJOT ====================================
class OcJOT : public OcFlw {
public:
        OcJOT(NMJot *, Metodo *);
        ~OcJOT();

        NMJot *getMnemo() const       { return static_cast<NMJot *>( mnemo ); }

	void preparaReferencias() {}
        void ejecuta();

	OcJOT * copia(Metodo * met);
};

// ================================== OcJOF ====================================
class OcJOF : public OcFlw {
public:
        OcJOF(NMJof *, Metodo *);
        ~OcJOF();

        NMJof *getMnemo() const       { return static_cast<NMJof *>( mnemo ); }

	void preparaReferencias() {}
        void ejecuta();

	OcJOF * copia(Metodo * met);
};

// ================================== OcFLT ====================================
class OcFLT : public Opcode {
public:
        OcFLT(NMFlt *, Metodo *);
        ~OcFLT();

        NMFlt *getMnemo() const       { return static_cast<NMFlt *>( mnemo ); }

	void preparaReferencias() {}
        void ejecuta();

	OcFLT * copia(Metodo * met);
};

// ================================== OcINT ====================================
class OcINT : public Opcode {
public:
        OcINT(NMInt *, Metodo *);
        ~OcINT();

        NMInt *getMnemo() const       { return static_cast<NMInt *>( mnemo ); }

	void preparaReferencias() {}
        void ejecuta();

	OcINT * copia(Metodo * met);
};

// ================================== OcTRW ====================================
class OcTRW : public Opcode {
public:
        OcTRW(NMTrw *, Metodo *);
        ~OcTRW();

        NMTrw *getMnemo() const       { return static_cast<NMTrw *>( mnemo ); }

	void preparaReferencias() {}
        void ejecuta();

	OcTRW * copia(Metodo * met);
};

// ================================== OcIOF ====================================
class OcIOF : public Opcode {
        Referencia *reg;
        Referencia *ref;
public:
        OcIOF(NMIof *, Metodo *);
        ~OcIOF();

        NMIof *getMnemo() const       { return static_cast<NMIof *>( mnemo ); }

        void preparaReferencias();

        void ejecuta();

	OcIOF * copia(Metodo * met);
};

}

#endif

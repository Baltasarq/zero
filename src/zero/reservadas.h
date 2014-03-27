// reservadas.h
/*
        Declaración de las palabras reservadas en el sistema Zero

        Pertenece a zvm, za, zd

        jbgarcia@uvigo.es
*/

#ifndef RESERVADAS_H
#define RESERVADAS_H

#include <string>

namespace Zero {

// Palabras reservadas ---------------------------------------------------------

// Variables locales -----------------------------------------------------------
extern const std::string LOC_ACC;
extern const std::string LOC_THIS;
extern const std::string LOC_RR;
extern const std::string LOC_EXC;
extern const std::string LOC_GP1;
extern const std::string LOC_GP2;
extern const std::string LOC_GP3;
extern const std::string LOC_GP4;
extern const std::string LOC_ERROR;
extern const std::string LOC_LIT;


// Atributos -------------------------------------------------------------------
extern const std::string ATR_PARENT;
extern const std::string ATR_SYSINFOATR;
extern const std::string ATR_SYSINFOMTH;
extern const std::string ATR_SYSCADENAINTERNA;
extern const std::string ATR_SYSREALINTERNO;
extern const std::string ATR_SYSINTINTERNO;
extern const std::string ATR_SYSVECTINTERNO;
extern const std::string ATR_SYSTRUEINTERNO;
extern const std::string ATR_SYSFALSEINTERNO;
extern const std::string ATR_SYSEXCEPCIONINTERNA;
extern const std::string ATR_SYSEXCEPIONMISMATCH;
extern const std::string ATR_SYSEXCEPIONMATES;
extern const std::string ATR_SYSEPRV;
extern const std::string ATR_SYSEOBJ;
extern const std::string ATR_SYSEMTH;
extern const std::string ATR_SYSFECHA;
extern const std::string ATR_SYSPRP;
extern const std::string ATR_OWNER;
extern const std::string ATR_THISCONT;
extern const std::string ATR_PS;
extern const std::string ATR_EXCTHROWER;
extern const std::string ATR_EXCMETODO;
extern const std::string ATR_EXCOBJETO;
extern const std::string ATR_EXCDETALLES;

// Métodos ---------------------------------------------------------------------
extern const std::string MET_ALCARGAR;
extern const std::string MET_COPY;
extern const std::string MET_CONSTRUCTOR;
extern const std::string MET_DESTRUCTOR;
extern const std::string MET_HAZLO;
extern const std::string MET_IQUALQUE;
extern const std::string MET_TOSTRING;
extern const std::string MET_HERDINAMICA;
extern const std::string MET_INSERTAR;
extern const std::string MET_ZERO;

// Acceso a miembros -----------------------------------------------------------
extern const std::string ACC_PRIVADO;
extern const std::string ACC_PUBLICO;

// Objetos ---------------------------------------------------------------------
extern const std::string OBJ_CONTAINER;
extern const std::string OBJ_ROOT;
extern const std::string OBJ_PS;
extern const std::string OBJ_OBJECT;
extern const std::string OBJ_EXE;
extern const std::string OBJ_INTZERO;
extern const std::string OBJ_INTSTDLIB;
extern const std::string OBJ_TOPLEVEL;
extern const std::string OBJ_NOTHING;
extern const std::string OBJ_VM;
extern const std::string OBJ_CONSOLE;
extern const std::string OBJ_STACK;
extern const std::string OBJ_STRING;
extern const std::string OBJ_FLOAT;
extern const std::string OBJ_INT;
extern const std::string OBJ_VECTOR;
extern const std::string OBJ_SYSTEM;
extern const std::string OBJ_STRINGZERO;
extern const std::string OBJ_FLOATZERO;
extern const std::string OBJ_VECTOR;
extern const std::string OBJ_RESEXE;
extern const std::string OBJ_VECTORAUX;

extern const std::string OBJ_PADRE_EXCEPCIONES;
extern const std::string OBJ_PADRE_VECTORES;
extern const std::string OBJ_PADRE_ENTEROS;
extern const std::string OBJ_PADRE_FLOTANTES;
extern const std::string OBJ_PADRE_CADENAS;

// Argumentos ------------------------------------------------------------------
extern const std::string ARG_ARG;
extern const std::string ARG_X;
extern const std::string ARG_Y;
extern const std::string ARG_N;
extern const std::string ARG_N1;
extern const std::string ARG_N2;
extern const std::string ARG_ARG1;
extern const std::string ARG_ARG2;
extern const std::string ARG_ARG3;
extern const std::string ARG_ARG4;

// Excepciones -----------------------------------------------------------------
extern const std::string EXC_SYSOBJNOENCONTRADOS;
extern const std::string EXC_FALTANATRS;
extern const std::string EXC_NOCOND;
extern const std::string EXC_NOMEM;
extern const std::string EXC_CPYERR;
extern const std::string EXC_DIVZERO;
extern const std::string EXC_CONENABLED;
extern const std::string EXC_CONDISPLAY;
extern const std::string EXC_NOCADENA;
extern const std::string EXC_NONUMERO;
extern const std::string EXC_NOVECTOR;
extern const std::string EXC_NOMETODO;
extern const std::string EXC_NOATR;
extern const std::string EXC_ARGNUMBER;
extern const std::string EXC_TRANSIENT;
extern const std::string EXC_SNDMSG;
extern const std::string EXC_OBJNOTFOUND;
extern const std::string EXC_MTHNOTFOUND;
extern const std::string EXC_FUERADERANGO;
extern const std::string EXC_YAEXISTE;
extern const std::string EXC_NOVISIBLE;
extern const std::string EXC_REC;
extern const std::string EXC_NOCONTAINER;
extern const std::string EXC_NOID;

// Otros -----------------------------------------------------------------------
extern const std::string NOMBRE_DIR_PS;
extern const std::string EXT_ARCHIVOS_ZERO;
extern const std::string EXT_ARCHIVOS_TEMP_ZERO;
extern const char CHR_SEP_IDENTIFICADOR;
extern const char CHR_ESC_STR;
extern const char CHR_MET_SUPER;

}

#endif

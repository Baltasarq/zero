// reservadas.cpp
/*
        Palabras reservadas en el sistema Zero.

        Este fichero pertenece a zvm, za y zd  ... y otros ...
        todos aquellos que necesiten hacer referencia a los identificadores
        reservados internos de Zero

        jbgarcia@uvigo.es
*/

#include "reservadas.h"

// Palabras reservadas ---------------------------------------------------------

// Variables locales -----------------------------------------------------------
const std::string Zero::LOC_ACC        = "__acc";   /// El acumulador en los mths
const std::string Zero::LOC_THIS       = "__this";  /// Referencia al objeto en los mths
const std::string Zero::LOC_RR         = "__rr";    /// La referencia a devolver en un mth.
const std::string Zero::LOC_EXC        = "__exc";   /// El objeto excep. lanzado
const std::string Zero::LOC_GP1        = "__gp1";   /// Registro de propósito general #1
const std::string Zero::LOC_GP2        = "__gp2";   /// Registro de propósito general #2
const std::string Zero::LOC_GP3        = "__gp3";   /// Registro de propósito general #3
const std::string Zero::LOC_GP4        = "__gp4";   /// Registro de propósito general #4
const std::string Zero::LOC_ERROR      = "<__err>"; /// Es el registro por defecto
const std::string Zero::LOC_LIT        = "__lit__"; /// Prefijo para literales locales mth

// Atributos -------------------------------------------------------------------
const std::string Zero::ATR_PARENT              = "parent";                // Delegación
const std::string Zero::ATR_SYSINFOATR          = "internalInfoAtr";
const std::string Zero::ATR_SYSINFOMTH          = "internalInfoMth";
const std::string Zero::ATR_SYSCADENAINTERNA    = "internalString";
const std::string Zero::ATR_SYSEXCEPIONMISMATCH = "internalEMismatch";
const std::string Zero::ATR_SYSEXCEPIONMATES    = "internalEMath";
const std::string Zero::ATR_SYSREALINTERNO      = "internalFloat";
const std::string Zero::ATR_SYSINTINTERNO       = "internalInt";
const std::string Zero::ATR_SYSVECTINTERNO      = "internalVector";
const std::string Zero::ATR_SYSTRUEINTERNO      = "internalTrue";
const std::string Zero::ATR_SYSFALSEINTERNO     = "internalFalse";
const std::string Zero::ATR_SYSEXCEPCIONINTERNA = "internalException";
const std::string Zero::ATR_SYSEOBJ             = "internalEObjNotFound";
const std::string Zero::ATR_SYSEMTH             = "internalEMthNotFound";
const std::string Zero::ATR_SYSEPRV             = "internalEPrivate";
const std::string Zero::ATR_SYSFECHA            = "internalTime";
const std::string Zero::ATR_SYSPRP              = "internalProperties";
const std::string Zero::ATR_OWNER               = "containerOwner";
const std::string Zero::ATR_THISCONT            = "containerThis";
const std::string Zero::ATR_PS                  = "psRoot";
const std::string Zero::ATR_EXCTHROWER          = "thrower";  /// Objeto lanzador excep.
const std::string Zero::ATR_EXCDETALLES         = "details";  /// Detalles de la excep.
const std::string Zero::ATR_EXCMETODO           = "method";   /// Mth en exc. donde? se produjo
const std::string Zero::ATR_EXCOBJETO           = "object";   // Obj del mth donde se produjo

// Acceso a miembros -----------------------------------------------------------
const std::string Zero::ACC_PRIVADO      = "private";  /// Atr. o mth privado
const std::string Zero::ACC_PUBLICO      = "public";   /// Atr. o mth privado

// Métodos ---------------------------------------------------------------------
const std::string Zero::MET_ALCARGAR     = "__Zero_OnRestore__"; /// Inicializar al cargar
const std::string Zero::MET_COPY         = "copy";     /// Crear nuevos objetos, por copia
const std::string Zero::MET_CONSTRUCTOR  = "onCopy";   /// Para construir objetos
const std::string Zero::MET_DESTRUCTOR   = "finalize"; /// Destructor de objetos
const std::string Zero::MET_HAZLO        = "doIt";     /// Mth "hacer()" en general
const std::string Zero::MET_IQUALQUE     = "isEqualTo";/// Mth que compara dos objetos
const std::string Zero::MET_TOSTRING     = "toString"; /// Mth que convierte a cadena
const std::string Zero::MET_HERDINAMICA  = "__dyn_inh__"; /// Mth a llamar her. din.
const std::string Zero::MET_INSERTAR     = "add";      /// Mth para insertar
const std::string Zero::MET_ZERO         = "zero";     /// Mth para pedir el dato min

// Objetos ---------------------------------------------------------------------
const std::string Zero::OBJ_CONTAINER    = "Container";/// El objeto que repre a los contenedores
const std::string Zero::OBJ_ROOT         = "Root";     /// Objeto root (por agregación)
const std::string Zero::OBJ_EXE          = "Exe";      /// Contenedor/Objeto exe
const std::string Zero::OBJ_INTSTDLIB    = "IntStdLib"; /// Contenedor/Objeto IntStdLib.
const std::string Zero::OBJ_OBJECT       = "Object";   /// Objeto root herencia
const std::string Zero::OBJ_NOTHING      = "Nothing";  /// Objeto NULL en Zero
const std::string Zero::OBJ_TOPLEVEL     = "TopLevel"; /// Objeto inicio ejec.
const std::string Zero::OBJ_VM           = "__VM__";   /// Objeto VM
const std::string Zero::OBJ_PS           = "__Zero_PersistentStore__"; /// Objeto PS
const std::string Zero::OBJ_CONSOLE      = "__Zero_Console__";     /// Consola de Zero
const std::string Zero::OBJ_STACK        = "__Zero_Stack__"; /// Objeto del stack de sistema
const std::string Zero::OBJ_STRING       = "__Zero_std::string__";      /// Gestor de cadenas
const std::string Zero::OBJ_VECTOR       = "__Zero_Vector__";/// Gestor de vectores
const std::string Zero::OBJ_INT          = "__Zero_Int__";   /// Gestor de enteros
const std::string Zero::OBJ_FLOAT        = "__Zero_Float__"; /// Gestor de flotantes
const std::string Zero::OBJ_SYSTEM       = "System";              /// Objeto de sistema
const std::string Zero::OBJ_STRINGZERO   = "__Zero_String_Zero__"; /// Cadena ""
const std::string Zero::OBJ_FLOATZERO    = "__Zero_Float_Zero__";  /// Flotante cero
const std::string Zero::OBJ_INTZERO      = "__Zero_Int_Zero__";    /// Entero cero
const std::string Zero::OBJ_RESEXE       = "__Zero_Exe_Res__"; /// Objeto resultado ejecuc.
const std::string Zero::OBJ_VECTORAUX    = "__vectorAux_aC"; /// Vector aux alCargar

const std::string Zero::OBJ_PADRE_EXCEPCIONES = "Exception";
const std::string Zero::OBJ_PADRE_VECTORES = "Vector";
const std::string Zero::OBJ_PADRE_ENTEROS  = "Int";
const std::string Zero::OBJ_PADRE_FLOTANTES = "Float";
const std::string Zero::OBJ_PADRE_CADENAS = "String";

// Argumentos ------------------------------------------------------------------
const std::string Zero::ARG_ARG          = "obj";
const std::string Zero::ARG_ARG1         = "obj1";
const std::string Zero::ARG_ARG2         = "obj2";
const std::string Zero::ARG_ARG3         = "obj3";
const std::string Zero::ARG_ARG4         = "obj4";
const std::string Zero::ARG_X            = "x";
const std::string Zero::ARG_Y            = "y";
const std::string Zero::ARG_N            = "n";
const std::string Zero::ARG_N1           = "begin";
const std::string Zero::ARG_N2           = "end";

// Excepciones -----------------------------------------------------------------
const std::string Zero::EXC_SYSOBJNOENCONTRADOS = "System objects not found in standard library";
const std::string Zero::EXC_FALTANATRS   = "Attribute not found in System object";
const std::string Zero::EXC_NOCOND       = "Conditional expected";
const std::string Zero::EXC_CPYERR       = "Object copy error";
const std::string Zero::EXC_NOMEM        = "Memory exhausted";
const std::string Zero::EXC_DIVZERO      = "Division by zero";
const std::string Zero::EXC_CONENABLED   = "Console not enabled";
const std::string Zero::EXC_CONDISPLAY   = "Tried to print something not a std::string";
const std::string Zero::EXC_NONUMERO     = "Is not a number";
const std::string Zero::EXC_NOCADENA     = "Is not text";
const std::string Zero::EXC_NOVECTOR     = "Is not a vector";
const std::string Zero::EXC_NOMETODO     = "Not a method";
const std::string Zero::EXC_NOATR        = "Not an attribute";
const std::string Zero::EXC_ARGNUMBER    = "Formal and actual arguments do not match calling ";
const std::string Zero::EXC_TRANSIENT    = "object is transient, no saving possible";
const std::string Zero::EXC_SNDMSG       = "Sending message ";
const std::string Zero::EXC_OBJNOTFOUND  = "Object not found";
const std::string Zero::EXC_MTHNOTFOUND  = "Method not found";
const std::string Zero::EXC_FUERADERANGO = "not a valid index";
const std::string Zero::EXC_YAEXISTE     = "already exists";
const std::string Zero::EXC_NOVISIBLE    = "not visible: '";
const std::string Zero::EXC_REC          = "Recursive call on method being executed";
const std::string Zero::EXC_NOCONTAINER  = "Not a container";
const std::string Zero::EXC_NOID         = "Not a valid identifier";

// Otros -----------------------------------------------------------------------
const std::string Zero::NOMBRE_DIR_PS          = "ZeroPS";
const std::string Zero::EXT_ARCHIVOS_ZERO      = ".zbj";
const std::string Zero::EXT_ARCHIVOS_TEMP_ZERO = ".zmp";
const char Zero::CHR_SEP_IDENTIFICADOR    = '.';
const char Zero::CHR_ESC_STR              = '\\';
const char Zero::CHR_MET_SUPER            = '^';


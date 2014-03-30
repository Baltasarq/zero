// zd.h
/*
      Desensamblador de ficheros de ensamblador Zero
*/

#ifndef ZD_H
#define ZD_H

#define UI_TEXT_MAIN      // Provee de una interfaz de texto, con main()

#include <string>
#include <sstream>

#include "mediosoporte.h"
#include "ident.h"

// ------------------------------------------------------------------ DesensamZ
class DesensamZ {
      private:
            Zero::MedioSoporte *ent;
            std::string nf;
            bool embellecedor;
            std::ostringstream listado;
            bool legal;
            bool fmtXml;

            void generarListado();

      public:
	    Zero::IdsPorPrograma idsProg;

        DesensamZ(Zero::MedioSoporte *f, const std::string &nomf, bool xml = false,
                  bool bello = false);
        ~DesensamZ();

        std::string getListado() const { return listado.str(); };
        bool esZeroLegal() const { return legal; }
};

#endif

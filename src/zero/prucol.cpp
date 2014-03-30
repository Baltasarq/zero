// pruCol.cpp
/*
        Comprueba que la clase colección funcione bien.
*/

#include <string>
#include <iostream>
#include <cstdlib>
#include "coleccion.h"

using namespace std;

class Personita {
private:
        string nombre;
        unsigned int edad;
public:
        Personita(const string &n, unsigned int e) : nombre(n), edad(e) {}
        const string &getNombre() const { return nombre; }
        int getEdad() const { return edad; }
};

int main()
{
		string s;
        Personita *p;
        AsocNombrePuntero<Personita> cont;
		char nombre[2];

		*( nombre + 1 ) = 0;

        for(unsigned int i = 0; i < 100000; ++i)
        {
				cout << "\nNuevo nombre ('' - termina): ";
				getline( cin, s );

			
				if ( s.empty() ) {
					break;
				}

/*				*nombre =  'A' + ( rand() % 26 );
                p = new Personita( nombre, rand() % 100 );
*/
                p = new Personita( s, rand() % 100 );
                cont.inserta( p->getNombre(), p );
        }

        for(unsigned int n=0;n<cont.getNumero(); ++n)
        {
                cout << cont.getElementoNumero(n)->getNombre() << endl
                     << cont.getElementoNumero(n)->getEdad()   << endl
                     << endl;
        }

        do {
            cout << "Busca por nombre ('' - termina): " << endl;
            getline( cin, s );

			if ( s.empty() ) {
				break;
			}

            p = cont.busca( s );
            if (p != NULL)
            {
                    cout << p->getNombre() << endl
                         << p->getEdad() << endl << endl;
            }
        } while( true );

/*	for(int n = 0; n < 1000000; ++n) {
				*nombre =  'A' + ( rand() % 26 );
				
				cont.busca( nombre );		
	}
*/
}


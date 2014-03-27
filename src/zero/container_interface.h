#ifndef __CONTAINER_INTERFACE_H
#define __CONTAINER_INTERFACE_H

class ContainerInterface
{
public:
    ContainerInterface() {}
    virtual ~ContainerInterface() {}

    virtual Atributo* buscaObjeto(const std::string &) = 0;
    virtual Objeto* creaObjeto(std::string n, Objeto* p) = 0;
    virtual void eliminarObjeto(Objeto*) = 0;
    virtual Objeto* insertaObjeto(Objeto*) = 0;

private:
    ContainerInterface( const ContainerInterface& source );
    void operator = ( const ContainerInterface& source );
};


#endif // __CONTAINER_INTERFACE_H

#include <iostream>
#include <fstream>
#include <string>

using namespace std;


int main()
{
    string nombre="infoServicios.bin";
    
    string servicio;
    ifstream archi("../Server/infoServicios.bin",ios::binary);
    //fstream fichero("infoServicios.bin", ios::binary|ios::out|ios::app);
    //archi.open("../Server/infoServicios.bin",ios::in);
    string prueba;
    //getline(archi, prueba);
    archi.read((char*)&servicio, sizeof(string));
    archi.close();
    cout << "EL SErvicio ES : " << servicio << endl;
    system("pause");
    
    /*if (archi.fail()) {
        cout << "No se puede abrir el archivo";

    }
    else {
        
    }
    

    return 0;
    /*if (fichero.good())
        cout << "archivo encontrado" << endl;
    else
    {
        cout << "el archivo no existe";
    }*/
    return 0;

}

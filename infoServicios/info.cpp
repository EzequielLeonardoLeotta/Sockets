#include <iostream>
#include <fstream>
#include <string>

using namespace std;


int main()
{
    string nombre="infoServicios.bin";
    
    int e;
    float f;
    ifstream archi("prueba.bin",ios::binary);
    //fstream fichero("infoServicios.bin", ios::binary);
    archi.read((char*)&e, sizeof(int));
    archi.read((char*)&f, sizeof(float));
    cout << "EL NUMERO ES : " << e << endl;
    cout << "EL char ES : " << f << endl;

    archi.close();
    return 0;
    /*if (fichero.good())
        cout << "archivo encontrado" << endl;
    else
    {
        cout << "el archivo no existe";
    }*/

}

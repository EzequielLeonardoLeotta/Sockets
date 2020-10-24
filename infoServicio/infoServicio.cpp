#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
typedef unsigned char BYTE;


using namespace std;


vector<BYTE> readFile(const char* filename)


{
    // Abro el archivo:
    streampos fileSize;
    ifstream file(filename, std::ios::binary);

    // consigue su tamaño
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Lee la data del archivo:
    vector<BYTE> fileData(fileSize);
    file.read((char*)&fileData[0], fileSize);
    return fileData;
}
int main()
{
    char buffer[100];
         
    vector<BYTE> fileData = readFile("../Server/infoServicios.bin");
    
    //Array con la data del archivo 
    for (int i = 0; i < fileData.size();i++) {   
        cout << fileData[i];
    }
    return 0;

}
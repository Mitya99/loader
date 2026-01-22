#include "Loader.h"

int main()
{
    //SetConsoleOutputCP(CP_UTF8);
    std::setlocale(LC_ALL, "ru_RU.UTF-8");

    //std::cout << "Press \"Enter\" to start ..." << std::flush;
    //std::cin.get();

    // !ОБРАБОТКА!
    std::string FName = "Voina_i_mir.txt";
    bool flag = onApiInput(FName.data(), nullptr);

    // выводим «Press any key to continue...»
    system("pause");   
 }
 

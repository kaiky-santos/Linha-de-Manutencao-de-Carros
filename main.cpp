#include <iostream>
#include "Fabrica.hpp"
 
int main() {
    int totalRobos, totalVeiculos;
    std::cin >> totalRobos >> totalVeiculos;
    std::cin.ignore();
 
    Fabrica fabrica(totalRobos, totalVeiculos);
 
    std::string linha;
    while (std::getline(std::cin, linha)) {
        if (linha == "FIM") break;
 
        int pos = linha.find(' ');
        int veiculo = std::stoi(linha.substr(0, pos));
        std::string tarefa = linha.substr(pos + 1);
 
        if (tarefa == "fim") {
            fabrica.encerrarVeiculo(veiculo);
        } else {
            fabrica.atenderTarefa(veiculo, tarefa);
        }
    }
    return 0;
}
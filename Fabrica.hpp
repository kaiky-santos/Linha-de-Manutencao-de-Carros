#pragma once
 
#include <iostream>
#include <unordered_map>
#include <string>
#include <list>
#include <queue>
#include <vector>
#include <utility>
 
class Atendimento final {
public:
    Atendimento(unsigned int roboId, unsigned int veiculoId);
    ~Atendimento();
    void prepararTarefa(const std::string &tarefa) const;
 
private:
    pid_t pid{-1};
    int fd[2]{-1, -1};
    unsigned int roboId;
    unsigned int veiculoId;
    void iniciar();
};
 
class Robo {
public:
    explicit Robo(unsigned int id);
    void iniciarAtendimento(unsigned int veiculo);
    void prepararTarefa(const std::string &tarefa);
    void encerrarAtendimento();
 
private:
    const unsigned int id;
    Atendimento *atendimento;
};
 
class Fabrica {
private:
    std::vector<Robo> robos;
    std::list<Robo*> disponiveis;
    std::unordered_map<int, Robo*> veiculoParaRobo;
    std::unordered_map<int, std::queue<std::string>> tarefaPorVeiculo;
    std::queue<int> veiculoEmEspera;
    int totalVeiculos;
 
public:
    Fabrica(int totalRobos, int totalVeiculos);
    void atenderTarefa(int veiculo, std::string &descricaoTarefa);
    void encerrarVeiculo(int veiculo);
    void consumirFilaTarefas(Robo* robo);
};
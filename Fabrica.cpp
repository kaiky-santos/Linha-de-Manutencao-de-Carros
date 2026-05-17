#include "Fabrica.hpp"
 
#include <iostream>
#include <csignal>
#include <unistd.h>
#include <fstream>
#include <sys/wait.h>
#include <cstring>
 
Atendimento::Atendimento(unsigned int roboId, unsigned int veiculoId) {
    this->roboId = roboId;
    this->veiculoId = veiculoId;
 
    if (pipe(fd) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    pid = fork();
 
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
 
    if (pid == 0) {
        iniciar();
        _exit(0);
    } else {
        close(fd[0]);
    }
}
 
Atendimento::~Atendimento() {
    if (pid > 0) {
        close(fd[1]);
        waitpid(pid, nullptr, 0);
    }
}
 
void Atendimento::prepararTarefa(const std::string &tarefa) const {
    std::string tarefaFormatada = tarefa + "\n";
    write(fd[1], tarefaFormatada.c_str(), tarefaFormatada.size());
}
 
void Atendimento::iniciar() {
    close(fd[1]);
 
    // verifica se arquivo ja existe para escrever o cabecalho
    std::string nomeArquivo = "Robo_" + std::to_string(roboId) + ".txt";
    std::ifstream teste(nomeArquivo);
    bool arquivoNovo = !teste.good();
    teste.close();
 
    std::ofstream arquivo(nomeArquivo, std::ios::app);
    if (arquivoNovo) {
        arquivo << "Robo_" << roboId << "\n";
    }
    arquivo << "\nVeiculo " << veiculoId << ":\n";
 
    char buffer[256];
    while (true) {
        ssize_t n = read(fd[0], buffer, sizeof(buffer) - 1);
        if (n <= 0) break;
        buffer[n] = '\0';
 
        char* token = strtok(buffer, "\n");
        while (token != nullptr) {
            std::string msg(token);
            if (msg == "fim") {
                arquivo.close();
                close(fd[0]);
                return;
            }
            arquivo << "- " << msg << "\n";
            token = strtok(nullptr, "\n");
        }
        arquivo.flush();
    }
 
    arquivo.close();
    close(fd[0]);
}
 
Robo::Robo(unsigned int id) : id(id), atendimento(nullptr) {}
 
void Robo::iniciarAtendimento(const unsigned int veiculo) {
    atendimento = new Atendimento(id, veiculo);
}
 
void Robo::prepararTarefa(const std::string &tarefa) {
    atendimento->prepararTarefa(tarefa);
}
 
void Robo::encerrarAtendimento() {
    delete atendimento;
    atendimento = nullptr;
}
 
Fabrica::Fabrica(int totalRobos, int totalVeiculos) {
    this->totalVeiculos = totalVeiculos;
    robos.reserve(totalRobos);
    for (int i = 1; i <= totalRobos; i++) {
        robos.push_back(Robo(i));
        disponiveis.push_back(&robos.back());
    }
}
 
void Fabrica::atenderTarefa(int veiculo, std::string &tarefa) {
    if (veiculoParaRobo.count(veiculo)) {
        veiculoParaRobo[veiculo]->prepararTarefa(tarefa);
    } else if (!disponiveis.empty()) {
        Robo* robo = disponiveis.front();
        veiculoParaRobo[veiculo] = robo;
        disponiveis.pop_front();
        robo->iniciarAtendimento(veiculo);
        robo->prepararTarefa(tarefa);
    } else {
        if (!tarefaPorVeiculo.count(veiculo)) {
            veiculoEmEspera.push(veiculo);
        }
        tarefaPorVeiculo[veiculo].push(tarefa);
    }
}
 
void Fabrica::encerrarVeiculo(int veiculo) {
    if (!veiculoParaRobo.count(veiculo)) {
        if (!tarefaPorVeiculo.count(veiculo)) {
            veiculoEmEspera.push(veiculo);
        }
        tarefaPorVeiculo[veiculo].push("fim");
        return;
    }
 
    Robo* robo = veiculoParaRobo[veiculo];
    robo->prepararTarefa("fim");
    robo->encerrarAtendimento();
    veiculoParaRobo.erase(veiculo);
 
    if (veiculoEmEspera.empty()) {
        disponiveis.push_back(robo);
    } else {
        consumirFilaTarefas(robo);
    }
}
 
void Fabrica::consumirFilaTarefas(Robo* robo) {
    if (veiculoEmEspera.empty()) {
        disponiveis.push_back(robo);
        return;
    }
 
    int veiculo = veiculoEmEspera.front();
    veiculoEmEspera.pop();
 
    auto it = tarefaPorVeiculo.find(veiculo);
    if (it == tarefaPorVeiculo.end()) {
        return;
    }
 
    veiculoParaRobo[veiculo] = robo;
    robo->iniciarAtendimento(veiculo);
 
    while (!it->second.empty()) {
        std::string tarefa = it->second.front();
        it->second.pop();
 
        if (tarefa == "fim") {
            robo->prepararTarefa("fim");
            robo->encerrarAtendimento();
            veiculoParaRobo.erase(veiculo);
            tarefaPorVeiculo.erase(it);
 
            if (veiculoEmEspera.empty()) {
                disponiveis.push_back(robo);
            } else {
                consumirFilaTarefas(robo);
            }
            return;
        } else {
            robo->prepararTarefa(tarefa);
        }
    }
}
#pragma once

#include <unistd.h>
#include <cstdint>
#include <csignal>
#include <atomic>

#define True true
#define False false

///< Variáveis que serão amplamente utilizadas.
inline constexpr const char* AGENT_HOST = "localhost";
inline constexpr int AGENT_PORT = 3100;
inline constexpr const char* TEAM_NAME = "RoboIME";
inline constexpr bool DEBUG_MODE = False;

///< Para tratarmos o encerramento brusco.
std::atomic<bool> is_running(True);

bool see_only_when_i_want = false;

/**
 * @brief Exclusivo para fazermos o encerramento do socket de forma robusta.
 */
void ender(int sinal){ if(sinal == SIGINT){ is_running = False; }}
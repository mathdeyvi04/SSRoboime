#pragma once

#include "../logger/Logger.hpp"
#include <iostream>
#include <string_view>
#include <charconv> ///< std::from_chars

#define True true
#define False false

/**
 * @brief Responsável por representar o ambiente externo ao robô
 * @details
 * Agrupará todos os métodos de interpretação do mundo.
 * Focaremos em performance e eficiência no uso da memória.
 *
 */
class Environment {
public:

    Logger& logger;

    /**
     * @brief Construtor da Classe
     */
    Environment(
        Logger& logger
    ) : logger(logger) {}

    /* Atributos Públicos de Ambiente */
    float time_server; ///< Instante de Tempo do Servidor, útil apenas para sincronização entre agentes
    float time_match;  ///< Instante de Tempo de Partida
    int goals_scored;  ///< Nossos Gols, pode ser útil para mudarmos de tática conforme o jogo avança
    int goals_conceded;  ///< Gols adversários, pode ser útil para mudarmos de tática conforme o jogo avança

    /**
     * @brief Apresentará os dados lidos do servidor
     */
    void
    print_status() const {
        printf("\n=== Environment State ===\n");
        printf("time_server    : %.3f\n", time_server);
        printf("time_match     : %.3f\n", time_match);
        printf("goals_scored   : %d\n", goals_scored);
        printf("goals_conceded : %d\n", goals_conceded);
    }

    enum class PlayMode : int {
        // Ao nosso favor
        OUR_KICKOFF = 0,
        OUR_KICK_IN = 1,
        OUR_CORNER_KICK = 2,
        OUR_GOAL_KICK = 3,
        OUR_FREE_KICK = 4,
        OUR_PASS = 5,
        OUR_DIR_FREE_KICK = 6,
        OUR_GOAL = 7,
        OUR_OFFSIDE = 8,

        // Ao favor deles
        THEIR_KICKOFF = 9,
        THEIR_KICK_IN = 10,
        THEIR_CORNER_KICK = 11,
        THEIR_GOAL_KICK = 12,
        THEIR_FREE_KICK = 13,
        THEIR_PASS = 14,
        THEIR_DIR_FREE_KICK = 15,
        THEIR_GOAL = 16,
        THEIR_OFFSIDE = 17,

        // Neutros
        BEFORE_KICKOFF = 18,
        GAME_OVER = 19,
        PLAY_ON = 20
    }; ///< Servidor nos fornecerá isso
    enum class PlayModeGroup : int {
        OUR_KICK = 0,
        THEIR_KICK = 1,
        ACTIVE_BEAM = 2,
        PASSIVE_BEAM = 3,
        OTHER = 4
    }; ///< Agente precisará de uma informação mais geral para tomada de decisões

    /* Métodos Inerentes a Execução da Aplicação */





    /* ------------------ Parser de Mensagem do Servidor ----------------------- */

    /**
     * @brief Responsável por prover ferramentas de auxílio de parsing.
     * @details
     * Centralizará todas as funções inerentes ao parsing das mensagens.
     */
    class Parsing {
    private:
        const char* buffer = nullptr; ///< Permitirá-nos saber o ponto da mensagem que estamos
        const char* end    = nullptr; ///< Permitirá-nos saber o ponto final
        Environment* env   = nullptr; ///< Permitirá-nos modificar atributos

    public:
        /* Métodos Simples de Cursor */

        /**
         * @brief Construtor do Parsing dedica à interpretação.
         * @param msg Mensagem bruta enviada pelo servidor.
         * @return Atualização de todas as variáveis de ambiente.
         */
        Parsing(
            std::string_view& message,
            Environment* env
        ) :
            buffer(message.data()),
            end(message.data() + message.size()),
            env(env)
        {}

        /**
         * @brief Avançará até encontrar um determinado caractere de parada, pulando-o em seguida.
         * @param caract Caractere de Parada.
         * @return True, caso encontre corretamente. False, caso chegue ao final da mensagem.
         */
        bool
        skip_until_char(char caract){
            while(*this->buffer != caract){
                if(this->buffer > this->end){ return False; }
                this->buffer++;
            }
            this->buffer++;
            return True;
        }

        /**
         * @brief Ignorando eventuais ' ', '(' e ')', obterá a próxima string, encerrando apenas a encontrar ' '. Pulando este último caractere.
         * @return String_view da string.
         */
        std::string_view
        get_str(){
            while(*this->buffer == ' ' || *this->buffer == '(' || *this->buffer == ')'){ this->buffer++; }
            const char* value_start = this->buffer;
            while(*this->buffer != ' ' && *this->buffer != ')'){ this->buffer++; }
            return std::string_view(value_start, size_t(this->buffer++ - value_start));
        }

        /**
         * @brief Fará a conversão de caracteres em inteiro ou float, dependendo do tipo de referência dado.
         * @details
         * Iniciará a leitura a partir do ponto que buffer se encontra. Encerrará ao encontrar ' ' ou ')', pulando este.
         * @param [out] Variável que receberá o valor
         * @return True, se não houve erro. False, caso contrário.
         */
        template<typename T>
        bool
        get_value(T& out){
            const char* value_start = this->buffer;
            while(*this->buffer != ' ' && *this->buffer != ')'){ this->buffer++; }
            return std::from_chars(value_start, this->buffer++, out).ec == std::errc{};
        }

        /**
         * @brief Avançará o cursor uma determinada quantidade.
         * @param n quantidade de avanços desejados
         * @return False, se o avanço não foi permitido. True, caso contrário.
         */
        bool
        advance(int n = 1){ if((this->buffer + n) > this->end){ return False; } this->buffer += n; return True; }

        /* -- Métodos de Parsing -- */

        /**
         * @brief Responsável pela interpretação da mensagem de 'time'.
         */
        void
        parse_time(){

            /*
            Buffer está aqui.
              |
              v
            ' (now 10.03)'
            */
            this->advance(5); ///< Vamos ter fé que nunca será diferente.
            this->get_value(env->time_server);
            this->advance(); ///< Sairemos da tag 'time'
        }

        /**
         * @brief Responsável pela interpretação da mensagem de 'GS'.
         */
        void
        parse_gamestate(){

            std::string_view lower_tag;
            while(True){
                lower_tag = this->get_str(); ///< Obteremos as subtags

                switch(lower_tag[0]){

                    case 's': ///< Poderá ser 'sl', 'sr'
                        this->get_value( (lower_tag[1] == 'l') ? env->goals_scored : env->goals_conceded );
                        break;

                    case 'p': ///< Há apenas 'pm'
                        // Não sabemos que lado estamos! Isso fará muita diferença!
                        /*
                        Segundo o repositório muleta, haverá uma nova lower_tag, denominada team ou outra coisa.
                        A partir dela identificaremos se estamos no lado esquerdo ou direito.
                        Minha sugestão é primeiro encerrar aqui esse desenvolvimento e iniciar o desenvolvimento do Logger, também em C++.
                        */
                        break;

                    case 't': ///< Há apenas 't'
                        this->get_value(env->time_match);

                    default:
                        break;
                }

                if(*this->buffer == ')'){ break; } ///< Se após encontrarmos um ')' houver outro ')', então chegamos ao final da lower_tag.
            }

        }
    };

    /**
     * @brief Interpretará as mensagens do servidor.
     * @param msg Mensagem bruta enviada pelo servidor.
     * @return Atualização de todas as variáveis de ambiente.
     */
    int
    update_from_server(
        std::string_view msg
    ){

        Parsing cursor(msg, this);
        std::string_view upper_tag;
        while(True){

            if(
                !cursor.skip_until_char('(')
            ){ this->print_status(); return 0; }

            upper_tag = cursor.get_str(); ///< Vamos extrair uma tag
            switch(upper_tag[0]){
                case 't': ///< Há apenas 'time'
                    cursor.parse_time();
                    break;

                case 'G': ///< Pode ser 'GS' ou 'GYR'
                    if(upper_tag[1] == 'S'){
                        cursor.parse_gamestate();
                    }
                    else if(upper_tag[1] == 'Y'){

                    }
                    else{
                        ///< Tag Desconhecida
                    }

                    break;

                case 'S':
                    break;

                case 'H':
                    break;

                case 'A':
                    break;

                default:
                    ///< Tag Superior Desconhecida
                    break;
            }
        }
    }
};





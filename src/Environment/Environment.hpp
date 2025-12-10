#pragma once

#include "../Booting/booting_templates.hpp"
#include "../Logger/Logger.hpp"
#include <iostream>
#include <string_view>
#include <charconv> // std::from_chars
#include <unordered_map>

/**
 * @class Environment
 * @brief Responsável por representar o ambiente externo ao robô.
 * @details
 * Focaremos em performance (uso de std::string_view e ponteiros) e eficiência no uso da memória.
 * Esta classe mantém o estado atual do jogo conforme percebido pelo agente.
 */
class Environment {
public:

    /**
     * @brief Referência ao sistema de log para debug e avisos.
     */
    Logger& logger;

    /**
     * @brief Construtor da Classe Environment.
     * @param logger Referência para a instância de Logger a ser utilizada.
     */
    Environment(
        Logger& logger
    ) : logger(logger) {}

    /* -- Definição de Ferramentas que serão amplamente Usadas -- */

    /**
     * @enum PlayMode
     * @brief Enumeração dos modos de jogo oficiais do servidor (RoboCup 3D).
     * @details Mapeado para uint8_t para economizar memória.
     */
    enum class PlayMode : uint8_t {
        // Ao nosso favor
        OUR_KICKOFF = 0,       ///< Tiro de saída nosso
        OUR_KICK_IN = 1,       ///< Lateral nosso
        OUR_CORNER_KICK = 2,   ///< Escanteio nosso
        OUR_GOAL_KICK = 3,     ///< Tiro de meta nosso
        OUR_FREE_KICK = 4,     ///< Tiro livre nosso
        OUR_PASS = 5,          ///< (Obsoleto/Específico) Passe nosso
        OUR_DIR_FREE_KICK = 6, ///< Tiro livre direto nosso
        OUR_GOAL = 7,          ///< Gol nosso (após o gol)
        OUR_OFFSIDE = 8,       ///< Impedimento nosso (cometemos)

        // Ao favor deles
        THEIR_KICKOFF = 9,       ///< Tiro de saída deles
        THEIR_KICK_IN = 10,      ///< Lateral deles
        THEIR_CORNER_KICK = 11,  ///< Escanteio deles
        THEIR_GOAL_KICK = 12,    ///< Tiro de meta deles
        THEIR_FREE_KICK = 13,    ///< Tiro livre deles
        THEIR_PASS = 14,         ///< (Obsoleto/Específico) Passe deles
        THEIR_DIR_FREE_KICK = 15,///< Tiro livre direto deles
        THEIR_GOAL = 16,         ///< Gol deles (sofremos gol)
        THEIR_OFFSIDE = 17,      ///< Impedimento deles

        // Neutros
        BEFORE_KICKOFF = 18, ///< Antes do início da partida
        GAME_OVER = 19,      ///< Fim de jogo
        PLAY_ON = 20         ///< Jogo rolando normalmente
    };

    /**
     * @enum PlayModeGroup
     * @brief Categorização de alto nível dos modos de jogo para tomada de decisão.
     */
    enum class PlayModeGroup : uint8_t {
        OUR_KICK = 0,      ///< É nossa vez de chutar parado (bola parada ativa)
        THEIR_KICK = 1,    ///< É vez deles de chutar parado (bola parada passiva)
        ACTIVE_BEAM = 2,   ///< Podemos usar o comando beam (posicionamento inicial)
        PASSIVE_BEAM = 3,  ///< Devemos esperar (beam passivo ou goleiro antes do chute)
        OTHER = 4          ///< Jogo rolando ou parado sem ação específica (PlayOn, GameOver)
    };

    /**
     * @struct Enabler_Stringview_Hash
     * @brief Functor de hash personalizado para permitir 'Heterogeneous Lookup'.
     * @details Permite buscar chaves `std::string_view` em um mapa cujas chaves são `std::string`
     * sem alocação temporária de memória.
     */
    struct Enabler_Stringview_Hash {
        using is_transparent = void; ///< Sinaliza ao unordered_map que essa struct suporta tipos heterogêneos.

        /**
         * @brief Hash para chaves do tipo std::string.
         */
        ::size_t operator()(const std::string& s) const { return std::hash<std::string>{}(s); }

        /**
         * @brief Hash para chaves de busca do tipo std::string_view.
         */
        ::size_t operator()(std::string_view sv) const { return std::hash<std::string_view>{}(sv); }
    };

    /**
     * @brief Tabela de conversão estática de strings do servidor para PlayMode.
     * @details
     * A chave é a string recebida (ex: "KickOff_Left").
     * O valor é um array com dois PlayModes: índice 0 para quando somos Left, índice 1 para quando somos Right.
     * Utiliza `inline static` (C++17) para inicialização no header.
     */
    inline static const std::unordered_map<
        std::string,
        std::array<PlayMode, 2>,
        Enabler_Stringview_Hash,
        std::equal_to<>
    > play_modes = {
        // --- Neutros (LEFT e RIGHT veem o mesmo modo) ---
        {"BeforeKickOff", {Environment::PlayMode::BEFORE_KICKOFF, Environment::PlayMode::BEFORE_KICKOFF}},
        {"GameOver",      {Environment::PlayMode::GAME_OVER,      Environment::PlayMode::GAME_OVER}},
        {"PlayOn",        {Environment::PlayMode::PLAY_ON,        Environment::PlayMode::PLAY_ON}},

        // --- LEFT Kick Events (LEFT é o nosso time, RIGHT é o time deles) ---
        {"KickOff_Left",           {Environment::PlayMode::OUR_KICKOFF,      Environment::PlayMode::THEIR_KICKOFF}},
        {"KickIn_Left",            {Environment::PlayMode::OUR_KICK_IN,      Environment::PlayMode::THEIR_KICK_IN}},
        {"corner_kick_left",       {Environment::PlayMode::OUR_CORNER_KICK,  Environment::PlayMode::THEIR_CORNER_KICK}},
        {"goal_kick_left",         {Environment::PlayMode::OUR_GOAL_KICK,    Environment::PlayMode::THEIR_GOAL_KICK}},
        {"free_kick_left",         {Environment::PlayMode::OUR_FREE_KICK,    Environment::PlayMode::THEIR_FREE_KICK}},
        {"pass_left",              {Environment::PlayMode::OUR_PASS,         Environment::PlayMode::THEIR_PASS}},
        {"direct_free_kick_left",  {Environment::PlayMode::OUR_DIR_FREE_KICK, Environment::PlayMode::THEIR_DIR_FREE_KICK}},
        {"Goal_Left",              {Environment::PlayMode::OUR_GOAL,         Environment::PlayMode::THEIR_GOAL}},
        {"offside_left",           {Environment::PlayMode::OUR_OFFSIDE,      Environment::PlayMode::THEIR_OFFSIDE}},
        // --- RIGHT Kick Events (RIGHT é o nosso time, LEFT é o time deles) ---
        {"KickOff_Right",          {Environment::PlayMode::THEIR_KICKOFF,    Environment::PlayMode::OUR_KICKOFF}},
        {"KickIn_Right",           {Environment::PlayMode::THEIR_KICK_IN,    Environment::PlayMode::OUR_KICK_IN}},
        {"corner_kick_right",      {Environment::PlayMode::THEIR_CORNER_KICK, Environment::PlayMode::OUR_CORNER_KICK}},
        {"goal_kick_right",        {Environment::PlayMode::THEIR_GOAL_KICK,  Environment::PlayMode::OUR_GOAL_KICK}},
        {"free_kick_right",        {Environment::PlayMode::THEIR_FREE_KICK,  Environment::PlayMode::OUR_FREE_KICK}},
        {"pass_right",             {Environment::PlayMode::THEIR_PASS,       Environment::PlayMode::OUR_PASS}},
        {"direct_free_kick_right", {Environment::PlayMode::THEIR_DIR_FREE_KICK, Environment::PlayMode::OUR_DIR_FREE_KICK}},
        {"Goal_Right",             {Environment::PlayMode::THEIR_GOAL,       Environment::PlayMode::OUR_GOAL}},
        {"offside_right",          {Environment::PlayMode::THEIR_OFFSIDE,    Environment::PlayMode::OUR_OFFSIDE}}
    };

    /* Atributos Públicos de Ambiente */

    float time_server;       ///< Instante de Tempo do Servidor, útil apenas para sincronização entre agentes.
    float time_match;        ///< Instante de Tempo de Partida (Game Time).
    uint8_t goals_scored;    ///< Nossos Gols marcados.
    uint8_t goals_conceded;  ///< Gols adversários sofridos.
    uint8_t unum;            ///< Número do Jogador (Uniform Number).
    bool is_left;            ///< Indica se estamos jogando no lado esquerdo do campo (true) ou direito (false).
    PlayMode current_mode;   ///< Modo de jogo atual processado para nossa perspectiva.

    /* Métodos Inerentes a Execução da Aplicação */

    /* ------------------ Parser de Mensagem do Servidor ----------------------- */

    /**
     * @class Parsing
     * @brief Responsável por prover ferramentas de auxílio de parsing.
     * @details
     * Centraliza a lógica de extração de dados da string bruta.
     * Mantém o cursor de leitura para evitar cópias desnecessárias.
     */
    class Parsing {
    private:
        const char* buffer = nullptr; ///< Ponteiro atual na string de mensagem (cursor).
        const char* end    = nullptr; ///< Ponteiro para o final da string de mensagem.
        Environment* env   = nullptr; ///< Ponteiro para o ambiente onde os dados serão salvos.

    public:
        /* Métodos Simples de Cursor */

        /**
         * @brief Construtor do Parsing dedicado à interpretação.
         * @param message Mensagem bruta (view) enviada pelo servidor.
         * @param env Ponteiro para a classe Environment que será populada.
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
         * @brief Avança o cursor até encontrar um determinado caractere, pulando-o em seguida.
         * @param caract Caractere de busca (ex: '(' ).
         * @return True se encontrou o caractere dentro dos limites, False caso chegue ao final.
         */
        bool
        skip_until_char(char caract){
            while(*this->buffer != caract){
                if(this->buffer >= this->end){ return False; }
                this->buffer++;
            }
            this->buffer++;
            return True;
        }

        /**
         * @brief Obtém a próxima substring delimitada por espaços ou parênteses.
         * @details Ignora ' ', '(' e ')' iniciais. A leitura para ao encontrar um delimitador, pulando-o em seguida.
         * @return std::string_view apontando para a string encontrada.
         */
        std::string_view
        get_str(){
            while(*this->buffer == ' ' || *this->buffer == '(' || *this->buffer == ')'){ this->buffer++; }
            const char* value_start = this->buffer;
            while(*this->buffer != ' ' && *this->buffer != ')'){ this->buffer++; }
            return std::string_view(value_start, ::size_t(this->buffer++ - value_start));
        }

        /**
         * @brief Converte a sequência de caracteres atual em um número (int ou float). Pulando o último caractere.
         * @tparam T Tipo do dado a ser extraído (int, float, uint8_t, etc).
         * @param[out] out Referência para a variável que receberá o valor.
         * @return True se a conversão foi bem-sucedida, False caso contrário.
         */
        template<typename T>
        bool
        get_value(T& out){
            const char* value_start = this->buffer;
            while(*this->buffer != ' ' && *this->buffer != ')'){ this->buffer++; }
            return std::from_chars(value_start, this->buffer++, out).ec == std::errc{};
        }

        /**
         * @brief Avança o cursor manualmente uma quantidade fixa de caracteres.
         * @param n Quantidade de bytes para avançar (padrão 1).
         * @return True se o avanço for seguro (dentro do buffer), False se ultrapassar.
         */
        bool
        advance(int n = 1){ if((this->buffer + n) > this->end){ return False; } this->buffer += n; return True; }

        /**
         * @brief Obtém um trecho da string ao redor do cursor atual para debug.
         * @details Usada somente em caso de erro, captura 30 chars antes e 30 depois.
         * @return std::string contendo o contexto do buffer.
         */
        std::string
        get(){
            return std::string(std::string_view(this->buffer - 20, 40)); ///< Vamos pegar alguns endereços antes e alguns depois.
        }

        /**
         * @brief Pula um bloco balanceado de parênteses de uma tag desconhecida.
         * @details Mantém um contador de parênteses abertos e fechados para ignorar toda a estrutura hierárquica da tag atual.
         */
        void
        skip_unknown(){
            ///< Como já iniciamos após ter visto o '('.
            uint8_t counter = 1;
            while(
                counter != 0
            ){
                counter += (*this->buffer == ')') * (- 1) + (*this->buffer == '(') * 1;
                this->buffer++;
            }
        }

        /* -- Métodos de Parsing -- */

        /**
         * @brief Interpreta a mensagem de tempo do servidor ('time').
         * @details Exemplo: (time (now 10.03)). Atualiza `env->time_server`.
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
            this->get_value(this->env->time_server);
            this->advance(); ///< Sairemos da tag 'time'
        }

        /**
         * @brief Interpreta a mensagem de GameState ('GS').
         * @details
         * Realiza o parsing de subtags como 'sl', 'sr', 'pm', 't', 'u', 'team'.
         */
        void
        parse_gamestate(){

            std::string_view lower_tag;
            while(True){
                lower_tag = this->get_str(); ///< Obteremos as subtags

                switch(lower_tag[0]){

                    case 's': { ///< Poderá ser 'sl' (score left), 'sr' (score right)
                        this->get_value( (lower_tag[1] == 'l') ? this->env->goals_scored : this->env->goals_conceded );
                        break;
                    }

                    case 'p': { ///< Há apenas 'pm' (playmode)
                        // É garantido que já tenhamos tido is_left
                        lower_tag = this->get_str();
                        auto it = play_modes.find(lower_tag);
                        if( it != play_modes.end() ){ this->env->current_mode = it->second[this->env->is_left]; }
                        break;
                    }

                    case 't': { ///< Há 't' e 'team'.
                        if(lower_tag.size() == 1){ this->get_value(this->env->time_match); } // Então é 't'
                        else if(lower_tag[1] == 'e'){ env->is_left = this->get_str()[0] == 'l'; } // Então é 'team'
                        break;
                    }

                    case 'u': { ///< Há apenas o 'u' (unum)
                        this->get_value(this->env->unum);
                        break;
                    }

                    default: {
                        this->env->logger.warn("[{}]Flag Desconhecida Encontrada em 'GS': {} \t Buffer Neste momento: {}", this->env->unum, lower_tag, this->get());
                        break;
                    }
                }

                if(*this->buffer == ')'){ break; } ///< Se após encontrarmos um ')' houver outro ')', então chegamos ao final da lower_tag.
            }
        }

        /**
         * @brief Interpreta a mensagem do Giroscópio ('GYR').
         * @details
         * Lê 3 valores float representando a velocidade angular (deg/s) nos eixos X, Y, Z.
         */
        void
        parse_gyroscope(){

            // Só há uma tag aqui. Logo, não é necessário loop e busca por tentativas.
            this->advance(14); // Colocamos 13, pois nunca se sabe se virá um '-' para nos atrapalhar.

            // Devemos usar Eigen
            float value;
            for(int i = 0; i < 3; i++){ this->get_value(value); }
        }

        /**
         * @brief Interpreta a mensagem do Acelerômetro ('ACC').
         * @details
         * Lê 3 valores float representando a aceleração linear do torso.
         */
         void
         parse_accelerometer(){

            this->advance(13);
            float value;
            for(int i = 3; i < 3; i++){ this->get_value(value); }
         }


         /**
         * @brief Interpreta a mensagem de Visão ('See').
         * @details
         * Processa informações visuais complexas, incluindo:
         * - Jogadores ('P'): time, número, partes do corpo.
         * - Bola ('B').
         * - Landmarks ('G', 'F').
         * - Linhas ('L').
         * Utiliza loops aninhados para lidar com a estrutura hierárquica da visão.
         */
         void
         parse_vision(){

            std::string_view lower_tag;
            while(True){

                lower_tag = this->get_str();

                switch(lower_tag[0]){

                    case 'P': ///< Estamos vendo um jogador. Há outras lowers tags a serem verificadas.
                        while(True){

                            lower_tag = this->get_str();

                            switch(lower_tag[0]){

                                case 't': { ///< Informação de 'team' do jogador visto
                                    this->get_str();
                                    break;
                                }

                                case 'i': { ///< Saberemos o unum do jogador visto
                                    uint8_t value;
                                    this->get_value(value);
                                    break;
                                }

                                // Após essas, qualquer informação dada será da parte do corpo dele.
                                case 'h': {

                                }
                                case 'r': {

                                }
                                case 'l': {
                                    // Vamos apenas pular as informações
                                    this->advance(5);
                                    float value;
                                    for(int i = 0; i < 3; i++){ this->get_value(value); }
                                    break;
                                }

                                default:
                                    this->env->logger.warn("[{}] Flag Desconhecida dentro de 'See:P': {}. \t Buffer Neste momento: {}", this->env->unum, lower_tag, this->buffer);
                                    break;
                            }

                            if(*this->buffer == ')'){ this->advance(1); if(*this->buffer == ')'){ break; } } ///< Se após encontrarmos um ')' houver outro ')', então chegamos ao final da lower_tag.
                        }
                        break;

                    case 'B': { ///< Obviamente, a bola.

                    }
                    // Landmarks
                    case 'G': {

                    }
                    case 'F': {
                        if(lower_tag == "F1R"){
                            this->advance(5);
                            float value;
                            for(int i = 0; i < 3; i++){
                                this->get_value(value);
                                if(i == 0 && see_only_when_i_want){
                                    printf(
                                        "\n%d-%f",
                                        this->env->unum,
                                        value
                                    );
                                    std::fflush;
                                }
                            }
                            break;
                        }
                        this->advance(5);
                        float value;
                        for(int i = 0; i < 3; i++){ this->get_value(value); }
                        break;
                    }

                    case 'L': { ///< Linhas Vistas

                        this->advance(5);
                        // Precisamos pegar ambos pontos da linha
                        float value;
                        for(int i = 0; i < 3; i++){ this->get_value(value); }

                        this->advance(6);
                        for(int i = 0; i < 3; i++){ this->get_value(value); }

                        break;
                    }

                    default:
                        this->env->logger.warn("[{}] Flag Desconhecida dentro de 'See': {}. \t Buffer Neste momento: {}", this->env->unum, lower_tag, this->buffer);
                        break;
                }

                if(*this->buffer == ')'){ this->advance(1); if(*this->buffer == ')'){ break; } } ///< Se após encontrarmos um ')' houver outro ')', então chegamos ao final da lower_tag.

            }
         }

         /**
         * @brief Interpreta a mensagem de Juntas ('HJ').
         * @details
         * Recebe o nome abreviado da junta (ex: hj1) e o ângulo atual em graus.
         */
         void
         parse_hingejoint(){

            // Dado que será sempre o mesmo padrão. É possível:
            this->advance(3);
            std::string_view nome_da_junta = this->get_str();
            this->advance(5);
            float value;
            this->get_value(value);
         }

        /**
         * @brief Interpreta a mensagem de Sensores de Força ('FRP').
         * @details
         * Sensores localizados nos pés (lf, rf).
         * Lê:
         * 1. Ponto de contato (vetor 3D) relativo ao centro do pé.
         * 2. Força total (vetor 3D) em kg*m/s^2.
         */
        void
        parse_force_resistance(){

            // Dado que será sempre o mesmo padrão, é possível:
            this->advance(3);
            this->get_str();

            this->advance(4);
            // Começamos a pegar o vetor
            float value;
            for(int i = 0; i < 3; i++){ this->get_value(value); }

            this->advance(4);
            for(int i = 0; i < 3; i++){ this->get_value(value); }
        }

        /**
         * @brief Interpreta a mensagem de Audição ('hear').
         * @details Responsável por processar mensagens de áudio (do juiz ou outros jogadores).
         */
        void
        parse_hear(){
            // sanha
        }
    };

    /**
     * @brief Responsável pela atualização do ambiente.
     * @details
     * Recebe a string bruta do servidor, instancia o parser e despacha para os métodos específicos
     * baseados nas tags de nível superior ('time', 'GS', 'See', etc).
     * @param msg Mensagem bruta (std::string_view) enviada pelo servidor.
     */
    void
    update_from_server(
        std::string_view msg
    ){

        Parsing cursor(msg, this);
        std::string_view upper_tag;
        while(True){

            if(
                !cursor.skip_until_char('(')
            ){ this->print_status(); return; }

            upper_tag = cursor.get_str(); ///< Vamos extrair uma tag
            switch(upper_tag[0]){
                case 't': { ///< Há apenas 'time'
                    cursor.parse_time();
                    break;
                }

                case 'G': { ///< Pode ser 'GS' ou 'GYR'
                    if(upper_tag[1] == 'S'){
                        cursor.parse_gamestate();
                    }
                    else if(upper_tag[1] == 'Y'){
                        cursor.parse_gyroscope();
                    }
                    else{
                        ///< Tag Desconhecida
                        this->logger.warn("[{}] Tag Superior Desconhecida: [{}]", this->unum, upper_tag);
                    }
                    break;
                }

                case 'A': {
                    if(upper_tag[1] == 'C'){ cursor.parse_accelerometer(); }
                    break;
                }

                case 'S': {
                    if(upper_tag[1] == 'e'){ cursor.parse_vision(); }
                    else{ this->logger.warn("[{}] Tag Superior Desconhecida: [{}] \t Buffer neste momento: [{}]", this->unum, upper_tag, cursor.get()); cursor.skip_unknown(); }
                    break;
                }

                case 'H': {
                    cursor.parse_hingejoint();
                    break;
                }

                case 'F': {
                    cursor.parse_force_resistance();
                    break;
                }

                default: {
                    ///< Tag Superior Desconhecida
                    this->logger.warn("[{}] Tag Superior Desconhecida: [{}] \t Buffer neste momento: [{}]", this->unum, upper_tag, cursor.get());
                    cursor.skip_unknown();
                    break;
                }
            }
        }
    }

private:

    /**
     * @brief Imprime o estado atual do ambiente no console (Debug).
     * @details Atualmente retorna imediatamente (desabilitado). Útil para verificar parsing.
     */
    void
    print_status() const {
        return;
        printf("\n=== Environment State ===\n");
        printf("time_server    : %.3f\n", time_server);
        printf("time_match     : %.3f\n", time_match);
        printf("goals_scored   : %d\n", goals_scored);
        printf("goals_conceded : %d\n", goals_conceded);
        printf("is_left        : %d\n", is_left);
        printf("playmode       : %d\n", static_cast<uint8_t>(current_mode));
    }
};
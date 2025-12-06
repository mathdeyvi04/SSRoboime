#include "Environment.hpp"

/* --- Definições de Estáticos Não Literais --- */
const std::unordered_map<
    std::string,
    std::array<Environment::PlayMode, 2>,
    Environment::Enabler_Stringview_Hash,
    std::equal_to<>
> Environment::play_modes = {
    // --- Neutros (LEFT e RIGHT veem o mesmo modo) ---
    {"BeforeKickOff", {Environment::PlayMode::BEFORE_KICKOFF, Environment::PlayMode::BEFORE_KICKOFF}},
    {"GameOver",      {Environment::PlayMode::GAME_OVER,      Environment::PlayMode::GAME_OVER}},
    {"PlayOn",        {Environment::PlayMode::PLAY_ON,        Environment::PlayMode::PLAY_ON}},

    // --- LEFT Kick Events (LEFT é o nosso time, RIGHT é o time deles) ---

    // LEFT_PLAY_MODE_TO_ID: KickOff_Left -> OUR_KICKOFF (0)
    // RIGHT_PLAY_MODE_TO_ID: KickOff_Left -> THEIR_KICKOFF (9)
    {"KickOff_Left",           {Environment::PlayMode::OUR_KICKOFF,      Environment::PlayMode::THEIR_KICKOFF}},

    // LEFT: OUR_KICK_IN (1); RIGHT: THEIR_KICK_IN (10)
    {"KickIn_Left",            {Environment::PlayMode::OUR_KICK_IN,      Environment::PlayMode::THEIR_KICK_IN}},

    // LEFT: OUR_CORNER_KICK (2); RIGHT: THEIR_CORNER_KICK (11)
    {"corner_kick_left",       {Environment::PlayMode::OUR_CORNER_KICK,  Environment::PlayMode::THEIR_CORNER_KICK}},

    // LEFT: OUR_GOAL_KICK (3); RIGHT: THEIR_GOAL_KICK (12)
    {"goal_kick_left",         {Environment::PlayMode::OUR_GOAL_KICK,    Environment::PlayMode::THEIR_GOAL_KICK}},

    // LEFT: OUR_FREE_KICK (4); RIGHT: THEIR_FREE_KICK (13)
    {"free_kick_left",         {Environment::PlayMode::OUR_FREE_KICK,    Environment::PlayMode::THEIR_FREE_KICK}},

    // LEFT: OUR_PASS (5); RIGHT: THEIR_PASS (14)
    {"pass_left",              {Environment::PlayMode::OUR_PASS,         Environment::PlayMode::THEIR_PASS}},

    // LEFT: OUR_DIR_FREE_KICK (6); RIGHT: THEIR_DIR_FREE_KICK (15)
    {"direct_free_kick_left",  {Environment::PlayMode::OUR_DIR_FREE_KICK, Environment::PlayMode::THEIR_DIR_FREE_KICK}},

    // LEFT: OUR_GOAL (7); RIGHT: THEIR_GOAL (16)
    {"Goal_Left",              {Environment::PlayMode::OUR_GOAL,         Environment::PlayMode::THEIR_GOAL}},

    // LEFT: OUR_OFFSIDE (8); RIGHT: THEIR_OFFSIDE (17)
    {"offside_left",           {Environment::PlayMode::OUR_OFFSIDE,      Environment::PlayMode::THEIR_OFFSIDE}},

    // --- RIGHT Kick Events (RIGHT é o nosso time, LEFT é o time deles) ---

    // LEFT_PLAY_MODE_TO_ID: KickOff_Right -> THEIR_KICKOFF (9)
    // RIGHT_PLAY_MODE_TO_ID: KickOff_Right -> OUR_KICKOFF (0)
    {"KickOff_Right",          {Environment::PlayMode::THEIR_KICKOFF,    Environment::PlayMode::OUR_KICKOFF}},

    // LEFT: THEIR_KICK_IN (10); RIGHT: OUR_KICK_IN (1)
    {"KickIn_Right",           {Environment::PlayMode::THEIR_KICK_IN,    Environment::PlayMode::OUR_KICK_IN}},

    // LEFT: THEIR_CORNER_KICK (11); RIGHT: OUR_CORNER_KICK (2)
    {"corner_kick_right",      {Environment::PlayMode::THEIR_CORNER_KICK, Environment::PlayMode::OUR_CORNER_KICK}},

    // LEFT: THEIR_GOAL_KICK (12); RIGHT: OUR_GOAL_KICK (3)
    {"goal_kick_right",        {Environment::PlayMode::THEIR_GOAL_KICK,  Environment::PlayMode::OUR_GOAL_KICK}},

    // LEFT: THEIR_FREE_KICK (13); RIGHT: OUR_FREE_KICK (4)
    {"free_kick_right",        {Environment::PlayMode::THEIR_FREE_KICK,  Environment::PlayMode::OUR_FREE_KICK}},

    // LEFT: THEIR_PASS (14); RIGHT: OUR_PASS (5)
    {"pass_right",             {Environment::PlayMode::THEIR_PASS,       Environment::PlayMode::OUR_PASS}},

    // LEFT: THEIR_DIR_FREE_KICK (15); RIGHT: OUR_DIR_FREE_KICK (6)
    {"direct_free_kick_right", {Environment::PlayMode::THEIR_DIR_FREE_KICK, Environment::PlayMode::OUR_DIR_FREE_KICK}},

    // LEFT: THEIR_GOAL (16); RIGHT: OUR_GOAL (7)
    {"Goal_Right",             {Environment::PlayMode::THEIR_GOAL,       Environment::PlayMode::OUR_GOAL}},

    // LEFT: THEIR_OFFSIDE (17); RIGHT: OUR_OFFSIDE (8)
    {"offside_right",          {Environment::PlayMode::THEIR_OFFSIDE,    Environment::PlayMode::OUR_OFFSIDE}}
};













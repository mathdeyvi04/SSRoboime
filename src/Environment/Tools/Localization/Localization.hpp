#pragma once

#define True true
#define False false

#include <array>
#include <vector>
#include <string_view>

/**
 * @brief Responsável por representar e agrupar as instâncias
 * auxiliares de localização e fornecer a lógica.
 */
class Localization {
public:

    // -- Atributos Inerentes à Localização
    std::array<float, 3> my_position = {99, 99, 99};


    struct Landmark {
    public:
        char tag[3];
        float fixed_position[3];
        float sph_position[3];

        Landmark(
            const char (&name)[4],
            float x,
            float y,
            float z
        ) :
            tag{name[0], name[1], name[2]},
            fixed_position{x, y, z}
        {}
    };

    std::array<Landmark, 8> list_landmark {{
        // Se referem a quando estamos no lado esquerdo
        {"F2L", -15.0f, -10.0f, 0.0f},
        {"F1L", -15.0f, +10.0f, 0.0f},
        {"F2R", +15.0f, -10.0f, 0.0f},
        {"F1R", +15.0f, +10.0f, 0.0f},
        {"G2L", -15.0f, -1.05f, 0.8f},
        {"G1L", -15.0f, +1.05f, 0.8f},
        {"G2R", +15.0f, -1.05f, 0.8f},
        {"G1R", +15.0f, +1.05f, 0.8f}
    }};

    std::vector<Landmark*> visibles_landmarks;

    // - Métodos Inerentes à Localização

    Localization(
       // Possíveis atributos que eu possa considerar
    ) {
      visibles_landmarks.reserve(8); // Assim evitamos construções inúteis.
    }

    // -- Funções de Atualização de Itens Visuais

    bool
    update_visible_landmark(
       std::string_view tag_lm,
       float values_from_shp_position[3]
    ){
        // Temos garantia que visible_landmarks está vazio e podemos inserir e modificar

        // Devemos iterar sobre os poucos elementos e inserir os valores correspondente
        for(
            int i = 0;
                i < 8;
                i++
        ){

            if(
                tag_lm == list_landmark[i].tag
            ){

                // Então temos um novo landmark visivel.
                for(
                    int j = 0;
                        j < 3;
                        j++
                ){
                    // Dependendo de qual lado estamos, também devemos fazer alterações

                    list_landmark[i].sph_position[j] = values_from_shp_position[j];
                }

                visibles_landmarks.push_back( &list_landmark[i] );
                return True;
            }
        }

        return False;
    }

    bool
    localize(){
        // Implementaremos a lógica de posição aqui.
        // Temos garantia que utilizaremos essa função logo após o parsing da mensagem

        if(visibles_landmarks.size() < 2){
            return False;
        }

        // Então temos garantia de que há mais de dois pontos.
        // Dependendo da distância, pode haver muito ruído na medida
        // Logo, se torna necessário sermos capazes de obter o valor esperado da medida





        visibles_landmarks.clear();
    }





};
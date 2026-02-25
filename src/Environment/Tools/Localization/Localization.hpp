#pragma once

#include "../../../Booting/Math.hpp"

#ifdef ENABLE_DEBUG_VISION
#include "../../../Drawer/Drawer.hpp"
#endif

#include <array>
#include <vector>
#include <string_view>
#include <cmath>

/**
 * @brief Responsável por representar e agrupar as instâncias
 * auxiliares de localização e fornecer a lógica.
 */
class Localization {
public:

    // -- Atributos Inerentes à Localização Pensada pelo Robô
    // -- x, y, z, rotation_xy, rotation_xz
    std::array<double, 5> my_pose = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    struct Landmark {
    public:
        char tag[3];
        double fixed_position[3];
        double cart_relative_position[3];
        double sph_relative_position[3]; //< Será alterável conforme o agente atualizá-los

        Landmark(
            const char (&name)[4],
            double x,
            double y,
            double z
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
       float values_from_shp_relative_position[3]
    ){
        // Temos garantia que visible_landmarks está vazio e podemos inserir e modificar

        // Devemos iterar sobre os poucos elementos e inserir os valores correspondente
        for(
            int i = 0;
                i < 8;
                i++
        ){

            if(
                // Vamos ter que fazer de forma bruta mesmo
                tag_lm[0] == list_landmark[i].tag[0] && tag_lm[1] == list_landmark[i].tag[1] && tag_lm[2] == list_landmark[i].tag[2]
            ){

                // Então temos um novo landmark visivel.
                for(
                    int j = 0;
                        j < 3;
                        j++
                ){
                    // Dependendo de qual lado estamos, também devemos fazer alterações
                    // 0 -> distance from the camera
                    // 1 -> ang_horizontal
                    // 2 -> ang_vertical
                    list_landmark[i].sph_relative_position[j] = values_from_shp_relative_position[j];
                }

                visibles_landmarks.push_back( &list_landmark[i] );
                return True;
            }
        }

        return False;
    }

    bool
    update_pose(){
        // Implementaremos a lógica de posição aqui.
        // Temos garantia que utilizaremos essa função logo após o parsing da mensagem

        if(visibles_landmarks.size() < 2){
            return False;
        }

        // Precisamos dos marcadores mais próximos, assim o erro será o menor.
        Landmark* closest_lm = nullptr;
        Landmark* almost_closest_lm = nullptr;
        for(
            const auto& lm : visibles_landmarks
        ){

            if(!closest_lm || lm->sph_relative_position[0] < closest_lm->sph_relative_position[0]){
                // O antigo "melhor" se torna o "segundo melhor"
                almost_closest_lm = closest_lm;

                // O atual se torna o "melhor"
                closest_lm = lm;
            }
            else if(!almost_closest_lm || lm->sph_relative_position[0] < almost_closest_lm->sph_relative_position[0]){
                // O atual não venceu o primeiro, mas venceu o segundo
                almost_closest_lm = lm;
            }
        }

        // Obtemos as coordenadas cartesianas considerando como referência a cabeça do robô
        double dist_on_2d_plane_to_closest = closest_lm->sph_relative_position[0] * std::cos(closest_lm->sph_relative_position[2] * Math::const_deg_to_rad);
        double x1_rel_to_camera = dist_on_2d_plane_to_closest * std::cos(closest_lm->sph_relative_position[1] * Math::const_deg_to_rad);
        double y1_rel_to_camera = dist_on_2d_plane_to_closest * std::sin(closest_lm->sph_relative_position[1] * Math::const_deg_to_rad);
        double z1_rel_to_camera = closest_lm->sph_relative_position[0] * std::sin(closest_lm->sph_relative_position[2] * Math::const_deg_to_rad);

        double dist_on_2d_plane_to_almost_closest = almost_closest_lm->sph_relative_position[0] * std::cos(closest_lm->sph_relative_position[2] * Math::const_deg_to_rad);
        double x2_rel_to_camera = dist_on_2d_plane_to_closest * std::cos(almost_closest_lm->sph_relative_position[1] * Math::const_deg_to_rad);
        double y2_rel_to_camera = dist_on_2d_plane_to_closest * std::sin(almost_closest_lm->sph_relative_position[1] * Math::const_deg_to_rad);
        double z2_rel_to_camera = almost_closest_lm->sph_relative_position[0] * std::sin(almost_closest_lm->sph_relative_position[2] * Math::const_deg_to_rad);

        // A partir do conhecimento do ponto fixo, podemos conhecer o ponto que estamos
        this->my_pose[0] = std::lerp( closest_lm->fixed_position[0] - x1_rel_to_camera, almost_closest_lm->fixed_position[0] - x2_rel_to_camera, 0.5 );
        this->my_pose[1] = std::lerp( closest_lm->fixed_position[1] - y1_rel_to_camera, almost_closest_lm->fixed_position[1] - y2_rel_to_camera, 0.5 );
        this->my_pose[2] = std::lerp( closest_lm->fixed_position[2] - z1_rel_to_camera, almost_closest_lm->fixed_position[2] - z2_rel_to_camera, 0.5 );

#ifdef ENABLE_DEBUG_VISION

        // Desejamos apresentar o local que estamos aproximadamente!
        Drawer& drawer = Drawer::get_instance();
        std::string status = "status";

        drawer.draw_line(
            this->my_pose[0],
            this->my_pose[1],
            this->my_pose[2],
            this->my_pose[0],
            this->my_pose[1],
            this->my_pose[2] + 0.5,
            10,
            0, 0, 0,
            status
        );
        drawer.draw_annotation("My Position", this->my_pose[0], this->my_pose[1], this->my_pose[2] + 1, 1, 1, 1, status);
        drawer.swap_buffers(status); // Commita o desenho
        drawer.flush();

#endif





        visibles_landmarks.clear();

        return True;
    }





};
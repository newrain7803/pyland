#include <cmath>
#include <fstream>
#include <glog/logging.h>
#include <glm/vec2.hpp>
#include <new>

#include "engine.hpp"
#include "entitythread.hpp"
#include "image.hpp"
#include "map_object.hpp"
#include "map_viewer.hpp"
#include "renderable_component.hpp"
#include "texture.hpp"
#include "texture_atlas.hpp"
#include "walkability.hpp"

#ifdef USE_GLES
#include <GLES2/gl2.h>
#endif

#ifdef USE_GL
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif

// WTF
MapObject::MapObject(glm::vec2 position,
                     std::string name,
                     Walkability walkability,
                     int sheet_id,
                     std::string sheet_name):

    Object(name),
    walkability(walkability),
    sheet_name(sheet_name),
    sheet_id(sheet_id),
    position(position) {

        VLOG(2) << "New map object: " << name;

        regenerate_blockers();

        init_shaders();
        load_textures();
        generate_tex_data();
        generate_vertex_data();

        LOG(INFO) << "MapObject initialized";
}

MapObject::~MapObject() {
    LOG(INFO) << "MapObject destructed";
}

void MapObject::set_walkability(Walkability walkability) {
    this->walkability = walkability;
    regenerate_blockers();
}

void MapObject::regenerate_blockers() {
    body_blockers.clear();
    switch (walkability) {
        case Walkability::WALKABLE: {
            break;
        }

        case Walkability::BLOCKED: {
            int x_left(int(position.x));
            int y_bottom(int(position.y));

            // If non-integral, the left or top have a higher
            // tile number. If integral, they do not.
            //
            // The test is done by checking if the truncation
            // changed the value
            int x_right(x_left   + (float(x_left)   != position.x));
            int y_top  (y_bottom + (float(y_bottom) != position.y));

            auto *map = Engine::get_map_viewer()->get_map();
            body_blockers = {
                map->block_tile(glm::ivec2(x_left,  y_top)),
                map->block_tile(glm::ivec2(x_left,  y_bottom)),
                map->block_tile(glm::ivec2(x_right, y_top)),
                map->block_tile(glm::ivec2(x_right, y_bottom))
            };

            break;
        }

        default: {
            throw std::runtime_error("WTF do I do now?");
        }
    }
}

void MapObject::generate_tex_data() {
    // holds the texture data
    // need 12 float for the 2D texture coordinates
    int num_floats = 12;

    GLfloat *map_object_tex_data;
    try {
        map_object_tex_data = new GLfloat[sizeof(GLfloat)*num_floats];
    }
    catch(std::bad_alloc &) {
        LOG(ERROR) << "ERROR in MapObject::generate_tex_data(), cannot allocate memory";
        return;
    }

    std::tuple<float,float,float,float> bounds = renderable_component.get_texture()->get_atlas()->index_to_coords(sheet_id);
    
    // bottom left
    map_object_tex_data[ 0] = std::get<0>(bounds);
    map_object_tex_data[ 1] = std::get<2>(bounds);

    // top left
    map_object_tex_data[ 2] = std::get<0>(bounds);
    map_object_tex_data[ 3] = std::get<3>(bounds);

    // bottom right
    map_object_tex_data[ 4] = std::get<1>(bounds);
    map_object_tex_data[ 5] = std::get<2>(bounds);

    // top left
    map_object_tex_data[ 6] = std::get<0>(bounds);
    map_object_tex_data[ 7] = std::get<3>(bounds);

    // top right
    map_object_tex_data[ 8] = std::get<1>(bounds);
    map_object_tex_data[ 9] = std::get<3>(bounds);

    // bottom right
    map_object_tex_data[10] = std::get<1>(bounds);
    map_object_tex_data[11] = std::get<2>(bounds);

    renderable_component.set_texture_coords_data(map_object_tex_data, sizeof(GLfloat)*num_floats, false);
}

void MapObject::set_sheet_id(int sheet_id) {
    this->sheet_id = sheet_id;
    generate_tex_data();
}

void MapObject::set_sheet_name(std::string sheet_name) {
    this->sheet_name = sheet_name;
    generate_tex_data();
}

void MapObject::generate_vertex_data() {
    //holds the map object's vertex data
    int num_dimensions(2);
    int num_floats(num_dimensions * 6); // GL's Triangles
    GLfloat *map_object_vert_data(nullptr);

    try {

        map_object_vert_data = new GLfloat[sizeof(GLfloat)*num_floats];
    }
    catch(std::bad_alloc& ba) {
        LOG(ERROR) << "ERROR in MapObject::generate_vertex_data(), cannot allocate memory";
        return;
    }

    float scale(Engine::get_actual_tile_size());

    //bottom left
    map_object_vert_data[0] = 0;
    map_object_vert_data[1] = 0;

    //top left
    map_object_vert_data[2] = 0;
    map_object_vert_data[3] = scale;

    //bottom right
    map_object_vert_data[4] = scale;
    map_object_vert_data[5] = 0;

    //top left
    map_object_vert_data[6] = 0;
    map_object_vert_data[7] = scale;

    //top right
    map_object_vert_data[8] = scale;
    map_object_vert_data[9] = scale;

    //bottom right
    map_object_vert_data[10] = scale;
    map_object_vert_data[11] = 0;

    renderable_component.set_vertex_data(map_object_vert_data, sizeof(GLfloat)*num_floats, false);
    renderable_component.set_num_vertices_render(num_floats / num_dimensions);//GL_TRIANGLES being used
}

// TODO: rewrite
void MapObject::load_textures() {
    // Image *texture_image = nullptr;

    // try {
    //     texture_image = new Image(sheet_name.c_str());
    // }
    // catch (std::exception e) {
    //     LOG(ERROR) << "Failed to create texture";
    //     return;
    // }

    // //Set the texture data in the rederable component
    // renderable_component.set_texture_image(texture_image);
    
    renderable_component.set_texture(Texture::get_shared(sheet_name.c_str(), 0));
}

bool MapObject::init_shaders() {
    std::shared_ptr<Shader> shader;
    try {
        shader = Shader::get_shared("tile_shader");
    }
    catch (std::exception e) {
        LOG(ERROR) << "Failed to create the shader";
        return false;
    }

    //Set the shader
    renderable_component.set_shader(shader);
    return true;
}

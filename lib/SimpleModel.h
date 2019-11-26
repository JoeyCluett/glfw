#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <map>
#include <array>
#include <fstream>

// class to assist in the loading of simple
// models described with plain ASCII text

typedef glm::vec3               vertex_t;
typedef std::array<vertex_t, 3> tri_t;
typedef std::vector<tri_t>      model_t;

struct ModelInfo {

    GLuint buffer_id;
    int vertices;

    friend std::ostream& operator<<(std::ostream& os, ModelInfo& mi) {
        os << mi.buffer_id << ' ' << mi.vertices;
        return os;
    }

};

struct ModelImportInfo {
    std::string filename;
    std::string modelname;
    ModelInfo* mi_ptr;
};

class ModelParser {
private:

    static std::string model_loc;

    model_t generate_circle(int numpts, float radius);

public:

    std::map<std::string, vertex_t> named_vertices;
    std::map<std::string, tri_t>    named_tris;
    std::map<std::string, model_t>  named_models;
    std::map<std::string, std::vector<std::string>>  m_namespace;
    std::map<std::string, model_t>  exported_models;
    std::map<std::string, ModelParser*>* files;

    static void setFileLocation(std::string loc);
    static auto calculateNormals(std::vector<float>& v) -> std::vector<float>;
    static auto loadForeignModelIntoRuntime(std::vector<GLfloat>& v, int render_mode = GL_TRIANGLES) -> ModelInfo;
    static void loadModelList(std::vector<ModelImportInfo> miov);

    // constructor user will use. other constructor is used internally
    // to create other ModelParser objects when importing files
    ModelParser(std::string filename);

    // this constructor should never be used by the user directly.
    // it exists as a way to prevent recursive inclusion when importing
    // SimpleModel files
    ModelParser(std::string filename, std::map<std::string, ModelParser*>& imported_files);

    // load a particular model into OpenGL. creates buffer id and counts
    // the number of vertices needed to draw it
    auto loadExportedModelIntoRuntime(const std::string& modelname) -> ModelInfo;

    // the meat of the parsing functionality
    void parse(std::string filename, std::map<std::string, ModelParser*>& imported_files);

    // get data associated with exported model. returns a vector of
    // floats and the number of vertices in the model
    auto getExportedModelData(const std::string& modelname)
            -> std::pair<std::vector<float>, int>;

    // get ordinary model data. returns a vector of floats and the
    // number of vertices in the model
    auto getModelData(const std::string& modelname)
            -> std::pair<std::vector<float>, int>;

    // split every triangle into 3 seperate triangles. modifies the
    // vertex array in place
    void tessellate(void);

    // convenience function to print the contents of a ModelParser
    friend std::ostream& operator<<(std::ostream& os, ModelParser& mp);

    // destructor
    ~ModelParser(void);

};

// initialize static data member to current directory
std::string ModelParser::model_loc = "./";

typedef ModelParser SimpleModelParser;

ModelParser::ModelParser(std::string filename) {
    this->files = new std::map<std::string, ModelParser*>;
    this->files->insert({ filename, NULL });
    this->parse(model_loc + filename, *this->files);

    // place exported models in the exported_models list
    for(auto& ns : this->m_namespace) {
        for(auto& str : ns.second) {
            this->exported_models.insert({
                ns.first + "." + str,
                this->named_models.at(str)
            });
        }
    }

    // do not get rid of
    this->named_vertices.clear();
    this->named_tris.clear();
    this->m_namespace.clear();
    SimpleModelParser::setFileLocation("../assets/models/");

}

ModelParser::ModelParser(std::string filename, std::map<std::string, ModelParser*>& imported_files) {
    this->files = NULL;

    if(imported_files.find(filename) != imported_files.end()) {
        throw std::runtime_error(
            "file already exists in workspace");
    }

    imported_files.insert({ filename, this });
    this->parse(model_loc + filename, imported_files);

    // place exported models in the exported_models list
    for(auto& ns : this->m_namespace) {
        for(auto& str : ns.second) {
            this->exported_models.insert({
                ns.first + "." + str,
                this->named_models.at(str)
            });
        }
    }

    this->named_vertices.clear();
    this->named_tris.clear();
    this->m_namespace.clear();
    this->named_models.clear();
}

void ModelParser::tessellate(void) {

    // iterate through every triangle


}

void ModelParser::setFileLocation(std::string loc) {

    ModelParser::model_loc = loc;

}

auto ModelParser::calculateNormals(std::vector<float>& v) -> std::vector<float> {
    std::vector<float> n;

    //std::cout << "NUM FLOATS[" << v.size() << "]" << std::flush;

    int sz = v.size();
    for(int i = 0; i < sz; i += 9) {

        //std::cout << "," << i << std::flush;

        float
            x1 = v[i+0], y1 = v[i+1], z1 = v[i+2],
            x2 = v[i+3], y2 = v[i+4], z2 = v[i+5],
            x3 = v[i+6], y3 = v[i+7], z3 = v[i+8];

        vertex_t d1 = { x2-x1, y2-y1, z2-z1 };
        vertex_t d2 = { x3-x2, y3-y2, z3-z2 };

        vertex_t c = {
            d1.y*d2.z - d1.z*d2.y,
            d1.z*d2.x - d1.x*d2.z,
            d1.x*d2.y - d1.y*d2.x
        };

        for(int j = 0; j < 3; j++) {
            n.push_back(c.x);
            n.push_back(c.y);
            n.push_back(c.z);
        }

    }

    return n;
}

auto ModelParser::getExportedModelData(const std::string& modelname) ->
        std::pair<std::vector<float>, int> {

    auto iter = this->exported_models.find(modelname);
    if(iter == this->exported_models.end())
        throw std::runtime_error("unable to find exported model in SimpleModel");

    std::vector<float> float_vec;
    for(auto& m : iter->second) {
        for(auto& v : m) {
            float_vec.push_back(v.x);
            float_vec.push_back(v.y);
            float_vec.push_back(v.z);
        }
    }

    return {
        float_vec,
        float_vec.size() / 3
    };

}

auto ModelParser::loadExportedModelIntoRuntime(const std::string& modelname) -> ModelInfo {
    auto iter = this->exported_models.find(modelname);
    if(iter == this->exported_models.end())
        throw std::runtime_error("unable to find exported model in SimpleModel: " + modelname);

    std::vector<float> float_vec;
    for(auto& m : iter->second) {
        for(auto& v : m) {
            float_vec.push_back(v.x);
            float_vec.push_back(v.y);
            float_vec.push_back(v.z);
        }
    }

    ModelInfo mi;

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, float_vec.size() * 4, float_vec.data(), GL_STATIC_DRAW);

    mi.buffer_id = vbo;
    mi.vertices = float_vec.size() / 3;

    //std::cout << mi << std::endl;

    return mi;

}

auto ModelParser::loadForeignModelIntoRuntime(std::vector<GLfloat>& v, int render_mode) -> ModelInfo {

    ModelInfo mi;
    glGenBuffers(1, &mi.buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, mi.buffer_id);
    glBufferData(GL_ARRAY_BUFFER, v.size() * 4, v.data(), GL_STATIC_DRAW);

    if(render_mode == GL_TRIANGLES)
        mi.vertices = v.size() / 3;
    else if(render_mode == GL_LINES)
        mi.vertices = v.size() / 2;
    else
        throw std::runtime_error("ModelParser::loadForeignModelIntoRuntime invalid render mode");

    return mi;
}

void ModelParser::loadModelList(std::vector<ModelImportInfo> miov) {

    for(auto& mio : miov) {
        ModelParser mp(mio.filename);
        auto mi = mp.loadExportedModelIntoRuntime(mio.modelname);

        mio.mi_ptr->buffer_id = mi.buffer_id;
        mio.mi_ptr->vertices  = mi.vertices;
    }

}

auto ModelParser::getModelData(const std::string& modelname) ->
        std::pair<std::vector<float>, int> {

    auto iter = this->named_models.find(modelname);
    if(iter == this->named_models.end())
        throw std::runtime_error("unable to find named model in SimpleModel");

    std::vector<float> float_vec;
    for(auto& m : iter->second) {
        for(auto& v : m) {
            float_vec.push_back(v.x);
            float_vec.push_back(v.y);
            float_vec.push_back(v.z);
        }
    }

    return {
        float_vec,
        float_vec.size() / 3
    };

}

model_t ModelParser::generate_circle(int numpts, float radius) {
    std::vector<std::pair<float, float>> pts;

    // generate the points needed
    for(int i = 0; i < numpts; i++) {
        //auto theta = linear_map(0.0, float(numpts), 0.0f, 2.0*M_PI, i);
        double theta = double(i) * 2.0 * M_PI / numpts;
        pts.push_back({ radius*cos(theta), radius*sin(theta) });
    }

    model_t m;

    for(int i = 0; i < numpts; i++) {

        tri_t t = {
            vertex_t({ pts[i].first,            0.0f, pts[i].second }),
            vertex_t({ pts[(i+1)%numpts].first, 0.0f, pts[(i+1)%numpts].second }),
            vertex_t({ 0.0f, 0.0f, 0.0f })
        };

        m.push_back(t);

    }

    return m;

}

void ModelParser::parse(std::string filename, std::map<std::string, ModelParser*>& imported_files) {

    const int STATE_default       = 0;
    const int STATE_named_vertex  = 1;
    const int STATE_named_tri     = 2;
    const int STATE_named_model   = 3;
    const int STATE_export        = 4;
    const int STATE_import        = 5;
    const int STATE_namespace     = 6;
    const int STATE_export_rename = 7;
    const int STATE_tf_xlat       = 8;
    const int STATE_tf_rot        = 9;
    const int STATE_tf_rotdeg     = 10;
    const int STATE_tf_scale      = 11;
    const int STATE_gen_circ      = 12;
    const int STATE_gen_cyl       = 13;
    int STATE_current = STATE_default;

    std::vector<std::string> token_list;

    // tokenize
    {
        bool in_comment = false;
        std::string   current_token;
        std::ifstream is(filename, std::ios::in);

        while(is >> current_token) {

            //std::cout << in_comment << " " << current_token << std::endl;

            if(in_comment) {
                if(current_token == "**>")
                    in_comment = false;
            }
            else {
                if(current_token == "<**")
                    in_comment = true;
                else
                    token_list.push_back(current_token);
            }
        }
    }

    // parse
    {
        int sz = token_list.size();
        for(int i = 0; i < sz;) {

            auto& str = token_list.at(i);
            auto& tl = token_list;

            //std::cout << str << std::endl << std::flush;

            const std::map<std::string, const int> default_state_map = {
                { "(nv)",      STATE_named_vertex },
                { "(nt)",      STATE_named_tri },
                { "(nm)",      STATE_named_model },
                { "(exm)",     STATE_export },
                { "(imp)",     STATE_import },
                { "(ns)",      STATE_namespace },
                { "(exrn)",    STATE_export_rename },
                { "(tfxlat)",  STATE_tf_xlat },
                { "(tfrotx)",  STATE_tf_rot },
                { "(tfroty)",  STATE_tf_rot },
                { "(tfrotz)",  STATE_tf_rot },
                { "(tfrotdx)", STATE_tf_rotdeg },
                { "(tfrotdy)", STATE_tf_rotdeg },
                { "(tfrotdz)", STATE_tf_rotdeg },
                { "(tfsc)",    STATE_tf_scale },
                { "(gcirc)",   STATE_gen_circ },
                { "(gcyl)",    STATE_gen_cyl }
            };

            switch(STATE_current) {
                case STATE_default:
                    {
                        auto iter = default_state_map.find(str);
                        if(iter == default_state_map.end())
                            throw std::runtime_error("unknown token '" + str + "' encountered");

                        STATE_current = iter->second;
                    }
                    i++;
                    break;

                case STATE_named_vertex:
                    {
                        if(this->named_vertices.find(str) != this->named_vertices.end()) {
                            throw std::runtime_error("repeated named vertex in ModelParser");
                        }

                        this->named_vertices.insert({
                            str,
                            {
                                std::stof(tl.at(i+1)),
                                std::stof(tl.at(i+2)),
                                std::stof(tl.at(i+3))
                            }
                        });

                        STATE_current = STATE_default;

                    }
                    i += 4;
                    break;

                case STATE_named_tri:
                    {
                        if(this->named_tris.find(str) != this->named_tris.end())
                            throw std::runtime_error("repeated named tri in ModelParser");

                        this->named_tris.insert({
                            str,
                            {
                                named_vertices.at(tl.at(i+1)),
                                named_vertices.at(tl.at(i+2)),
                                named_vertices.at(tl.at(i+3))
                            }
                        });

                        STATE_current = STATE_default;

                    }
                    i += 4;
                    break;

                case STATE_named_model:
                    {
                        auto name = str;
                        int numt = std::stoi(tl.at(i + 1));
                        i += 2;
                        std::vector<std::array<glm::vec3, 3>> mv;


                        for(int j = 0; j < numt; j++) {
                            auto model_name = tl.at(i+j);

                            if(this->named_tris.find(model_name) != this->named_tris.end()) {
                                mv.push_back(this->named_tris.at(model_name));

                            }
                            else if(this->named_models.find(model_name) != this->named_models.end()) {

                                auto m = this->named_models.at(model_name);
                                for(auto& t : m) {
                                    mv.push_back(t);
                                }

                            }
                            else {

                                throw std::runtime_error("NULL source when creating model");

                            }
                        }

                        this->named_models.insert({ name, mv });

                        i += numt;
                    }
                    STATE_current = STATE_default;
                    break;

                case STATE_export:
                    if(this->m_namespace.find(str) == this->m_namespace.end())
                        throw std::runtime_error(
                            "attempting to export to NULL namespace");

                    if(this->named_models.find(tl.at(i+1)) == this->named_models.end())
                        throw std::runtime_error(
                            "attempt to export non-existent model '" +
                            tl.at(i+1) + "'");

                    this->m_namespace.at(str).push_back(tl[i+1]);

                    i += 2;
                    STATE_current = STATE_default;
                    break;

                case STATE_import:
                    {
                        ModelParser* mp;

                        try {
                            mp = new ModelParser(str, imported_files);

                            //std::cout
                            //    << "Imported workspace:\n\n"
                            //    << mp << std::endl;

                            // iterate through exported models
                            // and add them to the current workspace
                            for(auto& v : mp->exported_models) {

                                // if exported model already exists, throw exception
                                if(this->named_models.find(v.first) != this->named_models.end())
                                    throw std::runtime_error(
                                        "importing preexisting model");

                                this->named_models.insert(v);

                            }
                        }
                        catch(std::runtime_error& ex) {
                            // harmless exception
                            // let it go
                        }
                        catch(std::exception& up) {
                            throw up; // lol
                        }
                    }
                    i++;
                    STATE_current = STATE_default;
                    break;

                case STATE_namespace:
                    if(this->m_namespace.find(str) != this->m_namespace.end())
                        throw std::runtime_error("duplicate namespace in ModelParser");

                    this->m_namespace.insert({ str, {} });
                    i++;
                    STATE_current = STATE_default;

                    break;

                case STATE_export_rename:
                    {
                        // add model directly to exported_models list
                        if(this->m_namespace.find(str) == this->m_namespace.end())
                            throw std::runtime_error(
                                "attempting to export to NULL namespace");

                        if(this->named_models.find(tl.at(i+1)) == this->named_models.end())
                            throw std::runtime_error(
                                "attempt to export non-existent model '" +
                                tl.at(i+1) + "'");

                        auto new_name = tl.at(i+2);

                        this->exported_models.insert({
                            str + "." + new_name,
                            this->named_models.at(tl.at(i+1))
                        });
                    }
                    i += 3;
                    STATE_current= STATE_default;
                    break;

                case STATE_tf_xlat:

                    if(this->named_models.find(str) == this->named_models.end())
                        throw std::runtime_error("NULL model for translate transform");

                    {
                        auto xlat_mat = glm::translate(
                            glm::mat4(1.0),
                            {
                                std::stof(tl.at(i+2)),
                                std::stof(tl.at(i+3)),
                                std::stof(tl.at(i+4))
                            });

                        // need to fetch the correct base model
                        model_t m = this->named_models[str];
                        for(auto& t : m) {
                            for(auto& v : t) {
                                auto res = (xlat_mat * glm::vec4(v, 1.0));
                                v = { res.x, res.y, res.z };
                            }
                        }

                        this->named_models[tl.at(i+1)] = m;

                    }

                    i += 5;
                    STATE_current = STATE_default;
                    break;

                case STATE_tf_rot:
                    if(this->named_models.find(str) == this->named_models.end())
                        throw std::runtime_error("NULL model for rotate(radians) transform");

                    {
                        glm::vec3 axis(0.0, 0.0, 0.0);
                        auto tok = tl.at(i-1);
                        if(tok == "(tfrotx)") { axis = { 1.0f, 0.0f, 0.0f }; }
                        else if(tok == "(tfroty)") { axis = { 0.0f, 1.0f, 0.0f }; }
                        else if(tok == "(tfrotz)") { axis = { 0.0f, 0.0f, 1.0f }; }

                        auto rot_mat = glm::rotate(
                            glm::mat4(1.0),
                            std::stof(tl.at(i+2)),
                            axis);

                        // need to fetch the correct base model
                        model_t m = this->named_models[str];
                        for(auto& t : m) {
                            for(auto& v : t) {
                                auto res = (rot_mat * glm::vec4(v, 1.0));
                                v = { res.x, res.y, res.z };
                            }
                        }

                        this->named_models[tl.at(i+1)] = m;

                    }

                    i += 3;
                    STATE_current = STATE_default;
                    break;

                case STATE_tf_rotdeg:
                    if(this->named_models.find(str) == this->named_models.end())
                        throw std::runtime_error("NULL model for rotate(degrees) transform");

                    {
                        glm::vec3 axis(0.0, 0.0, 0.0);
                        auto tok = tl.at(i-1);
                        if(tok == "(tfrotdx)") { axis = { 1.0f, 0.0f, 0.0f }; }
                        else if(tok == "(tfrotdy)") { axis = { 0.0f, 1.0f, 0.0f }; }
                        else if(tok == "(tfrotdz)") { axis = { 0.0f, 0.0f, 1.0f }; }

                        auto rot_mat = glm::rotate(
                            glm::mat4(1.0),
                            std::stof(tl.at(i+2)) / 57.295779513f,
                            axis);

                        // need to fetch the correct base model
                        model_t m = this->named_models[str];
                        for(auto& t : m) {
                            for(auto& v : t) {
                                auto res = (rot_mat * glm::vec4(v, 1.0));
                                v = { res.x, res.y, res.z };
                            }
                        }

                        this->named_models[tl.at(i+1)] = m;

                    }

                    i += 3;
                    STATE_current = STATE_default;
                    break;

                case STATE_tf_scale:
                    if(this->named_models.find(str) == this->named_models.end())
                        throw std::runtime_error("NULL model for scale transform");

                    {

                        auto scale_mat = glm::scale(
                            glm::mat4(1.0),
                            {
                                std::stof(tl.at(i+2)),
                                std::stof(tl.at(i+3)),
                                std::stof(tl.at(i+4))
                            });

                        // need to fetch the correct base model
                        model_t m = this->named_models[str];
                        for(auto& t : m) {
                            for(auto& v : t) {
                                auto res = (scale_mat * glm::vec4(v, 1.0));
                                v = { res.x, res.y, res.z };
                            }
                        }

                        this->named_models[tl.at(i+1)] = m;

                    }

                    i += 5;
                    STATE_current = STATE_default;
                    break;

                case STATE_gen_circ:
                    {
                        auto& modelname = str;
                        int numpts      = std::stoi(tl.at(i+1));
                        float radius    = std::stof(tl.at(i+2));

                        model_t m = this->generate_circle(numpts, radius);

                        this->named_models.insert({
                            modelname,
                            m
                        });
                    }

                    i += 3;
                    STATE_current = STATE_default;
                    break;

                case STATE_gen_cyl:
                    {
                        auto& modelname = str;
                        int   numpts    = std::stoi(tl.at(i+1));
                        float radius    = std::stof(tl.at(i+2));
                        float height    = std::stof(tl.at(i+3));

                        model_t bot = this->generate_circle(numpts, radius);
                        model_t top = bot;

                        // the top circle must be translated up
                        for(auto& t : top) {
                            for(auto& v : t) {
                                v.y = height;
                            }
                        }

                        std::vector<std::pair<float, float>> pts;
                        // generate the points needed
                        for(int i = 0; i < numpts; i++) {
                            //auto theta = linear_map(0.0, float(numpts), 0.0f, 2.0*M_PI, i);
                            double theta = double(i) * 2.0 * M_PI / numpts;
                            pts.push_back({ radius*cos(theta), radius*sin(theta) });
                        }

                        model_t sides;

                        for(int i = 0; i < numpts; i++) {
                            sides.push_back({
                                vertex_t({ pts[i].first,            0.0f, pts[i].second }),
                                vertex_t({ pts[(i+1)%numpts].first, 0.0f, pts[(i+1)%numpts].second }),
                                vertex_t({ pts[(i+1)%numpts].first, height, pts[(i+1)%numpts].second })
                            });

                            sides.push_back({
                                vertex_t({ pts[i].first,            0.0f, pts[i].second }),
                                vertex_t({ pts[(i+1)%numpts].first, height, pts[(i+1)%numpts].second }),
                                vertex_t({ pts[i].first, height, pts[i].second })
                            });

                        }

                        bot.insert(
                            bot.end(),
                            top.begin(),
                            top.end());

                        bot.insert(
                            bot.end(),
                            sides.begin(),
                            sides.end());

                        this->named_models.insert({
                            modelname,
                            bot
                        });

                    }

                    i += 4;
                    STATE_current = STATE_default;
                    break;

                default:
                    throw std::runtime_error("unknown state in ModelParser");
            }

        }
    }

}

std::ostream& operator<<(std::ostream& os, ModelParser& mp) {

    os << "\nNamed models:\n";
    for(auto& p : mp.named_models) {
        os << "    " << p.first << std::endl;
    }

    os << "\nExported models:\n";
    for(auto& ns : mp.exported_models) {
        os << "    " << ns.first << "\n";
    }

    return os;
}

ModelParser::~ModelParser(void) {
    if(this->files != NULL) {
            for(auto& v : *this->files) {
                delete v.second;
            }
    }
}

#pragma once
#include <string>
#include <vector>
#include <map>
#include "SOIL.h"

#include "vec.hpp"

struct Group
{
public:
  Group(const std::string& name);

public:
  std::string                         m_name;

  std::vector<kmuvcl::math::vec3f>    m_vertices;
  std::vector<kmuvcl::math::vec3f>    m_normals;
  std::vector<kmuvcl::math::vec2f>    m_texcoords;
  std::string                         m_mtl_name;
};

struct Material
{
public:
  Material();
  Material(const std::string& name,
    kmuvcl::math::vec4f& ambient,
    kmuvcl::math::vec4f& diffuse,
    kmuvcl::math::vec4f& specular,
    float& shininess);

public:
  std::string           m_name;

  kmuvcl::math::vec4f   m_ambient;
  kmuvcl::math::vec4f   m_diffuse;
  kmuvcl::math::vec4f   m_specular;
  float                 m_shininess;
};

class Object
{
public:
  Object() {}
  Material mtl;
  GLuint    textureid;
  GLuint    norm_textureid;

  void draw(int loc_a_vertex, int loc_a_normal,
    int loc_u_material_ambient, int loc_u_material_diffuse,
    int loc_u_material_specular, int loc_u_material_shininess);
  void draw(int loc_a_vertex, int loc_a_normal, int loc_a_texcoord,
    int loc_u_material_ambient, int loc_u_material_diffuse,
    int loc_u_material_specular, int loc_u_material_shininess);
  void set_value(const kmuvcl::math::vec3f& t, const GLfloat s, const GLfloat yr){
    translation = t;
    scale = s;
    rotate = yr;
  };
  void set_color(int r, int g, int b){
    mtl.m_ambient = (r, g, b, 1.0);
    mtl.m_diffuse = (r, g, b, 1.0);
    mtl.m_specular = (r, g, b, 1.0);
  };
  void print();

  void naming(const std::string name){this->name = name;}
  void texturename(const std::string texturename_){this->texturename_ = texturename_;}
	bool load_simple_obj(const std::string& filename);
  bool load_simple_mtl(const std::string& filename);
  void load_texture(const std::string& filename);
  void load_normtexture(const std::string& filename);

  void  move_left();
  void  move_right();
  void  move_forward();
  void  move_backward();
  void  rotate_left();
  void  rotate_right();
  void  smaller();
  void  taller();
  void  mark(){translation(1) += 2.0f;}
  const kmuvcl::math::vec3f Trans() const{return translation;}
  const GLfloat Tx() const{return translation(0);}
  const GLfloat Ty() const{return translation(1);}
  const GLfloat Tz() const{return translation(2);}
  const GLfloat S() const{return scale;}
  const GLfloat R() const{return rotate;}
  const std::string N() const{return name;}
  const std::string texture() const{return texturename_;}


private:
  kmuvcl::math::vec3f translation;
  GLfloat scale, rotate;
  std::string name;
  std::string texturename_;
  std::string PATH;
  std::vector<Group>              m_groups;     // Group 형식의 벡터
  std::map<std::string, Material> m_materials;  // string, Material 형식의 map
};

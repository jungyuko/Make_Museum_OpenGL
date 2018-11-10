// Defines the entry point for the console application.
//
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>

#include "Object.h"     //
#include "Shader.h"     // 쉐이더
#include "Camera.h"     // 카메라 함수 관리 헤더
#include "SOIL.h"

#include "vec.hpp"
#include "mat.hpp"
#include "transform.hpp"

#include <glm/ext.hpp>
#include <glm/glm.hpp>

void init();
void display();
void reshape(int, int);
void idle();
void keyboard(unsigned char, int, int);
void special(int, int, int);
bool load_scene(const std::string& filename);
bool store_scene(const std::string& filename);
bool text_paste(const std::string& filename, const std::string& filename2);

GLuint program;

GLint  loc_a_vertex;
GLint  loc_a_normal;
GLint  loc_a_texcoord;

GLint  loc_u_pvm_matrix;
GLint  loc_u_view_matrix;
GLint  loc_u_model_matrix;
GLint  loc_u_normal_matrix;

GLint  loc_u_light_vector;

GLint  loc_u_light_ambient;
GLint  loc_u_light_diffuse;
GLint  loc_u_light_specular;

GLint  loc_u_material_ambient;
GLint  loc_u_material_diffuse;
GLint  loc_u_material_specular;
GLint  loc_u_material_shininess;

GLint     loc_u_texid;
GLuint    texid;
GLint     index_ = 0;
GLint     mode = 0;

kmuvcl::math::mat4x4f   mat_PVM;

kmuvcl::math::vec4f light_vector      = kmuvcl::math::vec4f(10.0f, 10.0f, 10.0f);       // 빛 벡터
kmuvcl::math::vec4f light_ambient     = kmuvcl::math::vec4f(1.0f, 1.0f, 1.0f, 1.0f);    // 주변광 색깔
kmuvcl::math::vec4f light_diffuse     = kmuvcl::math::vec4f(1.0f, 1.0f, 1.0f, 1.0f);    // 난반사광 색깔
kmuvcl::math::vec4f light_specular    = kmuvcl::math::vec4f(1.0f, 1.0f, 1.0f, 1.0f);    // 정반사광 색깔

std::string g_filename, g_filename2;
Object      g_obj, g_selection;        // object
Camera      g_camera;
std::vector<Object> objects;

float model_scale = 1.0f;
float model_angle = 0.0f;
float a = 0.0f;

std::chrono::time_point<std::chrono::system_clock> prev, curr;

int main(int argc, char* argv[])
{
  if (argc > 1)
  {
    g_filename = argv[2];
  }
  else
  {
    g_filename = "./scene/scene.txt";
    g_filename2 = "./scene/scene2.txt";
  }

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(640, 640);
  glutCreateWindow("Modeling & Navigating Your Studio");

  load_scene(g_filename);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special);
  glutIdleFunc(idle);

  if (glewInit() != GLEW_OK)
  {
      std::cerr << "failed to initialize glew" << std::endl;
      return -1;
  }

  init();

  glutMainLoop();

  return 0;
}

void init()
{

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);    // for filled polygon rendering

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glEnable(GL_DEPTH_TEST);

  program = Shader::create_program("./shader/phong_vert.glsl", "./shader/phong_frag.glsl");

  loc_u_pvm_matrix         = glGetUniformLocation(program, "u_pvm_matrix");
  loc_u_view_matrix        = glGetUniformLocation(program, "u_view_matrix");
  loc_u_model_matrix       = glGetUniformLocation(program, "u_model_matrix");
  loc_u_normal_matrix      = glGetUniformLocation(program, "u_normal_matrix");

  loc_u_light_vector       = glGetUniformLocation(program, "u_light_vector");

  loc_u_light_ambient      = glGetUniformLocation(program, "u_light_ambient");
  loc_u_light_diffuse      = glGetUniformLocation(program, "u_light_diffuse");
  loc_u_light_specular     = glGetUniformLocation(program, "u_light_specular");

  loc_u_material_ambient   = glGetUniformLocation(program, "u_material_ambient");
  loc_u_material_diffuse   = glGetUniformLocation(program, "u_material_diffuse");
  loc_u_material_specular  = glGetUniformLocation(program, "u_material_specular");
  loc_u_material_shininess = glGetUniformLocation(program, "u_material_shininess");

  loc_u_texid              = glGetUniformLocation(program, "u_texid");

  loc_a_vertex             = glGetAttribLocation(program, "a_vertex");
  loc_a_normal             = glGetAttribLocation(program, "a_normal");
  loc_a_texcoord           = glGetAttribLocation(program, "a_texcoord");

  prev = curr = std::chrono::system_clock::now();
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(program);

  // Camera setting
  kmuvcl::math::mat4x4f   mat_Proj, mat_View_inv, mat_Model;

  // camera intrinsic param
  mat_Proj = kmuvcl::math::perspective(g_camera.fovy(), 1.0f, 0.001f, 10000.0f);

  // camera extrinsic param
  mat_View_inv = kmuvcl::math::lookAt(
  	g_camera.position().x, g_camera.position().y, g_camera.position().z,				// eye position
  	g_camera.center_position().x, g_camera.center_position().y, g_camera.center_position().z, // center position
  	g_camera.up_direction().x, g_camera.up_direction().y, g_camera.up_direction().z			// up direction
  	);

  mat_Model = kmuvcl::math::scale(model_scale, model_scale, model_scale);
  mat_Model = kmuvcl::math::rotate(model_angle*0.7f, 0.0f, 0.0f, 1.0f) * mat_Model;
  mat_Model = kmuvcl::math::rotate(model_angle,      0.0f, 1.0f, 0.0f) * mat_Model;
  mat_Model = kmuvcl::math::rotate(model_angle*0.5f, 1.0f, 0.0f, 0.0f) * mat_Model;

  mat_PVM = mat_Proj*mat_View_inv*mat_Model;

  kmuvcl::math::mat3x3f mat_Normal;

  mat_Normal(0, 0) = mat_Model(0, 0);
  mat_Normal(0, 1) = mat_Model(0, 1);
  mat_Normal(0, 2) = mat_Model(0, 2);
  mat_Normal(1, 0) = mat_Model(1, 0);
  mat_Normal(1, 1) = mat_Model(1, 1);
  mat_Normal(1, 2) = mat_Model(1, 2);
  mat_Normal(2, 0) = mat_Model(2, 0);
  mat_Normal(2, 1) = mat_Model(2, 1);
  mat_Normal(2, 2) = mat_Model(2, 2);

  kmuvcl::math::mat4x4f mat_View = kmuvcl::math::inverse(mat_View_inv);

	glUniformMatrix4fv(loc_u_pvm_matrix, 1, false, mat_PVM);
  glUniformMatrix4fv(loc_u_model_matrix, 1, false, mat_Model);
  glUniformMatrix4fv(loc_u_view_matrix, 1, false, mat_View);
  glUniformMatrix3fv(loc_u_normal_matrix, 1, false, mat_Normal);

  glUniform3fv(loc_u_light_vector, 1, light_vector);
  glUniform4fv(loc_u_light_ambient, 1, light_ambient);
  glUniform4fv(loc_u_light_diffuse, 1, light_diffuse);
  glUniform4fv(loc_u_light_specular, 1, light_specular);

  Shader::check_gl_error("glUniform4fv");

  // objects draw
  for(int i = 0; i < objects.size(); ++i){
    kmuvcl::math::mat4x4f mat_Trans, mat_Rotate, mat_Scale;
    mat_Trans  = kmuvcl::math::translate(objects.at(i).Tx(),objects.at(i).Ty(),objects.at(i).Tz());
    mat_Rotate = kmuvcl::math::rotate(objects.at(i).R());
    mat_Scale  = kmuvcl::math::scale(objects.at(i).S());
    mat_Model = mat_Trans * mat_Rotate * mat_Scale;
    mat_PVM = mat_Proj*mat_View_inv*mat_Model;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, objects.at(i).textureid);
    glUniform1i(loc_u_texid, 0);

    glUniformMatrix4fv(loc_u_pvm_matrix, 1, false, mat_PVM);
    Shader::check_gl_error("glUniform4fv");
    objects.at(i).draw(loc_a_vertex, loc_a_normal, loc_a_texcoord,
      loc_u_material_ambient, loc_u_material_diffuse,
      loc_u_material_specular, loc_u_material_shininess);
    Shader::check_gl_error("draw");
  }
  // mode1 일 때 selection object draw
  if(mode == 1){
    kmuvcl::math::mat4x4f mat_Trans, mat_Rotate, mat_Scale;
    mat_Trans  = kmuvcl::math::translate(g_selection.Tx(),g_selection.Ty(),g_selection.Tz());
    mat_Rotate = kmuvcl::math::rotate(g_selection.R());
    mat_Scale  = kmuvcl::math::scale(g_selection.S());
    mat_Model = mat_Trans * mat_Rotate * mat_Scale;
    mat_PVM = mat_Proj*mat_View_inv*mat_Model;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_selection.textureid);
    glUniform1i(loc_u_texid, 0);

    glUniformMatrix4fv(loc_u_pvm_matrix, 1, false, mat_PVM);
    Shader::check_gl_error("glUniform4fv");
    g_selection.draw(loc_a_vertex, loc_a_normal, loc_a_texcoord,
      loc_u_material_ambient, loc_u_material_diffuse,
      loc_u_material_specular, loc_u_material_shininess);
    Shader::check_gl_error("draw");
  }

	glUseProgram(0);

  glutSwapBuffers();
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);   //GLint x, GLint y, GLsizei w, GLsizei h
}

void keyboard(unsigned char key, int x, int y)
{
  // TODO: properly handle keyboard event
  if( key == 'a' || key == 'A' ){
    if(mode != 2){
      g_camera.rotate_left(0.1f);
    }
    if(mode == 2){
      objects.at(index_).rotate_left();
    }
  }
  else if( key == 'd' || key == 'D' ){
    if(mode != 2){
      g_camera.rotate_right(0.1f);
    }
    if(mode == 2){
      objects.at(index_).rotate_right();
    }
  }
  else if ( key == 'w' || key == 'W')
	{
		g_camera.move_up(0.1f);
	}
	else if ( key == 's' || key == 'S'){
		g_camera.move_down(0.1f);
	}
  else if ( key == 'm' || key == 'M'){
    if(mode == 0){
      g_selection.set_value(objects.at(index_).Trans(),0.005f,180.0f);
      g_selection.mark();
    }
    mode++;
    mode = mode % 3;
  }
  else if ( key == 'o' || key == 'O'){
    if(mode == 1){
      index_--;
      index_ %= objects.size();
      g_selection.set_value(objects.at(index_).Trans(),0.005f,180.0f);
      g_selection.mark();
    }
  }
  else if ( key == 'p' || key == 'P'){
    if(mode == 1){
      index_++;
      index_ %= objects.size();
      g_selection.set_value(objects.at(index_).Trans(),0.005f,180.0f);
      g_selection.mark();
    }
  }
  else if ( key == 'z' || key == 'Z'){
    if(mode == 2){
      objects.at(index_).smaller();
    }
  }
  else if ( key == 'x' || key == 'X'){
    if(mode == 2){
      objects.at(index_).taller();
    }
  }
  else if ( key == 'u' || key == 'U' ){
    store_scene(g_filename);
    text_paste(g_filename, g_filename2);   // 텍스트 파일 복사.
  }
  glutPostRedisplay();
}

void special(int key, int x, int y)
{
	// TODO: properly handle special keyboard event
  if( key == GLUT_KEY_LEFT ){
    if(mode != 2){
      g_camera.move_left(0.1f);
    }
    if(mode == 2){
      objects.at(index_).move_left();
    }
  }
  else if( key == GLUT_KEY_RIGHT ){
    if(mode != 2){
      g_camera.move_right(0.1f);
    }
    if(mode == 2){
      objects.at(index_).move_right();
    }
  }
  else if( key == GLUT_KEY_DOWN ){
    if(mode != 2){
      g_camera.move_backward(0.1f);
    }
    if(mode == 2){
      objects.at(index_).move_backward();
    }
  }
  else if( key == GLUT_KEY_UP ){
    if(mode != 2){
      g_camera.move_forward(0.1f);
    }
    if(mode == 2){
      objects.at(index_).move_forward();
    }
  }
  glutPostRedisplay();
}

bool load_scene(const std::string& filename){
  std::ifstream file(filename.c_str());

  if (!file.is_open())
  {
    std::cerr << "failed to open file: " << filename << std::endl;
    return false;
  }

  std::string type_str;
  std::string line;
  std::locale loc;
  std::stringstream ss;


  while (!file.eof())
  {
    std::getline(file, line);

    ss.clear();
    ss.str(line);

    std::string name;
    std::string texture_name;
    GLfloat yr, s;
    kmuvcl::math::vec3f t;

    Object obj;

    // comment or space
    if (line[0] == '#' || std::isspace(line[0], loc))
    {
      continue; // skip
    }
    else if (line.substr(0, 2) == "o "){
      ss >> type_str >> name >> yr >> s >> t(0) >> t(1) >> t(2) >> texture_name;
      obj.naming(name);
      obj.load_simple_obj(name);
      obj.load_texture(texture_name);
      obj.set_value(t,s,yr);
      objects.push_back(obj);
    }
    else if (line.substr(0, 2) == "s "){
      ss >> type_str >> name >> yr >> s >> t(0) >> t(1) >> t(2) >> texture_name;
      obj.naming(name);
      obj.texturename(texture_name);
      obj.load_simple_obj(name);
      obj.load_texture(texture_name);
      obj.set_value(t,s,yr);
      g_selection = obj;
    }
  }
  return true;
}

bool store_scene(const std::string& filename) {
// fileVar 화일 끝에 i 값을 append한다. append = 덧붙이다, 첨가하다.
  std::ifstream ifile(filename.c_str());
  std::ofstream ofile("./scene/scene2.txt");

  if (!ifile.is_open())
  {
    std::cerr << "failed to open file: " << filename << std::endl;
    return false;
  }

  std::string type_str;
  std::string line;
  std::locale loc;
  std::stringstream ss;

  while (!ifile.eof())
  {
    std::getline(ifile, line);

    ss.clear();
    ss.str(line);

    std::string name;
    std::string texture_name;
    GLfloat yr, s;
    kmuvcl::math::vec3f t;

    Object obj;

    // comment or space
    if (line[0] == '#' || std::isspace(line[0], loc))
    {
      continue; // skip
    }
    else if (line.substr(0, 2) == "o "){
      ss >> type_str >> name >> yr >> s >> t(0) >> t(1) >> t(2) >> texture_name;
      for(int i = 0; i < objects.size(); ++i){
        if(name == objects.at(i).N()){
          ofile << type_str << " " << name << "\t" << objects.at(i).R() << "\t" << objects.at(i).S() << "\t" << objects.at(i).Tx() << " " << objects.at(i).Ty() << " " << objects.at(i).Tz() << "\t" << texture_name << "\n";
        }
      }
    }
  }
  ofile << "s " << g_selection.N() << "\t" << g_selection.R() << "\t" << g_selection.S() << "\t" << g_selection.Tx() << " " << g_selection.Ty() << " " << g_selection.Tz() << "\t" << g_selection.texture() << "\n";
  return true;
}

bool text_paste(const std::string& filename, const std::string& filename2){
  remove(filename.c_str());
  std::ifstream ifile(filename2.c_str());
  std::ofstream ofile(filename.c_str());

  if (!ifile.is_open())
  {
    std::cerr << "failed to open file: " << filename << std::endl;
    return false;
  }

  std::string type_str;
  std::string line;
  std::locale loc;
  std::stringstream ss;

  while (!ifile.eof())
  {
    std::getline(ifile, line);

    ss.clear();
    ss.str(line);

    std::string name;
    std::string texture_name;
    GLfloat yr, s;
    kmuvcl::math::vec3f t;

    Object obj;

    // comment or space
    if (line[0] == '#' || std::isspace(line[0], loc))
    {
      continue; // skip
    }
    else if (line.substr(0, 2) == "o "){
      ss >> type_str >> name >> yr >> s >> t(0) >> t(1) >> t(2) >> texture_name;
      ofile << type_str << " " << name << "\t" << yr << "\t" << s << "\t" << t(0) << " " << t(1) << " " << t(2) << "\t" << texture_name << "\n";
    }
  }
  ofile << "s " << g_selection.N() << "\t" << g_selection.R() << "\t" << g_selection.S() << "\t" << g_selection.Tx() << " " << g_selection.Ty() << " " << g_selection.Tz() << "\t" << g_selection.texture() << "\n";

}

void idle()
{
  curr = std::chrono::system_clock::now();

  std::chrono::duration<float> elaped_seconds = (curr - prev);

  model_angle += 10 * elaped_seconds.count();

  prev = curr;

  a += 1.0f;

  glutPostRedisplay();
}

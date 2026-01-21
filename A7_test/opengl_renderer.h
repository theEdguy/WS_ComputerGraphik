#ifndef OPENGL_RENDERER
#define OPENGL_RENDERER


#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <iostream>
#include "matrix.h"
#include "physics.h"
#include "game.h"
#include "renderer.h"
#include "debug.h"
#include "wavefront.h" // Add wavefront include
#include <array>
#include <vector>
#include <memory>
#include <map>

// stores information on how to render a specific vertex buffer (vbo)
// the vob's layout used by the shaderProgram is hard coded into the render() method.
class OpenGLView {
protected:
  unsigned int shaderProgram;
  size_t vertices_size;
  GLuint mode;
  GLuint vao;
public:
  OpenGLView(GLuint vbo, unsigned int shaderProgram, size_t vertices_size, GLuint mode = GL_LINE_LOOP);  

  ~OpenGLView();
    
  void render( SquareMatrix<float,4> & matrice);  
};


class TypedBodyView : public OpenGLView {
  TypedBody * typed_body;    // the body that is rendered by this view
  float scale;
  SquareMatrix4df achsen_korrektur; // Zusätzliche Rotation/Transformation für das Modell
  std::function<bool()> draw; // view is rendered iff draw() returns true
  std::function<void(TypedBodyView *)> modify; // a callback which my change this TypedBodyView, for instance, for animations
  SquareMatrix4df create_object_transformation(Vector2df direction, float angle, float scale);
public:
  TypedBodyView(TypedBody * typed_body, GLuint vbo, unsigned int shaderProgram, size_t vertices_size, float scale = 1.0f, GLuint mode = GL_LINE_LOOP,
               SquareMatrix4df achsen_korrektur = {{1.0f,0.0f,0.0f,0.0f}, {0.0f,1.0f,0.0f,0.0f}, {0.0f,0.0f,1.0f,0.0f}, {0.0f,0.0f,0.0f,1.0f}},
               std::function<bool()> draw = []() -> bool {return true;},
               std::function<void(TypedBodyView *)> modify = [](TypedBodyView *) -> void {});

  // Gibt eine 4x4 Transformationsmatrix zurück, die ein Objekt gegen den Uhrzeigersinn um den gegebenen Winkel in der x/y Ebene rotiert,
  // es skaliert und in die gegebene Richtung verschiebt
 
  void render( SquareMatrix<float,4> & world) ;
  
 TypedBody * get_typed_body();

 void set_scale(float scale);
 
};


// OpenGLRenderer is responsible for creating and opening a window for the Asteroid-Game, when init() is called.
// Each time render() is called, it draws all visible game objects, score, ...
// exit() frees view resources and closes the window
class OpenGLRenderer : public Renderer {
  std::string title;
  int window_width;
  int window_height;
  SDL_Window * window = nullptr;
  SDL_GLContext context;
  unsigned int shaderProgram;
  std::vector< std::unique_ptr<TypedBodyView > > views;
  
  // Map speichert Name -> {VBO ID, Anzahl Vertices}
  std::map<std::string, std::pair<GLuint, size_t>> model_map; 
  std::vector<GLuint> vbo_list; // Zum Aufräumen

  std::unique_ptr<OpenGLView> spaceship_view;
  std::array< std::unique_ptr<OpenGLView>, 10> digit_views;
  void createVbos();
  void createSpaceShipView();
  void createDigitViews();
  void create(Spaceship * ship, std::vector< std::unique_ptr<TypedBodyView> > & views); 
  void create(Torpedo * torpedo, std::vector< std::unique_ptr<TypedBodyView> > & views);
  void create(Asteroid * asteroid, std::vector< std::unique_ptr<TypedBodyView> > & views);
  void create(Saucer * saucer, std::vector< std::unique_ptr<TypedBodyView> > & views);
  void create(SpaceshipDebris * debris, std::vector< std::unique_ptr<TypedBodyView> > & views);
  void create(Debris * debris, std::vector< std::unique_ptr<TypedBodyView> > & views);
  void renderFreeShips(SquareMatrix4df & matrice);
  void renderScore(SquareMatrix4df & matrice);
  void create_shader_programs();
public:
  OpenGLRenderer(Game & game, std::string title, int window_width = 1024, int window_height = 768)
    : Renderer(game), title(title), window_width(window_width), window_height(window_height) { }
  
    ~OpenGLRenderer() { 
      // VBOs are cleaned up in exit() or let them leak if OS handles it on exit
  }
  
  virtual bool init();
  
  virtual void render();
  
  virtual void exit(); 
  
};

#endif

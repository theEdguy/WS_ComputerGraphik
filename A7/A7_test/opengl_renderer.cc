#include "opengl_renderer.h"
#include <cassert>
#include <span>
#include <utility>
#include "wavefront.h"
#include <fstream>
#include <sstream>

// Helper to create vertex data from WavefrontImporter
std::vector<float> create_vertices(WavefrontImporter & wi) {
  std::vector<float> vertices;
  Material default_material = { {1.0f, 1.0f, 1.0f} };
  
  for (Face face : wi.get_faces() ) {
    for (ReferenceGroup group : face.reference_groups ) {
      // Vertex
      for (size_t i = 0; i < 3; i++) {
        vertices.push_back( group.vertice[i]);
      }
      // Normal
      for (size_t i = 0; i < 3; i++) {
        vertices.push_back( group.normal[i] );
      }
      // Material / Color
      if (face.material == nullptr) face.material = &default_material;
      for (size_t i = 0; i < 3; i++) {
        vertices.push_back( face.material->ambient[i]);
      }
    } 
  }
  return vertices;
}

void scale_vertices(std::vector<float>& vertices, float factor) {
  for (size_t i = 0; i < vertices.size(); i += 9) {
    vertices[i]   *= factor;
    vertices[i+1] *= factor;
    vertices[i+2] *= factor;
  }
}

// Helper to rotate vertices 90 degrees around Y axis (Z -> X)
// Used to align Z-oriented models (like spaceship/torpedo) to X-axis (0 degrees in game)
void rotate_vertices_y_90(std::vector<float>& vertices) {
  for (size_t i = 0; i < vertices.size(); i += 9) {
    float x = vertices[i];
    float z = vertices[i+2];
    
    // Rotate 90 degrees around Y: (x,y,z) -> (z, y, -x)
    vertices[i]   = z;
    vertices[i+2] = -x;
    
    // Rotate normals too
    float nx = vertices[i+3];
    float nz = vertices[i+5];
    vertices[i+3] = nz;
    vertices[i+5] = -nx;
  }
}

void rotate_vertices(std::vector<float>& vertices, const SquareMatrix4df& mat) {
  for (size_t i = 0; i < vertices.size(); i += 9) {
     // Pos
     Vector4df p = {vertices[i], vertices[i+1], vertices[i+2], 1.0f};
     Vector4df p_new = mat * p;
     vertices[i]   = p_new[0];
     vertices[i+1] = p_new[1];
     vertices[i+2] = p_new[2];
     
     // Normal (ignore translation, assume orthogonal rotation)
     Vector4df n = {vertices[i+3], vertices[i+4], vertices[i+5], 0.0f};
     Vector4df n_new = mat * n;
     vertices[i+3] = n_new[0];
     vertices[i+4] = n_new[1];
     vertices[i+5] = n_new[2];
  }
}

// Helper to rotate vertices around Y axis
void rotate_vertices_y(std::vector<float>& vertices, float angle_deg) {
  float rad = angle_deg * 3.14159f / 180.0f;
  float c = std::cos(rad);
  float s = std::sin(rad);
  for (size_t i = 0; i < vertices.size(); i += 9) {
    float x = vertices[i];
    float z = vertices[i+2];
    vertices[i]   = x * c + z * s;
    vertices[i+2] = -x * s + z * c;
    
    // Rotate normal too
    float nx = vertices[i+3];
    float nz = vertices[i+5];
    vertices[i+3] = nx * c + nz * s;
    vertices[i+5] = -nx * s + nz * c;
  }
}

// Helper to rotate vertices around X axis
void rotate_vertices_x(std::vector<float>& vertices, float angle_deg) {
  float rad = angle_deg * 3.14159f / 180.0f;
  float c = std::cos(rad);
  float s = std::sin(rad);
  for (size_t i = 0; i < vertices.size(); i += 9) {
    float y = vertices[i+1];
    float z = vertices[i+2];
    vertices[i+1] = y * c - z * s;
    vertices[i+2] = y * s + z * c;

    // Normal
    float ny = vertices[i+4];
    float nz = vertices[i+5];
    vertices[i+4] = ny * c - nz * s;
    vertices[i+5] = ny * s + nz * c;
  }
}

// Helper to convert 2D legacy data to 3D vertex format
std::vector<float> convert_2d_to_3d(const std::vector<Vector2df>& input) {
  std::vector<float> output;
  for (const auto& v : input) {
    // Pos
    output.push_back(v[0]);
    output.push_back(v[1]);
    output.push_back(0.0f);
    // Normal (0,0,1)
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(1.0f);
    // Color (1,1,1)
    output.push_back(1.0f);
    output.push_back(1.0f);
    output.push_back(1.0f);
  }
  return output;
}

// geometric data as in original game and game coordinates
std::vector<Vector2df> spaceship = {
  Vector2df{-6.0f,  3.0f},
  Vector2df{-6.0f, -3.0f},
  Vector2df{-10.0f, -6.0f},
  Vector2df{14.0f,  0.0f},
  Vector2df{-10.0f,  6.0f},
  Vector2df{ -6.0f,  3.0f}
};

std::vector<Vector2df> flame = { 
  Vector2df{-6, 3},
  Vector2df{-12, 0},
  Vector2df{-6, -3}
};

std::vector<Vector2df> torpedo_points = { 
  Vector2df{0, 0},
  Vector2df{0, 1}
};

std::vector<Vector2df> saucer_points = {
  Vector2df{-16, -6},
  Vector2df{16, -6}, 
  Vector2df{40, 6}, 
  Vector2df{-40, 6},
  Vector2df{-16, 18},
  Vector2df{16, 18},
  Vector2df{40, 6},
  Vector2df{16, -6},
  Vector2df{8, -18},
  Vector2df{-8, -18},
  Vector2df{-16, -6},
  Vector2df{-40, 6}
};

std::vector<Vector2df> asteroid_1 = {
  Vector2df{ 0, -12},
  Vector2df{16, -24},
  Vector2df{32, -12},
  Vector2df{24, 0},
  Vector2df{32, 12},
  Vector2df{8, 24}, 
  Vector2df{-16, 24}, 
  Vector2df{-32, 12}, 
  Vector2df{-32, -12}, 
  Vector2df{-16, -24},
  Vector2df{0, -12}
};

std::vector<Vector2df> asteroid_2 = {
  Vector2df{6, -6},
  Vector2df{32, -12},
  Vector2df{16, -24}, 
  Vector2df{0, -16}, 
  Vector2df{-16, -24}, 
  Vector2df{-24, -12},
  Vector2df{-16, -0}, 
  Vector2df{-32, 12}, 
  Vector2df{-16, 24}, 
  Vector2df{-8, 16}, 
  Vector2df{16, 24}, 
  Vector2df{32, 6}, 
  Vector2df{16, -6},
};

std::vector<Vector2df> asteroid_3 = {
  Vector2df{-16, 0}, 
  Vector2df{-32, 6}, 
  Vector2df{-16, 24}, 
  Vector2df{0, 6}, 
  Vector2df{0, 24}, 
  Vector2df{16, 24},
  Vector2df{32, 6}, 
  Vector2df{32, 6}, 
  Vector2df{16, -24}, 
  Vector2df{-8, -24}, 
  Vector2df{-32, -6}
};

std::vector<Vector2df> asteroid_4 = {
  Vector2df{8,0}, 
  Vector2df{32,-6}, 
  Vector2df{32, -12}, 
  Vector2df{8, -24}, 
  Vector2df{-16, -24}, 
  Vector2df{-8, -12}, 
  Vector2df{-32, -12}, 
  Vector2df{-32, 12}, 
  Vector2df{-16, 24}, 
  Vector2df{8, 16}, 
  Vector2df{16, 24}, 
  Vector2df{32, 12}, 
  Vector2df{8, 0}
};

std::vector<Vector2df> spaceship_debris = {
  Vector2df{-2, -1}, 
  Vector2df{-10, 7}, 
  Vector2df{3, 1}, 
  Vector2df{7, 8},
  Vector2df{0, 3}, 
  Vector2df{6, 1},
  Vector2df{3, -1}, 
  Vector2df{ -5, -7},
  Vector2df{0, -4}, 
  Vector2df{-6, -6},
  Vector2df{-2, 2}, 
  Vector2df{2, 5}
};
    
std::vector<Vector2df> spaceship_debris_direction = {
 Vector2df{-40, -23},
 Vector2df{50, 15},
 Vector2df{0, 45},
 Vector2df{60, -15}, 
 Vector2df{10, -52}, 
 Vector2df{-40, 30}
};

std::vector<Vector2df> debris_points = {
 Vector2df{-32, 32}, 
 Vector2df{-32, -16}, 
 Vector2df{-16, 0}, 
 Vector2df{-16, -32}, 
 Vector2df{-8, 24},
 Vector2df{8, -24}, 
 Vector2df{24, 32}, 
 Vector2df{24, -24}, 
 Vector2df{24, -32}, 
 Vector2df{32, -8}
};
        
std::vector<Vector2df> digit_0 = { {0,-8}, {4,-8}, {4,0}, {0,0}, {0, -8} };
std::vector<Vector2df> digit_1 = { {4,0}, {4,-8} };
std::vector<Vector2df> digit_2 = { {0,-8}, {4,-8}, {4,-4}, {0,-4}, {0,0}, {4,0}  };
std::vector<Vector2df> digit_3 = { {0,0}, {4, 0}, {4,-4}, {0,-4}, {4,-4}, {4, -8}, {0, -8}  };
std::vector<Vector2df> digit_4 = { {4,0}, {4,-8}, {4,-4}, {0,-4}, {0,-8}  };
std::vector<Vector2df> digit_5 = { {0,0}, {4,0}, {4,-4}, {0,-4}, {0,-8}, {4, -8}  };
std::vector<Vector2df> digit_6 = { {0,-8}, {0,0}, {4,0}, {4,-4}, {0,-4} };
std::vector<Vector2df> digit_7 = { {0,-8}, {4,-8}, {4,0} };
std::vector<Vector2df> digit_8 = { {0,-8}, {4,-8}, {4,0}, {0,0}, {0,-8}, {0, -4}, {4, -4} };
std::vector<Vector2df> digit_9 = { {4, 0}, {4,-8}, {0,-8}, {0, -4}, {4, -4} };
       
// std::vector< std::vector<Vector2df> * > vertice_data = {
//   &spaceship, &flame,
//   &torpedo_points, &saucer_points,
//   &asteroid_1, &asteroid_2, &asteroid_3, &asteroid_4,
//   &spaceship_debris, &spaceship_debris_direction,
//   &debris_points,
//   &digit_0, &digit_1, &digit_2, &digit_3, &digit_4, &digit_5, &digit_6, &digit_7, &digit_8, &digit_9 };                                  


// class OpenGLView

  OpenGLView::OpenGLView(GLuint vbo, unsigned int shaderProgram, size_t vertices_size, GLuint mode)
    : shaderProgram(shaderProgram), vertices_size(vertices_size), mode(mode) {
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    size_t stride = 9 * sizeof(float);
    // Pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    // Color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  OpenGLView::~OpenGLView() {
   glDeleteVertexArrays(1, &vao);
  }

  void OpenGLView::render( SquareMatrix<float,4> & mvp, const SquareMatrix<float,4> & model) {
    debug(2, "render() entry...");
    glBindVertexArray(vao);
    glUseProgram(shaderProgram);
    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &mvp[0][0] );
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0] );
    glDrawArrays(mode, 0, vertices_size );
    debug(2, "render() exit.");
  }

// class TypedBodyView

  TypedBodyView::TypedBodyView(TypedBody * typed_body, GLuint vbo, unsigned int shaderProgram, size_t vertices_size, float scale, GLuint mode,
               std::function<bool()> draw, std::function<void(TypedBodyView *)> modify)
        : OpenGLView(vbo, shaderProgram, vertices_size, mode),  typed_body(typed_body), scale(scale), draw(draw), modify(modify) {
  }
  
  SquareMatrix4df TypedBodyView::create_object_transformation(Vector2df direction, float angle, float scale) {
    SquareMatrix4df translation = make_translation_matrix({direction[0], direction[1], 0.0f});
    SquareMatrix4df rotation = make_rotation_z_matrix(angle);
    SquareMatrix4df scaling = make_scale_matrix({scale, scale, scale});
    
    return translation * rotation * scaling;
  }

  void TypedBodyView::render( SquareMatrix<float,4> & vp, const SquareMatrix<float,4> & tile_transform) {
    debug(2, "render() entry...");
    if ( draw() ) {
      modify(this);
      SquareMatrix4df object_transform = create_object_transformation(typed_body->get_position(), typed_body->get_angle(), scale);
      SquareMatrix4df model = tile_transform * object_transform;
      SquareMatrix4df mvp = vp * model;
      OpenGLView::render(mvp, model);
    }
    debug(2, "render() exit.");
  }
  
 TypedBody * TypedBodyView::get_typed_body() {
   return typed_body;
 }

 void TypedBodyView::set_scale(float scale) {
   this->scale = scale;
 }

// class OpenGLRenderer

void OpenGLRenderer::createVbos() {
 vbos = new GLuint[vertex_data.size()];
 glGenBuffers(vertex_data.size(), vbos);

 for (size_t i = 0; i < vertex_data.size(); i++) {
   glBindBuffer(GL_ARRAY_BUFFER, vbos[i]);
   glBufferData(GL_ARRAY_BUFFER, vertex_data[i].size() * sizeof(float), vertex_data[i].data(), GL_STATIC_DRAW);
 }
}

void OpenGLRenderer::create(Spaceship * ship, std::vector< std::unique_ptr<TypedBodyView> > & views) {
  debug(4, "create(Spaceship *) entry...");

  views.push_back(std::make_unique<TypedBodyView>(ship, vbos[0], shaderProgram, vertex_data[0].size() / 9, 1.0f, GL_TRIANGLES,
                  [ship]() -> bool {return ! ship->is_in_hyperspace();}) // only show ship if outside hyperspace
                 );   
  views.push_back(std::make_unique<TypedBodyView>(ship, vbos[1], shaderProgram, vertex_data[1].size() / 9, 1.0f, GL_LINE_LOOP,
                  [ship]() -> bool {return ! ship->is_in_hyperspace() && ship->is_accelerating();}) // only show flame if accelerating
                 );   
  
  debug(4, "create(Spaceship *) exit.");
}

void OpenGLRenderer::create(Saucer * saucer, std::vector< std::unique_ptr<TypedBodyView> > & views) {
  debug(4, "create(Saucer *) entry...");
  float scale = 0.5;
  if ( saucer->get_size() == 0 ) {
    scale = 0.25;
  }
  views.push_back(std::make_unique<TypedBodyView>(saucer, vbos[3], shaderProgram, vertex_data[3].size() / 9, scale, GL_TRIANGLES));   
  debug(4, "create(Saucer *) exit.");
}


void OpenGLRenderer::create(Torpedo * torpedo, std::vector< std::unique_ptr<TypedBodyView> > & views) {
  debug(4, "create(Torpedo *) entry...");
  views.push_back(std::make_unique<TypedBodyView>(torpedo, vbos[2], shaderProgram, vertex_data[2].size() / 9, 1.0f)); 
  debug(4, "create(Torpedo *) exit.");
}

void OpenGLRenderer::create(Asteroid * asteroid, std::vector< std::unique_ptr<TypedBodyView> > & views) {
  debug(4, "create(Asteroid *) entry...");
  GLuint rock_vbo_index = 4 +  asteroid->get_rock_type();

  float scale = (asteroid->get_size() == 3 ? 1.0 : ( asteroid->get_size() == 2 ? 0.5 : 0.25 ));
 
  views.push_back(std::make_unique<TypedBodyView>(asteroid, vbos[rock_vbo_index], shaderProgram, vertex_data[rock_vbo_index].size() / 9, scale, GL_TRIANGLES)); 
  debug(4, "create(Asteroid *) exit.");
}

void OpenGLRenderer::create(SpaceshipDebris * debris, std::vector< std::unique_ptr<TypedBodyView> > & views) {
  debug(4, "create(SpaceshipDebris *) entry...");
  views.push_back(std::make_unique<TypedBodyView>(debris, vbos[10], shaderProgram, vertex_data[10].size() / 9, 0.1f, GL_POINTS,
            []() -> bool {return true;},
            [debris](TypedBodyView * view) -> void { view->set_scale( 0.2f * (SpaceshipDebris::TIME_TO_DELETE - debris->get_time_to_delete()));}));   
  debug(4, "create(SpaceshipDebris *) exit.");
}

void OpenGLRenderer::create(Debris * debris, std::vector< std::unique_ptr<TypedBodyView> > & views) {
  debug(4, "create(Debris *) entry...");
  views.push_back(std::make_unique<TypedBodyView>(debris, vbos[10], shaderProgram, vertex_data[10].size() / 9, 0.1f, GL_POINTS,
            []() -> bool {return true;},
            [debris](TypedBodyView * view) -> void { view->set_scale(Debris::TIME_TO_DELETE - debris->get_time_to_delete());}));   
  debug(4, "create(Debris *) exit.");
}

void OpenGLRenderer::createSpaceShipView() {
  spaceship_view = std::make_unique<OpenGLView>(vbos[0], shaderProgram, vertex_data[0].size() / 9, GL_TRIANGLES);
}

void OpenGLRenderer::createDigitViews() {
  for (size_t i = 0; i < 10; i++ ) {
    digit_views[i] = std::make_unique<OpenGLView>(vbos[11 + i], shaderProgram, vertex_data[11 + i].size() / 9, GL_LINE_STRIP);
  }
}


void OpenGLRenderer::renderFreeShips(SquareMatrix4df & matrice) {
  constexpr float FREE_SHIP_X = 128;
  constexpr float FREE_SHIP_Y = 64;
  const float PIf = static_cast<float> ( PI );
  Vector2df position = {FREE_SHIP_X, FREE_SHIP_Y};
  SquareMatrix4df rotation = {   { std::cos(-PIf / 2.0f),  std::sin(-PIf / 2.0f), 0.0f, 0.0f},
                                 {-std::sin(-PIf / 2.0f),  std::cos(-PIf / 2.0f), 0.0f, 0.0f},
                                 { 0.0f,                 0.0f,                1.0f, 0.0f},
                                 { 0.0f,                 0.0f,                0.0f, 1.0f}
                               };
  SquareMatrix4df identity = make_identity_matrix();
  for (int i = 0; i < game.get_no_of_ships(); i++) {
    SquareMatrix4df  translation= { {1.0f,        0.0f,         0.0f, 0.0f},
                                    {0.0f,        1.0f,         0.0f, 0.0f},
                                    {0.0f,        0.0f,         1.0f, 0.0f},
                                    {position[0], position[1],  0.0f, 1.0f} };
    SquareMatrix4df render_matrice = matrice * translation * rotation;
    spaceship_view->render( render_matrice, identity);
    position[0] += 20.0;
  }
}

void OpenGLRenderer::renderScore(SquareMatrix4df & matrice) {
  constexpr float SCORE_X = 128 - 48;
  constexpr float SCORE_Y = 48 - 4;
  

  long long score = game.get_score();
  int no_of_digits = 0;
  if (score > 0) {
    no_of_digits = std::trunc( std::log10( score ) ) + 1;
  }

  Vector2df position = {SCORE_X + 20.0f * no_of_digits,  SCORE_Y};
  SquareMatrix4df identity = make_identity_matrix();
  do {
    int d = score % 10;
    score /= 10;
    SquareMatrix4df scale_translation= { {4.0f,        0.0f,         0.0f, 0.0f},
                                         {0.0f,        4.0f,         0.0f, 0.0f},
                                         {0.0f,        0.0f,         1.0f, 0.0f},
                                         {position[0], position[1],  0.0f, 1.0f} };
    SquareMatrix4df render_matrice = matrice * scale_translation;
    digit_views[d]->render( render_matrice, identity );
    no_of_digits--;
    position[0] -= 20;

  } while (no_of_digits > 0);
}


void OpenGLRenderer::create_shader_programs() {

static const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec3 innormal;\n"
    "layout (location = 2) in vec3 incolor;\n"
    "out vec3 color;\n"
    "out vec4 normal;\n"
    "uniform mat4 transform;\n"
    "uniform mat4 model;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = transform * vec4(position, 1.0);\n"
    "   color = incolor;\n"
    "   normal = normalize( model * vec4(innormal, 0.0));\n"
    "}\0";
static const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 color;\n"
    "in vec4 normal;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(color * (0.3 + 0.7 * max(0.0, dot(normal, normalize( vec4(0.2, 0.2, 1.0, 0.0))))) , 1.0);\n"
    "}\n\0";

    // build and compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        error( std::string("vertex shader compilation failed") + infoLog);
    }
    // build and compiler fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        error( std::string("fragment shader compilation failed") + infoLog);
    }
    

    // link both shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        error( std::string("linking shader programs failed") + infoLog);
    }
}



bool OpenGLRenderer::init() {
  if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
    error( std::string("Could not initialize SDL. SDLError: ") + SDL_GetError() );
  } else {
    window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
    if( window == nullptr ) {
      error( std::string("Could not create Window. SDLError: ") + SDL_GetError() );
    } else {
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG );
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

      context = SDL_GL_CreateContext(window);
      
      GLenum err = glewInit(); // to be called after OpenGL render context is created
      if (GLEW_OK != err) {
        error( "Could not initialize Glew. Glew error message: " );
        error( glewGetErrorString(err) );
      }
      debug(1, "Using GLEW Version: ");
      debug(1, glewGetString(GLEW_VERSION) );

      SDL_GL_SetSwapInterval(1);

      create_shader_programs();
      
      // Load Assets
      vertex_data.clear();
      // 0: Spaceship
      try {
        std::ifstream in("space_ship.obj");
        WavefrontImporter ship_importer(in);
        ship_importer.parse();
        auto verts = create_vertices(ship_importer);
        
        // Correct orientation: Nose -Z -> +X, Top +Y -> +Z, Right +X -> -Y
        SquareMatrix4df rot = {
           Vector4df{ 0.0f, -1.0f, 0.0f, 0.0f},
           Vector4df{ 0.0f,  0.0f, 1.0f, 0.0f},
           Vector4df{-1.0f,  0.0f, 0.0f, 0.0f},
           Vector4df{ 0.0f,  0.0f, 0.0f, 1.0f}
        };
        rotate_vertices(verts, rot);
        scale_vertices(verts, 20.0f);
        vertex_data.push_back(verts);
      } catch(...) {
         // Fallback if file missing
         vertex_data.push_back(convert_2d_to_3d(spaceship));
      }
      
      // 1: Flame
      vertex_data.push_back(convert_2d_to_3d(flame));
      
      // 2: Torpedo
      try {
        std::ifstream in("torpedo.obj");
        WavefrontImporter torpedo_importer(in);
        torpedo_importer.parse();
        auto torpedo_verts = create_vertices(torpedo_importer);
        rotate_vertices_y_90(torpedo_verts);
        scale_vertices(torpedo_verts, 25.0f);
        vertex_data.push_back(torpedo_verts);
      } catch(...) {
        auto torpedo_verts = convert_2d_to_3d(torpedo_points);
        scale_vertices(torpedo_verts, 30.0f);
        vertex_data.push_back(torpedo_verts);
      }
      
      // 3: Saucer
      try {
         std::ifstream in("ufo.obj");
         WavefrontImporter ufo_importer(in);
         ufo_importer.parse();
         auto verts = create_vertices(ufo_importer);
         scale_vertices(verts, 80.0f);
         vertex_data.push_back(verts);
      } catch(...) {
         vertex_data.push_back(convert_2d_to_3d(saucer_points));
      }
      
      // 4-7: Asteroids
      try {
        std::ifstream in("asteroid.obj");
        WavefrontImporter asteroid_importer(in);
        asteroid_importer.parse();
        std::vector<float> asteroid_verts = create_vertices(asteroid_importer);
        scale_vertices(asteroid_verts, 30.0f);
        for(int i=0; i<4; ++i) vertex_data.push_back(asteroid_verts);
      } catch(...) {
         vertex_data.push_back(convert_2d_to_3d(asteroid_1));
         vertex_data.push_back(convert_2d_to_3d(asteroid_2));
         vertex_data.push_back(convert_2d_to_3d(asteroid_3));
         vertex_data.push_back(convert_2d_to_3d(asteroid_4));
      }
      
      // 8: Spaceship Debris
      vertex_data.push_back(convert_2d_to_3d(spaceship_debris));
      
      // 9: Spaceship Debris Direction
      vertex_data.push_back(convert_2d_to_3d(spaceship_debris_direction));
      
      // 10: Debris
      vertex_data.push_back(convert_2d_to_3d(debris_points));
      
      // 11-20: Digits
      vertex_data.push_back(convert_2d_to_3d(digit_0));
      vertex_data.push_back(convert_2d_to_3d(digit_1));
      vertex_data.push_back(convert_2d_to_3d(digit_2));
      vertex_data.push_back(convert_2d_to_3d(digit_3));
      vertex_data.push_back(convert_2d_to_3d(digit_4));
      vertex_data.push_back(convert_2d_to_3d(digit_5));
      vertex_data.push_back(convert_2d_to_3d(digit_6));
      vertex_data.push_back(convert_2d_to_3d(digit_7));
      vertex_data.push_back(convert_2d_to_3d(digit_8));
      vertex_data.push_back(convert_2d_to_3d(digit_9));

      createVbos();
      createSpaceShipView();
      createDigitViews();
      return true;
    }
  }
  return false;
}

/* tile positions in world coordinates
   used to draw objects seemless between boundary
  +---+---+---+   
  | 5 | 7 | 2 |
  +---+---+---+
  | 4 | 0 | 1 |
  +---+---+---+
  | 6 | 8 | 3 |
  +---+---+---+
*/
static Vector2df tile_positions [] = {
                         {0.0f, 0.0f},
                         {1024.0f, 0.0f},
                         {1024.0f, 768.0f},
                         {1024.0f, -768.0f},
                         {-1024.0f, 0.0f},
                         {-1024.0f, 768.0f},
                         {-1024.0f, -768.0f},
                         {0.0f, 768.0f},
                         {0.0f, -768.0f} };

void OpenGLRenderer::render() {
  debug(2, "render() entry...");

//name
  // transformation to canonical view and from left handed to right handed coordinates
/*   SquareMatrix4df world_transformation =
                         SquareMatrix4df{
                           { 2.0f / 1024.0f,           0.0f,            0.0f,  0.0f},
                           {       0.0f,     -2.0f / 768.0f,            0.0f,  0.0f}, // (negative, because we have a left handed world coord. system)
                           {       0.0f,               0.0f,  2.0f / 1024.0f,  0.0f},
                           {      -1.0f,               1.0f,           -1.0f,  1.0f}
                         }; */
                                                 
  glClearColor ( 0.0, 0.0, 0.0, 1.0 );
  glClear ( GL_COLOR_BUFFER_BIT );
  
  debug(2, "remove views for deleted objects");

  // remove all views for typed bodies that have to be deleted 
  erase_if(views, []( std::unique_ptr<TypedBodyView> & view) { return view->get_typed_body()->is_marked_for_deletion();}); 

  auto new_bodies = game.get_physics().get_recently_added_bodies();
  for (Body2df * body : new_bodies) {
    assert(body != nullptr);
    TypedBody * typed_body = static_cast<TypedBody *>(body);
    auto type = typed_body->get_type();
    if (type == BodyType::spaceship) {
      create( static_cast<Spaceship *>(typed_body), views );
    } else if (type == BodyType::torpedo ) {
      create( static_cast<Torpedo *>(typed_body), views );
    } else  if (type == BodyType::asteroid) {
      create( static_cast<Asteroid *>(typed_body), views );
    } else if (type == BodyType::saucer) {
      create( static_cast<Saucer *>(typed_body), views );
    } else if (type == BodyType::spaceship_debris ) {
      create( static_cast<SpaceshipDebris *>(typed_body), views );
    } else if (type == BodyType::debris) {
      create( static_cast<Debris *>(typed_body), views );
    }
  }

  // 1. Projection
  static const float PI_F = (float)PI;
  float fov = 45.0f * PI_F / 180.0f;
  float aspect = (float)window_width / (float)window_height;
  SquareMatrix4df projection = make_perspective_projection_matrix(fov, aspect, 1.0f, 2000.0f);
  
  // 2. View - Camera follows ship
  Vector2df ship_pos = {512.0f, 384.0f};
  if (game.ship_exists() && game.get_ship() != nullptr) {
      ship_pos = game.get_ship()->get_position();
  } 
  
  Vector3df eye = {ship_pos[0], ship_pos[1], 800.0f};
  Vector3df center = {ship_pos[0], ship_pos[1], 0.0f};
  Vector3df up = {0.0f, 1.0f, 0.0f};
  
  SquareMatrix4df view = make_look_at_matrix(eye, center, up);
  
  SquareMatrix4df vp = projection * view;

  // 9-fach Tiling
  for (int t = 0; t < 9; ++t) {
    SquareMatrix4df tile_transform = make_translation_matrix({tile_positions[t][0], tile_positions[t][1], 0.0f});
    for (auto & view : views) {
      view->render(vp, tile_transform);
    }
  }

  // HUD (Score, Lives) - Use simple 2D projection
  glClear(GL_DEPTH_BUFFER_BIT);
   SquareMatrix4df canonical_transform =
                         SquareMatrix4df{
                           { 2.0f / 1024.0f,           0.0f,            0.0f,  0.0f},
                           {       0.0f,     -2.0f / 768.0f,            0.0f,  0.0f},
                           {       0.0f,               0.0f,  1.0f,  0.0f}, 
                           {      -1.0f,               1.0f,           0.0f,  1.0f}
                         };
  renderFreeShips(canonical_transform);
  renderScore(canonical_transform);

  SDL_GL_SwapWindow(window);
}

void OpenGLRenderer::exit() {
  views.clear();
  glDeleteBuffers(vertex_data.size(), vbos);
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow( window );
  SDL_Quit();
}
 

#include "opengl_renderer.h"
#include <cassert>
#include <span>
#include <utility>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>

// --- Hilfsfunktionen ---

// Erstellt ein VBO aus einer OBJ-Datei.
// Gibt {VBO_ID, Vertex_Count} zurück
std::pair<GLuint, size_t> erstelle_vbo_von_obj(const std::string& dateiname) {
    // Versuch, lokal oder im Ordner A7_test zu öffnen
    std::string pfad = dateiname;
    std::ifstream eingabe_datei(pfad);
    if (!eingabe_datei.is_open()) {
        pfad = "A7_test/" + dateiname;
        eingabe_datei.open(pfad);
        if (!eingabe_datei.is_open()) {
            std::cerr << "Fehler: Konnte OBJ-Datei " << dateiname << " nicht öffnen" << std::endl;
            // Gib leer zurück, um Absturz zu vermeiden, auch wenn nichts gerendert wird
            return {0, 0}; 
        }
    }

    WavefrontImporter importer(eingabe_datei);
    importer.parse();
    
    std::vector<float> puffer_daten;
    
    // Verschränkt: Pos(3), Normal(3), Farbe(3)
    for (const auto& face : importer.get_faces()) {
        const auto& mat = face.material;
        Color farbe = {1.0f, 1.0f, 1.0f}; 
        if (mat) {
            farbe = mat->ambient;
        }

        for (const auto& ref : face.reference_groups) {
            // Position
            puffer_daten.push_back(ref.vertice[0]);
            puffer_daten.push_back(ref.vertice[1]);
            puffer_daten.push_back(ref.vertice[2]);
            
            // Normale
            puffer_daten.push_back(ref.normal[0]);
            puffer_daten.push_back(ref.normal[1]);
            puffer_daten.push_back(ref.normal[2]);

            // Farbe
            puffer_daten.push_back(farbe[0]);
            puffer_daten.push_back(farbe[1]);
            puffer_daten.push_back(farbe[2]);
        }
    }
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, puffer_daten.size() * sizeof(float), puffer_daten.data(), GL_STATIC_DRAW);
    
    return {vbo, puffer_daten.size() / 9}; 
}

// Erstellt ein VBO aus alten 2D-Vektoren (auf 3D erweitert)
std::pair<GLuint, size_t> erstelle_vbo_von_2d(const std::vector<Vector2df>& punkte) {
    std::vector<float> puffer_daten;
    // Für Linien/Punkte duplizieren wir einfach die Vertices
    // Wir nehmen weiße Farbe und Z=0, Normale=(0,0,1) an
    
    for (const auto& p : punkte) {
        // Pos
        puffer_daten.push_back(p[0]);
        puffer_daten.push_back(p[1]);
        puffer_daten.push_back(0.0f);
        
        // Normale
        puffer_daten.push_back(0.0f);
        puffer_daten.push_back(0.0f);
        puffer_daten.push_back(1.0f);
        
        // Farbe
        puffer_daten.push_back(1.0f);
        puffer_daten.push_back(1.0f);
        puffer_daten.push_back(1.0f);
    }
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, puffer_daten.size() * sizeof(float), puffer_daten.data(), GL_STATIC_DRAW);
    
    return {vbo, puffer_daten.size() / 9};
}

// Legacy Digit Data for Score
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

/* ORIGINAL CODE:
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
       
std::vector< std::vector<Vector2df> * > vertice_data = {
  &spaceship, &flame,
  &torpedo_points, &saucer_points,
  &asteroid_1, &asteroid_2, &asteroid_3, &asteroid_4,
  &spaceship_debris, &spaceship_debris_direction,
  &debris_points,
  &digit_0, &digit_1, &digit_2, &digit_3, &digit_4, &digit_5, &digit_6, &digit_7, &digit_8, &digit_9 }; 
*/


// --- OpenGLView Implementierung ---

OpenGLView::OpenGLView(GLuint vbo, unsigned int shaderProgram, size_t vertices_size, GLuint mode)
: shaderProgram(shaderProgram), vertices_size(vertices_size), mode(mode) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Stride = 9 * float (Pos3, Norm3, Col3)
    GLsizei stride = 9 * sizeof(float);
    
    // Loc 0: Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    
    // Loc 2: Normale (Offset 3 floats) 
    // Wir haben Normale/Farbe im Buffer getauscht im Vergleich zur typischen Konvention:
    // Buffer: Pos(3), Normale(3), Farbe(3)
    // Shader: Loc 0=Pos, Loc 1=Farbe, Loc 2=Normale
    
    // Normale (Loc 2) -> Offset 3 floats (Index 3,4,5)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Farbe (Loc 1) -> Offset 6 floats (Index 6,7,8)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

OpenGLView::~OpenGLView() {
    glDeleteVertexArrays(1, &vao);
}

void OpenGLView::render( SquareMatrix<float,4> & matrice) {
    glBindVertexArray(vao);
    glUseProgram(shaderProgram);
    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &matrice[0][0] ); 
    glDrawArrays(mode, 0, vertices_size );
}


d

TypedBodyView::TypedBodyView(TypedBody * typed_body, GLuint vbo, unsigned int shaderProgram, size_t vertices_size, float scale, GLuint mode, SquareMatrix4df achsen_korrektur,
            std::function<bool()> draw, std::function<void(TypedBodyView *)> modify)
    : OpenGLView(vbo, shaderProgram, vertices_size, mode),  typed_body(typed_body), scale(scale), achsen_korrektur(achsen_korrektur), draw(draw), modify(modify) {
}

SquareMatrix4df TypedBodyView::create_object_transformation(Vector2df direction, float angle, float scale) {
    SquareMatrix4df  translation= { {1.0f,        0.0f,          0.0f, 0.0f},
                                    {0.0f,        1.0f,          0.0f, 0.0f},
                                    {0.0f,        0.0f,          1.0f, 0.0f},
                                    {direction[0], direction[1], 0.0f, 1.0f}
                                };
    // Drehung um Z-Achse (Lenkung)
    SquareMatrix4df rotation = { { std::cos(angle),  std::sin(angle), 0.0f, 0.0f},
                                {-std::sin(angle),  std::cos(angle), 0.0f, 0.0f},
                                { 0.0f,             0.0f,            1.0f, 0.0f},
                                { 0.0f,             0.0f,            0.0f, 1.0f}
                            };
    SquareMatrix4df  scaling = { { scale,    0.0f, 0.0f,   0.0f},
                                {  0.0f,   scale, 0.0f,   0.0f},
                                {  0.0f,    0.0f, scale,  0.0f},
                                {  0.0f,    0.0f, 0.0f,   1.0f}
                            };                                 

    // Reihenfolge: Verschieben -> Rotieren(Z) -> Skalieren -> Achsenkorrektur(Modell-Raum)
    return translation * rotation * scaling * achsen_korrektur;
}

/* ORIGINAL CODE:
  SquareMatrix4df TypedBodyView::create_object_transformation(Vector2df direction, float angle, float scale) {
    SquareMatrix4df  translation= { {1.0f,        0.0f,          0.0f, 0.0f},
                                    {0.0f,        1.0f,          0.0f, 0.0f},
                                    {0.0f,        0.0f,          1.0f, 0.0f},
                                    {direction[0], direction[1], 0.0f, 1.0f}
                                  };
    SquareMatrix4df rotation = { { std::cos(angle),  std::sin(angle), 0.0f, 0.0f},
                                 {-std::sin(angle),  std::cos(angle), 0.0f, 0.0f},
                                 { 0.0f,             0.0f,            1.0f, 0.0f},
                                 { 0.0f,             0.0f,            0.0f, 1.0f}
                               };
    SquareMatrix4df  scaling = { { scale,    0.0f, 0.0f,   0.0f},
                                 {  0.0f,   scale, 0.0f,   0.0f},
                                 {  0.0f,    0.0f, scale,  0.0f},
                                 {  0.0f,    0.0f, 0.0f,   1.0f}
                               };                                 

    return translation * rotation * scaling;
  }
*/

void TypedBodyView::render( SquareMatrix<float,4> & world) {
    if ( draw() ) {
        modify(this);
        auto transform = world * create_object_transformation(typed_body->get_position(), typed_body->get_angle(), scale);
        OpenGLView::render(transform);
    }
}

TypedBody * TypedBodyView::get_typed_body() {
    return typed_body;
}

void TypedBodyView::set_scale(float scale) {
    this->scale = scale;
}


// --- OpenGLRenderer Implementierung ---

void OpenGLRenderer::createVbos() {
    // Lade 3D Modelle
    model_map["spaceship"] = erstelle_vbo_von_obj("space_ship.obj");
    model_map["asteroid"] = erstelle_vbo_von_obj("asteroid.obj");
    model_map["torpedo"] = erstelle_vbo_von_obj("torpedo.obj");
    model_map["saucer"] = erstelle_vbo_von_obj("ufo.obj");
    
    // Benutze 'torpedo' oder 'asteroid' für Trümmer, falls kein spezielles Trümmer-Objekt existiert
    model_map["debris"] = erstelle_vbo_von_obj("asteroid.obj"); // Wiederverwendung

    // Lade 2D Ziffern
    model_map["digit_0"] = erstelle_vbo_von_2d(digit_0);
    model_map["digit_1"] = erstelle_vbo_von_2d(digit_1);
    model_map["digit_2"] = erstelle_vbo_von_2d(digit_2);
    model_map["digit_3"] = erstelle_vbo_von_2d(digit_3);
    model_map["digit_4"] = erstelle_vbo_von_2d(digit_4);
    model_map["digit_5"] = erstelle_vbo_von_2d(digit_5);
    model_map["digit_6"] = erstelle_vbo_von_2d(digit_6);
    model_map["digit_7"] = erstelle_vbo_von_2d(digit_7);
    model_map["digit_8"] = erstelle_vbo_von_2d(digit_8);
    model_map["digit_9"] = erstelle_vbo_von_2d(digit_9);
}

void OpenGLRenderer::create(Spaceship * ship, std::vector< std::unique_ptr<TypedBodyView> > & views) {
    auto [vbo, count] = model_map["spaceship"];
    
    SquareMatrix4df korrektur =  {{ 1.0f, 0.0f, 0.0f, 0.0f},
                                  { 0.0f, 0.0f,-1.0f, 0.0f},
                                  { 0.0f, 1.0f, 0.0f, 0.0f},
                                  { 0.0f, 0.0f, 0.0f, 1.0f} };
                                  
    SquareMatrix4df rotZ_minus90 = {{  0.0f, -1.0f, 0.0f, 0.0f},
                                    {  1.0f,  0.0f, 0.0f, 0.0f},
                                    {  0.0f,  0.0f, 1.0f, 0.0f},
                                    {  0.0f,  0.0f, 0.0f, 1.0f} };
                                    
    SquareMatrix4df kombiniert = rotZ_minus90 * korrektur;

    views.push_back(std::make_unique<TypedBodyView>(ship, vbo, shaderProgram, count, 16.0f, GL_TRIANGLES, kombiniert,
                    [ship]() -> bool {return ! ship->is_in_hyperspace();}) 
                    );   
}

void OpenGLRenderer::create(Saucer * saucer, std::vector< std::unique_ptr<TypedBodyView> > & views) {
    auto [vbo, count] = model_map["saucer"];
    float scale = 20.0f; 
    if ( saucer->get_size() == 0 ) {
        scale = 20.0f;
    }
    // Zurück zur Identität (Flache Ausrichtung)
    SquareMatrix4df identitaet = {{1.0f,0.0f,0.0f,0.0f}, {0.0f,1.0f,0.0f,0.0f}, {0.0f,0.0f,1.0f,0.0f}, {0.0f,0.0f,0.0f,1.0f}};
                                  
    views.push_back(std::make_unique<TypedBodyView>(saucer, vbo, shaderProgram, count, scale, GL_TRIANGLES, identitaet));   
}

void OpenGLRenderer::create(Torpedo * torpedo, std::vector< std::unique_ptr<TypedBodyView> > & views) {
    auto [vbo, count] = model_map["torpedo"];
    SquareMatrix4df korrektur = {{ 1.0f, 0.0f, 0.0f, 0.0f},
                                  { 0.0f, 0.0f,-1.0f, 0.0f},
                                  { 0.0f, 1.0f, 0.0f, 0.0f},
                                  { 0.0f, 0.0f, 0.0f, 1.0f} };
    SquareMatrix4df rotZ_minus90 = {{  0.0f, -1.0f, 0.0f, 0.0f},
                                    {  1.0f,  0.0f, 0.0f, 0.0f},
                                    {  0.0f,  0.0f, 1.0f, 0.0f},
                                    {  0.0f,  0.0f, 0.0f, 1.0f} };                        
    SquareMatrix4df kombiniert = rotZ_minus90 * korrektur;
    
    // Skalierung verdoppelt von 12.0f auf 24.0f
    views.push_back(std::make_unique<TypedBodyView>(torpedo, vbo, shaderProgram, count, 24.0f, GL_TRIANGLES, kombiniert)); 
}

void OpenGLRenderer::create(Asteroid * asteroid, std::vector< std::unique_ptr<TypedBodyView> > & views) {
    auto [vbo, count] = model_map["asteroid"];
    float base_scale = 40.0f;
    float scale = (asteroid->get_size() == 3 ? base_scale : ( asteroid->get_size() == 2 ? base_scale*0.5f : base_scale*0.25f ));
    
    // Zurück zur Identität
    SquareMatrix4df identitaet = {{1.0f,0.0f,0.0f,0.0f}, {0.0f,1.0f,0.0f,0.0f}, {0.0f,0.0f,1.0f,0.0f}, {0.0f,0.0f,0.0f,1.0f}};
    
    views.push_back(std::make_unique<TypedBodyView>(asteroid, vbo, shaderProgram, count, scale, GL_TRIANGLES, identitaet)); 
}

void OpenGLRenderer::create(SpaceshipDebris * debris, std::vector< std::unique_ptr<TypedBodyView> > & views) {
    auto [vbo, count] = model_map["debris"];
    
    SquareMatrix4df identitaet = {{1.0f,0.0f,0.0f,0.0f}, {0.0f,1.0f,0.0f,0.0f}, {0.0f,0.0f,1.0f,0.0f}, {0.0f,0.0f,0.0f,1.0f}};
    
    views.push_back(std::make_unique<TypedBodyView>(debris, vbo, shaderProgram, count, 2.0f, GL_TRIANGLES, identitaet,
            []() -> bool {return true;},
            [debris](TypedBodyView * view) -> void { view->set_scale( 2.0f * (SpaceshipDebris::TIME_TO_DELETE - debris->get_time_to_delete()));}));   
}

void OpenGLRenderer::create(Debris * debris, std::vector< std::unique_ptr<TypedBodyView> > & views) {
     auto [vbo, count] = model_map["debris"];
    
    SquareMatrix4df identitaet = {{1.0f,0.0f,0.0f,0.0f}, {0.0f,1.0f,0.0f,0.0f}, {0.0f,0.0f,1.0f,0.0f}, {0.0f,0.0f,0.0f,1.0f}};

    views.push_back(std::make_unique<TypedBodyView>(debris, vbo, shaderProgram, count, 1.0f, GL_TRIANGLES, identitaet,
            []() -> bool {return true;},
            [debris](TypedBodyView * view) -> void { view->set_scale(1.0f * (Debris::TIME_TO_DELETE - debris->get_time_to_delete()));}));   
}

void OpenGLRenderer::createSpaceShipView() {
    auto [vbo, count] = model_map["spaceship"];
    spaceship_view = std::make_unique<OpenGLView>(vbo, shaderProgram, count, GL_TRIANGLES);
}

void OpenGLRenderer::createDigitViews() {
    for (int i = 0; i < 10; i++ ) {
        std::string name = "digit_" + std::to_string(i);
        auto [vbo, count] = model_map[name];
        digit_views[i] = std::make_unique<OpenGLView>(vbo, shaderProgram, count, GL_LINE_STRIP); // Ziffern sind Linien
    }
}


void OpenGLRenderer::renderFreeShips(SquareMatrix4df & matrice) {
    constexpr float FREE_SHIP_X = 128;
    constexpr float FREE_SHIP_Y = 64;
    const float PIf = static_cast<float> ( PI );
    Vector2df position = {FREE_SHIP_X, FREE_SHIP_Y};
    
    // Rotation damit es nach oben zeigt?
    SquareMatrix4df rotation = {   { std::cos(-PIf / 2.0f),  std::sin(-PIf / 2.0f), 0.0f, 0.0f},
                                    {-std::sin(-PIf / 2.0f),  std::cos(-PIf / 2.0f), 0.0f, 0.0f},
                                    { 0.0f,                 0.0f,                1.0f, 0.0f},
                                    { 0.0f,                 0.0f,                0.0f, 1.0f}
                                };
    // Zusätzliche Skalierung für Anzeige
    SquareMatrix4df scaleMat = { { 3.0f, 0.0f, 0.0f, 0.0f},
                                 { 0.0f, 3.0f, 0.0f, 0.0f},
                                 { 0.0f, 0.0f, 3.0f, 0.0f},
                                 { 0.0f, 0.0f, 0.0f, 1.0f} };
                                
    for (int i = 0; i < game.get_no_of_ships(); i++) {
        SquareMatrix4df  translation= { {1.0f,        0.0f,         0.0f, 0.0f},
                                        {0.0f,        1.0f,         0.0f, 0.0f},
                                        {0.0f,        0.0f,         1.0f, 0.0f},
                                        {position[0], position[1],  0.0f, 1.0f} };
        
        SquareMatrix4df render_matrice = matrice * translation * rotation * scaleMat;
        spaceship_view->render( render_matrice );
        position[0] += 40.0;
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
    if (score == 0) no_of_digits = 1;

    Vector2df position = {SCORE_X + 20.0f * no_of_digits,  SCORE_Y};  
    do {
        int d = score % 10;
        score /= 10;
        SquareMatrix4df scale_translation= { {4.0f,        0.0f,         0.0f, 0.0f},
                                                {0.0f,        4.0f,         0.0f, 0.0f},
                                                {0.0f,        0.0f,         1.0f, 0.0f},
                                                {position[0], position[1],  0.0f, 1.0f} };
        SquareMatrix4df render_matrice = matrice * scale_translation;
        digit_views[d]->render( render_matrice );
        no_of_digits--;
        position[0] -= 20;

    } while (no_of_digits > 0 && score >= 0); // score >= 0 check allows 0
}


void OpenGLRenderer::create_shader_programs() {
    // 3D Shader mit Beleuchtung unter Verwendung von Pos(0), Col(1), Normal(2)
    static const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 p;\n"
        "layout (location = 1) in vec3 c;\n"
        "layout (location = 2) in vec3 n;\n"
        "out vec4 vColor;\n"
        "out vec3 vNormal;\n"
        "out vec3 vPos;\n"
        "uniform mat4 transform;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = transform * vec4(p, 1.0);\n"
        "   vPos = vec3(transform * vec4(p, 1.0));\n"
        "   // Transformiere Normale mit der oberen 3x3 der MVP (Näherung für uniforme Rotation)\n"
        "   vNormal = normalize(mat3(transform) * n);\n" 
        "   vColor = vec4(c, 1.0);\n"
        "}\0";
        
    static const char *fragmentShaderSource = "#version 330 core\n"
        "in vec4 vColor;\n"
        "in vec3 vNormal;\n"
        "in vec3 vPos;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   // Beleuchtung: Phong Modell\n"
        "   vec3 lightDir = normalize(vec3(0.2, 0.2, 1.0));\n" // Licht kommt leicht von oben-rechts
        "   vec3 norm = normalize(vNormal);\n"
        "   \n"
        "   // Ambient (Umgebungslicht)\n"
        "   float ambientStrength = 0.3;\n"
        "   vec3 ambient = ambientStrength * vColor.rgb;\n"
        "   \n"
        "   // Diffus\n"
        "   float diff = max(dot(norm, lightDir), 0.0);\n"
        "   vec3 diffuse = diff * vColor.rgb;\n"
        "   \n"
        "   // Spekular (Glanzlicht)\n"
        "   float specularStrength = 0.5;\n"
        "   vec3 viewDir = vec3(0.0, 0.0, 1.0);\n" // Orthographische Näherung
        "   vec3 reflectDir = reflect(-lightDir, norm);\n"
        "   float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);\n"
        "   vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);\n"
        "   \n"
        "   vec3 result = ambient + diffuse + specular;\n"
        "   FragColor = vec4(result, vColor.a);\n"
        "}\n\0";

    // Vertex-Shader erstellen und kompilieren
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Auf Shader-Kompilierungsfehler prüfen
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        error( std::string("Fehler bei Vertex-Shader Kompilierung: ") + infoLog);
    }
    // Fragment-Shader erstellen und kompilieren
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Auf Shader-Kompilierungsfehler prüfen
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        error( std::string("Fehler bei Fragment-Shader Kompilierung: ") + infoLog);
    }
    

    // Shader-Programm linken
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Auf Linker-Fehler prüfen
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        error( std::string("Fehler beim Linken des Shader-Programms: ") + infoLog);
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
      #ifdef __APPLE__
        // macOS needs specific context settings for Core Profile 3.3+
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG );
      #else
        // For Linux/Windows, we also want Core 3.3
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
      #endif
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        context = SDL_GL_CreateContext(window);
        
        GLenum err = glewInit(); 
        if (GLEW_OK != err) {
        error( "Could not initialize Glew. Glew error message: " );
        error( glewGetErrorString(err) );
        }
        debug(1, "Using GLEW Version: ");
        debug(1, glewGetString(GLEW_VERSION) );

        SDL_GL_SetSwapInterval(1);
        
        glEnable(GL_DEPTH_TEST); 

        create_shader_programs();
        createVbos();
        createSpaceShipView();
        createDigitViews();
        return true;
    }
    }
    return false;
}

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
    // transformation to canonical view (Top-Down Ortho-ish)
    SquareMatrix4df canonical_transform =
    SquareMatrix4df{
        { 2.0f / 1024.0f,           0.0f,            0.0f,  0.0f},
        {       0.0f,     -2.0f / 768.0f,            0.0f,  0.0f},
        {       0.0f,               0.0f,  2.0f / 1024.0f,  0.0f}, 
        {      -1.0f,               1.0f,           0.0f,  1.0f}  
    };
                                                 
    glClearColor ( 0.0, 0.0, 0.0, 1.0 );
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
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

    SquareMatrix4df scroll_transform = SquareMatrix4df{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    };
    if (game.ship_exists() && game.get_ship() != nullptr) {
        Vector2df ship_pos = game.get_ship()->get_position();
        scroll_transform = SquareMatrix4df{
            {1.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f},
            {512.0f - ship_pos[0], 384.0f - ship_pos[1], 0.0f, 1.0f}
        };
    }

    for (int t = 0; t < 9; ++t) {
        SquareMatrix4df tile_transform = SquareMatrix4df{
            {1.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f},
            {tile_positions[t][0], tile_positions[t][1], 0.0f, 1.0f}
        };
        SquareMatrix4df world = canonical_transform * scroll_transform * tile_transform;
        for (auto & view : views) {
            view->render(world);
        }
    }
    renderFreeShips(canonical_transform);
    renderScore(canonical_transform);

    SDL_GL_SwapWindow(window);
}

void OpenGLRenderer::exit() {
    views.clear();
    for(auto const& [name, val] : model_map) {
        glDeleteBuffers(1, &val.first);
    }
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow( window );
    SDL_Quit();
}


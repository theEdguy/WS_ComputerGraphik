#include "math.h"
#include "geometry.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <SDL2/SDL.h>



// Die folgenden Kommentare beschreiben Datenstrukturen und Funktionen
// Die Datenstrukturen und Funktionen die weiter hinten im Text beschrieben sind,
// hängen höchstens von den vorhergehenden Datenstrukturen ab, aber nicht umgekehrt.



// Ein "Bildschirm", der das Setzen eines Pixels kapselt
// Der Bildschirm hat eine Auflösung (Breite x Höhe)
// Kann zur Ausgabe einer PPM-Datei verwendet werden oder
class Screen 
{

size_t full_width, full_height;
std::vector<Vector3df> pixels;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

public:

  Screen (size_t width, size_t height) : full_width(width), full_height(height) 
  {
      pixels.resize(full_width*full_height, Vector3df{0.0, 0.0, 0.0});
      
      SDL_Init(SDL_INIT_VIDEO);
      SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);
  }

  // x und y sind Bildkoordinaten 
  void set_pixel(size_t x, size_t y, Vector3df color)
  {
   pixels[y * full_width + x] = color;
  };

  ~Screen()
  {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
  }

  void show()
  {
    SDL_SetRenderDrawColor(renderer, 0,0,0,255); //schwarz
    SDL_RenderClear(renderer);

    for (size_t y = 0; y < full_height; ++y) {
      for (size_t x = 0; x < full_width; ++x) {
        Vector3df c = pixels[y * full_width + x];

              int r = (int)(c[0] * 255);
              int g = (int)(c[1] * 255);
              int b = (int)(c[2] * 255);

              //falls Licht zu hell
              if (r > 255) r = 255;
              if (g > 255) g = 255;
              if (b > 255) b = 255;
              
              SDL_SetRenderDrawColor(renderer, r, g, b, 255);
              SDL_RenderDrawPoint(renderer, x, y);
      }
    }
    SDL_RenderPresent(renderer);

    SDL_Event e;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
        }
    }
  }
};


// Eine "Kamera", die von einem Augenpunkt aus in eine Richtung senkrecht auf ein Rechteck (das Bild) zeigt.
// Für das Rechteck muss die Auflösung oder alternativ die Pixelbreite und -höhe bekannt sein.
// Für ein Pixel mit Bildkoordinate kann ein Sehstrahl erzeugt werden.

class Camera
{

Vector3df eye;

// aufgespannter 3dimensionaler raum
//orthogonales (rechtwinkliges) Koordinatensystem
Vector3df direction;
Vector3df right;
Vector3df up;
//

size_t image_width, image_height;
float pixel_size;

public: 
Camera(Vector3df eye_pos, Vector3df look_at, Vector3df up_direction, size_t width, size_t height, float pixel_width) 
  : eye(eye_pos), image_width(width), image_height(height), pixel_size(pixel_width),
    direction(Vector3df{0.0, 0.0, 0.0}), right(Vector3df{0.0, 0.0, 0.0}), up(Vector3df{0.0, 0.0, 0.0})
{
  
  // wo wir hinschauen
  direction = look_at - eye;
  direction.normalize();
  
  // Rechts-Vektor berechnen
  right = direction.cross_product(up_direction);
  right.normalize();
  
  // Oben-Vektor
  up = right.cross_product(direction);
  up.normalize();
}

Ray3df get_ray(size_t x, size_t y)
{
  float offset_x = ((float)x - (float)image_width / 2.0f) * pixel_size;
  float offset_y = ((float)image_height / 2.0f - (float)y) * pixel_size;

  Vector3df ray_direction = direction + (offset_x * right) + (offset_y * up);
  ray_direction.normalize();

  return Ray3df {eye, ray_direction};
}
};


// Für die "Farbe" benötigt man nicht unbedingt eine eigene Datenstruktur.
// Sie kann als Vector3df implementiert werden mit Farbanteil von 0 bis 1.
// Vor Setzen eines Pixels auf eine bestimmte Farbe (z.B. 8-Bit-Farbtiefe),
// kann der Farbanteil mit 255 multipliziert  und der Nachkommaanteil verworfen werden.


// Das "Material" der Objektoberfläche mit ambienten, diffusem und reflektiven Farbanteil.

struct Light
{
  Vector3df position;
  Vector3df color;
};

class Material
{
public:
  Vector3df ambient;      // Umgebungsfarbe
  Vector3df diffuse;      // Matte Farbe (Hauptfarbe)
  Vector3df mirrorcolour; // Spiegelfarbe
  
  // Konstruktor
  Material(Vector3df amb, Vector3df diff, Vector3df spec)
    : ambient(amb), diffuse(diff), mirrorcolour(spec)
  {
  }
};

// verschiedene Materialdefinition, z.B. Mattes Schwarz, Mattes Rot, Reflektierendes Weiss, ...
// im wesentlichen Variablen, die mit Konstruktoraufrufen initialisiert werden.

Material mat_black(
  Vector3df{0.0, 0.0, 0.0},  
  Vector3df{0.0, 0.0, 0.0},  
  Vector3df{0.0, 0.0, 0.0}   
);

Material mat_red(
  Vector3df{0.1, 0.0, 0.0},  
  Vector3df{0.8, 0.0, 0.0},  
  Vector3df{0.0, 0.0, 0.0}   
);

Material mat_green(
  Vector3df{0.0, 0.1, 0.0},  
  Vector3df{0.0, 0.8, 0.0},  
  Vector3df{0.0, 0.0, 0.0}   
);

Material mat_blue(
  Vector3df{0.0, 0.0, 0.1},  
  Vector3df{0.0, 0.0, 0.8},  
  Vector3df{0.0, 0.0, 0.0}   
);

Material mat_white(
  Vector3df{0.1, 0.1, 0.1},  
  Vector3df{0.8, 0.8, 0.8},  
  Vector3df{0.0, 0.0, 0.0}   
);

Material mat_mirror(
  Vector3df{0.1, 0.1, 0.1},  
  Vector3df{0.1, 0.1, 0.1},  
  Vector3df{0.9, 0.9, 0.9}   
);

Material mat_shiny_red(
  Vector3df{0.1, 0.0, 0.0},  
  Vector3df{0.6, 0.0, 0.0},  
  Vector3df{0.4, 0.4, 0.4}   
);

Material mat_yellow(
  Vector3df{0.1, 0.1, 0.0},  
  Vector3df{0.8, 0.8, 0.0},  
  Vector3df{0.2, 0.2, 0.2}   
);


// Ein "Objekt", z.B. eine Kugel oder ein Dreieck, und dem zugehörigen Material der Oberfläche.
// Im Prinzip ein Wrapper-Objekt, das mindestens Material und geometrisches Objekt zusammenfasst.
// Kugel und Dreieck finden Sie in geometry.h/tcc

class Object
{
  Sphere3df sphere;
  Material material;
  Vector3df center; 

public:
  Object(Vector3df c, float r, Material m) 
    : sphere(Sphere3df{c, r}), material(m), center(c) {}

  float intersect(Ray3df ray){
    float distance = sphere.intersects(ray);
    if(distance > 0){ return distance;}
    return -1;
  }

  Material& get_material(){ return material;}

   Sphere3df& get_sphere(){ return sphere;}

   Vector3df& get_center(){ return center;}
};


// Die folgenden Werte zur konkreten Objekten, Lichtquellen und Funktionen, wie Lambertian-Shading
// oder die Suche nach einem Sehstrahl für das dem Augenpunkt am nächsten liegenden Objekte,
// können auch zusammen in eine Datenstruktur für die gesammte zu
// rendernde "Szene" zusammengefasst werden.

class Scene
{
  std::vector<Object> objects;
  std::vector<Light> lights;

  public:

  void add_object(Object obj){objects.push_back(obj);}

  void add_light(Vector3df pos, Vector3df col){
      lights.push_back(Light{pos, col});
  }

  Object* find_nearest(Ray3df ray, float& distanz_out);
  Vector3df shade(Ray3df ray, Object* obj, float distance);
  Vector3df trace(Ray3df ray, int depth);
};


// Die Cornelbox aufgebaut aus den Objekten
// Am besten verwendet man hier einen std::vector< ... > von Objekten.
// Punktförmige "Lichtquellen" können einfach als Vector3df implementiert werden mit weisser Farbe...

Scene create_cornell_box()
{
  Scene scene;
  float wall_r = 1e5f;

  // Wände
  scene.add_object(Object(Vector3df{-wall_r - 2.0f, 0.0f, -5.0f}, wall_r, mat_red));   // Links
  scene.add_object(Object(Vector3df{ wall_r + 2.0f, 0.0f, -5.0f}, wall_r, mat_green)); // Rechts
  scene.add_object(Object(Vector3df{0.0f, 0.0f, -wall_r - 10.0f}, wall_r, mat_yellow));  // Hinten
  scene.add_object(Object(Vector3df{0.0f, -wall_r - 2.0f, -5.0f}, wall_r, mat_blue)); // Boden
  scene.add_object(Object(Vector3df{0.0f,  wall_r + 2.0f, -5.0f}, wall_r, mat_white)); // Decke
  
  // Kugeln
  scene.add_object(Object(Vector3df{-1.0f, -1.0f, -6.0f}, 1.0f, mat_mirror));
  scene.add_object(Object(Vector3df{ 0.8f, -1.2f, -4.5f}, 0.8f, mat_yellow));
  scene.add_object(Object(Vector3df{ 0.0f, 0.5f, -5.0f}, 0.5f, mat_shiny_red));
  
  // Lichter  
  scene.add_light(Vector3df{0.0f, 1.8f, -5.0f}, Vector3df{0.8f, 0.8f, 0.8f});
  
  scene.add_light(Vector3df{1.5f, -1.0f, -2.0f}, Vector3df{0.0f, 0.0f, 0.4f});

  return scene;
}


// Sie benötigen eine Implementierung von Lambertian-Shading, z.B. als Funktion
// Benötigte Werte können als Parameter übergeben werden, oder wenn diese Funktion eine Objektmethode eines
// Szene-Objekts ist, dann kann auf die Werte teilweise direkt zugegriffen werden.
// Bei mehreren Lichtquellen muss der resultierende diffuse Farbanteil durch die Anzahl Lichtquellen geteilt werden.

Vector3df Scene::shade(Ray3df ray, Object* obj, float distance) {

    // Schnittpunkt berechnen
    Vector3df hit_point = ray.origin + (distance * ray.direction);

    // Normale berechnen
    Vector3df normal = hit_point - obj->get_center();
    normal.normalize();

    Vector3df diffuse_sum = {0,0,0};

    for ( auto& light : lights) {
        
        Vector3df light_vec = light.position - hit_point;
        float dist_to_light = sqrt(light_vec * light_vec);

        Vector3df light_dir = light_vec;
        light_dir.normalize(); 

        // Schatten 
        Vector3df shadow_origin = hit_point + (0.1f * normal);
        Ray3df shadow_ray = { shadow_origin, light_dir };

        float shadow_dist;
        Object* shadow_obj = find_nearest(shadow_ray, shadow_dist);

        bool in_shadow = false;
        if (shadow_obj != nullptr) {
            if (shadow_dist < dist_to_light) {
                in_shadow = true;
            }
        }

        // Nur beleuchten, wenn NICHT im Schatten
        if (!in_shadow) {
            float intensity = normal * light_dir;
            
            if (intensity > 0) {
                Vector3df combined_color = {
                    obj->get_material().diffuse[0] * light.color[0],
                    obj->get_material().diffuse[1] * light.color[1],
                    obj->get_material().diffuse[2] * light.color[2]
                };
                
                diffuse_sum = diffuse_sum + (intensity * combined_color);
            }
        }
    }

    return obj->get_material().ambient + diffuse_sum;
}


// Für einen Sehstrahl aus allen Objekte, dasjenige finden, das dem Augenpunkt am nächsten liegt.
// Am besten einen Zeiger auf das Objekt zurückgeben. Wenn dieser nullptr ist, dann gibt es kein sichtbares Objekt.

Object* Scene::find_nearest(Ray3df ray, float& distanz_out)
{
  Object* nearest_Object = nullptr;
  float current_min_dist = 9e9; 
  
  for (auto& obj : objects)
  {
    float distance = obj.intersect(ray);
    if (distance > 0.0f && distance < current_min_dist) {
      current_min_dist = distance;
      nearest_Object = &obj;
    }
  }
  distanz_out = current_min_dist;
  return nearest_Object;
}


// Die rekursive raytracing-Methode. Am besten ab einer bestimmten Rekursionstiefe (z.B. als Parameter übergeben) abbrechen.
// (Aktuell noch nicht rekursiv implementiert, das kommt später für Reflexionen)

Vector3df Scene::trace(Ray3df ray, int depth) {
    if (depth <= 0) {
        return Vector3df{0, 0, 0};
    }

    float distance;
    Object* hit_obj = find_nearest(ray, distance);

    if (hit_obj != nullptr) {
        // 1. Lokale Farbe berechnen
        Vector3df local_color = shade(ray, hit_obj, distance);

        // 2. Reflexion berechnen
        const Material& mat = hit_obj->get_material();
        bool is_reflective = (mat.mirrorcolour[0] > 0 || mat.mirrorcolour[1] > 0 || mat.mirrorcolour[2] > 0);

        if (is_reflective) {
            Vector3df hit_point = ray.origin + (distance * ray.direction);
            Vector3df normal = hit_point - hit_obj->get_center();
            normal.normalize();

            // Reflexionsvektor berechnen
            // Wir nutzen die Formel R = D - 2(N*D)N manuell, falls math.h keine Methode hat
            float dot = ray.direction * normal;
            Vector3df reflect_dir = ray.direction - (2.0f * dot * normal);
            reflect_dir.normalize();

            Vector3df reflect_origin = hit_point + (0.1f * normal);
            Ray3df reflect_ray = { reflect_origin, reflect_dir };

            // Rekursion
            Vector3df reflected_color = trace(reflect_ray, depth - 1);

            // Mischen
            Vector3df reflection_add = {
                reflected_color[0] * mat.mirrorcolour[0],
                reflected_color[1] * mat.mirrorcolour[1],
                reflected_color[2] * mat.mirrorcolour[2]
            };

            return local_color + reflection_add;
        }

        return local_color;
    }

    return Vector3df{0.0, 0.0, 0.0};
}


  // Bildschirm erstellen
  // Kamera erstellen
  // Für jede Pixelkoordinate x,y
  //   Sehstrahl für x,y mit Kamera erzeugen
  //   Farbe mit raytracing-Methode bestimmen
  //   Beim Bildschirm die Farbe für Pixel x,y, setzten

int main(void) {
  size_t width = 400;
  size_t height = 400;
  Screen screen(width, height);

  Camera cam(Vector3df{0.0f,0.0f,0.0f}, Vector3df{0.0f,0.0f,-1.0f}, Vector3df{0.0f,1.0f,0.0f}, width, height, 0.005f);

  Scene scene = create_cornell_box();

  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      
      Ray3df ray = cam.get_ray(x, y);
      
      Vector3df pixel_color = scene.trace(ray, 5);

      screen.set_pixel(x, y, pixel_color);
    }
  }
  screen.show();
  return 0;
}
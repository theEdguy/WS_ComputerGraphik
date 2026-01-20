# Asteroids 3D OpenGL Implementation Documentation

## Overview
This project upgrades the classic 2D Asteroids game to a 3D rendering engine using OpenGL 3.3 Core Profile. The game logic remains in 2D (physics on a plane), but the visualization uses 3D meshes loaded from Wavefront (.obj) files.

## 3D Rendering Pipeline

### 1. Model Loading (Wavefront .obj)
We utilize a custom `WavefrontImporter` to parse `.obj` files.
- **Files Used**: `space_ship.obj`, `asteroid.obj`, `torpedo.obj`, `ufo.obj`.
- **Method**: `erstelle_vbo_von_obj(filename)` (Helper function).
- **Process**:
  1. Parse the OBJ file to extract Vertices, Normals, and Faces.
  2. Flatten the indexed face data into a direct vertex array (GL_TRIANGLES).
  3. Create an **Interleaved Buffer**:
     - `Position (x, y, z)`: 3 Floats
     - `Normal (nx, ny, nz)`: 3 Floats
     - `Color (r, g, b)`: 3 Floats (Derived from Material ambient or default White)
  4. Upload to a Vertex Buffer Object (VBO).

### 2. OpenGL Vertex Array Object (VAO) Setup
The `OpenGLView` class configures the VAO to interpret the interleaved data:
- **Stride**: `9 * sizeof(float)` (Total size of one vertex attributes).
- **Attribute 0 (Position)**: Offset 0.
- **Attribute 1 (Color)**: Offset `6 * sizeof(float)` (After Pos and Normal).
- **Attribute 2 (Normal)**: Offset `3 * sizeof(float)` (After Pos).

*Note: The attribute locations (0, 1, 2) must match the Layout qualifiers in the Vertex Shader.*

### 3. Shaders & Lighting
We replaced the simple "White Color" shader with a rudimentary Lighting Shader.

**Vertex Shader:**
- Inputs: `vec3 p` (Pos), `vec3 c` (Color), `vec3 n` (Normal).
- Outputs: `vColor`, `vNormal`, `vPos`.
- Operation: Multiplies Position by the Model-View-Projection matrix (`transform`). Passes normal and color to the fragment shader.

**Fragment Shader:**
- Logic: **Lambertian Reflection**.
- Light Source: Directional light from viewer direction `(0, 0, 1)`.
- Calculation: `brightness = max(dot(Normal, LightDir), 0.2)` (0.2 is ambient base).
- Result: `FragColor = ObjectColor * brightness`.

### 4. Scene Management
- **Model Map**: A `std::map<string, pair<GLuint, size_t>>` caches VBOs. Each model (e.g., "asteroid") is loaded once and reused for all instances.
- **Coordinate System**:
  - The game uses a 2D coordinate system (1024x768).
  - We map this to OpenGL NDC (-1 to 1) using a `canonical_transform`.
  - The Z-axis is scaled similarly to X/Y to preserve the aspect ratio of 3D objects, ensuring spheres look like spheres.
  - `glEnable(GL_DEPTH_TEST)` is enabled to ensure correct occlusion (ships hide stars/debris behind them).

### 5. Legacy Support (Digits)
- The score digits use 2D line segments. 
- A helper `erstelle_vbo_von_2d` inflates these 2D points into the 3D format (setting Z=0, Normal=(0,0,1)), allowing the same shader to render both 3D meshes and 2D UI elements.

#include "matrix.h"
#include "matrix.tcc"
#include <cmath>

template class SquareMatrix<float, 2u>;
template class SquareMatrix<float, 3u>; 
template class SquareMatrix<float, 4u>;

template SquareMatrix<float, 2> operator*(const SquareMatrix<float, 2> factor1, const SquareMatrix<float,2> factor2);
template SquareMatrix<float, 3> operator*(const SquareMatrix<float, 3> factor1, const SquareMatrix<float,3> factor2);
template SquareMatrix<float, 4> operator*(const SquareMatrix<float, 4> factor1, const SquareMatrix<float,4> factor2);

SquareMatrix4df make_identity_matrix() {
  return {
    Vector4df{1.0f, 0.0f, 0.0f, 0.0f},
    Vector4df{0.0f, 1.0f, 0.0f, 0.0f},
    Vector4df{0.0f, 0.0f, 1.0f, 0.0f},
    Vector4df{0.0f, 0.0f, 0.0f, 1.0f}
  };
}

SquareMatrix4df make_translation_matrix(Vector3df translation) {
  // Eine Translationsmatrix in homogenen Koordinaten (Spaltenweise):
  // 1 0 0 Tx
  // 0 1 0 Ty
  // 0 0 1 Tz
  // 0 0 0 1
  return {
    Vector4df{1.0f, 0.0f, 0.0f, 0.0f},
    Vector4df{0.0f, 1.0f, 0.0f, 0.0f},
    Vector4df{0.0f, 0.0f, 1.0f, 0.0f},
    Vector4df{translation[0], translation[1], translation[2], 1.0f}
  };
}

SquareMatrix4df make_scale_matrix(Vector3df scale) {
  return {
    Vector4df{scale[0], 0.0f, 0.0f, 0.0f},
    Vector4df{0.0f, scale[1], 0.0f, 0.0f},
    Vector4df{0.0f, 0.0f, scale[2], 0.0f},
    Vector4df{0.0f, 0.0f, 0.0f, 1.0f}
  };
}

SquareMatrix4df make_rotation_z_matrix(float angle) {
  float c = std::cos(angle);
  float s = std::sin(angle);
  // spaltenweise:
  // col0: (c, s, 0, 0)
  // col1: (-s, c, 0, 0)
  // col2: (0, 0, 1, 0)
  // col3: (0, 0, 0, 1)
  return {
    Vector4df{c, s, 0.0f, 0.0f},
    Vector4df{-s, c, 0.0f, 0.0f},
    Vector4df{0.0f, 0.0f, 1.0f, 0.0f},
    Vector4df{0.0f, 0.0f, 0.0f, 1.0f}
  };
}

SquareMatrix4df make_perspective_projection_matrix(float fov_y, float aspect, float near_plane, float far_plane) {
  float const tanHalfFovy = std::tan(fov_y / 2.0f);
  
  // Wir bauen eine Matrix analog zu glm::perspectiveRH_NO
  // (Right Handed, Negative One to One depth range - OpenGL standard)
  /*
     1/(aspect*tan)  0             0                                0
     0               1/tan         0                                0
     0               0             -(far+near)/(far-near)          -1
     0               0             -(2*far*near)/(far-near)         0
  */
  
  float f = 1.0f / tanHalfFovy;
  
  Vector4df col0{ f / aspect, 0.0f, 0.0f, 0.0f };
  Vector4df col1{ 0.0f, f, 0.0f, 0.0f };
  Vector4df col2{ 0.0f, 0.0f, -(far_plane + near_plane) / (far_plane - near_plane), -1.0f };
  Vector4df col3{ 0.0f, 0.0f, -(2.0f * far_plane * near_plane) / (far_plane - near_plane), 0.0f };

  return { col0, col1, col2, col3 };
}

SquareMatrix4df make_look_at_matrix(Vector3df eye, Vector3df center, Vector3df up) {
  // f = normalize(center - eye)
  Vector3df f = center - eye;
  f.normalize();
  
  // s = normalize(cross(f, up))
  // manual cross product to ensure RH system if physics vector has weird cross
  // s = f x up
  Vector3df s{ 
      f[1] * up[2] - f[2] * up[1],
      f[2] * up[0] - f[0] * up[2],
      f[0] * up[1] - f[1] * up[0]
  };
  s.normalize();
  
  // u = cross(s, f)
  Vector3df u{
      s[1] * f[2] - s[2] * f[1],
      s[2] * f[0] - s[0] * f[2],
      s[0] * f[1] - s[1] * f[0]
  };
  
  // Result Matrix (Column Major):
  // Col 0: s.x, u.x, -f.x, 0
  // Col 1: s.y, u.y, -f.y, 0
  // Col 2: s.z, u.z, -f.z, 0
  // Col 3: -dot(s, eye), -dot(u, eye), dot(f, eye), 1
  
  // dot(s, eye) = s[0]*eye[0] + ...
  float dot_s_eye = s * eye;
  float dot_u_eye = u * eye;
  float dot_f_eye = f * eye;
  
  Vector4df col0{ s[0], u[0], -f[0], 0.0f };
  Vector4df col1{ s[1], u[1], -f[1], 0.0f };
  Vector4df col2{ s[2], u[2], -f[2], 0.0f };
  Vector4df col3{ -dot_s_eye, -dot_u_eye, dot_f_eye, 1.0f }; // dot_f_eye should be positive because f is negated in z-row?
  
  // GLM:
  // Result[0][0] = s.x;
  // Result[1][0] = s.y;
  // Result[2][0] = s.z;
  // Result[0][1] = u.x; ...
  // Result[0][2] =-f.x; ...
  // Result[3][0] =-dot(s, eye);
  // Result[3][1] =-dot(u, eye);
  // Result[3][2] = dot(f, eye);
  // Result[3][3] = 1;

  return { col0, col1, col2, col3 };
}


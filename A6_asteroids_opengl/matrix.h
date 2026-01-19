#ifndef MATRIX_H
#define MATRIX_H

#include "math.h"

template <class FLOAT, size_t N>
class SquareMatrix {
  static_assert(N > 0u);
  std::array< Vector<FLOAT,N>, N> matrix;  // Werte werden spaltenweise (als Vektoren) gespeichert
public:
  SquareMatrix() = default;

  SquareMatrix(std::initializer_list< Vector<FLOAT, N > > values);
    
  // Gibt eine Referenz auf den i-ten Spaltenvektor zurück
  Vector<FLOAT, N> & operator[](std::size_t i);

  // Gibt den i-ten Spaltenvektor zurück
  Vector<FLOAT, N> operator[](std::size_t i) const;
  
  // Gibt den Wert an der angegebenen Zeile und Spalte zurück
  FLOAT at(size_t row, size_t column) const;

  // Gibt eine Referenz auf den Wert an der angegebenen Zeile und Spalte zurück
  FLOAT & at(size_t row, size_t column);
  
  // Gibt das Produkt dieser SquareMatrix mit dem angegebenen Vektor zurück
  Vector<FLOAT,N> operator*(const Vector<FLOAT,N> vector) const;

  // Gibt das Produkt von zwei quadratischen Matrizen zurück
  template <class F, size_t K>
  friend SquareMatrix<F, K> operator*(const SquareMatrix<F, K> factor1, const SquareMatrix<F, K> factor2);

};


typedef SquareMatrix<float, 2u> SquareMatrix2df;
typedef SquareMatrix<float, 3u> SquareMatrix3df;
typedef SquareMatrix<float, 4u> SquareMatrix4df;

#endif

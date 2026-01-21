
template <class FLOAT, size_t N>
SquareMatrix<FLOAT, N>::SquareMatrix(std::initializer_list< Vector<FLOAT, N > > values) {
  auto iterator = values.begin();
  for (size_t i = 0u; i < N; i++) {
    if (iterator != values.end()) {
      matrix[i] = *iterator++;
    } else {
        matrix[i] = Vector<FLOAT, N>({static_cast<FLOAT>(0.0)});
    }
  }
}

template <class FLOAT, size_t N>
Vector<FLOAT, N> & SquareMatrix<FLOAT, N>::operator[](std::size_t i) {
  return matrix[i];
}

template <class FLOAT, size_t N>
Vector<FLOAT, N> SquareMatrix<FLOAT, N>::operator[](std::size_t i) const {
  return matrix[i];
}

template <class FLOAT, size_t N>
FLOAT SquareMatrix<FLOAT, N>::at(size_t row, size_t column) const {
  return matrix[column][row];
}

template <class FLOAT, size_t N>
FLOAT & SquareMatrix<FLOAT, N>::at(size_t row, size_t column) {
  return matrix[column][row];
}

//linearkombination
template <class FLOAT, size_t N>
Vector<FLOAT,N> SquareMatrix<FLOAT, N>::operator*(const Vector<FLOAT,N> vector) const {
  Vector<FLOAT, N> result({static_cast<FLOAT>(0.0)}); 
  for (size_t col = 0; col < N; ++col) {
      result += vector[col] * matrix[col];
  }
  return result;
}

template <class F, size_t K>
SquareMatrix<F, K> operator*(const SquareMatrix<F, K> factor1, const SquareMatrix<F, K> factor2) {
  SquareMatrix<F, K> result;
  for (size_t col = 0; col < K; ++col) {
      //Matrix 1 wird mit jeder Spalte aus Matrix2
      result[col] = factor1 * factor2[col];
  }
  return result;
}

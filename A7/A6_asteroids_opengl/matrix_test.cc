#include "matrix.h"
#include "gtest/gtest.h"

namespace {
	
TEST(MATRIX, ListInitialization2df) {
  SquareMatrix2df matrix = { Vector2df{1.0, 0.0},
                             Vector2df{0.0, 1.0} };
  
  EXPECT_NEAR(1.0, matrix.at(0,0), 0.00001);
  EXPECT_NEAR(0.0, matrix.at(0,1), 0.00001);
  EXPECT_NEAR(0.0, matrix.at(1,0), 0.00001);
  EXPECT_NEAR(1.0, matrix.at(1,1), 0.00001);
}

TEST(MATRIX, ListInitialization3df) {
  SquareMatrix3df matrix = { Vector3df{1.0, 0.0, 0.0},
                             Vector3df{0.0, 1.0, 0.0},
                             Vector3df{0.0, 0.0, 1.0} };
                               
  EXPECT_NEAR(1.0, matrix[0][0], 0.00001);
  EXPECT_NEAR(0.0, matrix[0][1], 0.00001);
  EXPECT_NEAR(0.0, matrix[0][2], 0.00001);
  EXPECT_NEAR(0.0, matrix[1][0], 0.00001);
  EXPECT_NEAR(1.0, matrix[1][1], 0.00001);
  EXPECT_NEAR(0.0, matrix[1][2], 0.00001);
  EXPECT_NEAR(0.0, matrix[2][0], 0.00001);
  EXPECT_NEAR(0.0, matrix[2][1], 0.00001);
  EXPECT_NEAR(1.0, matrix[2][2], 0.00001);
}

TEST(MATRIX, ListInitialization4df) {
  SquareMatrix4df matrix = { {1.0, 5.0, 9.0, 13.0},
                             {2.0, 6.0, 10.0, 14.0},
                             {3.0, 7.0, 11.0, 15.0},
                             {4.0, 8.0, 12.0, 16.0} };
  
  float v = 1.0f;
  for (size_t row = 0; row < 4; row++) {
    for (size_t column = 0;  column < 4; column++) {    
      EXPECT_NEAR(v++, matrix.at(row, column), 0.00001);
    }
  }
}



TEST(MATRIX, ProductWithVector3df) {
  SquareMatrix3df matrix = { {1.0, 0.0, 1.0},
                             {0.0, 1.0, 1.0},
                             {0.0, 0.0, 1.0} };
  Vector3df vector = {-6.0, 3.0,  1.0};
  Vector3df product = matrix * vector;  
  
  EXPECT_NEAR(-6.0, product[0], 0.00001);
  EXPECT_NEAR( 3.0, product[1], 0.00001);
  EXPECT_NEAR(-2.0, product[2], 0.00001);
}



TEST(MATRIX, ProductWithMatrix2df) {
  SquareMatrix3df matrix1 = { {1.0, 2.0},
                              {-1.0, 1.5} };
  SquareMatrix3df matrix2 = { {2.0, -1.0},
                              {1.0, 0.0} };
  SquareMatrix3df matrix = matrix1 * matrix2; 
  
  EXPECT_NEAR( 3.0, matrix.at(0,0), 0.00001);
  EXPECT_NEAR( 1.0, matrix.at(0,1), 0.00001);
  EXPECT_NEAR( 2.5, matrix.at(1,0), 0.00001);
  EXPECT_NEAR( 2.0, matrix.at(1,1), 0.00001);
}



}

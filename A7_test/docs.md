# Ausführliche Dokumentation der Asteroids-Implementierung

Diese Dokumentation beschreibt detailliert die Implementierung der mathematischen Grundlagen (Vektoren, Matrizen) und der Spiellogik (speziell das Scrolling) für die OpenGL-Version von Asteroids.

## 1. Mathematische Grundlagen (`math.h`, `math.tcc`)

Das Fundament der Grafikprogrammierung sind Vektoren. In diesem Projekt wird eine template-basierte `Vector`-Klasse verwendet.

### Der Vektor (`Vector` Struct)
Die Klasse ist ein Template `template<class FLOAT_TYPE, size_t N>`, was bedeutet, sie kann für beliebige Datentypen (meist `float`) und Dimensionen (2D, 3D, 4D) verwendet werden.

#### Default-Konstruktor
Eine der Aufgaben war die Implementierung eines leeren Default-Konstruktors. Dieser ist notwendig, damit Arrays von Vektoren (wie in der Matrix-Klasse) erstellt werden können, ohne dass sofort Werte zugewiesen werden müssen.

```cpp
// math.h
Vector();

// math.tcc
template <class FLOAT_TYPE, size_t N>
Vector<FLOAT_TYPE, N>::Vector() {
  // Tut nichts, Speicher bleibt uninitialisiert (oder wird je nach Compiler/Kontext 0-initialisiert)
  // Dies ist effizient für Arrays, die später gefüllt werden.
}
```

#### Wichtige Vektor-Operationen
Die Vektor-Klasse unterstützt Standard-Rechenoperationen, die für die Matrix-Multiplikation essenziell sind:
*   **Addition (`+`)**: Komponentenweise Addition.
*   **Skalarmultiplikation (`*`)**: Multiplikation jedes Elements mit einem Faktor.
*   **Indizierung (`[]`)**: Zugriff auf die Komponenten (0=x, 1=y, ...).

---

## 2. Matrix-Implementierung (`matrix.tcc`)

Die Matrix ist als quadratische Matrix (`SquareMatrix`) implementiert. OpenGL verwendet standardmäßig **Column-Major Order** (Spalten-Hauptordnung). Das bedeutet, die Matrix ist im Speicher nicht Zeile für Zeile, sondern Spalte für Spalte abgelegt.

### Speicherstruktur
```cpp
// matrix.h
std::array< Vector<FLOAT,N>, N> matrix;
```
Das interne Array `matrix` speichert `N` Vektoren. Jeder dieser Vektoren repräsentiert eine **Spalte** der Matrix.
`matrix[0]` ist die erste Spalte, `matrix[1]` die zweite, usw.

### Konstruktor
Der Konstruktor initialisiert die Matrix mit einer Liste von Spaltenvektoren. Werden zu wenige Vektoren übergeben, werden die restlichen mit 0 aufgefüllt.

```cpp
template <class FLOAT, size_t N>
SquareMatrix<FLOAT, N>::SquareMatrix(std::initializer_list< Vector<FLOAT, N > > values) {
  auto iterator = values.begin();
  for (size_t i = 0u; i < N; i++) {
    if (iterator != values.end()) {
      matrix[i] = *iterator++; // Kopiere den übergebenen Vektor in die i-te Spalte
    } else {
      // Wenn keine Werte mehr da sind, fülle mit Nullvektoren auf
      matrix[i] = Vector<FLOAT, N>({static_cast<FLOAT>(0.0)});
    }
  }
}
```
Dieser Code geht durch die Initialisierungsliste und füllt das interne `matrix`-Array Spalte für Spalte.

### Zugriff auf Elemente (`at`)
Da die Daten spaltenweise gespeichert sind, muss beim Zugriff über Zeile und Spalte die Reihenfolge beachtet werden.
Um auf das Element in Zeile `row` und Spalte `column` zuzugreifen, wählt man im internen Array erst die Spalte (`column`) und dann den Eintrag darin (`row`).

```cpp
template <class FLOAT, size_t N>
FLOAT SquareMatrix<FLOAT, N>::at(size_t row, size_t column) const {
  return matrix[column][row]; // Erst Spalte wählen, dann Zeile
}

template <class FLOAT, size_t N>
FLOAT & SquareMatrix<FLOAT, N>::at(size_t row, size_t column) {
  return matrix[column][row];
}
```

### Spaltenzugriff (`operator[]`)
Dies gibt einfach den `i`-ten Spaltenvektor zurück.

```cpp
template <class FLOAT, size_t N>
Vector<FLOAT, N> & SquareMatrix<FLOAT, N>::operator[](std::size_t i) {
  return matrix[i];
}
```

### Matrix-Vektor-Multiplikation (`operator*`)
Hier wird ein Vektor mit der Matrix transformiert: $y = M \cdot x$.
Mathematisch ist dies eine Linearkombination der Spaltenvektoren der Matrix, gewichtet mit den Komponenten des Eingabevektors.
$y = x_0 \cdot S_0 + x_1 \cdot S_1 + ... + x_n \cdot S_n$

```cpp
template <class FLOAT, size_t N>
Vector<FLOAT,N> SquareMatrix<FLOAT, N>::operator*(const Vector<FLOAT,N> vector) const {
  // Start mit einem Null-Vektor
  Vector<FLOAT, N> result({static_cast<FLOAT>(0.0)}); 
  
  for (size_t col = 0; col < N; ++col) {
      // Addiere die col-te Spalte, skaliert mit dem col-ten Wert des Vektors
      result += vector[col] * matrix[col];
  }
  return result;
}
```

### Matrix-Matrix-Multiplikation (`operator*`)
Das Produkt zweier Matrizen $C = A \cdot B$ wird ebenfalls spaltenweise berechnet.
Die $i$-te Spalte der Ergebnismatrix $C$ ist das Produkt der Matrix $A$ mit der $i$-ten Spalte von Matrix $B$.

```cpp
template <class F, size_t K>
SquareMatrix<F, K> operator*(const SquareMatrix<F, K> factor1, const SquareMatrix<F, K> factor2) {
  SquareMatrix<F, K> result; // Leere/Uninitialisierte Matrix
  for (size_t col = 0; col < K; ++col) {
      // Nutzt den oben definierten Matrix-Vektor-Operator
      // Spalte 'col' des Ergebnisses = Matrix A * Spalte 'col' von Matrix B
      result[col] = factor1 * factor2[col];
  }
  return result;
}
```

---

## 3. Scrolling-Logik ("Toroidal World")

In `game.cc` wird die Bewegungslogik implementiert. Asteroids nutzt eine "Endlos-Welt". Wenn ein Objekt den Bildschirm auf einer Seite verlässt, kommt es auf der gegenüberliegenden Seite wieder herein.

Die Funktion `displacement_fix` kümmert sich genau darum ("Toroidal Warp").

```cpp
// game.cc

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = (SCREEN_WIDTH * 3) / 4; // 768

void displacement_fix(Body2df * body, float seconds) {
  // Hole aktuelle Position
  float x = body->get_position()[0];
  float y = body->get_position()[1];
  Vector2df new_position = body->get_position();
  
  // Prüfe X-Achse (Links/Rechts)
  if ( x < 0 ) {
    // Wenn zu weit links -> Setze an den rechten Rand
    new_position[0] = SCREEN_WIDTH;
  }
  if ( x > SCREEN_WIDTH ) {
    // Wenn zu weit rechts -> Setze an den linken Rand (0)
    new_position[0] = 0;
  }
  
  // Prüfe Y-Achse (Oben/Unten)
  if ( y < 0 ) {
     // Wenn zu weit (vermutlich oben, je nach Koord-System) -> Setze ans andere Ende
    new_position[1] = SCREEN_HEIGHT;
  }
  if ( y > SCREEN_HEIGHT ) {
    // Wenn zu weit unten -> Setze auf 0
    new_position[1] = 0;
  }
  
  // Aktualisiere die Position des Objekts
  body->set_position(new_position);
}
```
**Erklärung:**
1.  Die Funktion erhält einen Zeiger auf einen Körper (`body`) und die vergangene Zeit (wird hier nicht direkt für die Positionskorrektur genutzt).
2.  Sie prüft, ob die x-Koordinate kleiner als 0 oder größer als `SCREEN_WIDTH` ist.
3.  Ist dies der Fall, wird die Koordinate "umgewickelt" (wrap-around).
4.  Dasselbe geschieht für die y-Koordinate bezüglich `SCREEN_HEIGHT`.
5.  `body->set_position` schreibt die korrigierten Werte zurück.

Dies ermöglicht, dass man endlos in eine Richtung fliegen kann, ohne gegen eine unsichtbare Wand zu stoßen.

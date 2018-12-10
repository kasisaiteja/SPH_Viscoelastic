#ifndef HELPER_COLOR_H
#define HELPER_COLOR_H

#include "helper.h"

#include <string>

namespace helper {

class Color {
 public:

  // Components.
  float r;
  float g;
  float b;
  float a;

  // constants
  static const Color White;
  static const Color Black;

  Color( float r = 0, float g = 0, float b = 0, float a = 1.0 )
      : r( r ), g( g ), b( b ), a( a ) { }

   Color( const unsigned char* arr );

  inline Color operator+( const Color& rhs ) const {
    return Color( r + rhs.r, g + rhs.g, b + rhs.b, a + rhs.a);
  }

  inline Color& operator+=( const Color& rhs ) {
    r += rhs.r; g += rhs.g; b += rhs.b; a += rhs.a;
    return *this;
  }

  inline Color operator*( const Color& rhs ) const {
    return Color( r * rhs.r, g * rhs.g, b * rhs.b, a * rhs.a);
  }

  inline Color& operator*=( const Color& rhs ) {
    r *= rhs.r; g *= rhs.g; b *= rhs.b; a *= rhs.a;
    return *this;
  }

  inline Color operator*( float s ) const {
    return Color( r * s, g * s, b * s, a * s );
  }

  inline Color& operator*=( float s ) {
    r *= s; g *= s; b *= s; a *= s;
    return *this;
  }

  // comparison
  inline bool operator==( const Color& rhs ) const {
    return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
  }

  inline bool operator!=( const Color& rhs ) const {
    return !operator==( rhs );
  }

  static Color fromHex( const char* s );

  std::string toHex( ) const;


}; // class Color


inline Color operator*( float s, const Color& c ) {
  return c * s;
}

// Prints components.
std::ostream& operator<<( std::ostream& os, const Color& c );

}

#endif

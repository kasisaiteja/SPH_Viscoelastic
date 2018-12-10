#include "stdafx.h"
#include "vector3D.h"

namespace helper {

  std::ostream& operator<<( std::ostream& os, const Vector3D& v ) {
    os << "(" << v.x << "," << v.y << "," << v.z << ")";
    return os;
  }

} 

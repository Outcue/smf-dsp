#pragma once
#include <iosfwd>

template <class T> class PointT;
typedef PointT<int> Point;

template <class T> struct Point_Traits;

//------------------------------------------------------------------------------
template <class T>
class PointT : public Point_Traits<T>::storage_type {
 public:
  typedef typename Point_Traits<T>::storage_type storage_type;

  PointT() : storage_type{} {}
  PointT(storage_type s) : storage_type(s) {}
  PointT(T x, T y) : storage_type{x, y} {}
};

//------------------------------------------------------------------------------
#include <SDL_rect.h>
template <> struct Point_Traits<int> {
  typedef SDL_Point storage_type;
};

template <class T> struct Point_Traits {
  struct storage_type { T x, y; };
};

//------------------------------------------------------------------------------
template <class T, class Ch>
std::basic_ostream<Ch> &operator<<(std::basic_ostream<Ch> &o, const PointT<T> &p) {
  return o << "{" << p.x << ", " << p.y << "}";
}

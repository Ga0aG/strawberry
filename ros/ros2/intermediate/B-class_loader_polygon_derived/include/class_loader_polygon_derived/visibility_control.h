#ifndef CLASS_LOADER_POLYGON_DERIVED__VISIBILITY_CONTROL_H_
#define CLASS_LOADER_POLYGON_DERIVED__VISIBILITY_CONTROL_H_

// This logic was borrowed (then namespaced) from the examples on the gcc wiki:
//     https://gcc.gnu.org/wiki/Visibility

#if defined _WIN32 || defined __CYGWIN__
#ifdef __GNUC__
#define CLASS_LOADER_POLYGON_DERIVED_EXPORT __attribute__((dllexport))
#define CLASS_LOADER_POLYGON_DERIVED_IMPORT __attribute__((dllimport))
#else
#define CLASS_LOADER_POLYGON_DERIVED_EXPORT __declspec(dllexport)
#define CLASS_LOADER_POLYGON_DERIVED_IMPORT __declspec(dllimport)
#endif
#ifdef CLASS_LOADER_POLYGON_DERIVED_BUILDING_LIBRARY
#define CLASS_LOADER_POLYGON_DERIVED_PUBLIC CLASS_LOADER_POLYGON_DERIVED_EXPORT
#else
#define CLASS_LOADER_POLYGON_DERIVED_PUBLIC CLASS_LOADER_POLYGON_DERIVED_IMPORT
#endif
#define CLASS_LOADER_POLYGON_DERIVED_PUBLIC_TYPE                               \
  CLASS_LOADER_POLYGON_DERIVED_PUBLIC
#define CLASS_LOADER_POLYGON_DERIVED_LOCAL
#else
#define CLASS_LOADER_POLYGON_DERIVED_EXPORT                                    \
  __attribute__((visibility("default")))
#define CLASS_LOADER_POLYGON_DERIVED_IMPORT
#if __GNUC__ >= 4
#define CLASS_LOADER_POLYGON_DERIVED_PUBLIC                                    \
  __attribute__((visibility("default")))
#define CLASS_LOADER_POLYGON_DERIVED_LOCAL __attribute__((visibility("hidden")))
#else
#define CLASS_LOADER_POLYGON_DERIVED_PUBLIC
#define CLASS_LOADER_POLYGON_DERIVED_LOCAL
#endif
#define CLASS_LOADER_POLYGON_DERIVED_PUBLIC_TYPE
#endif

#endif // CLASS_LOADER_POLYGON_DERIVED__VISIBILITY_CONTROL_H_

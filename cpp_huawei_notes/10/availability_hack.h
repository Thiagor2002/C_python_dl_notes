#if __cplusplus >= 201703L && defined(__APPLE__) && defined(__MACH__) &&   \
    ((defined(__clang__) && defined(__apple_build_version__) &&            \
      __apple_build_version__ >= 10000000) ||                              \
     (defined(__clang__) && __clang_major__ >= 8))
#include <Availability.h>  // __MAC_OS_X_VERSION_MIN_REQUIRED
#if (defined(__MAC_OS_X_VERSION_MIN_REQUIRED) &&                           \
     __MAC_OS_X_VERSION_MIN_REQUIRED < 101400)
#include <__config>  // _LIBCPP_AVAILABILITY_BAD_OPTIONAL_ACCESS
#undef  _LIBCPP_AVAILABILITY_BAD_OPTIONAL_ACCESS
#define _LIBCPP_AVAILABILITY_BAD_OPTIONAL_ACCESS
#endif
#endif

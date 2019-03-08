#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([src/glsl/standalone_scaffolding.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lEGL':
AC_CHECK_LIB([EGL], [main])
# FIXME: Replace `main' with a function in `-lX11':
AC_CHECK_LIB([X11], [main])
# FIXME: Replace `main' with a function in `-lXext':
AC_CHECK_LIB([Xext], [main])
# FIXME: Replace `main' with a function in `-lXfixes':
AC_CHECK_LIB([Xfixes], [main])
# FIXME: Replace `main' with a function in `-lXplugin':
AC_CHECK_LIB([Xplugin], [main])
# FIXME: Replace `main' with a function in `-lXv':
AC_CHECK_LIB([Xv], [main])
# FIXME: Replace `main' with a function in `-lXvMC':
AC_CHECK_LIB([XvMC], [main])
# FIXME: Replace `main' with a function in `-lXvMCW':
AC_CHECK_LIB([XvMCW], [main])
# FIXME: Replace `main' with a function in `-lc80':
AC_CHECK_LIB([c80], [main])
# FIXME: Replace `main' with a function in `-lclangAST':
AC_CHECK_LIB([clangAST], [main])
# FIXME: Replace `main' with a function in `-lclangAnalysis':
AC_CHECK_LIB([clangAnalysis], [main])
# FIXME: Replace `main' with a function in `-lclangBasic':
AC_CHECK_LIB([clangBasic], [main])
# FIXME: Replace `main' with a function in `-lclangCodeGen':
AC_CHECK_LIB([clangCodeGen], [main])
# FIXME: Replace `main' with a function in `-lclangDriver':
AC_CHECK_LIB([clangDriver], [main])
# FIXME: Replace `main' with a function in `-lclangEdit':
AC_CHECK_LIB([clangEdit], [main])
# FIXME: Replace `main' with a function in `-lclangFrontend':
AC_CHECK_LIB([clangFrontend], [main])
# FIXME: Replace `main' with a function in `-lclangFrontendTool':
AC_CHECK_LIB([clangFrontendTool], [main])
# FIXME: Replace `main' with a function in `-lclangLex':
AC_CHECK_LIB([clangLex], [main])
# FIXME: Replace `main' with a function in `-lclangParse':
AC_CHECK_LIB([clangParse], [main])
# FIXME: Replace `main' with a function in `-lclangRewrite':
AC_CHECK_LIB([clangRewrite], [main])
# FIXME: Replace `main' with a function in `-lclangSema':
AC_CHECK_LIB([clangSema], [main])
# FIXME: Replace `main' with a function in `-lclangSerialization':
AC_CHECK_LIB([clangSerialization], [main])
# FIXME: Replace `main' with a function in `-ld3d10':
AC_CHECK_LIB([d3d10], [main])
# FIXME: Replace `main' with a function in `-ldflags':
AC_CHECK_LIB([dflags], [main])
# FIXME: Replace `main' with a function in `-ldl':
AC_CHECK_LIB([dl], [main])
# FIXME: Replace `main' with a function in `-ldrm':
AC_CHECK_LIB([drm], [main])
# FIXME: Replace `main' with a function in `-ldrm_intel':
AC_CHECK_LIB([drm_intel], [main])
# FIXME: Replace `main' with a function in `-ldrm_nouveau':
AC_CHECK_LIB([drm_nouveau], [main])
# FIXME: Replace `main' with a function in `-ldrm_radeon':
AC_CHECK_LIB([drm_radeon], [main])
# FIXME: Replace `main' with a function in `-ldxgi':
AC_CHECK_LIB([dxgi], [main])
# FIXME: Replace `main' with a function in `-lgallium':
AC_CHECK_LIB([gallium], [main])
# FIXME: Replace `main' with a function in `-lgbm':
AC_CHECK_LIB([gbm], [main])
# FIXME: Replace `main' with a function in `-lgdi32':
AC_CHECK_LIB([gdi32], [main])
# FIXME: Replace `main' with a function in `-linker':
AC_CHECK_LIB([inker], [main])
# FIXME: Replace `main' with a function in `-linux':
AC_CHECK_LIB([inux], [main])
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])

# Checks for header files.
AC_PATH_X
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([OS.h arpa/inet.h fcntl.h float.h inttypes.h libintl.h limits.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h sys/timeb.h unistd.h wchar.h wctype.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_HEADER_MAJOR
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_FUNC_STRTOD
AC_CHECK_FUNCS([atexit bzero clock_gettime dup2 floor ftruncate getcwd gethostbyname getpagesize gettimeofday isascii localeconv memmove memset mkdir munmap pow regcomp rint rmdir select setenv socket sqrt strcasecmp strchr strcspn strdup strerror strrchr strspn strstr strtol strtoul strtoull])

AC_CONFIG_FILES([Makefile
                 doxygen/Makefile
                 src/Makefile
                 src/egl/Makefile
                 src/egl/drivers/Makefile
                 src/egl/drivers/dri2/Makefile
                 src/egl/drivers/glx/Makefile
                 src/egl/main/Makefile
                 src/egl/wayland/Makefile
                 src/egl/wayland/wayland-drm/Makefile
                 src/egl/wayland/wayland-egl/Makefile
                 src/gallium/Makefile
                 src/gallium/auxiliary/Makefile
                 src/gallium/auxiliary/pipe-loader/Makefile
                 src/gallium/auxiliary/pipebuffer/Makefile
                 src/gallium/docs/Makefile
                 src/gallium/drivers/Makefile
                 src/gallium/drivers/i915/Makefile
                 src/gallium/drivers/llvmpipe/Makefile
                 src/gallium/drivers/nouveau/Makefile
                 src/gallium/drivers/nv30/Makefile
                 src/gallium/drivers/nv50/Makefile
                 src/gallium/drivers/nvc0/Makefile
                 src/gallium/drivers/r300/Makefile
                 src/gallium/drivers/r600/Makefile
                 src/gallium/drivers/radeon/Makefile
                 src/gallium/drivers/radeonsi/Makefile
                 src/gallium/drivers/rbug/Makefile
                 src/gallium/drivers/softpipe/Makefile
                 src/gallium/drivers/svga/Makefile
                 src/gallium/drivers/trace/Makefile
                 src/gallium/state_trackers/Makefile
                 src/gallium/state_trackers/clover/Makefile
                 src/gallium/state_trackers/d3d1x/Makefile
                 src/gallium/state_trackers/d3d1x/d3d1xshader/Makefile
                 src/gallium/state_trackers/d3d1x/d3d1xstutil/Makefile
                 src/gallium/state_trackers/d3d1x/d3dapi/Makefile
                 src/gallium/state_trackers/d3d1x/docs/Makefile
                 src/gallium/state_trackers/d3d1x/dxgi/Makefile
                 src/gallium/state_trackers/d3d1x/dxgid3d10/Makefile
                 src/gallium/state_trackers/d3d1x/dxgid3d11/Makefile
                 src/gallium/state_trackers/d3d1x/gd3d10/Makefile
                 src/gallium/state_trackers/d3d1x/gd3d11/Makefile
                 src/gallium/state_trackers/d3d1x/gd3d1x/Makefile
                 src/gallium/state_trackers/d3d1x/gd3dapi/Makefile
                 src/gallium/state_trackers/d3d1x/progs/Makefile
                 src/gallium/state_trackers/d3d1x/winedlls/Makefile
                 src/gallium/state_trackers/d3d1x/winedlls/d3d10/Makefile
                 src/gallium/state_trackers/d3d1x/winedlls/d3d10_1/Makefile
                 src/gallium/state_trackers/d3d1x/winedlls/d3d11/Makefile
                 src/gallium/state_trackers/d3d1x/winedlls/dxgi/Makefile
                 src/gallium/state_trackers/dri/Makefile
                 src/gallium/state_trackers/dri/drm/Makefile
                 src/gallium/state_trackers/dri/sw/Makefile
                 src/gallium/state_trackers/egl/Makefile
                 src/gallium/state_trackers/gbm/Makefile
                 src/gallium/state_trackers/glx/Makefile
                 src/gallium/state_trackers/glx/xlib/Makefile
                 src/gallium/state_trackers/va/Makefile
                 src/gallium/state_trackers/vdpau/Makefile
                 src/gallium/state_trackers/vega/Makefile
                 src/gallium/state_trackers/xa/Makefile
                 src/gallium/state_trackers/xorg/Makefile
                 src/gallium/state_trackers/xvmc/Makefile
                 src/gallium/state_trackers/xvmc/tests/Makefile
                 src/gallium/targets/Makefile
                 src/gallium/targets/dri-i915/Makefile
                 src/gallium/targets/dri-nouveau/Makefile
                 src/gallium/targets/dri-r300/Makefile
                 src/gallium/targets/dri-r600/Makefile
                 src/gallium/targets/dri-radeonsi/Makefile
                 src/gallium/targets/dri-swrast/Makefile
                 src/gallium/targets/dri-vmwgfx/Makefile
                 src/gallium/targets/egl-static/Makefile
                 src/gallium/targets/gbm/Makefile
                 src/gallium/targets/libgl-xlib/Makefile
                 src/gallium/targets/opencl/Makefile
                 src/gallium/targets/pipe-loader/Makefile
                 src/gallium/targets/va-r300/Makefile
                 src/gallium/targets/va-r600/Makefile
                 src/gallium/targets/va-softpipe/Makefile
                 src/gallium/targets/vdpau-nouveau/Makefile
                 src/gallium/targets/vdpau-r300/Makefile
                 src/gallium/targets/vdpau-r600/Makefile
                 src/gallium/targets/vdpau-radeonsi/Makefile
                 src/gallium/targets/vdpau-softpipe/Makefile
                 src/gallium/targets/xa-vmwgfx/Makefile
                 src/gallium/targets/xorg-i915/Makefile
                 src/gallium/targets/xorg-nouveau/Makefile
                 src/gallium/targets/xorg-r300/Makefile
                 src/gallium/targets/xorg-r600/Makefile
                 src/gallium/targets/xorg-radeonsi/Makefile
                 src/gallium/targets/xvmc-nouveau/Makefile
                 src/gallium/targets/xvmc-r300/Makefile
                 src/gallium/targets/xvmc-r600/Makefile
                 src/gallium/targets/xvmc-softpipe/Makefile
                 src/gallium/tests/trivial/Makefile
                 src/gallium/tests/unit/Makefile
                 src/gallium/winsys/Makefile
                 src/gallium/winsys/i915/drm/Makefile
                 src/gallium/winsys/i915/sw/Makefile
                 src/gallium/winsys/nouveau/drm/Makefile
                 src/gallium/winsys/radeon/drm/Makefile
                 src/gallium/winsys/svga/drm/Makefile
                 src/gallium/winsys/sw/Makefile
                 src/gallium/winsys/sw/dri/Makefile
                 src/gallium/winsys/sw/fbdev/Makefile
                 src/gallium/winsys/sw/null/Makefile
                 src/gallium/winsys/sw/wayland/Makefile
                 src/gallium/winsys/sw/wrapper/Makefile
                 src/gallium/winsys/sw/xlib/Makefile
                 src/gbm/Makefile
                 src/glsl/Makefile
                 src/glsl/glcpp/Makefile
                 src/glsl/tests/Makefile
                 src/glx/Makefile
                 src/glx/apple/Makefile
                 src/glx/tests/Makefile
                 src/gtest/Makefile
                 src/mapi/es1api/Makefile
                 src/mapi/es2api/Makefile
                 src/mapi/glapi/Makefile
                 src/mapi/glapi/gen/Makefile
                 src/mapi/glapi/tests/Makefile
                 src/mapi/shared-glapi/Makefile
                 src/mapi/shared-glapi/tests/Makefile
                 src/mapi/vgapi/Makefile
                 src/mesa/Makefile
                 src/mesa/drivers/Makefile
                 src/mesa/drivers/dri/Makefile
                 src/mesa/drivers/dri/common/Makefile
                 src/mesa/drivers/dri/common/xmlpool/Makefile
                 src/mesa/drivers/dri/i915/Makefile
                 src/mesa/drivers/dri/i965/Makefile
                 src/mesa/drivers/dri/nouveau/Makefile
                 src/mesa/drivers/dri/r200/Makefile
                 src/mesa/drivers/dri/radeon/Makefile
                 src/mesa/drivers/dri/swrast/Makefile
                 src/mesa/drivers/osmesa/Makefile
                 src/mesa/drivers/x11/Makefile
                 src/mesa/libdricore/Makefile
                 src/mesa/main/tests/Makefile
                 src/mesa/state_tracker/Makefile
                 src/mesa/x86-64/Makefile
                 src/mesa/x86/Makefile])
AC_OUTPUT

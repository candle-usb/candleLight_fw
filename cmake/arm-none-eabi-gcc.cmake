set(TOOLCHAIN arm-none-eabi_14.2.rel1-1)

find_path(TOOLCHAIN_BIN_DIR
	NAMES arm-none-eabi-gcc arm-none-eabi-gcc.exe
	HINTS
		ENV TOOLCHAIN_BIN_DIR
		"$ENV{HOME}/${TOOLCHAIN}/bin"
		"/opt/${TOOLCHAIN}/bin"
		"/srv/${TOOLCHAIN}/bin"
		"/usr/local/${TOOLCHAIN}/bin"
		"C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/14.2 rel1/bin"
	DOC "Path to the ARM toolchain binaries"
)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(WIN32)
	set(TOOLCHAIN_EXE_SUFFIX .exe)
else()
	set(TOOLCHAIN_EXE_SUFFIX "")
endif()

set(CMAKE_C_COMPILER "${TOOLCHAIN_BIN_DIR}/arm-none-eabi-gcc${TOOLCHAIN_EXE_SUFFIX}")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_BIN_DIR}/arm-none-eabi-gcc${TOOLCHAIN_EXE_SUFFIX}")
set(CMAKE_C_COMPILER_TARGET arm-none-eabi)
set(CMAKE_AR "${TOOLCHAIN_BIN_DIR}/arm-none-eabi-gcc-ar${TOOLCHAIN_EXE_SUFFIX}")
set(CMAKE_RANLIB "${TOOLCHAIN_BIN_DIR}/arm-none-eabi-gcc-ranlib${TOOLCHAIN_EXE_SUFFIX}")
set(CMAKE_LINKER "${TOOLCHAIN_BIN_DIR}/arm-none-eabi-gcc${TOOLCHAIN_EXE_SUFFIX}")
set(CMAKE_OBJCOPY "${TOOLCHAIN_BIN_DIR}/arm-none-eabi-objcopy${TOOLCHAIN_EXE_SUFFIX}")
set(CMAKE_SIZE "${TOOLCHAIN_BIN_DIR}/arm-none-eabi-size${TOOLCHAIN_EXE_SUFFIX}")

SET(CMAKE_C_COMPILER_WORKS 1)
SET(CMAKE_CXX_COMPILER_WORKS 1)
SET(CMAKE_ASM_COMPILER_WORKS 1)

set(TOOLCHAIN arm-none-eabi_14.2.rel1-1)

find_path(TOOLCHAIN_BIN_DIR
	NAMES arm-none-eabi-gcc
	HINTS
		ENV TOOLCHAIN_BIN_DIR
		"$ENV{HOME}/${TOOLCHAIN}/bin"
		"/opt/${TOOLCHAIN}/bin"
		"/srv/${TOOLCHAIN}/bin"
		"/usr/local/${TOOLCHAIN}/bin"
	DOC "Path to the ARM toolchain binaries"
)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER "${TOOLCHAIN_BIN_DIR}/arm-none-eabi-gcc")
set(CMAKE_C_COMPILER_TARGET arm-none-eabi)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

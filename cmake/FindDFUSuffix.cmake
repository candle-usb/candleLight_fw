# find and set DFU_SUFFIX_EXECUTABLE

find_program(DFU_SUFFIX_EXECUTABLE
	NAMES dfu-suffix
	DOC "dfu-suffix executable"
)

find_program(DFU_UTIL_EXECUTABLE
	NAMES dfu-util
	DOC "dfu-util executable"
)

mark_as_advanced(
	DFU_SUFFIX_EXECUTABLE
	DFU_UTIL_EXECUTABLE
)

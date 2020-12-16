# find and set DFU_SUFFIX_EXECUTABLE

find_program ( DFU_SUFFIX_EXECUTABLE
	NAMES dfu-suffix
	DOC "dfu-suffix executable"
)

mark_as_advanced ( DFU_SUFFIX_EXECUTABLE )


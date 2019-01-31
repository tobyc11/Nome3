add_library(imgui
	imgui.cpp
	imgui.h
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_widgets.cpp
	imgui_internal.h
	imconfig.h
	imstb_rectpack.h
	imstb_textedit.h
	imstb_truetype.h
)
target_include_directories(imgui INTERFACE .)
target_compile_options(imgui ${DEFAULT_COMPILE_OPTIONS})

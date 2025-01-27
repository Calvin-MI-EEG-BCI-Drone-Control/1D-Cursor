# g++ DatasetCollector.cpp Collector.cpp GUIs/ExperimentGUI.cpp GUIs/SubjectGUI.cpp GUIs/imgui/imgui.cpp GUIs/imgui/imgui_draw.cpp GUIs/imgui/imgui_tables.cpp GUIs/imgui/imgui_widgets.cpp GUIs/imgui/imgui_impl_glfw.cpp GUIs/imgui/imgui_impl_opengl3.cpp -o DatasetCollector -I"../iWorxDAQ_64" -L"../iWorxDAQ_64" -liwxDAQ -I"$env:VCPKG_ROOT/installed/x64-windows/include" -L"$env:VCPKG_ROOT/installed/x64-windows/lib" -lsqlite3 -I. -L. -lglfw3 -lopengl32
CC		= g++

PROG	= DatasetCollector

GUIPROG	= GUIs/ExperimentGUI.cpp GUIs/SubjectGUI.cpp GUIs/imgui/imgui.cpp GUIs/imgui/imgui_draw.cpp GUIs/imgui/imgui_tables.cpp GUIs/imgui/imgui_widgets.cpp GUIs/imgui/imgui_impl_glfw.cpp GUIs/imgui/imgui_impl_opengl3.cpp
SRCS	= $(PROG).cpp Collector.cpp $(GUIPROG)

IWORX	= -I"../iWorxDAQ_64" -L"../iWorxDAQ_64" -liwxDAQ
# following package installed using VCPKG
SQLITE	= -I"$env:VCPKG_ROOT/installed/x64-windows/include" -L"$env:VCPKG_ROOT/installed/x64-windows/lib" -lsqlite3
IMGUI	= -I. -L. -lglfw3 -lopengl32
LIBS	= $(IWORX) $(SQLITE) $(IMGUI)

all: $(PROG)

$(PROG): $(SRCS)
	$(CC) $(SRCS) -o $(LIBS)

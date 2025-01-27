// #include "GUIs/ExperimentGUI.h"
#include "GUIs/SubjectGUI.h"
#include "Collector.h"

#include <thread>
#include <iostream>
#include <GLFW/glfw3.h>

using namespace std;

// compilation with iworx and sqlite
// g++ DatasetCollector.cpp -o DatasetCollector -I"../iWorxDAQ_64" -L"../iWorxDAQ_64" -liwxDAQ -I"$env:VCPKG_ROOT/installed/x64-windows/include" -L"$env:VCPKG_ROOT/installed/x64-windows/lib" -lsqlite3

// compilation with iworx, sqlite, and imgui
// g++ DatasetCollector.cpp Collector.cpp GUIs/ExperimentGUI.cpp GUIs/SubjectGUI.cpp GUIs/imgui/imgui.cpp GUIs/imgui/imgui_draw.cpp GUIs/imgui/imgui_tables.cpp GUIs/imgui/imgui_widgets.cpp GUIs/imgui/imgui_impl_glfw.cpp GUIs/imgui/imgui_impl_opengl3.cpp -o DatasetCollector -I"../iWorxDAQ_64" -L"../iWorxDAQ_64" -liwxDAQ -I"$env:VCPKG_ROOT/installed/x64-windows/include" -L"$env:VCPKG_ROOT/installed/x64-windows/lib" -lsqlite3 -I. -L. -lglfw3 -lopengl32

/** USAGE
 * ./DatasetCollector <database>
 */

int _tmain(int argc, char **argv)
{
	// shared variables
	// TODO

	// Initialize GLFW in the main thread
    // if (!glfwInit()) {
    //     std::cerr << "Failed to initialize GLFW\n";
    //     return -1;
    // }

    // Set GLFW window hints
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create windows in the main thread
	// GLFWwindow* ExperimentWindow = glfwCreateWindow(800, 600, "Experiment Example", nullptr, nullptr);
    // GLFWwindow* SubjectWindow = glfwCreateWindow(800, 600, "Subject Example", nullptr, nullptr);

    // if (!ExperimentWindow || !SubjectWindow) {
    //     std::cerr << "Failed to create GLFW windows\n";
    //     glfwTerminate();
    //     return -1;
    // }

	// thread experimentGUI(startExperimentGUI, ExperimentWindow);
	// thread subjectGUI(startSubjectGUI, SubjectWindow);
	// thread dataThread(dataCollector, argc, argv);

	// startSubjectGUI();
	dataCollector(argc, argv);

	// // experimentGUI.join();
	// subjectGUI.join();
	// dataThread.join();

    // glfwTerminate();

	return 0;
}
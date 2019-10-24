#include "GLFW/glfw3.h"
#include <stdlib.h>


// Necessary clean up to avoid leakage
void properExit(int code) {
	glfwTerminate();
	exit(code);
}

void conditionalExit(int code) {
	if (code) {
		properExit(code);
	}
}
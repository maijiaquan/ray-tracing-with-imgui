/*
ppm图片输出
*/

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include <stdio.h>
//#include <sys/time.h> // for gettimeofday()
#include <ctime>
//#include <unistd.h>
#include "vec3.h"
#include "draw.h"
#include <cstdlib>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#include <thread>
#include <fstream>
#include <vector>
#include <ctime>
//#include <sys/time.h>
#include<string>

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

using namespace std;

long curTime;
long totTime;
float timeRemaining;

//全局变量

ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.00f, 1.00f);
float progressDone = 0.0f, progressDir = 1.0f;
bool gRayTracingBegin = false;

int numPixelTotal;
int numThread = 50;
int numPixelRendered = 0;
int doneRecord = 0;

static int speedFactor = 1;

void DrawFrame()
{

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, display_w, 0.0, display_h, 0.0, 1.0);
    glBegin(GL_POINTS);

    for (int i = 0; i < display_h; i++)
    {

        int r, g, b;
        for (int j = 0; j < display_w; j++)
        {
            Color2RGB(framebuffer[i * display_w + j], r, g, b);
            glColor3f((float)r / 255, (float)g / 255, (float)b / 255);
            glVertex3f(j + 20, display_h - i-200, 0);
        }
    }

    glEnd();
}
//记录进度和时间
//void RecordProgressAndTime()
//{
//    progressDone = float(numPixelRendered) / (numPixelTotal);
//    totTime = (GetCurrentTimeMs() - curTime);
//    timeRemaining = totTime * (1 - progressDone) / progressDone / 1000; //根据过去的平均时间来计算剩余时间 x/t=pTodo/pDone -> x= t*pTodo/pDone = t*(1-pDone) /pDone
//}

int speedCount = 0;
void RayTracingInOneThread(int k)
{
    for (int j = ny - k; j >= 0; j -= numThread)
    {
        //RecordProgressAndTime();
        for (int i = 0; i < nx; i++)
        {
            //usleep(((j)%(numThread)+1)*25000/speedFactor);
            vec3 col(float(i) / float(nx), float(j) / float(ny), 0.8);
            int ir = int(255.99 * col[0]);
            int ig = int(255.99 * col[1]);
            int ib = int(255.99 * col[2]);
            DrawPixel(i, j, ir, ig, ib);
            numPixelRendered++;
        }
    }
}

void RayTracing()
{
    while (!gRayTracingBegin)
    {
        //wait until begin
    }

	framebuffer = new int[display_w * display_h];

    ns = 100;

    ofstream outFile("output_" + to_string(nx) + "x" + to_string(ny) + ".ppm");
    outFile << "P3\n"
            << nx << " " << ny << "\n255\n";
    vector<thread> threads;
    numPixelTotal = nx * ny;

    for (int k = 0; k < numThread; k++)
    {
        threads.push_back(thread(RayTracingInOneThread, k));
    }

    for (auto &thread : threads)
    {
        thread.join();
    }
    Framebuffer2File(nx, ny, ns, framebuffer, outFile, progressDone);
    return;
}

int main(int, char **)
{
    totTime = 0;
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    thread t(RayTracing);

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;
    GLFWwindow *window = glfwCreateWindow(1800, 900, "Ray Tracing", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {

        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Ray Tracing"); // Create a window called "Hello, world!" and append into it.

            if (ImGui::Button("Start")) // Buttons return true when clicked (most widgets return true when edited/activated)
            {
                gRayTracingBegin = true;
            }

            ImGui::ProgressBar(progressDone, ImVec2(0.0f, 0.0f));
            // ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::InputInt(" speed", &speedFactor);
            // ImGui::SameLine();
            ImGui::Text("Thread Num: %d ", numThread);
            ImGui::Text("Image Size:  %d x %d ", nx, ny);

            // ImGui::Text("Progress Bar");
            ImGui::Text("Total time %.3f s", (float)totTime / 1000);
            ImGui::Text("timeRemaining time %.3f s", timeRemaining);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        ImGui::Render();

        glfwGetFramebufferSize(window, &display_w, &display_h);

        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.00f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (gRayTracingBegin)
        {
            DrawFrame();
        }

        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

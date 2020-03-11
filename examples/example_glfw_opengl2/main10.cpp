/*
代码改动：
1.新增函数random_in_unit_disk()
2.修改相机类
3.修改入口函数
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
#include <thread>
#include <fstream>
#include <vector>
#include <ctime>
//#include <sys/time.h>
#include<string>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GLFW/glfw3.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif



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
int numThread = 2;
int numPixelRendered = 0;
int doneRecord = 0;
int framebufferInited = false;
int gFrameFinished = false;

int *lastFrameBuffer;

int gFov = 20;
float aperture = 2.2;
vec3 lookfrom(3, 3, 2);
// vec3 lookfrom(-2, 2, 1);
vec3 lookat(0, 0, -1);
// vec3 lookat(-1.0, 0, -1);
float dist_to_focus = (lookfrom - lookat).length();

static int speedFactor = 1;



vec3 random_in_unit_sphere()
{
    vec3 p;
    do
    {
        p = 2.0 * vec3(random_double(), random_double(), random_double()) - vec3(1, 1, 1);
    } while (p.squared_length() >= 1.0);
    return p;
}

void DrawFrame(int *fb)
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
            Color2RGB(fb[i * display_w + j], r, g, b);
            glColor3f((float)r / 255, (float)g / 255, (float)b / 255);
            glVertex3f(j + 20, display_h - i - 300, 0);
        }
    }

    glEnd();
}
//记录进度和时间
void RecordProgressAndTime()
{
    progressDone = float(numPixelRendered) / (numPixelTotal);
    // totTime = (GetCurrentTimeMs() - curTime);
    // timeRemaining = totTime * (1 - progressDone) / progressDone / 1000; //根据过去的平均时间来计算剩余时间 x/t=pTodo/pDone -> x= t*pTodo/pDone = t*(1-pDone) /pDone
}

struct hit_record;

class ray
{
public:
    vec3 A; //起点
    vec3 B; //方向

    ray() {}
    ray(const vec3 &a, const vec3 &b)
    {
        A = a;
        B = b;
    }
    vec3 origin() const { return A; }
    vec3 direction() const { return B; }
    vec3 point_at_parameter(float t) const { return A + t * B; } //终点的坐标
};

class material
{
public:
    //r_in为入射光线, scattered为散射光线, attenuation 意思为衰减量，实际为各通道的反射率
    virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const = 0;
};

class camera
{
public:
    vec3 origin;
    vec3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
    float lens_radius;

    //lookfrom为相机位置，lookat为观察位置，vup传(0,1,0)，vfov为视野角度，aspect为屏幕宽高比
    //aperture为光圈大小，focus_dist为相机到观察点的距离
    camera(vec3 lookfrom, vec3 lookat, vec3 vup, float vfov, float aspect, float aperture, float focus_dist)
    {
        lens_radius = aperture / 2;
        float theta = vfov * M_PI / 180;
        float half_height = tan(theta / 2);
        float half_width = aspect * half_height;
        origin = lookfrom;
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);
        lower_left_corner = origin - half_width * focus_dist * u - half_height * focus_dist * v - focus_dist * w;
        horizontal = 2 * half_width * focus_dist * u;
        vertical = 2 * half_height * focus_dist * v;
    }

    ray get_ray(float s, float t)
    {
        vec3 rd = lens_radius * random_in_unit_disk();
        vec3 offset = u * rd.x() + v * rd.y();
        return ray(origin + offset, lower_left_corner + s * horizontal + t * vertical - origin - offset);
    }
};

//记录命中信息
struct hit_record
{
    float t;     //命中射线的长度
    vec3 p;      //命中终点坐标
    vec3 normal; //命中点的法向量
    material *mat_ptr; //new
};

//物体的虚基类
class hittable
{
public:
    virtual bool hit(const ray &r, float t_min, float t_max, hit_record &rec) const = 0;
};

class sphere : public hittable
{
public:
    vec3 center;
    float radius;
    material *mat_ptr; /* NEW */
    sphere() {}
    sphere(vec3 cen, float r, material *m) : center(cen), radius(r), mat_ptr(m){}; //new
    //如果命中了，命中记录保存到rec
    virtual bool hit(const ray &r, float t_min, float t_max, hit_record &rec) const
    {
        vec3 oc = r.origin() - center;
        float a = dot(r.direction(), r.direction());
        float b = dot(oc, r.direction());
        float c = dot(oc, oc) - radius * radius;
        float discriminant = b * b - a * c;
        if (discriminant > 0)
        {
            float temp = (-b - sqrt(discriminant)) / a; //小实数根
            if (temp < t_max && temp > t_min)
            {
                rec.t = temp;
                rec.p = r.point_at_parameter(rec.t);
                rec.normal = (rec.p - center) / radius;
                rec.mat_ptr = mat_ptr; /* NEW */
                return true;
            }
            temp = (-b + sqrt(discriminant)) / a; //大实数根
            if (temp < t_max && temp > t_min)
            {
                rec.t = temp;
                rec.p = r.point_at_parameter(rec.t);
                rec.normal = (rec.p - center) / radius;
                rec.mat_ptr = mat_ptr; /* NEW */
                return true;
            }
        }
        return false;
    }
};

class hittable_list : public hittable
{
public:
    hittable **list;
    int list_size;

    hittable_list() {}
    hittable_list(hittable **l, int n)
    {
        list = l;
        list_size = n;
    }
    //如果命中了，命中记录保存到rec
    virtual bool hit(const ray &r, float t_min, float t_max, hit_record &rec) const
    {
        hit_record temp_rec;
        bool hit_anything = false;
        double closest_so_far = t_max; //记录目前最近的t值
        for (int i = 0; i < list_size; i++)
        {
            if (list[i]->hit(r, t_min, closest_so_far, temp_rec))
            {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec; //只记录打到的最近的球
            }
        }
        return hit_anything;
    }
};

class lambertian : public material
{
public:
    vec3 albedo; //反射率
    lambertian(const vec3 &a) : albedo(a) {}
    virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
    {
        vec3 s_world = rec.p + rec.normal + random_in_unit_sphere();
        scattered = ray(rec.p, s_world - rec.p); //scattered为散射光线
        attenuation = albedo;	//注意这是各通道的反射率！
        return true;
    }
};

class metal : public material
{
public:
    vec3 albedo;
    float fuzz;

    metal(const vec3 &a, float f) : albedo(a)
    {
        fuzz = f < 1 ? f : 1;
    }

    virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
    {
        vec3 v = unit_vector(r_in.direction());
        vec3 n = rec.normal;
        vec3 p = rec.p;
        vec3 r = reflect(v, n);
        vec3 offset = fuzz * random_in_unit_sphere();
        scattered = ray(p, r + offset);

        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
    }
};

class dielectric : public material
{
public:
    dielectric(float ri) : ref_idx(ri) {} //n2/n1
    virtual bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
    {
        vec3 outward_normal;
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        float ni_over_nt;
        attenuation = vec3(1.0, 1.0, 1.0);
        vec3 refracted;

        float reflect_prob; //反射概率
        float cosine;

        if (dot(r_in.direction(), rec.normal) > 0) //从里到外，即入射向量在法向量另外一侧的情况
        {
            outward_normal = -rec.normal;   //对法向量取反
            ni_over_nt = ref_idx;           
            cosine = ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
            //不知道为什么这里要乘一个ref_idx
        }
        else    //从外到里，即入射向量在法向量同一侧
        {
            outward_normal = rec.normal;    //法向量不变
            ni_over_nt = 1.0 / ref_idx;     
            cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
        }

        if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
        {
            reflect_prob = schlick(cosine, ref_idx);
        }
        else    //若无折射，则全反射
        {
            reflect_prob = 1.0;
        }

        if (random_double() < reflect_prob)
        {
            scattered = ray(rec.p, reflected);
        }
        else
        {
            scattered = ray(rec.p, refracted);
        }

        return true;
    }

    float ref_idx;
};

//发射一条射线，并采样该射线最终输出到屏幕的颜色值值
vec3 color(const ray &r, hittable *world, int depth){
    hit_record rec;
    if (world->hit(r, 0.001, FLT_MAX, rec)) //射线命中物体
    {
        ray scattered; //散射光线
        vec3 attenuation; //其实是反射率！
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        {
            return attenuation * color(scattered, world, depth + 1);
        }
        else
        {
            return vec3(0, 0, 0);
        }
    }
    else
    {
        vec3 unit_direction = unit_vector(r.direction());
        float t = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
    }
}

hittable *list[4];  //new
hittable *world;

vec3 lower_left_corner(-2.0, -1.0, -1.0); //左下角
vec3 horizontal(4.0, 0.0, 0.0);
vec3 vertical(0.0, 2.0, 0.0);
vec3 origin(0.0, 0.0, 0.0); // 相机原点

void RayTracingInOneThread(int k)
{
    float R = cos(M_PI/4);
    // list[0] = new sphere(vec3(-R, 0, -1), R, new lambertian(vec3(0, 0, 1)));
    // list[1] = new sphere(vec3(R, 0, -1), R, new lambertian(vec3(1, 0, 0)));
    // world = new hittable_list(list, 2);

    list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.8, 0.3, 0.3)));
    // list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5)));
    // list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
    list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.1/2, 0.2/2, 0.5/2)));
    list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.3));    
    list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5));
    world = new hittable_list(list, 4); 




    ns = 100;
    // camera cam(vec3(-2, 2, 1), vec3(-0.5, 0, -1), vec3(0, 1, 0), gFov, float(nx) / float(ny));



    camera cam(lookfrom, lookat, vec3(0, 1, 0), gFov, float(nx) / float(ny), aperture, dist_to_focus);

    for (int j = ny - k; j >= 0; j -= numThread)
    {
        RecordProgressAndTime();
        for (int i = 0; i < nx; i++)
        {
            vec3 col(0, 0, 0);
            for(int s = 0; s < ns; s++)
            {
                float u = float(i + random_double()) / float(nx);
                float v = float(j + random_double()) / float(ny);
                ray r = cam.get_ray(u, v);
                col += color(r, world, 0); 
            }
            col /= float(ns);
            col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));  

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
    while(true)
    {
        while (!gRayTracingBegin)
        {
            //wait until begin
        }

        // curTime = GetCurrentTimeMs();
        gFrameFinished = false;
        if(!framebufferInited)
        {
            framebuffer = new int[display_w * display_h];
            lastFrameBuffer = new int[display_w * display_h];

            for (int i = 0; i < display_h * display_w; i++)
            {
                lastFrameBuffer[i] = 0;
            }
            framebufferInited = true; 
        }
        numPixelRendered = 0;

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
        gFrameFinished = true;
        Framebuffer2File(nx, ny, ns, framebuffer, outFile, progressDone);
        for(int i = 0; i < display_h*display_w; i++ )
        {
            lastFrameBuffer[i] = framebuffer[i];
        }
        gRayTracingBegin = false;
    }

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
            ImGui::Text("Camera fov:  %d ", gFov);
            ImGui::Text("Camera aperture: %.3f ", (float)aperture);
            ImGui::Text("Camera dist to focus: %.3f ", (float)dist_to_focus );

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

        if (framebufferInited)
        {
            if(gFrameFinished)
            {
                DrawFrame(framebuffer);
            }
            else
            {
                //DrawFrame(lastFrameBuffer);
				 DrawFrame(framebuffer);
            }
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

# RayTracing
## 项目介绍
本项目参考自教程[《Ray Tracing in One Weekend》](https://raytracing.github.io/books/RayTracingInOneWeekend.html)，使用了ImGUI的图形化界面框架，使用官方自带的一个OpenGL2的例子，目的是用直接绘制的方法，在屏幕上逐像素输出整张图片。目前在MacOS（Xcode 10.3）和Windows（Visual Studio 2015）环境中上能顺利运行，其他环境待测试。


> 笔记目录：<br>
> [《用两天学习光线追踪》1.项目介绍和ppm图片输出](https://blog.csdn.net/MASILEJFOAISEGJIAE/article/details/104601464)<br>
> [《用两天学习光线追踪》2.射线、简单相机和背景输出](https://blog.csdn.net/masilejfoaisegjiae/article/details/104614268)<br>
> [《用两天学习光线追踪》3.球体和表面法向量](https://blog.csdn.net/masilejfoaisegjiae/article/details/104614303)<br>
> [《用两天学习光线追踪》4.封装成类](https://blog.csdn.net/masilejfoaisegjiae/article/details/104614323)<br>
> [《用两天学习光线追踪》5.抗锯齿](https://blog.csdn.net/masilejfoaisegjiae/article/details/104614372)<br>
> [《用两天学习光线追踪》6.漫反射材质](https://blog.csdn.net/masilejfoaisegjiae/article/details/104614582)<br>
> [《用两天学习光线追踪》7.反射向量和金属材质](https://blog.csdn.net/masilejfoaisegjiae/article/details/104614921)<br>
> [《用两天学习光线追踪》8.折射向量和电介质](https://blog.csdn.net/masilejfoaisegjiae/article/details/104614953)<br>
> [《用两天学习光线追踪》9.可放置相机](https://blog.csdn.net/masilejfoaisegjiae/article/details/104614989)<br>
> [《用两天学习光线追踪》10.散焦模糊](https://blog.csdn.net/masilejfoaisegjiae/article/details/104615003)<br>


因为直接搬运了ImGUI的opengl2的例子，所以整个工程都是ImGUI的，核心代码放在：/examples/example_glfw_opengl2/
每一节的内容会放到一个main.cpp文件中。每个mainX.cpp都是一个基于上一个文件，增加新的特性。

main1.cpp 多线程和ppm图片输出

<img src = "https://img-blog.csdnimg.cn/20200224215545946.gif">

main2.cpp 射线、简单相机和背景输出

<img src = "https://img-blog.csdnimg.cn/20200225175003661.png">

main3.cpp 球体和表面法向量

<img src = "https://img-blog.csdnimg.cn/20200225205810947.png">

main4.cpp 封装成类

<img src = "https://img-blog.csdnimg.cn/20200226174541161.png?">

main5.cpp 抗锯齿

<img src = "https://img-blog.csdnimg.cn/20200226192557768.png">



main6.cpp 漫反射材质

<img src = "https://img-blog.csdnimg.cn/20200226204757396.png">

main7.cpp 反射向量和金属材质

<img src = "https://img-blog.csdnimg.cn/20200227105212838.png">

main8.cpp 折射向量和电介质

<img src = "https://img-blog.csdnimg.cn/20200227163304126.png">

main9.cpp 可放置相机

<img src = "https://img-blog.csdnimg.cn/2020022719201438.gif">

main10.cpp 散焦模糊

<img src = "https://img-blog.csdnimg.cn/20200301211843200.gif">

main11.cpp 随机场景的最终效果

<img src = "https://img-blog.csdnimg.cn/20200301221558173.png">



## MacOS运行环境

要想运行相应例子，只需修改makefile中第18行的文件名，然后编译，
例如：

```
SOURCES = main8.cpp
```
修改后，在命令行中进入makefile同级目录，执行`make`，然后运行同级目录下生成的`的example_glfw_opengl2`

## Windows运行环境
运行\example\imgui_examples.sln，然后将对应小节的mainX.cpp的代码粘贴到main.cpp里面。
（支持visual studio 2015，其他版本未测试）

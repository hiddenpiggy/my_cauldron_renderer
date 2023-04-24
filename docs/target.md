# 目标
开发出一个可测试的renderer, 用于加载gltf模型，分四个renderpass,能够渲染shadowmap, 进行tonemapping,以及后处理并渲染UI的一个渲染器

模块分解：
1. Vulkan所有的类都应该根据实际情况实现OnCreate, OnUpdate, OnDestroy方法，分别用于vulkan资源的raii管理，尽量使用Vulkan-Hpp的raii类。
2. 对于Swapchain, Renderpass这些对象， 应当使用一个单独的类进行抽象, 这里尽量不使用智能指针
3. 对于渲染循环，消息处理的部分，使用glfw的方法来进行处理





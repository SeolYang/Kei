#pragma once
#include <PCH.h>

namespace sy
{
class CommandLineParser;
}

namespace sy::vk
{
class VulkanContext;
class ResourceStateTracker;
} // namespace sy::vk

namespace sy::render
{
class Renderer;
}

namespace sy::game
{
} // namespace sy::game

namespace sy::window
{
class Window;
class WindowBuilder;
} // namespace sy::window

namespace sy::app
{
class Context final : public Subsystem
{
public:
    Context(CommandLineParser& cmdLineParser, const window::WindowBuilder& windowBuilder);
    ~Context() override;

    void Run();

    void Startup() override;
    void Shutdown() override;

private:
    void InitializeLogger();
    void InitDefaultEngineResources();

private:
    CommandLineParser&                 cmdLineParser;
    std::unique_ptr<window::Window>    window;
    std::unique_ptr<Timer>             timer;
    std::unique_ptr<HandleManager>     handleManager;
    std::unique_ptr<vk::VulkanContext> vulkanContext;
    std::unique_ptr<render::Renderer>  renderer;
};
} // namespace sy::app

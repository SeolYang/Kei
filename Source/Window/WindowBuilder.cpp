#include <PCH.h>
#include <Window/WindowBuilder.h>
#include <Window/Window.h>

namespace sy::window
{
std::unique_ptr<sy::window::Window> WindowBuilder::Build() const
{
    return std::make_unique<Window>(*this);
}
} // namespace sy::window
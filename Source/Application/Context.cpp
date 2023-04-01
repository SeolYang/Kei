#include <PCH.h>
#include "transcoder/basisu.h"
#include "transcoder/basisu_transcoder_internal.h"
#include "encoder/basisu_enc.h"
#include "encoder/basisu_etc.h"
#include "encoder/basisu_gpu_texture.h"
#include "encoder/basisu_frontend.h"
#include "encoder/basisu_backend.h"
#include "encoder/basisu_comp.h"
#include "transcoder/basisu_transcoder.h"
#include "encoder/basisu_ssim.h"
#include <Application/Context.h>
#include <Core/Constants.h>
#include <Core/CommandLineParser.h>
#include <Game/GameContext.h>
#include <Game/World.h>
#include <Render/Material.h>
#include <Render/Renderer.h>
#include <VK/DescriptorManager.h>
#include <VK/ResourceStateTracker.h>
#include <VK/Sampler.h>
#include <VK/SamplerBuilder.h>
#include <VK/Texture.h>
#include <VK/TextureBuilder.h>
#include <VK/TextureView.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>
#include <Window/Window.h>
#include <Window/WindowBuilder.h>

namespace sy::app
{
Context::Context(
    CommandLineParser&           cmdLineParser,
    const window::WindowBuilder& windowBuilder) :
    cmdLineParser(cmdLineParser),
    window(windowBuilder.Build()),
    timer(std::make_unique<Timer>()),
    handleManager(std::make_unique<HandleManager>()),
    vulkanContext(std::make_unique<vk::VulkanContext>(*window)),
    renderer(std::make_unique<render::Renderer>(
        *window,
        *vulkanContext,
        *handleManager))
{}

Context::~Context()
{}

void Context::Startup()
{
    InitializeLogger();
    if (cmdLineParser.ShouldConvertAssets())
    {
        //asset::ConvertAssets(cmdLineParser.GetAssetPath());
    }
    spdlog::info("Startup Context.");

    spdlog::info("Initializing SDL.");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        spdlog::critical("Failed to initialize SDL!");
    }

    timer->Startup();
    window->Startup();
    vulkanContext->Startup();
    handleManager->Startup();
    InitDefaultEngineResources();
    renderer->Startup();
}

void Context::Shutdown()
{
    spdlog::info("Shutdown Context.");
    vulkanContext->GetRHI().WaitForDeviceIdle();

    renderer->Shutdown();
    handleManager->Shutdown();
    vulkanContext->Shutdown();
    window->Shutdown();
    timer->Shutdown();
}

void Context::InitializeLogger()
{
    const auto        currentTime = std::chrono::system_clock::now();
    const auto        localTime   = std::chrono::current_zone()->to_local(currentTime);
    const std::string fileName =
        std::format("LOG_{:%F_%H_%M_%S}.log", localTime);

    fs::path logFilePath = "Logs";
    logFilePath /= fileName;
    const auto consoleSink =
        std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
    const auto logFilePathAnsi = WStringToAnsi(logFilePath.c_str());
    const auto fileSink        = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        logFilePathAnsi,
        true);

    const auto sinksInitList = {
        std::static_pointer_cast<spdlog::sinks::sink>(consoleSink),
        std::static_pointer_cast<spdlog::sinks::sink>(fileSink)};

    spdlog::set_default_logger(
        std::make_unique<spdlog::logger>("Core", sinksInitList));

#if defined(_DEBUG) || defined(DEBUG)
    spdlog::set_level(spdlog::level::trace);
#else
    spdlog::set_level(spdlog::level::warn);
#endif

    spdlog::info("Logger initialized: output : {}", logFilePathAnsi);
}

void Context::InitDefaultEngineResources()
{
    constexpr std::array white{0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};

    auto defaultTexBuilder =
        vk::TextureBuilder::Texture2DShaderResourceTemplate(*vulkanContext)
            .SetName(core::constants::res::DefaultWhiteTexture)
            .SetExtent(Extent2D<uint32_t>{2, 2})
            .SetFormat(VK_FORMAT_R8G8B8A8_SRGB);

    auto defaultWhiteTex = handleManager->Add(
        defaultTexBuilder
            .SetDataToTransfer(std::span{white.data(), white.size()})
            .Build());
    defaultWhiteTex.SetAlias(core::constants::res::DefaultWhiteTexture);

    constexpr std::array black = {0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff};

    auto defaultBlackTex = handleManager->Add(
        defaultTexBuilder.SetName(core::constants::res::DefaultBlackTexture)
            .SetDataToTransfer(std::span{white.data(), white.size()})
            .Build());

    defaultBlackTex.SetAlias(core::constants::res::DefaultBlackTexture);

    auto linearSampler = handleManager->Add(vk::SamplerBuilder{*vulkanContext}
                                                .SetName(core::constants::res::TrilinearRepeatSampler)
                                                .Build());

    linearSampler.SetAlias(core::constants::res::TrilinearRepeatSampler);

    const auto defaultWhiteTexView = handleManager->Add<vk::TextureView>(
        std::format("{}_View", core::constants::res::DefaultWhiteTexture),
        *vulkanContext,
        *defaultWhiteTex,
        VK_IMAGE_VIEW_TYPE_2D);
    defaultWhiteTex.SetAlias(core::constants::res::DefaultWhiteTexture);

    auto& descriptorManager = vulkanContext->GetDescriptorManager();

    auto defaultWhiteDescriptor =
        handleManager->Add<vk::Descriptor>(descriptorManager.RequestDescriptor(
            *handleManager,
            defaultWhiteTex,
            defaultWhiteTexView,
            linearSampler,
            vk::ETextureState::AnyShaderReadSampledImage));
    defaultWhiteDescriptor.SetAlias(core::constants::res::DefaultWhiteTexture);

    auto defaultMaterial =
        handleManager->Add<render::Material>(defaultWhiteDescriptor);
    defaultMaterial.SetAlias(core::constants::res::DefaultMaterialInstance);
}

void Context::Run()
{
    spdlog::info("Startup main loop.");
    SDL_Event ev;
    bool      bExit = false;

    while (!bExit)
    {
        timer->Begin();
        vulkanContext->BeginFrame();

        while (SDL_PollEvent(&ev) != 0)
        {
            if (ev.type == SDL_QUIT)
            {
                bExit = true;
            }
        }

        vulkanContext->BeginRender();
        renderer->Render();
        vulkanContext->EndRender();

        vulkanContext->EndFrame();
        timer->End();
    }
    spdlog::info("Main loop finished.");
}
} // namespace sy::app

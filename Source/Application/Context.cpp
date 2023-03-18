#include <PCH.h>
#include <Application/Context.h>
#include <Core/CommandLineParser.h>
#include <Window/Window.h>
#include <Core/Utils.h>
#include <VK/VulkanContext.h>
#include <VK/ResourceStateTracker.h>
#include <VK/Texture.h>
#include <VK/TextureView.h>
#include <VK/Sampler.h>
#include <VK/SamplerBuilder.h>
#include <VK/DescriptorManager.h>
#include <VK/VulkanRHI.h>
#include <Render/Renderer.h>
#include <Render/Material.h>
#include <Asset/AssetConverter.h>
#include <Game/World.h>
#include <Game/GameContext.h>

#include "VK/TextureBuilder.h"

namespace sy::app
{
	Context::Context(const int argc, char** argv)
	{
		Startup(argc, argv);
	}

	Context::~Context()
	{
	}

	void Context::Startup(const int argc, char** argv)
	{
		InitializeLogger();
		InitializeCommandLineParser(argc, argv);

		spdlog::info("Initializing SDL.");
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
		{
			spdlog::critical("Failed to initialize SDL!");
		}

		spdlog::info("Initializing Timer sub-context.");
		timer = std::make_unique<Timer>();
		spdlog::info("Initializing Window sub-context.");
		window = std::make_unique<window::Window>("Test", Extent2D<uint32_t>{ 1280, 720 });
		spdlog::info("Initializing Handle Manager.");
		handleManager = std::make_unique<HandleManager>();
		spdlog::info("Initializing Vulkan context.");
		vulkanContext = std::make_unique<vk::VulkanContext>(*window);
		spdlog::info("Initializing Resource State Tracker.");
		resourceStateTracker = std::make_unique<vk::ResourceStateTracker>(*handleManager);
		spdlog::info("Initializing Default engine resources.");
		InitDefaultEngineResources();
		spdlog::info("Initializing Renderer sub-context.");
		renderer = std::make_unique<render::Renderer>(*window, *vulkanContext, *resourceStateTracker, *handleManager);
	}

	void Context::InitializeLogger()
	{
		const auto currentTime = std::chrono::system_clock::now();
		const auto localTime = std::chrono::current_zone()->to_local(currentTime);
		const std::string fileName = std::format("LOG_{:%F_%H_%M_%S}.log", localTime);

		fs::path logFilePath = "Logs";
		logFilePath /= fileName;
		const auto consoleSink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
		const auto logFilePathAnsi = WStringToAnsi(logFilePath.c_str());
		const auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePathAnsi, true);

		const auto sinksInitList = {
			std::static_pointer_cast<spdlog::sinks::sink>(consoleSink),
			std::static_pointer_cast<spdlog::sinks::sink>(fileSink)
		};

		spdlog::set_default_logger(std::make_unique<spdlog::logger>("Core", sinksInitList));

#if defined(_DEBUG) || defined(DEBUG)
		spdlog::set_level(spdlog::level::trace);
#else
		spdlog::set_level(spdlog::level::warn);
#endif

		spdlog::info("Logger initialized: output : {}", logFilePathAnsi);
	}

	void Context::InitializeCommandLineParser(const int argc, char** argv)
	{
		spdlog::info("Initializing Command Line Parser sub-context.");
		cmdLineParser = std::make_unique<CommandLineParser>(argc, argv);
		if (cmdLineParser->ShouldConvertAssets())
		{
			asset::ConvertAssets(cmdLineParser->GetAssetPath());
		}
	}

	void Context::InitDefaultEngineResources()
	{
		const std::array white{ 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
		auto defaultTexBuilder = vk::TextureBuilder::Texture2DShaderResourceTemplate(*vulkanContext)
									 .SetName("DefaultWhite")
									 .SetExtent(Extent2D<uint32_t>{ 2, 2 })
									 .SetFormat(VK_FORMAT_R8G8B8A8_SRGB);

		auto defaultWhiteTex = handleManager->Add(defaultTexBuilder.SetDataToTransfer(std::span{
																						  white.data(),
																						  white.size() })
													  .Build());
		defaultWhiteTex.SetAlias(vk::DefaultWhiteTexture);

		constexpr std::array<uint32_t, 4> black = { 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff };
		auto defaultBlackTex = handleManager->Add(defaultTexBuilder.SetName("DefaultBlack").SetDataToTransfer(std::span{ white.data(), white.size() }).Build());
		defaultBlackTex.SetAlias(vk::DefaultBlackTexture);

		auto linearSampler = handleManager->Add<
			vk::Sampler>(vk::SamplerBuilder{ *vulkanContext }.SetName(vk::LinearSamplerRepeat).Build());
		linearSampler.SetAlias(vk::LinearSamplerRepeat);

		const auto defaultWhiteTexView = handleManager->Add<
			vk::TextureView>(std::format("{}_View", vk::DefaultWhiteTexture), vulkanContext->GetRHI(),
			*defaultWhiteTex, VK_IMAGE_VIEW_TYPE_2D);

		auto& descriptorManager = vulkanContext->GetDescriptorManager();
		auto defaultMaterial = handleManager->Add<render::Material>(handleManager->Add<
																	vk::Descriptor>(descriptorManager.RequestDescriptor(*handleManager,
			defaultWhiteTex,
			defaultWhiteTexView,
			linearSampler,
			vk::ETextureState::AnyShaderReadSampledImage)));
		defaultMaterial.SetAlias(render::DefaultMaterial);
	}

	void Context::Cleanup()
	{
		vulkanContext->GetRHI().WaitForDeviceIdle();

		spdlog::info("Clean-up Resource State Tracker.");
		resourceStateTracker.reset();

		spdlog::info("Clean-up Renderer sub-context.");
		renderer.reset();

		spdlog::info("Clean-up Handle Manager.");
		handleManager.reset();

		spdlog::info("Clean-up Vulkan context.");
		vulkanContext.reset();

		spdlog::info("Clean-up Window sub-context");
		window.reset();

		spdlog::info("Clean-up Command Line Parser sub-context");
		cmdLineParser.reset();
	}

	void Context::Run()
	{
		spdlog::info("Startup main loop.");
		SDL_Event ev;
		bool bExit = false;

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

		Cleanup();
	}
} // namespace sy::app

#include <PCH.h>
#include <VK/PipelineBuilder.h>
#include <VK/ShaderModule.h>
#include <VK/VertexInputBuilder.h>

namespace sy
{
	namespace vk
	{
		/** Graphics Pipeline Builder */
		GraphicsPipelineBuilder::GraphicsPipelineBuilder()
		{
			SetDefault();
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDefault()
		{
			ZeroMemory(&pipelineCreateFlags, sizeof(pipelineCreateFlags));
			ZeroMemory(&inputAssembly, sizeof(inputAssembly));
			ZeroMemory(&tessellation, sizeof(tessellation));
			ZeroMemory(&rasterizer, sizeof(rasterizer));
			ZeroMemory(&colorBlend, sizeof(colorBlend));
			ZeroMemory(&multiSampling, sizeof(multiSampling));
			ZeroMemory(&depthStencil, sizeof(depthStencil));

			SetPipelineCreateFlags(0);
			vertexInput = std::nullopt;
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.pNext = nullptr;
			tessellation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			tessellation.pNext = nullptr;
			viewports.clear();
			scissors.clear();
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.pNext = nullptr;
			viewportState.flags = 0;
			viewportState.viewportCount = 0;
			viewportState.pViewports = nullptr;
			viewportState.scissorCount = 0;
			viewportState.pScissors = nullptr;
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.pNext = nullptr;
			colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlend.pNext = nullptr;
			colorBlendAttachments.clear();
			colorBlend.attachmentCount = 0;
			colorBlend.pAttachments = nullptr;
			multiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multiSampling.pNext = nullptr;
			pipelineLayout = VK_NULL_HANDLE;
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.pNext = nullptr;

			SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
			SetPrimitiveRestart(false);

			SetDepthClampEnable(false);
			SetRasterizerDiscardEnable(false);
			SetPolygonMode(VK_POLYGON_MODE_FILL);
			SetCullMode(VK_CULL_MODE_BACK_BIT);
			SetFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
			SetDepthBiasEnable(false);
			SetDepthBiasConstantFactor(0.f);
			SetDepthBiasClamp(0.f);
			SetDepthBiasSlopeFactor(0.f);
			SetLineWidth(1.f);

			SetSampleShadingEnable(false);
			SetRasterizerSamples(VK_SAMPLE_COUNT_1_BIT);
			SetMinSampleShading(1.f);
			SetSampleMask(nullptr);
			SetAlphaToCoverageEnable(false);
			SetAlphaToOneEnable(false);

			SetDepthTestEnable(true);
			SetDepthWriteEnable(true);
			SetDepthCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL);
			SetDepthBoundsTestEnable(false);
			SetStencilTestEnable(false);
			SetMinDepthBounds(0.f);
			SetMaxDepthBounds(0.f);

			renderingCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
				.pNext = nullptr
			};
			SetDepthStencilFormat();

			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetPipelineCreateFlags(VkPipelineCreateFlags flags)
		{
			pipelineCreateFlags = flags;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddShaderStage(const ShaderModule& shaderModule)
		{
			shaderStages.emplace_back(
				VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				nullptr, 0,
				shaderModule.GetShaderType(), shaderModule.GetNativeHandle(),
				shaderModule.GetEntryPoint().data(),
				nullptr);

			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetVertexInputLayout(const VertexInputBuilder& builder)
		{
			vertexInput = builder.Build();
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetPrimitiveTopology(VkPrimitiveTopology topology)
		{
			inputAssembly.topology = topology;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetPrimitiveRestart(bool bPrimitiveRestartEnable)
		{
			inputAssembly.primitiveRestartEnable = (bPrimitiveRestartEnable ? VK_TRUE : VK_FALSE);
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetPatchControlPoints(uint32_t patchControlPoints)
		{
			tessellation.patchControlPoints = patchControlPoints;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddViewport(VkViewport viewport)
		{
			viewports.emplace_back(viewport);
			++viewportState.viewportCount;
			viewportState.pViewports = viewports.data();
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddViewport(float x, float y, float width, float height,
			float minDepth, float maxDepth)
		{
			viewports.emplace_back(x, y, width, height, minDepth, maxDepth);
			++viewportState.viewportCount;
			viewportState.pViewports = viewports.data();
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddScissor(VkRect2D scissor)
		{
			scissors.emplace_back(scissor);
			++viewportState.scissorCount;
			viewportState.pScissors = scissors.data();
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
		{
			scissors.emplace_back(VkOffset2D{ x, y }, VkExtent2D{ width, height });
			++viewportState.scissorCount;
			viewportState.pScissors = scissors.data();
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthClampEnable(bool bDepthClampEnable)
		{
			rasterizer.depthClampEnable = bDepthClampEnable ? VK_TRUE : VK_FALSE;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetRasterizerDiscardEnable(bool bRasterizerDiscardEnable)
		{
			rasterizer.rasterizerDiscardEnable = bRasterizerDiscardEnable ? VK_TRUE : VK_FALSE;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetPolygonMode(VkPolygonMode polygonMode)
		{
			rasterizer.polygonMode = polygonMode;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetCullMode(VkCullModeFlags cullMode)
		{
			rasterizer.cullMode = cullMode;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetFrontFace(VkFrontFace frontFace)
		{
			rasterizer.frontFace = frontFace;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthBiasEnable(bool bDepthBiasEnable)
		{
			rasterizer.depthBiasEnable = bDepthBiasEnable ? VK_TRUE : VK_FALSE;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthBiasConstantFactor(float depthBiasConstantFactor)
		{
			rasterizer.depthBiasConstantFactor = depthBiasConstantFactor;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthBiasClamp(float depthBiasClamp)
		{
			rasterizer.depthBiasClamp = depthBiasClamp;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthBiasSlopeFactor(float depthBiasSlopeFactor)
		{
			rasterizer.depthBiasSlopeFactor = depthBiasSlopeFactor;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetLineWidth(float lineWidth)
		{
			rasterizer.lineWidth = lineWidth;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetRasterizerSamples(VkSampleCountFlagBits rasterizerSamples)
		{
			multiSampling.rasterizationSamples = rasterizerSamples;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetSampleShadingEnable(bool bSampleShadingEnable)
		{
			multiSampling.sampleShadingEnable = bSampleShadingEnable ? VK_TRUE : VK_FALSE;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetMinSampleShading(float minSampleShading)
		{
			multiSampling.minSampleShading = minSampleShading;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetSampleMask(uint32_t* sampleMask)
		{
			multiSampling.pSampleMask = sampleMask;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetAlphaToCoverageEnable(bool bAlphaToCoverageEnable)
		{
			multiSampling.alphaToCoverageEnable = bAlphaToCoverageEnable ? VK_TRUE : VK_FALSE;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetAlphaToOneEnable(bool bAlphaToOne)
		{
			multiSampling.alphaToOneEnable = bAlphaToOne ? VK_TRUE : VK_FALSE;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthTestEnable(bool bDepthTestEnable)
		{
			depthStencil.depthTestEnable = bDepthTestEnable ? VK_TRUE : VK_FALSE;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthWriteEnable(bool bDepthWriteEnable)
		{
			depthStencil.depthWriteEnable = bDepthWriteEnable ? VK_TRUE : VK_FALSE;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthCompareOp(VkCompareOp depthCompareOp)
		{
			depthStencil.depthCompareOp = depthCompareOp;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthBoundsTestEnable(bool bDepthBoundsTestEnable)
		{
			depthStencil.depthBoundsTestEnable = bDepthBoundsTestEnable ? VK_TRUE : VK_FALSE;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetStencilTestEnable(bool bStencilTestEnable)
		{
			depthStencil.stencilTestEnable = bStencilTestEnable ? VK_TRUE : VK_FALSE;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetStencilOpStateFront(VkStencilOp failOp, VkStencilOp passOp,
			VkStencilOp depthFailOp, VkCompareOp compareOp, uint32_t compareMask, uint32_t writeMask, uint32_t reference)
		{
			depthStencil.front.failOp = failOp;
			depthStencil.front.passOp = passOp;
			depthStencil.front.depthFailOp = depthFailOp;
			depthStencil.front.compareOp = compareOp;
			depthStencil.front.compareMask = compareMask;
			depthStencil.front.writeMask = writeMask;
			depthStencil.front.reference = reference;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetStencilOpStateBack(
			VkStencilOp failOp, VkStencilOp passOp,
			VkStencilOp depthFailOp, VkCompareOp compareOp,
			uint32_t compareMask, uint32_t writeMask, uint32_t reference)
		{
			depthStencil.back.failOp = failOp;
			depthStencil.back.passOp = passOp;
			depthStencil.back.depthFailOp = depthFailOp;
			depthStencil.back.compareOp = compareOp;
			depthStencil.back.compareMask = compareMask;
			depthStencil.back.writeMask = writeMask;
			depthStencil.back.reference = reference;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetMinDepthBounds(float minDepthBounds)
		{
			depthStencil.minDepthBounds = minDepthBounds;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetMaxDepthBounds(float maxDepthBounds)
		{
			depthStencil.maxDepthBounds = maxDepthBounds;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddColorBlendAttachmentState(bool bBlendEnable,
			VkBlendFactor srcColorBlendFactor, VkBlendFactor dstColorBlendFactor, VkBlendOp colorBlendOp,
			VkBlendFactor srcAlphaBlendFactor, VkBlendFactor dstAlphaBlendFactor, VkBlendOp alphaBlendOp,
			VkColorComponentFlags colorWriteMask)
		{
			colorBlendAttachments.emplace_back(bBlendEnable ? VK_TRUE : VK_FALSE, srcColorBlendFactor, dstColorBlendFactor, colorBlendOp, srcAlphaBlendFactor, dstAlphaBlendFactor, alphaBlendOp, colorWriteMask);
			colorBlend.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
			colorBlend.pAttachments = colorBlendAttachments.data();
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::DefaultColorBlendAttachment()
		{
			VkPipelineColorBlendAttachmentState colorBlendAttachment;
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
				VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachments.emplace_back(colorBlendAttachment);
			colorBlend.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
			colorBlend.pAttachments = colorBlendAttachments.data();
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetColorBlendLogicOpEnable(bool bLogicOpEnable)
		{
			colorBlend.logicOpEnable = bLogicOpEnable ? VK_TRUE : VK_FALSE;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetColorBlendLogicOp(VkLogicOp logicOp)
		{
			colorBlend.logicOp = logicOp;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetColorBlendConstants(std::array<float, 4> blendConstants)
		{
			colorBlend.blendConstants[0] = blendConstants[0];
			colorBlend.blendConstants[1] = blendConstants[1];
			colorBlend.blendConstants[2] = blendConstants[2];
			colorBlend.blendConstants[3] = blendConstants[3];
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetPipelineLayout(VkPipelineLayout pipelineLayout)
		{
			this->pipelineLayout = pipelineLayout;
			return *this;
		}

		GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthStencilFormat(VkFormat format)
		{
			this->renderingCreateInfo.depthAttachmentFormat = format;
			this->renderingCreateInfo.stencilAttachmentFormat = format;
			return *this;
		}

		VkGraphicsPipelineCreateInfo GraphicsPipelineBuilder::Build() const
		{
			/** @todo Optional style builder refactoring */
			const VkGraphicsPipelineCreateInfo createInfo
			{
				.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.pNext = &renderingCreateInfo,
				.flags = pipelineCreateFlags,
				.stageCount = static_cast<uint32_t>(shaderStages.size()),
				.pStages = shaderStages.data(),
				.pVertexInputState = vertexInput.has_value() ? &vertexInput.value() : nullptr,
				.pInputAssemblyState = &inputAssembly,
				.pTessellationState = &tessellation,
				.pViewportState = &viewportState,
				.pRasterizationState = &rasterizer,
				.pMultisampleState = &multiSampling,
				.pDepthStencilState = &depthStencil,
				.pColorBlendState = &colorBlend,
				// @TODO Implement Dynamic State of Pipeline
				// .pDynamicState =
				.layout = pipelineLayout
			};

			return createInfo;
		}

		ComputePipelineBuilder::ComputePipelineBuilder()
		{
			SetDefault();
		}

		ComputePipelineBuilder& ComputePipelineBuilder::SetDefault()
		{
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.flags = 0;
			shaderStage.module = VK_NULL_HANDLE;
			shaderStage.pSpecializationInfo = nullptr;
			shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;

			SetCreateFlags(0);
			SetPipelineLayout(VK_NULL_HANDLE);

			return *this;
		}

		ComputePipelineBuilder& ComputePipelineBuilder::SetCreateFlags(VkPipelineCreateFlags flags)
		{
			createFlags = flags;
			return *this;
		}

		ComputePipelineBuilder& ComputePipelineBuilder::SetShader(const ShaderModule& shaderModule)
		{
			SY_ASSERT((shaderModule.GetShaderType() & VK_SHADER_STAGE_COMPUTE_BIT) != 0, "Shader is not a type of compute shader.");
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.pNext = nullptr;
			shaderStage.module = shaderModule.GetNativeHandle();
			shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
			return *this;
		}

		ComputePipelineBuilder& ComputePipelineBuilder::SetPipelineLayout(VkPipelineLayout layout)
		{
			this->layout = layout;
			return *this;
		}

		VkComputePipelineCreateInfo ComputePipelineBuilder::Build() const
		{
			const VkComputePipelineCreateInfo createInfo
			{
				.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
				.pNext = nullptr,
				.flags = createFlags,
				.stage = shaderStage,
				.layout = layout
			};

			return createInfo;
		}
	}
}
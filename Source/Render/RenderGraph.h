//#pragma once
//#include <Core/Core.h>
//
///**
// * Internal Resources - Transient
// * External Resources - Persistent
// * Assume only write once to resource in graph.
// */
//
//namespace sy::render
//{
//
//	struct AttachmentInfo
//	{
//	};
//
//	struct BufferInfo
//	{
//	};
//
//	class ResourceRegistry
//	{
//	public:
//		/** Transient */
//		void AddRenderTarget(std::string_view name, AttachmentInfo info);
//		void AddDepthStencil(std::string_view name, AttachmentInfo info);
//		void AddStorageBuffer(std::string_view name, BufferInfo info);
//
//		/** Persistent */
//		/* ... */
//
//		bool HasResource(const std::string_view name) const
//		{
//			const auto key = std::hash<std::string_view>()(name);
//			return HasResource(key);
//		}
//
//		bool HasResource(const uint64_t key) const
//		{
//			return attachments.find(key) != attachments.end() || buffers.find(key) != buffers.end();
//		}
//
//	private:
//		robin_hood::unordered_map<uint64_t, AttachmentInfo> attachments;
//		robin_hood::unordered_map<uint64_t, BufferInfo> buffers;
//
//	};
//
//	class RenderPass : public NamedType
//	{
//	public:
//		RenderPass(const std::string_view name) : NamedType(name)
//		{
//		}
//
//		void UseAsyncCompute() noexcept { bIsAsyncCompute = true; }
//		[[nodiscard]] bool IsAsyncCompute() const noexcept { return bIsAsyncCompute; }
//
//		void WriteResource(const std::string_view name)
//		{
//			resourceWrites.emplace_back(std::hash<std::string_view>()(name));
//		}
//
//		void ReadResource(const std::string_view name)
//		{
//			resourceReads.emplace_back(std::hash<std::string_view>()(name));
//		}
//
//	private:
//		bool bIsAsyncCompute = false;
//		std::vector<uint64_t> resourceWrites;
//		std::vector<uint64_t> resourceReads;
//
//	};
//
//	class RenderGraph
//	{
//	public:
//		//void AddRootRenderPass(std::unique_ptr<RenderPass> root)
//		//{
//		//	// assert render pass == 0
//		//	renderPasses.emplace_back(std::move(root));
//		//}
//
//		void AddRenderPass(std::unique_ptr<RenderPass> renderPass)
//		{
//			// assert render passes > 0
//			renderPasses.emplace_back(std::move(renderPass));
//		}
//
//		void OrderingPasses()
//		{
//			// Adj List
//			std::vector<std::vector<size_t>> adjList;
//			adjList.resize(renderPasses.size());
//			for (size_t idx = 0; idx < renderPasses.size(); ++idx)
//			{
//				for (size_t dependencyIdx = 0; dependencyIdx < renderPasses.size(); ++dependencyIdx)
//				{
//					if (dependencyIdx != idx)
//					{
//						for (const auto& writes : renderPasses[idx]->resourceWrites)
//						{
//							if (std::find(renderPasses[dependencyIdx]->resourceReads.begin(), renderPasses[dependencyIdx]->resourceReads.end(), writes) != renderPasses[dependencyIdx]->resourceReads.end())
//							{
//								adjList[idx].emplace_back(dependencyIdx);
//								break;
//							}
//						}
//					}
//				}
//			}
//
//			std::vector<bool> visited;
//			std::vector<bool> onStack;
//			std::vector<size_t> ordered;
//			visited.resize(renderPasses.size());
//			onStack.resize(renderPasses.size());
//			DFS(0, adjList, visited, onStack, ordered);
//			std::reverse(ordered.begin(), ordered.end());
//
//			std::vector<size_t> dist;
//			dist.resize(renderPasses.size());
//			for (const size_t renderPass : ordered)
//			{
//				for (const size_t adjRenderPass : adjList[renderPass])
//				{
//					if (dist[adjRenderPass] < dist[renderPass] + 1)
//					{
//						++dist[adjRenderPass];
//					}
//				}
//			}
//		}
//
//		void DFS(size_t root, std::vector<std::vector<size_t>>& adjList, std::vector<bool>& visited, std::vector<bool>& onStack, std::vector<size_t>& ordered)
//		{
//			if (visited[root] && onStack[root])
//			{
//				assert(false); // circular dependency
//			}
//
//			else if (!visited[root])
//			{
//				visited[root] = true;
//				onStack[root] = true;
//				for (const size_t dependency : adjList[root])
//				{
//					DFS(dependency, adjList, visited, onStack, ordered);
//
//				}
//				ordered.emplace_back( root );
//				onStack[root] = false;
//			}
//		}
//
//	private:
//		std::vector<std::unique_ptr<RenderPass>> renderPasses;
//		std::vector<size_t> orderedRenderPass;
//
//		robin_hood::unordered_map<std::string, size_t> resourceWriteAt;
//
//	};
//}
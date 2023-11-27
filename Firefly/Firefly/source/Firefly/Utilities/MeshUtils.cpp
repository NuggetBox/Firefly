#include "FFpch.h"
#include "MeshUtils.h"
#define TRACE
#include "meshoptimizer.h"
namespace Firefly
{
	std::vector<uint32_t> LODSubmeshPerserveTopology(std::vector<Vertex>& aVertices, std::vector<uint32_t>& aIndices, float aFactor)
	{
		std::vector<uint32_t> newIndices(aIndices.size());

		const uint32_t indicesCount = aIndices.size();
		const uint32_t targetIndicesCount = aIndices.size() * aFactor;

		const float* vertexPostion = reinterpret_cast<float*>(&aVertices.data()->Position[0]);
		float target_error = 1e-1f;
		float* error = nullptr;

		const size_t newIndicesCount = meshopt_simplify(
			newIndices.data(),
			aIndices.data(),
			indicesCount,
			vertexPostion,
			aVertices.size(),
			sizeof(Vertex),
			targetIndicesCount,
			target_error,
			0, error);


		newIndices.resize(newIndicesCount);

		if (newIndicesCount == 0)
		{
			newIndices = LODSubmeshPerserveTopology(aVertices, aIndices, aFactor += 0.05);
		}

		return newIndices;
	}
	std::vector<uint32_t> LODSubmeshIgnoreTopology(std::vector<Vertex>& aVertices, std::vector<uint32_t>& aIndices, float aFactor)
	{
		std::vector<uint32_t> newIndices(aIndices.size());

		const uint32_t indicesCount = aIndices.size();
		const uint32_t targetIndicesCount = aIndices.size() * aFactor;

		const float* vertexPostion = reinterpret_cast<float*>(&aVertices.data()->Position[0]);
		float target_error = 1e-1f;
		float* error = nullptr;

		const size_t newIndicesCount = meshopt_simplifySloppy(
			newIndices.data(),
			aIndices.data(),
			indicesCount,
			vertexPostion,
			aVertices.size(),
			sizeof(Vertex),
			targetIndicesCount,
			target_error,
			error);

		newIndices.resize(newIndicesCount);

		if (newIndicesCount == 0)
		{
			newIndices = LODSubmeshIgnoreTopology(aVertices, aIndices, aFactor + 0.05);
		}

		return newIndices;
	}
	bool OptimizeVertexFetching(std::vector<Vertex>& aVertices, std::vector<uint32_t>& aIndices)
	{
		std::vector<Vertex> newVertices(aVertices.size());

		newVertices.resize(meshopt_optimizeVertexFetch(newVertices.data(), aIndices.data(), aIndices.size(), aVertices.data(), aVertices.size(), sizeof(Vertex)));

		aVertices = newVertices;

		return true;
	}
	//bool OptimizeVertexFetchRemap(std::vector<Vertex>& aVertices, std::vector<uint32_t>& aIndices)
	//{
	//	std::vector<Vertex> newVertices(aVertices.size());

	//	newVertices.resize(meshopt_optimizeVertexFetchRemap(newVertices.data(), aIndices.data(), aIndices.size(), sizeof(Vertex)));

	//	aVertices = newVertices;

	//	return true;
	//}
}
#pragma once
#include <vector>
#include "Firefly/Rendering/Mesh/Vertices.hpp"

namespace Firefly
{
	std::vector<uint32_t> LODSubmeshPerserveTopology(std::vector<Vertex>& aVertices, std::vector<uint32_t>& aIndices, float aFactor);
	std::vector<uint32_t> LODSubmeshIgnoreTopology(std::vector<Vertex>& aVertices, std::vector<uint32_t>& aIndices, float aFactor);

	// will optimize the vertex layout and index buffer to make vertexvalues fetch alot faster.
	bool OptimizeVertexFetching(std::vector<Vertex>& aVertices, std::vector<uint32_t>& aIndices);
	//bool OptimizeVertexFetchRemap(std::vector<Vertex>& aVertices, std::vector<uint32_t>& aIndices);
}
#include "FFpch.h"
#include "SubMesh.h"

#include <Firefly/Rendering/RenderCommands.h>
#include "Firefly/Rendering/GraphicsContext.h"
#include <meshoptimizer.h>

#include "Firefly/Utilities/MeshUtils.h"

namespace Firefly
{
	SubMesh::SubMesh(std::vector<Vertex>& aVertices, std::vector<uint32_t>& aIndices)
	{
		CreateVertexIndex(aVertices, aIndices);

		std::hash<float> hasher;
		std::hash<size_t> sizehasher;
		myHash = 0;
		myHash ^= hasher(myVertexMidPoint.x) + 0x9e3779b9 + (myHash << 6) + (myHash >> 2);
		myHash ^= hasher(myVertexMidPoint.y) + 0x9e3779b9 + (myHash << 6) + (myHash >> 2);
		myHash ^= hasher(myVertexMidPoint.z) + 0x9e3779b9 + (myHash << 6) + (myHash >> 2);
		myHash ^= sizehasher(aVertices.size()) + 0x9e3779b9 + (myHash << 6) + (myHash >> 2);
		myHash ^= sizehasher(aIndices.size()) + 0x9e3779b9 + (myHash << 6) + (myHash >> 2);
		
	}

	SubMesh::SubMesh(std::vector<Vertex>& aVertices, std::vector<uint32_t>& aIndices, std::string aPath, uint32_t submeshID)
	{
		CreateVertexIndex(aVertices, aIndices);

		std::hash<std::string> hasher;
		std::hash<size_t> sizehasher;
		myHash = 0;
		myHash ^= sizehasher(static_cast<size_t>(submeshID)) + 0x9e3779b9 + (myHash << 6) + (myHash >> 2);
		myHash ^= hasher(aPath) + 0x9e3779b9 + (myHash << 6) + (myHash >> 2);
	}

	void SubMesh::CreateVertexIndex(std::vector<Vertex>& aVertices, std::vector<uint32_t>& aIndices)
	{
		myVertexLowPoint = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
		myVertexHighPoint = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };
		for (int i = 0; i < aVertices.size(); i++)
		{
			myVerticesPositions.emplace_back(aVertices[i].Position[0], aVertices[i].Position[1], aVertices[i].Position[2]);
			if (myVerticesPositions[i].y < myVertexLowPoint.y)
			{
				myVertexLowPoint = myVerticesPositions[i];
			}
			if (myVerticesPositions[i].y > myVertexHighPoint.y)
			{
				myVertexHighPoint = myVerticesPositions[i];
			}
			myVertexMidPoint += myVerticesPositions[i];
		}


		myVertexMidPoint /= static_cast<float>(aVertices.size());


		myVertices.resize(aVertices.size());
		memcpy(myVertices.data(), aVertices.data(), aVertices.size() * sizeof(Vertex));
		myIndices = aIndices;

		//OptimizeVertexFetching(myVertices, myIndices);

		auto lod1 = LODSubmeshPerserveTopology(myVertices, myIndices, 0.75f);
		auto lod2 = LODSubmeshPerserveTopology(myVertices, myIndices, 0.1f);
		auto lod3 = LODSubmeshPerserveTopology(myVertices, myIndices, 0.01f);
		

		VertexBufferInfo info{ };
		info.Data = myVertices.data();
		info.ObjectSize = sizeof(Vertex);
		info.Count = myVertices.size();
		myVertexBuffer = VertexBuffer::Create(info);

		myIndexBuffers[0] = IndexBuffer::Create(myIndices.data(), static_cast<uint32_t>(myIndices.size()));
		myIndexBuffers[1] = IndexBuffer::Create(lod1.data(), static_cast<uint32_t>(lod1.size()));
		myIndexBuffers[2] = IndexBuffer::Create(lod2.data(), static_cast<uint32_t>(lod2.size()));
		myIndexBuffers[3] = IndexBuffer::Create(lod3.data(), static_cast<uint32_t>(lod3.size()));
	}
}
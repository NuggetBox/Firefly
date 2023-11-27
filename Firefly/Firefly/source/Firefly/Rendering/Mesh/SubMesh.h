#pragma once
#include <vector>
#include <array>
#include "Vertices.hpp"
#include "Firefly/Rendering/Buffer/IndexBuffer.h"
#include "Firefly/Rendering/Buffer/VertexBuffer.h"
#include "Utils/Math/Sphere.hpp"

namespace Firefly
{
	class SubMesh
	{
	public:
		SubMesh() = default;
		SubMesh(std::vector<Vertex>& aVertices, std::vector<uint32_t>& aIndices);
		SubMesh(std::vector<Vertex>& aVertices, std::vector<uint32_t>& aIndices, std::string aPath, uint32_t submeshID);

		inline const Utils::Vector3f& GetHighPoint() { return myVertexHighPoint; }
		inline const Utils::Vector3f& GetMidPoint() { return myVertexMidPoint; }
		inline const Utils::Vector3f& GetLowPoint() { return myVertexLowPoint; }
		inline const std::vector<Utils::Vector3f>& GetVerticesPositions() const { return myVerticesPositions; }
		inline const std::vector<Vertex>& GetVertices() const { return myVertices; }
		inline const std::vector<uint32_t>& GetIndices() const { return myIndices; }

		inline size_t GetLODOffset(uint32_t aIndex) { return myLodIndicesOffsets[aIndex]; }

		inline Utils::Sphere<float>& GetBound() { return myBoundingSphere; };

		inline Ref<VertexBuffer> GetVertexBuffer() { return myVertexBuffer; }
		inline Ref<IndexBuffer> GetIndexBuffer(uint32_t aIndex = 0) { return myIndexBuffers[aIndex]; }

		bool operator==(SubMesh& other)
		{
			return (myHash == other.myHash);
		}

		[[nodiscard]] FORCEINLINE size_t GetHash() const { return myHash; }

	private:
		void CreateVertexIndex(std::vector<Vertex>& aVertices, std::vector<uint32_t>& aIndices);

		std::vector<Utils::Vector3f> myVerticesPositions;
		Utils::Vector3f myVertexMidPoint;
		Utils::Vector3f myVertexLowPoint;
		Utils::Vector3f myVertexHighPoint;
		std::vector<Vertex> myVertices;
		std::vector<uint32_t> myIndices;
		Ref<VertexBuffer> myVertexBuffer;
		size_t myHash = 0;

		std::array<size_t, 4> myLodIndicesOffsets;
		std::array<Ref<IndexBuffer>, 4> myIndexBuffers;

		Utils::Sphere<float> myBoundingSphere;
	};
}
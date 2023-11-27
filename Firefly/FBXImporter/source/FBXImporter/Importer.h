#pragma once
#include <string>

#include "TgaFbxStructs.h"

namespace TGA
{
	namespace FBX
	{
		class Importer
		{
		public:

			/**
			* Initializes the Importer (or re-initializes if it has been uninitialized) to make it ready for use.
			*/
			static void InitImporter();

			/**
			 * Tell the Importer to clean up. Intended to be used to save memory when the Importer is no longer needed.
			 */
			static void UninitImporter();

			/**
			 * Attempts to load a FBX model into the provided Model structure. Supports skeletal meshes, multiple meshes
			 * and multiple materials. Will treat multiple meshes in the file as belonging to the same Model.
			 * @param someFilePath The FBX file to load.
			 * @param outModel The model data read from the file.
			 * @param bRegenerateNormals If True existing normal data will be discarded and completely regenerated from smoothing groups.
			 * @returns True if the model was successfully loaded, otherwise false.
			 */
			static bool LoadModel(const std::wstring& someFilePath, Model& outModel, bool bRegenerateNormals = false, bool bMergeDuplicateVertices = true);

			/**
			 * Attempts to load a FBX animation into the provided Animation structure. Does not require a mesh present in the
			 * file to function.
			 * @param someFilePath The FBX file to load.
			 * @param outAnimation The animation data read from the file.
			 * @returns True if the animation was successfully loaded, otherwise false.
			 */
			static bool LoadAnimation(const std::wstring& someFilePath, Animation& outAnimation);

			/**
			 * Attempts to load an FBX file exported by the TGA Unreal Plugin navmesh exporter. May work with other exports as well.
			 * Will ignore all data except the control points and indices in the mesh.
			 * @param someFilePath The FBX file to load.
			 * @param outNavMesh The NavMesh data read from the file.
			 * @param shouldTriangulate If import should triangulate the NavMesh, this will result in more polygons.
			 */
			static bool LoadNavMesh(const std::wstring& someFilePath, NavMesh& outNavMesh, bool shouldTriangulate = false);

			static bool IsAnimation(const std::wstring& someFilePath);
		};

	}
}

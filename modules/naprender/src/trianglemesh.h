#pragma once

// Local Includes
#include "mesh.h"

// External includes
#include <rect.h>

namespace nap
{
	/**
	 * Predefined rectangular mesh with a specific size centered at the origin on the xy axis. 
	 * When there is no size given the mesh is a uniform 1m2. The UV coordinates are always 0-1
	 * By default the plane has 1 row and 1 column
	 */
	class NAPAPI TriangleMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		TriangleMesh() = default;
		TriangleMesh(RenderService& renderService);

		/**
		 * Sets up and initializes the plane as a mesh based on the provided parameters.
		 * @param errorState contains the error message if the mesh could not be created.
		 * @return if the mesh was successfully created and initialized.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* Creates and prepares the mesh instance but doesn't initialize it.
		* Call this when you want to prepare a grid without creating the GPU representation.
		* You have to manually call init() on the mesh instance afterwards.
		* @param error contains the error code if setup fails
		* @return if setup succeeded
		*/
		bool setup(utility::ErrorState& error);

		/**
		 *	@return the mesh used for rendering
		 */
		virtual MeshInstance& getMeshInstance() override				{ return *mMeshInstance; }

		/**
		 *	@return the mesh used for rendering
		 */
		virtual const MeshInstance& getMeshInstance() const override	{ return *mMeshInstance; }

	private:
		RenderService* mRenderService = nullptr;
		std::unique_ptr<MeshInstance> mMeshInstance;

		/**
		 * Constructs the plane with x amount of columns and rows
		 * @param planeRect the rectangle used to construct the mesh
		 * @param mesh the mesh that needs to be constructed
		 */
		void constructTriangle(nap::MeshInstance& mesh);
	};

	using TriangleMeshCreator = rtti::ObjectCreator<TriangleMesh, RenderService>;
}

#pragma once

namespace nap
{
	class UniformLeafInstance;
	class UniformBufferObjectDeclaration;

	/**
	 * Blend mode for Materials.
	 */
	enum class EBlendMode : int
	{
		NotSet = 0,				///< Default value for MaterialInstances, means that the Material's blend mode is used instead
		Opaque,					///< Regular opaque, similar to (One, Zero) blend
		AlphaBlend,				///< Transparent object (SrcAlpha, InvSrcAlpha) blend
		Additive				///< Additive, (One, One) blend
	};

	/**
	 * Determines how the z-buffer is used for reading and writing.
	 */
	enum class EDepthMode : int
	{
		NotSet = 0,				///< Default value for MaterialInstances, means that the Material's blend is used instead
		InheritFromBlendMode,	///< Transparent objects do not write depth, but do read depth. Opaque objects read and write depth.
		ReadWrite,				///< Read and write depth
		ReadOnly,				///< Only read depth
		WriteOnly,				///< Only write depth
		NoReadWrite				///< Neither read or write depth
	};

	class UniformBufferObject
	{
	public:
		using UniformList = std::vector<const UniformLeafInstance*>;

		UniformBufferObject(const UniformBufferObjectDeclaration& declaration) :
			mDeclaration(&declaration)
		{
		}

		const UniformBufferObjectDeclaration*	mDeclaration;
		UniformList								mUniforms;
	};
}

#pragma once
#include <cstdint>
#include <Utils/Math/Vector3.hpp>
#include <Utils/Math/Matrix4x4.hpp>
#include <Utils/Math/Vector2.hpp>
namespace Firefly
{


	// An atmosphere layer of width 'width', and whose density is defined as
//   'exp_term' * exp('exp_scale' * h) + 'linear_term' * h + 'constant_term',
// clamped to [0,1], and where h is the altitude.
	struct DensityProfileLayer
	{
		float width;
		float exp_term;
		float exp_scale;
		float linear_term;
		float constant_term;
	};

	// An atmosphere density profile made of several layers on top of each other
	// (from bottom to top). The width of the last layer is ignored, i.e. it always
	// extend to the top atmosphere boundary. The profile values vary between 0
	// (null density) to 1 (maximum density).
	struct DensityProfile
	{
		DensityProfileLayer layers[2];
	};

	struct AtmosphereParameters
	{
		// The solar irradiance at the top of the atmosphere.
		Utils::Vec3 solar_irradiance;
		// The sun's angular radius. Warning: the implementation uses approximations
		// that are valid only if this angle is smaller than 0.1 radians.
		float sun_angular_radius;
		// The distance between the planet center and the bottom of the atmosphere.
		float bottom_radius;
		// The distance between the planet center and the top of the atmosphere.
		float top_radius;
		// The density profile of air molecules, i.e. a function from altitude to
		// dimensionless values between 0 (null density) and 1 (maximum density).
		DensityProfile rayleigh_density;
		// The scattering coefficient of air molecules at the altitude where their
		// density is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The scattering coefficient at altitude h is equal to
		// 'rayleigh_scattering' times 'rayleigh_density' at this altitude.
		Utils::Vec3 rayleigh_scattering;
		// The density profile of aerosols, i.e. a function from altitude to
		// dimensionless values between 0 (null density) and 1 (maximum density).
		DensityProfile mie_density;
		// The scattering coefficient of aerosols at the altitude where their density
		// is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The scattering coefficient at altitude h is equal to
		// 'mie_scattering' times 'mie_density' at this altitude.
		Utils::Vec3 mie_scattering;
		// The extinction coefficient of aerosols at the altitude where their density
		// is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The extinction coefficient at altitude h is equal to
		// 'mie_extinction' times 'mie_density' at this altitude.
		Utils::Vec3 mie_extinction;
		// The asymetry parameter for the Cornette-Shanks phase function for the
		// aerosols.
		float mie_phase_function_g;
		// The density profile of air molecules that absorb light (e.g. ozone), i.e.
		// a function from altitude to dimensionless values between 0 (null density)
		// and 1 (maximum density).
		DensityProfile absorption_density;
		// The extinction coefficient of molecules that absorb light (e.g. ozone) at
		// the altitude where their density is maximum, as a function of wavelength.
		// The extinction coefficient at altitude h is equal to
		// 'absorption_extinction' times 'absorption_density' at this altitude.
		Utils::Vec3 absorption_extinction;
		// The average albedo of the ground.
		Utils::Vec3 ground_albedo;
		// The cosine of the maximum Sun zenith angle for which atmospheric scattering
		// must be precomputed (for maximum precision, use the smallest Sun zenith
		// angle yielding negligible sky light radiance values. For instance, for the
		// Earth case, 102 degrees is a good choice - yielding mu_s_min = -0.2).
		float mu_s_min;
	};

	void SetEarthAtmosphere(AtmosphereParameters& aInfo);


	struct SkyAtmosphereConstantBufferStructure
	{
		//
		// From AtmosphereParameters
		//

		Utils::Vec3 solar_irradiance;
		float sun_angular_radius;

		Utils::Vec3 absorption_extinction;
		float mu_s_min;

		Utils::Vec3 rayleigh_scattering;
		float mie_phase_function_g;

		Utils::Vec3 mie_scattering;
		float bottom_radius;

		Utils::Vec3 mie_extinction;
		float top_radius;

		Utils::Vec3 mie_absorption;
		float pad00;

		Utils::Vec3 ground_albedo;
		float pad0;

		float rayleigh_density[12];
		float mie_density[12];
		float absorption_density[12];

		//
		// Add generated static header constant
		//

		int TRANSMITTANCE_TEXTURE_WIDTH;
		int TRANSMITTANCE_TEXTURE_HEIGHT;
		int IRRADIANCE_TEXTURE_WIDTH;
		int IRRADIANCE_TEXTURE_HEIGHT;

		int SCATTERING_TEXTURE_R_SIZE;
		int SCATTERING_TEXTURE_MU_SIZE;
		int SCATTERING_TEXTURE_MU_S_SIZE;
		int SCATTERING_TEXTURE_NU_SIZE;

		Utils::Vec3 SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
		float  pad3;
		Utils::Vec3 SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
		float  pad4;

		//
		// Other globals
		//
		Utils::Matrix4f gSkyViewProjMat;
		Utils::Matrix4f gSkyInvViewProjMat;
		Utils::Matrix4f gSkyInvProjMat;
		Utils::Matrix4f gSkyInvViewMat;
		Utils::Matrix4f gShadowmapViewProjMat;

		Utils::Vec3 camera;
		float  pad5;
		Utils::Vec3 sun_direction;
		float  pad6;
		Utils::Vec3 view_ray;
		float  pad7;

		float MultipleScatteringFactor;
		float MultiScatteringLUTRes;
		float pad9;
		float pad10;
	};

	struct LookUpTablesInfo
	{
		uint32_t TransmittanceTextureWidth = 256;
		uint32_t TransmittanceTextureHeight = 64;

		uint32_t ScatteringTextureRSize = 32;
		uint32_t ScatteringTextureMUSize = 128;
		uint32_t ScatteringTextureMUSSize = 32;
		uint32_t ScatteringTextureNUSize = 8;

		uint32_t IrradianceTextureWidth = 64;
		uint32_t IrradianceTextureHeight = 16;
		// Derived from above
		uint32_t ScatteringTextureWidth = 0xDEADBEEF;
		uint32_t ScatteringTextureHeight = 0xDEADBEEF;
		uint32_t ScatteringTextureDepth = 0xDEADBEEF;

		void UpdateDerivedData()
		{
			ScatteringTextureWidth = ScatteringTextureNUSize * ScatteringTextureMUSSize;
			ScatteringTextureHeight = ScatteringTextureMUSize;
			ScatteringTextureDepth = ScatteringTextureRSize;
		}

		LookUpTablesInfo() { UpdateDerivedData(); }
	};


	struct HelperValues
	{
		Utils::Matrix4f gViewProjMat;

		Utils::Vector4f gColor;

		Utils::Vec3 gSunIlluminance;
		int gScatteringMaxPathDepth;

		Utils::Vector2<uint32_t> gResolution;
		float gFrameTimeSec;
		float gTimeSec;

		Utils::Vector2<uint32_t> gMouseLastDownPos;
		uint32_t gFrameId;
		uint32_t gTerrainResolution;
		float gScreenshotCaptureActive;

		Utils::Vector2f RayMarchMinMaxSPP;
		float pad60057457;
	};
}
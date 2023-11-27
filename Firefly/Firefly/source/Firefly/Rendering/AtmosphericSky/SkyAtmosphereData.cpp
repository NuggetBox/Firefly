#include "FFpch.h"
#include "SkyAtmosphereData.h"
constexpr float pi = 3.14159265358979323846f;
void Firefly::SetEarthAtmosphere(AtmosphereParameters& aInfo)
{
	// Values shown here are the result of integration over wavelength power spectrum integrated with paricular function.
	// Refer to https://github.com/ebruneton/precomputed_atmospheric_scattering for details.

	// All units in kilometers
	static float EarthBottomRadius = 6360.0f;
	static float EarthTopRadius = 6560.0f;   // 100km atmosphere radius, less edge visible and it contain 99.99% of the atmosphere medium https://en.wikipedia.org/wiki/K%C3%A1rm%C3%A1n_line
	const float EarthRayleighScaleHeight = 8.0f;
	const float EarthMieScaleHeight = 1.2f;

	// Sun - This should not be part of the sky model...
	//info.solar_irradiance = { 1.474000f, 1.850400f, 1.911980f };
	aInfo.solar_irradiance = { 1.0f, 1.0f, 1.0f };	// Using a normalise sun illuminance. This is to make sure the LUTs acts as a transfert factor to apply the runtime computed sun irradiance over.
	aInfo.sun_angular_radius = 0.004675f;
	// Earth
	aInfo.bottom_radius = EarthBottomRadius;
	aInfo.top_radius = EarthTopRadius;
	aInfo.ground_albedo = { 0.0f, 0.0f, 0.0f };

	// Raleigh scattering
	aInfo.rayleigh_density.layers[0] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	aInfo.rayleigh_density.layers[1] = { 0.0f, 1.0f, -1.0f / EarthRayleighScaleHeight, 0.0f, 0.0f };
	aInfo.rayleigh_scattering = { 0.005802f, 0.013558f, 0.033100f };		// 1/km

	// Mie scattering
	aInfo.mie_density.layers[0] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	aInfo.mie_density.layers[1] = { 0.0f, 1.0f, -1.0f / EarthMieScaleHeight, 0.0f, 0.0f };
	aInfo.mie_scattering = { 0.003996f, 0.003996f, 0.003996f };			// 1/km
	aInfo.mie_extinction = { 0.004440f, 0.004440f, 0.004440f };			// 1/km
	aInfo.mie_phase_function_g = 0.8f;

	// Ozone absorption
	aInfo.absorption_density.layers[0] = { 25.0f, 0.0f, 0.0f, 1.0f / 15.0f, -2.0f / 3.0f };
	aInfo.absorption_density.layers[1] = { 0.0f, 0.0f, 0.0f, -1.0f / 15.0f, 8.0f / 3.0f };
	aInfo.absorption_extinction = { 0.000650f, 0.001881f, 0.000085f };	// 1/km

	const double max_sun_zenith_angle = pi * 120.0 / 180.0; // (use_half_precision_ ? 102.0 : 120.0) / 180.0 * kPi;
	aInfo.mu_s_min = (float)cos(max_sun_zenith_angle);
}

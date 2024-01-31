/**
 * @file volume.hpp
 * @brief Contains the volume interface
 */

#pragma once

#include <lightwave/color.hpp>
#include <lightwave/core.hpp>
#include <lightwave/math.hpp>

namespace lightwave {


/// @brief A Bsdf, representing the scattering distribution of a surface.
class Medium : public Object {
public:

    /**
     * @brief samples a distance based on the density of the medium
     */
    virtual float sample_distance(Sampler &rng) const { NOT_IMPLEMENTED; };

};

} // namespace lightwave

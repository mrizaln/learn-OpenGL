#ifndef MATERIAL_H
#define MATERIAL_H

// texture
#include <texture_header/texture.h>


// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using MaterialBasic = glm::vec3;
using MaterialTextured = Texture;

template <class material_type = MaterialBasic>
class Material
{
    material_type ambient{};
    material_type diffuse{};
    material_type specular{};
    float shininess{ 1.0f };

public:
    Material() = default;

    Material(material_type amb, material_type diff, material_type spec, float shin)
        : ambient{ amb }       // ambient material for textured material should be the same as diffuse
        , diffuse{ diff }
        , specular{ spec }
        , shininess{ shin }
    {
    }

    material_type& getAmbient() { return ambient; }
    material_type& getDiffuse() { return diffuse; }
    material_type& getSpecular() { return specular; }
    float getShininess() { return shininess; }
};

namespace materialCollection
{
    Material emerald{
        { 0.0215f,  0.1745f,   0.0215f },
        { 0.07568f, 0.61424f,  0.7568f },
        { 0.633f,   0.727811f, 0.633f  },
        0.6f * 128.0f
    };

    Material jade{
        { 0.135f,    0.2225f,   0.1575f   },
        { 0.54f,     0.89f,     0.63f     },
        { 0.316228f, 0.316228f, 0.316228f },
        0.1f * 128.0f
    };

    Material obsidian{
        { 0.05375f,	 0.05f,     0.06625f  },
        { 0.18275f,  0.17f,     0.22525f  },
        { 0.332741f, 0.328634f, 0.346435f },
        0.3f * 128.0f
    };

    Material pearl{
        { 0.25f,     0.20725f,  0.20725f  },
        { 1.0f,      0.829f,    0.829f    },
        { 0.296648f, 0.296648f, 0.296648f },
        0.088f * 128.0f
    };

    Material ruby{
        { 0.1745f,   0.01175f, 0.01175f   },
        { 0.61424f,  0.04136f, 0.04136f   },
        { 0.727811f, 0.626959f, 0.626959f },
        0.6f * 128.0f
    };

    Material turqoise{
        { 0.1f,      0.18725f, 0.1745f   },
        { 0.396f,    0.74151f, 0.69102f  },
        { 0.297254f, 0.30829f, 0.306678f },
        0.1f * 128.0f
    };

    Material brass{
        { 0.329412f, 0.223529f, 0.027451f },
        { 0.780392f, 0.568627f, 0.113725f },
        { 0.992157f, 0.941176f, 0.807843f },
        0.21794872f * 128.0f
    };

    Material bronze{
        { 0.2125f,   0.1275f,   0.054f   },
        { 0.714f,    0.4284f,   0.18144f },
        { 0.393548f, 0.271906f, 0.166721 },
        0.2 * 128.0f
    };

    Material chrome{
        { 0.25f,     0.25f,     0.25f     },
        { 0.4f,      0.4f,      0.4f      },
        { 0.774597f, 0.774597f, 0.774597f },
        0.6 * 128.0f
    };

    Material copper{
        { 0.19125f,  0.0735f,   0.0225f   },
        { 0.7038f,   0.27048f,  0.0828f   },
        { 0.256777f, 0.137622f, 0.086014f },
        0.1 * 128.0f
    };

    Material gold{
        { 0.24725f,  0.1995f,   0.0745f   },
        { 0.75164f,	 0.60648f,  0.22648f  },
        { 0.628281f, 0.555802f, 0.366065f },
        0.4 * 128.0f
    };

    Material silver{
        { 0.19225f,  0.19225f,  0.19225f  },
        { 0.50754f,  0.50754f,  0.50754f  },
        { 0.508273f, 0.508273f, 0.508273f },
        0.4 * 128.0f
    };

    Material<MaterialBasic> materialArray[]{
        emerald, jade, obsidian, pearl, ruby, turqoise, brass, bronze, chrome, copper, gold, silver
    };
}


#endif
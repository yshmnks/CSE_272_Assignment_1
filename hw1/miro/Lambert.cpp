#define _USE_MATH_DEFINES
#include "Lambert.h"
#include "Ray.h"
#include "Scene.h"
#include <algorithm>
#include <random>

Lambert::Lambert(const Vector3 & kd, const Vector3 & ka) :
    m_kd(kd), m_ka(ka)
{ 
}

Lambert::~Lambert()
{
}

Vector3 Lambert::BRDF(const Vector3& in, const Vector3& normal, const Vector3& out, const bool& isFront) const {
    if (dot(normal, out) < 0) return 0;
    else return m_kd / M_PI;
}

Vector3 Lambert::shade(const Ray& ray, const HitInfo& hit, const Scene& scene, const bool& isFront) const
{
    Ray rayLight;
    HitInfo hitLight;
    Vector3 L = Vector3(0.0f, 0.0f, 0.0f);
    
    const Vector3 viewDir = -ray.d; // d is a unit vector

    const PointLights *plightlist = scene.pointLights();
    // loop over all of the POINT lights
    PointLights::const_iterator plightIter;
    for (plightIter = plightlist->begin(); plightIter != plightlist->end(); plightIter++)
    {
        PointLight* pLight = *plightIter;
        Vector3 l = pLight->position() - hit.P;
        rayLight.o = hit.P;
        rayLight.d = l.normalized();
        Vector3 brdf = BRDF(rayLight.d, hit.N, -ray.d);
        if (brdf == 0) continue;
        if (scene.trace(hitLight, rayLight, 0.0001, l.length())) continue;

        // the inverse-squared falloff
        float falloff = l.length2();

        // normalize the light direction
        l /= sqrt(falloff);

        // get the diffuse component
        float nDotL = std::max(0.0f,dot(hit.N, l));
        Vector3 result = pLight->color();

        L += nDotL / falloff * pLight->wattage()*brdf * result;
    }

    const AreaLights *alightlist = scene.areaLights();
    // loop over all of the lights
    AreaLights::const_iterator alightIter;
    for (alightIter = alightlist->begin(); alightIter != alightlist->end(); alightIter++)
    {
        AreaLight* aLight = *alightIter;
        vec3pdf vp = aLight->randPt();
        Vector3 l = vp.v - hit.P; // shoot a shadow ray to a random point on the area light
        rayLight.o = hit.P;
        rayLight.d = l.normalized();

        Vector3 brdf = BRDF(rayLight.d, hit.N, -ray.d);
        if (brdf == 0) continue;
        // if the shadow ray hits the "backside of the light" continue to the next area light
        if (!aLight->intersect(hitLight, rayLight)){
            //printf("front-side of light not visible\n");
            continue;
        }
        // if the shadow ray is occluded by another (hence the "skip") object continue the next light
        if (scene.trace(hitLight, rayLight, aLight, 0.0001, l.length())){
            //printf("random point on light occluded\n");
            continue;
        }

        // the inverse-squared falloff
        float falloff = l.length2();

        // normalize the light direction
        l /= sqrt(falloff);

        // get the diffuse component
        float nDotL = std::max(0.0f, dot(hit.N, l));
        Vector3 result = aLight->color();

        L += std::max(0.0f, dot(hitLight.N, -l))*nDotL / falloff*
            aLight->wattage() / aLight->area()*brdf * result / (vp.p);
    }
    
    // add the ambient component
    L += m_ka;
    
    return L;
}//*/

vec3pdf Lambert::randReflect(const Vector3& in, const Vector3& normal, const bool& isFront) const{
    //double phi = 2.0 * M_PI*((double)rand() / RAND_MAX);
    //double theta = acos((double)rand() / RAND_MAX);
    //Vector3 d = Vector3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
    double u = ((double)rand() / RAND_MAX);
    while (u == 1) u = ((double)rand() / RAND_MAX);
    double v = 2.0 * M_PI*((double)rand() / RAND_MAX);
    Vector3 d = Vector3(cos(v)*sqrt(u), sin(v)*sqrt(u), sqrt(1 - u));

    // generate a basis with surface normal hit.N as the z-axis
    Vector3 z = normal.normalized();
    float a = dot(Vector3(1, 0, 0), z);
    float b = dot(Vector3(0, 1, 0), z);
    Vector3 y;
    if (fabs(a) < fabs(b)) y = Vector3(1, 0, 0).orthogonal(z).normalize();
    else y = Vector3(0, 1, 0).orthogonal(z).normalize();
    Vector3 x = cross(y, z).normalize();
    return vec3pdf(d[0] * x + d[1] * y + d[2] * z, sqrt(1 - u) / M_PI);
}

float Lambert::reflectPDF(const Vector3& in, const Vector3& normal, const Vector3& out, const bool& isFront) const {
    return std::max(0.0f, dot(out, normal)) / M_PI;
}


vec3pdf Lambert::randEmit(const Vector3& n) const {
    double u = ((double)rand() / RAND_MAX);
    while (u == 1.0) u = ((double)rand() / RAND_MAX);
    double v = 2.0 * M_PI*((double)rand() / RAND_MAX);
    Vector3 d = Vector3(cos(v)*sqrt(u), sin(v)*sqrt(u), sqrt(1 - u));
    Vector3 z = n.normalized();

    float a = dot(Vector3(1, 0, 0), z);
    float b = dot(Vector3(0, 1, 0), z);
    Vector3 y;
    if (fabs(a) < fabs(b)) y = Vector3(1, 0, 0).orthogonal(z).normalize();
    else y = Vector3(0, 1, 0).orthogonal(z).normalize();
    Vector3 x = cross(y, z).normalize();

    return vec3pdf(d[0] * x + d[1] * y + d[2] * z, sqrt(1-u)/M_PI);
}

float Lambert::emitPDF(const Vector3& n, const Vector3& v) const {
    float z = dot(n, v);
    if (z < 0) return 0;
    else return z / M_PI;
}

Vector3 Lambert::radiance(const Vector3& normal, const Vector3& direction) const {
    if (dot(normal, direction) < 0) return Vector3(0, 0, 0);
    return m_powerPerArea / (2.0*M_PI);
}

Vector3 Lambert::sum_L_cosTheta_dOmega() const {
    return M_PI*radiance(Vector3(0, 0, 1), Vector3(0, 0, 1));
}
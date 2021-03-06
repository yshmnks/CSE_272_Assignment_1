#ifndef CSE168_PHOTONMAP_H_INCLUDED
#define CSE168_PHOTONMAP_H_INCLUDED

#include <vector>
#include <queue>
#include <algorithm>
#include <functional>
#include "RayPath.h"
#include "Sphere.h"

class PhotonDeposit {
public:
    Vector3 m_power;
    LightPath* m_lightPath;
    int m_hitIndex;

    PhotonDeposit() : m_power(Vector3(0, 0, 0)), m_lightPath(NULL), m_hitIndex(-1) {}
    PhotonDeposit(const Vector3& power) : m_power(power), m_lightPath(NULL), m_hitIndex(-1) {}
    PhotonDeposit(const Vector3& power, LightPath* lightPath, const int& hitIndex) : m_power(power), m_lightPath(lightPath), m_hitIndex(hitIndex) {}
    PhotonDeposit(const PhotonDeposit& copy) : m_power(copy.m_power), m_lightPath(copy.m_lightPath), m_hitIndex(copy.m_hitIndex) {}

    Vector3 location() const {
        if (m_hitIndex < 0) return m_lightPath->m_lightHit.P;
        else return m_lightPath->m_hit[m_hitIndex].P;
    }
};

struct RadiusDensityPhotons {
    float m_radius;
    Vector3 m_density;
    std::vector<PhotonDeposit> m_photons;

    RadiusDensityPhotons() : m_radius(0), m_density(0), m_photons(std::vector<PhotonDeposit>(0)) {}
};

struct RsqrPhoton {
    float m_r2;
    PhotonDeposit m_photon;
    RsqrPhoton() : m_r2(0), m_photon(PhotonDeposit()) {}
    RsqrPhoton(const float& r2, const PhotonDeposit& photon) : m_r2(r2), m_photon(photon) {}
    bool operator<(const RsqrPhoton& rhs) const {
        if (m_r2 < rhs.m_r2) return true;
        else if (m_r2 > rhs.m_r2) return false;
        else if (m_photon.location()[0] < rhs.m_photon.location()[0]) return true;
        else if (m_photon.location()[0] > rhs.m_photon.location()[0]) return false;
        else if (m_photon.location()[1] < rhs.m_photon.location()[1]) return true;
        else if (m_photon.location()[1] > rhs.m_photon.location()[1]) return false;
        else if (m_photon.location()[2] < rhs.m_photon.location()[2]) return true;
        else if (m_photon.location()[2] > rhs.m_photon.location()[2]) return false;
        else if (m_photon.m_power[0] < rhs.m_photon.m_power[0]) return true;
        else if (m_photon.m_power[0] > rhs.m_photon.m_power[0]) return false;
        else if (m_photon.m_power[1] < rhs.m_photon.m_power[1]) return true;
        else if (m_photon.m_power[1] > rhs.m_photon.m_power[1]) return false;
        else if (m_photon.m_power[2] < rhs.m_photon.m_power[2]) return true;
        else if (m_photon.m_power[2] > rhs.m_photon.m_power[2]) return false;
        else return false;
    }
};


class SequentialPhotonMap {
protected:
    Vector3 m_xyz;
    Vector3 m_XYZ;
    std::vector<PhotonDeposit> m_photons;

    void buildBalancedTree(int& nPhotons, PhotonMap*& photonMap, std::vector<PhotonDeposit> photons, int depth = 0, bool verbose = false);
public:
    SequentialPhotonMap() : m_photons(std::vector<PhotonDeposit>(0)), m_xyz(Vector3(0, 0, 0)), m_XYZ(Vector3(0, 0, 0)) {}
    ~SequentialPhotonMap() {}

    float xMin() { return m_xyz.x; }
    float yMin() { return m_xyz.y; }
    float zMin() { return m_xyz.z; }
    float xMax() { return m_XYZ.x; }
    float yMax() { return m_XYZ.y; }
    float zMax() { return m_XYZ.z; }
    int nPhotons() { return m_photons.size(); }
    PhotonDeposit operator[](int i) const { return m_photons[i]; }
    PhotonDeposit& operator[](int i) { return m_photons[i]; }
    Vector3 powerDensity(const Vector3& x, const float& r);
    void addPhoton(const PhotonDeposit& photonDeposit);
    RadiusDensityPhotons radiusDensityPhotons(const Vector3& x, const int& n);
    PhotonMap* buildTree();
    PhotonMap* buildBalancedTree(int depth = 0, bool verbose = false);

    std::vector<PhotonDeposit> getPhotons() { return m_photons; }
};

class PhotonMap {
protected:
    int m_depth;
    int m_axis;
    Vector3 m_xyz;
    Vector3 m_XYZ;
    PhotonMap* m_parent;
    PhotonMap* m_child0;
    PhotonMap* m_child1;
    PhotonDeposit* m_photon; // the photon associated with this octant if this is a leaf node

    void setAxis() { m_axis = m_depth % 3; } // this is just here in case one wishes to define a different splitting convention
    void getNearestPhotons(const Vector3& x, const int& k, std::priority_queue<RsqrPhoton>& photons);
public:
    PhotonMap() {};
    PhotonMap(
        const Vector3& xyz, const Vector3& XYZ,
        PhotonDeposit* photon = NULL,
        PhotonMap* parent = NULL,
        PhotonMap* child0 = NULL, PhotonMap* child1 = NULL) :
        m_xyz(xyz), m_XYZ(XYZ), m_photon(photon), m_parent(parent), m_child0(child0), m_child1(child1)
    {
        if (m_parent == NULL) m_depth = 0;
        else m_depth = m_parent->m_depth + 1;
        setAxis();
    }
    PhotonMap(const PhotonMap& copy) :
        m_xyz(copy.m_xyz), m_XYZ(copy.m_XYZ), m_photon(copy.m_photon), m_parent(copy.m_parent), m_child0(copy.m_child0), m_child1(copy.m_child1) {}
    ~PhotonMap() { delete m_child0; delete m_child1; } // recursively delete children

    PhotonMap* getLeafNode(const Vector3& x);
    inline bool isLeafNode() const { return m_child0 == NULL; }
    void addPhoton(PhotonDeposit photon);
    std::vector<PhotonDeposit> getPhotons(const Vector3& bmin, const Vector3& bmax);
    std::vector<PhotonDeposit> getPhotons(const Vector3& x, const float& r) {
        std::vector<PhotonDeposit> candidatePhotons = getPhotons(x-Vector3(r,r,r),x+Vector3(r,r,r));
        std::vector<PhotonDeposit> photons(0);
        for (unsigned int i = 0; i < candidatePhotons.size(); i++) {
            if ((candidatePhotons[i].location() - x).length2() < r*r) photons.push_back(candidatePhotons[i]);
        }
        return photons;
    }
    std::vector<PhotonDeposit> getAllPhotons() { return getPhotons(m_xyz, m_XYZ); }
    std::vector<PhotonDeposit> getNearestPhotons(const Vector3& x, const int& n); // returns the n nearest neighbors of input location x
    void buildBalancedTree(std::vector<PhotonDeposit> spm, int depth = 0);
    RadiusDensityPhotons radiusDensityPhotons(const Vector3& x, const int& n);

    inline PhotonMap* getSibling() {
        if (m_parent == NULL) return NULL;
        else if (m_parent->m_child0 == this) return m_parent->m_child1;
        else return m_parent->m_child0;
    }
};

#endif // CSE168_PHOTONMAP_H_INCLUDED
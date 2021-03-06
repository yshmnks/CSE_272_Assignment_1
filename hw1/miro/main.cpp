#include <math.h>
#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"

#include "ParallelogramLight.h"
#include "TriangleLight.h"
#include "PointLight.h"
#include "Sphere.h"
#include "Parallelogram.h"
#include "PolygonMesh.h"
#include "Triangle.h"
#include "Quad.h"
#include "Lambert.h"
#include "Mirror.h"
#include "Phong.h"
#include "RefractiveInterface.h"
#include "MiroWindow.h"

void
makeTestScene() {
    g_camera = new Camera;
    g_scene = new Scene;
    g_image = new Image;
    g_image->resize(180, 180);

    //Vector3 eye = Vector3(-0.5, 2.75, 1.5).rotate(-60, Vector3(1, 0, 0)) + Vector3(30.661);
    //g_camera->setEye(eye);
    
    g_camera->setEye(Vector3(-100, 100, 200));
    g_camera->setLookAt(Vector3(-64, 0, 0));
    g_camera->setUp(Vector3(0, 1, 0));
    g_camera->setFOV(40);

    g_scene->setPreview(true);
    g_scene->setSamplesPerPix(1024);
    g_scene->setBidiSamplesPerPix(256);
    g_scene->setMaxBounces(100);
    g_scene->setMaxEyePaths(64);
    g_scene->setMaxLightPaths(64);
    g_scene->setPhotonSamples(100000000);

    PolygonMesh* mesh = new PolygonMesh();
    mesh->load("models/caustic.obj");
    mesh->addMeshToScene(g_scene);
    g_scene->preCalc();
}


void
makeRoomScene(){
    g_camera = new Camera;
    g_scene = new Scene;
    g_image = new Image;
    g_image->resize(256, 256);

    // set up the camera
    g_camera->setEye(Vector3(0, -4, 1));
    g_camera->setLookAt(Vector3(0, 0, 1));
    g_camera->setUp(Vector3(0, 0, 1));
    g_camera->setFOV(40);

    g_scene->setPreview(true);
    g_scene->setSamplesPerPix(16);
    g_scene->setBidiSamplesPerPix(4);
    g_scene->setMaxBounces(16);
    g_scene->setMaxEyePaths(64);
    g_scene->setMaxLightPaths(64);
    g_scene->setPhotonSamples(100000000);

    // create room geometry
    
    Lambert* mat = new Lambert(Vector3(1.0f, 1.0f, 1.0f));
    mat->setKd(0.8f);
    Mirror* mir = new Mirror(Vector3(1.0f, 1.0f, 1.0f));
    mir->setKs(0.8f);
    Phong* pho = new Phong(0.0f, 0.8f, 50);
    Lambert* coverMat = new Lambert(Vector3(0.0f, 0.0f, 0.0f));
    coverMat->setKd(0.0f);


    Parallelogram * wall_F = new Parallelogram(Vector3(0, 1, 1), Vector3(1, 0, 0), Vector3(0, 0, 1), 1, 1); // far
    Parallelogram * wall_L = new Parallelogram(Vector3(-1, 0, 1), Vector3(0, 1, 0), Vector3(0, 0, 1), 1, 1); // left
    Parallelogram * wall_R = new Parallelogram(Vector3(1, 0, 1), Vector3(0, 0, 1), Vector3(0, 1, 0), 1, 1); // right
    Parallelogram * wall_T = new Parallelogram(Vector3(0, 0, 2), Vector3(0, 1, 0), Vector3(1, 0, 0), 1, 1); // top
    Parallelogram * wall_B = new Parallelogram(Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0), 1, 1); // bottom
    wall_T->setMaterial(mat);
    wall_B->setMaterial(mat);
    wall_L->setMaterial(mat);
    wall_R->setMaterial(mat);
    wall_F->setMaterial(mat);
    g_scene->addObject(wall_B);
    g_scene->addObject(wall_F);
    g_scene->addObject(wall_L);
    g_scene->addObject(wall_R);
    g_scene->addObject(wall_T);//*/

    ParallelogramLight * light = new ParallelogramLight(Vector3(0, 0, 1.98), Vector3(1, 0, 0), Vector3(0, 1, 0), 0.1, 0.1);
    g_scene->addAreaLight(light, 20000);

    Parallelogram * cover = new Parallelogram(Vector3(0, 0, 1.98), Vector3(0, 1, 0), Vector3(1, 0, 0), 0.1, 0.1);
    g_scene->addObject(cover);
    cover->disableBack();
    cover->setMaterial(coverMat);

    //light->flip(); cover->flip(); light->setWattage(2);
    

    g_scene->preCalc();
}

int
main(int argc, char*argv[])
{
    // create a scene
    makeRoomScene();
    /*unsigned int cw;
    _controlfp_s(&cw, 0, 0);
    cw &=~(EM_OVERFLOW|EM_UNDERFLOW|EM_ZERODIVIDE|
            EM_DENORMAL|EM_INVALID);
    unsigned int cwOriginal;
    _controlfp_s(&cwOriginal,cw, _MCW_EM);*/
    MiroWindow miro(&argc, argv);
    miro.mainLoop();

    return 0; // never executed
}

#include <vtkGlyph3D.h>
#include <vtkLODActor.h>
#include <vtkNamedColors.h>
#include <vtkPDBReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkTubeFilter.h>

#include <cmath>

#include "UIElements.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Expected pdb filenname" << std::endl;
        return EXIT_FAILURE;
    }

    // init vtk window and make it interactive
    vtkNew<vtkNamedColors> colors;

    vtkNew<vtkRenderer> renderer;
    renderer->SetBackground(colors->GetColor3d("SlateGray").GetData());

    vtkNew<vtkRenderWindow> window;
    window->AddRenderer(renderer);
    window->SetSize(640, 480);

    vtkNew<vtkRenderWindowInteractor> interactor;
    interactor->SetRenderWindow(window);

    // read pdb file
    vtkNew<vtkPDBReader> pdb;
    pdb->SetFileName(argv[1]);
    pdb->SetHBScale(1.0);
    pdb->SetBScale(1.0);
    pdb->Update();

    std::cout << "Number of atoms: " << pdb->GetNumberOfAtoms() << std::endl;

    // if we have a lot of atoms we lower the resolution to improve rendering
    double resolution = std::sqrt(300000.0 / pdb->GetNumberOfAtoms());

    if (resolution > 20) { // max resolution
        resolution = 20;
    }
    if (resolution < 4) {  // min resolution
        resolution = 4;
    }
    std::cout << "Resolution: " << resolution << std::endl;

    // create spheres for atoms
    vtkNew<vtkSphereSource> sphere;
    sphere->SetCenter(0, 0, 0);
    sphere->SetRadius(1);
    sphere->SetThetaResolution(static_cast<int>(resolution));
    sphere->SetPhiResolution(static_cast<int>(resolution));

    vtkNew<vtkGlyph3D> glyph;
    glyph->SetInputConnection(pdb->GetOutputPort());
    glyph->SetOrient(1);
    glyph->SetColorMode(1);
    glyph->ScalingOn();
    glyph->SetScaleMode(2);
    glyph->SetScaleFactor(0.25);
    glyph->SetSourceConnection(sphere->GetOutputPort());

    vtkNew<vtkPolyDataMapper> atomMapper;
    atomMapper->SetInputConnection(glyph->GetOutputPort());
    atomMapper->UseLookupTableScalarRangeOff();
    atomMapper->ScalarVisibilityOn();
    atomMapper->SetScalarModeToDefault();

    vtkNew<vtkLODActor> atom;
    atom->SetMapper(atomMapper);
    atom->GetProperty()->SetRepresentationToSurface();
    atom->GetProperty()->SetInterpolationToGouraud();
    atom->GetProperty()->SetAmbient(0.1);
    atom->GetProperty()->SetDiffuse(0.7);
    atom->GetProperty()->SetSpecular(0.5);
    atom->GetProperty()->SetSpecularPower(80);
    atom->GetProperty()->SetSpecularColor(colors->GetColor3d("White").GetData());
    atom->SetNumberOfCloudPoints(30000);

    renderer->AddActor(atom);

    // add tubes for bonds between atoms
    vtkNew<vtkTubeFilter> tube;
    tube->SetInputConnection(pdb->GetOutputPort());
    tube->SetNumberOfSides(static_cast<int>(resolution));
    tube->CappingOff();
    tube->SetRadius(0.2);
    tube->SetVaryRadius(0);
    tube->SetRadiusFactor(10);

    vtkNew<vtkPolyDataMapper> bondMapper;
    bondMapper->SetInputConnection(tube->GetOutputPort());
    bondMapper->UseLookupTableScalarRangeOff();
    bondMapper->ScalarVisibilityOff();
    bondMapper->SetScalarModeToDefault();

    vtkNew<vtkLODActor> bond;
    bond->SetMapper(bondMapper);
    bond->GetProperty()->SetRepresentationToSurface();
    bond->GetProperty()->SetInterpolationToGouraud();
    bond->GetProperty()->SetAmbient(0.1);
    bond->GetProperty()->SetDiffuse(0.7);
    bond->GetProperty()->SetSpecular(0.5);
    bond->GetProperty()->SetSpecularPower(80);
    bond->GetProperty()->SetSpecularColor(colors->GetColor3d("White").GetData());

    renderer->AddActor(bond);

    UIElements ui(interactor);
    ui.AddResolutionSlider(sphere, tube, "ResolutionSlider");

    window->SetWindowName("ProteinViewer");
    window->Render();
    interactor->Initialize();
    interactor->Start();

    return EXIT_SUCCESS;
}
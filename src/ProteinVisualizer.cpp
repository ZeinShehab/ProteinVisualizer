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
#include <vtkSliderRepresentation2D.h>
#include <vtkSliderWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkTextProperty.h>

#include <cmath>


namespace {
// this does the actual work of the sliders
// callback for the interaction.
class vtkRadiusSliderCallback : public vtkCallbackCommand
{
public:
  static vtkRadiusSliderCallback* New()
  {
    return new vtkRadiusSliderCallback;
  }
  virtual void Execute(vtkObject* caller, unsigned long, void*)
  {
    vtkSliderWidget* sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);
    
    this->SphereSource->SetRadius(
        static_cast<vtkSliderRepresentation*>(sliderWidget->GetRepresentation())
            ->GetValue());
  }
  vtkRadiusSliderCallback() : SphereSource(0)
  {
  }
  vtkSphereSource* SphereSource;
};

class vtkResolutionSliderCallback : public vtkCallbackCommand
{
public:
  static vtkResolutionSliderCallback* New()
  {
    return new vtkResolutionSliderCallback;
  }
  virtual void Execute(vtkObject* caller, unsigned long, void*)
  {
    vtkSliderWidget* sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);

    this->SphereSource->SetThetaResolution(
        static_cast<vtkSliderRepresentation*>(sliderWidget->GetRepresentation())
            ->GetValue());
    
    this->SphereSource->SetPhiResolution(
        static_cast<vtkSliderRepresentation*>(sliderWidget->GetRepresentation())
            ->GetValue());
  }
  vtkResolutionSliderCallback() : SphereSource(0)
  {
  }
  vtkSphereSource* SphereSource;
};
} // namespace

void DrawAtoms(vtkPDBReader* pdb, vtkRenderer* renderer, vtkSphereSource* sphereSource,vtkNamedColors* colors, int resolution) 
{
    vtkNew<vtkGlyph3D> glyph;
    glyph->SetInputConnection(pdb->GetOutputPort());
    glyph->SetOrient(1);
    glyph->SetColorMode(1);
    glyph->ScalingOn();
    glyph->SetScaleMode(2);
    glyph->SetScaleFactor(0.25);
    glyph->SetSourceConnection(sphereSource->GetOutputPort());

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
}

void DrawBonds(vtkPDBReader* pdb, vtkRenderer* renderer, vtkTubeFilter* tube, vtkNamedColors* colors, int resolution)
{
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
}

int main(int argc, char* argv[]) 
{
    if (argc < 2) {
        std::cerr << "Expected pdb filenname" << std::endl;
        return EXIT_FAILURE;
    }

    // init vtk window and make it interactive
    vtkNew<vtkNamedColors> colors;
    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkRenderWindow> window;
    vtkNew<vtkRenderWindowInteractor> interactor;

    renderer->SetBackground(colors->GetColor3d("SlateGray").GetData());

    window->AddRenderer(renderer);
    window->SetSize(800, 600);

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

    // add tubes for bonds between atoms
    vtkNew<vtkTubeFilter> tube;
    tube->SetInputConnection(pdb->GetOutputPort());
    tube->SetNumberOfSides(static_cast<int>(resolution));
    tube->CappingOff();
    tube->SetRadius(0.2);
    tube->SetVaryRadius(0);
    tube->SetRadiusFactor(10);

    DrawAtoms(pdb, renderer, sphere, colors, resolution);
    DrawBonds(pdb, renderer, tube, colors, resolution);

    // add slider to control atom radius 
    vtkNew<vtkSliderRepresentation2D> radiusSlider;

    radiusSlider->SetMinimumValue(1.0);
    radiusSlider->SetMaximumValue(4.0);
    radiusSlider->SetValue(1.0);
    radiusSlider->SetTitleText("Atom Radius");

    radiusSlider->GetTitleProperty()->SetColor(colors->GetColor3d("AliceBlue").GetData());

    radiusSlider->SetSliderLength(0.02);
    radiusSlider->SetSliderWidth(0.05);
    radiusSlider->SetEndCapLength(0.02);
    radiusSlider->SetEndCapWidth(0.02);

    radiusSlider->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
    radiusSlider->GetPoint1Coordinate()->SetValue(0.1, 0.1);
    radiusSlider->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
    radiusSlider->GetPoint2Coordinate()->SetValue(0.4, 0.1);

    vtkNew<vtkSliderWidget> radiusSliderWidget;
    radiusSliderWidget->SetInteractor(interactor);
    radiusSliderWidget->SetRepresentation(radiusSlider);
    radiusSliderWidget->SetAnimationModeToAnimate();
    radiusSliderWidget->EnabledOn();

    vtkNew<vtkRadiusSliderCallback> radiusCallback;
    radiusCallback->SphereSource = sphere;

    radiusSliderWidget->AddObserver(vtkCommand::EndInteractionEvent, radiusCallback);

    // slider to control resolutionn slider
    vtkNew<vtkSliderRepresentation2D> resolutionSlider;

    resolutionSlider->SetMinimumValue(4.0);
    resolutionSlider->SetMaximumValue(10.0);
    resolutionSlider->SetValue(resolution);
    resolutionSlider->SetTitleText("Atom Resolution");

    resolutionSlider->GetTitleProperty()->SetColor(colors->GetColor3d("AliceBlue").GetData());

    resolutionSlider->SetSliderLength(0.03);
    resolutionSlider->SetSliderWidth(0.05);
    resolutionSlider->SetEndCapWidth(0.02);
    resolutionSlider->SetEndCapLength(0.02);

    resolutionSlider->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
    resolutionSlider->GetPoint1Coordinate()->SetValue(0.6, 0.1);
    resolutionSlider->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
    resolutionSlider->GetPoint2Coordinate()->SetValue(0.9, 0.1);

    vtkNew<vtkSliderWidget> resolutionSliderWidget;
    resolutionSliderWidget->SetInteractor(interactor);
    resolutionSliderWidget->SetRepresentation(resolutionSlider);
    resolutionSliderWidget->SetAnimationModeToAnimate();
    resolutionSliderWidget->EnabledOn();

    vtkNew<vtkResolutionSliderCallback> resolutionCallback;
    resolutionCallback->SphereSource = sphere;

    resolutionSliderWidget->AddObserver(vtkCommand::EndInteractionEvent, resolutionCallback);

    window->SetWindowName("ProteinViewer");
    interactor->Initialize();
    interactor->Start();
    window->Render();

    return EXIT_SUCCESS;
}
#include "UIElements.h"
#include <vtkSliderRepresentation2D.h>
#include <vtkProperty2D.h>
#include <vtkTextProperty.h>

SliderCallback* SliderCallback::New() {
    return new SliderCallback;
}

void SliderCallback::SetSphereSource(vtkSphereSource* sphereSource) {
    this->SphereSource = sphereSource;
}

void SliderCallback::SetTubeFilter(vtkTubeFilter* tubeFilter) {
    this->TubeFilter = tubeFilter;
}

void SliderCallback::Execute(vtkObject* caller, unsigned long, void*) {
    auto slider = reinterpret_cast<vtkSliderWidget*>(caller);
    auto sliderRep = dynamic_cast<vtkSliderRepresentation2D*>(slider->GetRepresentation());
    if (!sliderRep) {
        std::cerr << "Error: Unable to cast to vtkSliderRepresentation2D." << std::endl;
        return;
    }

    double value = sliderRep->GetValue();

    if (this->SphereSource) {
        this->SphereSource->SetThetaResolution(static_cast<int>(value));
        this->SphereSource->SetPhiResolution(static_cast<int>(value));
        this->SphereSource->Update();
    }

    if (this->TubeFilter) {
        this->TubeFilter->SetNumberOfSides(static_cast<int>(value));
        this->TubeFilter->Update();
    }
}

UIElements::UIElements(vtkRenderWindowInteractor* interactor) : Interactor(interactor) {}

void UIElements::AddResolutionSlider(vtkSphereSource* sphere, vtkTubeFilter* tube, const std::string& name) {
    vtkNew<vtkSliderRepresentation2D> sliderRep;
    sliderRep->SetMinimumValue(4.0);
    sliderRep->SetMaximumValue(20.0);
    sliderRep->SetValue(10.0);
    sliderRep->SetTitleText("Resolution");

    sliderRep->GetSliderProperty()->SetColor(0.8, 0.8, 0.8);
    sliderRep->GetTitleProperty()->SetColor(0.8, 0.8, 0.8);
    sliderRep->GetLabelProperty()->SetColor(0.8, 0.8, 0.8);
    sliderRep->GetSelectedProperty()->SetColor(1.0, 0.5, 0.0);

    sliderRep->SetSliderLength(0.02);
    sliderRep->SetSliderWidth(0.03);
    sliderRep->SetEndCapLength(0.01);
    sliderRep->SetEndCapWidth(0.03);
    sliderRep->SetTubeWidth(0.005);

    vtkNew<vtkSliderWidget> sliderWidget;
    sliderWidget->SetInteractor(this->Interactor);
    sliderWidget->SetRepresentation(sliderRep);
    sliderWidget->SetAnimationModeToAnimate();
    sliderWidget->EnabledOn();

    vtkNew<SliderCallback> callback;
    callback->SetSphereSource(sphere);
    callback->SetTubeFilter(tube);

    sliderWidget->AddObserver(vtkCommand::InteractionEvent, callback);

    Sliders[name] = sliderWidget;
}

#ifndef UIELEMENTS_H
#define UIELEMENTS_H

#include <vtkSliderWidget.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSphereSource.h>
#include <vtkTubeFilter.h>
#include <vtkCommand.h>
#include <map>
#include <string>

class SliderCallback : public vtkCommand {
public:
    static SliderCallback* New();
    void SetSphereSource(vtkSphereSource* sphereSource);
    void SetTubeFilter(vtkTubeFilter* tubeFilter);
    void Execute(vtkObject* caller, unsigned long eventId, void* callData) override;

private:
    vtkSphereSource* SphereSource = nullptr;
    vtkTubeFilter* TubeFilter = nullptr;
};

class UIElements {
public:
    UIElements(vtkRenderWindowInteractor* interactor);

    void AddResolutionSlider(vtkSphereSource* sphere, vtkTubeFilter* tube, const std::string& name);

    // TODO

private:
    vtkRenderWindowInteractor* Interactor;
    // map of sliders by name
    std::map<std::string, vtkSliderWidget*> Sliders;
};

#endif

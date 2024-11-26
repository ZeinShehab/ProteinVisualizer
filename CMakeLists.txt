cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(ProteinVisualizer)

find_package(VTK COMPONENTS
    CommonColor
    FiltersCore
    FiltersSources
    IOChemistry
    InteractionStyle
    RenderingContextOpenGL2
    RenderingCore
    RenderingFreeType
    RenderingGL2PSOpenGL2
    RenderingLOD
    RenderingOpenGL2
)

if (NOT VTK_FOUND)
    message(FATAL_ERROR "ProteinVisualizer: Can't find VTK build folder")
endif()

add_executable(ProteinVisualizer MACOSX_BUNDLE ProteinVisualizer.cpp)
target_link_libraries(ProteinVisualizer PRIVATE ${VTK_LIBRARIES})

vtk_module_autoinit(
    TARGETS ProteinVisualizer
    MODULES ${VTK_LIBRARIES}
)
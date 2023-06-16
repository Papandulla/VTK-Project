#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkVolume16Reader.h"
#include "vtkPolyDataMapper.h"
#include "vtkDataSetMapper.h"
#include "vtkActor.h"
#include "vtkOutlineFilter.h"
#include "vtkCamera.h"
#include "vtkProperty.h"
#include "vtkPolyDataNormals.h"
#include "vtkContourFilter.h"
#include "vtkStructuredPointsReader.h"
#include "vtkMarchingCubes.h"
#include "vtkRecursiveDividingCubes.h"
#include "vtkScalarBarWidget.h"
#include "vtkScalarBarActor.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"
#include "vtkVolumeProperty.h"
#include "vtkFixedPointVolumeRayCastMapper.h"
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTextRepresentation.h>
#include <vtkTextWidget.h>
#include <vtkNamedColors.h>

bool rayMarchingMode = false;
double iso1 = 1100.0;
double iso2 = 300.0;
int rayStepSize = 150;
double color5 = 1150.0;
double opacity5 = 0.9;

vtkRenderer* aRenderer = vtkRenderer::New();
vtkRenderWindow* renWin = vtkRenderWindow::New();
vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();

void IsoSurface(vtkRenderer* aRenderer, vtkRenderWindow* renWin, vtkRenderWindowInteractor* iren)
{
    vtkVolume16Reader* reader = vtkVolume16Reader::New();
    reader->SetDataDimensions(256, 256);
    reader->SetImageRange(1, 109);
    reader->SetDataByteOrderToLittleEndian();
    reader->SetFilePrefix("../data/MRbrain");
    reader->SetDataSpacing(1, 1, 2);

    // This next section creates two contours for the density data.  A
    //    vtkContourFilter object is created that takes the input data from
    //    the reader.																		
    vtkContourFilter* contourExtractor = vtkContourFilter::New();
    contourExtractor->SetInputConnection(reader->GetOutputPort());
    contourExtractor->SetValue(0, iso1);

    vtkContourFilter* contourExtractor2 = vtkContourFilter::New();
    contourExtractor2->SetInputConnection(reader->GetOutputPort());
    contourExtractor2->SetValue(0, iso2);


    // This section creates the polygon normals for the contour surfaces
    //    and creates the mapper that takes in the newly normalized surfaces
    vtkPolyDataNormals* contourNormals = vtkPolyDataNormals::New();
    contourNormals->SetInputConnection(contourExtractor->GetOutputPort());
    contourNormals->SetFeatureAngle(60.0);
    vtkPolyDataMapper* contourMapper = vtkPolyDataMapper::New();
    contourMapper->SetInputConnection(contourNormals->GetOutputPort());
    contourMapper->ScalarVisibilityOff();
    vtkPolyDataNormals* contourNormals2 = vtkPolyDataNormals::New();
    contourNormals2->SetInputConnection(contourExtractor2->GetOutputPort());
    contourNormals2->SetFeatureAngle(60.0);
    vtkPolyDataMapper* contourMapper2 = vtkPolyDataMapper::New();
    contourMapper2->SetInputConnection(contourNormals2->GetOutputPort());
    contourMapper2->ScalarVisibilityOff();


    // This section sets up the Actor that takes the contour
    //    This is where you can set the color and opacity of the two contours
    vtkActor* contour = vtkActor::New();
    contour->SetMapper(contourMapper);
    contour->GetProperty()->SetColor(0.8, 0.4, 0.6);
    contour->GetProperty()->SetOpacity(0.3);
    vtkActor* contour2 = vtkActor::New();
    contour2->SetMapper(contourMapper2);
    contour2->GetProperty()->SetColor(0.8, 0.8, 0.8);
    contour2->GetProperty()->SetOpacity(1.0);

    // This part creates the colorMap function to the volume rendering.
    /*vtkPiecewiseFunction* opacityTransferFunction = vtkPiecewiseFunction::New();
    opacityTransferFunction->AddPoint(20, 0.0);
    opacityTransferFunction->AddPoint(495, 0.0);
    opacityTransferFunction->AddPoint(500, 0.3);
    opacityTransferFunction->AddPoint(1150, 0.5);
    opacityTransferFunction->AddPoint(1500, 0.9);

    vtkColorTransferFunction* colorTransferFunction = vtkColorTransferFunction::New();
    colorTransferFunction->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
    colorTransferFunction->AddRGBPoint(500.0, 1.0, 0.6, 0.0);
    colorTransferFunction->AddRGBPoint(700.0, 1.0, 0.6, 0.0);
    colorTransferFunction->AddRGBPoint(800.0, 1.0, 0.0, 0.0);
    colorTransferFunction->AddRGBPoint(1150.0, 0.9, 0.9, 0.9);

    contour->GetProperty()->SetColor(colorTransferFunction->GetRedValue(contourExtractor->GetValue(0)), colorTransferFunction->GetGreenValue(contourExtractor->GetValue(0)), colorTransferFunction->GetBlueValue(contourExtractor->GetValue(0)));
    contour->GetProperty()->SetOpacity(opacityTransferFunction->GetValue(contourExtractor->GetValue(0)));
    contour2->GetProperty()->SetColor(colorTransferFunction->GetRedValue(contourExtractor->GetValue(0)), colorTransferFunction->GetGreenValue(contourExtractor->GetValue(0)), colorTransferFunction->GetBlueValue(contourExtractor->GetValue(0)));
    contour2->GetProperty()->SetOpacity(opacityTransferFunction->GetValue(contourExtractor->GetValue(0)));*/

    // An outline provides context around the data.
    vtkOutlineFilter* outlineData = vtkOutlineFilter::New();
    outlineData->SetInputConnection(reader->GetOutputPort());
    vtkPolyDataMapper* mapOutline = vtkPolyDataMapper::New();
    mapOutline->SetInputConnection(outlineData->GetOutputPort());
    vtkActor* outline = vtkActor::New();
    outline->SetMapper(mapOutline);
    outline->GetProperty()->SetColor(0, 0, 0);

    // It is convenient to create an initial view of the data. The FocalPoint
    // and Position form a vector direction. Later on (ResetCamera() method)
    // this vector is used to position the camera to look at the data in
    // this direction.
    vtkCamera* aCamera = vtkCamera::New();
    aCamera->SetViewUp(0, 0, -1);
    aCamera->SetPosition(0, 1, 0);
    aCamera->SetFocalPoint(0, 0, 0);
    aCamera->ComputeViewPlaneNormal();

    // Actors are added to the renderer. An initial camera view is created.
    // The Dolly() method moves the camera towards the FocalPoint,
    // thereby enlarging the image.
    aRenderer->AddActor(outline);
    aRenderer->AddActor(contour);
    aRenderer->AddActor(contour2);
    aRenderer->SetActiveCamera(aCamera);
    aRenderer->ResetCamera();
    aCamera->Dolly(1.5);

    // Set a background color for the renderer and set the size of the
    // render window (expressed in pixels).
    aRenderer->SetBackground(1, 1, 1);
    renWin->SetSize(800, 600);

    // Note that when camera movement occurs (as it does in the Dolly()
    // method), the clipping planes often need adjusting. Clipping planes
    // consist of two planes: near and far along the view direction. The 
    // near plane clips out objects in front of the plane; the far plane
    // clips out objects behind the plane. This way only what is drawn
    // between the planes is actually rendered.
    aRenderer->ResetCameraClippingRange();

    /*vtkScalarBarWidget* scalarWidget = vtkScalarBarWidget::New();
    scalarWidget->SetInteractor(iren);
    scalarWidget->GetScalarBarActor()->SetTitle("Transfer Function");
    scalarWidget->GetScalarBarActor()->SetLookupTable(colorTransferFunction);*/

    renWin->Render();
    //scalarWidget->EnabledOn();

    // It is important to delete all objects created previously to prevent
    // memory leaks. In this case, since the program is on its way to
    // exiting, it is not so important. But in applications it is
    // essential.
    //reader->Delete();
    contourExtractor->Delete();
    contourNormals->Delete();
    contourMapper->Delete();
    contour->Delete();
    outlineData->Delete();
    mapOutline->Delete();
    outline->Delete();
    aCamera->Delete();
    //iren->Delete();
    //renWin->Delete();
    //aRenderer->Delete();
}

void rayMarching(vtkRenderer* aRenderer, vtkRenderWindow* renWin, vtkRenderWindowInteractor* iren)
{
    vtkVolume16Reader* reader = vtkVolume16Reader::New();
    reader->SetDataDimensions(256, 256);
    reader->SetImageRange(1, 109);
    reader->SetDataByteOrderToLittleEndian();
    reader->SetFilePrefix("../data/MRbrain");
    reader->SetDataSpacing(1, 1, 2);

    // This part creates the colorMap function to the volume rendering.
    vtkPiecewiseFunction* opacityTransferFunction = vtkPiecewiseFunction::New();
    opacityTransferFunction->AddPoint(20, 0.0);
    opacityTransferFunction->AddPoint(495, 0.0);
    opacityTransferFunction->AddPoint(500, 0.3);
    opacityTransferFunction->AddPoint(1150, 0.5);
    opacityTransferFunction->AddPoint(1500, opacity5);

    vtkColorTransferFunction* colorTransferFunction = vtkColorTransferFunction::New();
    colorTransferFunction->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
    colorTransferFunction->AddRGBPoint(500.0, 1.0, 0.6, 0.0);
    colorTransferFunction->AddRGBPoint(700.0, 1.0, 0.6, 0.0);
    colorTransferFunction->AddRGBPoint(800.0, 1.0, 0.0, 0.0);
    colorTransferFunction->AddRGBPoint(color5, 0.9, 0.9, 0.9);

    // The property describes how the data will look
    vtkVolumeProperty* volumeProperty = vtkVolumeProperty::New();
    volumeProperty->SetColor(colorTransferFunction);
    volumeProperty->SetScalarOpacity(opacityTransferFunction);
    volumeProperty->ShadeOn();
    volumeProperty->SetInterpolationTypeToLinear();

    vtkFixedPointVolumeRayCastMapper* volumeMapper = vtkFixedPointVolumeRayCastMapper::New();
    volumeMapper->SetInputConnection(reader->GetOutputPort());

    // Set the sampling rate
    volumeMapper->SetSampleDistance(rayStepSize);

    // The volume holds the mapper and the property and
    // can be used to position/orient the volume
    vtkVolume* volume = vtkVolume::New();
    volume->SetMapper(volumeMapper);
    volume->SetProperty(volumeProperty);

    // An outline provides context around the data.
    vtkOutlineFilter* outlineData = vtkOutlineFilter::New();
    outlineData->SetInputData((vtkDataSet*)reader->GetOutput());
    vtkPolyDataMapper* mapOutline = vtkPolyDataMapper::New();
    mapOutline->SetInputConnection(outlineData->GetOutputPort());
    vtkActor* outline = vtkActor::New();
    outline->SetMapper(mapOutline);
    outline->GetProperty()->SetColor(0, 0, 0);

    // It is convenient to create an initial view of the data. The FocalPoint
    // and Position form a vector direction. Later on (ResetCamera() method)
    // this vector is used to position the camera to look at the data in
    // this direction.
    vtkCamera* aCamera = vtkCamera::New();
    aCamera->SetViewUp(0, 0, -1);
    aCamera->SetPosition(0, 1, 0);
    aCamera->SetFocalPoint(0, 0, 0);
    aCamera->ComputeViewPlaneNormal();

    // Actors are added to the renderer. An initial camera view is created.
    // The Dolly() method moves the camera towards the FocalPoint,
    // thereby enlarging the image.
    aRenderer->AddActor(outline);
    aRenderer->AddVolume(volume);
    aRenderer->SetActiveCamera(aCamera);
    aRenderer->ResetCamera();
    aCamera->Dolly(1.5);

    // Set a background color for the renderer and set the size of the
    // render window (expressed in pixels).
    aRenderer->SetBackground(1, 1, 1);
    renWin->SetSize(800, 600);

    // Note that when camera movement occurs (as it does in the Dolly()
    // method), the clipping planes often need adjusting. Clipping planes
    // consist of two planes: near and far along the view direction. The 
    // near plane clips out objects in front of the plane; the far plane
    // clips out objects behind the plane. This way only what is drawn
    // between the planes is actually rendered.
    aRenderer->ResetCameraClippingRange();


    vtkScalarBarWidget* scalarWidget = vtkScalarBarWidget::New();
    scalarWidget->SetInteractor(iren);
    scalarWidget->GetScalarBarActor()->SetTitle("Transfer Function");
    scalarWidget->GetScalarBarActor()->SetLookupTable(colorTransferFunction);


    // Initialize the event loop and then start it.
    //iren->Initialize();
    //renWin->SetWindowName("Simple Volume Renderer");
    renWin->Render();
    scalarWidget->EnabledOn();
    //iren->Start();
}

class KeyInterpreter : public vtkCommand
{
public:
    // Allocator
    static KeyInterpreter* New(void) { return new KeyInterpreter; }

    // Internal data.  This needs to be set immediately after allocation.
    vtkFixedPointVolumeRayCastMapper* map;
    vtkRenderer* aRenderer;
    vtkRenderWindow* renWin;
    vtkRenderWindowInteractor* iren;

    // Called upon the registered event (i.e., a key press)
    void Execute(vtkObject* caller, unsigned long eventId, void* callData)
    {
        double dist;
        vtkRenderWindowInteractor* interactor =
            reinterpret_cast<vtkRenderWindowInteractor*>(caller);

        switch (interactor->GetKeyCode())
        {
        case 't':
            dist = map->GetSampleDistance();
            aRenderer->RemoveAllViewProps();
            rayMarching(aRenderer, renWin, iren);
            map->SetSampleDistance(dist);
            break;

        case ']':
            dist = map->GetSampleDistance();

            // Do something here
            iso1 += 100;
            iso2 += 100;
            cout << "  iso1 = " << iso1;
            cout << "  iso2 = " << iso2;
            aRenderer->RemoveAllViewProps();
            IsoSurface(aRenderer, renWin, iren);

            map->SetSampleDistance(dist);
            break;

        case '[':
            dist = map->GetSampleDistance();

            // Do something here
            iso1 -= 100;
            iso2 -= 100;
            cout << "  iso1 = " << iso1;
            cout << "  iso2 = " << iso2;
            aRenderer->RemoveAllViewProps();
            IsoSurface(aRenderer, renWin, iren);

            map->SetSampleDistance(dist);
            break;

        case 'T':
            dist = map->GetSampleDistance();
            rayMarchingMode = false;
            aRenderer->RemoveAllViewProps();
            IsoSurface(aRenderer, renWin, iren);
            map->SetSampleDistance(dist);
            break;

        case '>':
            dist = map->GetSampleDistance();

            // Do something here
            cout << "  rayStepSize = " << rayStepSize;
            rayStepSize += 10;
            aRenderer->RemoveAllViewProps();
            rayMarching(aRenderer, renWin, iren);
            map->SetSampleDistance(dist);
            break;

        case '<':
            dist = map->GetSampleDistance();

            // Do something here
            cout << "  rayStepSize = " << rayStepSize;
            rayStepSize -= 10;
            aRenderer->RemoveAllViewProps();
            rayMarching(aRenderer, renWin, iren);
            map->SetSampleDistance(dist);
            break;
        case '+':
            dist = map->GetSampleDistance();

            // Do something here
            color5 += 50.0;
            aRenderer->RemoveAllViewProps();
            rayMarching(aRenderer, renWin, iren);
            map->SetSampleDistance(dist);
            break;
        case '-':
            dist = map->GetSampleDistance();

            // Do something here
            color5 -= 50.0;
            aRenderer->RemoveAllViewProps();
            rayMarching(aRenderer, renWin, iren);
            map->SetSampleDistance(dist);
            break;
        case 'p':
            dist = map->GetSampleDistance();

            // Do something here
            opacity5 += 0.1;
            aRenderer->RemoveAllViewProps();
            rayMarching(aRenderer, renWin, iren);
            map->SetSampleDistance(dist);
            break;
        case 'P':
            dist = map->GetSampleDistance();

            // Do something here
            opacity5 -= 0.1;
            aRenderer->RemoveAllViewProps();
            rayMarching(aRenderer, renWin, iren);
            map->SetSampleDistance(dist);
            break;
        }
        interactor->GetRenderWindow()->Render();
    }
};

int main(int argc, char** argv)
{
    // Create the renderer, the render window, and the interactor. The renderer
    // draws into the render window, the interactor enables mouse- and 
    // keyboard-based interaction with the data within the render window.
    vtkRenderer* aRenderer = vtkRenderer::New();
    vtkRenderWindow* renWin = vtkRenderWindow::New();
    vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();

    renWin->AddRenderer(aRenderer);
    iren->SetRenderWindow(renWin);

    vtkVolume16Reader* reader = vtkVolume16Reader::New();
    reader->SetDataDimensions(256, 256);
    reader->SetImageRange(1, 109);
    reader->SetDataByteOrderToLittleEndian();
    reader->SetFilePrefix("../data/MRbrain");
    reader->SetDataSpacing(1, 1, 2);

    vtkFixedPointVolumeRayCastMapper* volumeMapper = vtkFixedPointVolumeRayCastMapper::New();
    volumeMapper->SetInputConnection(reader->GetOutputPort());

    if (!rayMarchingMode)
        IsoSurface(aRenderer, renWin, iren);
    else
        rayMarching(aRenderer, renWin, iren);

    iren->Initialize();

    // To register the keyboard callback
    KeyInterpreter* key = KeyInterpreter::New();
    key->map = volumeMapper;
    key->renWin = renWin;
    key->iren = iren;
    key->aRenderer = aRenderer;
    iren->AddObserver(vtkCommand::KeyPressEvent, key);
    renWin->SetWindowName("Simple Volume Renderer");
    renWin->Render();
    iren->Start();
    return 0;
}

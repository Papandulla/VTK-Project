#include "vtkRenderer.h"
#include "vtkPointSet.h"
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
//#include "vtkVolumeRayCastCompositeFunction.h"
#include "vtkFixedPointVolumeRayCastMapper.h"
//#include "vtkVolumeRayCastMIPFunction.h"
#include "vtkStructuredPointsReader.h"
#include "vtkThresholdPoints.h"
#include "vtkMaskPoints.h"
#include "vtkConeSource.h"
#include "vtkGlyph3D.h"
#include "vtkVertexGlyphFilter.h"
#include "vtkLookupTable.h"
#include "vtkCommand.h"
#include "vtkHedgeHog.h"
#include "vtkHyperStreamline.h"
#include "vtkPoints.h"
#include "vtkStructuredGrid.h"
#include "vtkStreamTracer.h"
#include "vtkNamedColors.h"
#include "vtkPointload.h"
#include "vtkImageData.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"


int main(int argc, char** argv)
{
	// Create the renderer, the render window, and the interactor. The renderer
	// draws into the render window, the interactor enables mouse- and 
	// keyboard-based interaction with the data within the render window.
	vtkRenderer* aRenderer = vtkRenderer::New();
	vtkRenderWindow* renWin = vtkRenderWindow::New();
	renWin->AddRenderer(aRenderer);
	vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();
	iren->SetRenderWindow(renWin);

	// Read
	vtkStructuredPointsReader* reader = vtkStructuredPointsReader::New();
	reader->SetFileName("../data/carotid.vtk");
	reader->Update();

	// Hedgehog setup.
	vtkHedgeHog *hhog = vtkHedgeHog::New();
	hhog->SetInputConnection( reader->GetOutputPort() );
	hhog->SetScaleFactor( 0.25 );

	//vtkConeSource *cone = vtkConeSource::New();
	//cone->Update();

	// Glyph 3D setup
	//vtkGlyph3D *glyph = vtkGlyph3D::New();
	//glyph->SetInputConnection(reader->GetOutputPort());
	//glyph->SetSourceConnection(cone->GetOutputPort());
	//glyph->SetScaleFactor(2);
	//glyph->SetScaleModeToScaleByVector();

	// Grid parameters
	// carotid
	int xVector = 76;
	int yVector = 49;
	int zVector = 45;

	int maxX = 76;
	int minX = 0;
	int maxY = 49;
	int minY = 0;
	int maxZ = 45;
	int minZ = 0;

	double xSpacing = (maxX - minX) / xVector;
	double ySpacing = (maxY - minY) / yVector;
	double zSpacing = (maxZ - minZ) / zVector;

	// Generate starting points
	vtkPoints* points = vtkPoints::New();
	for (int k = 0; k < zVector; k++) {
		double zCoord = minZ + k * zSpacing;
		for (int j = 0; j < yVector; j++) {
			double yCoord = minY + j * ySpacing;
			for (int i = 0; i < xVector; i++) {
				double xCoord = minX + i * xSpacing;
				points->InsertNextPoint(xCoord, yCoord, zCoord);
			}
		}
	}

	vtkPointSet* startPoint = vtkPointSet::New();
	startPoint->SetPoints(points);

	double origin[3] = { 100.0, 80.0, 1.0 };
	vtkTransform* transform = vtkTransform::New();
	transform->Translate(-origin[0], -origin[1], -origin[2]);

	vtkTransformFilter* transformFilter = vtkTransformFilter::New();
	transformFilter->SetInputConnection(reader->GetOutputPort());
	transformFilter->SetTransform(transform);
	transformFilter->Update();

	vtkStreamTracer* streamLine = vtkStreamTracer::New();
	streamLine->SetInputConnection(transformFilter->GetOutputPort());
	streamLine->SetSourceData(startPoint);
	streamLine->SetIntegrationDirectionToBoth();
	streamLine->SetMaximumPropagation(20.0);
	streamLine->SetInitialIntegrationStep(0.1);
	streamLine->SetIntegrationStepUnit(1.0);
	streamLine->Update();

	// Calculate sampling points along streamlines
	vtkMaskPoints* maskPoints = vtkMaskPoints::New();
	maskPoints->SetInputConnection(streamLine->GetOutputPort());
	maskPoints->SetOnRatio(10); // Adjust the OnRatio value to control the number of sample points along the streamlines

	vtkConeSource* cone = vtkConeSource::New();
	cone->SetHeight(0.1);
	cone->SetRadius(0.05);
	cone->Update();

	// Glyph 3D setup
	vtkGlyph3D* glyph = vtkGlyph3D::New();
	glyph->SetInputConnection(maskPoints->GetOutputPort());
	glyph->SetSourceConnection(cone->GetOutputPort());
	glyph->SetScaleFactor(0.25);
	glyph->SetScaleModeToScaleByVector();

	// Lookup Table
	vtkLookupTable* lut = vtkLookupTable::New();
	lut->SetHueRange(0.667, 0.0);
	lut->Build();

	// Poly Mapper
	vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
	mapper->SetInputConnection(hhog->GetOutputPort());
	mapper->SetScalarRange(0.0, 1.0);
	mapper->SetLookupTable(lut);

	// Actor
	vtkActor* actor = vtkActor::New();
	actor->SetMapper(mapper);
	//actor->VisibilityOn();
	//actor->GetProperty()->SetColor(namedColors->GetColor3d("White").GetData());

	// Actors are added to the renderer. An initial camera view is created.
	// The Dolly() method moves the camera towards the FocalPoint,
	// thereby enlarging the image.
	aRenderer->AddActor(actor);

	// Set a background color for the renderer and set the size of the
	// render window (expressed in pixels).
	aRenderer->SetBackground(0, 0, 0);
	renWin->SetSize(800, 600);

	// Initialize the event loop and then start it.
	iren->Initialize();
	renWin->SetWindowName("Simple Volume Renderer");
	renWin->Render();
	iren->Start();

	return 0;
}


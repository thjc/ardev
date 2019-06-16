/* -- 2007-05-07 --
 *  ardev - an augmented reality library for robot developers
 *  Copyright 2005-2007 - Toby Collett (ardev _at_ plan9.net.nz)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
#include "calibapp.h"

//#include <Q3Canvas>
//#include <Q3CanvasPixmap>
#include <QLayout>
#include <QFileDialog>
//#include <Q3ValueList>
#include <QImage>
#include <QSpinBox>
#include <QTimer>
#include <QLineEdit>
#include <QPainter>
#include <QPicture>
#include <QMessageBox>

#include <Magick++.h>
#include <sys/time.h>

#include <ardev/ardev.h>

#include "hough.h"
#include <libthjc/matrix.h>
#include <libthjc/misc.h>

#include <fstream>

#include "camera.h"

using namespace Magick;

CalibApp::CalibApp(int _argc, char **_argv, QWidget* parent, Qt::WFlags flags) : QMainWindow(parent,flags)
{
	setupUi(this);

	ImageFrame->setWidgetResizable( true );
	ImageFrame->setWidget( outputImage );

	argc=_argc;
	argv=_argv;

	CalibrationDone = false;

	LoadedPoints = false;

	StageBox->setMinimum(0);
	StageBox->setMaximum(NumStages-1);

	CubeOffset[0] = 0;
	CubeOffset[1] = 0;
	CubeOffset[2] = 0;

	LoadCalibPoints();


	Capturing = false;
	AcquireTimer = new QTimer( this );
    connect( AcquireTimer, SIGNAL(timeout()), SLOT(Idle()) );

	PClient = NULL;
	Pos = NULL;
	cp = NULL;
	camServer = NULL;

	// Connect up the slots
	QObject::connect(OriginalButton, SIGNAL(clicked()), this, SLOT(OriginalClicked()));
	QObject::connect(Calibrate, SIGNAL(clicked()), this, SLOT(CalibrateClicked()));
	QObject::connect(ProcessButton, SIGNAL(clicked()), this, SLOT(ProcessImageClicked()));
	QObject::connect(StageButton, SIGNAL(clicked()), this, SLOT(ShowStageClicked()));
	QObject::connect(StageBox, SIGNAL(valueChanged(int)), this, SLOT(ShowStageClicked()));

	//QObject::connect(Calibrate, SIGNAL(selectionChanged()), this, SLOT(calibChanged()));

	QObject::connect(AcquireButton, SIGNAL(toggled(bool)), this, SLOT(AcquireToggled(bool)));
	QObject::connect(PlaceButton, SIGNAL(toggled(bool)), this, SLOT(PlacingToggled(bool)));

	QObject::connect(fileOpenAction, SIGNAL(triggered()), this, SLOT(fileOpen()));
	QObject::connect(fileSaveAction, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
	QObject::connect(fileSaveAsAction, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
	QObject::connect(fileLoad_2D_PointsAction, SIGNAL(triggered()), this, SLOT(Load2DPoints()));
	QObject::connect(fileSave_2D_PointsAction, SIGNAL(triggered()), this, SLOT(Save2DPoints()));
	QObject::connect(fileLoad_3D_PointsAction, SIGNAL(triggered()), this, SLOT(Load3DPoints()));

	QObject::connect(outputImage, SIGNAL(clicked(int,int)), this, SLOT(ImageClicked(int,int)));
	QObject::connect(PointBox, SIGNAL(valueChanged(int)), this, SLOT(PointChanged()));
}

void CalibApp::ShowStageClicked()
{
	int Stage = StageBox->value();
	if (Stage >= 0 && Stage < NumStages)
		ShowImage(Stages[Stage]);
}

void CalibApp::OriginalClicked()
{
	ShowImage(Original);
}

double RealPoints[27][3] = {
	{0.5,0.981,0.5},
	{0.8,0.981,0.8},
	{0.5,0.981,0.8},
	{0.2,0.981,0.8},
	{0.2,0.981,0.5},
	{0.2,0.981,0.2},
	{0.5,0.981,0.2},
	{0.8,0.981,0.2},
	{0.8,0.981,0.5},

	{0.981,0.5,0.5},
	{0.981,0.8,0.8},
	{0.981,0.8,0.5},
	{0.981,0.8,0.2},
	{0.981,0.5,0.2},
	{0.981,0.2,0.2},
	{0.981,0.2,0.5},
	{0.981,0.2,0.8},
	{0.981,0.5,0.8},

	{0.5,0.5,0.019},
	{0.8,0.8,0.019},
	{0.5,0.8,0.019},
	{0.2,0.8,0.019},
	{0.2,0.5,0.019},
	{0.2,0.2,0.019},
	{0.5,0.2,0.019},
	{0.8,0.2,0.019},
	{0.8,0.5,0.019}
	 };

void CalibApp::LoadCalibPoints()
{
	printf("preload default Points file for calibration\n");
	memcpy(CubePoints, RealPoints, sizeof(double)*27*3);

	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i],"--offset") == 0 && argc > i)
		{
			std::ifstream fin;
			// First load the cube offset if specified
			if (argc > 1)
			{
				fin.open(argv[++i]);
				if (fin.is_open())
				{
					fin >> CubeOffset[0] >> CubeOffset[1] >> CubeOffset[2];
					fin.close();
				}
				else
				{
					printf("Unable to open offset file: %s\n",argv[i]);
				}

			}
		}
		else if (strcmp(argv[i],"--points") == 0 && argc > i)
		{
			std::ifstream fin;
			fin.open(argv[++i]);
			if (fin.is_open())
			{
				for (int i = 0 ; i < 27; i++)
					fin >> CubePoints[i][0] >> CubePoints[i][1] >> CubePoints[i][2];
				fin.close();
			}
			else
			{
				printf("Unable to open points file: %s\n",argv[i]);
			}
		}
		else
		{
			printf("Usage: --offset <cubeoffset file> --points <3d points file> ");
			exit(0);
		}
	}

	PointChanged();
}

void CalibApp::CalibrateClicked()
{
	CalibrationDone = false;
	OriginalClicked();
	if ( Original.data )
		PutDotsOnOriginal(OrderedPoints,Original);
//	LinesClicked();

	printf("Starting Calibration\n");

	list<CalibrationPair> CalibPoints;
	CalibrationPair Temp;

	if ( OrderedPoints.size() < 27 )
	{
		QMessageBox::warning(this, "No 2D Points", "No 2D points loaded or placed. Aborting calibration");
		return;
	}

	vector<Point2D>::iterator itr = OrderedPoints.begin();

	for (int i = 0; i < 27; ++ i)
	{
		Temp.x = itr->x;
		Temp.y = itr->y;
		Temp.X = CubePoints[i][0] + CubeOffset[0];
		Temp.Y = CubePoints[i][1] + CubeOffset[1];
		Temp.Z = CubePoints[i][2] + CubeOffset[2];
		printf("%02d: %f %f %f %f %f\n",i,Temp.x,Temp.y,Temp.X,Temp.Y,Temp.Z);
		CalibPoints.push_back(Temp);
		itr++;
	}

	//PrintList("CalibPoints",CalibPoints);

	cam.width = Original.x_size;
	cam.height = Original.y_size;

	cam.Ncx = cam.width;
	cam.Nfx = cam.width;

	cam.dx = cam.dy = 0.0000078347;//0.03175;
	cam.Cx = cam.width/2;
	cam.Cy = cam.height/2;
	try
	{
		cam.Calibrate(CalibPoints);
	}
	catch (const char * e)
	{
		printf("Calibration Exception: %s\n",e);
		return;
	}

	this->posX->setText(QString().setNum(cam.GetTx()));
	this->posY->setText(QString().setNum(cam.GetTy()));
	this->posZ->setText(QString().setNum(cam.GetTz()));

	this->rotX->setText(QString().setNum(cam.GetRx()));
	this->rotY->setText(QString().setNum(cam.GetRy()));
	this->rotZ->setText(QString().setNum(cam.GetRz()));

	this->FocalDistance->setText(QString().setNum(cam.GetFocalDistance()));

	if ( outputImage )
	{
		outputImage->setPoints(cam.GetProjectedPoints());
		//outputImage->repaint();
	}

	//PrintList("Projected Points", ProjPoints);
	printf("Focal Length = %f\n",cam.f);

	printf("Errors: \n");
	cout << "2D=" << cam.GetErrMatrix() << "3D="  << cam.Get3DErrMatrix() << "Averages 2d: " << cam.GetErrMatrix().Avg() << " 3d: " << cam.Get3DErrMatrix().Avg();;

	printf("Calibration Done\n");
	CalibrationDone = true;
}


// if the user manually changes the calibration paramteres
void CalibApp::calibChanged()
{
	if (!CalibrationDone)
		return;
	OriginalClicked();
	PutDotsOnOriginal(OrderedPoints,Original);
//	LinesClicked();

	cam.SetTx(posX->text().toDouble());
	cam.SetTy(posY->text().toDouble());
	cam.SetTz(posZ->text().toDouble());
	cam.SetRx(rotX->text().toDouble());
	cam.SetRy(rotY->text().toDouble());
	cam.SetRz(rotZ->text().toDouble());
	cam.SetFocalDistance(FocalDistance->text().toDouble());

	outputImage->setPoints(cam.GetProjectedPoints());
	//outputImage->repaint();

	//PrintList("Projected Points", ProjPoints);
	printf("Focal Length = %f\n",cam.f);

	printf("Errors: \n");
	cout << "2D=" << cam.GetErrMatrix() << "3D="  << cam.Get3DErrMatrix() << "Averages 2d: " << cam.GetErrMatrix().Avg() << " 3d: " << cam.Get3DErrMatrix().Avg();;

	printf("Calibration Done\n");
}

void CalibApp::ShowImage(const ARImage & im)
{
	QImage Temp(im.x_size,im.y_size,QImage::Format_RGB32);
	if (im.ByteDepth == 3)
	{
		//printf("32 bit Image\n");
		for (unsigned int x = 0; x < im.x_size; x++)
		{
			for (unsigned int y = 0; y < im.y_size; y++)
			{
				if (im.ColourFormat == GL_RGB)
					Temp.setPixel(x,y,qRgb(im.GetPixel(x*3,y),im.GetPixel(x*3+1,y),im.GetPixel(x*3+2,y)));
				else if (im.ColourFormat == GL_BGR)
					Temp.setPixel(x,y,qRgb(im.GetPixel(x*3+2,y),im.GetPixel(x*3+1,y),im.GetPixel(x*3,y)));
				else
					printf("Unsupported colour format\n");

			}
		}
	}
	else if (im.ByteDepth == 1)
	{
		//printf("8 bit Image\n");
		for (unsigned int x = 0; x < im.x_size; x++)
		{
			for (unsigned int y = 0; y < im.y_size; y++)
			{
				Temp.setPixel(x,y,qRgb(im.GetPixel(x,y),im.GetPixel(x,y),im.GetPixel(x,y)));
			}
		}
	}

	outputImage->setImage(Temp);
	outputImage->clearPoints();
	//outputImage->repaint();
}

void CalibApp::AcquireToggled(bool State)
{
	if (State)
	{
		// Create player server connection for FOB
#ifdef FOB
		if (PClient == NULL)
		{
			PClient = new PlayerClient("localhost",6665);
			if (PClient == NULL)
			{
				printf("Unable to connect to player server\n");
				exit(1);
			}
			PClient->SetDataMode(PLAYER_DATAMODE_PULL_NEW);
			PClient->SetReplaceRule(-1,-1,PLAYER_MSGTYPE_DATA,-1,1);
			Pos = new Position3dProxy(PClient,0);
			if (Pos == NULL)
			{
				printf("Unable to create player proxy\n");
				exit(1);
			}
		}
#endif
		// Create player server connection for Camera
		if (camServer == NULL)
		{
			try
			{
				QByteArray vidServText = VideoServer->text().toAscii(); // Careful of lifetime
				camServer = new PlayerClient(static_cast<const char *> (vidServText),6665);
			}
			catch(const PlayerError& x)
			{
				camServer = NULL;
			}
			if (camServer == NULL)
			{
				QMessageBox::warning(this, "Connection Failed", "Unable to connect to video server.");
				AcquireButton->setChecked(false);
				return;
			}
			camServer->SetDataMode(PLAYER_DATAMODE_PULL);
			camServer->SetReplaceRule(true);
			try
			{
				cp = new CameraProxy(camServer,cameraIndex->text().toInt());
			}
			catch(const PlayerError& x)
			{
				printf("%s: %s\n", x.GetErrorFun().c_str(), x.GetErrorStr().c_str());
				camServer = NULL;
				return;
			}
			if (cp == NULL)
			{
				printf("Unable to create player proxy\n");
				delete camServer;
				camServer = NULL;
				return;
			}
		}

		AcquireTimer->start( 0 );
	}
	else
	{
#ifdef FOB
		if (PClient)
		{
			delete PClient;
			PClient = NULL;
		}
#endif

		if (camServer)
		{
			AcquireTimer->stop();
			printf("Acquired Image for calibration\n");
			delete cp;
			cp = NULL;
			delete camServer;
			camServer = NULL;
			//reverse color order
			Original = Acquired;
			Original.ColourFormat = GL_RGB;
			Original_Backup = Original;
			PutDotsOnOriginal(OrderedPoints,Original);
			OriginalClicked();
		}
	}

}

void CalibApp::PlacingToggled(bool State)
{
	if (State && !LoadedPoints)
		QMessageBox::warning(this, "No 3D Points", "No 3D points file has been loaded. Using default points.");
	// Make sure there are enough points.
	while (OrderedPoints.size() < 27)
		OrderedPoints.push_back(Point2D(0, 0));
}

void CalibApp::fileOpen()
{
	QString FileName = QFileDialog::getOpenFileName(this, "Image to calibrate from","./","Images (*.jpeg *.jpg *.png)");
	if (FileName == "")
		return;

	Image im;
	QByteArray asciiFilename = FileName.toAscii(); // Careful of lifetime
	printf("Opening file %s\n",static_cast<const char *> (asciiFilename));
	im.read(static_cast<const char *>(asciiFilename));
	Original.y_size = im.rows();
	Original.x_size = im.columns();
	Original.ByteDepth = 3;
	Original.ColourFormat = GL_RGB;
	Original.Allocate();
	im.write(0,0,im.columns(),im.rows(), "RGB", CharPixel, Original.data);
	Original_Backup = Original;

	OriginalClicked();
}

void CalibApp::Load2DPoints()
{
	if (Original.y_size ==0)
		return;
	QString FileName = QFileDialog::getOpenFileName(this,"2D Points File","./","*");

	std::ifstream fin;
	QByteArray asciiFile = FileName.toAscii(); // Careful of lifetime
	fin.open(asciiFile);
	if (fin.is_open())
	{
		printf("Loading 2d Points\n");
		Original = Original_Backup;
		vector<Point2D> Temp;
		OrderedPoints.clear();
		double x, y;
		for (int i = 0 ; i < 9; i++)
		{
			fin >> x >> y;
			printf("%f %f\n",x,y);
			Temp.push_back(Point2D(x,y));
		}
		OrderedPoints.insert(OrderedPoints.end(),Temp.begin(),Temp.end());

		Temp.clear();
		for (int i = 0 ; i < 9; i++)
		{
			fin >> x >> y;
			printf("%f %f\n",x,y);
			Temp.push_back(Point2D(x,y));
		}
		OrderedPoints.insert(OrderedPoints.end(),Temp.begin(),Temp.end());

		Temp.clear();
		for (int i = 0 ; i < 9; i++)
		{
			fin >> x >> y;
			printf("%f %f\n",x,y);
			Temp.push_back(Point2D(x,y));
		}
		OrderedPoints.insert(OrderedPoints.end(),Temp.begin(),Temp.end());
		printf("Done\n");
		if ( Original.data )
			PutDotsOnOriginal(OrderedPoints,Original);
		OriginalClicked();
		fin.close();
	}

}

void CalibApp::Save2DPoints()
{
	if (OrderedPoints.empty())
	{
		QMessageBox::warning(this, "No Points", "No 2D points to save.");
		return;
	}

	QString FileName = QFileDialog::getSaveFileName(this,"Save 2D Points to","./","Points File");

	std::ofstream fout;
	QByteArray asciiFile = FileName.toAscii(); // Careful of lifetime
	fout.open(asciiFile);
	if (fout.is_open())
	{
		printf("Saving 2d Points\n");
		for (unsigned i = 0; i < OrderedPoints.size(); i++)
			fout << OrderedPoints[i].x << ' ' << OrderedPoints[i].y << '\n';

		fout.close();
	}
}

void CalibApp::Load3DPoints()
{
	QString FileName = QFileDialog::getOpenFileName(this,"3D Points File","./","*");

	std::ifstream fin;
	QByteArray asciiFile = FileName.toAscii(); // Careful of lifetime
	fin.open(asciiFile);
	if (fin.is_open())
	{
		for (int i = 0 ; i < 27; i++)
			fin >> CubePoints[i][0] >> CubePoints[i][1] >> CubePoints[i][2];
		fin.close();
	}

	PointChanged();

	LoadedPoints = true;
}


void CalibApp::ProcessImageClicked()
{
	if (!Original.data)
	{
		QMessageBox::warning(this, "No Image", "No image has been acquired. Please open or acquire an image before processing.");
		return;
	}

	if (!LoadedPoints)
		QMessageBox::warning(this, "No 3D Points", "No 3D points file has been loaded. Using default points.");

	Image im, imG, imB;
	im.read(Original.x_size,Original.y_size, "RGB", CharPixel, Original.data);
	for (int i = 0; i < NumStages; ++i)
	{
		Stages[i].x_size = Original.x_size;
		Stages[i].y_size = Original.y_size;
		Stages[i].ByteDepth = 1;
		Stages[i].Allocate();
	}

	printf("Stage 0: Grey Scale\n");
	doGray(Original,Stages[0]);

	printf("Stage 1: Segment\n");
	list<vector<Point2D> > Segments = doLabelSegmentsGray(3,Stages[0],Stages[1]);
	im.read(Stages[1].x_size,Stages[1].y_size,"R",CharPixel,Stages[1].data);
	im.normalize();
	im.write(0,0,im.columns(),im.rows(), "R", CharPixel, Stages[1].data);

	printf("Stage 2-4: Get best Segments for cube\n");
	vector<Point2D> Segment1 = getBestRect(Stages[0],Segments,Stages[2]);
	vector<Point2D> Segment2 = getBestRect(Stages[0],Segments,Stages[3]);
	vector<Point2D> Segment3 = getBestRect(Stages[0],Segments,Stages[4]);

	int Left,Right,Up;
	getLRU(Left,Right,Up,Segment1,Segment2,Segment3);

	printf("Stage 5: Segment Again -> Left\n");
	list<vector<Point2D> > PointSegmentsL = doLabelSegmentsGray(1,Stages[1+Left],Stages[5]);
	im.read(Stages[5].x_size,Stages[5].y_size,"R",CharPixel,Stages[5].data);
	im.normalize();
	im.write(0,0,im.columns(),im.rows(), "R", CharPixel, Stages[5].data);

	printf("Stage 6: Segment Again -> Right\n");
	list<vector<Point2D> > PointSegmentsR = doLabelSegmentsGray(1,Stages[1+Right],Stages[6]);
	im.read(Stages[6].x_size,Stages[6].y_size,"R",CharPixel,Stages[6].data);
	im.normalize();
	im.write(0,0,im.columns(),im.rows(), "R", CharPixel, Stages[6].data);

	printf("Stage 7: Segment Again -> Down\n");
	list<vector<Point2D> > PointSegmentsD = doLabelSegmentsGray(1,Stages[1+Up],Stages[7]);
	im.read(Stages[7].x_size,Stages[7].y_size,"R",CharPixel,Stages[7].data);
	im.normalize();
	im.write(0,0,im.columns(),im.rows(), "R", CharPixel, Stages[7].data);

	printf("Get the Dots\n");
	vector<Point2D> UnorderedLeftDots = getNineDotCenters(PointSegmentsL);
	vector<Point2D> UnorderedRightDots = getNineDotCenters(PointSegmentsR);
	vector<Point2D> UnorderedDownDots = getNineDotCenters(PointSegmentsD);

	printf("Order The Dots\n");
	vector<Point2D> LeftDots = orderNineDots(UnorderedLeftDots);
	vector<Point2D> RightDots = orderNineDots(UnorderedRightDots);
	vector<Point2D> DownDots = orderNineDots(UnorderedDownDots);


	// finally stick the points in the real ordered list, L, then R then Up
	OrderedPoints.clear();
	OrderedPoints.insert(OrderedPoints.end(),LeftDots.begin(),LeftDots.end());
	OrderedPoints.insert(OrderedPoints.end(),RightDots.begin(),RightDots.end());
	OrderedPoints.insert(OrderedPoints.end(),DownDots.begin(),DownDots.end());

	printf("Display the dots on the original\n");
	PutDotsOnOriginal(OrderedPoints,Original);

	printf("Done Processing Image\n");

	OriginalClicked();

}


void CalibApp::fileSaveAs()
{
	QString Filename = QFileDialog::getSaveFileName(this,"Save Calibration to","./","Camera Calibration File (*.calib)");
	std::ofstream fout;
	QByteArray asciiFile = Filename.toAscii(); // Careful of lifetime
	fout.open(asciiFile);
	if (fout.is_open())
	{
		// write out trans and rot matricies
		fout << cam.T.GetValue(0,0) << " " << cam.T.GetValue(1,0) << " " << cam.T.GetValue(2,0) << endl << endl;
		fout << cam.R.GetValue(0,0) << " " << cam.R.GetValue(0,1) << " " << cam.R.GetValue(0,2) << endl;
		fout << cam.R.GetValue(1,0) << " " << cam.R.GetValue(1,1) << " " << cam.R.GetValue(1,2) << endl;
		fout << cam.R.GetValue(2,0) << " " << cam.R.GetValue(2,1) << " " << cam.R.GetValue(2,2) << endl << endl;

		// now other misc camera parameters
		fout << cam.f << endl << cam.dx << " " << cam.dy << endl << cam.width << " " << cam.height << endl << endl;
		fout << cam.sx << " " << cam.sy << endl;

		fout << std::endl;
		fout << "File Format:" << std::endl << "Offset from Tracker Position (m X,Y,Z)" << std::endl;
		fout << "Rotation Matrix" << std::endl << "Focal Distance" << std::endl << "Physical sensor size: X,Y" << std::endl;
		fout << "Width Height (pixels)\nScale Factor (X,Y)" << std::endl;

	}
	else
		printf("Unable to open output file: %s\n",static_cast<const char *> (asciiFile));
}


void CalibApp::Idle()
{


//	cp->fresh = false;
	camServer->Read();
//	if (cp->fresh)
	{
		//cp->Decompress();
		Acquired.y_size = cp->GetHeight();
		Acquired.x_size= cp->GetWidth();
		Acquired.ByteDepth = cp->GetDepth()/8;
		unsigned char * data = Acquired.Allocate();
		cp->GetImage(data);
//		memcpy(data, cp->image, Acquired.GetDataSize());
	}
	//Acquired = &Cap->GetFrame();

#ifdef FOB
	static struct timeval Before = {0,0};
	struct timeval Now;
	gettimeofday(&Now,NULL);
	double Elapsed = static_cast<double> (Now.tv_sec-Before.tv_sec)*1000000.0 + Now.tv_usec - Before.tv_usec;
	if (Elapsed > 10000)
	{
		// Update Player Info\n
		PClient->RequestData();
		PClient->Read();
		posX->setText(QString().setNum(Pos->GetXPos()));
		posY->setText(QString().setNum(Pos->GetYPos()));
		posZ->setText(QString().setNum(Pos->GetZPos()));
		rotX->setText(QString().setNum(Pos->GetPitch()));
		rotY->setText(QString().setNum(Pos->GetYaw()));
		rotZ->setText(QString().setNum(Pos->GetRoll()));
		Before = Now;
	}
#endif

	ShowImage(Acquired);
}

void CalibApp::ImageClicked(int x, int y)
{
	if (PlaceButton->isChecked())
	{
		printf("Clicked at pos: %d, %d\n", x, y);
		unsigned point = PointBox->value();
		if (point >= OrderedPoints.size())
			point = 0;
		if (OrderedPoints.size())
		{
			OrderedPoints[point].x = x;
			OrderedPoints[point].y = y;
		}

		if (point == OrderedPoints.size() - 1)
		{
			PointBox->setValue(0);
			PlaceButton->setChecked(false); // Finished placing points (presumably).
		}
		else
			PointBox->setValue(point + 1);

		Original = Original_Backup;
		if (Original.data)
			PutDotsOnOriginal(OrderedPoints,Original);
		OriginalClicked();
	}
}

void CalibApp::PointChanged()
{
	unsigned point = PointBox->value();
	if (point >= 27)
	{
		printf("Invalid point selected: %u", point);
		return;
	}
	PointX->setText(QString::number(CubePoints[point][0]));
	PointY->setText(QString::number(CubePoints[point][1]));
	PointZ->setText(QString::number(CubePoints[point][2]));
}

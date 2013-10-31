#include <Application.h>
#include "GridView.h"
#include "MainWindow.h"
#include "Preferences.h"

MainWindow::MainWindow(const BRect &frame)
 : BWindow(frame,"Lights Off",B_TITLED_WINDOW,B_NOT_ZOOMABLE | B_NOT_RESIZABLE)
{
	gv = new GridView;
	AddChild(gv);
}

MainWindow::~MainWindow(void)
{
}

bool
MainWindow::QuitRequested(void)
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

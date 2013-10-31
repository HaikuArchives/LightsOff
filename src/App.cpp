#include <Application.h>
#include "MainWindow.h"

class MyApp : public BApplication
{
public:
	MyApp(void);
};

MyApp::MyApp(void)
 : BApplication("application/x-vnd.wgp-LightsOff")
{
	MainWindow *win = new MainWindow(BRect(100,100,360,380));
	win->Show();
}

int main(void)
{
	MyApp app;
	app.Run();
	return 0;
}

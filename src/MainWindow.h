#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Window.h>

class GridView;

class MainWindow : public BWindow
{
public:
	MainWindow(const BRect &frame);
	~MainWindow(void);
	bool QuitRequested(void);
	
private:
	GridView *gv;
};

#endif

#include "frmMain.h"
#include "../E-N TabConverter/converter.h"
#include <QFileDialog>
#include <QMessageBox>

frmMain::frmMain(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.statusBar->showMessage("press 'Scan' to start. ");
}

frmMain::~frmMain()
{
	delete setting;
}

void frmMain::onScan() {
	ui.statusBar->showMessage("then select an image. ");

	QFileDialog* fileDialog = new QFileDialog(this, "Select a Image", ".", "Image(.bmp;*.jpg;*.JPG;*.jpeg;*.png;)");
	fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
	fileDialog->setFileMode(QFileDialog::ExistingFile);
	fileDialog->setViewMode(QFileDialog::List);//�ļ�����ϸ����ʽ��ʾ����ʾ�ļ�������С���������ڵ���Ϣ��

																	  //������һ����ʽQFileDialog::List�����ֻ�ǰ��ļ����ļ������б����ʽ��ʾ����
	if (fileDialog->exec() == QDialog::Accepted) {//ע��ʹ�õ���QFileDialog::Accepted����QDialog::Accepted,����QFileDialog::Accept
		QString path = fileDialog->selectedFiles()[0];
		try {
			go(string(path.toLocal8Bit()), ui.ckbCut->isChecked(), 
						[this](string x) {ui.statusBar->showMessage(QString::fromLocal8Bit(x.data())); }, 
						[this](int x) {ui.progressBar->setValue(x);});
		}
		catch (std::runtime_error ex) {
			ui.statusBar->showMessage(ex.what());
		}
	}
}

void frmMain::onSetting() {
	if(!setting) setting = new frmSetting(this);
	setting->show();
}

void frmMain::showAbout() {
	QMessageBox::about(this, "About", "Developed by JamzumSum. \n"
		"Credit: OpenCV");
}
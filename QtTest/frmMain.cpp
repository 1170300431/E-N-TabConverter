#include "frmMain.h"
#include "../E-N TabConverter/converter.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

frmMain::frmMain(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.statusBar->showMessage("press 'Scan' to start. ");
	QSettings s("settings.ini", QSettings::IniFormat);
	s.setIniCodec("UTF8");
	outputDir = s.value("Recognize/OutputDir", ".").toString();
}

frmMain::~frmMain()
{
	delete setting;
}

void frmMain::onScan() {
	ui.statusBar->showMessage("then select an image. ");
	
	QFileDialog* fileDialog = new QFileDialog(this, "Select a Image", outputDir, "Image(.bmp;*.jpg;*.JPG;*.jpeg;*.png;)");
	fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
	fileDialog->setFileMode(QFileDialog::ExistingFile);
	fileDialog->setViewMode(QFileDialog::List);//�ļ�����ϸ����ʽ��ʾ����ʾ�ļ�������С���������ڵ���Ϣ��

																	  //������һ����ʽQFileDialog::List�����ֻ�ǰ��ļ����ļ������б����ʽ��ʾ����
	if (fileDialog->exec() == QDialog::Accepted) {//ע��ʹ�õ���QFileDialog::Accepted����QDialog::Accepted,����QFileDialog::Accept
		QStringList path = fileDialog->selectedFiles();
		vector<string> list(path.size());
		std::transform(path.begin(), path.end(), list.begin(), [](const QString& x) -> string {return string(x.toLocal8Bit()); });
		Converter converter(list);
		converter.setCut(ui.ckbCut->isChecked());
		converter.setOutputDir(string(outputDir.toLocal8Bit()));
		try {
			converter.scan(
				[this](string x) {ui.statusBar->showMessage(QString::fromLocal8Bit(x.data())); },
				[this](int x) {ui.progressBar->setValue(x); }
			);
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
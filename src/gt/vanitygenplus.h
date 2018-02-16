#ifndef VANITYGENPLUS_H
#define VANITYGENPLUS_H

#ifdef _MSC_VER
#include "guiutil.h"
#else
#endif

namespace Ui {
    class VanitygenPlus;
}

class VanitygenPlus : public QWidget
{
    Q_OBJECT

public:
	explicit VanitygenPlus(QWidget *parent = 0);
    ~VanitygenPlus();

public slots:

signals:

private:
    Ui::VanitygenPlus *ui;
	int timerId;
	void timerEvent(QTimerEvent *event);
	unsigned short makeArgcDynamicArgvDynamic();
	unsigned short makeArgcDynamicArgvDynamic2();
	void prepareThread();
	void import_encryptedWallet(QString password, QString privkey);
	unsigned short import_encryptedWallet_execute(QString command);
	void export_encryptedWallet(QString password, QString address);
	unsigned short export_encryptedWallet_execute(QString command, unsigned short index);

private slots:
	void vanitygenButton1_clicked();
	void vanitygenButton2_clicked();
	void vanitygenButton3_clicked();
	void vanitygenButton4_clicked();
	void vanitygenButton5_clicked();
	void vanitygenButton6_clicked();
	void vanitygenButton7_clicked();
	void vanitygenButton14_clicked();
	void vanitygenButton15_clicked();
	void suspendButton8_clicked();
	void terminateButton9_clicked();
	void clearButton10_clicked();
	void addButton11_clicked();
	void removeButton12_clicked();
	void saveButton13_clicked();
	void validateButton16_clicked();
	void importButton17_clicked();
	void exportButton18_clicked();
	void changeCombobox2_currentIndexChanged();
};

#ifndef _MSC_VER
#include "vanitygenplus.moc"
#endif

#endif // VANITYGENPLUS_H
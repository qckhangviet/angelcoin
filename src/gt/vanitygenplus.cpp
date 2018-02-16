#include "vanitygenplus.h"
#include "ui_vanitygenplus.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "util.h"
#include <QDesktopServices>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#else
#include <QUrl>
#endif

//HOW INTEGRATED VANITYGEN IN THIS PORJECT(AngelcoinQt, angelcoinqt.vcxproj)
//
//1.
//I can't access QT controls (except QLabel) from "int __cdecl printf2(...)" or "int __cdecl fprintf2(...)"
//Example of error if try to access the button:
//ASSERTfailure: "Cannot send events to objects owned by a different thread."
//if ( temp.indexOf("FINISH: ") != -1 )
//	uiii->pushButtonCoinControl->setDisabled(false);
//
//2.
//NOT WORK WITH: QPlainTextEdit, QTextEdit, QTextBrowser
//BUT WORK WITH: QLabel, but without Qt::TextSelectableByMouse or Qt::TextSelectableByKeyboard (ui->plaintext_5555->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard));
//
//3.
//AngelcoinQt->Property Pages->C++->General->Additional Include Directories=...;c:\temp\Vanitygenplus
//AngelcoinQt->Property Pages->Linker->General->Additional Library Directories=...;c:\temp\Vanitygenplus\Release\;
//AngelcoinQt->Property Pages->Linker->Input->Additional Dependencies=...;Vanitygenplus.lib
//
//4.
//4.1: to suppress warning: "warning LNK4099: PDB 'vc110.pdb' was not found with 'Vanitygenplus.lib(pthread.obj)' or at 'E:\MyProjects\angelcoin-0.8.6.2-portedMSVC2012\MSVC\Win32\Debug\vc110.pdb'; linking object as if no debug info"
//AngelcoinQt->Property Pages->Linker->Debugging->Generate Debug Info=NO [instead of original "Yes (/DEBUG)"]
//4.2: to suppress warning in DEBUG: "..."
//AngelcoinQt->Property Pages->Linker->Command Line->/NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcmt.lib
//4.3: to suppress note in RELEASE: "lib-vanitygenplus-vanitygen.lib(vanitygen.obj) : MSIL .netmodule or module compiled with /GL found; restarting link with /LTCG; add /LTCG to the link command line to improve linker performance"
//AngelcoinQt->Property Pages->Linker->Command Line->/LTCG
//4.4: to suppress warning: warning LNK4075: ignoring '/EDITANDCONTINUE' due to '/OPT:LBR' specification
//AngelcoinQt->Property Pages->C++->General->Debug Information Format=Program Database (/Zi)
//Vanitygenplus->Property Pages->C++->General->Debug Information Format=Program Database (/Zi)
//
//5.
//To be able to use oclvanityminer (curl library) in AngelcoinQt must:
//a)"vc6curl.sln":
//LIB Debug
//libcurl->Property Pages->C/C++->Code Generation->Runtime Library=replace "Multi-threaded Debug DLL (/MDd)" with "Multi-threaded Debug (/MTd)"
//LIB Release
//libcurl->Property Pages->C/C++->Code Generation->Runtime Library=replace "Multi-threaded DLL (/MD)" with "Multi-threaded (/MT)"
//b)"AngelcoinSolution.sln":
//AngelcoinQt->Property Pages->Linker->Input->Additional Dependencies=...;Wldap32.lib

//DOCs:
//Qt Widgets C++ Classes
//http://doc.qt.io/qt-5/qtwidgets-module.html
//QString Class
//http://doc.qt.io/qt-5/qstring.html
//QPlainTextEdit Class
//http://doc.qt.io/qt-5/qplaintextedit.html
//QLineEdit Class
//http://doc.qt.io/qt-5/qlineedit.html
//QLabel Class
//http://doc.qt.io/qt-5/qlabel.html
//QComboBox Class
//http://doc.qt.io/qt-5/qcombobox.html
//QMessageBox Class
//http://doc.qt.io/qt-5/qmessagebox.html
//Layout Management
//http://doc.qt.io/qt-5/layout.html
//Qt Style Sheets Reference
//http://doc.qt.io/qt-5/stylesheet-reference.html
//Qt5 Tutorial QComboBox - 2017 
//http://www.bogotobogo.com/Qt/Qt5_QComboBox.php
//Qt5 Tutorial QMessageBox with Radio Buttons - 2017 
//http://www.bogotobogo.com/Qt/Qt5_QMessageBox.php

//Example: View some variable in MessageBox
//char procID[10];
//sprintf(procID, "%d", myThreadHandle);
//QString aa=procID;
//QMessageBox::warning(this, aa,aa,QMessageBox::Ok);
//QMessageBox::warning(this, aa,aa, QMessageBox::Ok, QMessageBox::Ok);

//Example: Start file
//boost::filesystem::path pathDebug = GetDataDir() / "debug.log";
//boost::filesystem::path pathDebug = "c:\\temp\\Run vanitygen in cmd.exe\\system.bat";
//if (boost::filesystem::exists(pathDebug))
//    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(pathDebug.string())));

#include "vanitygen.h"		//vanitygen(argcDynamic, argvDynamic);
#include "oclvanitygen.h"	//oclvanitygen(argcDynamic, argvDynamic);
#include "oclvanityminer.h"	//oclvanityminer(argcDynamic, argvDynamic);
#include "keyconv.h"		//keyconv(argcDynamic, argvDynamic);
#include <QFileDialog>		//saveButton13_clicked()
#include "bitcoinrpc.h"		//importButton16_clicked()
extern bool parseCommandLine(std::vector<std::string> &args, const std::string &strCommand);	//importButton16_clicked()
Ui::VanitygenPlus *pointerUI=NULL;
QString summary;
int argcDynamic=0;
char **argvDynamic=0;
unsigned short whichVanitygenplus=0;
HANDLE myThreadHandle = NULL;
DWORD WINAPI myThread(LPVOID lpParameter);
int printf2fprintf2(const char *format, va_list argptr);
unsigned short stopThreadVanitygen=0;
#define MAXAttrLen 20 //the len of each *(argvDynamic + i) must be less than 20 chars
#define LogPatterns "patterns.txt"
#define LogCalculations "calculations.txt"
#define OpenCL 1	//OpenCL=1 enabled; OpenCL=0 disabled. It is necessary to prevent starting error "OpenCL.dll is missing from your computer" on systems withot GPU support!

VanitygenPlus::VanitygenPlus(QWidget *parent) :
    QWidget(parent),
	ui(new Ui::VanitygenPlus)
{
    ui->setupUi(this);
	connect(ui->qpushbutton_l, SIGNAL(clicked()), this, SLOT(vanitygenButton1_clicked()));
	connect(ui->qpushbutton_2, SIGNAL(clicked()), this, SLOT(vanitygenButton2_clicked()));
	connect(ui->qpushbutton_3, SIGNAL(clicked()), this, SLOT(vanitygenButton3_clicked()));
	connect(ui->qpushbutton_4, SIGNAL(clicked()), this, SLOT(vanitygenButton4_clicked()));
	connect(ui->qpushbutton_5, SIGNAL(clicked()), this, SLOT(vanitygenButton5_clicked()));
	connect(ui->qpushbutton_6, SIGNAL(clicked()), this, SLOT(vanitygenButton6_clicked()));
	connect(ui->qpushbutton_7, SIGNAL(clicked()), this, SLOT(vanitygenButton7_clicked()));
	connect(ui->qpushbutton_8, SIGNAL(clicked()), this, SLOT(suspendButton8_clicked()));
	connect(ui->qpushbutton_9, SIGNAL(clicked()), this, SLOT(terminateButton9_clicked()));
	connect(ui->qpushbutton_10, SIGNAL(clicked()), this, SLOT(clearButton10_clicked()));
	connect(ui->qpushbutton_11, SIGNAL(clicked()), this, SLOT(addButton11_clicked()));
	connect(ui->qpushbutton_12, SIGNAL(clicked()), this, SLOT(removeButton12_clicked()));
	connect(ui->qpushbutton_13, SIGNAL(clicked()), this, SLOT(saveButton13_clicked()));
	connect(ui->qpushbutton_14, SIGNAL(clicked()), this, SLOT(vanitygenButton14_clicked()));
	connect(ui->qpushbutton_15, SIGNAL(clicked()), this, SLOT(vanitygenButton15_clicked()));
	connect(ui->qpushbutton_16, SIGNAL(clicked()), this, SLOT(validateButton16_clicked()));
	connect(ui->qpushbutton_17, SIGNAL(clicked()), this, SLOT(importButton17_clicked()));
	connect(ui->qpushbutton_18, SIGNAL(clicked()), this, SLOT(exportButton18_clicked()));
	connect(ui->qcombobox_2, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(changeCombobox2_currentIndexChanged()));
	pointerUI = ui;
	this->timerId = NULL;

	ui->qpushbutton_8->setDisabled(true);
	ui->qpushbutton_9->setDisabled(true);

	//Current List of Available Coins for Address Generation. Vanitygenplus Version 1.50 Plus, released Dec 18, 2017
	//https://github.com/exploitagency/vanitygen-plus#current-list-of-available-coins-for-address-generation
	ui->qcombobox_1->setCurrentIndex(1);
	ui->qcombobox_1->setItemData(0, "User defined parameters", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(1, "Angelcoin,A", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(2, "Angelcoin Testnet,a", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(3, "Bitcoin,1", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(4, "Bitcoin Testnet,m or n", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(5, "Bitcoin SegWit,3", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(6, "42coin,4", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(7, "Asiacoin,A", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(8, "Advanced Internet Block by IOBOND,A", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(9, "Anoncoin,A", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(10, "Arkstone,A", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(11, "Atmos,N", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(12, "Auroracoin,A", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(13, "Axe,X", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(14, "Blackcoin,B", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(15, "BBQcoin,b", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(16, "Bitcoin Dark,R", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(17, "Chococoin,7", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(18, "Cannacoin,C", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(19, "Canadaecoin,C", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(20, "Clamcoin,x", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(21, "Chinacoin,C", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(22, "C-Note,C", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(23, "PayCon,P", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(24, "Crown,1", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(25, "Dash,X", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(26, "DeepOnion,D", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(27, "Digibyte,D", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(28, "Digitalcoin,D", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(29, "Diamond,d", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(30, "Doge Dark Coin,D", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(31, "Dogecoin,D", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(32, "Dopecoin,4", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(33, "Devcoin,1", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(34, "Electronic-Gulden-Foundation,L", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(35, "Emercoin,E", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(36, "Exclusivecoin,E", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(37, "Faircoin2,f", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(38, "FLOZ,F", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(39, "Feathercoin,6 or 7", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(40, "GameCredits,G", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(41, "Gapcoin,G", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(42, "Global Currency Reserve,G", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(43, "GridcoinResearch,R or S", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(44, "Garlicoin,G", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(45, "Groestlcoin,F", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(46, "Guncoin,G or H", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(47, "HamRadiocoin,1", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(48, "HoboNickels(and BottleCaps),E or F", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(49, "HOdlcoin,H", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(50, "Ixcoin,x", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(51, "Jumbucks,j", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(52, "LBRY,b", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(53, "Leafcoin,f", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(54, "Litecoin,L", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(55, "Memorycoin,M", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(56, "Monacoin,M", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(57, "Monetary Unit,7", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(58, "Myriadcoin,M", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(59, "Mazacoin,M", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(60, "NEETCOIN,N", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(61, "Neoscoin,S", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(62, "Gulden,G", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(63, "Namecoin,M or N", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(64, "Novacoin,4", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(65, "Nyancoin,K", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(66, "OK Cash,P", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(67, "Omnicoin,o", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(68, "Piggycoin,p", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(69, "Pinkcoin,2", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(70, "PIVX,D", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(71, "Parkbyte,P", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(72, "Pandacoin,P", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(73, "Potcoin,P", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(74, "Peercoin,P", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(75, "Pesetacoin,K", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(76, "Protoshares,P", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(77, "QTUM,Q", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(78, "Rubycoin,R", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(79, "Reddcoin,R", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(80, "Scamcoin,S", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(81, "Shadowcoin,S", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(82, "Skeincoin,S", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(83, "Spreadcoin,S", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(84, "Startcoin,s", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(85, "Sexcoin,R or S", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(86, "Templecoin,T", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(87, "Unitus,U", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(88, "Unobtanium,u", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(89, "Viacoin,V", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(90, "Vpncoin,V", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(91, "Vertcoin,V", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(92, "Worldcoin Global,W", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(93, "Wankcoin,1", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(94, "Dubstepcoin,D", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(95, "XCurrency,X", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(96, "Primecoin,A", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(97, "Yacoin,Y", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(98, "BitZeny,Z", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(99, "Zoom coin,i", Qt::ToolTipRole);
	ui->qcombobox_1->setItemData(100, "Ziftrcoin,Z", Qt::ToolTipRole);

	//It is not possible to apply stylesheets (in .ul file) to layout items (QLayout, QVBoxLayout, etc.)
	//https://forum.qt.io/topic/11532/stylesheets-control-layout-objects/2
	//So i am using them here:
	ui->QVBoxLayout_2->setSpacing(0);
	ui->QVBoxLayout_2->setMargin(0);
	ui->QVBoxLayout_2->setContentsMargins(0,0,0,0);

	//Load saved previous patterns from file:
	boost::filesystem::path pathRecords = LogPatterns; //GetDataDir() / LogPatterns;
	QFile file(pathRecords.string().c_str());
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QString tmp;
		while (!file.atEnd())
		{
			QByteArray line = file.readLine();
			line.truncate(line.length()-1);
			tmp=line;
			tmp.remove(' ');
			tmp.remove('\n');
			tmp.remove('\r');
			if ( tmp.length() > 0 )
				ui->qcombobox_2->addItem(line);
		}
		file.close();
		ui->qcombobox_2->setCurrentIndex(-1);
	}

	//Deny resizing of Add and Remove buttons:
	ui->qpushbutton_11->setFixedSize(11,11);
	ui->qpushbutton_12->setFixedSize(11,11);
}

VanitygenPlus::~VanitygenPlus()
{
    delete ui;
	this->killTimer(this->timerId);
	this->timerId = NULL;
}

unsigned short VanitygenPlus::makeArgcDynamicArgvDynamic()
{
	//for:
	//vanitygen
	//oclvanitygen
	//oclvanityminer

	//BTC: vanitygen -X 0 1BTC
	//BTC: vanitygen -C BTC
	//ALC: vanitygen -X 23 ALC
	//ALC: vanitygen -C ALC
	//BTC SegWit: vanitygen -F script -C BTC 3Patern
	//BTC SegWit: vanitygen -F script -C BTC -i 3Patern
	//https://github.com/samr7/vanitygen/issues/100
	//https://github.com/exploitagency/vanitygen-plus/issues/30

	//1.
	//delete existing (if any) argvDynamic;	
	if ( argvDynamic )
	{
		for( int i=0; i<argcDynamic; i++ )
			if ( argvDynamic[i] )
			{
				delete [] argvDynamic[i];
				argvDynamic[i] = 0;
			}
		delete [] argvDynamic;
		argvDynamic = 0;
		argcDynamic = 0;		
	}

	//2.
	//set custom QStringList array, containing only not empty elements (argv items)
	QStringList custom;
	QStringList tmp = ui->qlineedit_1->text().split(" ");
	for( int i=0; i < tmp.length(); i++ )
		if ( tmp[i].length() > 0 )
			custom.append(tmp[i]);
	tmp.clear();

	//3.
	//set new argcDynamic
	QString coin = ui->qcombobox_1->currentText();
	if ( coin == "custom" )
	{
		//custom: Patern(P1 P2 ... PN)
		argcDynamic = custom.length() + 1;
	}
	else
	if ( coin == "BTCsw" )
	{
		//BTCsw: vanitygen -F script -C BTC 3Patern
		argcDynamic = 6;
	}
	else
	{
		//ALC: vanitygen -C ALC Patern
		argcDynamic = 4;
	}

	//4.
	//set new argvDynamic
	argvDynamic = new char*[argcDynamic];
	if ( !argvDynamic )
	{
		argvDynamic = 0;
		argcDynamic = 0;
		return 0;
	}
	unsigned int len = 0;
	for( int i=0; i < argcDynamic; i++ )
	{
		//1.Set len for each *(argvDynamic + i)
		len = 0;
		if ( coin == "custom" )
		{
			//custom: Patern(P1 P2 ... PN)
			if ( i == 0 )
				switch (whichVanitygenplus)
				{
					case 1:
						len = 9;//strncpy(*(argvDynamic+i),"vanitygen",10);
						break;
					case 2:
						len = 12;//strncpy(*(argvDynamic+i),"oclvanitygen",13);
						break;
					case 3:
						len = 14;//strncpy(*(argvDynamic+i),"oclvanityminer",15);
						break;
				}
			else
				len = custom[i-1].length();//strcpy(*(argvDynamic+(i)),custom[i-1].toStdString().c_str());
		}
		else
		if ( coin == "BTCsw" )
		{
			//BTCsw: vanitygen -F script -C BTC 3Patern
			switch (i)
			{
				case 0:
					switch (whichVanitygenplus)
					{
						case 1:
							len = 9;//strncpy(*(argvDynamic+i),"vanitygen",10);
							break;
						case 2:
							len = 12;//strncpy(*(argvDynamic+i),"oclvanitygen",13);
							break;
					}
					break;
				case 1:
					len = 2;//strncpy(*(argvDynamic+i),"-F",3);
					break;
				case 2:
					len = 6;//strncpy(*(argvDynamic+i),"script",7);
					break;
				case 3:
					len = 2;//strncpy(*(argvDynamic+i),"-C",3);
					break;
				case 4:
					len = 3;//strncpy(*(argvDynamic+i),"BTC",4);
					break;
				case 5:
					len = ui->qlineedit_1->text().length();
					//*(*(argvDynamic+i)+0)='3';
					//strcpy(*(argvDynamic+i)+1,(const char *)ui->qlineedit_1->text().toStdString().c_str());
					break;
			}
		}
		else
		{
			//ALC: vanitygen -C ALC Patern
			switch (i)
			{
				case 0:
					switch (whichVanitygenplus)
					{
						case 1:
							len = 9;//strncpy(*(argvDynamic+i),"vanitygen",10);
							break;
						case 2:
							len = 12;//strncpy(*(argvDynamic+i),"oclvanitygen",13);
							break;
					}
					break;
				case 1:
					len = 2;//strncpy(*(argvDynamic+i),"-C",3);
					break;
				case 2:
					len = coin.length();//strcpy(*(argvDynamic+i),coin.toStdString().c_str());
					break;
				case 3:
					len = ui->qlineedit_1->text().length();//strcpy(*(argvDynamic+i),(const char *)ui->qlineedit_1->text().toStdString().c_str());
					break;
			}
		}

		//2.Onnly for "vanitygen" and "oclvanitygen":
		//Control check by len of each *(argvDynamic + i):
		//the len must be less than 20 chars
		if ( whichVanitygenplus == 0 || whichVanitygenplus == 1 )
			if ( len > MAXAttrLen )
			{
				delete [] argvDynamic;
				argvDynamic = 0;
				argcDynamic = 0;
				return 0;
			}

		//3.Create each *(argvDynamic + i)
		len = len + 1;//to add '\0';
		*(argvDynamic + i) = new char[len];
		if ( !*(argvDynamic + i) )
		{
			delete [] argvDynamic;
			argvDynamic = 0;
			argcDynamic = 0;
			return 0;
		}

		//4.Set each *(argvDynamic + i)
		if ( coin == "custom" )
		{
			//custom: Patern(P1 P2 ... PN)
			if ( i == 0 )
				switch (whichVanitygenplus)
				{
					case 1:
						strncpy(*(argvDynamic+i),"vanitygen",len);
						break;
					case 2:
						strncpy(*(argvDynamic+i),"oclvanitygen",len);
						break;
					case 3:
						strncpy(*(argvDynamic+i),"oclvanityminer",len);
						break;
				}
			else
				strncpy(*(argvDynamic+(i)),custom[i-1].toStdString().c_str(),len);
		}
		else
		if ( coin == "BTCsw" )
		{
			//BTCsw: vanitygen -F script -C BTC 3Patern
			switch (i)
			{
				case 0:
					switch (whichVanitygenplus)
					{
						case 1:
							strncpy(*(argvDynamic+i),"vanitygen",len);
							break;
						case 2:
							strncpy(*(argvDynamic+i),"oclvanitygen",len);
							break;
					}
					break;
				case 1:
					strncpy(*(argvDynamic+i),"-F",len);
					break;
				case 2:
					strncpy(*(argvDynamic+i),"script",len);
					break;
				case 3:
					strncpy(*(argvDynamic+i),"-C",len);
					break;
				case 4:
					strncpy(*(argvDynamic+i),"BTC",len);
					break;
				case 5:
					strncpy(*(argvDynamic+i),(const char *)ui->qlineedit_1->text().toStdString().c_str(),len);
					break;
			}
		}
		else
		{
			//ALC: vanitygen -C ALC Patern
			switch (i)
			{
				case 0:
					switch (whichVanitygenplus)
					{
						case 1:
							strncpy(*(argvDynamic+i),"vanitygen",len);
							break;
						case 2:
							strncpy(*(argvDynamic+i),"oclvanitygen",len);
							break;
					}
					break;
				case 1:
					strncpy(*(argvDynamic+i),"-C",len);
					break;
				case 2:
					strncpy(*(argvDynamic+i),coin.toStdString().c_str(),len);
					break;
				case 3:
					strncpy(*(argvDynamic+i),(const char *)ui->qlineedit_1->text().toStdString().c_str(),len);
					break;
			}
		}
	}
	return 1;
}

unsigned short VanitygenPlus::makeArgcDynamicArgvDynamic2()
{
	//for:
	//keyconv

	//BTC: keyconv -X 0 -G
	//BTC: keyconv -C BTC -G
	//ALC: keyconv -X 23 -G
	//ALC: keyconv -C ALC -G
	//ALC: keyconv -v -X 23 3o1XCbv4BU6myPgdTZSyoyJ9XxJZUBCUy1xdHtMDjEPqwFqqos7			=>AcYcX9FAVfLh7rMfUnroCKuRrd36ybjDR1
	//ALC: keyconv -v -C ALC 3o1XCbv4BU6myPgdTZSyoyJ9XxJZUBCUy1xdHtMDjEPqwFqqos7		=>AcYcX9FAVfLh7rMfUnroCKuRrd36ybjDR1
	//https://github.com/DeckerSU/keyconv

	//1.
	//delete existing (if any) argvDynamic;	
	if ( argvDynamic )
	{
		for( int i=0; i<argcDynamic; i++ )
			if ( argvDynamic[i] )
			{
				delete [] argvDynamic[i];
				argvDynamic[i] = 0;
			}
		delete [] argvDynamic;
		argvDynamic = 0;
		argcDynamic = 0;		
	}

	//2.
	QStringList custom;
	QString coin = ui->qcombobox_1->currentText();
	if ( coin != "custom" )
	//NO CUSTOM selected
	{
		if ( ui->qlineedit_1->text().length() == 0 )
		//EMPTY QLINEEDIT
		{
			//ALC: keyconv -C ALC -G
			argcDynamic = 4;
			argvDynamic = new char*[argcDynamic];
			if ( !argvDynamic )
			{
				argvDynamic = 0;
				argcDynamic = 0;
				return 0;
			}
			else
			{
				//Create each *(argvDynamic + i)
				for( int i=0; i < argcDynamic; i++ )
				{
					unsigned short len=0;
					switch (i)
					{
						case 0:
							len = 7+1;
							break;
						case 1:
							len = 2+1;
							break;
						case 2:
							len = (coin=="BTCsw"?3+1:coin.length()+1);
							break;
						case 3:
							len = 2+1;
							break;
					}

					*(argvDynamic + i) = new char[len];
					if ( !*(argvDynamic + i) )
					{
						delete [] argvDynamic;
						argvDynamic = 0;
						argcDynamic = 0;
						return 0;
					}
					else
						switch (i)
						{
							case 0:
								strncpy(*(argvDynamic+i),"keyconv",len);
								break;
							case 1:
								strncpy(*(argvDynamic+i),"-C",len);
								break;
							case 2:
								strncpy(*(argvDynamic+i),(coin=="BTCsw"?"BTC":coin.toStdString().c_str()),len);
								break;
							case 3:
								strncpy(*(argvDynamic+i),"-G",len);
								break;
						}
				}
			}
		}
		else
		//NOT EMPTY QLINEEDIT
		{
			//ALC: keyconv -v -C ALC 3o1XCbv4BU6myPgdTZSyoyJ9XxJZUBCUy1xdHtMDjEPqwFqqos7	=>AcYcX9FAVfLh7rMfUnroCKuRrd36ybjDR1
			argcDynamic = 5;
			argvDynamic = new char*[argcDynamic];
			if ( !argvDynamic )
			{
				argvDynamic = 0;
				argcDynamic = 0;
				return 0;
			}
			else
			{
				//Create each *(argvDynamic + i)
				for( int i=0; i < argcDynamic; i++ )
				{
					unsigned short len=0;
					switch (i)
					{
						case 0:
							len = 7+1;
							break;
						case 1:
							len = 2+1;
							break;
						case 2:
							len = 2+1;
							break;
						case 3:
							len = (coin=="BTCsw"?3+1:coin.length()+1);
							break;
						case 4:
							len = ui->qlineedit_1->text().length()+1;
							break;
					}

					*(argvDynamic + i) = new char[len];
					if ( !*(argvDynamic + i) )
					{
						delete [] argvDynamic;
						argvDynamic = 0;
						argcDynamic = 0;
						return 0;
					}
					else
						switch (i)
						{
							case 0:
								strncpy(*(argvDynamic+i),"keyconv",len);
								break;
							case 1:
								strncpy(*(argvDynamic+i),"-v",len);
								break;
							case 2:
								strncpy(*(argvDynamic+i),"-C",len);
								break;
							case 3:
								strncpy(*(argvDynamic+i),(coin=="BTCsw"?"BTC":coin.toStdString().c_str()),len);
								break;
							case 4:
								strncpy(*(argvDynamic+i),(const char *)ui->qlineedit_1->text().toStdString().c_str(),len);
								break;
						}
				}
			}
		}
	}
	else
	//CUSTOM selected
	{
		if ( ui->qlineedit_1->text().length() == 0 )
		//EMPTY QLINEEDIT
			return 0;
		else
		//NOT EMPTY QLINEEDIT
		{
			//custom: keyconv P1 P2 ... PN
			//custom (ALC): keyconv -v -X 23 3o1XCbv4BU6myPgdTZSyoyJ9XxJZUBCUy1xdHtMDjEPqwFqqos7		=>AcYcX9FAVfLh7rMfUnroCKuRrd36ybjDR1
			//set custom QStringList array, containing only not empty elements (argv items)
			QStringList tmp = ui->qlineedit_1->text().split(" ");
			for( int i=0; i < tmp.length(); i++ )
				if ( tmp[i].length() > 0 )
					custom.append(tmp[i]);
			tmp.clear();
			if ( custom.length() == 0 )
				return 0;
			else
			{
				argcDynamic = custom.length() + 1;
				argvDynamic = new char*[argcDynamic];
				if ( !argvDynamic )
				{
					argvDynamic = 0;
					argcDynamic = 0;
					return 0;
				}
				else
				{
					//Create first *(argvDynamic + 0)
					*(argvDynamic + 0) = new char[7+1];
					if ( !*(argvDynamic + 0) )
					{
						delete [] argvDynamic;
						argvDynamic = 0;
						argcDynamic = 0;
						return 0;
					}
					else
						strncpy(*(argvDynamic+0),"keyconv",7+1);
					//Create rest *(argvDynamic + i)
					for( int i=0; i < custom.length(); i++ )
					{
						*(argvDynamic + i+1) = new char[custom[i].length()+1];
						if ( !*(argvDynamic + i+1) )
						{
							delete [] argvDynamic;
							argvDynamic = 0;
							argcDynamic = 0;
							return 0;
						}
						else
							strncpy(*(argvDynamic+i+1),custom[i].toStdString().c_str(),custom[i].length()+1);
					}
				}
			}
		}
	}

	return 1;
}

void VanitygenPlus::prepareThread()
{
	unsigned short ret=0;
	switch (whichVanitygenplus)
	{
		case 1:
			//vanitygen
		case 2:
			//oclvanitygen
		case 3:
			//oclvanityminer
			ret=makeArgcDynamicArgvDynamic();
			break;
		case 4:
			//keyconv
			ret=makeArgcDynamicArgvDynamic2();
			break;
	}

	if ( !ret )
		QMessageBox::warning(this,"Error:","Input data is not correct!",QMessageBox::Ok);
	else
	{
		ui->qpushbutton_l->setDisabled(true);
		ui->qpushbutton_2->setDisabled(true);
		ui->qpushbutton_3->setDisabled(true);
		ui->qpushbutton_4->setDisabled(true);
		ui->qcombobox_1->setDisabled(true);
		ui->qlineedit_1->setDisabled(true);
		ui->qpushbutton_8->setDisabled(false);
		ui->qpushbutton_9->setDisabled(false);
		stopThreadVanitygen=0;
		if ( myThreadHandle )
		{
			CloseHandle(myThreadHandle);
			myThreadHandle = NULL;
		}
		if ( this->timerId )
		{
			killTimer(this->timerId);
			this->timerId = NULL;
		}
		myThreadHandle = CreateThread(NULL, 0, myThread, NULL, 0, NULL);
		this->timerId = startTimer(1000);
	}
}
void VanitygenPlus::timerEvent(QTimerEvent *event)
{
	static unsigned int i=1;
	DWORD ExitCode;
	GetExitCodeThread(myThreadHandle, &ExitCode);
	if ( !ExitCode )
	{
		ui->qpushbutton_l->setDisabled(false);
		ui->qpushbutton_2->setDisabled(false);
		ui->qpushbutton_3->setDisabled(false);
		ui->qpushbutton_4->setDisabled(false);
		ui->qcombobox_1->setDisabled(false);
		ui->qlineedit_1->setDisabled(false);
		ui->qpushbutton_8->setDisabled(true);
		ui->qpushbutton_9->setDisabled(true);
		ui->qpushbutton_8->setText("Pause");
		//remove last new line:
		summary.truncate(summary.lastIndexOf("\n"));
		ui->qplaintextedit_1->appendPlainText(summary);
		ui->qplaintextedit_1->appendHtml("<font size='13' color='black'>COMPLETE #" + QString::number(i++) + "</font>");
		ui->qplaintextedit_1->appendPlainText("");
		ui->qplaintextedit_1->appendPlainText("");
		ui->qplaintextedit_1->ensureCursorVisible();
		ui->qlabel_1->clear();
		ui->qlabel_1->setText("");
		summary.clear();
		if ( myThreadHandle )
		{
			CloseHandle(myThreadHandle);
			myThreadHandle = NULL;
		}
		if ( this->timerId )
		{
			killTimer(this->timerId);
			this->timerId = NULL;
		}
		stopThreadVanitygen=0;
		whichVanitygenplus=0;
	}
}

void VanitygenPlus::vanitygenButton1_clicked()
{
	whichVanitygenplus=1;
	this->prepareThread();
}
void VanitygenPlus::vanitygenButton2_clicked()
{
#if OpenCL == 1
	if ( ui->qlineedit_1->text().length() <= 1 )
		QMessageBox::warning(this,"Error:","Input data must be longer than 1 symbol for this particularly Miner!",QMessageBox::Ok);
	else
	{
		whichVanitygenplus=2;
		this->prepareThread();
	}
#else
	QMessageBox::warning(this,"Error:","This version of Angelcoin is without OpenCL support. Choose another version!",QMessageBox::Ok);
#endif
}
void VanitygenPlus::vanitygenButton3_clicked()
{
#if OpenCL == 1
	if ( ui->qcombobox_1->currentIndex() != 0 )
		QMessageBox::warning(this,"Error:","Only the \"custom\" field must be selected for this particularly Miner!",QMessageBox::Ok);
	else
	{
		whichVanitygenplus=3;
		this->prepareThread();
	}
#else
	QMessageBox::warning(this,"Error:","This version of Angelcoin is without OpenCL support. Install another version!",QMessageBox::Ok);
#endif
}
void VanitygenPlus::vanitygenButton4_clicked()
{
	whichVanitygenplus=4;
	this->prepareThread();
}
void VanitygenPlus::vanitygenButton5_clicked()
{
	//Can't pass arguments with QDesktopServices::openUrl(...)
	QString command="start Miner-coin-CPU-minerd\\start.bat ";
	QString arguments=ui->qlineedit_1->text();
	command.append(arguments);
	system(command.toStdString().c_str());
}
void VanitygenPlus::vanitygenButton6_clicked()
{
	//Can't pass arguments with QDesktopServices::openUrl(...)
	QString command="start Miner-coin-GPU-ccminer\\start.bat ";
	QString arguments=ui->qlineedit_1->text();
	command.append(arguments);
	system(command.toStdString().c_str());
}
void VanitygenPlus::vanitygenButton14_clicked()
{
	//Can't pass arguments with QDesktopServices::openUrl(...)
	QString command="start Miner-coin-GPU-cgminer\\start.bat ";
	QString arguments=ui->qlineedit_1->text();
	command.append(arguments);
	system(command.toStdString().c_str());
}
void VanitygenPlus::vanitygenButton15_clicked()
{
	//Can't pass arguments with QDesktopServices::openUrl(...)
	QString command="start Miner-coin-GPU-bfgminer\\start.bat ";
	QString arguments=ui->qlineedit_1->text();
	command.append(arguments);
	system(command.toStdString().c_str());
}
void VanitygenPlus::vanitygenButton7_clicked()
{
	boost::filesystem::path pathDebug = "Gen-address-WalletGenerator.net\\index.html";
	if (boost::filesystem::exists(pathDebug))
		QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(pathDebug.string())));
	else
		QMessageBox::warning(this, "Error:","All-in-one index.html document file is not found!",QMessageBox::Ok);
}
void VanitygenPlus::suspendButton8_clicked()
{
	if ( ui->qpushbutton_8->text() == "Pause" )
	{
		SuspendThread(myThreadHandle);
		ui->qpushbutton_8->setText("Resume");
	}
	else
	{
		ResumeThread(myThreadHandle);
		ui->qpushbutton_8->setText("Pause");
	}
}
void VanitygenPlus::terminateButton9_clicked()
{
	//The Practical Guide to Multithreading - Part 1
	//https://www.codeguru.com/csharp/csharp/cs_misc/sampleprograms/article.php/c16423/The-Practical-Guide-to-Multithreading--Part-1.htm

	//Can't terminate my thread with "TerminateThread(...)" =>thats why i am using my own extern variable
	//only decrease CPU works from 95% to 55% and after that after while crashing?!?
	//DWORD ExitCode;
	//GetExitCodeThread(myThreadHandle, &ExitCode);
	//TerminateThread(myThreadHandle, ExitCode);
	//CloseHandle(myThreadHandle);

	//Close hole program:
	//#1
	//DWORD ExitCode;
	//GetExitCodeThread(myThreadHandle, &ExitCode);
	//ExitThread(ExitCode);
	//#2
	//DWORD ExitCode;
	//GetExitCodeProcess(myThreadHandle, &ExitCode);
	//ExitProcess(ExitCode);

	//if vanitygen thread is suspended, then first restore it and then close it
	if ( ui->qpushbutton_8->text() == "Resume" )
	{
		this->suspendButton8_clicked();
		Sleep(1000);
	}

	DWORD ExitCode;
	GetExitCodeThread(myThreadHandle, &ExitCode);
	if ( ExitCode )
		stopThreadVanitygen=1;
}
void VanitygenPlus::clearButton10_clicked()
{
	ui->qplaintextedit_1->clear();
	//if vanitygen thread is not running or not suspended, then clear the follwing two variables:
	DWORD ExitCode;
	GetExitCodeThread(myThreadHandle, &ExitCode);
	if ( !ExitCode )
	{
		ui->qlabel_1->clear();
		summary.clear();
	}
}
void VanitygenPlus::addButton11_clicked()
{
	//1.
	//Do not add empty
	QString tmp = ui->qlineedit_1->text();
	tmp.remove(' ');
	if ( tmp.length() > 0 )
	{
		//2.
		//Do not add duplicate
		bool flag = false;	
		for( int i=0; i < ui->qcombobox_2->count(); i++ )
			if ( ui->qcombobox_2->itemText(i) == ui->qlineedit_1->text() )
			{
				flag = true;
				break;
			}
		if ( !flag )
		{
			//3.
			boost::filesystem::path pathRecords = LogPatterns; //GetDataDir() / LogPatterns;
			QFile file(pathRecords.string().c_str());
			if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
				QMessageBox::warning(this, "Error: Can't update file", file.errorString(), QMessageBox::Ok);
			else
			{
				//3.1.
				ui->qcombobox_2->addItem(ui->qlineedit_1->text());
				ui->qcombobox_2->setCurrentIndex(ui->qcombobox_2->count()-1);

				//3.2.
				for( int i=0; i < ui->qcombobox_2->count(); i++ )
				{
					file.write(ui->qcombobox_2->itemText(i).toStdString().c_str());
					file.write("\n");
				}
				file.close();
			}
		}
	}
}
void VanitygenPlus::removeButton12_clicked()
{
	//1.
	if ( ui->qcombobox_2->currentIndex() != -1 )
	{
		//2.
		boost::filesystem::path pathRecords = LogPatterns; //GetDataDir() / LogPatterns;
		QFile file(pathRecords.string().c_str());
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			QMessageBox::warning(this, "Error: Can't update file", file.errorString(), QMessageBox::Ok);
		else
		{
			//2.1.
			ui->qcombobox_2->removeItem(ui->qcombobox_2->currentIndex());
			ui->qcombobox_2->setCurrentIndex(-1);
			
			//2.2.
			for( int i=0; i < ui->qcombobox_2->count(); i++ )
			{
				file.write(ui->qcombobox_2->itemText(i).toStdString().c_str());
				file.write("\n");
			}
			file.close();
		}
	}
}
void VanitygenPlus::saveButton13_clicked()
{
	QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save current calculations"), LogCalculations,
        tr("calculations (*.txt);;All Files (*)"));
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		QMessageBox::warning(this, "Error: Can't save file", file.errorString(), QMessageBox::Ok);
	else
	{
		file.write(ui->qplaintextedit_1->toPlainText().toStdString().c_str());
		file.close();
	}
}
void VanitygenPlus::validateButton16_clicked()
{
	//This code is taken and modificated from src/qt/rpcconsole.cpp
	QString command="validateaddress";
	command +=" ";
	command +=ui->qlineedit_1->text();
	std::vector<std::string> args;
    if(!parseCommandLine(args, command.toStdString()))
    {
		QMessageBox::warning(this, "Error:","Parse error: unbalanced ' or \"",QMessageBox::Ok);
        return;
    }
    if(args.empty())
	{
		QMessageBox::warning(this, "Error:","The string is empty",QMessageBox::Ok);
        return; // Nothing to do
	}
	try
    {
        std::string strPrint;
        // Convert argument list to JSON objects in method-dependent way,
        // and pass it along with the method name to the dispatcher.
        json_spirit::Value result = tableRPC.execute(
            args[0],
            RPCConvertValues(args[0], std::vector<std::string>(args.begin() + 1, args.end())));

        // Format result reply
        if (result.type() == json_spirit::null_type)
            strPrint = "";
        else if (result.type() == json_spirit::str_type)
            strPrint = result.get_str();
        else
            strPrint = write_string(result, true);

		QString res = QString::fromStdString(strPrint);
		if ( res.length() > 0 )
			ui->qplaintextedit_1->appendPlainText(ui->qlineedit_1->text() + ": " + res);
		else
			QMessageBox::warning(this, "Error:", "The Validating is not successful");
    }
    catch (json_spirit::Object& objError)
    {
        try // Nice formatting for standard-format error
        {
            int code = find_value(objError, "code").get_int();
            std::string message = find_value(objError, "message").get_str();
			QMessageBox::warning(this, "Error:",QString::fromStdString(message) + " (code " + QString::number(code) + ")");
        }
        catch(std::runtime_error &) // raised when converting to invalid type, i.e. missing code or message
        {   // Show raw JSON object
			QMessageBox::warning(this, "Error:",QString::fromStdString(write_string(json_spirit::Value(objError), false)));
        }
    }
    catch (std::exception& e)
    {
		QMessageBox::warning(this, "Error:",QString("Error: ") + QString::fromStdString(e.what()));
    }
}
void VanitygenPlus::importButton17_clicked()
{
	QStringList words=ui->qlineedit_1->text().split(" ");
	if ( words.length() > 2 )
		QMessageBox::warning(this, "Error:","The format is not correct",QMessageBox::Ok);
	else
	if ( words.length() == 2 && words[0].length()>0 && words[1].length()>0 )
	//Import in encrypted wallet
		import_encryptedWallet(words[0], words[1]);
	else
	//Import in normal wallet
	{
		//This code is taken and modificated from src/qt/rpcconsole.cpp
		QString command="importprivkey";
		command +=" ";
		command +=ui->qlineedit_1->text();
		std::vector<std::string> args;
		if(!parseCommandLine(args, command.toStdString()))
		{
			QMessageBox::warning(this, "Error:","Parse error: unbalanced ' or \"",QMessageBox::Ok);
			return;
		}
		if(args.empty())
		{
			QMessageBox::warning(this, "Error:","The string is empty",QMessageBox::Ok);
			return; // Nothing to do
		}
		try
		{
			std::string strPrint;
			// Convert argument list to JSON objects in method-dependent way,
			// and pass it along with the method name to the dispatcher.
			json_spirit::Value result = tableRPC.execute(
				args[0],
				RPCConvertValues(args[0], std::vector<std::string>(args.begin() + 1, args.end())));

			// Format result reply
			if (result.type() == json_spirit::null_type)
				strPrint = "";
			else if (result.type() == json_spirit::str_type)
				strPrint = result.get_str();
			else
				strPrint = write_string(result, true);

			QString res = QString::fromStdString(strPrint);
			if ( res.length() == 0 )
				QMessageBox::warning(this, "The Importing was successful:","You can see the new-imported address in \"Addresses\" tab. After restart, the new-imported address will be in \"Receive\" tab!",QMessageBox::Ok);
			else
				QMessageBox::warning(this, "Error: The Importing is not successful",res,QMessageBox::Ok);
		}
		catch (json_spirit::Object& objError)
		{
			try // Nice formatting for standard-format error
			{
				int code = find_value(objError, "code").get_int();
				std::string message = find_value(objError, "message").get_str();
				QMessageBox::warning(this, "Error:",QString::fromStdString(message) + " (code " + QString::number(code) + ")");
			}
			catch(std::runtime_error &) // raised when converting to invalid type, i.e. missing code or message
			{   // Show raw JSON object
				QMessageBox::warning(this, "Error:",QString::fromStdString(write_string(json_spirit::Value(objError), false)));
			}
		}
		catch (std::exception& e)
		{
			QMessageBox::warning(this, "Error:",QString("Error: ") + QString::fromStdString(e.what()));
		}
	}
}
void VanitygenPlus::import_encryptedWallet(QString password, QString privkey)
{
	QString command;
	for(int i=0;i<3;i++)
		if ( i == 0 )
		{
			command ="walletpassphrase";
			command +=" ";
			command +=password;
			command +=" ";
			command +="10";
			if ( import_encryptedWallet_execute(command) == 0 )
				break;
		}
		else
		if ( i == 1 )
		{
			command ="importprivkey";
			command +=" ";
			command +=privkey;
			if ( import_encryptedWallet_execute(command) == 0 )
				break;
		}
		else
		if ( i == 2 )
		{
			command ="walletlock";
			if ( import_encryptedWallet_execute(command) == 0 )
				break;
			else
				QMessageBox::warning(this, "The Importing was successful:","You can see the new-imported address in \"Addresses\" tab. After restart, the new-imported address will be in \"Receive\" tab!",QMessageBox::Ok);
		}
}
unsigned short VanitygenPlus::import_encryptedWallet_execute(QString command)
{
	//This code is taken and modificated from src/qt/rpcconsole.cpp
	std::vector<std::string> args;
	if(!parseCommandLine(args, command.toStdString()))
	{
		QMessageBox::warning(this, "Error:","Parse error: unbalanced ' or \"",QMessageBox::Ok);
		return 0;
	}
	if(args.empty())
	{
		QMessageBox::warning(this, "Error:","The string is empty",QMessageBox::Ok);
		return 0; // Nothing to do
	}
	try
	{
		std::string strPrint;
		// Convert argument list to JSON objects in method-dependent way,
		// and pass it along with the method name to the dispatcher.
		json_spirit::Value result = tableRPC.execute(
			args[0],
			RPCConvertValues(args[0], std::vector<std::string>(args.begin() + 1, args.end())));

		// Format result reply
		if (result.type() == json_spirit::null_type)
			strPrint = "";
		else if (result.type() == json_spirit::str_type)
			strPrint = result.get_str();
		else
			strPrint = write_string(result, true);

		QString res = QString::fromStdString(strPrint);
		if ( res.length() == 0 )
		//success
			return 1;
		else
		//failure
		{
			QMessageBox::warning(this, "Error:",res,QMessageBox::Ok);
			return 0;
		}
	}
	catch (json_spirit::Object& objError)
	{
		try // Nice formatting for standard-format error
		{
			int code = find_value(objError, "code").get_int();
			std::string message = find_value(objError, "message").get_str();
			QMessageBox::warning(this, "Error:",QString::fromStdString(message) + " (code " + QString::number(code) + ")");
			return 0;
		}
		catch(std::runtime_error &) // raised when converting to invalid type, i.e. missing code or message
		{   // Show raw JSON object
			QMessageBox::warning(this, "Error:",QString::fromStdString(write_string(json_spirit::Value(objError), false)));
			return 0;
		}
	}
	catch (std::exception& e)
	{
		QMessageBox::warning(this, "Error:",QString("Error: ") + QString::fromStdString(e.what()));
		return 0;
	}
}
void VanitygenPlus::exportButton18_clicked()
{
	QStringList words=ui->qlineedit_1->text().split(" ");
	if ( words.length() > 2 )
		QMessageBox::warning(this, "Error:","The format is not correct",QMessageBox::Ok);
	else
	if ( words.length() == 2 && words[0].length()>0 && words[1].length()>0 )
	//Export from encrypted wallet
		export_encryptedWallet(words[0], words[1]);
	else
	//Export from normal wallet
	{
		//This code is taken and modificated from src/qt/rpcconsole.cpp
		QString command="dumpprivkey";
		command +=" ";
		command +=ui->qlineedit_1->text();
		std::vector<std::string> args;
		if(!parseCommandLine(args, command.toStdString()))
		{
			QMessageBox::warning(this, "Error:","Parse error: unbalanced ' or \"",QMessageBox::Ok);
			return;
		}
		if(args.empty())
		{
			QMessageBox::warning(this, "Error:","The string is empty",QMessageBox::Ok);
			return; // Nothing to do
		}
		try
		{
			std::string strPrint;
			// Convert argument list to JSON objects in method-dependent way,
			// and pass it along with the method name to the dispatcher.
			json_spirit::Value result = tableRPC.execute(
				args[0],
				RPCConvertValues(args[0], std::vector<std::string>(args.begin() + 1, args.end())));

			// Format result reply
			if (result.type() == json_spirit::null_type)
				strPrint = "";
			else if (result.type() == json_spirit::str_type)
				strPrint = result.get_str();
			else
				strPrint = write_string(result, true);

			QString res = QString::fromStdString(strPrint);
			if ( res.length() > 0 )
				ui->qplaintextedit_1->appendPlainText(ui->qlineedit_1->text() + ": " + res);
			else
				QMessageBox::warning(this, "Error:", "The Exporting is not successful");
		}
		catch (json_spirit::Object& objError)
		{
			try // Nice formatting for standard-format error
			{
				int code = find_value(objError, "code").get_int();
				std::string message = find_value(objError, "message").get_str();
				QMessageBox::warning(this, "Error:",QString::fromStdString(message) + " (code " + QString::number(code) + ")");
			}
			catch(std::runtime_error &) // raised when converting to invalid type, i.e. missing code or message
			{   // Show raw JSON object
				QMessageBox::warning(this, "Error:",QString::fromStdString(write_string(json_spirit::Value(objError), false)));
			}
		}
		catch (std::exception& e)
		{
			QMessageBox::warning(this, "Error:",QString("Error: ") + QString::fromStdString(e.what()));
		}
	}
}
void VanitygenPlus::export_encryptedWallet(QString password, QString address)
{
	QString command;
	for(int i=0;i<3;i++)
		if ( i == 0 )
		{
			command ="walletpassphrase";
			command +=" ";
			command +=password;
			command +=" ";
			command +="10";
			if ( export_encryptedWallet_execute(command,i) == 0 )
				break;
		}
		else
		if ( i == 1 )
		{
			command ="dumpprivkey";
			command +=" ";
			command +=address;
			if ( export_encryptedWallet_execute(command,i) == 0 )
				break;
		}
		else
		if ( i == 2 )
		{
			command ="walletlock";
			if ( export_encryptedWallet_execute(command,i) == 0 )
				break;
		}
}
unsigned short VanitygenPlus::export_encryptedWallet_execute(QString command, unsigned short index)
{
	//This code is taken and modificated from src/qt/rpcconsole.cpp
	std::vector<std::string> args;
	if(!parseCommandLine(args, command.toStdString()))
	{
		QMessageBox::warning(this, "Error:","Parse error: unbalanced ' or \"",QMessageBox::Ok);
		return 0;
	}
	if(args.empty())
	{
		QMessageBox::warning(this, "Error:","The string is empty",QMessageBox::Ok);
		return 0; // Nothing to do
	}
	try
	{
		std::string strPrint;
		// Convert argument list to JSON objects in method-dependent way,
		// and pass it along with the method name to the dispatcher.
		json_spirit::Value result = tableRPC.execute(
			args[0],
			RPCConvertValues(args[0], std::vector<std::string>(args.begin() + 1, args.end())));

		// Format result reply
		if (result.type() == json_spirit::null_type)
			strPrint = "";
		else if (result.type() == json_spirit::str_type)
			strPrint = result.get_str();
		else
			strPrint = write_string(result, true);

		QString res = QString::fromStdString(strPrint);
		if ( index == 0 || index == 2 )
		{
			if ( res.length() == 0 )
			//success
				return 1;
			else
			//failure
			{
				QMessageBox::warning(this, "Error:",res,QMessageBox::Ok);
				return 0;
			}
		}
		else
		if ( index == 1 )
		{
			if ( res.length() > 0 )
			//success
			{
				ui->qplaintextedit_1->appendPlainText(ui->qlineedit_1->text() + ": " + res);
				return 1;
			}
			else
			//failure
			{
				QMessageBox::warning(this, "Error:", "The Exporting is not successful");
				return 0;
			}
		}
		else
		//never happen
			return 0;
	}
	catch (json_spirit::Object& objError)
	{
		try // Nice formatting for standard-format error
		{
			int code = find_value(objError, "code").get_int();
			std::string message = find_value(objError, "message").get_str();
			QMessageBox::warning(this, "Error:",QString::fromStdString(message) + " (code " + QString::number(code) + ")");
			return 0;
		}
		catch(std::runtime_error &) // raised when converting to invalid type, i.e. missing code or message
		{   // Show raw JSON object
			QMessageBox::warning(this, "Error:",QString::fromStdString(write_string(json_spirit::Value(objError), false)));
			return 0;
		}
	}
	catch (std::exception& e)
	{
		QMessageBox::warning(this, "Error:",QString("Error: ") + QString::fromStdString(e.what()));
		return 0;
	}
}
void VanitygenPlus::changeCombobox2_currentIndexChanged()
{
	ui->qlineedit_1->setText(ui->qcombobox_2->itemText(ui->qcombobox_2->currentIndex()));
}

DWORD WINAPI myThread(LPVOID lpParameter)
{
	switch (whichVanitygenplus)
	{
		case 1:
			vanitygen(argcDynamic, argvDynamic);
			break;
		case 2:
#if OpenCL == 1
			oclvanitygen(argcDynamic, argvDynamic);
#else
#endif
			break;
		case 3:
#if OpenCL == 1
			oclvanityminer(argcDynamic, argvDynamic);
#else
#endif
			break;
		case 4:
			keyconv(argcDynamic, argvDynamic);
			break;
		default:
			break;
	}

	return 0;
}

int __cdecl printf2(const char *format, ...)
{
	va_list argptr;
    va_start(argptr, format);
	int ret = printf2fprintf2(format, argptr);
	va_end(argptr);
	return ret;
}
int __cdecl fprintf2(FILE *__restrict __stream,  const char *format, ...)
{
	va_list argptr;
    va_start(argptr, format);
	int ret = printf2fprintf2(format, argptr);
	va_end(argptr);
	return ret;
}
int printf2fprintf2(const char *format, va_list argptr)
{
	static unsigned int i=1;

	//1.
	//With static buffer:
	//char buf[1024];
	//int ret = vsnprintf(buf, sizeof(buf), format, argptr);
	//if ( *(buf+0) == '[' )
	//	printf("\n%d%s",i++,buf);
	//else
	//	printf("\n%s",buf);
	//return ret;
	//
	//or
	//2.
	//With dynamic buffer:
	int maxsize = _vscprintf(format, argptr);
	if ( maxsize <= 0 )
		return 0;
	else
		if ( maxsize > 10000 )
		//possible hack intervention -> stop all
			return 0;
	char *buf=new char[maxsize+1];
	if ( !buf )
		return 0;
	*(buf + maxsize) = '\0';
	int ret = vsnprintf(buf, maxsize, format, argptr);
	char *actualend=strstr(buf,"               ");
	if ( actualend )
	{
		int actualsize = actualend - buf;
		if ( actualsize <= 0 )
		{
			delete [] buf;
			return 0;
		}
		else
			if ( actualsize > 10000 )
			//possible hack intervention -> stop all
			{
				delete [] buf;
				return 0;
			}
		char *actualbuf = new char[actualsize+1];
		if ( !actualbuf )
		{
			delete [] buf;
			return 0;
		}
		*(actualbuf + actualsize) = '\0';
		strncpy(actualbuf,buf,actualsize);
		delete [] buf;
		buf = actualbuf;
	}
	if ( *(buf+0) == '[' )
	{
		//printf("\n%d%s",i++,buf);
		summary.append("\n");
		summary.append(QString::number(i++) + buf);
	}
	else
	{
		//printf("\n%s",buf);
		summary.append(buf);
	}
	QString temp=QString::fromLatin1(buf);
	QStringList tmp = temp.split("\n");
	temp = tmp[0];
	if ( temp.length() > 110 )
		temp.truncate(110);
	pointerUI->qlabel_1->setText(temp);
	if ( temp.startsWith("FINISH: ") )
		i=1;
	delete [] buf;
	return maxsize;
}
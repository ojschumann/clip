#include "mwdataprovider.h"
#include "image/dataproviderfactory.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QStringList>
#include <iostream>
#include <cmath>

#include "tools/xmltools.h"

using namespace std;
/*
 Multiwire Image class for *.hs2 (and someday *.his) files
 written by Gregory Tucker supported by Ames Laboratory and Iowa State University
 based on basdataprovider written by Olaf Schumann

 version 1, March 21, 2011
*/

MWDataProvider::MWDataProvider(QObject *parent) :
    DataProvider(parent)
{
}

MWDataProvider::~MWDataProvider() {
}


QStringList MWDataProvider::Factory::fileFormatFilters() {
  //return QStringList() << "hs2" << "his"; //hs2 is 16-bit, his is 8-bit
  return QStringList() << "hs2";
}

DataProvider* MWDataProvider::Factory::getProvider(QString filename, QObject *parent) {
  //load in information about the file
  QFileInfo info(filename);

  // Return if file does not exist
  if (!info.exists() || !info.isReadable()) return NULL;

  // Check if suffix is .hs2 or .his
  QString suffix = info.suffix().toLower();
  //if (suffix!="hs2" || suffix!="his") return NULL;
  if (suffix!="hs2") return NULL;

  //load in the histogram image file
  QFile hs2File(info.filePath());

  if (!hs2File.open(QFile::ReadOnly)) return NULL;

  QMap<QString, QVariant> headerData;
  headerData.insert("OriginalFilename",info.filePath());
  headerData.insert("IP-Size",1);
  int xpxsize = 300000/256; // x-pixel size in microns
  headerData.insert("X-PixelSizeUM",xpxsize);
  int ypxsize = 300000/256; // y-pixel size in microns
  headerData.insert("Y-PixelSizeUM",ypxsize);
/*
  if suffix=="hs2" {
    int bpp = 16;
  else
    int bpp = 8;
  }
*/
  int bpp = 16;
  headerData.insert("BitsPerPixel",bpp);
  headerData.insert("Width",256);  //pixels
  headerData.insert("Height",256); //pixels
  headerData.insert("Sensitivity",1);
  headerData.insert("Latitude",0);
  headerData.insert("Exposure Date",0);
  headerData.insert("UnixTime",0);
  headerData.insert("OverflowPixels",0);
  headerData.insert("Comment","");

  quint64 unixtime = headerData["UnixTime"].toULongLong();
  QDateTime date;
  date.setTime_t(unixtime);
  headerData["UnixTime"] = QVariant(QString("%1 (%2)").arg(unixtime).arg(date.toString(Qt::DefaultLocaleLongDate)));

  int pixelCount = headerData["Width"].toInt()*headerData["Height"].toInt();
  //int bytesPerPixel = (headerData["BitsPerPixel"].toInt()>8)?2:1;



  QVector<float> pixelData(pixelCount);

  QDataStream in(&hs2File);
  in.setByteOrder(QDataStream::BigEndian);
  quint16 pixel;
  for (int i=0; i<pixelCount; i++) {
    in >> pixel;
    pixelData[i]=1.0*pixel;
    // Multiwire NortStar displays high intensity spots as dark spots on a white background
    // flipping the 16-bit value ensures Clip does the same.
    //pixelData[i]=65535.0-1.0*pixel; //2^16-1 - pixel value
  }

  /*
  There is more useful data stored after the image, however I'm not sure of it's exact format 
  so I've left it out at the moment. Among other things, the user-defined sample-to-detector
  distance is stored in every hs2 file; if this was read automatically it could save on one
  small annoyance.
  */

  MWDataProvider* provider = new MWDataProvider(parent);
  provider->insertFileInformation(filename);
  provider->providerInformation.unite(headerData);
  provider->pixelData = pixelData;
  return provider;
}

const void* MWDataProvider::getData() {
  return (void*)pixelData.data();
}

QSize MWDataProvider::size() {
  return QSize(providerInformation["Width"].toInt(), providerInformation["Height"].toInt());
}

int MWDataProvider::bytesCount() {
  return pixelData.size()*sizeof(float);
}

int MWDataProvider::pixelCount() {
  return pixelData.size();
}

DataProvider::Format MWDataProvider::format() {
  return Float32;
}

QSizeF MWDataProvider::absoluteSize() {
  QSize s(size());
  return QSizeF(0.001*s.width()*providerInformation["X-PixelSizeUM"].toDouble(), 0.001*s.height()*providerInformation["Y-PixelSizeUM"].toDouble());
}


bool MWRegisterOK = DataProviderFactory::registerImageLoader(0, new MWDataProvider::Factory());

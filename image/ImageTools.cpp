#include "ImageTools.h"

#include <iostream>
#include <stdio.h>
#include <algorithm>

#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

//TODO: For BG Fitting by splines, use http://www.netlib.org/dierckx/index.html (Scipy.interpolate)

ImageTransfer::ImageTransfer(): transferedData(), values(), imgData() {
  for (int i=4; i--; )
    curves.append(BezierCurve());
  qimg=NULL;
  schedTransfer=false;
  CWRSteps=0;
  flipImg=false;
}

ImageTransfer::~ImageTransfer() {
  deleteData();
}

void ImageTransfer::deleteData() {
  if (qimg!=NULL) {
    delete qimg;
    qimg=NULL;
  }
}

void ImageTransfer::setData(int width, int height, int format, char *inData, int len) {
  //cout << "Img Set data" << endl;
  deleteData();
  imageWidth=width;
  imageHeight=height;
  rawType=format;


  if (format==0) {
    // floating point
    int N=len/4;
#ifdef __DEBUG__
    cout << "Imgsize " << N << " " << width*height << endl;
#endif
    float* arr=(float*)inData;

    values.clear();
    QList<QList<int> > indexForValues;

    for (int n=N; n--; ) {
      float val=arr[n];
      QList<float>::iterator iter=qLowerBound(values.begin(), values.end(), val);
      int idx=iter-values.begin();
      if (idx>=values.size() or values[idx]!=val) {
        // Value is new
        values.insert(idx, val);
        indexForValues.insert(idx,QList<int>());
      }
      indexForValues[idx].append(n);
    }

#ifdef __DEBUG__
    cout << "Values: " << values.size() << endl;
#endif

    float norm=1.0/values.last();
    for (int n=values.size(); n--; ) {
      values[n]*=norm;
    }

    imgData.resize(N);
    for (int i=indexForValues.size(); i--; ) {
      for (int j=indexForValues[i].size(); j--; ) {
        imgData[indexForValues[i][j]]=i;
      }
    }

#ifdef __DEBUG__
    cout << "imgData Set: " << endl;
#endif


    transferedData.resize(N);
  } else if (format==1) {
    // 24bit RGB
    imgData.resize(len/3);
    for (int n=imgData.size(); n--; )
      imgData[n]=(inData[3*n+2]<<16) | (inData[3*n+1]<<8) | (inData[3*n]);
    transferedData.resize(len/3);
  }

  doImgRotation(0, false);
  schedTransfer=true;
}


void ImageTransfer::setTransferCurves(QList<BezierCurve> bc) {
  curves=bc;
  schedTransfer=true;
}

QList<BezierCurve> ImageTransfer::getTransferCurves() {
  return curves;
}

void ImageTransfer::doImgRotation(int _CWRSteps, bool flip) {
  CWRSteps=(CWRSteps+_CWRSteps)%4;
  flipImg=flipImg xor flip;
#ifdef __DEBUG__
  cout << "doImgRot " << _CWRSteps << " " << flip << " " << imageWidth << " " << imageHeight << " " << imgData.size() << endl;
#endif

  QVector<int> tmp(imgData);

  int w=imageWidth;
  int h=imageHeight;
  int m;
  if (flip) {
    if (_CWRSteps==0) {
      for (int n=tmp.size(); n--; )
        imgData[w-1-n%w+w*(n/w)]=tmp[n];
    } else if (_CWRSteps==1) {
      for (int n=tmp.size(); n--; )
        imgData[n/w+h*(n%w)]=tmp[n];
    } else if (_CWRSteps==2) {
      for (int n=tmp.size(); n--; )
        imgData[n%w+w*(h-1-n/w)]=tmp[n];
    } else if (_CWRSteps==3) {
      for (int n=tmp.size(); n--; )
        imgData[h-1-n/w+h*(w-1-n%w)]=tmp[n];
    }
  } else {
    if (_CWRSteps==1) {
      for (int n=tmp.size(); n--; )
        imgData[n/w+h*(w-1-n%w)]=tmp[n];
    } else if (_CWRSteps==2) {
      for (int n=tmp.size(); n--; )
        imgData[w-1-n%w+w*(h-1-n/w)]=tmp[n];
    } else if (_CWRSteps==3) {
      for (int n=tmp.size(); n--; )
        imgData[h-1-n/w+h*(n%w)]=tmp[n];
    }
  }
  if (_CWRSteps%2==1)
    qSwap(imageWidth, imageHeight);


  if (qimg!=NULL) {
    delete qimg;
    qimg=NULL;
  }
#ifdef __DEBUG__
  cout << "New QImage" << transferedData.size() << endl;
#endif
  qimg = new QImage((unsigned char *)transferedData.data(), imageWidth, imageHeight, QImage::Format_RGB32);
  schedTransfer=true;
}


void ImageTransfer::doFloatTransfer() {
#ifdef __DEBUG__
  cout << "float transfer: calc Colortable" << endl;
#endif

  QList<float> vMap = curves[0].mapSorted(values);
  QVector<quint32> colors(vMap.size());

  int hints[4];
  for (int i=4; i--; ) hints[i]=0;
  for (int n=vMap.size(); n--; ) {
    float vval=vMap[n];
    int rgbVal=0xFF;
    for (int i=0; i<3; i++) {
      rgbVal<<=8;
      rgbVal|=(int)(255.0*curves[i+1](vval,hints[i+1]));
    }
    colors[n]=rgbVal;
  }
#ifdef __DEBUG__
  cout << "float transfer: Transfer Image" << endl;
#endif
  for (int n=transferedData.size(); n--; ) {
    transferedData[n]=colors[imgData[n]];
  }
}

void ImageTransfer::doRGBTransfer() {
  cout << "Do RGB Transfer" << endl;
  QList<float> valCurve=curves[0].range(0, 1.0/255, 256);
  QVector<int> rgbCurve[3];
  for (int i=3; i--; ) {
    QList<float> tmp=curves[i+1].mapSorted(valCurve);
    rgbCurve[i].resize(256);
    int shift=16-8*i;
    for (int n=256; n-- ; ) {
      int val=((int)(n*tmp[n]))<<shift;
      rgbCurve[i][n]=val;
    }
  }
  union rgbTuple {
    int val;
    char argb[4];
  };
  for (int n=transferedData.size(); n--; ) {
    rgbTuple t;
    t.val=imgData[n];
    int rgbVal=0xFF000000 | rgbCurve[0][t.argb[0]] | rgbCurve[1][t.argb[1]] | rgbCurve[2][t.argb[2]];
    transferedData[n]=rgbVal;
  }
}

void ImageTransfer::doTransfer() {
  if (rawType==0) {
    doFloatTransfer();
  } else if (rawType==1) {
    doRGBTransfer();
  }
}

QImage* ImageTransfer::qImg() {
#ifdef __DEBUG__
  cout << "Image Request " << schedTransfer << endl;
#endif
  if (schedTransfer) {
    doTransfer();
    schedTransfer=false;
  }
  return qimg;
}

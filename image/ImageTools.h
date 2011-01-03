#ifndef __IMAGETOOLS_H__
#define __IMAGETOOLS_H__

#include <string>

#include <QImage>
#include <tools/vec3D.h>
#include <image/BezierCurve.h>

//TODO: Remove this class

class ImageTransfer {
public:
  ImageTransfer();
  ~ImageTransfer();

  void setData(int width, int height, int format, char *data, int len);
  QList<BezierCurve> getTransferCurves();
  void setTransferCurves(QList<BezierCurve> bc);
  void doImgRotation(int CWRSteps, bool flip);

  QImage* qImg();
private:
  void deleteData();
  void doTransfer();
  void doRGBTransfer();
  void doFloatTransfer();

  int imageWidth;
  int imageHeight;


  int rawType;

  QVector<quint32> transferedData;

  // Needed for FloatTransfer
  QList<float> values;

  QVector<int> imgData;

  QList<BezierCurve> curves;

  QImage* qimg;
  bool schedTransfer;

  int CWRSteps;
  bool flipImg;
};

#endif
